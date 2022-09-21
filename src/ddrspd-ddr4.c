// SPDX-License-Identifier: GPL-2.0-or-later

// Copyright 2014-2016 Freescale Semiconductor, Inc.
// Copyright 2017-2018 NXP Semiconductor

// Copyright 2020 RnD Center "ELVEES", JSC

#include <string.h>
#include <common.h>
#include <ddrcfg.h>
#include <ddrspd.h>
#include <i2c.h>
#include <plat/plat.h>

static uint8_t i2c_addr_get(int ctrl_id)
{
	uint8_t i2c_addr[2];
	switch (ctrl_id) {
	case 0:
		i2c_addr[0] = CONFIG_DIMM0_I2C_ADDR;
		i2c_addr[1] = CONFIG_DIMM1_I2C_ADDR;
		break;
	case 1:
		i2c_addr[0] = CONFIG_DIMM2_I2C_ADDR;
		i2c_addr[1] = CONFIG_DIMM3_I2C_ADDR;
		break;
	case 2:
		i2c_addr[0] = CONFIG_DIMM4_I2C_ADDR;
		i2c_addr[1] = CONFIG_DIMM5_I2C_ADDR;
		break;
	case 3:
		i2c_addr[0] = CONFIG_DIMM6_I2C_ADDR;
		i2c_addr[1] = CONFIG_DIMM7_I2C_ADDR;
		break;
	default:
		return 0;
	}
	/* TODO: Use all DIMM slots for every controller */
	return i2c_addr[0];
}

int spd_get(int ctrl_id, struct ddr4_spd *spd)
{
	int ret, size, i2c_ctrl_id;

	i2c_ctrl_id = i2c_ctrl_id_get(ctrl_id);
	if (i2c_ctrl_id < 0)
		return i2c_ctrl_id;
	i2c_cfg(i2c_ctrl_id, i2c_addr_get(ctrl_id));

#ifdef CONFIG_DRAM_TYPE_DDR4
	size = sizeof(struct ddr4_spd);
#endif

	ret = i2c_spd_read(i2c_ctrl_id, (uint8_t *)spd, size);
	if (ret)
		return ret;

	return 0;
}

/*
 * Calculate the density of each physical rank for DDR4.
 * Returned size is in bytes.
 *
 * Total DIMM size =
 * sdram capacity(bit) / 8 * primary bus width / sdram width
 *                     * Logical Ranks per DIMM
 *
 * where: sdram capacity = spd byte4[3:0]
 *        primary bus width = spd byte13[2:0]
 *        sdram width = spd byte12[2:0]
 *        Logical Ranks per DIMM = spd byte12[5:3] for SDP, DDP, QDP
 *                                 spd byte12{5:3] * spd byte6[6:4] for 3DS
 *
 * To simplify each rank size = total DIMM size / Number of Package Ranks
 * where Number of Package Ranks = spd byte12[5:3]
 *
 * SPD byte4 - sdram density and banks
 *	bit[3:0]	size(bit)	size(byte)
 *	0000		256Mb		32MB
 *	0001		512Mb		64MB
 *	0010		1Gb		128MB
 *	0011		2Gb		256MB
 *	0100		4Gb		512MB
 *	0101		8Gb		1GB
 *	0110		16Gb		2GB
 *      0111		32Gb		4GB
 *
 * SPD byte13 - module memory bus width
 *	bit[2:0]	primary bus width
 *	000		8bits
 *	001		16bits
 *	010		32bits
 *	011		64bits
 *
 * SPD byte12 - module organization
 *	bit[2:0]	sdram device width
 *	000		4bits
 *	001		8bits
 *	010		16bits
 *	011		32bits
 *
 * SPD byte12 - module organization
 *	bit[5:3]	number of package ranks per DIMM
 *	000		1
 *	001		2
 *	010		3
 *	011		4
 *
 * SPD byte6 - SDRAM package type
 *	bit[6:4]	Die count
 *	000		1
 *	001		2
 *	010		3
 *	011		4
 *	100		5
 *	101		6
 *	110		7
 *	111		8
 *
 * SPD byte6 - SRAM package type
 *	bit[1:0]	Signal loading
 *	00		Not specified
 *	01		Multi load stack
 *	10		Sigle load stack (3DS)
 *	11		Reserved
 */
static uint64_t ranksize_get(struct ddr4_spd *spd)
{
	uint64_t rank_size;
	int sdram_die_size = 0;
	int nbit_primary_bus_width = 0;
	int nbit_sdram_width = 0;
	int die_count = 0;
	uint8_t package_3ds;

	if ((spd->density_banks & 0xf) <= 7)
		sdram_die_size = (spd->density_banks & 0xf) + 28;
	if ((spd->bus_width & 0x7) < 4)
		nbit_primary_bus_width = (spd->bus_width & 0x7) + 3;
	if ((spd->organization & 0x7) < 4)
		nbit_sdram_width = (spd->organization & 0x7) + 2;

	package_3ds = (spd->package_type & 0x3) == 0x2;
	if ((spd->package_type & 0x80) && !package_3ds) { /* other than 3DS */
		print_dbg("Unsupported SDRAM package type\n");
		return 0;
	}

	if (package_3ds)
		die_count = (spd->package_type >> 4) & 0x7;

	rank_size = 1ULL
		    << (sdram_die_size - 3 + nbit_primary_bus_width - nbit_sdram_width + die_count);

	return rank_size;
}

/*
 * CRC16 compute for DDR4 SPD
 * Copied from DDR4 SPD spec.
 */
static int crc16(uint8_t *ptr, int count)
{
	int crc = 0, i;

	while (--count >= 0) {
		crc = crc ^ (int)*ptr++ << 8;
		for (i = 0; i < 8; ++i)
			if (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			else
				crc = crc << 1;
	}

	return crc & 0xffff;
}

static int spd_crc_check(struct ddr4_spd *spd)
{
	uint8_t *p = (uint8_t *)spd;
	int csum16, len = 126;
	uint8_t crc_lsb; /* SPD byte 126 */
	uint8_t crc_msb; /* SPD byte 127 */

	csum16 = crc16(p, len);

	crc_lsb = (uint8_t)(csum16 & 0xff);
	crc_msb = (uint8_t)(csum16 >> 8);

	if (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb)
		return -ESPDCHECKSUM;

	p = (uint8_t *)((unsigned long)spd + 128);
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (uint8_t)(csum16 & 0xff);
	crc_msb = (uint8_t)(csum16 >> 8);

	if (spd->mod_section.uc[126] != crc_lsb || spd->mod_section.uc[127] != crc_msb)
		return -ESPDCHECKSUM;

	return 0;
}

static void spd_error_fix(struct ddr4_spd *spd)
{
	uint8_t *ptr;
	const uint8_t udimm_rc_e_dq[18] = { 0x0c, 0x2c, 0x15, 0x35, 0x15, 0x35, 0x0b, 0x2c, 0x15,
					    0x35, 0x0b, 0x35, 0x0b, 0x2c, 0x0b, 0x35, 0x15, 0x36 };
	int spd_error = 0, i;

	if ((spd->mod_section.unbuffered.mod_height & 0xe0) == 0 &&
	    (spd->mod_section.unbuffered.ref_raw_card == 0x04)) {
		/* Fix SPD error found on DIMMs with raw card E0 */
		for (i = 0; i < 18; i++) {
			if (spd->mapping[i] == udimm_rc_e_dq[i])
				continue;
			spd_error = 1;
			print_dbg("SPD byte%d 0x%x, should be 0x%x\n", 60 + i, spd->mapping[i],
				  udimm_rc_e_dq[i]);
			ptr = (uint8_t *)&spd->mapping[i];
			*ptr = udimm_rc_e_dq[i];
		}
		if (spd_error)
			print_dbg("SPD DQ mapping error fixed\n");
	}
}

#define spd2ps(mtb, ftb) ((mtb)*mtb_ps + (ftb)*ftb_ps)

int spd_parse(struct ddr4_spd *spd, struct ddr_cfg *cfg)
{
	int i, ret, mtb_ps, ftb_ps;
	uint32_t tmp;

	ret = spd_crc_check(spd);
	if (ret)
		return ret;

	/*
	 * The part name in ASCII in the SPD EEPROM is not null terminated.
	 * Guarantee null termination here by presetting all bytes to 0
	 * and copying the part name in ASCII from the SPD onto it
	 */
	memset(cfg->mpart, 0, sizeof(cfg->mpart));
	if ((spd->info_size_crc & 0xF) > 2)
		memcpy(cfg->mpart, spd->mpart, sizeof(cfg->mpart) - 1);

	cfg->ranks = ((spd->organization >> 3) & 0x7) + 1;
	cfg->rank_size = ranksize_get(spd);
	cfg->full_size = cfg->ranks * cfg->rank_size;
	cfg->die_size = (spd->density_banks & 0xf) + 28;
	cfg->primary_sdram_width = 1 << (3 + (spd->bus_width & 0x7));

	if ((spd->bus_width >> 3) & 0x3)
		cfg->ecc_sdram_width = 8;
	else
		cfg->ecc_sdram_width = 0;

	cfg->full_sdram_width = cfg->primary_sdram_width + cfg->ecc_sdram_width;
	cfg->device_width = 1 << ((spd->organization & 0x7) + 2);

	cfg->package_3ds = (spd->package_type & 0x3) == 0x2 ? (spd->package_type >> 4) & 0x7 : 0;

	cfg->mirrored_dimm = 0;
	cfg->registered_dimm = 0;

	switch (spd->module_type & DDR4_SPD_MODULETYPE_MASK) {
	case DDR4_SPD_MODULETYPE_RDIMM:
		/* TBD */
		break;
	case DDR4_SPD_MODULETYPE_UDIMM:
	case DDR4_SPD_MODULETYPE_SO_DIMM:
		if (spd->mod_section.unbuffered.addr_mapping & 0x1)
			cfg->mirrored_dimm = 1;
		spd_error_fix(spd);
		break;
	default:
		return -EDIMMCFG;
	}

	/* SDRAM device parameters */
	cfg->row_addr_bits = ((spd->addressing >> 3) & 0x7) + 12;
	cfg->col_addr_bits = (spd->addressing & 0x7) + 9;
	cfg->bank_addr_bits = ((spd->density_banks >> 4) & 0x3) + 2;
	cfg->bank_group_bits = (spd->density_banks >> 6) & 0x3;

	/*
	 * SPD spec doesn't have ECC bit. Try to guess about ECC capability.
	 */
	// 	if (cfg->ecc_sdram_width)
	// 		cfg->edc_config = 0x02;
	// 	else
	// 		cfg->edc_config = 0x00;

	if ((spd->timebases & 0xf) == 0x0) {
		mtb_ps = 125;
		ftb_ps = 1;
	} else {
		return -EDIMMCFG;
	}

	cfg->tckmin = spd2ps(spd->tck_min, spd->fine_tck_min);
	cfg->tckmax = spd2ps(spd->tck_max, spd->fine_tck_max);

#if defined(CONFIG_DRAM_RATE_SPD_MAXIMUM)
	cfg->tck = cfg->tckmin;
#elif defined(CONFIG_DRAM_RATE_SPD_MINIMUM)
	cfg->tck = cfg->tckmax;
#elif defined(CONFIG_DRAM_RATE_CUSTOM) && defined(CONFIG_DRAM_TCK)
	cfg->tck = CONFIG_DRAM_TCK;
#endif

	if (cfg->tck < cfg->tckmin || cfg->tck > cfg->tckmax)
		return -EDIMMCFG;

#if !defined(CONFIG_DRAM_CAS_AUTO)
	cfg->taa = CONFIG_DRAM_CAS_LATENCY;
#else
	cfg->taa = ps2clk_jedec(spd2ps(spd->taa_min, spd->fine_taa_min), cfg->tck);
#endif

	cfg->trcd = ps2clk_jedec(spd2ps(spd->trcd_min, spd->fine_trcd_min), cfg->tck);
	cfg->trp = ps2clk_jedec(spd2ps(spd->trp_min, spd->fine_trp_min), cfg->tck);

	tmp = (((spd->tras_trc_ext & 0xf) << 8) | spd->tras_min_lsb) * mtb_ps;
	cfg->trasmin = ps2clk_jedec(tmp, cfg->tck);

	tmp = spd2ps((((spd->tras_trc_ext & 0xf0) << 4) | spd->trc_min_lsb), spd->fine_trc_min);
	cfg->trc = ps2clk_jedec(tmp, cfg->tck);

	tmp = ((spd->trfc1_min_msb << 8) | (spd->trfc1_min_lsb)) * mtb_ps;
	cfg->trfc1 = ps2clk_jedec(tmp, cfg->tck);

	tmp = ((spd->trfc2_min_msb << 8) | (spd->trfc2_min_lsb)) * mtb_ps;
	cfg->trfc2 = ps2clk_jedec(tmp, cfg->tck);

	tmp = ((spd->trfc4_min_msb << 8) | (spd->trfc4_min_lsb)) * mtb_ps;
	cfg->trfc4 = ps2clk_jedec(tmp, cfg->tck);

	tmp = (((spd->tfaw_msb & 0xf) << 8) | spd->tfaw_min) * mtb_ps;
	cfg->tfaw = ps2clk_jedec(tmp, cfg->tck);

	cfg->trrds = ps2clk_jedec(spd2ps(spd->trrds_min, spd->fine_trrds_min), cfg->tck);
	cfg->trrdl = ps2clk_jedec(spd2ps(spd->trrdl_min, spd->fine_trrdl_min), cfg->tck);
	cfg->tccdl = ps2clk_jedec(spd2ps(spd->tccdl_min, spd->fine_tccdl_min), cfg->tck);

	// 	if (cfg->package_3ds) {
	// 		if (pdimm->die_density <= 0x4) {
	// 			pdimm->trfc_slr_ps = 260000;
	// 		} else if (pdimm->die_density <= 0x5) {
	// 			pdimm->trfc_slr_ps = 350000;
	// 		} else {
	// 			printf("WARN: Unsupported logical rank density 0x%x\n",
	// 			       pdimm->die_density);
	// 		}
	// 	}

	for (i = 0; i < 18; i++)
		cfg->dq_mapping[i] = spd->mapping[i];

	cfg->dq_mapping_ors = ((spd->mapping[0] >> 6) & 0x3) == 0 ? 1 : 0;

	return 0;
}

#ifdef CONFIG_DEBUG_SPD_DUMP
int spd_dump(uint8_t *buf)
{
	uint8_t read_val;

	for (int i = 0; i < 16; i++) {
		printf("\n%03x  ", 16 * i);
		for (int j = 0; j < 16; j++) {
			read_val = *(buf)++;
			printf("%02x ", read_val);
		}
	}
	printf("\n");
	return 0;
}
#endif
