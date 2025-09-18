// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2023 RnD Center "ELVEES", JSC

#include <string.h>
#include <common.h>
#include <ddrmc.h>
#include <dram/ddr3/phy-training-params-ddr3.h>
#include <phy.h>
#include <regs.h>
#include <plat/plat.h>

static PMU_SMB_DDR3U_1D_t mb_DDR3U_1D = { 0 };

static void dll_init(int ctrl_id)
{
	uint32_t tmp;

	/* Set DbyteDllModeCntrl */
	phy_write32_with_dbg(ctrl_id, PHY_DBYTE_DLLMODE_CTL, 0);

	/* Set DllGainCtl */
	tmp = PREP_FIELD2(PHY_DLLGANE_CTL, DLLGAIN_TV, 6, DLLGAIN_IV, 1);
	phy_write32_with_dbg(ctrl_id, PHY_DLLGANE_CTL, tmp);

	/* Set DllLockParam */
	tmp = PREP_FIELD2(PHY_DLLLOCK_PARAM, DIS_DLLGAIN_IVSEED, 1, LCD_LSEED0, 0x21);
	phy_write32_with_dbg(ctrl_id, PHY_DLLLOCK_PARAM, tmp);
}

static void odt_init(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t tmp1, tmp2;
	int i;

	/* Set ProcOdtTimeCtl */
	if (cfg->tck >= DRAM_TCK_1866)
		tmp1 = 10;
	else if (cfg->tck < DRAM_TCK_1866 && cfg->tck >= DRAM_TCK_2400)
		tmp1 = (CONFIG_PHY_D4RX_PREAMBLE_LEN == 1) ? 2 : 6;
	else
		tmp1 = (CONFIG_PHY_D4RX_PREAMBLE_LEN == 1) ? 3 : 7;

	phy_write32_with_dbg(ctrl_id, PHY_PROCODT_TIMECTL, tmp1);

	/* Set TxOdtDrvStren */
	if (IS_ENABLED(CONFIG_PHY_ODT_240))
		tmp2 = 0x1;
	else if (IS_ENABLED(CONFIG_PHY_ODT_120))
		tmp2 = 0x2;
	else if (IS_ENABLED(CONFIG_PHY_ODT_80))
		tmp2 = 0x3;
	else if (IS_ENABLED(CONFIG_PHY_ODT_60))
		tmp2 = 0x8;
	else if (IS_ENABLED(CONFIG_PHY_ODT_40))
		tmp2 = 0xa;
	else if (IS_ENABLED(CONFIG_PHY_ODT_34))
		tmp2 = 0xb;
	else if (IS_ENABLED(CONFIG_PHY_ODT_HIZ))
		tmp2 = 0x0;

	tmp1 = PREP_FIELD2(PHY_TX_ODT_DRV_STREN, ODT_STREN_P, tmp2, ODT_STREN_N, tmp2);

	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++) {
		phy_write32_with_dbg(ctrl_id, PHY_TX_ODT_DRV_STREN_B0(i), tmp1);
		phy_write32_with_dbg(ctrl_id, PHY_TX_ODT_DRV_STREN_B1(i), tmp1);
	}

	/* Set TxImpedanceCtrl1 */
	if (IS_ENABLED(CONFIG_TX_IMPEDANCE_240))
		tmp2 = 0x2;
	else if (IS_ENABLED(CONFIG_TX_IMPEDANCE_120))
		tmp2 = 0x8;
	else if (IS_ENABLED(CONFIG_TX_IMPEDANCE_80))
		tmp2 = 0xa;
	else if (IS_ENABLED(CONFIG_TX_IMPEDANCE_60))
		tmp2 = 0x18;
	else if (IS_ENABLED(CONFIG_TX_IMPEDANCE_40))
		tmp2 = 0x38;
	else if (IS_ENABLED(CONFIG_TX_IMPEDANCE_34))
		tmp2 = 0x3a;
	else if (IS_ENABLED(CONFIG_TX_IMPEDANCE_HIZ))
		tmp2 = 0x0;

	tmp1 = PREP_FIELD2(PHY_TX_IMPEDANCE_CTRL1, DRV_STREN_FSDQ_P, tmp2, DRV_STREN_FSDQ_N, tmp2);

	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++) {
		phy_write32_with_dbg(ctrl_id, PHY_TX_IMPEDANCE_CTRL1_B0(i), tmp1);
		phy_write32_with_dbg(ctrl_id, PHY_TX_IMPEDANCE_CTRL1_B1(i), tmp1);
	}

	/* Set ATxImpedance */
	if (IS_ENABLED(CONFIG_ATX_IMPEDANCE_120))
		tmp2 = 0x0;
	else if (IS_ENABLED(CONFIG_ATX_IMPEDANCE_60))
		tmp2 = 0x1;
	else if (IS_ENABLED(CONFIG_ATX_IMPEDANCE_40))
		tmp2 = 0x3;
	else if (IS_ENABLED(CONFIG_ATX_IMPEDANCE_30))
		tmp2 = 0x7;
	else if (IS_ENABLED(CONFIG_ATX_IMPEDANCE_24))
		tmp2 = 0xf;
	else if (IS_ENABLED(CONFIG_ATX_IMPEDANCE_20))
		tmp2 = 0x1f;

	tmp1 = PREP_FIELD2(PHY_ATX_IMPEDANCE, ADRV_STREN_P, tmp2, ADRV_STREN_N, tmp2);

	for (i = 0; i < CONFIG_PHY_ANIB_NUM; i++)
		phy_write32_with_dbg(ctrl_id, PHY_ATX_IMPEDANCE(i), tmp1);
}

static void dfi_init(int ctrl_id)
{
	/* Set DfiMode */
	if (IS_ENABLED(CONFIG_PHY_DFI1_EXIST))
		phy_write32_with_dbg(ctrl_id, PHY_DFI_MODE, 0x5);
	else
		phy_write32_with_dbg(ctrl_id, PHY_DFI_MODE, 0x1);

	/* Set DfiCAMode */
	phy_write32_with_dbg(ctrl_id, PHY_DFI_CA_MODE, 0x0);
}

static void calibration_init(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t tmp1, tmp2;

	/* Set CalDrvStr0 */
	if (IS_ENABLED(CONFIG_PHY_CALIBRATION_EXT_RESISTOR_240))
		tmp2 = 0;
	else if (IS_ENABLED(CONFIG_PHY_CALIBRATION_EXT_RESISTOR_120))
		tmp2 = 1;
	else
		tmp2 = 2;

	tmp1 = PREP_FIELD2(PHY_CAL_DRV_STR0, PD50, tmp2, PU50, tmp2);
	phy_write32_with_dbg(ctrl_id, PHY_CAL_DRV_STR0, tmp1);

	/* Set CalUclkInfo */
	phy_write32_with_dbg(ctrl_id, PHY_CAL_UCLK_INFO, DIV_ROUND_UP(1000000, 2 * cfg->tck));

	/* Set CalRate */
	tmp1 = PREP_FIELD2(PHY_CAL_RATE, INTERVAL, CONFIG_PHY_CALIBRATION_INTERVAL, ONCE, 0);
	phy_write32_with_dbg(ctrl_id, PHY_CAL_RATE, tmp1);
}

void phy_init(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t tmp1, tmp2;
	int i;

	/* Set TxSlewRate */
	tmp1 = PREP_FIELD3(PHY_TX_SLEW_RATE, PREDRV_MODE, 0x3, TXPRE_N, CONFIG_PHY_TXSLEW_FALL_DQ,
			   TXPRE_P, CONFIG_PHY_TXSLEW_RISE_DQ);

	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++) {
		phy_write32_with_dbg(ctrl_id, PHY_TX_SLEW_RATE_B0(i), tmp1);
		phy_write32_with_dbg(ctrl_id, PHY_TX_SLEW_RATE_B1(i), tmp1);
	}

	/* Set ATxSlewRate */
	for (i = 0; i < CONFIG_PHY_ANIB_NUM; i++) {
		tmp2 = (i == 4 || i == 5) ? 0 : 3;
		tmp1 = PREP_FIELD3(PHY_ATX_SLEW_RATE, PREDRV_MODE, tmp2, ATXPRE_N,
				   CONFIG_PHY_TXSLEW_FALL_AC, ATXPRE_P, CONFIG_PHY_TXSLEW_RISE_AC);
		phy_write32_with_dbg(ctrl_id, PHY_ATX_SLEW_RATE(i), tmp1);
	}

	/* Set PllCtrl2 register */
	if (cfg->tck >= DRAM_TCK_1250)
		tmp1 = 0x6;
	else if (cfg->tck < DRAM_TCK_1250 && cfg->tck >= DRAM_TCK_1866)
		tmp1 = 0xb;
	else if (cfg->tck < DRAM_TCK_1866 && cfg->tck >= DRAM_TCK_2400)
		tmp1 = 0xa;
	else
		tmp1 = 0x19;
	phy_write32_with_dbg(ctrl_id, PHY_PLLCTRL2, tmp1);

	/* Set ARdPtrInitVal */
	if (cfg->tck <= DRAM_TCK_1866)
		tmp1 = 0x2;
	else
		tmp1 = 0x1;
	phy_write32_with_dbg(ctrl_id, PHY_ARDPTR_INITVAL, tmp1);

	/* Set DqsPreambleControl */
	phy_write32_with_dbg(ctrl_id, PHY_DQS_PREAMBLE_CTL, 0);

	dll_init(ctrl_id);
	odt_init(ctrl_id, cfg);
	dfi_init(ctrl_id);
	calibration_init(ctrl_id, cfg);

	/* TODO: Set VREF depending on PHY training params */
	phy_write32_with_dbg(ctrl_id, PHY_VREF_IN_GLOBAL, 0x208);

	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++) {
		phy_write32_with_dbg(ctrl_id, PHY_DQDQS_RCV_CNTRL_B0(i), 0x581);
		phy_write32_with_dbg(ctrl_id, PHY_DQDQS_RCV_CNTRL_B1(i), 0x581);
	}

	phy_write32_with_dbg(ctrl_id, PHY_DFI_FREQ_RATIO, 1);
	phy_write32_with_dbg(ctrl_id, PHY_TRISTATE_MODE_CA, 4);

	for (i = 0; i < 7; i++)
		phy_write32_with_dbg(ctrl_id, PHY_DFI_FREQ_XLAT(i), 0x5555);

	phy_write32_with_dbg(ctrl_id, PHY_DFI_FREQ_XLAT(7), 0xF000);

	phy_write32_with_dbg(ctrl_id, PHY_MASTER_X4_CONFIG, 0);
	phy_write32_with_dbg(ctrl_id, PHY_ACX4_ANIB_DIS, 0);
}

void phy_training_params_load(int ctrl_id, struct ddr_cfg *cfg)
{
	/* As the DDRMC0 is always initialized and the same params
	 * are used for all DDR controllers, we can initialize mb_DDR3U_1D
	 * once when ctrl_id == 0 and then reuse it to save boot time.
	 */
	if (!ctrl_id) {
		mb_DDR3U_1D.Reserved00 = 0x60;
		mb_DDR3U_1D.DramType = 0x1;
		mb_DDR3U_1D.Pstate = 0;
		mb_DDR3U_1D.SequenceCtrl = 0x31f;
		mb_DDR3U_1D.PhyConfigOverride = 0;
		mb_DDR3U_1D.DRAMFreq = CONFIG_DRAM_RATE;

		if (IS_ENABLED(CONFIG_DEBUG))
			mb_DDR3U_1D.HdtCtrl = 0x00;
		else
			mb_DDR3U_1D.HdtCtrl = 0xff;

		mb_DDR3U_1D.MsgMisc = 0;
		mb_DDR3U_1D.DFIMRLMargin = 1;
		mb_DDR3U_1D.PhyVref = 0x56;

		mb_DDR3U_1D.CsPresent = BIT(cfg->ranks) - 1;
		mb_DDR3U_1D.CsPresentD0 = BIT(cfg->ranks) - 1;
		mb_DDR3U_1D.CsPresentD1 = 0;
		mb_DDR3U_1D.AddrMirror = 0xa;

		mb_DDR3U_1D.AcsmOdtCtrl0 = 0x21;
		mb_DDR3U_1D.AcsmOdtCtrl1 = 0x12;
		mb_DDR3U_1D.AcsmOdtCtrl2 = 0x84;
		mb_DDR3U_1D.AcsmOdtCtrl3 = 0x48;

		mb_DDR3U_1D.EnabledDQs = 64;
		mb_DDR3U_1D.PhyCfg = 0;

		mb_DDR3U_1D.MR0 = ddr3_mr0_get(cfg);
		mb_DDR3U_1D.MR1 = ddr3_mr1_get(cfg);
		mb_DDR3U_1D.MR2 = ddr3_mr2_get(cfg);

		mb_DDR3U_1D.Share2DVrefResult = 0x1;
	}

	unsigned long read_offset = (unsigned long)&mb_DDR3U_1D;
	uint32_t val, write_offset = CONFIG_PHY_DMEM_OFFSET;
	int i;

	/* Write training firmware parameters */
	for (i = 0; i < sizeof(mb_DDR3U_1D) / 4; i++) {
		val = read32(read_offset);
		phy_write32_with_dbg(ctrl_id, write_offset, val & 0xffff);
		phy_write32_with_dbg(ctrl_id, write_offset + 4, (val >> 16) & 0xffff);
		write_offset += 8;
		read_offset += 4;
	}

	/* Write the last bytes if params size is not a multiple of 4 */
	int tail = sizeof(mb_DDR3U_1D) % 4;
	if (tail != 0) {
		val = read32(read_offset);
		phy_write32_with_dbg(ctrl_id, write_offset, val & GENMASK(8 * tail - 1, 0));
		if (tail == 3)
			phy_write32_with_dbg(ctrl_id, write_offset + 4, (val >> 16) & 0xff);
	}
}
