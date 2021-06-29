// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 RnD Center "ELVEES", JSC
 */

#include <common.h>
#include <config.h>
#include <ddrcfg.h>
#include <plat/plat.h>
#include <plat/mcom03/regs.h>

enum subsystem_reset_lines {
	CPU_SUBS,
	SDR_SUBS,
	MEDIA_SUBS,
	CORE_SUBS,
	HSPERIPH_SUBS,
	LSPERIPH0_SUBS,
	LSPERIPH1_SUBS,
	DDR_SUBS,
	TOP_SUBS,
	RISC0_SUBS
};

enum ucg_qfsm_state {
	Q_FSM_STOPPED = 0,
	Q_FSM_CLK_EN = 1,
	Q_FSM_REQUEST = 2,
	Q_FSM_DENIED = 3,
	Q_FSM_EXIT = 4,
	Q_FSM_RUN = 6,
	Q_FSM_CONTINUE = 7
};

struct pll_settings {
	uint32_t xtal;
	uint16_t tck;
	uint8_t nr;
	uint16_t nf;
	uint8_t od;
};

struct pll_settings pll_settings[2][6] = {
	{
		{ 27000000, DRAM_TCK_533, 4, 393, 15 },
		{ 27000000, DRAM_TCK_1066, 8, 1063, 11 },
		{ 27000000, DRAM_TCK_1600, 1, 236, 7 },
		{ 27000000, DRAM_TCK_2133, 8, 1065, 5 },
		{ 27000000, DRAM_TCK_2666, 2, 295, 3 },
		{ 27000000, DRAM_TCK_3200, 1, 236, 3 },
	},
	{
		{ 27000000, 833, 8, 799, 1 },
	},
};

void phy_write32(int ctrl_id, unsigned long addr, uint32_t val)
{
	write32(PHY_BASE(ctrl_id) + addr, val);
}

void phy_write16(int ctrl_id, unsigned long addr, uint16_t val)
{
	write16(PHY_BASE(ctrl_id) + addr, val);
}

uint32_t phy_read32(int ctrl_id, unsigned long addr)
{
	return read32(PHY_BASE(ctrl_id) + addr);
}

static void subsystem_reset_deassert(enum subsystem_reset_lines line)
{
	write32(SERVICE_X_PPOLICY(line), RESET_PPOLICY_PP_ON);
	while (read32(SERVICE_X_PSTATUS(line)) != RESET_PPOLICY_PP_ON)
		continue;
}

int platform_power_up(void)
{
	uint32_t val;
	int i;
	int reset_lines[] = { LSPERIPH0_SUBS, LSPERIPH1_SUBS, DDR_SUBS };

	for (i = 0; i < ARRAY_SIZE(reset_lines); i++)
		subsystem_reset_deassert(reset_lines[i]);

	val = read32(SERVICE_TOP_CLK_GATE);
	val |= SERVICE_TOP_CLK_GATE_LSPERIPH0 |
	       SERVICE_TOP_CLK_GATE_LSPERIPH1 |
	       SERVICE_TOP_CLK_GATE_DDR;
	write32(SERVICE_TOP_CLK_GATE, val);

	return 0;
}

int platform_reset_ctl(int ctrl_id, enum reset_type reset, enum reset_action action)
{
	uint32_t status;
	uint32_t request = (action == RESET_DEASSERT) ? RESET_PPOLICY_PP_ON : 0;
	int ret;

	if (reset == PRESETN) {
		write32(DDR_PRESETN_PPOLICY(ctrl_id), request);
		ret = read32_poll_timeout(status, status == request, USEC, MSEC,
					  DDR_PRESETN_PSTATUS(ctrl_id));
	}
	else {
		write32(DDR_RESETN_PPOLICY(ctrl_id), request);
		ret = read32_poll_timeout(status, status == request, USEC, MSEC,
					  DDR_RESETN_PSTATUS(ctrl_id));
	}

	return ret;
}

static int pll_settings_get(int pll_id, int tck, struct pll_settings *pll_cfg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(pll_settings[pll_id]); i++) {
		if (tck == pll_settings[pll_id][i].tck &&
		    CONFIG_DDR_XTAL_FREQ == pll_settings[pll_id][i].xtal)  {
			pll_cfg->nr = pll_settings[pll_id][i].nr;
			pll_cfg->od = pll_settings[pll_id][i].od;
			pll_cfg->nf = pll_settings[pll_id][i].nf;
			break;
		}
	}

	if (i == ARRAY_SIZE(pll_settings[pll_id]))
		return -ECLOCKCFG;

	print_dbg("pll_settings_get: pll_id %d, tck %d, nr: %d, nf: %d, od: %d,\n",
		  pll_id, tck, pll_cfg->nr, pll_cfg->nf, pll_cfg->od);

	return 0;
}

static int pll_cfg(int pll_id, int tck)
{
	struct pll_settings pll_cfg;
	uint32_t val;
	int ret;

	ret = pll_settings_get(pll_id, tck, &pll_cfg);
	if (ret)
		return ret;

	val = FIELD_PREP(DDR_PLL_CFG_SEL, 1) |
	      FIELD_PREP(DDR_PLL_CFG_MAN, 1) |
	      FIELD_PREP(DDR_PLL_CFG_OD, pll_cfg.od) |
	      FIELD_PREP(DDR_PLL_CFG_NF, pll_cfg.nf) |
	      FIELD_PREP(DDR_PLL_CFG_NR, pll_cfg.nr);
	write32(DDR_PLL_CFG(pll_id), val);

	ret = read32_poll_timeout(val, val & DDR_PLL_CFG_LOCK, USEC, MSEC, DDR_PLL_CFG(pll_id));
	if (ret)
		return -ECLOCKCFG;

	return 0;
}

static void ucg_bypass_enable(int ucg_id, int chan_id)
{
	uint32_t val;

	val = read32(DDR_UCG_CTR(ucg_id, chan_id));
	if (FIELD_GET(DDR_UCG_CTR_QFSM_STATE, val) == Q_FSM_RUN) {
		val = read32(DDR_UCG_BP(ucg_id));
		val |= BIT(chan_id);
		write32(DDR_UCG_BP(ucg_id), val);
	}
}

static void ucg_bypass_disable(int ucg_id, int chan_id)
{
	uint32_t val;

	val = read32(DDR_UCG_BP(ucg_id));

	if (val & BIT(chan_id))
		val &= ~BIT(chan_id);

	write32(DDR_UCG_BP(ucg_id), val);
}

static int ucg_channel_cfg(int ucg_id, int chan_id, int div)
{
	uint32_t val;
	int ret;

	val = read32(DDR_UCG_CTR(ucg_id, chan_id));
	val &= ~DDR_UCG_CTR_DIFF_COEFF;
	write32(DDR_UCG_CTR(ucg_id, chan_id), FIELD_PREP(DDR_UCG_CTR_DIFF_COEFF, div));

	ret = read32_poll_timeout(val, val & DDR_UCG_CTR_DIFF_LOCK, USEC, MSEC,
				  DDR_UCG_CTR(ucg_id, chan_id));
	if (ret)
		return -ECLOCKCFG;

	val = read32(DDR_UCG_CTR(ucg_id, chan_id));
	if (FIELD_GET(DDR_UCG_CTR_QFSM_STATE, val) != Q_FSM_RUN) {
		val &= ~DDR_UCG_CTR_LPI_EN;
		val |= DDR_UCG_CTR_CLK_EN;
		write32(DDR_UCG_CTR(ucg_id, chan_id), val);

		ret = read32_poll_timeout(val, FIELD_GET(DDR_UCG_CTR_QFSM_STATE, val) == Q_FSM_RUN,
					  USEC, MSEC, DDR_UCG_CTR(ucg_id, chan_id));
		if (ret)
			return -ECLOCKCFG;
	}

	return 0;
}

int platform_clk_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	static int pll_init_done;
	int ret;
	uint32_t val;
	uint8_t axi_chan_divs[] = {
		6, /* SYS channel  200 MHz */
		2, /* SDR channel  600 MHz */
		2, /* PCIe channel 600 MHz */
		6, /* ISP channel  200 MHz */
		2, /* GPU channel  600 MHz */
		4, /* VPU channel  300 MHz */
		3, /* DP channel   400 MHz */
		2, /* CPU channel  600 MHz */
		4, /* SERV channel 300 MHz */
		3, /* HSP channel  400 MHz */
		6, /* LSP0 channel 200 MHz */
		6  /* LSP1 channel 200 MHz */
	};

	if (pll_init_done == 0) {
		int i;

		for (i = 0; i < CONFIG_DDRMC_AXI_PORTS + 1; i++)
			ucg_bypass_enable(1, i);

		ret = pll_cfg(1, 833);
		if (ret)
			return ret;

		for (i = 0; i < CONFIG_DDRMC_AXI_PORTS + 1; i++) {
			ret = ucg_channel_cfg(1, i, axi_chan_divs[i]);
			if (ret)
				return ret;
		}

		for (i = 0; i < CONFIG_DDRMC_AXI_PORTS + 1; i++)
			ucg_bypass_disable(1, i);

		for (i = 0; i < 4; i++)
			ucg_bypass_enable(0, i);

		ret = pll_cfg(0, cfg->tck);
		if (ret)
			return ret;

		pll_init_done = 1;
	}

	ret = ucg_channel_cfg(0, 2 * ctrl_id, 1);
	if (ret)
		return ret;

	ucg_bypass_disable(0, 2 * ctrl_id);

	/* Enable all AXI channels */
	val = DDR_AXI_CHS_ENABLE_SDR | DDR_AXI_CHS_ENABLE_PCIE |
	      DDR_AXI_CHS_ENABLE_ISP | DDR_AXI_CHS_ENABLE_GPU |
	      DDR_AXI_CHS_ENABLE_VPU | DDR_AXI_CHS_ENABLE_DP |
	      DDR_AXI_CHS_ENABLE_CPU | DDR_AXI_CHS_ENABLE_SERVICE |
	      DDR_AXI_CHS_ENABLE_HSP | DDR_AXI_CHS_ENABLE_LSP0 |
	      DDR_AXI_CHS_ENABLE_LSP1;

	if (ctrl_id == 0)
		val = FIELD_PREP(DDR_AXI_CHS_ENABLE_DDR0, val);
	else
		val = FIELD_PREP(DDR_AXI_CHS_ENABLE_DDR1, val);

	val |= read32(DDR_AXI_CHS_ENABLE);
	write32(DDR_AXI_CHS_ENABLE, val);

	return 0;
}

int platform_ddrcfg_get(int ctrl_id, struct ddr_cfg *cfg)
{
	/* TBD */

	return 0;
}

uint32_t platform_get_timer_count(void)
{
	return 0;
}

static void mem_regions_set(int init_mask, struct sysinfo *info)
{
	uint64_t *dsize = &info->dram_size[0];
	int i, free_region_idx = 0;
	uint64_t cfg_size[] = { CONFIG_MEM_REGION0_SIZE,
				CONFIG_MEM_REGION1_SIZE,
				CONFIG_MEM_REGION2_SIZE,
				CONFIG_MEM_REGION3_SIZE };
	unsigned long cfg_start[] = { CONFIG_MEM_REGION0_START,
				      CONFIG_MEM_REGION1_START,
				      CONFIG_MEM_REGION2_START,
				      CONFIG_MEM_REGION3_START };

	for (i = 0; i < CONFIG_DDRMC_MAX_NUMBER; i++) {
		if (!(init_mask & BIT(i)))
			continue;

		info->mem_regions[free_region_idx].start = cfg_start[i * 2];
		info->mem_regions[free_region_idx].size = min(dsize[i], cfg_size[i * 2]);
		free_region_idx++;
		info->mem_regions[free_region_idx].start = cfg_start[i * 2 + 1];
		info->mem_regions[free_region_idx].size =
			dsize[i] - min(dsize[i], cfg_size[i * 2]);
		free_region_idx++;
	}
}

int platform_system_init(int init_mask, struct sysinfo *info)
{
	mem_regions_set(init_mask, info);
	return 0;
}

int platform_uart_cfg(void)
{
	return 0;
}
