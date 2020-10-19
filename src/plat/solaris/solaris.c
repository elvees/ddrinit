// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#include <bitops.h>
#include <common.h>
#include <ddrcfg.h>
#include <plat/plat.h>
#include <plat/solaris/regs.h>

enum ddr_ucg_id { DDR_UCG_ID_WEST, DDR_UCG_ID_EAST };

enum pdci_reset_lines {
	PCDI_LINE_RESERVED = 0,
	PCDI_LINE_STARTUP,
	PCDI_LINE_NOC,
	PCDI_LINE_PERIPH_A,
	PCDI_LINE_DDR_NW,
	PCDI_LINE_DDR_SW,
	PCDI_LINE_DDR_SE,
	PCDI_LINE_DDR_NE,
	PCDI_LINE_NPU,
	PCDI_LINE_CPU1,
	PCDI_LINE_CPU2,
	PCDI_LINE_GPU_DUTTON,
	PCDI_LINE_QUELCORE0,
	PCDI_LINE_QUELCORE1,
	PCDI_LINE_QUELCORE2,
	PCDI_LINE_QUELCORE3,
	PCDI_LINE_ELVEES,
	PCDI_LINE_SATA,
	PCDI_LINE_USB0,
	PCDI_LINE_USB1,
	PCDI_LINE_PCIE,
	PCDI_LINE_VXD_ELBAITE0,
	PCDI_LINE_VXD_ELBAITE1,
	PCDI_LINE_VXE_JASPER,
	PCDI_LINE_VXE_TRIDYMITE0,
	PCDI_LINE_VXE_TRIDYMITE1,
	PCDI_LINE_CHIMERA,
	PCDI_LINE_ISP_EAGLE,
	PCDI_LINE_PERIPH_B,
	PCDI_LINE_PERIPH_C,
	PCDI_LINE_DEBUG,
	PCDI_LINE_CPU0,
	PDCI_LINE_MAX
};

struct pll_settings {
	uint16_t tck;
	uint16_t fbdiv;
	uint32_t frac;
	uint8_t postdiv1;
};

static const struct pll_settings pll_settings[] = {
	{ DRAM_TCK_1333, 41, 10480001, 5 }, /* 1333 MT/s */
	{ DRAM_TCK_1600, 59, 16770001, 6 }, /* 1600 MT/s */
	{ DRAM_TCK_1866, 23, 5030001, 2 }, /* 1866 MT/s */
	{ DRAM_TCK_2133, 53, 5030001, 4 }, /* 2133 MT/s */
	{ DRAM_TCK_2400, 104, 16770001, 7 }, /* 2400 MT/s */
	{ DRAM_TCK_2666, 83, 4190001, 5 }, /* 2666 MT/s */
	{ DRAM_TCK_2933, 128, 4610001, 7 }, /* 2933 MT/s */
	{ DRAM_TCK_3200, 119, 16770001, 6 }, /* 3200 MT/s */
};

int power_up(int ctrl_id)
{
	return 0;
}

static void pdci_reset_deassert(enum pdci_reset_lines line, int reset_level)
{
	/* Bit 0 of PMU_PDCI_RESETN_REQ register is for primary reset.
	 * Bits 1 and above represent the subsystem's reset stages.
	 */
	uint32_t pdci_reset_stage = reset_level + 1;

	/* Create mask with pdci_reset_stage number of bits set to '1' */
	uint32_t mask = BIT(pdci_reset_stage + 1) - 1;

	write32(PMU_PDCI_RESETN_REQ(line), mask);

	while (read32(PMU_PDCI_RESETN_ACK(line)) != mask)
		continue;
}

void reset_ctl(int ctrl_id, enum reset_type reset, enum reset_action action)
{
	/* Support only deassert */
	if (action == RESET_ASSERT)
		return;

	pdci_reset_deassert(ctrl_id + 4, (reset == PRESETN) ? 1 : 3);

	/* Deassert soft reset */
	if (reset == CORERESETN)
		write32(DDRSUBS_REGBANK_SYSTEM_CTRL(ctrl_id), 0);
}

static void pll_cfg(enum ddr_ucg_id ucg_id, uint16_t fbdiv, uint32_t frac, uint8_t postdiv1)
{
	uint32_t val;

	/* Deassert soft reset */
	write32(UCG_UFG_REG1(ucg_id, 0), 0);

	/* Assert power down */
	val = read32(UCG_UFG_REG0(ucg_id, 0));
	val |= FIELD_PREP(UCG_UFG_REG0_PLL_PD, 1) | FIELD_PREP(UCG_UFG_REG0_VCO_PD, 1) |
	       FIELD_PREP(UCG_UFG_REG0_OP_PD, 1) | FIELD_PREP(UCG_UFG_REG0_PH_PD, 1);
	write32(UCG_UFG_REG0(ucg_id, 0), val);

	/* Enable Bypass */
	write32(UCG_UFG_REG5(ucg_id, 0), FIELD_PREP(UCG_UFG_REG5_BYPASS, 1));

	/* Enable fractional mode */
	val = read32(UCG_UFG_REG0(ucg_id, 0));
	val &= ~UCG_UFG_REG0_DOC_PD & ~UCG_UFG_REG0_DSM_PD;
	write32(UCG_UFG_REG0(ucg_id, 0), val);

	/* Set REFDIV */
	write32(UCG_UFG_REG2(ucg_id, 0), 1);

	/* Set POSTDIV1 and POSTDIV2 */
	val = read32(UCG_UFG_REG10(ucg_id, 0));
	val &= ~UCG_UFG_REG10_POSTDIV1 & ~UCG_UFG_REG10_POSTDIV2;
	val |= FIELD_PREP(UCG_UFG_REG10_POSTDIV1, postdiv1) | FIELD_PREP(UCG_UFG_REG10_POSTDIV2, 1);
	write32(UCG_UFG_REG10(ucg_id, 0), val);

	/* Set FBDIV */
	val = read32(UCG_UFG_REG3(ucg_id, 0));
	val &= ~UCG_UFG_REG3_FBDIV;
	val |= FIELD_PREP(UCG_UFG_REG3_FBDIV, fbdiv);
	write32(UCG_UFG_REG3(ucg_id, 0), val);

	/* Set FRAC */
	write32(UCG_UFG_REG4(ucg_id, 0), frac);

	/* Propogate changes */
	write32(UCG_UFG_REG6(ucg_id, 0), FIELD_PREP(UCG_UFG_REG6_UPDATE, 1));

	/* Disable bypass */
	val = read32(UCG_UFG_REG5(ucg_id, 0));
	val &= ~UCG_UFG_REG5_BYPASS;
	write32(UCG_UFG_REG5(ucg_id, 0), val);

	/* Deassert power down */
	val = read32(UCG_UFG_REG0(ucg_id, 0));
	val &= ~UCG_UFG_REG0_PLL_PD & ~UCG_UFG_REG0_OP_PD;
	write32(UCG_UFG_REG0(ucg_id, 0), val);

	/* Wait for locking */
	while (!(read32(UCG_UFG_REG0(ucg_id, 0)) & UCG_UFG_REG0_LOCKSTAT))
		continue;
}

int clocks_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	static uint8_t ucg_west_configured, ucg_east_configured;
	int ucg_id = (ctrl_id > 1) ? DDR_UCG_ID_EAST : DDR_UCG_ID_WEST;
	uint32_t val;
	int chan;

	if ((ctrl_id > 1 && !ucg_east_configured) || (ctrl_id <= 1 && !ucg_west_configured)) {
		uint16_t fbdiv = 0;
		uint32_t frac = 0;
		uint8_t postdiv1 = 0;
		int i;

		for (i = 0; i < ARRAY_SIZE(pll_settings); i++) {
			if (cfg->tck != pll_settings[i].tck)
				continue;
			fbdiv = pll_settings[i].fbdiv;
			frac = pll_settings[i].frac;
			postdiv1 = pll_settings[i].postdiv1;
		}

		if (fbdiv == 0 && frac == 0 && postdiv1 == 0)
			return -ECLOCKCFG;

		/* Use only PLL0 for all clocks */
		pll_cfg(ucg_id, fbdiv, frac, postdiv1);

		/* Set dividers for NOC clocks */
		write32(UCG_FIRSTDIV(ucg_id, 0), FIELD_PREP(UCG_FIRSTDIV_NDIV, 0));
		write32(UCG_FIRSTDIV(ucg_id, 1), FIELD_PREP(UCG_FIRSTDIV_NDIV, 3));
		write32(UCG_FIRSTDIV(ucg_id, 2), FIELD_PREP(UCG_FIRSTDIV_NDIV, 1));
		write32(UCG_FIRSTDIV(ucg_id, 3), FIELD_PREP(UCG_FIRSTDIV_NDIV, 3));

		/* Enable NOC clocks */
		val = FIELD_PREP(UCG_XBAR_SEL, 5) | FIELD_PREP(UCG_XBAR_ENABLE, 1);
		for (i = 0; i < 4; i++)
			write32(UCG_XBAR(ucg_id, i), val);
	}

	/* Set divider for DDR_CTRL (APB) clock and enable it */
	chan = 19 - (ctrl_id % 2) * 14;
	write32(UCG_FIRSTDIV(ucg_id, chan), FIELD_PREP(UCG_FIRSTDIV_NDIV, 3));
	val = FIELD_PREP(UCG_XBAR_SEL, 5) | FIELD_PREP(UCG_XBAR_ENABLE, 1);
	write32(UCG_XBAR(ucg_id, chan), val);

	/* Enable DDR CORE clock */
	chan = 18 - (ctrl_id % 2) * 14;
	write32(UCG_XBAR(ucg_id, chan), val);

	if (ucg_id == DDR_UCG_ID_WEST)
		ucg_west_configured = 1;
	else
		ucg_east_configured = 1;

	return 0;
}

/* Switch MFIO PADs to UART0 mode */
static void uart0_pads_cfg(void)
{
	/* TBD: Verify PADs voltage */
	write32(MFIO_NX_FUNCTION_CTRL(46), 0xfa);
	write32(MFIO_CR_GPION_BIT_EN(2), 0x40000000);

	write32(MFIO_NX_FUNCTION_CTRL(47), 0xfa);
	write32(MFIO_CR_GPION_BIT_EN(2), 0x80000000);

	write32(MFIO_NX_FUNCTION_CTRL(48), 0xfa);
	write32(MFIO_CR_GPION_BIT_EN(3), 0x10000);

	write32(MFIO_NX_FUNCTION_CTRL(49), 0xfa);
	write32(MFIO_CR_GPION_BIT_EN(3), 0x20000);
}

void platform_uart_cfg(void)
{
	pdci_reset_deassert(PCDI_LINE_PERIPH_A, 2);
	uart0_pads_cfg();
}

void platform_i2c_cfg(void)
{
	/* TBD */
}

#ifndef CONFIG_SPD_EEPROM
int platform_ddrcfg_get(int ctrl_id, struct ddr_cfg *cfg)
{
	strcpy(cfg->mpart, "KHX3200C16D416GX");

	/* DIMM parameters */
	cfg->ranks = 2;
	cfg->die_size = (133 & 0xf) + 28;
	cfg->rank_size = 1ULL << 33;
	cfg->full_size = cfg->ranks * cfg->rank_size;
	cfg->primary_sdram_width = 1 << (3 + (3 & 0x7)); // 64
	cfg->ecc_sdram_width = 0;
	cfg->full_sdram_width = cfg->primary_sdram_width + cfg->ecc_sdram_width;
	cfg->registered_dimm = 0;
	cfg->mirrored_dimm = 0;
	cfg->package_3ds = 0;
	cfg->device_width = 8;

	/* SDRAM parameters */
	cfg->row_addr_bits = 16;
	cfg->col_addr_bits = 10;
	cfg->bank_addr_bits = 2;
	cfg->bank_group_bits = 2;

	/* DIMM timing parameters */
	cfg->tckmin = DRAM_TCK_2400;
	cfg->tckmax = DRAM_TCK_1250;
	cfg->tck = CONFIG_DRAM_TCK;
	cfg->taa = ps2clk_jedec(13750, cfg->tck);
	cfg->tfaw = ps2clk_jedec(21000, cfg->tck);
	cfg->trcd = ps2clk_jedec(13750, cfg->tck);
	cfg->trp = ps2clk_jedec(13750, cfg->tck);
	cfg->trasmin = ps2clk_jedec(32000, cfg->tck);
	cfg->trfc1 = ps2clk_jedec(350000, cfg->tck);
	cfg->trfc2 = ps2clk_jedec(260000, cfg->tck);
	cfg->trfc4 = ps2clk_jedec(160000, cfg->tck);
	cfg->trrds = ps2clk_jedec(3300, cfg->tck);
	cfg->trrdl = ps2clk_jedec(4900, cfg->tck);
	cfg->tccdl = ps2clk_jedec(5000, cfg->tck);
	cfg->trc = ps2clk_jedec(45750, cfg->tck);

	return 0;
}
#endif
