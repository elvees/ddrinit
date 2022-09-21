// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2020 RnD Center "ELVEES", JSC

#include <string.h>
#include <common.h>
#include <ddrmc.h>
#include <ddrspd.h>
#include <phy.h>
#include <plat/plat.h>
#include <regs.h>
#include <uart.h>

#define ddrinit_panic(ret)                                                    \
	({                                                                    \
		printf("Failed to initialize DDR: %s\n", errcode2str((ret))); \
		while (1) {                                                   \
			/* ... */                                             \
		}                                                             \
	})

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

	ret = platform_reset_ctl(ctrl_id, PRESETN, RESET_ASSERT);
	if (ret)
		return ret;

	ret = platform_reset_ctl(ctrl_id, CORERESETN, RESET_ASSERT);
	if (ret)
		return ret;

	ret = platform_clk_cfg(ctrl_id, cfg);
	if (ret)
		return ret;

	ret = platform_reset_ctl(ctrl_id, PRESETN, RESET_DEASSERT);
	if (ret)
		return ret;

	/* Step 2 */
	ddrmc_cfg(ctrl_id, cfg);

	/* Step 3 */
	ret = platform_reset_ctl(ctrl_id, CORERESETN, RESET_DEASSERT);
	if (ret)
		return ret;

	/* Step 4 */
	val = read32(DDRMC_RFSHCTL3(ctrl_id));
	val |= DDRMC_RFSHCTL3_DIS_AUTO_REFRESH;
	write32_with_dbg(DDRMC_RFSHCTL3(ctrl_id), val);

	val = read32(DDRMC_PWRCTL(ctrl_id));
	val &= ~DDRMC_PWRCTL_SELF_REFRESH_EN & ~DDRMC_PWRCTL_POWERDOWN_EN &
	       ~DDRMC_PWRCTL_DFI_DRAMCLK_DIS;
	val |= DDRMC_PWRCTL_SELFREF_SW;
	write32_with_dbg(DDRMC_PWRCTL(ctrl_id), val);

	/* Step 5 */
	write32_with_dbg(DDRMC_SWCTL(ctrl_id), 0);

	/* Step 6 */
	val = read32(DDRMC_DFIMISC(ctrl_id));
	val &= ~DDRMC_DFIMISC_COMPLETE_EN;
	write32_with_dbg(DDRMC_DFIMISC(ctrl_id), val);

	/* Step 7 */
	write32_with_dbg(DDRMC_SWCTL(ctrl_id), 1);
	ret = read32_poll_timeout(val, val & DDRMC_SWSTAT_SWDONE_ACK, USEC, 100 * USEC,
				  DDRMC_SWSTAT(ctrl_id));
	if (ret)
		return ret;

	/* Step 8 - Step 14 */
	ret = phy_cfg(ctrl_id, cfg);
	if (ret)
		return ret;

	/* Step 15 */
	write32_with_dbg(DDRMC_SWCTL(ctrl_id), 0);

	/* Step 16 */
	val = read32(DDRMC_DFIMISC(ctrl_id));
	val |= DDRMC_DFIMISC_INIT_START;
	write32_with_dbg(DDRMC_DFIMISC(ctrl_id), val);

	/* Step 17 */
	write32_with_dbg(DDRMC_SWCTL(ctrl_id), 1);
	ret = read32_poll_timeout(val, val & DDRMC_SWSTAT_SWDONE_ACK, USEC, 100 * USEC,
				  DDRMC_SWSTAT(ctrl_id));
	if (ret)
		return ret;

	/* Step 18 */
	ret = read32_poll_timeout(val, val & DDRMC_DFISTAT_INIT_COMPLETE, USEC, 100 * USEC,
				  DDRMC_DFISTAT(ctrl_id));
	if (ret)
		return ret;

	/* Step 19 */
	write32_with_dbg(DDRMC_SWCTL(ctrl_id), 0);

	/* Step 20 */
	val = read32(DDRMC_DFIMISC(ctrl_id));
	val &= ~DDRMC_DFIMISC_INIT_START;
	write32_with_dbg(DDRMC_DFIMISC(ctrl_id), val);

	/* Step 21: optional */

	/* Step 22 */
	val = read32(DDRMC_DFIMISC(ctrl_id));
	val |= DDRMC_DFIMISC_COMPLETE_EN;
	write32_with_dbg(DDRMC_DFIMISC(ctrl_id), val);

	/* Step 23 */
	val = read32(DDRMC_PWRCTL(ctrl_id));
	val &= ~DDRMC_PWRCTL_SELFREF_SW;
	write32_with_dbg(DDRMC_PWRCTL(ctrl_id), val);

	/* Step 24 */
	write32_with_dbg(DDRMC_SWCTL(ctrl_id), 1);
	ret = read32_poll_timeout(val, val & DDRMC_SWSTAT_SWDONE_ACK, USEC, 100 * USEC,
				  DDRMC_SWSTAT(ctrl_id));
	if (ret)
		return ret;

	/* Step 25 */
	ret = read32_poll_timeout(val, (val & DDRMC_STAT_OPER_MODE) == 1, USEC, 100 * USEC,
				  DDRMC_STAT(ctrl_id));
	if (ret)
		return ret;

	/* Step 26 */
	val = read32(DDRMC_RFSHCTL3(ctrl_id));
	val &= ~DDRMC_RFSHCTL3_DIS_AUTO_REFRESH;
	write32_with_dbg(DDRMC_RFSHCTL3(ctrl_id), val);

	/* Enable all AXI ports */
	for (i = 0; i < CONFIG_DDRMC_AXI_PORTS; i++)
		write32_with_dbg(DDRMC_PCTRL(ctrl_id, i), 1);

	return 0;
}

char *errcode2str(int id)
{
	switch (-id) {
	case EDDRMC0INITFAIL:
		return "Required DDRMCs have not been initialized";
	case EPOWERUP:
		return "Failed to power up";
	case ECLOCKCFG:
		return "Failed to configure clock";
	case EI2CXFER:
		return "Failed to communicate over I2C";
	case ESPDCHECKSUM:
		return "Incorrect SPD checksum";
	case EDIMMCFG:
		return "Failed to configure DIMM";
	case EFWTYPE:
		return "Unknown firmware type";
	case ETRAINFAIL:
		return "PHY training error";
	case EI2CCFG:
		return "Failed to configure I2C controller";
	case ETIMEDOUT:
		return "Event timed out";
	case ETRAINTIMEOUT:
		return "PHY training timed out";
	case EUARTCFG:
		return "Failed to configure UART";
	case EVMMUCFG:
		return "Failed to configure VMMU";
	default:
		return "Unknown error";
	}
}

int main(void)
{
	struct ddr_cfg cfg;
	struct sysinfo info;
	int ret, i, init_mask = 0;
	int timer_start, timer_end;

	memset(&info, 0, sizeof(info));
	cfg.sysinfo = &info;

	ret = platform_power_up();
	if (ret) {
		while (1)
			continue;
	}

	ret = uart_cfg();
	/* Stop initializing DDR if failed to init UART */
	if (ret) {
		while (1)
			continue;
	}

#if defined(VERSION)
	printf("ddrinit: %s\n", VERSION);
#endif

	for (i = 0; i < CONFIG_DDRMC_MAX_NUMBER; i++) {
		timer_start = timer_get_usec();
		ret = ddrcfg_get(i, &cfg);
		if (ret) {
			printf("DDRMC%d: Failed to get configuration: %s\n", i, errcode2str(ret));
			continue;
		}
		info.speed[i] = 2000000 / cfg.tck;

		ret = ddr_init(i, &cfg);
		timer_end = timer_get_usec();
		if (ret != 0) {
			printf("DDRMC%d: Failed to initialize, time elapsed: %d us: %s\n", i,
			       timer_end - timer_start, errcode2str(ret));
		} else {
			printf("DDRMC%d: Initialized successfully within %d us, %d ranks, speed %d MT/s\n",
			       i, timer_end - timer_start, cfg.ranks, info.speed[i]);
			init_mask |= BIT(i);
			info.dram_size[i] = cfg.full_size;
			info.total_dram_size += info.dram_size[i];
		}
	}

	if ((init_mask & CONFIG_DDRMC_INIT_REQUIRED_MASK) == CONFIG_DDRMC_INIT_REQUIRED_MASK) {
		ret = platform_system_init(init_mask, &info);
		if (ret)
			ddrinit_panic(ret);

		memcpy((void *)CONFIG_MEM_REGIONS_VIRT_ADDR, &info, sizeof(info));

		printf("Total DDR memory size %d MiB\n", (int)(info.total_dram_size / 1024 / 1024));

		if (info.interleaving.enable)
			printf("Memory interleaving: %d KiB, %d-channel\n",
			       info.interleaving.size / 1024, info.interleaving.channels);
		else
			printf("Memory interleaving: disabled\n");

		return 0;
	}

	ddrinit_panic(-EDDRMC0INITFAIL);
}
