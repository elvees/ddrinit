// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2020 RnD Center "ELVEES", JSC

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

static unsigned long i2c_base_addr[] = {
	0xffffffffbcc60000, 0xffffffffbcc70000, 0xffffffffbd060000,
	0xffffffffbd070000, 0xffffffffbf970000,
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

int platform_power_up(void)
{
	return 0;
}

static int pdci_reset_deassert(enum pdci_reset_lines line, int reset_level)
{
	/* Bit 0 of PMU_PDCI_RESETN_REQ register is for primary reset.
	 * Bits 1 and above represent the subsystem's reset stages.
	 */
	uint32_t pdci_reset_stage = reset_level + 1, val = 0;

	/* Create mask with pdci_reset_stage number of bits set to '1' */
	uint32_t mask = BIT(pdci_reset_stage + 1) - 1;

	write32(PMU_PDCI_RESETN_REQ(line), mask);

	return read32_poll_timeout(val, val == mask, USEC, MSEC, PMU_PDCI_RESETN_ACK(line));
}

int platform_reset_ctl(int ctrl_id, enum reset_type reset, enum reset_action action)
{
	int ret;
	/* Support only deassert */
	if (action == RESET_ASSERT)
		return 0;

	ret = pdci_reset_deassert(ctrl_id + 4, (reset == PRESETN) ? 1 : 3);
	if (ret)
		return ret;

	/* Deassert soft reset for DDRMC and PHY */
	if (reset == CORERESETN) {
		write32(DDRSUBS_REGBANK_SYSTEM_CTRL(ctrl_id), 4);
		delay_usec(100);
		write32(DDRSUBS_REGBANK_SYSTEM_CTRL(ctrl_id), 6);
		delay_usec(100);
		write32(DDRSUBS_REGBANK_SYSTEM_CTRL(ctrl_id), 2);
	}

	return 0;
}

void platform_post_reset_deassert(int ctrl_id)
{
}

static int pll_cfg(enum ddr_ucg_id ucg_id, int pll_id, uint32_t freq, uint32_t xtal, uint32_t div)
{
	int ret;
	uint16_t fbdiv;
	uint32_t frac, val;

	fbdiv = freq / xtal;
	frac = (freq % xtal) * 16777216ULL / xtal;

	print_dbg("PLL%d: XTAL: %d, PLL VCO/2: %d, PLL FOUTPOSTDIV: %d\n", pll_id, xtal, freq,
		  freq / div);
	print_dbg("PLL%d: fbdiv: %d, frac: %d\n", pll_id, fbdiv, frac);

	/* Enable Bypass */
	write32(UCG_UFG_REG5(ucg_id, pll_id), UCG_UFG_REG5_BYPASS);

	/* Assert power down */
	write32(UCG_UFG_REG0(ucg_id, pll_id), UCG_UFG_REG0_PLL_PD);
	print_dbg("PLL%d: PLL PD asserted\n", pll_id);

	/* Deassert soft reset */
	write32(UCG_UFG_REG1(ucg_id, pll_id), 0);

	/* Set REFDIV */
	write32(UCG_UFG_REG2(ucg_id, pll_id), 1);

	/* Set POSTDIV1 and POSTDIV2 */
	val = read32(UCG_UFG_REG10(ucg_id, pll_id));
	val &= ~UCG_UFG_REG10_POSTDIV1 & ~UCG_UFG_REG10_POSTDIV2;
	val |= FIELD_PREP(UCG_UFG_REG10_POSTDIV1, div) | FIELD_PREP(UCG_UFG_REG10_POSTDIV2, 1);
	write32(UCG_UFG_REG10(ucg_id, pll_id), val);

	/* Set FBDIV */
	val = read32(UCG_UFG_REG3(ucg_id, pll_id));
	val &= ~UCG_UFG_REG3_FBDIV;
	val |= FIELD_PREP(UCG_UFG_REG3_FBDIV, fbdiv);
	write32(UCG_UFG_REG3(ucg_id, pll_id), val);

	/* Set FRAC */
	write32(UCG_UFG_REG4(ucg_id, pll_id), frac);

	/* Propagate changes */
	write32(UCG_UFG_REG6(ucg_id, pll_id), UCG_UFG_REG6_UPDATE);

	/* Deassert power down */
	write32(UCG_UFG_REG0(ucg_id, pll_id), 0);

	/* Disable bypass */
	write32(UCG_UFG_REG5(ucg_id, pll_id), 0);

	/* Wait for locking */

	ret = read32_poll_timeout(val, val & UCG_UFG_REG0_LOCKSTAT, USEC, MSEC,
				  UCG_UFG_REG0(ucg_id, pll_id));
	if (ret)
		return ret;

	print_dbg("PLL%d: UFG0 REGS 0, 3, 5: 0x%x, 0x%x, 0x%x\n", pll_id,
		  read32(UCG_UFG_REG0(ucg_id, pll_id)), read32(UCG_UFG_REG3(ucg_id, pll_id)),
		  read32(UCG_UFG_REG5(ucg_id, pll_id)));

	return 0;
}

int platform_clk_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	static uint8_t ucg_west_configured, ucg_east_configured;
	int ucg_id = (ctrl_id > 1) ? DDR_UCG_ID_EAST : DDR_UCG_ID_WEST;
	uint32_t val;
	int chan;

	if ((ctrl_id > 1 && !ucg_east_configured) || (ctrl_id <= 1 && !ucg_west_configured)) {
		struct ddrcfg_override *cfg_o = (struct ddrcfg_override *)&ddrcfg_override_start;
		uint32_t div, xtal = CONFIG_DDR_XTAL_FREQ;
		int i, ret;

		if (cfg_o->magic == DDRCFG_OVERRIDE_MAGIC)
			xtal = cfg_o->xtal_freq;

		if (cfg->tck <= DRAM_TCK_1066 && cfg->tck > DRAM_TCK_1600)
			div = 5;
		else if (cfg->tck <= DRAM_TCK_1600 && cfg->tck > DRAM_TCK_2133)
			div = 4;
		else if (cfg->tck <= DRAM_TCK_2133 && cfg->tck > DRAM_TCK_2666)
			div = 3;
		else if (cfg->tck <= DRAM_TCK_2666 && cfg->tck >= DRAM_TCK_3200)
			div = 2;
		else
			return -ECLOCKCFG;

		ret = pll_cfg(ucg_id, 0, tck2freq(cfg->tck) / 2 * div, xtal, div);
		if (ret)
			return ret;

		ret = pll_cfg(ucg_id, 1, CONFIG_AXI_BUS_FREQUENCY * 2, xtal, 2);
		if (ret)
			return ret;

		/* Set dividers for NOC clocks */
		write32(UCG_FIRSTDIV(ucg_id, 0), FIELD_PREP(UCG_FIRSTDIV_NDIV, 0));
		write32(UCG_FIRSTDIV(ucg_id, 1), FIELD_PREP(UCG_FIRSTDIV_NDIV, 3));
		write32(UCG_FIRSTDIV(ucg_id, 2), FIELD_PREP(UCG_FIRSTDIV_NDIV, 1));
		write32(UCG_FIRSTDIV(ucg_id, 3), FIELD_PREP(UCG_FIRSTDIV_NDIV, 3));

		/* Enable NOC clocks */
		val = FIELD_PREP(UCG_XBAR_SEL, 15) | FIELD_PREP(UCG_XBAR_ENABLE, 1);
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

#ifdef CONFIG_I2C
/* Switch MFIO PADs to I2C mode */
static void i2c_pads_cfg(int i2c_ctrl_id)
{
	switch (i2c_ctrl_id) {
	case 0:
		write32(MFIO_NX_FUNCTION_CTRL(54), 0xfa);
		write32(MFIO_CR_GPION_BIT_EN(3), 0x400000);

		write32(MFIO_NX_FUNCTION_CTRL(55), 0xfa);
		write32(MFIO_CR_GPION_BIT_EN(3), 0x800000);
		break;
	case 1:
		write32(MFIO_NX_FUNCTION_CTRL(56), 0xfa);
		write32(MFIO_CR_GPION_BIT_EN(3), 0x1000000);

		write32(MFIO_NX_FUNCTION_CTRL(57), 0xfa);
		write32(MFIO_CR_GPION_BIT_EN(3), 0x2000000);
		break;
	case 2:
		write32(MFIO_SX_FUNCTION_CTRL(8), 0xfa);
		write32(MFIO_CR_GPIOS_BIT_EN(0), 0x1000000);

		write32(MFIO_SX_FUNCTION_CTRL(9), 0xfa);
		write32(MFIO_CR_GPIOS_BIT_EN(0), 0x2000000);
		break;
	case 3:
		write32(MFIO_SX_FUNCTION_CTRL(10), 0xfa);
		write32(MFIO_CR_GPIOS_BIT_EN(0), 0x4000000);

		write32(MFIO_SX_FUNCTION_CTRL(11), 0xfa);
		write32(MFIO_CR_GPIOS_BIT_EN(0), 0x8000000);
		break;
	case 4:
		write32(MFIO_NX_FUNCTION_CTRL(35), 0xfa);
		write32(MFIO_CR_GPION_BIT_EN(2), 0x80000);

		write32(MFIO_NX_FUNCTION_CTRL(36), 0xfa);
		write32(MFIO_CR_GPION_BIT_EN(2), 0x100000);
		break;
	}
}
#endif

int platform_uart_cfg(void)
{
	int ret;

	ret = pdci_reset_deassert(PCDI_LINE_PERIPH_A, 2);
	if (ret)
		return -EUARTCFG;

	uart0_pads_cfg();

	return 0;
}

unsigned long platform_i2c_base_get(int ctrl_id)
{
	if (ctrl_id < 0 || ctrl_id >= ARRAY_SIZE(i2c_base_addr))
		return 0;

	return i2c_base_addr[ctrl_id]; // cppcheck-suppress arrayIndexOutOfBoundsCond
}

#ifdef CONFIG_I2C
int platform_i2c_cfg(int ctrl_id, uint32_t *clk_rate)
{
	int ret;

	ret = pdci_reset_deassert(PCDI_LINE_PERIPH_B, 2);
	if (ret)
		return -EI2CCFG;

	i2c_pads_cfg(ctrl_id);

	*clk_rate = CONFIG_I2C_FREQ;

	return 0;
}
#endif

#ifdef CONFIG_INTERLEAVING
static void interleaving_enable(int init_mask, struct sysinfo *info)
{
	uint32_t hash_func[4] = { 0, 0, 0, 0 };

	/* Enable memory interleaving only if all 4 DDRMC have been initialized */
	if ((init_mask & 0xf) != 0xf)
		return;

	switch (CONFIG_INTERLEAVING_SIZE) {
	case 0:
		hash_func[2] = 0x400;
		hash_func[0] = 0x800;
		info->interleaving.size = 1024;
		break;
	case 1:
		hash_func[2] = 0x800;
		hash_func[0] = 0x1000;
		info->interleaving.size = 2048;
		break;
	case 2:
		hash_func[2] = 0x2000;
		hash_func[0] = 0x1000;
		info->interleaving.size = 4096;
		break;
	default:
		return;
	}

	info->interleaving.enable = true;
	info->interleaving.channels = 4;

	uint32_t tmp =
		FIELD_PREP(DDRSUBS_REGBANK_SOC_INTERLEAVE_BOUNDARY, CONFIG_INTERLEAVING_SIZE) |
		FIELD_PREP(DDRSUBS_REGBANK_SOC_INTERLEAVE_ENABLE, 1);

	for (int i = 0; i < CONFIG_DDRMC_MAX_NUMBER; i++)
		write32(DDRSUBS_REGBANK_SOC_INTERLEAVE(i), tmp);

	uint64_t addrs[] = { CPU_HASH_FUNC_HASH_0(0),
			     CPU_HASH_FUNC_HASH_0(1),
			     CPU_HASH_FUNC_HASH_0(2),
			     VXE_HASH_FUNC_HASH_1(0),
			     VXE_HASH_FUNC_HASH_1(1),
			     VXE_HASH_FUNC_HASH_1(2),
			     VXD_HASH_FUNC_HASH_1(0),
			     VXD_HASH_FUNC_HASH_1(1),
			     VIDEO_IN_HASH_FUNC_HASH_1,
			     VIDEO_OUT_HASH_FUNC_HASH_1,
			     GPU_HASH_FUNC_HASH_1(0),
			     GPU_HASH_FUNC_HASH_1(1),
			     VELCORE_Q0_Q1_HASH_FUNC_HASH_1(0),
			     VELCORE_Q0_Q1_HASH_FUNC_HASH_1(1),
			     VELCORE_Q2_Q3_HASH_FUNC_HASH_1(0),
			     VELCORE_Q2_Q3_HASH_FUNC_HASH_1(1),
			     PCIE_MSTR_HASH_FUNC_HASH_1(0),
			     PCIE_MSTR_HASH_FUNC_HASH_1(1),
			     PCIE_MSTR_HASH_FUNC_HASH_1(2),
			     PCIE_MSTR_HASH_FUNC_HASH_1(3),
			     SATA_HASH_FUNC_HASH_1,
			     NPU_HASH_FUNC_HASH_1,
			     USB_HASH_FUNC_HASH_1(0),
			     USB_HASH_FUNC_HASH_1(1),
			     STARTUP_HASH_FUNC_HASH_1,
			     PERIPH_A_HASH_FUNC_HASH_1,
			     PERIPH_B_HASH_FUNC_HASH_1,
			     DEBUG_HASH_FUNC_HASH_1,
			     ELVEES_HASH_FUNC_HASH_1 };

	for (int i = 0; i < ARRAY_SIZE(addrs); i++)
		for (int j = 0; j < 4; j++)
			write32(addrs[i] + j * 4, hash_func[j]);
}
#else
static void interleaving_enable(int init_mask, struct sysinfo *info)
{
	return;
}
#endif

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

	/* Set dynamic allocation policy for CPU0, CPU1, CPU2, GPU, ELVEES Periph and
	 * VELcore Q0 - Q3. Allocation policy for all other initiators is set as static,
	 * disabled.
	 */
	for (i = 0; i < CONFIG_DDRMC_MAX_NUMBER; i++) {
		if (BIT(i) & init_mask) {
			write32(NOC_AGENT_LLC_X_0_LLC_ALLOC_ARCACHE_EN(i), 0x7);
			write32(NOC_AGENT_LLC_X_0_LLC_ALLOC_AWCACHE_EN(i), 0x7);
			write32(NOC_AGENT_LLC_X_0_LLC_ALLOC_RD_EN(i), 0);
			write32(NOC_AGENT_LLC_X_0_LLC_ALLOC_WR_EN(i), 0);
			write32(NOC_AGENT_LLC_X_0_LLC_CACHE_WAY_ENABLE(i), 0xFFFFFFFF);
		}
	}

	for (i = 2; i < CONFIG_DDRMC_MAX_NUMBER; i++) {
		if ((BIT(i) & init_mask) || !IS_ENABLED(CONFIG_LLC_AS_RAM))
			continue;

		write32(NOC_AGENT_LLC_X_0_LLC_RAM_ADDRESS_BASE(i), LLC_RAM_BASE_ADDR(i));
		write32(NOC_AGENT_LLC_X_0_LLC_GLOBAL_ALLOC(i), 0x0);
		write32(NOC_AGENT_LLC_X_0_LLC_CACHE_WAY_ENABLE(i), 0x0);
		write32(NOC_AGENT_LLC_X_0_LLC_RAM_WAY_SECURE(i), 0x0);
		write32(NOC_AGENT_LLC_X_0_LLC_RAM_WAY_ENABLE(i), 0xFFFFFFFF);
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
__attribute__((unused)) static int south_ultrasoc_disable(void)
{
	int i, ret;
	uint32_t data0[] = { 0x1452108, 0x1452308, 0x1452f08, 0x1453308,
			     0x5453308, 0x1453508, 0x3453708 };
	uint32_t data1[] = { 0x3452108, 0x3452308, 0x3452f08, 0x3453308,
			     0x7453308, 0x1453708, 0x1453b08 };
	uint32_t val = 0;

	write32(AXI_COMM_US_CTL, 1);
	write32(AXI_COMM_DS_CTL, 1);

	ret = read32_poll_timeout(val, val & 0x2, USEC, 5 * MSEC, AXI_COMM_US_BUF_STS);
	if (ret)
		return ret;

	for (i = 0; i < ARRAY_SIZE(data0); i++) {
		write32(AXI_COMM_US_DATA, data0[i]);
		write32(AXI_COMM_US_DATA, 0);
		write32(AXI_COMM_US_DATA, 0x700);
		write32(AXI_COMM_US_DATA, data1[i]);
		write32(AXI_COMM_US_DATA, 0);
		write32(AXI_COMM_US_DATA, 0x700);
	}

	return 0;
}

static void mem_regions_set(int init_mask, struct sysinfo *info)
{
	uint64_t *dsize = &info->dram_size[0];

	if (dsize[0] > CONFIG_MEM_REGION1_SIZE + CONFIG_MEM_REGION0_SIZE) {
		dsize[0] = CONFIG_MEM_REGION1_SIZE + CONFIG_MEM_REGION0_SIZE;
		printf("DDRMC0: Memory size truncated to %lu MiB\n", dsize[0] / 1024 / 1024);
	}
	/* Assume that DDR3/DDR4 DIMM size can't be less than 1 GiB, so 2 first regions
	 * will always be used.
	 */
	info->mem_regions[0].start = CONFIG_MEM_REGION0_START;
	info->mem_regions[0].size = CONFIG_MEM_REGION0_SIZE;
	info->mem_regions[1].start = CONFIG_MEM_REGION1_START;
	info->mem_regions[1].size = dsize[0] - CONFIG_MEM_REGION0_SIZE;
	int free_region_idx = 2;
	for (int i = 1; i < CONFIG_DDRMC_MAX_NUMBER; i++) {
		uint64_t cfg_size[] = { CONFIG_MEM_REGION2_SIZE, CONFIG_MEM_REGION3_SIZE,
					CONFIG_MEM_REGION4_SIZE };
		unsigned long cfg_start[] = { CONFIG_MEM_REGION2_START, CONFIG_MEM_REGION3_START,
					      CONFIG_MEM_REGION4_START };
		if (!(init_mask & BIT(i)))
			continue;
		if (dsize[i] > cfg_size[i - 1]) {
			dsize[i] = cfg_size[i - 1];
			printf("DDRMC%d: Memory size truncated to %lu MiB\n", i,
			       dsize[i] / 1024 / 1024);
		}
		if (info->interleaving.enable) {
			info->mem_regions[1].size += dsize[i];
		} else {
			info->mem_regions[free_region_idx].start = cfg_start[i - 1];
			info->mem_regions[free_region_idx].size = dsize[i];
		}
		free_region_idx++;
	}
}

int platform_system_init(int init_mask, struct sysinfo *info)
{
	interleaving_enable(init_mask, info);
	llc_enable(init_mask);
	axprot_override();
	iommu_bypass_enable();
	mem_regions_set(init_mask, info);

	/* TBD: UltraSoC disabling hangs */
	/* south_ultrasoc_disable(); */

	return 0;
}

uint32_t platform_get_timer_count()
{
	uint32_t res;
	__asm__ __volatile__("mfc0 %0, $9" : "=r"(res));
	return res;
}
