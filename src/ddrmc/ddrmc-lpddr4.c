// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 RnD Center "ELVEES", JSC
 */

#include <common.h>
#include <ddrmc.h>
#include <regs.h>

#define LPDDR4_MR1_BL GENMASK(1, 0)
#define LPDDR4_MR1_WR_PRE BIT(2)
#define LPDDR4_MR1_RD_PRE BIT(3)
#define LPDDR4_MR1_NWR GENMASK(6, 4)
#define LPDDR4_MR1_RPST BIT(7)

#define LPDDR4_MR2_RL GENMASK(2, 0)
#define LPDDR4_MR2_WL GENMASK(5, 3)
#define LPDDR4_MR2_WLS BIT(6)
#define LPDDR4_MR2_WRLEV BIT(7)

#define LPDDR4_MR11_DQ_ODT GENMASK(2, 0)
#define LPDDR4_MR11_CA_ODT GENMASK(6, 4)

static int cwl_get(int tck)
{
	if (tck >= DRAM_TCK_533)
		return 4;
	else if (tck < DRAM_TCK_533 && tck >= DRAM_TCK_1066)
		return 6;
	else if (tck < DRAM_TCK_1066 && tck >= DRAM_TCK_1600)
		return 8;
	else if (tck < DRAM_TCK_1600 && tck >= DRAM_TCK_2133)
		return 10;
	else if (tck < DRAM_TCK_2133 && tck >= DRAM_TCK_2666)
		return 12;
	else if (tck < DRAM_TCK_2666 && tck >= DRAM_TCK_3200)
		return 14;
	else
		return 0;
}

static int odtlon_get(int tck)
{
	if (tck < DRAM_TCK_1600 && tck >= DRAM_TCK_2666)
		return 4;
	else if (tck < DRAM_TCK_2666 && tck >= DRAM_TCK_3200)
		return 6;
	else
		return 0;
}

static int odtloff_get(int tck)
{
	int ret;

	if (tck < DRAM_TCK_1600 && tck >= DRAM_TCK_2133)
		ret = 20;
	else if (tck < DRAM_TCK_2133 && tck >= DRAM_TCK_2666)
		ret = 22;
	else if (tck < DRAM_TCK_2666 && tck >= DRAM_TCK_3200)
		ret = 24;
	else
		return 0;

	return (CONFIG_BURST_LEN == 16) ? ret : ret + 8;
}

static int twr_get(int tck)
{
	int twr = ps2clk_jedec(CONFIG_DRAM_TIMING_TWR, tck);
	return max(twr, 6);
}

static int trtp_get(int tck)
{
	int trtp = ps2clk_jedec(CONFIG_DRAM_TIMING_TRTP, tck);
	return max(trtp, 8);
}

static int txp_get(struct ddr_cfg *cfg)
{
	int txp = ps2clk_jedec(CONFIG_DRAM_TIMING_TXP, cfg->tck);
	return max(txp, 5);
}

static int trfc_ab_get(struct ddr_cfg *cfg)
{
	/* TODO: Set tRFC according to density per channel */
	return ps2clk_jedec(CONFIG_DRAM_TIMING_TRFC_AB, cfg->tck);
}

static int tzqoper_get(struct ddr_cfg *cfg)
{
	return ps2clk_jedec(CONFIG_DRAM_TIMING_TZQOPER, cfg->tck);
}

static int tzqcs_get(struct ddr_cfg *cfg)
{
	int tzqcs = ps2clk_jedec(CONFIG_DRAM_TIMING_TZQCS, cfg->tck);
	tzqcs = max(tzqcs, 8);

	return tzqcs;
}

static int dramtmg0_trasmin_get(struct ddr_cfg *cfg)
{
	return DIV_ROUND_UP(cfg->trasmin, 2);
}

static int dramtmg0_trasmax_get(struct ddr_cfg *cfg)
{
	int trasmax = CONFIG_DRAM_TIMING_TRASMAX / cfg->tck / 1024;
	return (trasmax - 1) / 2;
}

static int dramtmg0_tfaw_get(struct ddr_cfg *cfg)
{
	if ((cfg->bank_addr_bits > 2) || (cfg->bank_addr_bits * cfg->bank_group_bits > 2))
		return DIV_ROUND_UP(cfg->tfaw, 2);
	else
		return 1;
}

static int dramtmg0_wr2pre_get(struct ddr_cfg *cfg)
{
	int ret = cwl_get(cfg->tck) + CONFIG_BURST_LEN / 2 + twr_get(cfg->tck);
	return DIV_ROUND_UP(ret + 1, 2);
}

static int dramtmg1_rd2pre_get(struct ddr_cfg *cfg)
{
	int ret = CONFIG_BURST_LEN / 2 + trtp_get(cfg->tck) - 8;
	return DIV_ROUND_UP(ret, 2);
}

static int dramtmg2_wr2rd_get(struct ddr_cfg *cfg)
{
	int ret = cwl_get(cfg->tck) + CONFIG_BURST_LEN / 2;
	int twtr = ps2clk_jedec(CONFIG_DRAM_TIMING_TWTR, cfg->tck);

	twtr = max(twtr, 8);
	ret += twtr + 2;

	return DIV_ROUND_UP(ret, 2);
}

static int dramtmg2_rd2wr_get(struct ddr_cfg *cfg)
{
	int ret = cfg->taa + CONFIG_BURST_LEN / 2;

#ifdef CONFIG_READ_POSTAMBLE_0_5T
	int postamble_len = 1;
#else
	int postamble_len = 2;
#endif
	ret += DIV_ROUND_UP(CONFIG_DRAM_TIMING_TDQSCKMAX, cfg->tck) + postamble_len;

#ifdef CONFIG_RTT_DQ_DISABLED
	ret += CONFIG_WRITE_PREAMBLE_LEN - cwl_get(cfg->tck);
#else
	ret += 1 - odtlon_get(cfg->tck) - DIV_ROUND_UP(CONFIG_DRAM_TIMING_TODTON_MIN, cfg->tck);
#endif
	return DIV_ROUND_UP(ret, 2);
}

static int dramtmg3_tmod_get(struct ddr_cfg *cfg)
{
	return 0;
}

static int dramtmg3_tmrd_get(struct ddr_cfg *cfg)
{
	int tmrd = 0;

	tmrd = ps2clk_jedec(CONFIG_DRAM_TIMING_TMRD, cfg->tck);
	tmrd = max(tmrd, 10);
	return DIV_ROUND_UP(tmrd, 2);
}

static int dramtmg3_tmrw_get(struct ddr_cfg *cfg)
{
	int tmrw = 0;

	tmrw = ps2clk_jedec(CONFIG_DRAM_TIMING_TMRW, cfg->tck);
	tmrw = max(tmrw, 10);
	return DIV_ROUND_UP(tmrw, 2);
}

static int dramtmg4_trcd_get(struct ddr_cfg *cfg)
{
	int trcd;

	trcd = ps2clk_jedec(CONFIG_DRAM_TIMING_TRCD, cfg->tck);
	trcd = max(trcd, 4);
	return DIV_ROUND_UP(trcd, 2);
}

static int dramtmg4_trrd_get(struct ddr_cfg *cfg)
{
	int trrd;

	trrd = ps2clk_jedec(CONFIG_DRAM_TIMING_TRRD, cfg->tck);
	trrd = max(trrd, 4);
	return DIV_ROUND_UP(trrd, 2);
}

static int dramtmg4_tccd_get(struct ddr_cfg *cfg)
{
	return DIV_ROUND_UP(8, 2);
}

static int dramtmg4_trp_get(struct ddr_cfg *cfg)
{
	int trp = DIV_ROUND_UP(CONFIG_DRAM_TIMING_TRP, cfg->tck);

	trp = max(trp, 4);
	return DIV_ROUND_UP(trp, 2);
}

static int dramtmg5_tcke_get(struct ddr_cfg *cfg)
{
	int tcke = ps2clk_jedec(CONFIG_DRAM_TIMING_TCKE, cfg->tck);
	int tsr = ps2clk_jedec(CONFIG_DRAM_TIMING_TSR, cfg->tck);

	tsr = max(tsr, 3);
	tcke = max(tcke, 4);
	tcke = max(tcke, tsr);
	return DIV_ROUND_UP(tcke, 2);
}

static int dramtmg5_tckesr_get(struct ddr_cfg *cfg)
{
	int tckesr = dramtmg5_tcke_get(cfg);
	return tckesr;
}

static int dramtmg5_tcksre_get(struct ddr_cfg *cfg)
{
	int tcksre;

	tcksre = ps2clk_jedec(CONFIG_DRAM_TIMING_TCKELCK, cfg->tck);
	tcksre = max(tcksre, 5);
	return DIV_ROUND_UP(tcksre, 2);
}

static int dramtmg5_tcksrx_get(struct ddr_cfg *cfg)
{
	int tcksrx;

	tcksrx = ps2clk_jedec(CONFIG_DRAM_TIMING_TCKCKEH, cfg->tck);
	tcksrx = max(tcksrx, 3);
	return DIV_ROUND_UP(tcksrx, 2);
}

static int dramtmg7_tckpdx_get(struct ddr_cfg *cfg)
{
	int tckpdx = 2;

	tckpdx = ps2clk_jedec(CONFIG_DRAM_TIMING_TCKCKEH, cfg->tck);
	tckpdx = max(tckpdx, 3);
	return DIV_ROUND_UP(tckpdx, 2);
}

static int dramtmg7_tckpde_get(struct ddr_cfg *cfg)
{
	int tckpde = 2;

	tckpde = ps2clk_jedec(CONFIG_DRAM_TIMING_TCKELCK, cfg->tck);
	tckpde = max(tckpde, 5);
	return DIV_ROUND_UP(tckpde, 2);
}

static int dramtmg14_txsr_get(struct ddr_cfg *cfg)
{
	int txsr = max(trfc_ab_get(cfg) + 7500, 2);

	return DIV_ROUND_UP(txsr, 2);
}

uint8_t lpddr4_mr1_get(struct ddr_cfg *cfg)
{
	uint8_t mr1;
	uint8_t nwr = 0;
	uint8_t bl = (CONFIG_BURST_LEN == 16) ? 0 : 1;
	uint8_t rpst;

#ifdef CONFIG_READ_POSTAMBLE_0_5T
	rpst = 0;
#else
	rpst = 1;
#endif

	switch (twr_get(cfg->tck)) {
	case 6:
		nwr = 0;
		break;
	case 10:
		nwr = 1;
		break;
	case 16:
		nwr = 2;
		break;
	case 20:
		nwr = 3;
		break;
	case 24:
		nwr = 4;
		break;
	case 30:
		nwr = 5;
		break;
	case 34:
		nwr = 6;
		break;
	case 40:
		nwr = 7;
		break;
	}

	mr1 = FIELD_PREP(LPDDR4_MR1_BL, bl) |
	      FIELD_PREP(LPDDR4_MR1_WR_PRE, 1) |
	      FIELD_PREP(LPDDR4_MR1_RD_PRE, 0) |
	      FIELD_PREP(LPDDR4_MR1_NWR, nwr) |
	      FIELD_PREP(LPDDR4_MR1_RPST, rpst);

	return mr1;
}

uint8_t lpddr4_mr2_get(struct ddr_cfg *cfg)
{
	uint8_t mr2, rl = 0;

	switch (cfg->taa) {
	case 6:
		rl = 0;
		break;
	case 10:
		rl = 1;
		break;
	case 14:
		rl = 2;
		break;
	case 20:
		rl = 3;
		break;
	case 24:
		rl = 4;
		break;
	case 28:
		rl = 5;
		break;
	case 32:
		rl = 6;
		break;
	case 36:
		rl = 7;
		break;
	}

	mr2 = FIELD_PREP(LPDDR4_MR2_RL, rl) |
	      FIELD_PREP(LPDDR4_MR2_WL, cwl_get(cfg->tck) / 2 - 2) |
	      FIELD_PREP(LPDDR4_MR2_WLS, 0) |
	      FIELD_PREP(LPDDR4_MR2_WRLEV, 0);

	return mr2;
}

uint8_t lpddr4_mr11_get(struct ddr_cfg *cfg)
{
	uint8_t mr11, dq_odt = 0, ca_odt = 0;

#ifdef CONFIG_DRAM_RTT_NOM_34
	dq_odt = 6;
#elif CONFIG_DRAM_RTT_NOM_40
	dq_odt = 6;
#elif CONFIG_DRAM_RTT_NOM_48
	dq_odt = 5;
#elif CONFIG_DRAM_RTT_NOM_60
	dq_odt = 4;
#elif CONFIG_DRAM_RTT_NOM_80
	dq_odt = 3;
#elif CONFIG_DRAM_RTT_NOM_120
	dq_odt = 2;
#elif CONFIG_DRAM_RTT_NOM_240
	dq_odt = 1;
#endif

#ifdef CONFIG_DRAM_CA_ODT_40
	ca_odt = 6;
#elif CONFIG_DRAM_CA_ODT_48
	ca_odt = 5;
#elif CONFIG_DRAM_CA_ODT_60
	ca_odt = 4;
#elif CONFIG_DRAM_CA_ODT_80
	ca_odt = 3;
#elif CONFIG_DRAM_CA_ODT_120
	ca_odt = 2;
#elif CONFIG_DRAM_CA_ODT_240
	ca_odt = 1;
#endif

	mr11 = FIELD_PREP(LPDDR4_MR11_DQ_ODT, dq_odt) |
	       FIELD_PREP(LPDDR4_MR11_CA_ODT, ca_odt);

	return mr11;
}

uint8_t lpddr4_mr22_get(struct ddr_cfg *cfg)
{
	uint8_t mr22 = 0;

#ifdef CONFIG_SOC_ODT_40
	mr22 = 6;
#elif CONFIG_SOC_ODT_48
	mr22 = 5;
#elif CONFIG_SOC_ODT_60
	mr22 = 4;
#elif CONFIG_SOC_ODT_80
	mr22 = 3;
#elif CONFIG_SOC_ODT_120
	mr22 = 2;
#elif CONFIG_SOC_ODT_240
	mr22 = 1;
#endif
	return mr22;
}

void dram_timings_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t val;

	val = FIELD_PREP(DDRMC_DRAMTMG0_TRASMIN, dramtmg0_trasmin_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG0_TRASMAX, dramtmg0_trasmax_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG0_TFAW, dramtmg0_tfaw_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG0_WR2PRE, dramtmg0_wr2pre_get(cfg));
	write32_with_dbg(DDRMC_DRAMTMG0(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG1_TRC, DIV_ROUND_UP(cfg->trc, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG1_RD2PRE, dramtmg1_rd2pre_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG1_TXP, DIV_ROUND_UP(txp_get(cfg), 2));
	write32_with_dbg(DDRMC_DRAMTMG1(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG2_WR2RD, dramtmg2_wr2rd_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG2_RD2WR, dramtmg2_rd2wr_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG2_RDLAT, DIV_ROUND_UP(cfg->taa, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG2_WRLAT, DIV_ROUND_UP(cwl_get(cfg->tck), 2));
	write32_with_dbg(DDRMC_DRAMTMG2(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG3_TMOD, dramtmg3_tmod_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG3_TMRD, dramtmg3_tmrd_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG3_TMRW, dramtmg3_tmrw_get(cfg));
	write32_with_dbg(DDRMC_DRAMTMG3(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG4_TRP, dramtmg4_trp_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG4_TRRD, dramtmg4_trrd_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG4_TCCD, dramtmg4_tccd_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG4_TRCD, dramtmg4_trcd_get(cfg));
	write32_with_dbg(DDRMC_DRAMTMG4(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG5_TCKE, dramtmg5_tcke_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG5_TCKESR, dramtmg5_tckesr_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG5_TCKSRE, dramtmg5_tcksre_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG5_TCKSRX, dramtmg5_tcksrx_get(cfg));
	write32_with_dbg(DDRMC_DRAMTMG5(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG6_TCKCSX, DIV_ROUND_UP(txp_get(cfg) + 2, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG6_TCKDPDX, 2) |
	      FIELD_PREP(DDRMC_DRAMTMG6_TCKDPDE, 2);
	write32_with_dbg(DDRMC_DRAMTMG6(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG7_TCKPDX, dramtmg7_tckpdx_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG7_TCKPDE, dramtmg7_tckpde_get(cfg));
	write32_with_dbg(DDRMC_DRAMTMG7(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG13_TPPD, 2) |
	      FIELD_PREP(DDRMC_DRAMTMG13_TCCDMW, 16) |
	      FIELD_PREP(DDRMC_DRAMTMG13_ODTLOFF, DIV_ROUND_UP(odtloff_get(cfg->tck), 2));
	write32_with_dbg(DDRMC_DRAMTMG13(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG14_TXSR, dramtmg14_txsr_get(cfg));
	write32_with_dbg(DDRMC_DRAMTMG14(ctrl_id), val);
}

static void addrmap_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint8_t next_bit;
	uint32_t val, tmp1, tmp2;

	write32_with_dbg(DDRMC_ADDRMAP2(ctrl_id), 0);
	write32_with_dbg(DDRMC_ADDRMAP3(ctrl_id), 0);

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
	write32_with_dbg(DDRMC_ADDRMAP4(ctrl_id), val);
	next_bit = cfg->col_addr_bits;

	val = FIELD_PREP(DDRMC_ADDRMAP1_BANK_B0, next_bit - 2) |
	      FIELD_PREP(DDRMC_ADDRMAP1_BANK_B1, next_bit - 2) |
	      FIELD_PREP(DDRMC_ADDRMAP1_BANK_B2, (cfg->bank_addr_bits == 2) ? 63 : next_bit - 2);
	write32_with_dbg(DDRMC_ADDRMAP1(ctrl_id), val);
	next_bit += cfg->bank_addr_bits;

	val = FIELD_PREP(DDRMC_ADDRMAP8_BG_B0, (cfg->bank_group_bits == 0) ? 63 : next_bit - 2) |
	      FIELD_PREP(DDRMC_ADDRMAP8_BG_B1, (cfg->bank_addr_bits == 2) ? next_bit - 2 : 63);
	write32_with_dbg(DDRMC_ADDRMAP8(ctrl_id), val);
	next_bit += cfg->bank_group_bits;

	val = FIELD_PREP(DDRMC_ADDRMAP5_ROW_B0, next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP5_ROW_B1, next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP5_ROW_B2_B10, next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP5_ROW_B11, next_bit - 6);
	write32_with_dbg(DDRMC_ADDRMAP5(ctrl_id), val);

	val = FIELD_PREP(DDRMC_ADDRMAP6_ROW_B12, (cfg->row_addr_bits <= 12) ? 15 : next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP6_ROW_B13, (cfg->row_addr_bits <= 13) ? 15 : next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP6_ROW_B14, (cfg->row_addr_bits <= 14) ? 15 : next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP6_ROW_B15, (cfg->row_addr_bits <= 15) ? 15 : next_bit - 6);
	write32_with_dbg(DDRMC_ADDRMAP6(ctrl_id), val);

	val = FIELD_PREP(DDRMC_ADDRMAP7_ROW_B16, (cfg->row_addr_bits <= 16) ? 15 : next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP7_ROW_B17, (cfg->row_addr_bits <= 17) ? 15 : next_bit - 6);
	write32_with_dbg(DDRMC_ADDRMAP7(ctrl_id), val);
	next_bit += cfg->row_addr_bits;

	val = FIELD_PREP(DDRMC_ADDRMAP0_CS_B0, (cfg->ranks == 1) ? 31 : next_bit - 6) |
	      FIELD_PREP(DDRMC_ADDRMAP0_CS_B1, (cfg->ranks <= 2) ? 31 : next_bit - 6);
	write32_with_dbg(DDRMC_ADDRMAP0(ctrl_id), val);
}

static void dfi_timings_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t val;
	uint8_t cl = cfg->taa;
	uint8_t cwl = cwl_get(cfg->tck);
	uint8_t ctrl_delay = 4;

	/* TODO: Some DFI timings should be moved to Kconfig */
	val = FIELD_PREP(DDRMC_DFITMG0_WRLAT, cwl + 1 - 5) |
	      FIELD_PREP(DDRMC_DFITMG0_WRDATA, 2) |
	      FIELD_PREP(DDRMC_DFITMG0_WRDATA_DFICLK, 1) |
	      FIELD_PREP(DDRMC_DFITMG0_RDDATAEN, cl - 5) |
	      FIELD_PREP(DDRMC_DFITMG0_RDDATA_DFI_CLK, 1) |
	      FIELD_PREP(DDRMC_DFITMG0_CTRLDELAY, ctrl_delay);
	write32_with_dbg(DDRMC_DFITMG0(ctrl_id), val);

	/* TODO: Verification of DFITMG1_WRDATA_DELAY is needed */
	val = FIELD_PREP(DDRMC_DFITMG1_DRAM_CLK_EN, ctrl_delay - 2) |
	      FIELD_PREP(DDRMC_DFITMG1_DRAM_CLK_DIS, ctrl_delay - 2) |
	      FIELD_PREP(DDRMC_DFITMG1_WRDATA_DELAY, DIV_ROUND_UP(ctrl_delay + 6 + CONFIG_BURST_LEN / 2, 2) + 1);
	write32_with_dbg(DDRMC_DFITMG1(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DFITMG2_WRCSLAT, cwl + 1 - 5) |
	      FIELD_PREP(DDRMC_DFITMG2_RDCSLAT, cl - 5);
	write32_with_dbg(DDRMC_DFITMG2(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DFIUPD0_CTRLUPD_MIN, 24) | FIELD_PREP(DDRMC_DFIUPD0_CTRLUPD_MAX, 48);
	write32_with_dbg(DDRMC_DFIUPD0(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DFIUPD1_CTRLUPD_INTMIN, 64) |
	      FIELD_PREP(DDRMC_DFIUPD1_CTRLUPD_INTMAX, 192);
	write32_with_dbg(DDRMC_DFIUPD1(ctrl_id), val);
}

void ddrmc_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t val;
	uint16_t tck = cfg->tck;

	val = FIELD_PREP(DDRMC_MSTR_LPDDR4, 1) |
	      FIELD_PREP(DDRMC_MSTR_BURST_RDWR, CONFIG_BURST_LEN / 2) |
	      FIELD_PREP(DDRMC_MSTR_RANKS, (1 << cfg->ranks) - 1) |
	      FIELD_PREP(DDRMC_MSTR_DEVCONFIG, __builtin_ffsll(cfg->device_width) - 3);
	write32_with_dbg(DDRMC_MSTR(ctrl_id), val);

	val = FIELD_PREP(DDRMC_INIT0_PRECKE, DIV_ROUND_UP(2000000000, tck * 2048)) |
	      FIELD_PREP(DDRMC_INIT0_POSTCKE, DIV_ROUND_UP(2000000, tck * 2048)) |
	      FIELD_PREP(DDRMC_INIT0_SKIP_DRAM_INIT, 3);
	write32_with_dbg(DDRMC_INIT0(ctrl_id), val);

	val = FIELD_PREP(DDRMC_RFSHTMG_TRFC, DIV_ROUND_UP(trfc_ab_get(cfg), 2)) |
	      FIELD_PREP(DDRMC_RFSHTMG_TREFI, CONFIG_DRAM_TIMING_TREFI / tck / 64);
	write32_with_dbg(DDRMC_RFSHTMG(ctrl_id), val);

	val = FIELD_PREP(DDRMC_ZQCTL0_TZQCS, DIV_ROUND_UP(tzqcs_get(cfg), 2)) |
	      FIELD_PREP(DDRMC_ZQCTL0_TZQOPER, DIV_ROUND_UP(tzqoper_get(cfg), 2));
	write32_with_dbg(DDRMC_ZQCTL0(ctrl_id), val);

	write32_with_dbg(DDRMC_DBICTL(ctrl_id), 0);
	write32_with_dbg(DDRMC_ODTMAP(ctrl_id), 0);

	dram_timings_cfg(ctrl_id, cfg);
	dfi_timings_cfg(ctrl_id, cfg);
	addrmap_cfg(ctrl_id, cfg);
}
