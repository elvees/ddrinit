// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2023 RnD Center "ELVEES", JSC

#include <common.h>
#include <ddrmc.h>
#include <regs.h>

#define DDR3_MR0_BL  GENMASK(1, 0)
#define DDR3_MR0_CAS GENMASK(6, 2)
#define DDR3_MR0_WR  GENMASK(11, 9)

#define DDR3_MR1_ODS_A1	    BIT(1)
#define DDR3_MR1_ODS_A5	    BIT(5)
#define DDR3_MR1_RTT_NOM_A2 BIT(2)
#define DDR3_MR1_RTT_NOM_A6 BIT(6)

#define DDR3_MR2_CWL GENMASK(5, 3)

static int cwl_get(int tck)
{
	if (tck >= DRAM_TCK_1066)
		return 6;
	else if (tck < DRAM_TCK_1066 && tck >= DRAM_TCK_1333)
		return 7;
	else if (tck < DRAM_TCK_1333 && tck >= DRAM_TCK_1600)
		return 8;
	else if (tck < DRAM_TCK_1600 && tck >= DRAM_TCK_1866)
		return 9;
	else if (tck < DRAM_TCK_1866 && tck >= DRAM_TCK_2133)
		return 10;
	else
		return 0;
}

static int twr_get(int tck)
{
	return ps2clk_jedec(CONFIG_DRAM_TIMING_TWR, tck);
}

static int trtp_get(int tck)
{
	int trtp = ps2clk_jedec(CONFIG_DRAM_TIMING_TRTP, tck);
	return max(trtp, 4);
}

static int txp_get(struct ddr_cfg *cfg)
{
	int txp = ps2clk_jedec(CONFIG_DRAM_TIMING_TXP, cfg->tck);
	return max(txp, 10);
}

static int tzqoper_get(struct ddr_cfg *cfg)
{
	int tzqoper = ps2clk_jedec(CONFIG_DRAM_TIMING_TZQOPER, cfg->tck);
	return max(tzqoper, 256);
}

static int tzqcs_get(struct ddr_cfg *cfg)
{
	int tzqcs = ps2clk_jedec(CONFIG_DRAM_TIMING_TZQCS, cfg->tck);
	return max(tzqcs, 64);
}

static int dramtmg0_trasmax_get(struct ddr_cfg *cfg)
{
	int trasmax = ps2clk_jedec(CONFIG_DRAM_TIMING_TRASMAX, cfg->tck) / 1024;
	trasmax = (trasmax - 1) / 2;

	return max(trasmax, 1);
}

static int dramtmg0_wr2pre_get(struct ddr_cfg *cfg)
{
	int ret = cwl_get(cfg->tck) + CONFIG_BURST_LEN / 2 + twr_get(cfg->tck);
	return ret / 2;
}

static int dramtmg1_rd2pre_get(struct ddr_cfg *cfg)
{
	int ret = max(trtp_get(cfg->tck), 4);
	return ret / 2;
}

static int dramtmg2_wr2rd_get(struct ddr_cfg *cfg)
{
	int ret = ps2clk_jedec(CONFIG_DRAM_TIMING_TWTR, cfg->tck);
	ret = max(ret, 4);
	ret += 2 + cwl_get(cfg->tck) + CONFIG_BURST_LEN / 2;
	return DIV_ROUND_UP(ret, 2);
}

static int dramtmg2_rd2wr_get(struct ddr_cfg *cfg)
{
	int ret = 4 + cfg->taa - cwl_get(cfg->tck) + CONFIG_BURST_LEN / 2;
	return DIV_ROUND_UP(ret, 2);
}

static int dramtmg3_tmod_get(struct ddr_cfg *cfg)
{
	int ret = ps2clk_jedec(CONFIG_DRAM_TIMING_TMOD, cfg->tck);
	ret = max(ret, 12);
	return DIV_ROUND_UP(ret, 2);
}

static int dramtmg5_tcke_get(struct ddr_cfg *cfg)
{
	int tcke = ps2clk_jedec(CONFIG_DRAM_TIMING_TCKE, cfg->tck);
	tcke = max(tcke, 3);
	return DIV_ROUND_UP(tcke, 2);
}

static int dramtmg5_tckesr_get(struct ddr_cfg *cfg)
{
	int tckesr = ps2clk_jedec(CONFIG_DRAM_TIMING_TCKE, cfg->tck);
	tckesr = max(tckesr, 3) + 1;
	return DIV_ROUND_UP(tckesr, 2);
}

static int dramtmg5_tcksre_get(struct ddr_cfg *cfg)
{
	int tcksre = ps2clk_jedec(10000, cfg->tck);
	tcksre = max(tcksre, 5);
	return DIV_ROUND_UP(tcksre, 2);
}

static int dramtmg5_tcksrx_get(struct ddr_cfg *cfg)
{
	int tcksrx;

	tcksrx = ps2clk_jedec(CONFIG_DRAM_TIMING_TCKSRX, cfg->tck);
	tcksrx = max(tcksrx, 5);
	return DIV_ROUND_UP(tcksrx, 2);
}

static int dramtmg8_txs_get(struct ddr_cfg *cfg)
{
	int txs = ps2clk_jedec(10000, cfg->tck) + cfg->trfc1;
	txs = max(txs, 5);
	return DIV_ROUND_UP(txs, 64);
}

uint16_t ddr3_mr0_get(struct ddr_cfg *cfg)
{
	uint8_t bl = (CONFIG_BURST_LEN == 8) ? 0 : 2;
	uint8_t cas = cfg->taa;
	uint8_t twr = twr_get(cfg->tck);
	uint16_t val;

	if (cas <= 11)
		cas = 4 * cas - 16;
	else if (cas >= 12)
		cas = 4 * cas - 47;

	if (twr <= 8)
		twr -= 4;
	else if (twr == 10 || twr == 12 || twr == 14)
		twr = twr >> 1;
	else if (twr == 16)
		twr = 0;

	val = FIELD_PREP(DDR3_MR0_BL, bl) | FIELD_PREP(DDR3_MR0_CAS, cas) |
	      FIELD_PREP(DDR3_MR0_WR, twr);

	return val;
}

uint16_t ddr3_mr1_get(struct ddr_cfg *cfg)
{
	uint16_t rtt_nom = 0, ods = 0;

	if (IS_ENABLED(CONFIG_DRAM_RTT_NOM_40))
		rtt_nom = DDR3_MR1_RTT_NOM_A2 | DDR3_MR1_RTT_NOM_A6;
	else if (IS_ENABLED(CONFIG_DRAM_RTT_NOM_60))
		rtt_nom = DDR3_MR1_RTT_NOM_A2;
	else if (IS_ENABLED(CONFIG_DRAM_RTT_NOM_120))
		rtt_nom = DDR3_MR1_RTT_NOM_A6;

	if (IS_ENABLED(CONFIG_DRAM_TX_IMPEDANCE_34))
		ods = DDR3_MR1_ODS_A1;

	return rtt_nom | ods;
}

uint16_t ddr3_mr2_get(struct ddr_cfg *cfg)
{
	return FIELD_PREP(DDR3_MR2_CWL, cwl_get(cfg->tck) - 5);
}

void dram_timings_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t val;

	val = FIELD_PREP(DDRMC_DRAMTMG0_TRASMIN, cfg->trasmin / 2) |
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
	      FIELD_PREP(DDRMC_DRAMTMG3_TMRD, 2);
	write32_with_dbg(DDRMC_DRAMTMG3(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG4_TRP, cfg->trp / 2 + 1) |
	      FIELD_PREP(DDRMC_DRAMTMG4_TRRD, DIV_ROUND_UP(cfg->trrds, 2)) |
	      FIELD_PREP(DDRMC_DRAMTMG4_TCCD, 2) |
	      FIELD_PREP(DDRMC_DRAMTMG4_TRCD, DIV_ROUND_UP(cfg->trcd, 2));
	write32_with_dbg(DDRMC_DRAMTMG4(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG5_TCKE, dramtmg5_tcke_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG5_TCKESR, dramtmg5_tckesr_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG5_TCKSRE, dramtmg5_tcksre_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG5_TCKSRX, dramtmg5_tcksrx_get(cfg));
	write32_with_dbg(DDRMC_DRAMTMG5(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DRAMTMG8_TXS, dramtmg8_txs_get(cfg)) |
	      FIELD_PREP(DDRMC_DRAMTMG8_TXSDLL, 8);
	write32_with_dbg(DDRMC_DRAMTMG8(ctrl_id), val);
}

static void addrmap_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	uint8_t next_bit;
	uint32_t val, tmp1 = 0, tmp2 = 0;

	write32_with_dbg(DDRMC_ADDRMAP2(ctrl_id), 0);
	write32_with_dbg(DDRMC_ADDRMAP3(ctrl_id), 0);

	if (cfg->col_addr_bits <= 10) {
		tmp1 = 31;
		tmp2 = 31;
	} else if (cfg->col_addr_bits == 11) {
		tmp2 = 31;
	}
	val = FIELD_PREP(DDRMC_ADDRMAP4_COL_B10, tmp1) | FIELD_PREP(DDRMC_ADDRMAP4_COL_B11, tmp2);
	write32(DDRMC_ADDRMAP4(ctrl_id), val);

	next_bit = cfg->col_addr_bits;

	/* TODO Add interleaving support */

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
	uint8_t dram_clk_en = (IS_ENABLED(CONFIG_PLATFORM_MCOM03)) ? ctrl_delay - 2 : ctrl_delay;
	uint8_t wrdata_delay = 6 + CONFIG_BURST_LEN / 2;

	if (IS_ENABLED(CONFIG_PLATFORM_MCOM03))
		wrdata_delay += ctrl_delay;

	/* TODO: Some DFI timings should be moved to Kconfig */
	val = FIELD_PREP(DDRMC_DFITMG0_WRLAT, cwl - 5) | FIELD_PREP(DDRMC_DFITMG0_WRDATA, 2) |
	      FIELD_PREP(DDRMC_DFITMG0_WRDATA_DFICLK, 1) |
	      FIELD_PREP(DDRMC_DFITMG0_RDDATAEN, cl - 5) |
	      FIELD_PREP(DDRMC_DFITMG0_RDDATA_DFI_CLK, 1) |
	      FIELD_PREP(DDRMC_DFITMG0_CTRLDELAY, ctrl_delay);
	write32_with_dbg(DDRMC_DFITMG0(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DFITMG1_DRAM_CLK_EN, dram_clk_en) |
	      FIELD_PREP(DDRMC_DFITMG1_DRAM_CLK_DIS, dram_clk_en) |
	      FIELD_PREP(DDRMC_DFITMG1_WRDATA_DELAY, DIV_ROUND_UP(wrdata_delay, 2) + 1);
	write32_with_dbg(DDRMC_DFITMG1(ctrl_id), val);

	val = FIELD_PREP(DDRMC_DFITMG2_WRCSLAT, cwl - 5) |
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

	val = FIELD_PREP(DDRMC_MSTR_DDR3, 1) |
	      FIELD_PREP(DDRMC_MSTR_BURST_RDWR, CONFIG_BURST_LEN / 2) |
	      FIELD_PREP(DDRMC_MSTR_RANKS, (1 << cfg->ranks) - 1);
	write32_with_dbg(DDRMC_MSTR(ctrl_id), val);

	write32_with_dbg(DDRMC_INIT0(ctrl_id), FIELD_PREP(DDRMC_INIT0_SKIP_DRAM_INIT, 3));

	val = FIELD_PREP(DDRMC_RFSHTMG_TRFC, DIV_ROUND_UP(cfg->trfc1, 2)) |
	      FIELD_PREP(DDRMC_RFSHTMG_TREFI, CONFIG_DRAM_TIMING_TREFI / tck / 64);
	write32_with_dbg(DDRMC_RFSHTMG(ctrl_id), val);

	val = FIELD_PREP(DDRMC_ZQCTL0_TZQCS, DIV_ROUND_UP(tzqcs_get(cfg), 2)) |
	      FIELD_PREP(DDRMC_ZQCTL0_TZQOPER, DIV_ROUND_UP(tzqoper_get(cfg), 2));
	write32_with_dbg(DDRMC_ZQCTL0(ctrl_id), val);

	val = FIELD_PREP(DDRMC_ODTCFG_RD_DELAY, cfg->taa - cwl_get(tck)) |
	      FIELD_PREP(DDRMC_ODTCFG_RD_HOLD, 6) | FIELD_PREP(DDRMC_ODTCFG_WR_HOLD, 6);
	write32_with_dbg(DDRMC_ODTCFG(ctrl_id), val);

	dram_timings_cfg(ctrl_id, cfg);
	dfi_timings_cfg(ctrl_id, cfg);
	addrmap_cfg(ctrl_id, cfg);
	sar_cfg(ctrl_id, cfg);
}
