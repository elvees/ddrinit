// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#include <string.h>
#include <common.h>
#include <ddrmc.h>
#include <ddrspd.h>
#include <phy.h>
#include <plat/plat.h>
#include <regs.h>
#include <uart.h>

/* Initialize DDRMC, PHY and DRAM as descibed in DWC_UMCTL2 databook:
 * Step 1:  Follow the PHYs power up procedure.
 * Step 2:  Program the DWC_ddr_umctl2 registers.
 * Step 3:  De-assert reset signal core_ddrc_rstn.
 * Step 4:  Disable auto-refreshes, self-refresh, powerdown and
 *          assertion of dfi_dram_clk_disable.
 * Step 5:  Set SWCTL.sw_done to 0.
 * Step 6:  Set DFIMISC.dfi_init_complete_en to 0.
 * Step 7:  Set SWCTL.sw_done to 1.
 * Step 8:  Start PHY initialization and training by accessing relevant
 *          PUB registers.
 * Step 9:  Poll the PUB register APBONLY.UctShadowRegs[0] = 1’b0.
 * Step 10: Read the PUB Register APBONLY.UctWriteOnlyShadow for training
 *          status.
 * Step 11: Write the PUB Register APBONLY.DctWriteProt = 0.
 * Step 12: Poll the PUB register APBONLY.UctShadowRegs[0] = 1’b1.
 * Step 13: Write the PUB Register APBONLY.DctWriteProt = 1.
 * Step 14: Poll the PUB register MASTER.CalBusy = 0.
 * Step 15: Set SWCTL.sw_done to 0.
 * Step 16: Set DFIMISC.dfi_init_start to 1.
 * Step 17: Set SWCTL.sw_done to 1.
 * Step 18: Poll DFISTAT.dfi_init_complete = 1.
 * Step 19: Set SWCTL.sw_done to 0.
 * Step 20: Set DFIMISC.dfi_init_start to 0.
 * Step 21: Update RANKCTL, DRAMTMG2, registers.
 * Step 22: Set DFIMISC.dfi_init_complete_en to 1.
 * Step 23: Set PWRCTL.selfref_sw to 0.
 * Step 24: Set SWCTL.sw_done to 1.
 * Step 25: Wait for DWC_ddr_umctl2 to move to normal operating mode by
 *          monitoring STAT.operating_mode signal.
 * Step 26: Set back registers in step 4 to the original values if desired.
 */
int ddr_init(int ctrl_id, struct ddr_cfg *cfg)
{
	int ret, i;
	uint32_t val;

	/* Step 1 */
	ret = platform_power_up(ctrl_id);
	if (ret)
		return ret;

	platform_reset_ctl(ctrl_id, PRESETN, RESET_ASSERT);
	platform_reset_ctl(ctrl_id, CORERESETN, RESET_ASSERT);

	ret = platform_clk_cfg(ctrl_id, cfg);
	if (ret)
		return ret;

	platform_reset_ctl(ctrl_id, PRESETN, RESET_DEASSERT);

	/* Step 2 */
	ddrmc_cfg(ctrl_id, cfg);

	/* Step 3 */
	platform_reset_ctl(ctrl_id, CORERESETN, RESET_DEASSERT);

	/* Step 4 */
	val = read32(DDRMC_RFSHCTL3(ctrl_id));
	val |= DDRMC_RFSHCTL3_DIS_AUTO_REFRESH;
	write32(DDRMC_RFSHCTL3(ctrl_id), val);

	val = read32(DDRMC_PWRCTL(ctrl_id));
	val &= ~DDRMC_PWRCTL_SELF_REFRESH_EN & ~DDRMC_PWRCTL_POWERDOWN_EN &
	       ~DDRMC_PWRCTL_DFI_DRAMCLK_DIS;
	val |= DDRMC_PWRCTL_SELFREF_SW;
	write32(DDRMC_PWRCTL(ctrl_id), val);

	/* Step 5 */
	write32(DDRMC_SWCTL(ctrl_id), 0);

	/* Step 6 */
	val = read32(DDRMC_DFIMISC(ctrl_id));
	val &= ~DDRMC_DFIMISC_COMPLETE_EN;
	write32(DDRMC_DFIMISC(ctrl_id), val);

	/* Step 7 */
	write32(DDRMC_SWCTL(ctrl_id), 1);
	while (!(read32(DDRMC_SWSTAT(ctrl_id)) & DDRMC_SWSTAT_SWDONE_ACK))
		continue;

	/* Step 8 - Step 14 */
	ret = phy_cfg(ctrl_id, cfg);
	if (ret)
		return ret;

	/* Step 15 */
	write32(DDRMC_SWCTL(ctrl_id), 0);

	/* Step 16 */
	val = read32(DDRMC_DFIMISC(ctrl_id));
	val |= DDRMC_DFIMISC_INIT_START;
	write32(DDRMC_DFIMISC(ctrl_id), val);

	/* Step 17 */
	write32(DDRMC_SWCTL(ctrl_id), 1);
	while (!(read32(DDRMC_SWSTAT(ctrl_id)) & DDRMC_SWSTAT_SWDONE_ACK))
		continue;

	/* Step 18 */
	while (!(read32(DDRMC_DFISTAT(ctrl_id)) & DDRMC_DFISTAT_INIT_COMPLETE))
		continue;

	/* Step 19 */
	write32(DDRMC_SWCTL(ctrl_id), 0);

	/* Step 20 */
	val = read32(DDRMC_DFIMISC(ctrl_id));
	val &= ~DDRMC_DFIMISC_INIT_START;
	write32(DDRMC_DFIMISC(ctrl_id), val);

	/* Step 21: optional */

	/* Step 22 */
	val = read32(DDRMC_DFIMISC(ctrl_id));
	val |= DDRMC_DFIMISC_COMPLETE_EN;
	write32(DDRMC_DFIMISC(ctrl_id), val);

	/* Step 23 */
	val = read32(DDRMC_PWRCTL(ctrl_id));
	val &= ~DDRMC_PWRCTL_SELFREF_SW;
	write32(DDRMC_PWRCTL(ctrl_id), val);

	/* Step 24 */
	write32(DDRMC_SWCTL(ctrl_id), 1);
	while (!(read32(DDRMC_SWSTAT(ctrl_id)) & DDRMC_SWSTAT_SWDONE_ACK))
		continue;

	/* Step 25 */
	while (FIELD_GET(DDRMC_STAT_OPER_MODE, read32(DDRMC_STAT(ctrl_id))) != 1)
		continue;

	/* Step 26 */
	val = read32(DDRMC_RFSHCTL3(ctrl_id));
	val &= ~DDRMC_RFSHCTL3_DIS_AUTO_REFRESH;
	write32(DDRMC_RFSHCTL3(ctrl_id), val);

	/* Enable all AXI ports */
	for (i = 0; i < CONFIG_DDRMC_AXI_PORTS; i++)
		write32(DDRMC_PCTRL(ctrl_id, i), 1);

	return 0;
}

static int ddrcfg_get(int ctrl_id, struct ddr_cfg *cfg)
{
	int ret;
#ifdef CONFIG_SPD_EEPROM
#ifdef CONFIG_DRAM_TYPE_DDR4
	struct ddr4_spd spd;
#endif

	ret = spd_get(ctrl_id, &spd);
	if (ret)
		return ret;

	ret = spd_parse(&spd, cfg);
#else
	ret = platform_ddrcfg_get(ctrl_id, cfg);
#endif
	return ret;
}

char *errcode2str(int id)
{
	switch (-id) {
		case EPOWERUP: return "Failed to power up";
		case ECLOCKCFG: return "Failed to configure clock";
		case EI2CREAD: return "Failed to read over I2C";
		case ESPDCHECKSUM: return "Incorrect SPD checksum";
		case EDIMMCFG: return "Failed to configure DIMM";
		case EFWTYPE: return "Unknown firmware type";
		case ETRAINFAIL: return "PHY trainining error";
		default: return "Unknown error";
	}
}

int main(void)
{
	struct ddr_cfg cfg;
	struct sysinfo info;
	int ret, i;

	memset(&info, 0, sizeof(info));
	cfg.sysinfo = &info;

	uart_cfg();

	for (i = 0; i < CONFIG_DDRMC_MAX_NUMBER; i++) {
		ret = ddrcfg_get(i, &cfg);
		if (ret) {
			print_dbg("DDRMC%d: Failed to get configuration\n", i);
			continue;
		}

		ret = ddr_init(i, &cfg);
		if (ret != 0)
			printf("DDRMC%d: Failed to initialize: %s\n", i, errcode2str(ret));
		else
			printf("DDRMC%d: Initialized successfully, speed %d MT/s\n", i,
			       info.speed[i]);
	}

	platform_system_init();

	printf("Total DDR memory size %d MiB\n", (int)(info.dram_size / 1024 / 1024));
	printf("Interleaving %s\n", (info.interleaving_enabled) ? "enabled" : "disabled");

	return 0;
}
