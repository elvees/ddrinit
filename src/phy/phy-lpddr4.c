// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2021 RnD Center "ELVEES", JSC

#include <common.h>
#include <ddrmc.h>
#include <dram/lpddr4/phy-training-params-lpddr4.h>
#include <phy.h>
#include <plat/plat.h>
#include <regs.h>
#include <string.h>

static void dll_init(int ctrl_id)
{
	uint32_t tmp;

	/* Set DbyteDllModeCntrl */
	tmp = PREP_FIELD(PHY_DBYTE_DLLMODE_CTL, DLLRX_PREAMBLE_MODE, 1);
	phy_write32_with_dbg(ctrl_id, PHY_DBYTE_DLLMODE_CTL, tmp);

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
#ifdef CONFIG_PHY_ODT_240
	tmp2 = 0x2;
#elif CONFIG_PHY_ODT_120
	tmp2 = 0x8;
#elif CONFIG_PHY_ODT_80
	tmp2 = 0xa;
#elif CONFIG_PHY_ODT_60
	tmp2 = 0x18;
#elif CONFIG_PHY_ODT_40
	tmp2 = 0x38;
#elif CONFIG_PHY_ODT_34
	tmp2 = 0x3a;
#elif CONFIG_PHY_ODT_HIZ
	tmp2 = 0;
#endif
	tmp1 = PREP_FIELD2(PHY_TX_ODT_DRV_STREN, ODT_STREN_N, tmp2, ODT_STREN_P, 0);

	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++) {
		phy_write32_with_dbg(ctrl_id, PHY_TX_ODT_DRV_STREN_B0(i), tmp1);
		phy_write32_with_dbg(ctrl_id, PHY_TX_ODT_DRV_STREN_B1(i), tmp1);
	}

	/* Set TxImpedanceCtrl1 */
#ifdef CONFIG_PHY_TX_IMPEDANCE_240
	tmp2 = 0x2;
#elif CONFIG_PHY_TX_IMPEDANCE_120
	tmp2 = 0x8;
#elif CONFIG_PHY_TX_IMPEDANCE_80
	tmp2 = 0xa;
#elif CONFIG_PHY_TX_IMPEDANCE_60
	tmp2 = 0x18;
#elif CONFIG_PHY_TX_IMPEDANCE_40
	tmp2 = 0x38;
#elif CONFIG_PHY_TX_IMPEDANCE_34
	tmp2 = 0x3a;
#elif CONFIG_PHY_TX_IMPEDANCE_HIZ
	tmp2 = 0;
#endif
	tmp1 = PREP_FIELD2(PHY_TX_IMPEDANCE_CTRL1, DRV_STREN_FSDQ_P, tmp2, DRV_STREN_FSDQ_N, tmp2);

	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++) {
		phy_write32_with_dbg(ctrl_id, PHY_TX_IMPEDANCE_CTRL1_B0(i), tmp1);
		phy_write32_with_dbg(ctrl_id, PHY_TX_IMPEDANCE_CTRL1_B1(i), tmp1);
	}

	/* Set ATxImpedance */
#ifdef CONFIG_PHY_ATX_IMPEDANCE_120
	tmp2 = 0;
#elif CONFIG_PHY_ATX_IMPEDANCE_60
	tmp2 = 0x1;
#elif CONFIG_PHY_ATX_IMPEDANCE_40
	tmp2 = 0x3;
#elif CONFIG_PHY_ATX_IMPEDANCE_30
	tmp2 = 0x7;
#elif CONFIG_PHY_ATX_IMPEDANCE_24
	tmp2 = 0xf;
#elif CONFIG_PHY_ATX_IMPEDANCE_20
	tmp2 = 0x1f;
#endif
	tmp1 = PREP_FIELD2(PHY_ATX_IMPEDANCE, ADRV_STREN_P, tmp2, ADRV_STREN_N, tmp2);

	for (i = 0; i < CONFIG_PHY_ANIB_NUM; i++)
		phy_write32_with_dbg(ctrl_id, PHY_ATX_IMPEDANCE(i), tmp1);
}

static void dfi_init(int ctrl_id)
{
	/* Set DfiMode */
	phy_write32_with_dbg(ctrl_id, PHY_DFI_MODE, 0x3);

	/* Set DfiCAMode */
	phy_write32_with_dbg(ctrl_id, PHY_DFI_CA_MODE, 0x4);
}

static void calibration_init(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t tmp1, tmp2;

#ifdef CONFIG_IMPEDANCE_CALIBRATION_DISABLE
	phy_write32_with_dbg(ctrl_id, PHY_CALMISC, 0x6);
	phy_write32_with_dbg(ctrl_id, PHY_CALPEXT_OVR, CONFIG_PHY_CALPEXT_OVR);
	phy_write32_with_dbg(ctrl_id, PHY_CALNINT_OVR, CONFIG_PHY_CALNINT_OVR);
#endif

	/* Set CalDrvStr0 */
#ifdef CONFIG_PHY_CALIBRATION_EXT_RESISTOR_240
	tmp2 = 0;
#elif CONFIG_PHY_CALIBRATION_EXT_RESISTOR_120
	tmp2 = 1;
#else
	tmp2 = 2;
#endif
	tmp1 = PREP_FIELD2(PHY_CAL_DRV_STR0, PD50, tmp2, PU50, tmp2);
	phy_write32_with_dbg(ctrl_id, PHY_CAL_DRV_STR0, tmp1);

	/* Set CalUclkInfo */
	tmp1 = DIV_ROUND_UP(1000000, 2 * cfg->tck);
	if (tmp1 < 24)
		tmp1 = 24;
	phy_write32_with_dbg(ctrl_id, PHY_CAL_UCLK_INFO, tmp1);

	/* Set CalRate */
	tmp1 = PREP_FIELD2(PHY_CAL_RATE, INTERVAL, CONFIG_PHY_CALIBRATION_INTERVAL, ONCE, 0);
	phy_write32_with_dbg(ctrl_id, PHY_CAL_RATE, tmp1);
}

static void dfifreq_xlat_cfg(int ctrl_id)
{
	uint16_t vals[] = {0, 0, 0x4444, 0x8888, 0x5555, 0, 0, 0xf000};
	int i;

/* According to phyinit code DFI_FREQ_XLAT0 and DFI_FREQ_XLAT4 must be increased
 * in case of PLL bypass. There is no any explanation in PHY PUB databook.
 */
#ifdef CONFIG_PHY_PLL_BYPASS
	vals[0] += 6;
	vals[4] += 1;
#endif
	/* Set DfiXlat */
	for (i = 0; i < 8; i++)
		phy_write32_with_dbg(ctrl_id, PHY_DFI_FREQ_XLAT(i), vals[i]);
}

void phy_init(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t tmp1;
	int i;

	/* Set TxSlewRate */
	tmp1 = PREP_FIELD3(PHY_TX_SLEW_RATE, PREDRV_MODE, 0x1, TXPRE_N, CONFIG_PHY_TXSLEW_FALL_DQ,
			   TXPRE_P, CONFIG_PHY_TXSLEW_RISE_DQ);

	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++) {
		phy_write32_with_dbg(ctrl_id, PHY_TX_SLEW_RATE_B0(i), tmp1);
		phy_write32_with_dbg(ctrl_id, PHY_TX_SLEW_RATE_B1(i), tmp1);
	}

	/* Set ATxSlewRate */
	for (i = 0; i < CONFIG_PHY_ANIB_NUM; i++) {
		tmp1 = PREP_FIELD3(PHY_ATX_SLEW_RATE, PREDRV_MODE, 0x1, ATXPRE_N,
				   CONFIG_PHY_TXSLEW_FALL_AC, ATXPRE_P, CONFIG_PHY_TXSLEW_RISE_AC);
		phy_write32_with_dbg(ctrl_id, PHY_ATX_SLEW_RATE(i), tmp1);
	}

	/* Set PllCtrl2 */
	if (cfg->tck > DRAM_TCK_1066)
		tmp1 = 0x7;
	else if (cfg->tck >= DRAM_TCK_1250)
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

/* According to PHY PUB databook ARdPtrInitVal must be increased by 1 if PLL bypass is used */
#ifdef CONFIG_PHY_PLL_BYPASS
	tmp1 += 1;
#endif
	phy_write32_with_dbg(ctrl_id, PHY_ARDPTR_INITVAL, tmp1);

	/* Set Seq0BGPR4 */
	phy_write32_with_dbg(ctrl_id, PHY_SEQ0BGPR4, 0x0);

	/* Set DqsPreambleControl */
	tmp1 = PREP_FIELD3(PHY_DQS_PREAMBLE_CTL,
			   TWOTCK_RX_DQSPRE, 1,
			   TWOTCK_TX_DQSPRE, 1,
			   LP4_TGL_TWOTCK_TXDQS_PRE, 1);

	tmp1 |= PREP_FIELD3(PHY_DQS_PREAMBLE_CTL,
			   LP4_POSTAMLE_EXT, 1,
			   LP4_STTC_PREBRIDGE_RXEN, 1,
			   WDQS_EXT, 0);
	phy_write32_with_dbg(ctrl_id, PHY_DQS_PREAMBLE_CTL, tmp1);

	dll_init(ctrl_id);
	odt_init(ctrl_id, cfg);
	dfi_init(ctrl_id);
	calibration_init(ctrl_id, cfg);

	/* TBD: Set following registers depending on DDR configuration */
	/* Set VrefInGlobal */
	phy_write32_with_dbg(ctrl_id, PHY_VREF_IN_GLOBAL, 0x104);

	/* Set DqDqsRcvCntrl */
	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++) {
		phy_write32_with_dbg(ctrl_id, PHY_DQDQS_RCV_CNTRL_B0(i), 0x5a1);
		phy_write32_with_dbg(ctrl_id, PHY_DQDQS_RCV_CNTRL_B1(i), 0x5a1);
	}

	/* Set DfiFreqRatio */
	phy_write32_with_dbg(ctrl_id, PHY_DFI_FREQ_RATIO, 1);

	/* Set TristateModeCA */
	phy_write32_with_dbg(ctrl_id, PHY_TRISTATE_MODE_CA, 1);

	dfifreq_xlat_cfg(ctrl_id);

	/* Set DqDqsRcvCntrl1 */
	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++)
		phy_write32_with_dbg(ctrl_id, PHY_DQ_DQS_RCV_CNTRL1(i), 0x500);

	/* Set MasterX4Config */
	phy_write32_with_dbg(ctrl_id, PHY_MASTER_X4_CONFIG, 0);

	/* Set DMIPinPresent */
	phy_write32_with_dbg(ctrl_id, PHY_DMI_PIN_PRESENT, 0);

	/* Set Acx4AnibDis */
	phy_write32_with_dbg(ctrl_id, PHY_ACX4_ANIB_DIS, 0);
}

void phy_training_params_load(int ctrl_id, struct ddr_cfg *cfg)
{
	PMU_SMB_LPDDR4_1D_t params;

	memset(&params, 0, sizeof(params));

	params.Pstate = 0;
	params.DRAMFreq = CONFIG_DRAM_RATE;

#ifdef CONFIG_PHY_PLL_BYPASS
	params.PllBypassEn = 1;
#endif

#ifdef CONFIG_DISABLE_CA_TRAINING
	params.SequenceCtrl =  0x31f;
#else
	params.SequenceCtrl =  0x131f;
#endif
	params.PhyConfigOverride = 0;

#ifdef CONFIG_DEBUG
	params.HdtCtrl = 0x00;
#else
	params.HdtCtrl = 0xff;
#endif
	params.MsgMisc = 0;
	params.Reserved00 = 0xe0;
	params.DFIMRLMargin = 2;
	params.DfiFreqRatio = 2;
	params.PhyVref = 0x14;
	params.EnabledDQsChA = 16;
	params.CsPresentChA = (CONFIG_DRAM_RANKS == 2) ? 3 : 1;
	params.EnabledDQsChB = 16;
	params.CsPresentChB = (CONFIG_DRAM_RANKS == 2) ? 3 : 1;
	params.UseBroadcastMR = 0;
	params.Lp4Misc = 0;
	params.CATerminatingRankChA = 0;
	params.CATerminatingRankChB = 0;
	params.Lp4Quickboot = 0;
	params.CATrainOpt = 0;
	params.X8Mode = 0;

	params.MR1_A0 = lpddr4_mr1_get(cfg);
	params.MR2_A0 = lpddr4_mr2_get(cfg);
	params.MR3_A0 = 0x33;
	params.MR4_A0 = 0;
	params.MR11_A0 = lpddr4_mr11_get(cfg);
	params.MR12_A0 = 0x4d;
	params.MR13_A0 = BIT(5);
	params.MR14_A0 = 0x4f;
	params.MR16_A0 = 0;
	params.MR17_A0 = 0;
	params.MR22_A0 = lpddr4_mr22_get(cfg);
	params.MR24_A0 = 0;
	params.MR1_A1 = lpddr4_mr1_get(cfg);
	params.MR2_A1 = lpddr4_mr2_get(cfg);
	params.MR3_A1 = 0x33;
	params.MR4_A1 = 0;
	params.MR11_A1 = lpddr4_mr11_get(cfg);
	params.MR12_A1 = 0x4d;
	params.MR13_A1 = BIT(5);
	params.MR14_A1 = 0x4f;
	params.MR16_A1 = 0;
	params.MR17_A1 = 0;
	params.MR22_A1 = lpddr4_mr22_get(cfg);
	params.MR24_A1 = 0;

	params.MR1_B0 = lpddr4_mr1_get(cfg);
	params.MR2_B0 = lpddr4_mr2_get(cfg);
	params.MR3_B0 = 0x33;
	params.MR4_B0 = 0;
	params.MR11_B0 = lpddr4_mr11_get(cfg);
	params.MR12_B0 = 0x4d;
	params.MR13_B0 = BIT(5);
	params.MR14_B0 = 0x4f;
	params.MR16_B0 = 0;
	params.MR17_B0 = 0;
	params.MR22_B0 = lpddr4_mr22_get(cfg);
	params.MR24_B0 = 0;
	params.MR1_B1 = lpddr4_mr1_get(cfg);
	params.MR2_B1 = lpddr4_mr2_get(cfg);
	params.MR3_B1 = 0x33;
	params.MR4_B1 = 0;
	params.MR11_B1 = lpddr4_mr11_get(cfg);
	params.MR12_B1 = 0x4d;
	params.MR13_B1 = BIT(5);
	params.MR14_B1 = 0x4f;
	params.MR16_B1 = 0;
	params.MR17_B1 = 0;
	params.MR22_B1 = lpddr4_mr22_get(cfg);
	params.MR24_B1 = 0;

	params.Share2DVrefResult = 1;

	unsigned long read_offset = (unsigned long)&params;
	uint32_t val, write_offset = CONFIG_PHY_DMEM_OFFSET;
	int i;

	/* Write training firmware parameters */
	for (i = 0; i < sizeof(params) / 4; i++) {
		val = read32(read_offset);
		phy_write32_with_dbg(ctrl_id, write_offset, val & 0xffff);
		phy_write32_with_dbg(ctrl_id, write_offset + 4, (val >> 16) & 0xffff);
		write_offset += 8;
		read_offset += 4;
	}

	/* Write the last bytes if params size is not a multiple of 4 */
	int tail = sizeof(params) % 4;
	if (tail != 0) {
		val = read32(read_offset);
		phy_write32_with_dbg(ctrl_id, write_offset, val & GENMASK(8 * tail, 0));
		if (tail == 3)
			phy_write32_with_dbg(ctrl_id, write_offset + 4, (val >> 16) & 0xff);
	}
}
