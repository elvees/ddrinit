/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#ifndef _REGS_SOLARIS_H
#define _REGS_SOLARIS_H

#include <regs.h>

#define UCG_BASE(i) (0x1DC00000 + (i)*0x400000)

#define UCG_UFG_REG0(i, j) (UCG_BASE(i) + 0x20 + (j)*0x40)
#define UCG_UFG_REG0_PLL_PD BIT(2)
#define UCG_UFG_REG0_DOC_PD BIT(3)
#define UCG_UFG_REG0_DSM_PD BIT(4)
#define UCG_UFG_REG0_VCO_PD BIT(5)
#define UCG_UFG_REG0_OP_PD BIT(6)
#define UCG_UFG_REG0_PH_PD BIT(7)
#define UCG_UFG_REG0_LOCKSTAT BIT(30)

#define UCG_UFG_REG1(i, j) (UCG_BASE(i) + 0x24 + (j)*0x40)

#define UCG_UFG_REG2(i, j) (UCG_BASE(i) + 0x28 + (j)*0x40)

#define UCG_UFG_REG3(i, j) (UCG_BASE(i) + 0x2C + (j)*0x40)
#define UCG_UFG_REG3_FBDIV GENMASK(11, 0)

#define UCG_UFG_REG4(i, j) (UCG_BASE(i) + 0x30 + (j)*0x40)

#define UCG_UFG_REG5(i, j) (UCG_BASE(i) + 0x34 + (j)*0x40)
#define UCG_UFG_REG5_BYPASS BIT(1)

#define UCG_UFG_REG6(i, j) (UCG_BASE(i) + 0x38 + (j)*0x40)
#define UCG_UFG_REG6_UPDATE BIT(0)

#define UCG_UFG_REG10(i, j) (UCG_BASE(i) + 0x48 + (j)*0x50)
#define UCG_UFG_REG10_POSTDIV1 GENMASK(2, 0)
#define UCG_UFG_REG10_POSTDIV2 GENMASK(5, 3)

#define UCG_XBAR(i, j) (UCG_BASE(i) + 0x420 + (j)*4)
#define UCG_XBAR_SEL GENMASK(4, 0)
#define UCG_XBAR_ENABLE BIT(16)

#define UCG_FIRSTDIV(i, j) (UCG_BASE(i) + 0x820 + (j)*4)
#define UCG_FIRSTDIV_NDIV GENMASK(8, 1)

#define PMU_BASE 0x1F800000
#define PMU_PDCI_RESETN_REQ(i) (PMU_BASE + 0x50 + 0x20 * (i))
#define PMU_PDCI_RESETN_ACK(i) (PMU_BASE + 0x54 + 0x20 * (i))

#define DDRSUBS_REGBANK_BASE 0x10000
#define DDRSUBS_REGBANK_SYSTEM_CTRL(i) (DDRSUB_BASE(i) + DDRSUBS_REGBANK_BASE + 0x10)

#define MFIO_NPAD_CTRL (PMU_BASE + 0x230000)
#define MFIO_NX_FUNCTION_CTRL(i) (MFIO_NPAD_CTRL + 0x50C + (i)*4)
#define MFIO_CR_GPION_BIT_EN(i) (MFIO_NPAD_CTRL + 0x84 + (i)*0x24)

#endif /* _REGS_SOLARIS_H */
