// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2021 RnD Center "ELVEES", JSC

#include <bitops.h>
#include <common.h>
#include <ddrcfg.h>
#include <ddrspd.h>
#include <plat/plat.h>

#ifdef CONFIG_DEBUG_DDRCFG_PRINT
void ddrcfg_print(struct ddr_cfg *cfg)
{
	printf("Ranks:                                                %u\n", cfg->ranks);
	printf("Rank size:                                            %u MB\n",
	       (uint32_t)(cfg->rank_size / 1024 / 1024));
	printf("Full size:                                            %u MB\n",
	       (uint32_t)(cfg->full_size / 1024 / 1024));
	printf("Device Width:                                         %d bits\n",
	       cfg->device_width);

	printf("Number of Row Addresses:                              %d\n",
	       BIT(cfg->row_addr_bits));
	printf("Number of Column Addresses:                           %d\n",
	       BIT(cfg->col_addr_bits));
	printf("Number of Bank Addresses:                             %d\n",
	       BIT(cfg->bank_addr_bits));
	printf("Bank Group Addressing:                                %d\n",
	       BIT(cfg->bank_group_bits));

	printf("Clock Cycle Time (tCK):                               %d ps\n", cfg->tck);
	printf("CAS# Latency Time (tAA min):                          %d ps\n",
	       cfg->taa * cfg->tck);
	printf("Minimum Four Activate Windows Delay (tFAW):           %d ps\n",
	       cfg->tfaw * cfg->tck);
	printf("RAS to CAS Delay Time (tRCD min):                     %d ps\n",
	       cfg->trcd * cfg->tck);
	printf("Row Precharge Delay Time (tRP min):                   %d ps\n",
	       cfg->trp * cfg->tck);
	printf("Active to Precharge Delay Time (tRAS min):            %d ps\n",
	       cfg->trasmin * cfg->tck);
	printf("Minimum Active to Auto-Refresh Delay (tRC):           %d ps\n\n",
	       cfg->trc * cfg->tck);
#ifdef CONFIG_DRAM_TYPE_DDR4
	printf("Full SDRAM Width:                                     %d bits\n",
	       cfg->full_sdram_width);
	printf("Primary SDRAM Width:                                  %d bits\n",
	       cfg->primary_sdram_width);
	printf("ECC SDRAM Width:                                      %d bits\n",
	       cfg->ecc_sdram_width);
	printf("Minimum Clock Cycle Time (tCK min):                   %d ps\n", cfg->tckmin);
	printf("Maximum Clock Cycle Time (tCK max):                   %d ps\n", cfg->tckmax);
	printf("Normal Refresh Recovery Delay Time (tRFC1 min):       %d ps\n",
	       cfg->trfc1 * cfg->tck);
	printf("2x Mode Refresh Recovery Delay Time (tRFC2 min):      %d ps\n",
	       cfg->trfc2 * cfg->tck);
	printf("4x Mode Refresh Recovery Delay Time (tRFC4 min):      %d ps\n",
	       cfg->trfc4 * cfg->tck);
	printf("Short Row Active to Row Active Delay (tRRD_S min):    %d ps\n",
	       cfg->trrds * cfg->tck);
	printf("Long Row Active to Row Active Delay (tRRD_L min):     %d ps\n",
	       cfg->trrdl * cfg->tck);
	printf("Long CAS to CAS Delay Time (tCCD_L min):              %d ps\n",
	       cfg->tccdl * cfg->tck);
#endif
}
#endif

#ifndef CONFIG_SPD_EEPROM
static int _ddrcfg_get(int ctrl_id, struct ddr_cfg *cfg)
{
	uint64_t sdram_size = 1ULL * BIT(CONFIG_DRAM_COLUMN_ADDR_BITS) *
			      BIT(CONFIG_DRAM_ROW_ADDR_BITS) * BIT(CONFIG_DRAM_BANK_ADDR_BITS) *
			      CONFIG_DRAM_DEVICE_WIDTH;
#ifdef CONFIG_DRAM_TYPE_DDR4
	sdram_size *= BIT(CONFIG_DRAM_BANK_GROUP_ADDR_BITS);
#endif

	/* Common parameters for all SDRAM types */
	cfg->ranks = CONFIG_DRAM_RANKS;
	cfg->rank_size = sdram_size * CONFIG_DDRMC_DQ_BUS_WIDTH / CONFIG_DRAM_DEVICE_WIDTH / 8;
	cfg->full_size = cfg->ranks * cfg->rank_size;
	cfg->device_width = CONFIG_DRAM_DEVICE_WIDTH;

	cfg->row_addr_bits = CONFIG_DRAM_ROW_ADDR_BITS;
	cfg->col_addr_bits = CONFIG_DRAM_COLUMN_ADDR_BITS;
	cfg->bank_addr_bits = CONFIG_DRAM_BANK_ADDR_BITS;

	cfg->tck = CONFIG_DRAM_TCK;
	cfg->taa = CONFIG_DRAM_CAS_LATENCY;

	cfg->tfaw = ps2clk_jedec(CONFIG_DRAM_TIMING_TFAW, cfg->tck);
	cfg->trasmin = ps2clk_jedec(CONFIG_DRAM_TIMING_TRASMIN, cfg->tck);
	cfg->trp = ps2clk_jedec(CONFIG_DRAM_TIMING_TRP, cfg->tck);
	cfg->trrds = ps2clk_jedec(CONFIG_DRAM_TIMING_TRRD, cfg->tck);
	cfg->trcd = ps2clk_jedec(CONFIG_DRAM_TIMING_TRCD, cfg->tck);
	cfg->trc = ps2clk_jedec(CONFIG_DRAM_TIMING_TRP + CONFIG_DRAM_TIMING_TRASMIN, cfg->tck);

#if defined(CONFIG_DRAM_TYPE_DDR4) || defined(CONFIG_DRAM_TYPE_DDR3)
	uint32_t trfc1, trfc2, trfc4;

#ifdef CONFIG_DRAM_TYPE_DDR4
	cfg->bank_group_bits = CONFIG_DRAM_BANK_GROUP_ADDR_BITS;
	cfg->trrdl = ps2clk_jedec(CONFIG_DRAM_TIMING_TRRDL, cfg->tck);
	cfg->tccdl = ps2clk_jedec(CONFIG_DRAM_TIMING_TCCDL, cfg->tck);
#endif

	switch (sdram_size / 1024 / 1024) {
	case 512: /* used only for DDR3 */
		trfc1 = 90000;
		trfc2 = 0;
		trfc4 = 0;
		break;
	case 1024: /* used only for DDR3 */
		trfc1 = 110000;
		trfc2 = 0;
		trfc4 = 0;
		break;
	case 2048:
		trfc1 = 160000;
		trfc2 = 110000;
		trfc4 = 90000;
		break;
	case 4096:
		trfc1 = 260000;
		trfc2 = 160000;
		trfc4 = 110000;
		break;
	case 8192:
		trfc1 = 350000;
		trfc2 = 260000;
		trfc4 = 160000;
		break;
	case 16384:
		trfc1 = 550000;
		trfc2 = 350000;
		trfc4 = 260000;
		break;
	default:
		return -EDIMMCFG;
	}

	cfg->trfc1 = ps2clk_jedec(trfc1, cfg->tck);
	cfg->trfc2 = ps2clk_jedec(trfc2, cfg->tck);
	cfg->trfc4 = ps2clk_jedec(trfc4, cfg->tck);
#endif

	return 0;
}
#endif

int ddrcfg_get(int ctrl_id, struct ddr_cfg *cfg)
{
	int ret;

	if (!(BIT(ctrl_id) & CONFIG_DDRMC_ACTIVE_MASK))
		return -EDIMMCFG;

#ifdef CONFIG_SPD_EEPROM
#ifdef CONFIG_DRAM_TYPE_DDR4
	struct ddr4_spd spd;
#endif

	ret = spd_get(ctrl_id, &spd);
	if (ret)
		return ret;

	ret = spd_parse(&spd, cfg);
	if (ret)
		return ret;

#ifdef CONFIG_DEBUG_SPD_DUMP
	spd_dump((uint8_t *)&spd);
#endif

#else
	ret = _ddrcfg_get(ctrl_id, cfg);
#endif

#ifdef CONFIG_DEBUG_DDRCFG_PRINT
	ddrcfg_print(cfg);
#endif
	return ret;
}
