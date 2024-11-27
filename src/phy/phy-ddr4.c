// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2020 RnD Center "ELVEES", JSC

#include <string.h>
#include <common.h>
#include <ddrmc.h>
#include <dram/ddr4/phy-training-params-ddr4.h>
#include <phy.h>
#include <regs.h>
#include <plat/plat.h>

static void dll_init(int ctrl_id)
{
	uint32_t tmp;

	/* Set DbyteDllModeCntrl */
	tmp = PREP_FIELD(PHY_DBYTE_DLLMODE_CTL, DLLRX_PREAMBLE_MODE, 1);
	phy_write32(ctrl_id, PHY_DBYTE_DLLMODE_CTL, tmp);

	/* Set DllGainCtl */
	tmp = PREP_FIELD2(PHY_DLLGANE_CTL, DLLGAIN_TV, 6, DLLGAIN_IV, 1);
	phy_write32(ctrl_id, PHY_DLLGANE_CTL, tmp);

	/* Set DllLockParam */
	tmp = PREP_FIELD2(PHY_DLLLOCK_PARAM, DIS_DLLGAIN_IVSEED, 1, LCD_LSEED0, 0x21);
	phy_write32(ctrl_id, PHY_DLLLOCK_PARAM, tmp);
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

	phy_write32(ctrl_id, PHY_PROCODT_TIMECTL, tmp1);

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
	tmp1 = PREP_FIELD2(PHY_TX_ODT_DRV_STREN, ODT_STREN_P, tmp2, ODT_STREN_N, 0);

	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++) {
		phy_write32(ctrl_id, PHY_TX_ODT_DRV_STREN_B0(i), tmp1);
		phy_write32(ctrl_id, PHY_TX_ODT_DRV_STREN_B1(i), tmp1);
	}

	/* Set TxImpedanceCtrl1 */
#ifdef CONFIG_TX_IMPEDANCE_240
	tmp2 = 0x2;
#elif CONFIG_TX_IMPEDANCE_120
	tmp2 = 0x8;
#elif CONFIG_TX_IMPEDANCE_80
	tmp2 = 0xa;
#elif CONFIG_TX_IMPEDANCE_60
	tmp2 = 0x18;
#elif CONFIG_TX_IMPEDANCE_40
	tmp2 = 0x38;
#elif CONFIG_TX_IMPEDANCE_34
	tmp2 = 0x3a;
#elif CONFIG_TX_IMPEDANCE_HIZ
	tmp2 = 0;
#endif
	tmp1 = PREP_FIELD2(PHY_TX_IMPEDANCE_CTRL1, DRV_STREN_FSDQ_P, tmp2, DRV_STREN_FSDQ_N, tmp2);

	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++) {
		phy_write32(ctrl_id, PHY_TX_IMPEDANCE_CTRL1_B0(i), tmp1);
		phy_write32(ctrl_id, PHY_TX_IMPEDANCE_CTRL1_B1(i), tmp1);
	}

	/* Set ATxImpedance */
#ifdef CONFIG_ATX_IMPEDANCE_120
	tmp2 = 0;
#elif CONFIG_ATX_IMPEDANCE_60
	tmp2 = 0x1;
#elif CONFIG_ATX_IMPEDANCE_40
	tmp2 = 0x3;
#elif CONFIG_ATX_IMPEDANCE_30
	tmp2 = 0x7;
#elif CONFIG_ATX_IMPEDANCE_24
	tmp2 = 0xf;
#elif CONFIG_ATX_IMPEDANCE_20
	tmp2 = 0x1f;
#endif
	tmp1 = PREP_FIELD2(PHY_ATX_IMPEDANCE, ADRV_STREN_P, tmp2, ADRV_STREN_N, tmp2);

	for (i = 0; i < CONFIG_PHY_ANIB_NUM; i++)
		phy_write32(ctrl_id, PHY_ATX_IMPEDANCE(i), tmp1);
}

static void dfi_init(int ctrl_id)
{
	/* Set DfiMode */
#ifdef CONFIG_PHY_DFI1_EXIST
	phy_write32(ctrl_id, PHY_DFI_MODE, 0x5);
#else
	phy_write32(ctrl_id, PHY_DFI_MODE, 0x1);
#endif

	/* Set DfiCAMode */
	phy_write32(ctrl_id, PHY_DFI_CA_MODE, 0x2);
}

static void calibration_init(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t tmp1, tmp2;

	/* Set CalDrvStr0 */
#ifdef CONFIG_PHY_CALIBRATION_EXT_RESISTOR_240
	tmp2 = 0;
#elif CONFIG_PHY_CALIBRATION_EXT_RESISTOR_120
	tmp2 = 1;
#else
	tmp2 = 2;
#endif
	tmp1 = PREP_FIELD2(PHY_CAL_DRV_STR0, PD50, tmp2, PU50, tmp2);
	phy_write32(ctrl_id, PHY_CAL_DRV_STR0, tmp1);

	/* Set CalUclkInfo */
	phy_write32(ctrl_id, PHY_CAL_UCLK_INFO, DIV_ROUND_UP(1000000, 2 * cfg->tck));

	/* Set CalRate */
	tmp1 = PREP_FIELD2(PHY_CAL_RATE, INTERVAL, CONFIG_PHY_CALIBRATION_INTERVAL, ONCE, 0);
	phy_write32(ctrl_id, PHY_CAL_RATE, tmp1);
}

void phy_init(int ctrl_id, struct ddr_cfg *cfg)
{
	uint32_t tmp1, tmp2;
	int i;

	/* Set TxSlewRate */
	tmp1 = PREP_FIELD3(PHY_TX_SLEW_RATE, PREDRV_MODE, 0x2, TXPRE_N, CONFIG_PHY_TXSLEW_FALL_DQ,
			   TXPRE_P, CONFIG_PHY_TXSLEW_RISE_DQ);

	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++) {
		phy_write32(ctrl_id, PHY_TX_SLEW_RATE_B0(i), tmp1);
		phy_write32(ctrl_id, PHY_TX_SLEW_RATE_B1(i), tmp1);
	}

	/* Set ATxSlewRate */
	for (i = 0; i < CONFIG_PHY_ANIB_NUM; i++) {
		tmp2 = (i == 4 || i == 5) ? 0 : 3;
		tmp1 = PREP_FIELD3(PHY_ATX_SLEW_RATE, PREDRV_MODE, tmp2, ATXPRE_N,
				   CONFIG_PHY_TXSLEW_FALL_AC, ATXPRE_P, CONFIG_PHY_TXSLEW_RISE_AC);
		phy_write32(ctrl_id, PHY_ATX_SLEW_RATE(i), tmp1);
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
	phy_write32(ctrl_id, PHY_PLLCTRL2, tmp1);

	/* Set ARdPtrInitVal */
	if (cfg->tck <= DRAM_TCK_1866)
		tmp1 = 0x2;
	else
		tmp1 = 0x1;
	phy_write32(ctrl_id, PHY_ARDPTR_INITVAL, tmp1);

	/* Set DqsPreambleControl */
	tmp1 = PREP_FIELD3(PHY_DQS_PREAMBLE_CTL, TWOTCK_RX_DQSPRE, CONFIG_PHY_D4RX_PREAMBLE_LEN,
			   TWOTCK_TX_DQSPRE, CONFIG_PHY_D4TX_PREAMBLE_LEN, POSITION_DFE_INIT, 2);
	phy_write32(ctrl_id, PHY_DQS_PREAMBLE_CTL, tmp1);

	dll_init(ctrl_id);
	odt_init(ctrl_id, cfg);
	dfi_init(ctrl_id);
	calibration_init(ctrl_id, cfg);

	/* TBD: Set following regs depending on DDR configuration */

	phy_write32(ctrl_id, PHY_VREF_IN_GLOBAL, 0x208);

	for (i = 0; i < CONFIG_PHY_DBYTE_NUM; i++) {
		phy_write32(ctrl_id, PHY_DQDQS_RCV_CNTRL_B0(i), 0x5b1);
		phy_write32(ctrl_id, PHY_DQDQS_RCV_CNTRL_B1(i), 0x5b1);
	}

	phy_write32(ctrl_id, PHY_DFI_FREQ_RATIO, 1);
	phy_write32(ctrl_id, PHY_TRISTATE_MODE_CA, 5);

	for (i = 0; i < 7; i++)
		phy_write32(ctrl_id, PHY_DFI_FREQ_XLAT(i), 0x5555);

	phy_write32(ctrl_id, PHY_DFI_FREQ_XLAT(7), 0xF000);

	phy_write32(ctrl_id, PHY_MASTER_X4_CONFIG, 0);
	phy_write32(ctrl_id, PHY_DMI_PIN_PRESENT, 0);
	phy_write32(ctrl_id, PHY_ACX4_ANIB_DIS, 0);
}

void phy_training_params_load(int ctrl_id, struct ddr_cfg *cfg)
{
	PMU_SMB_DDR4U_1D_t mb_DDR4U_1D;

	memset(&mb_DDR4U_1D, 0, sizeof(mb_DDR4U_1D));

	mb_DDR4U_1D.Reserved00 = 0x60;
	mb_DDR4U_1D.DramType = 0x2;
	mb_DDR4U_1D.Pstate = 0;
	mb_DDR4U_1D.SequenceCtrl = 0x31f;
	mb_DDR4U_1D.PhyConfigOverride = 0;
#ifdef CONFIG_DEBUG
	mb_DDR4U_1D.HdtCtrl = 0x00;
#else
	mb_DDR4U_1D.HdtCtrl = 0xff;
#endif
	mb_DDR4U_1D.MsgMisc = 0;
	mb_DDR4U_1D.DFIMRLMargin = 1;
	mb_DDR4U_1D.PhyVref = 0x56;

	mb_DDR4U_1D.CsPresent = BIT(cfg->ranks) - 1;
	mb_DDR4U_1D.CsPresentD0 = BIT(cfg->ranks) - 1;
	mb_DDR4U_1D.CsPresentD1 = 0;
	mb_DDR4U_1D.AddrMirror = CONFIG_PHY_ADDR_MIRROR;

	mb_DDR4U_1D.AcsmOdtCtrl0 = 0x21; /* Needs to be verified */
	mb_DDR4U_1D.AcsmOdtCtrl1 = 0x12;

	mb_DDR4U_1D.EnabledDQs = CONFIG_DDRMC_DQ_BUS_WIDTH;
	mb_DDR4U_1D.PhyCfg = 0;
	mb_DDR4U_1D.X16Present = 0;
	mb_DDR4U_1D.D4Misc = 1;
	mb_DDR4U_1D.CsSetupGDDec = 1;

	mb_DDR4U_1D.MR0 = ddr4_mr0_get(cfg);
	mb_DDR4U_1D.MR1 = ddr4_mr1_get(cfg);
	mb_DDR4U_1D.MR2 = ddr4_mr2_get(cfg);
	mb_DDR4U_1D.MR3 = ddr4_mr3_get(cfg);
	mb_DDR4U_1D.MR4 = ddr4_mr4_get(cfg);
	mb_DDR4U_1D.MR5 = ddr4_mr5_get(cfg);
	mb_DDR4U_1D.MR6 = ddr4_mr6_get(cfg);

	mb_DDR4U_1D.ALT_CAS_L = 0;
	mb_DDR4U_1D.ALT_WCAS_L = 0;

	mb_DDR4U_1D.Share2DVrefResult = 0x1;

	unsigned long read_offset = (unsigned long)&mb_DDR4U_1D;
	uint32_t val, write_offset = CONFIG_PHY_DMEM_OFFSET;
	int i;

	/* Load training firmware parameters */
	for (i = 0; i < 254; i++) {
		val = read32(read_offset);
		phy_write32(ctrl_id, write_offset, val & 0x0000ffff);
		phy_write32(ctrl_id, write_offset + 4, (val >> 16) & 0x0000ffff);
		write_offset += 8;
		read_offset += 4;
	}

	phy_write16(ctrl_id, write_offset, mb_DDR4U_1D.D4Misc);
}
