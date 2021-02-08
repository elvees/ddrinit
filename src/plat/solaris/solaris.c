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

void phy_write32(int ctrl_id, unsigned long addr, uint32_t val)
{
	if (addr >= 0x200000) {
		write32(DDRSUBS_REGBANK_PHY_PAGE_ADDR(ctrl_id), 1);
		addr -= 0x200000;
	} else {
		write32(DDRSUBS_REGBANK_PHY_PAGE_ADDR(ctrl_id), 0);
	}

	write32(PHY_BASE(ctrl_id) + addr, val);
}

void phy_write16(int ctrl_id, unsigned long addr, uint16_t val)
{
	if (addr >= 0x200000) {
		write32(DDRSUBS_REGBANK_PHY_PAGE_ADDR(ctrl_id), 1);
		addr -= 0x200000;
	} else {
		write32(DDRSUBS_REGBANK_PHY_PAGE_ADDR(ctrl_id), 0);
	}

	write16(PHY_BASE(ctrl_id) + addr, val);
}

uint32_t phy_read32(int ctrl_id, unsigned long addr)
{
	if (addr >= 0x200000) {
		write32(DDRSUBS_REGBANK_PHY_PAGE_ADDR(ctrl_id), 1);
		addr -= 0x200000;
	} else {
		write32(DDRSUBS_REGBANK_PHY_PAGE_ADDR(ctrl_id), 0);
	}

	return read32(PHY_BASE(ctrl_id) + addr);
}

int platform_power_up(int ctrl_id)
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

void platform_reset_ctl(int ctrl_id, enum reset_type reset, enum reset_action action)
{
	/* Support only deassert */
	if (action == RESET_ASSERT)
		return;

	pdci_reset_deassert(ctrl_id + 4, (reset == PRESETN) ? 1 : 3);

	/* Deassert soft reset for DDRMC and PHY */
	if (reset == CORERESETN) {
		write32(DDRSUBS_REGBANK_SYSTEM_CTRL(ctrl_id), 4);
		delay_usec(100);
		write32(DDRSUBS_REGBANK_SYSTEM_CTRL(ctrl_id), 6);
		delay_usec(100);
		write32(DDRSUBS_REGBANK_SYSTEM_CTRL(ctrl_id), 2);
	}
}

static int pll_cfg(enum ddr_ucg_id ucg_id, int tck)
{
	uint32_t freq, xtal, frac, val, div1;
	uint16_t fbdiv;
	struct ddrcfg_override *cfg_o = (struct ddrcfg_override *)&ddrcfg_override_start;

	if (tck <= DRAM_TCK_1250 && tck > DRAM_TCK_1600)
		div1 = 5;
	else if (tck <= DRAM_TCK_1600 && tck > DRAM_TCK_2133)
		div1 = 4;
	else if (tck <= DRAM_TCK_2133 && tck >= DRAM_TCK_2400)
		div1 = 3;
	else if (tck <= DRAM_TCK_2666 && tck >= DRAM_TCK_3200)
		div1 = 2;
	else
		return -ECLOCKCFG;

	freq = tck2freq(tck) / 2 * div1;

	if (cfg_o->magic == DDRCFG_OVERRIDE_MAGIC)
		xtal = cfg_o->xtal_freq;
	else
		xtal = CONFIG_DDR_XTAL_FREQ;

	fbdiv = freq / xtal;
	frac = (freq % xtal) * 16777216ULL / xtal;

	print_dbg("pll_cfg(): XTAL: %d, PLL VCO/2: %d, PLL FOUTPOSTDIV: %d\n", xtal, freq,
		  freq / div1);
	print_dbg("pll_cfg(): fbdiv: %d, frac: %d\n", fbdiv, frac);

	/* Enable Bypass */
	write32(UCG_UFG_REG5(ucg_id, 0), UCG_UFG_REG5_BYPASS);

	/* Assert power down */
	write32(UCG_UFG_REG0(ucg_id, 0), UCG_UFG_REG0_PLL_PD);
	print_dbg("pll_cfg(): PLL PD asserted\n");

	/* Deassert soft reset */
	write32(UCG_UFG_REG1(ucg_id, 0), 0);

	/* Set REFDIV */
	write32(UCG_UFG_REG2(ucg_id, 0), 1);

	/* Set POSTDIV1 and POSTDIV2 */
	val = read32(UCG_UFG_REG10(ucg_id, 0));
	val &= ~UCG_UFG_REG10_POSTDIV1 & ~UCG_UFG_REG10_POSTDIV2;
	val |= FIELD_PREP(UCG_UFG_REG10_POSTDIV1, div1) |
	       FIELD_PREP(UCG_UFG_REG10_POSTDIV2, 1);
	write32(UCG_UFG_REG10(ucg_id, 0), val);

	/* Set FBDIV */
	val = read32(UCG_UFG_REG3(ucg_id, 0));
	val &= ~UCG_UFG_REG3_FBDIV;
	val |= FIELD_PREP(UCG_UFG_REG3_FBDIV, fbdiv);
	write32(UCG_UFG_REG3(ucg_id, 0), val);

	/* Set FRAC */
	write32(UCG_UFG_REG4(ucg_id, 0), frac);

	/* Propogate changes */
	write32(UCG_UFG_REG6(ucg_id, 0), UCG_UFG_REG6_UPDATE);

	/* Deassert power down */
	write32(UCG_UFG_REG0(ucg_id, 0), 0);

	/* Disable bypass */
	write32(UCG_UFG_REG5(ucg_id, 0), 0);

	/* Wait for locking */
	while (!(read32(UCG_UFG_REG0(ucg_id, 0)) & UCG_UFG_REG0_LOCKSTAT))
		continue;

	print_dbg("pll_cfg(): UFG0 REGS 0, 3, 5: 0x%x, 0x%x, 0x%x\n",
		  read32(UCG_UFG_REG0(ucg_id, 0)), read32(UCG_UFG_REG3(ucg_id, 0)),
		  read32(UCG_UFG_REG5(ucg_id, 0)));

	return 0;
}

int platform_clk_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	static uint8_t ucg_west_configured, ucg_east_configured;
	int ucg_id = (ctrl_id > 1) ? DDR_UCG_ID_EAST : DDR_UCG_ID_WEST;
	uint32_t val;
	int chan;

	if ((ctrl_id > 1 && !ucg_east_configured) || (ctrl_id <= 1 && !ucg_west_configured)) {
		int i, ret;

		/* Use only PLL0 for all clocks */
		ret = pll_cfg(ucg_id, cfg->tck);
		if (ret)
			return ret;

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
	int i;

	for (i = 0; i < 4; i++)
		write32(MFIO_NX_FUNCTION_CTRL(i + 46), 0xfa);

	write32(MFIO_CR_GPION_BIT_EN(2), 0xc0000000);
	write32(MFIO_CR_GPION_BIT_EN(3), 0x30000);
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

static void llc_enable(int init_mask)
{
	int i;

	/* Invalidate LLC tag RAM */
	for (i = 0; i < CONFIG_DDRMC_MAX_NUMBER; i++) {
		if (BIT(i) & init_mask)
			write64(NOC_AGENT_LLC_X_0_LLC_TAG_INV_CTL(i), 0xFFFFFFFFFFFFFFFF);
	}

	/* Wait until TAG_INV_CTL regs read back 0 */
	while (1) {
		uint64_t val = 0;

		for (i = 0; i < CONFIG_DDRMC_MAX_NUMBER; i++) {
			if (BIT(i) & init_mask)
				val |= read64(NOC_AGENT_LLC_X_0_LLC_TAG_INV_CTL(i));
		}

		if (val == 0)
			break;
	}

	/* Set allocation policy to 0xFF and enable LLC */
	for (i = 0; i < CONFIG_DDRMC_MAX_NUMBER; i++) {
		if (BIT(i) & init_mask) {
			write32(NOC_AGENT_LLC_X_0_LLC_ALLOC_ARCACHE_EN(i), 0);
			write32(NOC_AGENT_LLC_X_0_LLC_ALLOC_AWCACHE_EN(i), 0);
			write32(NOC_AGENT_LLC_X_0_LLC_ALLOC_RD_EN(i), 0xFF);
			write32(NOC_AGENT_LLC_X_0_LLC_ALLOC_WR_EN(i), 0xFF);
			write32(NOC_AGENT_LLC_X_0_LLC_CACHE_WAY_ENABLE(i), 0xFFFFFFFF);
		}
	}
}

/*
 * Netspeed NoC in general including LLC considers secure and non-secure traffic
 * physically separated. This is a broken security feature. So we are expected
 * not to rely on AxPROT, and subsystems tied this off. In order to make sure that
 * the following subsystems do not request secure traffic we need to set their
 * memif_init registers correctly.
 */
static void axprot_override(void)
{
	int i;

	write64(NOC_BRIDGE_VIDEO_IN_MEMIF_INIT_AROVRD, 0x2200);
	write64(NOC_BRIDGE_VIDEO_IN_MEMIF_INIT_AWOVRD, 0x2200);
	write64(NOC_BRIDGE_VIDEO_OUT_MEMIF_INIT_AROVRD, 0x2200);
	write64(NOC_BRIDGE_VIDEO_OUT_MEMIF_INIT_AWOVRD, 0x2200);

	for (i = 0; i < 2; i++) {
		write64(NOC_BRIDGE_VXD_MEMIF_INIT_AROVRD(i), 0x2200);
		write64(NOC_BRIDGE_VXD_MEMIF_INIT_AWOVRD(i), 0x2200);
	}

	for (i = 0; i < 3; i++) {
		write64(NOC_BRIDGE_VXE_MEMIF_INIT_AROVRD(i), 0x2200);
		write64(NOC_BRIDGE_VXE_MEMIF_INIT_AWOVRD(i), 0x2200);
	}

	for (i = 0; i < 2; i++) {
		write64(NOC_BRIDGE_GPU_MEMIF_INIT_AROVRD(i), 0x2200);
		write64(NOC_BRIDGE_GPU_MEMIF_INIT_AWOVRD(i), 0x2200);
	}

	write64(NOC_BRIDGE_ELVEES_MEMIF_INIT_AROVRD, 0x2200);
	write64(NOC_BRIDGE_ELVEES_MEMIF_INIT_AWOVRD, 0x2200);
	write64(NOC_BRIDGE_NPU_MEMIF_INIT_AROVRD, 0x2200);
	write64(NOC_BRIDGE_NPU_MEMIF_INIT_AWOVRD, 0x2200);
	write64(NOC_BRIDGE_SATA_MEMIF_INIT_AROVRD, 0x2200);
	write64(NOC_BRIDGE_SATA_MEMIF_INIT_AWOVRD, 0x2200);

	for (i = 0; i < 2; i++) {
		write64(NOC_BRIDGE_USB_MEMIF_INIT_AROVRD(i), 0x2200);
		write64(NOC_BRIDGE_USB_MEMIF_INIT_AWOVRD(i), 0x2200);
	}

	for (i = 0; i < 4; i++) {
		write64(NOC_BRIDGE_PCIE_MEMIF_INIT_AROVRD(i), 0x2200);
		write64(NOC_BRIDGE_PCIE_MEMIF_INIT_AWOVRD(i), 0x2200);
	}

	for (i = 0; i < 2; i++) {
		write64(NOC_BRIDGE_PERIPH_MEMIF_INIT_AROVRD(i), 0x2200);
		write64(NOC_BRIDGE_PERIPH_MEMIF_INIT_AWOVRD(i), 0x2200);
	}

	write64(NOC_BRIDGE_STARTUP_MEMIF_INIT_AROVRD, 0x2200);
	write64(NOC_BRIDGE_STARTUP_MEMIF_INIT_AWOVRD, 0x2200);
}

static void iommu_bypass_enable(void)
{
	int i;
	uint32_t val;
	uint64_t addrs[] = { IOMMU_VIDEO_OUT_GCFG,    IOMMU_PERIPHERAL_B_GCFG,
			     IOMMU_PERIPHERAL_A_GCFG, IOMMU_ELVEES_PERIPHERAL_GCFG,
			     IOMMU_NPU_GCFG,	      IOMMU_SATA_GCFG,
			     IOMMU_USB_GCFG(0),	      IOMMU_USB_GCFG(1),
			     IOMMU_PCIE_GCFG(0),      IOMMU_PCIE_GCFG(1),
			     IOMMU_PCIE_GCFG(2),      IOMMU_PCIE_GCFG(3),
			     IOMMU_VELCOREQ_GCFG(0),  IOMMU_VELCOREQ_GCFG(1),
			     IOMMU_VELCOREQ_GCFG(2),  IOMMU_VELCOREQ_GCFG(3),
			     IOMMU_VIDEO_IN_GCFG,     IOMMU_VXD_GCFG(0),
			     IOMMU_VXD_GCFG(1),	      IOMMU_VXE_GCFG(0),
			     IOMMU_VXE_GCFG(1),	      IOMMU_VXE_GCFG(2) };

	for (i = 0; i < ARRAY_SIZE(addrs); i++) {
		val = read32(addrs[i]);
		val &= ~IOMMU_GCFG_EN & ~IOMMU_GCFG_OCR;
		write32(addrs[i], val);
	}
}

/*
 * Disable the UltraSoC probes in South Partition as timing is not met
 * for the probe flops in the 800MHz NoC clock domain. See SBM88645.
 */
static void south_ultrasoc_disable(void)
{
	int i;
	uint32_t data0[] = { 0x1452108, 0x1452308, 0x1452f08, 0x1453308,
			     0x5453308, 0x1453508, 0x3453708 };
	uint32_t data1[] = { 0x3452108,  0x3452308, 0x3452f08, 0x3453308,
			     0x7453308, 0x1453708, 0x1453b08 };

	write32(AXI_COMM_US_CTL, 1);
	write32(AXI_COMM_DS_CTL, 1);

	while ((read32(AXI_COMM_US_BUF_STS) & 0x2) == 0)
		continue;

	for (i = 0; i < ARRAY_SIZE(data0); i++) {
		write32(AXI_COMM_US_DATA, data0[i]);
		write32(AXI_COMM_US_DATA, 0);
		write32(AXI_COMM_US_DATA, 0x700);
		write32(AXI_COMM_US_DATA, data1[i]);
		write32(AXI_COMM_US_DATA, 0);
		write32(AXI_COMM_US_DATA, 0x700);
	}
}

int platform_system_init(int init_mask)
{
	llc_enable(init_mask);
	axprot_override();
	iommu_bypass_enable();

	/* TBD: UltraSoC disabling hangs */
	/* south_ultrasoc_disable(); */

	return 0;
}

#ifndef CONFIG_SPD_EEPROM
int platform_ddrcfg_get(int ctrl_id, struct ddr_cfg *cfg)
{
	if (!(BIT(ctrl_id) & CONFIG_DDRMC_ACTIVE_MASK))
		return -EDIMMCFG;

	strcpy(cfg->mpart, "KHX3200C16D416GX");

	/* DIMM parameters */
	cfg->ranks = CONFIG_DRAM_RANKS;
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

uint32_t platform_get_timer_count()
{
	uint32_t res;
	__asm__ __volatile__("mfc0 %0, $9"
			     : "=r" (res));
	return res;
}
