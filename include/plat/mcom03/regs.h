/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2021 RnD Center "ELVEES", JSC */

#ifndef _REGS_MCOM03_H
#define _REGS_MCOM03_H

#include <regs.h>

#ifdef CONFIG_ARCH_MIPS32
#define ARCH_OFFSET 0xA0000000
#else
#define ARCH_OFFSET 0
#endif

#define DDR_SUBS_URB_BASE    (CONFIG_DDRSUB0_BASE + 0x8000000)
#define DDR_SUBS_UCG_BASE(i) (CONFIG_DDRSUB0_BASE + 0x8010000 + (i) * 0x10000)

#define LSPERIPH1_URB_BASE    (ARCH_OFFSET + 0x17E0000)
#define LSPERIPH1_GPIO_BASE   (ARCH_OFFSET + 0x1780000)
#define LSPERIPH1_TIMERS_BASE (ARCH_OFFSET + 0x1790000)
#define LSPERIPH1_UCG_BASE    (ARCH_OFFSET + 0x17C0000)

#define SERVICE_SUBS_URB_BASE	 (ARCH_OFFSET + 0x1F000000)
#define SERVICE_SUBS_URB_PLL	 (SERVICE_SUBS_URB_BASE + 0x1000)
#define SERVICE_SUBS_UCG_BASE	 (SERVICE_SUBS_URB_BASE + 0x20000)
#define SERVICE_SUBS_UCG_CHAN(i) (SERVICE_SUBS_UCG_BASE + (i) * 0x4)

#define SERVICE_SUBS_VMMU_BASE (ARCH_OFFSET + 0x1FD04000)

#define DDR_PLL_CFG(i)	 (DDR_SUBS_URB_BASE + (i) * 0x8)
#define DDR_PLL_CFG_SEL	 GENMASK(7, 0)
#define DDR_PLL_CFG_MAN	 BIT(9)
#define DDR_PLL_CFG_OD	 GENMASK(13, 10)
#define DDR_PLL_CFG_NF	 GENMASK(26, 14)
#define DDR_PLL_CFG_NR	 GENMASK(30, 27)
#define DDR_PLL_CFG_LOCK BIT(31)

#define UCG_CTR(ucg_addr, chan) ((ucg_addr) + (chan) * 0x4)
#define UCG_CTR_LPI_EN		BIT(0)
#define UCG_CTR_CLK_EN		BIT(1)
#define UCG_CTR_CLK_EN_STS	GENMASK(4, 2)
#define UCG_CTR_QACTIVE_CTL_EN	BIT(6)
#define UCG_CTR_QFSM_STATE	GENMASK(9, 7)
#define UCG_CTR_DIV_COEFF	GENMASK(29, 10)
#define UCG_CTR_DIV_LOCK	BIT(30)

#define UCG_BP(ucg_addr)   ((ucg_addr) + 0x40)
#define UCG_SYNC(ucg_addr) ((ucg_addr) + 0x44)

#define DDR_AXI_CHS_ENABLE	   (DDR_SUBS_URB_BASE + 0x14)
#define DDR_AXI_CHS_ENABLE_DDR0	   GENMASK(10, 0)
#define DDR_AXI_CHS_ENABLE_DDR1	   GENMASK(26, 16)
#define DDR_AXI_CHS_ENABLE_SDR	   BIT(0)
#define DDR_AXI_CHS_ENABLE_PCIE	   BIT(1)
#define DDR_AXI_CHS_ENABLE_ISP	   BIT(2)
#define DDR_AXI_CHS_ENABLE_GPU	   BIT(3)
#define DDR_AXI_CHS_ENABLE_VPU	   BIT(4)
#define DDR_AXI_CHS_ENABLE_DP	   BIT(5)
#define DDR_AXI_CHS_ENABLE_CPU	   BIT(6)
#define DDR_AXI_CHS_ENABLE_SERVICE BIT(7)
#define DDR_AXI_CHS_ENABLE_HSP	   BIT(8)
#define DDR_AXI_CHS_ENABLE_LSP0	   BIT(9)
#define DDR_AXI_CHS_ENABLE_LSP1	   BIT(10)

#define DDR_XDECODER_MODE (DDR_SUBS_URB_BASE + 0x10)

#define DDR_RESETN_PPOLICY(i)  (DDR_SUBS_URB_BASE + 0x60 + (i) * 0x10)
#define DDR_RESETN_PSTATUS(i)  (DDR_SUBS_URB_BASE + 0x64 + (i) * 0x10)
#define DDR_PRESETN_PPOLICY(i) (DDR_SUBS_URB_BASE + 0x68 + (i) * 0x10)
#define DDR_PRESETN_PSTATUS(i) (DDR_SUBS_URB_BASE + 0x6C + (i) * 0x10)

#define RESET_PPOLICY_PP_OFF	  BIT(0)
#define RESET_PPOLICY_PP_WARM_RST BIT(3)
#define RESET_PPOLICY_PP_ON	  BIT(4)

#define SERVICE_X_PPOLICY(i) (SERVICE_SUBS_URB_BASE + (i) * 0x8)
#define SERVICE_X_PSTATUS(i) (SERVICE_SUBS_URB_BASE + 0x4 + (i) * 0x8)

#define SERVICE_TOP_CLK_GATE	       (SERVICE_SUBS_URB_BASE + 0x1008)
#define SERVICE_TOP_CLK_GATE_SDR       BIT(3)
#define SERVICE_TOP_CLK_GATE_MEDIA     BIT(1)
#define SERVICE_TOP_CLK_GATE_LSPERIPH0 BIT(5)
#define SERVICE_TOP_CLK_GATE_LSPERIPH1 BIT(6)
#define SERVICE_TOP_CLK_GATE_DDR       BIT(7)

#define LSPERIPH1_GPIO_SWPORTB_DR  (LSPERIPH1_GPIO_BASE + 0xC)
#define LSPERIPH1_GPIO_SWPORTB_DDR (LSPERIPH1_GPIO_BASE + 0x10)
#define LSPERIPH1_GPIO_SWPORTB_CTL (LSPERIPH1_GPIO_BASE + 0x14)
#define LSPERIPH1_GPIO_UART0_SOUT  BIT(6)
#define LSPERIPH1_GPIO_UART0_SIN   BIT(7)
#define GPIO_INPUT_DIRECTION	   0x0
#define GPIO_OUTPUT_DIRECTION	   0x1

#define LSPERIPH1_GPIO_PORTBN_PADCTR(i)	 (LSPERIPH1_URB_BASE + 0x40 + (i) * 0x4)
#define LSPERIPH1_GPIO_PORTBN_PADCTR_SUS BIT(0)
#define LSPERIPH1_GPIO_PORTBN_PADCTR_PU	 BIT(1)
#define LSPERIPH1_GPIO_PORTBN_PADCTR_PD	 BIT(2)
#define LSPERIPH1_GPIO_PORTBN_PADCTR_SL	 GENMASK(4, 3)
#define LSPERIPH1_GPIO_PORTBN_PADCTR_CTL GENMASK(10, 5)
#define LSPERIPH1_GPIO_PORTBN_PADCTR_E	 BIT(12)
#define LSPERIPH1_GPIO_PORTBN_PADCTR_CLE BIT(13)
#define LSPERIPH1_GPIO_PORTBN_PADCTR_OD	 BIT(14)

#define LSPERIPH1_UCG_CTR(i) (LSPERIPH1_UCG_BASE + (i) * 0x4)

enum pad_driver_strength {
	PAD_DRIVER_STREGTH_2mA = 0x1,
	PAD_DRIVER_STREGTH_4mA = 0x3,
	PAD_DRIVER_STREGTH_6mA = 0x7,
	PAD_DRIVER_STREGTH_8mA = 0xf,
	PAD_DRIVER_STREGTH_10mA = 0x1f,
	PAD_DRIVER_STREGTH_12mA = 0x3f
};

#define DW_APB_TIMER(i)		(LSPERIPH1_TIMERS_BASE + (i) * 0x14)
#define DW_APB_LOAD_COUNT(i)	(DW_APB_TIMER(i) + 0x0)
#define DW_APB_CURRENT_VALUE(i) (DW_APB_TIMER(i) + 0x4)
#define DW_APB_CTRL(i)		(DW_APB_TIMER(i) + 0x8)

#endif /* _REGS_MCOM03_H */
