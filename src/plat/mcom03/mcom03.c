// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 RnD Center "ELVEES", JSC
 */

#include <common.h>
#include <config.h>
#include <ddrcfg.h>
#include <i2c.h>
#include <plat/plat.h>
#include <plat/mcom03/regs.h>

#define DDRMC_SAR_MINSIZE 0x10000000

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

struct pll_settings pll_settings[2][9] = {
	{
		{ 27000000, DRAM_TCK_266, 0, 38, 15 },
		{ 27000000, DRAM_TCK_533, 0, 67, 13 },
		{ 27000000, DRAM_TCK_1066, 0, 117, 11 },
		{ 27000000, DRAM_TCK_1600, 0, 87, 5 },
		{ 27000000, DRAM_TCK_1866, 0, 102, 5 },
		{ 27000000, DRAM_TCK_2133, 0, 77, 3 },
		{ 27000000, DRAM_TCK_2400, 0, 131, 5 },
		{ 27000000, DRAM_TCK_2666, 0, 97, 3 },
		{ 27000000, DRAM_TCK_3200, 0, 117, 3 },
	},
	{
		{ 27000000, 833, 0, 87, 1 },
	},
};

static unsigned long i2c_base_addr[] = {
	(ARCH_OFFSET + 0x1630000),
	(ARCH_OFFSET + 0x1710000),
	(ARCH_OFFSET + 0x1720000),
	(ARCH_OFFSET + 0x1730000),
	(ARCH_OFFSET + 0x1f090000),
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

static int ucg_channel_cfg(unsigned long ucg_addr, int chan_id, int div)
{
	uint32_t val;
	int ret;

	val = read32(UCG_CTR(ucg_addr, chan_id));
	val &= ~(UCG_CTR_DIV_COEFF | UCG_CTR_LPI_EN);
	write32(UCG_CTR(ucg_addr, chan_id), val | FIELD_PREP(UCG_CTR_DIV_COEFF, div));
	ret = read32_poll_timeout(val, val & UCG_CTR_DIV_LOCK, USEC, MSEC,
				  UCG_CTR(ucg_addr, chan_id));
	if (ret)
		return -ECLOCKCFG;

	val = read32(UCG_CTR(ucg_addr, chan_id));
	if (FIELD_GET(UCG_CTR_QFSM_STATE, val) != Q_FSM_RUN) {
		val |= UCG_CTR_CLK_EN;
		write32(UCG_CTR(ucg_addr, chan_id), val);

		ret = read32_poll_timeout(val, FIELD_GET(UCG_CTR_QFSM_STATE, val) == Q_FSM_RUN,
					  USEC, MSEC, UCG_CTR(ucg_addr, chan_id));
		if (ret)
			return -ECLOCKCFG;
	}

	return 0;
}

#ifdef CONFIG_I2C_FREQ
int platform_i2c_cfg(int ctrl_id)
{
	uint32_t div;
	int ret;

	switch (ctrl_id) {
	/* TODO: Init subsystem, pads and clocks for other I2C */
	case 4:
		div = FIELD_GET(DDR_PLL_CFG_SEL, read32(SERVICE_SUBS_URB_PLL)) + 1;
		div = DIV_ROUND_UP(div * CONFIG_DDR_XTAL_FREQ, CONFIG_I2C_FREQ);
		/* Set dividers for clocks CLK_I2C4 and CLK_I2C4_EXT and enable them */
		ret = ucg_channel_cfg(SERVICE_SUBS_UCG_BASE, 9, div);
		if (ret)
			return ret;

		ret = ucg_channel_cfg(SERVICE_SUBS_UCG_BASE, 12, div);
		if (ret)
			return ret;

		return 0;
	default:
		return -EI2CCFG;
	}
}
#else
int platform_i2c_cfg(int ctrl_id)
{
	return 0;
}
#endif

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

	/* Init timer */
	write32(LSPERIPH1_UCG_CTR(7), 0x2);
	write32(DW_APB_LOAD_COUNT(0), 0);
	write32(DW_APB_CTRL(0), 0x5);

#if defined(CONFIG_DDR1_POWER_ENABLE) && (CONFIG_DDRMC_ACTIVE_MASK & 0x2)
	i = i2c_cfg(CONFIG_DDR1_I2C, 0x9);
	if (i)
		return i;

	i = i2c_write_reg(CONFIG_DDR1_I2C, 0x68, 0x25);
	if (i)
		return i;
#endif

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

unsigned long platform_i2c_base_get(int ctrl_id)
{
	if (ctrl_id < 0 || ctrl_id >= ARRAY_SIZE(i2c_base_addr))
		return 0;

	return i2c_base_addr[ctrl_id];  // cppcheck-suppress arrayIndexOutOfBoundsCond
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

static void ucg_bypass_enable(unsigned long ucg_addr, int chan_id)
{
	uint32_t val;

	val = read32(UCG_CTR(ucg_addr, chan_id));
	if (FIELD_GET(UCG_CTR_QFSM_STATE, val) == Q_FSM_RUN) {
		val = read32(UCG_BP(ucg_addr));
		val |= BIT(chan_id);
		write32(UCG_BP(ucg_addr), val);
	}
}

static void ucg_bypass_disable(unsigned long ucg_addr, int chan_id)
{
	uint32_t val;

	val = read32(UCG_BP(ucg_addr));

	if (val & BIT(chan_id))
		val &= ~BIT(chan_id);

	write32(UCG_BP(ucg_addr), val);
}

int platform_clk_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	static int pll_init_done;
	int ret;
	uint32_t val;
	uint8_t axi_chan_divs[] = {
		12, /* SYS channel  100 MHz */
		4, /* SDR channel  300 MHz */
		4, /* PCIe channel 300 MHz */
		12, /* ISP channel  100 MHz */
		4, /* GPU channel  300 MHz */
		8, /* VPU channel  150 MHz */
		6, /* DP channel   200 MHz */
		4, /* CPU channel  300 MHz */
		8, /* SERV channel 150 MHz */
		6, /* HSP channel  200 MHz */
		12, /* LSP0 channel 100 MHz */
		12  /* LSP1 channel 100 MHz */
	};

	if (pll_init_done == 0) {
		int i;

		for (i = 0; i < CONFIG_DDRMC_AXI_PORTS + 1; i++)
			ucg_bypass_enable(DDR_SUBS_UCG_BASE(1), i);

		ret = pll_cfg(1, 833);
		if (ret)
			return ret;

		for (i = 0; i < CONFIG_DDRMC_AXI_PORTS + 1; i++) {
			ret = ucg_channel_cfg(DDR_SUBS_UCG_BASE(1), i, axi_chan_divs[i]);
			if (ret)
				return ret;
		}

		for (i = 0; i < CONFIG_DDRMC_AXI_PORTS + 1; i++)
			ucg_bypass_disable(DDR_SUBS_UCG_BASE(1), i);

		for (i = 0; i < 4; i++)
			ucg_bypass_enable(DDR_SUBS_UCG_BASE(0), i);

#ifdef CONFIG_PHY_PLL_BYPASS
		int tck = cfg->tck / 4;
#else
		int tck = cfg->tck;
#endif
		ret = pll_cfg(0, tck);
		if (ret)
			return ret;

		pll_init_done = 1;
	}

#ifdef CONFIG_PHY_PLL_BYPASS
	ret = ucg_channel_cfg(0, 2 * ctrl_id, 4);
	if (ret)
		return ret;

	ret = ucg_channel_cfg(0, 2 * ctrl_id + 1, 1);
	if (ret)
		return ret;

	ucg_bypass_disable(0, 2 * ctrl_id + 1);
#else
	ret = ucg_channel_cfg(DDR_SUBS_UCG_BASE(0), 2 * ctrl_id, 1);
	if (ret)
		return ret;
#endif

	ucg_bypass_disable(DDR_SUBS_UCG_BASE(0), 2 * ctrl_id);

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

#ifndef CONFIG_SPD_EEPROM
int platform_ddrcfg_get(int ctrl_id, struct ddr_cfg *cfg)
{
	if (!(BIT(ctrl_id) & CONFIG_DDRMC_ACTIVE_MASK))
		return -EDIMMCFG;

	cfg->ranks = CONFIG_DRAM_RANKS;
	cfg->rank_size = 1ULL << 31;
	cfg->full_size = cfg->ranks * cfg->rank_size;
	cfg->device_width = 16;

	cfg->row_addr_bits = 16;
	cfg->col_addr_bits = 10;
	cfg->bank_addr_bits = 3;
	cfg->bank_group_bits = 0;

	cfg->tck = CONFIG_DRAM_TCK;
	cfg->taa = CONFIG_DRAM_CAS_LATENCY;
	cfg->trasmin = ps2clk_jedec(42000, cfg->tck);
	cfg->tfaw = ps2clk_jedec(40000, cfg->tck);
	cfg->trc = cfg->trasmin + ps2clk_jedec(21000, cfg->tck);

	return 0;
}
#endif

uint32_t platform_get_timer_count(void)
{
	/* The DW APB counter counts down, but this function
	 * requires the count to be incrementing. Invert the
	 * result.
	 */
	return ~read32(DW_APB_CURRENT_VALUE(0));
}

static void mem_regions_set(int init_mask, struct sysinfo *info)
{
	uint64_t *dsize = &info->dram_size[0];
	int i, free_region_idx = 0;
	uint64_t cfg_size[] = { CONFIG_MEM_REGION0_SIZE,
				CONFIG_MEM_REGION1_SIZE,
				CONFIG_MEM_REGION2_SIZE,
				CONFIG_MEM_REGION3_SIZE };
	uint64_t cfg_start[] = { CONFIG_MEM_REGION0_START,
				      CONFIG_MEM_REGION1_START,
				      CONFIG_MEM_REGION2_START,
				      CONFIG_MEM_REGION3_START };

	if (info->interleaving.enable) {
		cfg_start[2] = cfg_start[0];
		cfg_start[3] = cfg_start[1];
		for (i = 0; i < ARRAY_SIZE(cfg_size); i++)
			cfg_size[i] *= 2;
	}

	for (i = 0; i < CONFIG_DDRMC_MAX_NUMBER; i++) {
		if (!(init_mask & BIT(i)))
			continue;

		info->mem_regions[free_region_idx].start = cfg_start[i * 2];
		info->mem_regions[free_region_idx].size = min(dsize[i], cfg_size[i * 2]);
		free_region_idx++;

		write32(DDRMC_SARBASE(i, 0), cfg_start[i * 2] / DDRMC_SAR_MINSIZE);
		write32(DDRMC_SARBASE(i, 1), cfg_start[i * 2 + 1] / DDRMC_SAR_MINSIZE);
		write32(DDRMC_SARBASE(i, 2), 256);
		write32(DDRMC_SARBASE(i, 3), 257);

		write32(DDRMC_SARSIZE(i, 0), cfg_size[i * 2] / DDRMC_SAR_MINSIZE - 1);
		write32(DDRMC_SARSIZE(i, 1), cfg_size[i * 2 + 1] / DDRMC_SAR_MINSIZE - 1);
		write32(DDRMC_SARSIZE(i, 2), 0);
		write32(DDRMC_SARSIZE(i, 3), 0);
	}
}

static void interleaving_init(int init_mask, struct sysinfo *info)
{
	if (IS_ENABLED(CONFIG_INTERLEAVING) && (init_mask & 0x3) == 0x3) {
		info->interleaving.enable = true;
		info->interleaving.channels = 2;
		info->interleaving.size = 4096;
		write32(DDR_XDECODER_MODE, 1);
	} else {
		write32(DDR_XDECODER_MODE, 0);
	}
}

int platform_system_init(int init_mask, struct sysinfo *info)
{
	interleaving_init(init_mask, info);
	mem_regions_set(init_mask, info);

	return 0;
}

void gpio_portb_cfg(int hw_enable_mask, int direction)
{
	uint32_t mode_mask = read32(LSPERIPH1_GPIO_SWPORTB_CTL);
	uint32_t dir_mask = read32(LSPERIPH1_GPIO_SWPORTB_DDR);

	mode_mask |= hw_enable_mask;

	if (direction == GPIO_OUTPUT_DIRECTION)
		dir_mask |= hw_enable_mask;
	else
		dir_mask &= ~hw_enable_mask;

	write32(LSPERIPH1_GPIO_SWPORTB_CTL, mode_mask);
	write32(LSPERIPH1_GPIO_SWPORTB_DDR, dir_mask);
}

static void uart0_pads_cfg(void)
{
	uint32_t val;

	val = FIELD_PREP(LSPERIPH1_GPIO_PORTBN_PADCTR_SL, 3) |
	      FIELD_PREP(LSPERIPH1_GPIO_PORTBN_PADCTR_CTL, PAD_DRIVER_STREGTH_6mA);

	write32(LSPERIPH1_GPIO_PORTBN_PADCTR(6), val);

	val |= FIELD_PREP(LSPERIPH1_GPIO_PORTBN_PADCTR_SUS, 1) |
	       FIELD_PREP(LSPERIPH1_GPIO_PORTBN_PADCTR_E, 1);

	write32(LSPERIPH1_GPIO_PORTBN_PADCTR(7), val);
}

int platform_uart_cfg(void)
{
	gpio_portb_cfg(LSPERIPH1_GPIO_UART0_SOUT, GPIO_OUTPUT_DIRECTION);
	gpio_portb_cfg(LSPERIPH1_GPIO_UART0_SIN, GPIO_INPUT_DIRECTION);
	uart0_pads_cfg();

	write32(LSPERIPH1_UCG_CTR(6), 0x2);

	return 0;
}
