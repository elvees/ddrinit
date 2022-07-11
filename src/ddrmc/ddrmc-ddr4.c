// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2020 RnD Center "ELVEES", JSC

#include <common.h>
#include <ddrmc.h>
#include <regs.h>

/* DRAM timings from JESD79-4B standard that are common for all DDR4 devices. */
#define DRAM_TIMING_TWR_PS 15000
#define DRAM_TIMING_TREFI_PS 7800000
#define DRAM_TIMING_TRASMAX_PS (9 * DRAM_TIMING_TREFI_PS)
#define DRAM_TIMING_TRTP_PS 7500
#define DRAM_TIMING_TXP_PS 6000
#define DRAM_TIMING_TWTRL_PS 7500
#define DRAM_TIMING_TMOD_PS 15000
#define DRAM_TIMING_TCKE_PS 5000
#define DRAM_TIMING_TCKSRE_PS 10000
#define DRAM_TIMING_TCKSRX_PS 10000
#define DRAM_TIMING_TWTRS_PS 2500

#define DRAM_TIMING_TZQINIT 1024
#define DRAM_TIMING_TZQCS 128
#define DRAM_TIMING_TZQOPER 512
#define DRAM_TIMING_TMRD 8
#define DRAM_TIMING_TDLLK 1024
#define DRAM_TIMING_TCCDS 4

#define DRAM_TIMING_TRASMAX(tck) (ps2clk_jedec(DRAM_TIMING_TRASMAX_PS, (tck)))
#define DRAM_TIMING_TWR(tck) (ps2clk_jedec(DRAM_TIMING_TWR_PS, (tck)))
#define DRAM_TIMING_TRTP(tck) (max(ps2clk_jedec(DRAM_TIMING_TRTP_PS, (tck)), 4))
#define DRAM_TIMING_TXP(tck) (max(ps2clk_jedec(DRAM_TIMING_TXP_PS, (tck)), 4))
#define DRAM_TIMING_TWTRL(tck) (max(ps2clk_jedec(DRAM_TIMING_TWTRL_PS, (tck)), 4))
#define DRAM_TIMING_TMOD(tck) (max(ps2clk_jedec(DRAM_TIMING_TMOD_PS, (tck)), 24))
#define DRAM_TIMING_TCKE(tck) (max(ps2clk_jedec(DRAM_TIMING_TCKE_PS, (tck)), 3))
#define DRAM_TIMING_TCKSRE(tck) (max(ps2clk_jedec(DRAM_TIMING_TCKSRE_PS, (tck)), 5))
#define DRAM_TIMING_TCKSRX(tck) (max(ps2clk_jedec(DRAM_TIMING_TCKSRX_PS, (tck)), 5))
#define DRAM_TIMING_TWTRS(tck) (max(ps2clk_jedec(DRAM_TIMING_TWTRS_PS, (tck)), 2))
#define DRAM_TIMING_TXS(trfc1, tck) (ps2clk_jedec(10000, (tck)) + (trfc1))
#define DRAM_TIMING_TXS_ABORT(trfc4, tck) (ps2clk_jedec(10000, (tck)) + (trfc4))
#define DRAM_TIMING_TXS_FAST(trfc4, tck) (ps2clk_jedec(10000, (tck)) + (trfc4))
#define DRAM_TIMING_TREFI(tck) (DIV_ROUND_UP(DRAM_TIMING_TREFI_PS, (tck)))

static uint8_t ddr4_cwl_get(uint32_t tck)
{
	if (tck >= DRAM_TCK_1333)
		return 9;
	else if (tck < DRAM_TCK_1333 && tck >= DRAM_TCK_1600)
		return 11;
	else if (tck < DRAM_TCK_1600 && tck >= DRAM_TCK_1866)
		return 12;
	else if (tck < DRAM_TCK_1866 && tck >= DRAM_TCK_2133)
		return 14;
	else if (tck < DRAM_TCK_2133 && tck >= DRAM_TCK_2400)
		return 16;
	else if (tck < DRAM_TCK_2400 && tck >= DRAM_TCK_2666)
		return 18;
	else
		return 20;
}

uint16_t ddr4_mr0_get(struct ddr_cfg *cfg)
{
	uint32_t twr, trtp, tmp = 0;
	uint16_t mr0 = 0;

	twr = ps2clk_jedec(DRAM_TIMING_TWR_PS, cfg->tck);
	trtp = twr / 2;

	if (cfg->taa >= 9 && cfg->taa <= 16)
		tmp = cfg->taa - 9;
	else if (cfg->taa == 17 || cfg->taa == 19 || cfg->taa == 21)
		tmp = cfg->taa / 2 + 5;
	else if (cfg->taa == 18 || cfg->taa == 20 || cfg->taa == 22)
		tmp = cfg->taa / 2 - 1;

	if (tmp & 1)
		mr0 = BIT(2);
	if (tmp & 2)
		mr0 |= BIT(4);
	if (tmp & 4)
		mr0 |= BIT(5);
	if (tmp & 8)
		mr0 |= BIT(6);
	if (tmp & 16)
		mr0 |= BIT(12);

	tmp = trtp - 5;

	if (tmp & 1)
		mr0 |= BIT(9);
	if (tmp & 2)
		mr0 |= BIT(10);
	if (tmp & 4)
		mr0 |= BIT(11);
	if (tmp & 8)
		mr0 |= BIT(13);

	return mr0;
}

uint16_t ddr4_mr1_get(struct ddr_cfg *cfg)
{
	uint16_t mr1 = 0;

#ifdef CONFIG_DRAM_TX_IMPEDANCE_48
	mr1 = BIT(1);
#endif

#ifdef CONFIG_DRAM_RTT_NOM_34
	mr1 |= BIT(10) | BIT(9) | BIT(8);
#elif CONFIG_DRAM_RTT_NOM_40
	mr1 |= BIT(9) | BIT(8);
#elif CONFIG_DRAM_RTT_NOM_48
	mr1 |= BIT(10) | BIT(8);
#elif CONFIG_DRAM_RTT_NOM_60
	mr1 |= BIT(8);
#elif CONFIG_DRAM_RTT_NOM_80
	mr1 |= BIT(10) | BIT(9);
#elif CONFIG_DRAM_RTT_NOM_120
	mr1 |= BIT(9);
#elif CONFIG_DRAM_RTT_NOM_240
	mr1 |= BIT(10);
#endif
	return mr1 | 1;
}

uint16_t ddr4_mr2_get(struct ddr_cfg *cfg)
{
	uint16_t mr2 = 0;
	uint8_t cwl = ddr4_cwl_get(cfg->tck);

	if ((cwl == 9) || (cwl == 11) || (cwl == 12))
		mr2 = cwl - 9;
	else
		mr2 = (cwl / 2) - 3;

	mr2 <<= 3;

#ifdef CONFIG_DRAM_RTT_WR_80
	mr2 |= BIT(11);
#elif CONFIG_DRAM_RTT_WR_120
	mr2 |= BIT(9);
#elif CONFIG_DRAM_RTT_WR_240
	mr2 |= BIT(10);
#elif CONFIG_DRAM_RTT_WR_HIZ
	mr2 |= BIT(10) | BIT(9);
#endif
	return mr2;
}

uint16_t ddr4_mr3_get(struct ddr_cfg *cfg)
{
	uint16_t mr3 = 0;

	if (cfg->tck < DRAM_TCK_1600 && cfg->tck >= DRAM_TCK_2666)
		mr3 = BIT(9);
	else if (cfg->tck < DRAM_TCK_2666 && cfg->tck >= DRAM_TCK_3200)
		mr3 = BIT(10);

	return mr3;
}

uint16_t ddr4_mr4_get(struct ddr_cfg *cfg)
{
	return 0;
}

uint16_t ddr4_mr5_get(struct ddr_cfg *cfg)
{
	uint16_t mr5 = 0;

#ifdef CONFIG_DRAM_RTT_PARK_34
	mr5 |= BIT(8) | BIT(7) | BIT(6);
#elif CONFIG_DRAM_RTT_PARK_40
	mr5 |= BIT(7) | BIT(6);
#elif CONFIG_DRAM_RTT_PARK_48
	mr5 |= BIT(8) | BIT(6);
#elif CONFIG_DRAM_RTT_PARK_60
	mr5 |= BIT(6);
#elif CONFIG_DRAM_RTT_PARK_80
	mr5 |= BIT(8) | BIT(7);
#elif CONFIG_DRAM_RTT_PARK_120
	mr5 |= BIT(7);
#elif CONFIG_DRAM_RTT_PARK_240
	mr5 |= BIT(8);
#endif
	return mr5;
}

static uint8_t ddr4_ccdl_get(struct ddr_cfg *cfg)
{
	if (cfg->tck >= DRAM_TCK_1333)
		return 4;
	else if (cfg->tck < DRAM_TCK_1333 && cfg->tck >= DRAM_TCK_1866)
		return 5;
	else if (cfg->tck < DRAM_TCK_1866 && cfg->tck >= DRAM_TCK_2400)
		return 6;
	else
		return ps2clk_jedec(cfg->tccdl, cfg->tckmin);
}

uint16_t ddr4_mr6_get(struct ddr_cfg *cfg)
{
	uint16_t mr6 = 0;
	uint8_t ccdl = ddr4_ccdl_get(cfg);

	switch (ccdl) {
	case 5:
		mr6 = BIT(10);
		break;
	case 6:
		mr6 = BIT(11);
		break;
	case 7:
		mr6 = BIT(10) | BIT(11);
		break;
	case 8:
		mr6 = BIT(12);
		break;
	}

	return mr6;
}

static void dfi_timings_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t val;
	uint8_t cl = cfg->taa;
	uint8_t cwl = ddr4_cwl_get(cfg->tck);
	uint8_t ctrl_delay = DIV_ROUND_UP((cfg->tck <= DRAM_TCK_1866) ? 2 : 1, 2) + 3;

	/* TODO: Some DFI timings should be moved to Kconfig */
	val = FIELD_PREP(DDRMC_DFITMG0_WRLAT, cwl - 5) | FIELD_PREP(DDRMC_DFITMG0_WRDATA, 2) |
	      FIELD_PREP(DDRMC_DFITMG0_WRDATA_DFICLK, 1) |
	      FIELD_PREP(DDRMC_DFITMG0_RDDATAEN, cl - 5) |
	      FIELD_PREP(DDRMC_DFITMG0_RDDATA_DFI_CLK, 1) |
	      FIELD_PREP(DDRMC_DFITMG0_CTRLDELAY, ctrl_delay);
	write32(DDRMC_DFITMG0(ctrl_id), val);

	/* TODO: Verification of DFITMG1_WRDATA_DELAY is needed */
	val = FIELD_PREP(DDRMC_DFITMG1_DRAM_CLK_EN, ctrl_delay) |
	      FIELD_PREP(DDRMC_DFITMG1_DRAM_CLK_DIS, ctrl_delay) |
	      FIELD_PREP(DDRMC_DFITMG1_WRDATA_DELAY, 6 + CONFIG_BURST_LEN / 2);
	write32(DDRMC_DFITMG1(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DFITMG2_WRCSLAT, cwl - 5) |
	      FIELD_PREP(DDRMC_DFITMG2_RDCSLAT, cl - 5);
	write32(DDRMC_DFITMG2(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DFIUPD0_CTRLUPD_MIN, 24) | FIELD_PREP(DDRMC_DFIUPD0_CTRLUPD_MAX, 48);
	write32(DDRMC_DFIUPD0(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DFIUPD1_CTRLUPD_INTMIN, 64) |
	      FIELD_PREP(DDRMC_DFIUPD1_CTRLUPD_INTMAX, 192);
	write32(DDRMC_DFIUPD1(ctrl_id), val);
}

static void dram_timings_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t val;
	uint16_t tck = cfg->tck;
	uint8_t cl = cfg->taa;
	uint8_t cwl = ddr4_cwl_get(tck);
	uint8_t hbl = CONFIG_BURST_LEN / 2;

	val = FIELD_PREP(DDRMC_DRAMTMG0_TRASMIN, cfg->trasmin / 2) |
	      FIELD_PREP(DDRMC_DRAMTMG0_TRASMAX, (DRAM_TIMING_TRASMAX(tck) - 1) / 2048) |
	      FIELD_PREP(DDRMC_DRAMTMG0_TFAW, DIV_ROUND_UP(cfg->tfaw, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG0_WR2PRE, (cwl + hbl + DRAM_TIMING_TWR(tck)) / 2);
	write32(DDRMC_DRAMTMG0(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG1_TRC, DIV_ROUND_UP(cfg->trc, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG1_RD2PRE,
			 max(DRAM_TIMING_TRTP(tck), cl + hbl - cfg->trp) / 2) |
	      FIELD_PREP(DDRMC_DRAMTMG1_TXP, DIV_ROUND_UP(DRAM_TIMING_TXP(tck), 2));
	write32(DDRMC_DRAMTMG1(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG2_WR2RD,
			 DIV_ROUND_UP(cwl + hbl + 2 + DRAM_TIMING_TWTRL(tck), 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG2_RD2WR, DIV_ROUND_UP(cl + 2 + hbl + 1 + 1 - cwl, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG2_RDLAT, DIV_ROUND_UP(cl, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG2_WRLAT, DIV_ROUND_UP(cwl, 2));
	write32(DDRMC_DRAMTMG2(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG3_TMOD, DIV_ROUND_UP(DRAM_TIMING_TMOD(tck), 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG3_TMRD, DIV_ROUND_UP(DRAM_TIMING_TMRD, 2));
	write32(DDRMC_DRAMTMG3(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG4_TRP, cfg->trp / 2 + 1) |
	      FIELD_PREP(DDRMC_DRAMTMG4_TRRD, DIV_ROUND_UP(cfg->trrdl, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG4_TCCD, DIV_ROUND_UP(ddr4_ccdl_get(cfg), 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG4_TRCD, DIV_ROUND_UP(cfg->trcd, 2));
	write32(DDRMC_DRAMTMG4(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG5_TCKE, DIV_ROUND_UP(DRAM_TIMING_TCKE(tck), 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG5_TCKESR, DIV_ROUND_UP(DRAM_TIMING_TCKE(tck) + 1, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG5_TCKSRE, DIV_ROUND_UP(DRAM_TIMING_TCKSRE(tck), 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG5_TCKSRX, DIV_ROUND_UP(DRAM_TIMING_TCKSRX(tck), 2));
	write32(DDRMC_DRAMTMG5(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG8_TXS, DIV_ROUND_UP(DRAM_TIMING_TXS(cfg->trfc1, tck), 64)) |
	      FIELD_PREP(DDRMC_DRAMTMG8_TXSDLL, DIV_ROUND_UP(DRAM_TIMING_TDLLK, 64)) |
	      FIELD_PREP(DDRMC_DRAMTMG8_TXSABORT,
			 DIV_ROUND_UP(DRAM_TIMING_TXS_ABORT(cfg->trfc4, tck), 64)) |
	      FIELD_PREP(DDRMC_DRAMTMG8_TXSFAST,
			 DIV_ROUND_UP(DRAM_TIMING_TXS_FAST(cfg->trfc4, tck), 64));
	write32(DDRMC_DRAMTMG8(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG9_WR2RDS,
			 DIV_ROUND_UP(cwl + hbl + DRAM_TIMING_TWTRS(tck), 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG9_TRRDS, DIV_ROUND_UP(cfg->trrds, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG9_TCCDS, DIV_ROUND_UP(DRAM_TIMING_TCCDS, 2));
	write32(DDRMC_DRAMTMG9(ctrl_id), val);

	/* TODO: Set DRAMTMG10 - DRAMTMG19 registers */
}

static void addrmap_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint8_t next_bit;
	uint32_t val, tmp1, tmp2;

	write32(DDRMC_ADDRMAP2(ctrl_id), 0);
	write32(DDRMC_ADDRMAP3(ctrl_id), 0);

	if (cfg->col_addr_bits <= 10) {
		tmp1 = 31;
		tmp2 = 31;
	} else {
		if (cfg->col_addr_bits == 11) {
			tmp1 = 0;
			tmp2 = 31;
		} else {
			tmp1 = 0;
			tmp2 = 0;
		}
	}
	val = FIELD_PREP(DDRMC_ADDRMAP4_COL_B10, tmp1) | FIELD_PREP(DDRMC_ADDRMAP4_COL_B11, tmp2);
	write32(DDRMC_ADDRMAP4(ctrl_id), val);
	next_bit = cfg->col_addr_bits;

	/* TODO: Add interleave mode support */

	val = FIELD_PREP(DDRMC_ADDRMAP1_BANK_B0, next_bit - 2) |
	      FIELD_PREP(DDRMC_ADDRMAP1_BANK_B1, next_bit - 2) |
	      FIELD_PREP(DDRMC_ADDRMAP1_BANK_B2, (cfg->bank_addr_bits == 2) ? 63 : next_bit - 2);
	write32(DDRMC_ADDRMAP1(ctrl_id), val);
	next_bit += cfg->bank_addr_bits;

	val = FIELD_PREP(DDRMC_ADDRMAP8_BG_B0, (cfg->bank_group_bits == 0) ? 63 : next_bit - 2) |
	      FIELD_PREP(DDRMC_ADDRMAP8_BG_B1, (cfg->bank_addr_bits == 2) ? next_bit - 2 : 63);
	write32(DDRMC_ADDRMAP8(ctrl_id), val);
	next_bit += cfg->bank_group_bits;

	val = FIELD_PREP(DDRMC_ADDRMAP5_ROW_B0, next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP5_ROW_B1, next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP5_ROW_B2_B10, next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP5_ROW_B11, next_bit - 6);
	write32(DDRMC_ADDRMAP5(ctrl_id), val);

	val = FIELD_PREP(DDRMC_ADDRMAP6_ROW_B12, (cfg->row_addr_bits <= 12) ? 15 : next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP6_ROW_B13, (cfg->row_addr_bits <= 13) ? 15 : next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP6_ROW_B14, (cfg->row_addr_bits <= 14) ? 15 : next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP6_ROW_B15, (cfg->row_addr_bits <= 15) ? 15 : next_bit - 6);
	write32(DDRMC_ADDRMAP6(ctrl_id), val);

	val = FIELD_PREP(DDRMC_ADDRMAP7_ROW_B16, (cfg->row_addr_bits <= 16) ? 15 : next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP7_ROW_B17, (cfg->row_addr_bits <= 17) ? 15 : next_bit - 6);
	write32(DDRMC_ADDRMAP7(ctrl_id), val);
	next_bit += cfg->row_addr_bits;

	val = FIELD_PREP(DDRMC_ADDRMAP0_CS_B0, (cfg->ranks == 1) ? 31 : next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP0_CS_B1, (cfg->ranks <= 2) ? 31 : next_bit - 6);
	write32(DDRMC_ADDRMAP0(ctrl_id), val);

	/* TODO: Add 3DS support */
}

static void dimm_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	/* TODO: */
}

static void ecc_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	/* TODO: */
}

void ddrmc_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t val, tmp;
	uint16_t tck = cfg->tck;

	val = FIELD_PREP(DDRMC_MSTR_DDR4, 1) |
	      FIELD_PREP(DDRMC_MSTR_BURST_RDWR, CONFIG_BURST_LEN / 2) |
	      FIELD_PREP(DDRMC_MSTR_LOG_RANKS, 0) |
	      FIELD_PREP(DDRMC_MSTR_RANKS, (1 << cfg->ranks) - 1) |
	      FIELD_PREP(DDRMC_MSTR_DEVCONFIG, __builtin_ffsll(cfg->device_width) - 3);
	write32(DDRMC_MSTR(ctrl_id), val);

	tmp = cfg->trfc1 + ps2clk_jedec(10000, tck);
	val = FIELD_PREP(DDRMC_INIT0_PRECKE, DIV_ROUND_UP(500000000, tck * 2048)) |
	      FIELD_PREP(DDRMC_INIT0_POSTCKE, DIV_ROUND_UP(tmp, 2048)) |
	      FIELD_PREP(DDRMC_INIT0_SKIP_DRAM_INIT, 3);
	write32(DDRMC_INIT0(ctrl_id), val);

	write32(DDRMC_INIT1(ctrl_id), FIELD_PREP(DDRMC_INIT1_DRAM_RSTN,
						 DIV_ROUND_UP(200000000, tck * 2048)));

	val = FIELD_PREP(DDRMC_INIT3_MR, ddr4_mr0_get(cfg)) |
	      FIELD_PREP(DDRMC_INIT3_EMR, ddr4_mr1_get(cfg));
	write32(DDRMC_INIT3(ctrl_id), val);

	val = FIELD_PREP(DDRMC_INIT4_EMR2, ddr4_mr2_get(cfg)) |
	      FIELD_PREP(DDRMC_INIT4_EMR3, ddr4_mr3_get(cfg));
	write32(DDRMC_INIT4(ctrl_id), val);

	val = FIELD_PREP(DDRMC_INIT5_ZQINIT, DIV_ROUND_UP(DRAM_TIMING_TZQINIT, 64));
	write32(DDRMC_INIT5(ctrl_id), val);

	val = FIELD_PREP(DDRMC_INIT6_MR4, ddr4_mr4_get(cfg)) |
	      FIELD_PREP(DDRMC_INIT6_MR5, ddr4_mr5_get(cfg));
	write32(DDRMC_INIT6(ctrl_id), val);

	val = FIELD_PREP(DDRMC_INIT7_MR6, ddr4_mr6_get(cfg));
	write32(DDRMC_INIT7(ctrl_id), val);

	val = FIELD_PREP(DDRMC_RFSHTMG_TRFC, DIV_ROUND_UP(cfg->trfc1, 2)) |
	      FIELD_PREP(DDRMC_RFSHTMG_TREFI, DRAM_TIMING_TREFI(tck) / 64);
	write32(DDRMC_RFSHTMG(ctrl_id), val);

	val = FIELD_PREP(DDRMC_ZQCTL0_TZQCS, DIV_ROUND_UP(DRAM_TIMING_TZQCS, 2)) |
	      FIELD_PREP(DDRMC_ZQCTL0_TZQOPER, DIV_ROUND_UP(DRAM_TIMING_TZQOPER, 2));
	write32(DDRMC_ZQCTL0(ctrl_id), val);

	write32(DDRMC_DBICTL(ctrl_id), 0);

	val = FIELD_PREP(DDRMC_ODTCFG_RD_DELAY, cfg->taa - ddr4_cwl_get(tck)) |
	      FIELD_PREP(DDRMC_ODTCFG_RD_HOLD, 6) | FIELD_PREP(DDRMC_ODTCFG_WR_HOLD, 6);
	write32(DDRMC_ODTCFG(ctrl_id), val);

	dram_timings_cfg(ctrl_id, cfg);
	dfi_timings_cfg(ctrl_id, cfg);
	addrmap_cfg(ctrl_id, cfg);
	dimm_cfg(ctrl_id, cfg);
	ecc_cfg(ctrl_id, cfg);
}
