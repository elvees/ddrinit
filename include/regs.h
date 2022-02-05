/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2020 RnD Center "ELVEES", JSC */

#ifndef _REGS_H
#define _REGS_H

#include <bitops.h>

/* DDR controller registers */

#define DDRSUB_BASE(i) (CONFIG_DDRSUB0_BASE + (i)*CONFIG_DDRSUB_GAP)
#define DDRMC_BASE(i)  (DDRSUB_BASE(i) + CONFIG_DDRMC_OFFSET)
#define PHY_BASE(i)    (DDRSUB_BASE(i) + CONFIG_PHY_OFFSET)

#define DDRMC_MSTR(i)	      (DDRMC_BASE(i) + 0x0)
#define DDRMC_MSTR_DDR4	      BIT(4)
#define DDRMC_MSTR_LPDDR4     BIT(5)
#define DDRMC_MSTR_BURST_RDWR GENMASK(19, 16)
#define DDRMC_MSTR_LOG_RANKS  GENMASK(21, 20)
#define DDRMC_MSTR_RANKS      GENMASK(27, 24)
#define DDRMC_MSTR_DEVCONFIG  GENMASK(31, 30)

#define DDRMC_INIT0(i)		   (DDRMC_BASE(i) + 0xD0)
#define DDRMC_INIT0_PRECKE	   GENMASK(11, 0)
#define DDRMC_INIT0_POSTCKE	   GENMASK(25, 16)
#define DDRMC_INIT0_SKIP_DRAM_INIT GENMASK(31, 30)

#define DDRMC_INIT1(i)	      (DDRMC_BASE(i) + 0xD4)
#define DDRMC_INIT1_DRAM_RSTN GENMASK(24, 16)

#define DDRMC_INIT3(i)	(DDRMC_BASE(i) + 0xDC)
#define DDRMC_INIT3_MR	GENMASK(31, 16)
#define DDRMC_INIT3_EMR GENMASK(15, 0)

#define DDRMC_INIT4(i)	 (DDRMC_BASE(i) + 0xE0)
#define DDRMC_INIT4_EMR2 GENMASK(31, 16)
#define DDRMC_INIT4_EMR3 GENMASK(15, 0)

#define DDRMC_INIT5(i)	   (DDRMC_BASE(i) + 0xE4)
#define DDRMC_INIT5_ZQINIT GENMASK(23, 16)

#define DDRMC_INIT6(i)	(DDRMC_BASE(i) + 0xE8)
#define DDRMC_INIT6_MR4 GENMASK(31, 16)
#define DDRMC_INIT6_MR5 GENMASK(15, 0)

#define DDRMC_INIT7(i)	(DDRMC_BASE(i) + 0xEC)
#define DDRMC_INIT7_MR6 GENMASK(15, 0)

#define DDRMC_RFSHTMG(i)    (DDRMC_BASE(i) + 0x64)
#define DDRMC_RFSHTMG_TRFC  GENMASK(9, 0)
#define DDRMC_RFSHTMG_TREFI GENMASK(27, 16)

#define DDRMC_ZQCTL0(i)	     (DDRMC_BASE(i) + 0x180)
#define DDRMC_ZQCTL0_TZQCS   GENMASK(9, 0)
#define DDRMC_ZQCTL0_TZQOPER GENMASK(26, 16)

#define DDRMC_ZQCTL1(i)		    (DDRMC_BASE(i) + 0x184)
#define DDRMC_ZQCTL1_TZQCS_INTERVAL GENMASK(19, 0)
#define DDRMC_ZQCTL1_TZQ_RESET_NOP  GENMASK(29, 20)

#define DDRMC_DRAMTMG0(i)      (DDRMC_BASE(i) + 0x100)
#define DDRMC_DRAMTMG0_TRASMIN GENMASK(5, 0)
#define DDRMC_DRAMTMG0_TRASMAX GENMASK(14, 8)
#define DDRMC_DRAMTMG0_TFAW    GENMASK(21, 16)
#define DDRMC_DRAMTMG0_WR2PRE  GENMASK(30, 24)

#define DDRMC_DRAMTMG1(i)     (DDRMC_BASE(i) + 0x104)
#define DDRMC_DRAMTMG1_TRC    GENMASK(6, 0)
#define DDRMC_DRAMTMG1_RD2PRE GENMASK(13, 8)
#define DDRMC_DRAMTMG1_TXP    GENMASK(20, 16)

#define DDRMC_DRAMTMG2(i)    (DDRMC_BASE(i) + 0x108)
#define DDRMC_DRAMTMG2_WR2RD GENMASK(5, 0)
#define DDRMC_DRAMTMG2_RD2WR GENMASK(13, 8)
#define DDRMC_DRAMTMG2_RDLAT GENMASK(21, 16)
#define DDRMC_DRAMTMG2_WRLAT GENMASK(29, 24)

#define DDRMC_DRAMTMG3(i)   (DDRMC_BASE(i) + 0x10C)
#define DDRMC_DRAMTMG3_TMOD GENMASK(9, 0)
#define DDRMC_DRAMTMG3_TMRD GENMASK(17, 12)
#define DDRMC_DRAMTMG3_TMRW GENMASK(29, 20)

#define DDRMC_DRAMTMG4(i)   (DDRMC_BASE(i) + 0x110)
#define DDRMC_DRAMTMG4_TRP  GENMASK(4, 0)
#define DDRMC_DRAMTMG4_TRRD GENMASK(11, 8)
#define DDRMC_DRAMTMG4_TCCD GENMASK(19, 16)
#define DDRMC_DRAMTMG4_TRCD GENMASK(28, 24)

#define DDRMC_DRAMTMG5(i)     (DDRMC_BASE(i) + 0x114)
#define DDRMC_DRAMTMG5_TCKE   GENMASK(3, 0)
#define DDRMC_DRAMTMG5_TCKESR GENMASK(13, 8)
#define DDRMC_DRAMTMG5_TCKSRE GENMASK(19, 16)
#define DDRMC_DRAMTMG5_TCKSRX GENMASK(27, 24)

#define DDRMC_DRAMTMG6(i)      (DDRMC_BASE(i) + 0x118)
#define DDRMC_DRAMTMG6_TCKCSX  GENMASK(3, 0)
#define DDRMC_DRAMTMG6_TCKDPDX GENMASK(19, 16)
#define DDRMC_DRAMTMG6_TCKDPDE GENMASK(27, 24)

#define DDRMC_DRAMTMG7(i)     (DDRMC_BASE(i) + 0x11C)
#define DDRMC_DRAMTMG7_TCKPDX GENMASK(3, 0)
#define DDRMC_DRAMTMG7_TCKPDE GENMASK(11, 8)

#define DDRMC_DRAMTMG8(i)	(DDRMC_BASE(i) + 0x120)
#define DDRMC_DRAMTMG8_TXS	GENMASK(6, 0)
#define DDRMC_DRAMTMG8_TXSDLL	GENMASK(14, 8)
#define DDRMC_DRAMTMG8_TXSABORT GENMASK(22, 16)
#define DDRMC_DRAMTMG8_TXSFAST	GENMASK(30, 24)

#define DDRMC_DRAMTMG9(i)     (DDRMC_BASE(i) + 0x124)
#define DDRMC_DRAMTMG9_WR2RDS GENMASK(5, 0)
#define DDRMC_DRAMTMG9_TRRDS  GENMASK(11, 8)
#define DDRMC_DRAMTMG9_TCCDS  GENMASK(18, 16)

#define DDRMC_DRAMTMG13(i)	(DDRMC_BASE(i) + 0x134)
#define DDRMC_DRAMTMG13_TPPD	GENMASK(2, 0)
#define DDRMC_DRAMTMG13_TCCDMW	GENMASK(21, 16)
#define DDRMC_DRAMTMG13_ODTLOFF GENMASK(30, 24)

#define DDRMC_DRAMTMG14(i)   (DDRMC_BASE(i) + 0x138)
#define DDRMC_DRAMTMG14_TXSR GENMASK(11, 0)

#define DDRMC_DFITMG0(i)	     (DDRMC_BASE(i) + 0x190)
#define DDRMC_DFITMG0_WRLAT	     GENMASK(5, 0)
#define DDRMC_DFITMG0_WRDATA	     GENMASK(13, 8)
#define DDRMC_DFITMG0_WRDATA_DFICLK  BIT(15)
#define DDRMC_DFITMG0_RDDATAEN	     GENMASK(22, 16)
#define DDRMC_DFITMG0_RDDATA_DFI_CLK BIT(23)
#define DDRMC_DFITMG0_CTRLDELAY	     GENMASK(28, 24)

#define DDRMC_DFITMG1(i)	   (DDRMC_BASE(i) + 0x194)
#define DDRMC_DFITMG1_DRAM_CLK_EN  GENMASK(4, 0)
#define DDRMC_DFITMG1_DRAM_CLK_DIS GENMASK(12, 8)
#define DDRMC_DFITMG1_WRDATA_DELAY GENMASK(20, 16)

#define DDRMC_DFITMG2(i)      (DDRMC_BASE(i) + 0x1B4)
#define DDRMC_DFITMG2_WRCSLAT GENMASK(5, 0)
#define DDRMC_DFITMG2_RDCSLAT GENMASK(14, 8)

#define DDRMC_DFIUPD0(i)	  (DDRMC_BASE(i) + 0x1A0)
#define DDRMC_DFIUPD0_CTRLUPD_MIN GENMASK(9, 0)
#define DDRMC_DFIUPD0_CTRLUPD_MAX GENMASK(25, 16)

#define DDRMC_DFIUPD1(i)	     (DDRMC_BASE(i) + 0x1A4)
#define DDRMC_DFIUPD1_CTRLUPD_INTMAX GENMASK(7, 0)
#define DDRMC_DFIUPD1_CTRLUPD_INTMIN GENMASK(23, 16)

#define DDRMC_ADDRMAP0(i)    (DDRMC_BASE(i) + 0x200)
#define DDRMC_ADDRMAP0_CS_B0 GENMASK(4, 0)
#define DDRMC_ADDRMAP0_CS_B1 GENMASK(12, 8)

#define DDRMC_ADDRMAP1(i)      (DDRMC_BASE(i) + 0x204)
#define DDRMC_ADDRMAP1_BANK_B0 GENMASK(5, 0)
#define DDRMC_ADDRMAP1_BANK_B1 GENMASK(13, 8)
#define DDRMC_ADDRMAP1_BANK_B2 GENMASK(21, 16)

#define DDRMC_ADDRMAP2(i) (DDRMC_BASE(i) + 0x208)
#define DDRMC_ADDRMAP3(i) (DDRMC_BASE(i) + 0x20C)

#define DDRMC_ADDRMAP4(i)      (DDRMC_BASE(i) + 0x210)
#define DDRMC_ADDRMAP4_COL_B10 GENMASK(4, 0)
#define DDRMC_ADDRMAP4_COL_B11 GENMASK(12, 8)

#define DDRMC_ADDRMAP5(i)	  (DDRMC_BASE(i) + 0x214)
#define DDRMC_ADDRMAP5_ROW_B0	  GENMASK(3, 0)
#define DDRMC_ADDRMAP5_ROW_B1	  GENMASK(11, 8)
#define DDRMC_ADDRMAP5_ROW_B2_B10 GENMASK(19, 16)
#define DDRMC_ADDRMAP5_ROW_B11	  GENMASK(27, 24)

#define DDRMC_ADDRMAP6(i)      (DDRMC_BASE(i) + 0x218)
#define DDRMC_ADDRMAP6_ROW_B12 GENMASK(3, 0)
#define DDRMC_ADDRMAP6_ROW_B13 GENMASK(11, 8)
#define DDRMC_ADDRMAP6_ROW_B14 GENMASK(19, 16)
#define DDRMC_ADDRMAP6_ROW_B15 GENMASK(27, 24)

#define DDRMC_ADDRMAP7(i)      (DDRMC_BASE(i) + 0x21C)
#define DDRMC_ADDRMAP7_ROW_B16 GENMASK(3, 0)
#define DDRMC_ADDRMAP7_ROW_B17 GENMASK(11, 8)

#define DDRMC_ADDRMAP8(i)    (DDRMC_BASE(i) + 0x220)
#define DDRMC_ADDRMAP8_BG_B0 GENMASK(5, 0)
#define DDRMC_ADDRMAP8_BG_B1 GENMASK(13, 8)

#define DDRMC_ODTCFG(i)	      (DDRMC_BASE(i) + 0x240)
#define DDRMC_ODTCFG_RD_DELAY GENMASK(6, 2)
#define DDRMC_ODTCFG_RD_HOLD  GENMASK(11, 8)
#define DDRMC_ODTCFG_WR_DELAY GENMASK(20, 16)
#define DDRMC_ODTCFG_WR_HOLD  GENMASK(27, 24)

#define DDRMC_ODTMAP(i) (DDRMC_BASE(i) + 0x244)

#define DDRMC_RFSHCTL3(i)		(DDRMC_BASE(i) + 0x60)
#define DDRMC_RFSHCTL3_DIS_AUTO_REFRESH BIT(0)

#define DDRMC_PWRCTL(i)		     (DDRMC_BASE(i) + 0x30)
#define DDRMC_PWRCTL_SELF_REFRESH_EN BIT(0)
#define DDRMC_PWRCTL_POWERDOWN_EN    BIT(1)
#define DDRMC_PWRCTL_DFI_DRAMCLK_DIS BIT(3)
#define DDRMC_PWRCTL_SELFREF_SW	     BIT(5)

#define DDRMC_SWCTL(i) (DDRMC_BASE(i) + 0x320)

#define DDRMC_SWSTAT(i)		(DDRMC_BASE(i) + 0x324)
#define DDRMC_SWSTAT_SWDONE_ACK BIT(0)

#define DDRMC_DFIMISC(i)	  (DDRMC_BASE(i) + 0x1B0)
#define DDRMC_DFIMISC_COMPLETE_EN BIT(0)
#define DDRMC_DFIMISC_INIT_START  BIT(5)

#define DDRMC_DFISTAT(i)	    (DDRMC_BASE(i) + 0x1BC)
#define DDRMC_DFISTAT_INIT_COMPLETE BIT(0)

#define DDRMC_DBICTL(i)	       (DDRMC_BASE(i) + 0x1C0)
#define DDRMC_DBICTL_WR_DBI_EN BIT(1)
#define DDRMC_DBICTL_RD_DBI_EN BIT(2)

#define DDRMC_STAT(i)	     (DDRMC_BASE(i) + 0x4)
#define DDRMC_STAT_OPER_MODE GENMASK(2, 0)

#define DDRMC_PCTRL(i, j)   (DDRMC_BASE(i) + 0x490 + 0xb0 * (j))
#define DDRMC_SARBASE(i, j) (DDRMC_BASE(i) + 0xf04 + 8 * (j))
#define DDRMC_SARSIZE(i, j) (DDRMC_BASE(i) + 0xf08 + 8 * (j))

/* PHY registers */

#define PREP_FIELD(REG, FIELD, VAL) (((VAL) << REG##_##FIELD##_SHIFT) & REG##_##FIELD)

#define PREP_FIELD2(REG, FIELD1, VAL1, FIELD2, VAL2)           \
	(((VAL1) << REG##_##FIELD1##_SHIFT & REG##_##FIELD1) | \
	 ((VAL2) << REG##_##FIELD2##_SHIFT & REG##_##FIELD2))

#define PREP_FIELD3(REG, FIELD1, VAL1, FIELD2, VAL2, FIELD3, VAL3) \
	(((VAL1) << REG##_##FIELD1##_SHIFT & REG##_##FIELD1) |     \
	 ((VAL2) << REG##_##FIELD2##_SHIFT & REG##_##FIELD2) |     \
	 ((VAL3) << REG##_##FIELD3##_SHIFT & REG##_##FIELD3))

#define PHYREG(BLOCK, INSTANCE, REG) \
	(CONFIG_PHY_##BLOCK##_OFFSET + (CONFIG_PHY_REG_INSTANCE_OFFSET * (INSTANCE) + (REG)) * 4)

#define PHY_TX_SLEW_RATE_B0(i)		   PHYREG(DBYTE, (i), 0x5F)
#define PHY_TX_SLEW_RATE_B1(i)		   PHYREG(DBYTE, (i), 0x15F)
#define PHY_TX_SLEW_RATE_TXPRE_P	   GENMASK(3, 0)
#define PHY_TX_SLEW_RATE_TXPRE_P_SHIFT	   0
#define PHY_TX_SLEW_RATE_TXPRE_N	   GENMASK(7, 4)
#define PHY_TX_SLEW_RATE_TXPRE_N_SHIFT	   4
#define PHY_TX_SLEW_RATE_PREDRV_MODE	   GENMASK(10, 8)
#define PHY_TX_SLEW_RATE_PREDRV_MODE_SHIFT 8

#define PHY_TX_ODT_DRV_STREN_B0(i)	       PHYREG(DBYTE, (i), 0x4D)
#define PHY_TX_ODT_DRV_STREN_B1(i)	       PHYREG(DBYTE, (i), 0x14D)
#define PHY_TX_ODT_DRV_STREN_ODT_STREN_P       GENMASK(5, 0)
#define PHY_TX_ODT_DRV_STREN_ODT_STREN_P_SHIFT 0
#define PHY_TX_ODT_DRV_STREN_ODT_STREN_N       GENMASK(11, 6)
#define PHY_TX_ODT_DRV_STREN_ODT_STREN_N_SHIFT 6

#define PHY_DQ_DQS_RCV_CNTRL1(i) PHYREG(DBYTE, (i), 0x4A)

#define PHY_TX_IMPEDANCE_CTRL1_B0(i)		      PHYREG(DBYTE, (i), 0x49)
#define PHY_TX_IMPEDANCE_CTRL1_B1(i)		      PHYREG(DBYTE, (i), 0x149)
#define PHY_TX_IMPEDANCE_CTRL1_DRV_STREN_FSDQ_P	      GENMASK(5, 0)
#define PHY_TX_IMPEDANCE_CTRL1_DRV_STREN_FSDQ_P_SHIFT 0
#define PHY_TX_IMPEDANCE_CTRL1_DRV_STREN_FSDQ_N	      GENMASK(11, 6)
#define PHY_TX_IMPEDANCE_CTRL1_DRV_STREN_FSDQ_N_SHIFT 6

#define PHY_DQDQS_RCV_CNTRL_B0(i) PHYREG(DBYTE, (i), 0x43)
#define PHY_DQDQS_RCV_CNTRL_B1(i) PHYREG(DBYTE, (i), 0x143)

#define PHY_DQN_LN_SEL(i, j) (PHYREG(DBYTE, (i), 0xA0) + (j)*4)

#define PHY_ATX_SLEW_RATE(i)		    PHYREG(ANIB, i, 0x55)
#define PHY_ATX_SLEW_RATE_ATXPRE_P	    GENMASK(3, 0)
#define PHY_ATX_SLEW_RATE_ATXPRE_P_SHIFT    0
#define PHY_ATX_SLEW_RATE_ATXPRE_N	    GENMASK(7, 4)
#define PHY_ATX_SLEW_RATE_ATXPRE_N_SHIFT    4
#define PHY_ATX_SLEW_RATE_PREDRV_MODE	    GENMASK(10, 8)
#define PHY_ATX_SLEW_RATE_PREDRV_MODE_SHIFT 8

#define PHY_ATX_IMPEDANCE(i)		     PHYREG(ANIB, (i), 0x43)
#define PHY_ATX_IMPEDANCE_ADRV_STREN_P	     GENMASK(4, 0)
#define PHY_ATX_IMPEDANCE_ADRV_STREN_P_SHIFT 0
#define PHY_ATX_IMPEDANCE_ADRV_STREN_N	     GENMASK(9, 5)
#define PHY_ATX_IMPEDANCE_ADRV_STREN_N_SHIFT 5

#define PHY_CAL_UCLK_INFO PHYREG(MASTER, 0, 0x8)

#define PHY_SEQ0BDLY0 PHYREG(MASTER, 0, 0xb)
#define PHY_SEQ0BDLY1 PHYREG(MASTER, 0, 0xc)
#define PHY_SEQ0BDLY2 PHYREG(MASTER, 0, 0xd)
#define PHY_SEQ0BDLY3 PHYREG(MASTER, 0, 0xe)

#define PHY_DFI_MODE PHYREG(MASTER, 0, 0x18)

#define PHY_TRISTATE_MODE_CA PHYREG(MASTER, 0, 0x19)

#define PHY_DQS_PREAMBLE_CTL				    PHYREG(MASTER, 0, 0x24)
#define PHY_DQS_PREAMBLE_CTL_TWOTCK_RX_DQSPRE		    BIT(0)
#define PHY_DQS_PREAMBLE_CTL_TWOTCK_RX_DQSPRE_SHIFT	    0
#define PHY_DQS_PREAMBLE_CTL_TWOTCK_TX_DQSPRE		    BIT(1)
#define PHY_DQS_PREAMBLE_CTL_TWOTCK_TX_DQSPRE_SHIFT	    1
#define PHY_DQS_PREAMBLE_CTL_POSITION_DFE_INIT		    GENMASK(4, 2)
#define PHY_DQS_PREAMBLE_CTL_POSITION_DFE_INIT_SHIFT	    2
#define PHY_DQS_PREAMBLE_CTL_LP4_TGL_TWOTCK_TXDQS_PRE	    BIT(5)
#define PHY_DQS_PREAMBLE_CTL_LP4_TGL_TWOTCK_TXDQS_PRE_SHIFT 5
#define PHY_DQS_PREAMBLE_CTL_LP4_POSTAMLE_EXT		    BIT(6)
#define PHY_DQS_PREAMBLE_CTL_LP4_POSTAMLE_EXT_SHIFT	    6
#define PHY_DQS_PREAMBLE_CTL_LP4_STTC_PREBRIDGE_RXEN	    BIT(7)
#define PHY_DQS_PREAMBLE_CTL_LP4_STTC_PREBRIDGE_RXEN_SHIFT  7
#define PHY_DQS_PREAMBLE_CTL_WDQS_EXT			    BIT(8)
#define PHY_DQS_PREAMBLE_CTL_WDQS_EXT_SHIFT		    8

#define PHY_MASTER_X4_CONFIG PHYREG(MASTER, 0, 0x25)

#define PHY_ACX4_ANIB_DIS PHYREG(MASTER, 0, 0x2C)

#define PHY_DMI_PIN_PRESENT PHYREG(MASTER, 0, 0x2D)

#define PHY_ARDPTR_INITVAL PHYREG(MASTER, 0, 0x2E)

#define PHY_DBYTE_DLLMODE_CTL				PHYREG(MASTER, 0, 0x3A)
#define PHY_DBYTE_DLLMODE_CTL_DLLRX_PREAMBLE_MODE	BIT(1)
#define PHY_DBYTE_DLLMODE_CTL_DLLRX_PREAMBLE_MODE_SHIFT 1

#define PHY_CALPEXT_OVR PHYREG(MASTER, 0, 0x49)

#define PHY_CALCMPR5_OVR PHYREG(MASTER, 0, 0x4A)

#define PHY_CALNINT_OVR PHYREG(MASTER, 0, 0x4B)

#define PHY_CAL_DRV_STR0	    PHYREG(MASTER, 0, 0x50)
#define PHY_CAL_DRV_STR0_PD50	    GENMASK(3, 0)
#define PHY_CAL_DRV_STR0_PD50_SHIFT 0
#define PHY_CAL_DRV_STR0_PU50	    GENMASK(7, 4)
#define PHY_CAL_DRV_STR0_PU50_SHIFT 4

#define PHY_PROCODT_TIMECTL PHYREG(MASTER, 0, 0x56)

#define PHY_MEM_ALERT_CONTROL PHYREG(MASTER, 0, 0x5B)

#define PHY_MEM_ALERT_CONTROL2 PHYREG(MASTER, 0, 0x5C)

#define PHY_MEMRESETL PHYREG(MASTER, 0, 0x60)

#define PHY_DFI_CA_MODE PHYREG(MASTER, 0, 0x75)

#define PHY_DLLGANE_CTL			 PHYREG(MASTER, 0, 0x7C)
#define PHY_DLLGANE_CTL_DLLGAIN_IV	 GENMASK(3, 0)
#define PHY_DLLGANE_CTL_DLLGAIN_IV_SHIFT 0
#define PHY_DLLGANE_CTL_DLLGAIN_TV	 GENMASK(7, 4)
#define PHY_DLLGANE_CTL_DLLGAIN_TV_SHIFT 4

#define PHY_DLLLOCK_PARAM			   PHYREG(MASTER, 0, 0x7D)
#define PHY_DLLLOCK_PARAM_DIS_DLLGAIN_IVSEED	   GENMASK(3, 1)
#define PHY_DLLLOCK_PARAM_DIS_DLLGAIN_IVSEED_SHIFT 1
#define PHY_DLLLOCK_PARAM_LCD_LSEED0		   GENMASK(15, 4)
#define PHY_DLLLOCK_PARAM_LCD_LSEED0_SHIFT	   4

#define PHY_CAL_RATE		    PHYREG(MASTER, 0, 0x88)
#define PHY_CAL_RATE_INTERVAL	    GENMASK(3, 0)
#define PHY_CAL_RATE_INTERVAL_SHIFT 0
#define PHY_CAL_RATE_RUN	    BIT(4)
#define PHY_CAL_RATE_RUN_SHIFT	    4
#define PHY_CAL_RATE_ONCE	    BIT(5)
#define PHY_CAL_RATE_ONCE_SHIFT	    5

#define PHY_CALMISC PHYREG(MASTER, 0, 0x9A)

#define PHY_CALCMPR5 PHYREG(MASTER, 0, 0x9C)

#define PHY_CALNINT PHYREG(MASTER, 0, 0x9D)

#define PHY_CALPEXT PHYREG(MASTER, 0, 0x9E)

#define PHY_VREF_IN_GLOBAL PHYREG(MASTER, 0, 0xB2)

#define PHY_PLLCTRL2 PHYREG(MASTER, 0, 0xC5)

#define PHY_DFI_FREQ_XLAT(i) PHYREG(MASTER, 0, 0xF0 + (i))

#define PHY_DFI_FREQ_RATIO PHYREG(MASTER, 0, 0xFA)

#define PHY_SEQ0BGPR4 PHYREG(PIE, 0, 0x204)

#define PHY_MICROCONT_MUXSEL PHYREG(APBONLY, 0, 0)

#define PHY_UCT_SHADOW_REGS PHYREG(APBONLY, 0, 4)

#define PHY_DCT_WRITE_PROT PHYREG(APBONLY, 0, 0x31)

#define PHY_UCT_WRITE_ONLY_SHADOW PHYREG(APBONLY, 0, 0x32)

#define PHY_UCT_DAT_WRITE_ONLY_SHADOW PHYREG(APBONLY, 0, 0x34)

#define PHY_MICRO_RESET PHYREG(APBONLY, 0, 0x99)

/* UART registers */

#define UART_THR CONFIG_UART_BASE

#define UART_DLL CONFIG_UART_BASE

#define UART_IER (CONFIG_UART_BASE + 0x4)

#define UART_DLH (CONFIG_UART_BASE + 0x4)

#define UART_FCR	(CONFIG_UART_BASE + 0x8)
#define UART_FCR_FIFOE	BIT(0)
#define UART_FCR_RFIFOR BIT(1)
#define UART_FCR_XFIFOR BIT(2)

#define UART_LCR      (CONFIG_UART_BASE + 0xC)
#define UART_LCR_DLAB BIT(7)

#define UART_MCR     (CONFIG_UART_BASE + 0x10)
#define UART_MCR_DTR BIT(0)
#define UART_MCR_RTS BIT(1)

#define UART_LSR      (CONFIG_UART_BASE + 0x14)
#define UART_LSR_THRE BIT(5)

/* I2C registers */

#define I2C_CON(b)		(b)
#define I2C_CON_SLAVE_DISABLE	BIT(6)
#define I2C_CON_RESTART_EN	BIT(5)
#define I2C_CON_SPEED_MASK_100	BIT(1)
#define I2C_CON_SPEED_MASK_FAST BIT(2)
#define I2C_CON_MASTER_MODE	BIT(0)

#define I2C_TAR(b) ((b) + 0x4)

#define I2C_DATA_CMD(b) ((b) + 0x10)

#define I2C_SS_HCNT(b) ((b) + 0x14)

#define I2C_SS_LCNT(b) ((b) + 0x18)

#define I2C_FS_HCNT(b) ((b) + 0x1c)

#define I2C_FS_LCNT(b) ((b) + 0x20)

#define I2C_ENABLE(b) ((b) + 0x6C)

#define I2C_STATUS(b) ((b) + 0x70)

#define I2C_TX_EMPTY	    BIT(2)
#define I2C_RX_DATA_PRESENT BIT(3)
#define I2C_MST_ACTIVITY    BIT(5)

#endif /* _REGS_H */
