/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2021 RnD Center "ELVEES", JSC
 */

#ifndef _REGS_MCOM03_H
#define _REGS_MCOM03_H

#include <regs.h>

#define DDR_SUBS_URB_BASE			(CONFIG_DDRSUB0_BASE + 0x8000000)
#define DDR_SUBS_UCG_BASE(i)			(CONFIG_DDRSUB0_BASE + 0x8010000 + (i)*0x10000)
#define SERVICE_SUBS_URB_BASE			0x1F000000

#define DDR_PLL_CFG(i)				(DDR_SUBS_URB_BASE + (i)*0x8)
#define DDR_PLL_CFG_SEL				GENMASK(7, 0)
#define DDR_PLL_CFG_MAN				BIT(9)
#define DDR_PLL_CFG_OD				GENMASK(13, 10)
#define DDR_PLL_CFG_NF				GENMASK(26, 14)
#define DDR_PLL_CFG_NR				GENMASK(30, 27)
#define DDR_PLL_CFG_LOCK			BIT(31)

#define DDR_UCG_CTR(i, j)			(DDR_SUBS_UCG_BASE(i) + (j)*0x4)
#define DDR_UCG_CTR_LPI_EN			BIT(0)
#define DDR_UCG_CTR_CLK_EN			BIT(1)
#define DDR_UCG_CTR_CLK_EN_STS			GENMASK(4, 2)
#define DDR_UCG_CTR_QACTIVE_CTL_EN		BIT(6)
#define DDR_UCG_CTR_QFSM_STATE			GENMASK(9, 7)
#define DDR_UCG_CTR_DIFF_COEFF			GENMASK(29, 10)
#define DDR_UCG_CTR_DIFF_LOCK			BIT(30)

#define DDR_UCG_BP(i)				(DDR_SUBS_UCG_BASE(i) + 0x40)

#define DDR_AXI_CHS_ENABLE			(DDR_SUBS_URB_BASE + 0x14)
#define DDR_AXI_CHS_ENABLE_DDR0			GENMASK(10, 0)
#define DDR_AXI_CHS_ENABLE_DDR1			GENMASK(26, 16)
#define DDR_AXI_CHS_ENABLE_SDR			BIT(0)
#define DDR_AXI_CHS_ENABLE_PCIE			BIT(1)
#define DDR_AXI_CHS_ENABLE_ISP			BIT(2)
#define DDR_AXI_CHS_ENABLE_GPU			BIT(3)
#define DDR_AXI_CHS_ENABLE_VPU			BIT(4)
#define DDR_AXI_CHS_ENABLE_DP			BIT(5)
#define DDR_AXI_CHS_ENABLE_CPU			BIT(6)
#define DDR_AXI_CHS_ENABLE_SERVICE		BIT(7)
#define DDR_AXI_CHS_ENABLE_HSP			BIT(8)
#define DDR_AXI_CHS_ENABLE_LSP0			BIT(9)
#define DDR_AXI_CHS_ENABLE_LSP1			BIT(10)

#define DDR_RESETN_PPOLICY(i)			(DDR_SUBS_URB_BASE + 0x60 + (i)*0x10)
#define DDR_RESETN_PSTATUS(i)			(DDR_SUBS_URB_BASE + 0x64 + (i)*0x10)
#define DDR_PRESETN_PPOLICY(i)			(DDR_SUBS_URB_BASE + 0x68 + (i)*0x10)
#define DDR_PRESETN_PSTATUS(i)			(DDR_SUBS_URB_BASE + 0x6C + (i)*0x10)

#define RESET_PPOLICY_PP_OFF			BIT(0)
#define RESET_PPOLICY_PP_WARM_RST		BIT(3)
#define RESET_PPOLICY_PP_ON			BIT(4)

#define SERVICE_X_PPOLICY(i)			(SERVICE_SUBS_URB_BASE + (i)*0x8)
#define SERVICE_X_PSTATUS(i)			(SERVICE_SUBS_URB_BASE + 0x4 + (i)*0x8)

#define SERVICE_TOP_CLK_GATE			(SERVICE_SUBS_URB_BASE + 0x1008)
#define SERVICE_TOP_CLK_GATE_SDR		BIT(3)
#define SERVICE_TOP_CLK_GATE_MEDIA		BIT(1)
#define SERVICE_TOP_CLK_GATE_LSPERIPH0		BIT(5)
#define SERVICE_TOP_CLK_GATE_LSPERIPH1		BIT(6)
#define SERVICE_TOP_CLK_GATE_DDR		BIT(7)

#endif /* _REGS_MCOM03_H */
