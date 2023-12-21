// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2021-2023 RnD Center "ELVEES", JSC

#include <common.h>
#include <ddrmc.h>
#include <regs.h>

#define LPDDR4_MR1_BL	  GENMASK(1, 0)
#define LPDDR4_MR1_WR_PRE BIT(2)
#define LPDDR4_MR1_RD_PRE BIT(3)
#define LPDDR4_MR1_NWR	  GENMASK(6, 4)
#define LPDDR4_MR1_RPST	  BIT(7)

#define LPDDR4_MR2_RL	 GENMASK(2, 0)
#define LPDDR4_MR2_WL	 GENMASK(5, 3)
#define LPDDR4_MR2_WLS	 BIT(6)
#define LPDDR4_MR2_WRLEV BIT(7)

#define LPDDR4_MR3_PUCAL	BIT(0)
#define LPDDR4_MR3_WR_POSTAMBLE BIT(1)
#define LPDDR4_MR3_PDDS		GENMASK(5, 3)
#define LPDDR4_MR3_DBI_RD	BIT(6)
#define LPDDR4_MR3_DBI_WR	BIT(7)

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

static int tzqcs_interval_get(struct ddr_cfg *cfg)
{
	/* There is no such timing in JEDEC spec. NXP uses 32 ms. */
	return 32000000000ULL / 1024 / cfg->tck;
}

static int tzq_reset_nop_get(struct ddr_cfg *cfg)
{
	return ps2clk_jedec(CONFIG_DRAM_TIMING_TZQRESET, cfg->tck);
}

static int dramtmg0_trasmin_get(struct ddr_cfg *cfg)
{
	return DIV_ROUND_UP(cfg->trasmin, 2);
}

static int dramtmg0_trasmax_get(struct ddr_cfg *cfg)
{
	int trasmax = CONFIG_DRAM_TIMING_TRASMAX / cfg->tck / 1024;
	trasmax = (trasmax - 1) / 2;

	return max(trasmax, 1);
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

	return DIV_ROUND_UP(ret, 2) * 13 / 10;
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
	return DIV_ROUND_UP(ret, 2) * 13 / 10;
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
	int tmrwckel = 0;

	tmrw = ps2clk_jedec(CONFIG_DRAM_TIMING_TMRW, cfg->tck);
	tmrw = max(tmrw, 10);

	tmrwckel = ps2clk_jedec(CONFIG_DRAM_TIMING_TMRWCKEL, cfg->tck);
	tmrwckel = max(tmrwckel, 10);

	return DIV_ROUND_UP(max(tmrw, tmrwckel), 2);
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
	int txsr = trfc_ab_get(cfg) + ps2clk_jedec(7500, cfg->tck);

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

	mr1 = FIELD_PREP(LPDDR4_MR1_BL, bl) | FIELD_PREP(LPDDR4_MR1_WR_PRE, 1) |
	      FIELD_PREP(LPDDR4_MR1_RD_PRE, 0) | FIELD_PREP(LPDDR4_MR1_NWR, nwr) |
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
	case 12:
		rl = 1;
		break;
	case 14:
	case 16:
		rl = 2;
		break;
	case 20:
	case 22:
		rl = 3;
		break;
	case 24:
		rl = 4;
		break;
	case 28:
		rl = (IS_ENABLED(CONFIG_READ_DBI)) ? 4 : 5;
		break;
	case 32:
		rl = (IS_ENABLED(CONFIG_READ_DBI)) ? 5 : 6;
		break;
	}

	mr2 = FIELD_PREP(LPDDR4_MR2_RL, rl) | FIELD_PREP(LPDDR4_MR2_WL, cwl_get(cfg->tck) / 2 - 2) |
	      FIELD_PREP(LPDDR4_MR2_WLS, 0) | FIELD_PREP(LPDDR4_MR2_WRLEV, 0);

	return mr2;
}

uint8_t lpddr4_mr3_get(struct ddr_cfg *cfg)
{
	uint8_t mr3;
	bool read_dbi_en = (IS_ENABLED(CONFIG_READ_DBI)) ? true : false;
	bool write_dbi_en = (IS_ENABLED(CONFIG_WRITE_DBI)) ? true : false;

	mr3 = FIELD_PREP(LPDDR4_MR3_PUCAL, 1) | FIELD_PREP(LPDDR4_MR3_WR_POSTAMBLE, 1) |
	      FIELD_PREP(LPDDR4_MR3_PDDS, 6) | FIELD_PREP(LPDDR4_MR3_DBI_RD, read_dbi_en) |
	      FIELD_PREP(LPDDR4_MR3_DBI_WR, write_dbi_en);

	return mr3;
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

	mr11 = FIELD_PREP(LPDDR4_MR11_DQ_ODT, dq_odt) | FIELD_PREP(LPDDR4_MR11_CA_ODT, ca_odt);

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
	      FIELD_PREP(DDRMC_DRAMTMG0_TFAW, DIV_ROUND_UP(cfg->tfaw, 2)) |
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

	val = FIELD_PREP(DDRMC_DRAMTMG4_TRP, DIV_ROUND_UP(cfg->trp, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG4_TRRD, DIV_ROUND_UP(cfg->trrds, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG4_TCCD, DIV_ROUND_UP(8, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG4_TRCD, DIV_ROUND_UP(cfg->trcd, 2));
	write32_with_dbg(DDRMC_DRAMTMG4(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG5_TCKE, dramtmg5_tcke_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG5_TCKESR, dramtmg5_tckesr_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG5_TCKSRE, dramtmg5_tcksre_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG5_TCKSRX, dramtmg5_tcksrx_get(cfg));
	write32_with_dbg(DDRMC_DRAMTMG5(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG6_TCKCSX, DIV_ROUND_UP(txp_get(cfg) + 2, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG6_TCKDPDX, 1) | FIELD_PREP(DDRMC_DRAMTMG6_TCKDPDE, 1);
	write32_with_dbg(DDRMC_DRAMTMG6(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG7_TCKPDX, dramtmg7_tckpdx_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG7_TCKPDE, dramtmg7_tckpde_get(cfg));
	write32_with_dbg(DDRMC_DRAMTMG7(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG13_TPPD, 2) | FIELD_PREP(DDRMC_DRAMTMG13_TCCDMW, 16) |
	      FIELD_PREP(DDRMC_DRAMTMG13_ODTLOFF, DIV_ROUND_UP(odtloff_get(cfg->tck), 2));
	write32_with_dbg(DDRMC_DRAMTMG13(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG14_TXSR, dramtmg14_txsr_get(cfg));
	write32_with_dbg(DDRMC_DRAMTMG14(ctrl_id), val);
}

static void addrmap_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint8_t next_bit;
	uint32_t val;

	/* According to LPDDR4 specification, all memories have 10 bits of column address */
	write32_with_dbg(DDRMC_ADDRMAP2(ctrl_id), 0);
	write32_with_dbg(DDRMC_ADDRMAP3(ctrl_id), 0);
	write32_with_dbg(DDRMC_ADDRMAP4(ctrl_id), FIELD_PREP(DDRMC_ADDRMAP4_COL_B10, 31) |
							  FIELD_PREP(DDRMC_ADDRMAP4_COL_B11, 31));

	next_bit = cfg->col_addr_bits;

	/* Do not assign 12th bit of AXI address (10th bit of HIF address)
	 * if 4 KiB interleaving is enabled.
	 */
	if (IS_ENABLED(CONFIG_PLATFORM_MCOM03) && IS_ENABLED(CONFIG_INTERLEAVING_SIZE_4K))
		next_bit += 1;

	val = FIELD_PREP(DDRMC_ADDRMAP1_BANK_B0, next_bit - 2) |
	      FIELD_PREP(DDRMC_ADDRMAP1_BANK_B1, next_bit - 2) |
	      FIELD_PREP(DDRMC_ADDRMAP1_BANK_B2, (cfg->bank_addr_bits == 2) ? 63 : next_bit - 2);
	write32_with_dbg(DDRMC_ADDRMAP1(ctrl_id), val);
	next_bit += cfg->bank_addr_bits;

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

/* TODO: Make the code below platform independent */
#ifdef CONFIG_PLATFORM_MCOM03
static void sar_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint64_t ddr_low_size = min(CONFIG_DDR_LOW_SIZE * 1024 * 1024ULL, cfg->full_size);
	uint64_t ddr_high_size = cfg->full_size - ddr_low_size;
	uint64_t ddr_low_start = (ctrl_id == 0) ? CONFIG_MEM_REGION0_START :
						  CONFIG_MEM_REGION2_START;
	uint64_t ddr_high_start = (ctrl_id == 0) ? CONFIG_MEM_REGION1_START :
						   CONFIG_MEM_REGION3_START;

	if (IS_ENABLED(CONFIG_INTERLEAVING)) {
		ddr_low_start = CONFIG_MEM_REGION0_START;
		ddr_high_start = CONFIG_MEM_REGION1_START;
		ddr_low_size *= 2;
		ddr_high_size *= 2;
	}

	write32_with_dbg(DDRMC_SARBASE(ctrl_id, 0), ddr_low_start / CONFIG_DDRMC_SAR_MINSIZE);
	write32_with_dbg(DDRMC_SARSIZE(ctrl_id, 0), ddr_low_size / CONFIG_DDRMC_SAR_MINSIZE - 1);
	write32_with_dbg(DDRMC_SARBASE(ctrl_id, 1), ddr_high_start / CONFIG_DDRMC_SAR_MINSIZE);
	write32_with_dbg(DDRMC_SARSIZE(ctrl_id, 1), ddr_high_size / CONFIG_DDRMC_SAR_MINSIZE - 1);
	write32_with_dbg(DDRMC_SARBASE(ctrl_id, 2), 256);
	write32_with_dbg(DDRMC_SARSIZE(ctrl_id, 2), 0);
	write32_with_dbg(DDRMC_SARBASE(ctrl_id, 3), 257);
	write32_with_dbg(DDRMC_SARSIZE(ctrl_id, 3), 0);
}
#else
static void sar_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
}
#endif

static void dfi_timings_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t val;
	uint8_t cl = cfg->taa;
	uint8_t cwl = cwl_get(cfg->tck);
	uint8_t ctrl_delay = 4;

	/* TODO: Some DFI timings should be moved to Kconfig */
	val = FIELD_PREP(DDRMC_DFITMG0_WRLAT, cwl + 1 - 5) | FIELD_PREP(DDRMC_DFITMG0_WRDATA, 2) |
	      FIELD_PREP(DDRMC_DFITMG0_WRDATA_DFICLK, 1) |
	      FIELD_PREP(DDRMC_DFITMG0_RDDATAEN, cl - 5) |
	      FIELD_PREP(DDRMC_DFITMG0_RDDATA_DFI_CLK, 1) |
	      FIELD_PREP(DDRMC_DFITMG0_CTRLDELAY, ctrl_delay);
	write32_with_dbg(DDRMC_DFITMG0(ctrl_id), val);

	/* TODO: Verification of DFITMG1_WRDATA_DELAY is needed */
	val = FIELD_PREP(DDRMC_DFITMG1_DRAM_CLK_EN, ctrl_delay - 2) |
	      FIELD_PREP(DDRMC_DFITMG1_DRAM_CLK_DIS, ctrl_delay - 2) |
	      FIELD_PREP(DDRMC_DFITMG1_WRDATA_DELAY,
			 DIV_ROUND_UP(ctrl_delay + 6 + CONFIG_BURST_LEN / 2, 2) + 1);
	write32_with_dbg(DDRMC_DFITMG1(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DFITMG2_WRCSLAT, cwl + 1 - 5) |
	      FIELD_PREP(DDRMC_DFITMG2_RDCSLAT, cl - 5);
	write32_with_dbg(DDRMC_DFITMG2(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DFIUPD0_CTRLUPD_MIN, 24) | FIELD_PREP(DDRMC_DFIUPD0_CTRLUPD_MAX, 64);
	write32_with_dbg(DDRMC_DFIUPD0(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DFIUPD1_CTRLUPD_INTMIN, 64) |
	      FIELD_PREP(DDRMC_DFIUPD1_CTRLUPD_INTMAX, 192);
	write32_with_dbg(DDRMC_DFIUPD1(ctrl_id), val);
}

static void port_priority_cfg(int ctrl_id)
{
	uint32_t val;
	int i;

	for (i = 0; i < CONFIG_DDRMC_AXI_PORTS; i++) {
		if (CONFIG_DDRMC_HPR_PORT_MASK & BIT(i)) {
			val = read32(DDRMC_PCFGQOS0(ctrl_id, i));
			val &= ~DDRMC_PCFGQOS0_RQOS_MAP_REGION0;
			val |= FIELD_PREP(DDRMC_PCFGQOS0_RQOS_MAP_REGION0, 2);
			write32_with_dbg(DDRMC_PCFGQOS0(ctrl_id, i), val);
		}

		if (CONFIG_DDRMC_PORT_READ_TIMEOUT_MASK & BIT(i)) {
			val = read32(DDRMC_PCFGR(ctrl_id, i));
			val &= ~DDRMC_PCFGR_RD_PORT_PRIORITY;
			val |= FIELD_PREP(DDRMC_PCFGR_RD_PORT_PRIORITY, 0x1) |
			       DDRMC_PCFGR_RD_PORT_AGING_EN;
			write32_with_dbg(DDRMC_PCFGR(ctrl_id, i), val);
		}

		if (CONFIG_DDRMC_PORT_WRITE_TIMEOUT_MASK & BIT(i)) {
			val = read32(DDRMC_PCFGW(ctrl_id, i));
			val &= ~DDRMC_PCFGW_WR_PORT_PRIORITY;
			val |= FIELD_PREP(DDRMC_PCFGW_WR_PORT_PRIORITY, 0x1) |
			       DDRMC_PCFGW_WR_PORT_AGING_EN;
			write32_with_dbg(DDRMC_PCFGW(ctrl_id, i), val);
		}
	}
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

	val = FIELD_PREP(DDRMC_ZQCTL1_TZQCS_INTERVAL, DIV_ROUND_UP(tzqcs_interval_get(cfg), 2)) |
	      FIELD_PREP(DDRMC_ZQCTL1_TZQ_RESET_NOP, DIV_ROUND_UP(tzq_reset_nop_get(cfg), 2));
	write32_with_dbg(DDRMC_ZQCTL1(ctrl_id), val);

	val = 0;
	if (IS_ENABLED(CONFIG_WRITE_DBI))
		val |= FIELD_PREP(DDRMC_DBICTL_WR_DBI_EN, 1);

	if (IS_ENABLED(CONFIG_READ_DBI))
		val |= FIELD_PREP(DDRMC_DBICTL_RD_DBI_EN, 1);

	write32_with_dbg(DDRMC_DBICTL(ctrl_id), val);

	write32_with_dbg(DDRMC_ODTMAP(ctrl_id), 0);

	dram_timings_cfg(ctrl_id, cfg);
	dfi_timings_cfg(ctrl_id, cfg);
	addrmap_cfg(ctrl_id, cfg);
	sar_cfg(ctrl_id, cfg);
	port_priority_cfg(ctrl_id);
}
