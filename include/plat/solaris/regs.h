/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2020 RnD Center "ELVEES", JSC */

#ifndef _REGS_SOLARIS_H
#define _REGS_SOLARIS_H

#include <regs.h>

#define VIRT_ADDR_BASE 0x9000000000000000

#define UCG_BASE(i)	    (VIRT_ADDR_BASE + 0x1e000000 - (i) * 0x400000)
#define UCG_UFG_REG0(i, j)  (UCG_BASE(i) + 0x20 + (j) * 0x40)
#define UCG_UFG_REG1(i, j)  (UCG_BASE(i) + 0x24 + (j) * 0x40)
#define UCG_UFG_REG2(i, j)  (UCG_BASE(i) + 0x28 + (j) * 0x40)
#define UCG_UFG_REG3(i, j)  (UCG_BASE(i) + 0x2C + (j) * 0x40)
#define UCG_UFG_REG4(i, j)  (UCG_BASE(i) + 0x30 + (j) * 0x40)
#define UCG_UFG_REG5(i, j)  (UCG_BASE(i) + 0x34 + (j) * 0x40)
#define UCG_UFG_REG6(i, j)  (UCG_BASE(i) + 0x38 + (j) * 0x40)
#define UCG_UFG_REG10(i, j) (UCG_BASE(i) + 0x48 + (j) * 0x40)
#define UCG_XBAR(i, j)	    (UCG_BASE(i) + 0x420 + (j) * 4)
#define UCG_FIRSTDIV(i, j)  (UCG_BASE(i) + 0x820 + (j) * 4)

#define UCG_UFG_REG0_PLL_PD    BIT(2)
#define UCG_UFG_REG0_DOC_PD    BIT(3)
#define UCG_UFG_REG0_DSM_PD    BIT(4)
#define UCG_UFG_REG0_VCO_PD    BIT(5)
#define UCG_UFG_REG0_OP_PD     BIT(6)
#define UCG_UFG_REG0_PH_PD     BIT(7)
#define UCG_UFG_REG0_LOCKSTAT  BIT(30)
#define UCG_UFG_REG3_FBDIV     GENMASK(11, 0)
#define UCG_UFG_REG5_BYPASS    BIT(1)
#define UCG_UFG_REG6_UPDATE    BIT(0)
#define UCG_UFG_REG10_POSTDIV1 GENMASK(2, 0)
#define UCG_UFG_REG10_POSTDIV2 GENMASK(5, 3)
#define UCG_XBAR_SEL	       GENMASK(4, 0)
#define UCG_XBAR_ENABLE	       BIT(16)
#define UCG_FIRSTDIV_NDIV      GENMASK(8, 1)

#define PMU_BASE	       (VIRT_ADDR_BASE + 0x1f800000)
#define PMU_PDCI_RESETN_REQ(i) (PMU_BASE + 0x50 + 0x20 * (i))
#define PMU_PDCI_RESETN_ACK(i) (PMU_BASE + 0x54 + 0x20 * (i))

#define DDRSUBS_REGBANK_OFFSET			0x10000
#define DDRSUBS_REGBANK_SYSTEM_CTRL(i)		(DDRSUB_BASE(i) + DDRSUBS_REGBANK_OFFSET + 0x10)
#define DDRSUBS_REGBANK_PHY_PAGE_ADDR(i)	(DDRSUB_BASE(i) + DDRSUBS_REGBANK_OFFSET + 0x14)
#define DDRSUBS_REGBANK_SOC_INTERLEAVE(i)	(DDRSUB_BASE(i) + DDRSUBS_REGBANK_OFFSET + 0x18)
#define DDRSUBS_REGBANK_SOC_INTERLEAVE_BOUNDARY GENMASK(5, 4)
#define DDRSUBS_REGBANK_SOC_INTERLEAVE_ENABLE	BIT(0)

#define MFIO_NPAD_CTRL		 (PMU_BASE + 0x230000)
#define MFIO_NX_FUNCTION_CTRL(i) (MFIO_NPAD_CTRL + 0x50c + (i) * 4)
#define MFIO_CR_GPION_BIT_EN(i)	 (MFIO_NPAD_CTRL + 0x84 + (i) * 0x24)

#define MFIO_SPAD_CTRL		 (VIRT_ADDR_BASE + 0x1d030000)
#define MFIO_SX_FUNCTION_CTRL(i) (MFIO_SPAD_CTRL + 0x504 + (i) * 4)
#define MFIO_CR_GPIOS_BIT_EN(i)	 (MFIO_SPAD_CTRL + 0x84 + (i) * 0x24)

#define NOC_AGENT_LLC_X_0_LLC_GLOBAL_ALLOC(i)	  (VIRT_ADDR_BASE + 0x282b4040 + (i) * 0x4000)
#define NOC_AGENT_LLC_X_0_LLC_CACHE_WAY_ENABLE(i) (VIRT_ADDR_BASE + 0x282b4048 + (i) * 0x4000)
#define NOC_AGENT_LLC_X_0_LLC_ALLOC_ARCACHE_EN(i) (VIRT_ADDR_BASE + 0x282b4050 + (i) * 0x4000)
#define NOC_AGENT_LLC_X_0_LLC_ALLOC_RD_EN(i)	  (VIRT_ADDR_BASE + 0x282b4058 + (i) * 0x4000)
#define NOC_AGENT_LLC_X_0_LLC_ALLOC_AWCACHE_EN(i) (VIRT_ADDR_BASE + 0x282b4060 + (i) * 0x4000)
#define NOC_AGENT_LLC_X_0_LLC_ALLOC_WR_EN(i)	  (VIRT_ADDR_BASE + 0x282b4068 + (i) * 0x4000)
#define NOC_AGENT_LLC_X_0_LLC_RAM_WAY_ENABLE(i)	  (VIRT_ADDR_BASE + 0x282b4080 + (i) * 0x4000)
#define NOC_AGENT_LLC_X_0_LLC_RAM_WAY_SECURE(i)	  (VIRT_ADDR_BASE + 0x282b4088 + (i) * 0x4000)
#define NOC_AGENT_LLC_X_0_LLC_RAM_ADDRESS_BASE(i) (VIRT_ADDR_BASE + 0x282b4090 + (i) * 0x4000)
#define NOC_AGENT_LLC_X_0_LLC_TAG_INV_CTL(i)	  (VIRT_ADDR_BASE + 0x282b40c0 + (i) * 0x4000)
#define NOC_AGENT_LLC_X_0_LLC_WAY_FLUSH(i)	  (VIRT_ADDR_BASE + 0x282b40d0 + (i) * 0x4000)
#define LLC_RAM_BASE_ADDR(i)			  (0x40000000 + ((i) - 2) * 0x400000)

#define NOC_BRIDGE_VIDEO_IN_MEMIF_INIT_AROVRD  (VIRT_ADDR_BASE + 0x28237f60)
#define NOC_BRIDGE_VIDEO_IN_MEMIF_INIT_AWOVRD  (VIRT_ADDR_BASE + 0x28237f68)
#define NOC_BRIDGE_VIDEO_OUT_MEMIF_INIT_AROVRD (VIRT_ADDR_BASE + 0x2824bf60)
#define NOC_BRIDGE_VIDEO_OUT_MEMIF_INIT_AWOVRD (VIRT_ADDR_BASE + 0x2824bf68)
#define NOC_BRIDGE_VXD_MEMIF_INIT_AROVRD(i)    (VIRT_ADDR_BASE + 0x28257f60 + (i) * 0x14000)
#define NOC_BRIDGE_VXD_MEMIF_INIT_AWOVRD(i)    (VIRT_ADDR_BASE + 0x28257f68 + (i) * 0x14000)
#define NOC_BRIDGE_VXE_MEMIF_INIT_AROVRD(i)    (VIRT_ADDR_BASE + 0x28277f60 + (i) * 0x14000)
#define NOC_BRIDGE_VXE_MEMIF_INIT_AWOVRD(i)    (VIRT_ADDR_BASE + 0x28277f68 + (i) * 0x14000)
#define NOC_BRIDGE_GPU_MEMIF_INIT_AROVRD(i)    (VIRT_ADDR_BASE + 0x280d3f60 + (i) * 0x4000)
#define NOC_BRIDGE_GPU_MEMIF_INIT_AWOVRD(i)    (VIRT_ADDR_BASE + 0x280d3f68 + (i) * 0x4000)
#define NOC_BRIDGE_ELVEES_MEMIF_INIT_AROVRD    (VIRT_ADDR_BASE + 0x280c3f60)
#define NOC_BRIDGE_ELVEES_MEMIF_INIT_AWOVRD    (VIRT_ADDR_BASE + 0x280c3f68)
#define NOC_BRIDGE_NPU_MEMIF_INIT_AROVRD       (VIRT_ADDR_BASE + 0x280f7f60)
#define NOC_BRIDGE_NPU_MEMIF_INIT_AWOVRD       (VIRT_ADDR_BASE + 0x280f7f68)
#define NOC_BRIDGE_SATA_MEMIF_INIT_AROVRD      (VIRT_ADDR_BASE + 0x28173f60)
#define NOC_BRIDGE_SATA_MEMIF_INIT_AWOVRD      (VIRT_ADDR_BASE + 0x28173f68)
#define NOC_BRIDGE_USB_MEMIF_INIT_AROVRD(i)    (VIRT_ADDR_BASE + 0x281a7f60 + (i) * 0x14000)
#define NOC_BRIDGE_USB_MEMIF_INIT_AWOVRD(i)    (VIRT_ADDR_BASE + 0x281a7f68 + (i) * 0x14000)
#define NOC_BRIDGE_PCIE_MEMIF_INIT_AROVRD(i)   (VIRT_ADDR_BASE + 0x28123f60 + (i) * 0x4000)
#define NOC_BRIDGE_PCIE_MEMIF_INIT_AWOVRD(i)   (VIRT_ADDR_BASE + 0x28123f68 + (i) * 0x4000)
#define NOC_BRIDGE_PERIPH_MEMIF_INIT_AROVRD(i) (VIRT_ADDR_BASE + 0x28147f60 + (i) * 0xC000)
#define NOC_BRIDGE_PERIPH_MEMIF_INIT_AWOVRD(i) (VIRT_ADDR_BASE + 0x28147f68 + (i) * 0xC000)
#define NOC_BRIDGE_STARTUP_MEMIF_INIT_AROVRD   (VIRT_ADDR_BASE + 0x2817bf60)
#define NOC_BRIDGE_STARTUP_MEMIF_INIT_AWOVRD   (VIRT_ADDR_BASE + 0x2817bf68)

#define IOMMU_VIDEO_OUT_GCFG	     (VIRT_ADDR_BASE + 0x3DA00050)
#define IOMMU_VIDEO_IN_GCFG	     (VIRT_ADDR_BASE + 0x31600050)
#define IOMMU_PERIPHERAL_B_GCFG	     (VIRT_ADDR_BASE + 0x3CE00050)
#define IOMMU_PERIPHERAL_A_GCFG	     (VIRT_ADDR_BASE + 0x3D200050)
#define IOMMU_ELVEES_PERIPHERAL_GCFG (VIRT_ADDR_BASE + 0x3CA00050)
#define IOMMU_NPU_GCFG		     (VIRT_ADDR_BASE + 0x3C600050)
#define IOMMU_SATA_GCFG		     (VIRT_ADDR_BASE + 0x3C200050)
#define IOMMU_USB_GCFG(i)	     (VIRT_ADDR_BASE + 0x3B600000 + (i) * 0x400000)
#define IOMMU_PCIE_GCFG(i)	     (VIRT_ADDR_BASE + 0x3A200000 + (i) * 0x400000)
#define IOMMU_VELCOREQ_GCFG(i)	     (VIRT_ADDR_BASE + 0x37200000 + (i) * 0x400000)
#define IOMMU_VXD_GCFG(i)	     (VIRT_ADDR_BASE + 0x30E00000 + (i) * 0x400000)
#define IOMMU_VXE_GCFG(i)	     (VIRT_ADDR_BASE + 0x30200000 + (i) * 0x400000)

#define CPU_HASH_FUNC_HASH_0(i)		  (VIRT_ADDR_BASE + 0x2800f400 + (i) * 0x20000)
#define DEBUG_HASH_FUNC_HASH_1		  (VIRT_ADDR_BASE + 0x280a3400)
#define ELVEES_HASH_FUNC_HASH_1		  (VIRT_ADDR_BASE + 0x280c3400)
#define GPU_HASH_FUNC_HASH_1(i)		  (VIRT_ADDR_BASE + 0x280d3400 + (i) * 0x4000)
#define NPU_HASH_FUNC_HASH_1		  (VIRT_ADDR_BASE + 0x280f7400)
#define PCIE_MSTR_HASH_FUNC_HASH_1(i)	  (VIRT_ADDR_BASE + 0x28123400 + (i) * 0x4000)
#define PERIPH_A_HASH_FUNC_HASH_1	  (VIRT_ADDR_BASE + 0x28147400)
#define PERIPH_B_HASH_FUNC_HASH_1	  (VIRT_ADDR_BASE + 0x28153400)
#define SATA_HASH_FUNC_HASH_1		  (VIRT_ADDR_BASE + 0x28173400)
#define STARTUP_HASH_FUNC_HASH_1	  (VIRT_ADDR_BASE + 0x2817b400)
#define USB_HASH_FUNC_HASH_1(i)		  (VIRT_ADDR_BASE + 0x281A7400 + (i) * 0x14000)
#define VELCORE_Q0_Q1_HASH_FUNC_HASH_1(i) (VIRT_ADDR_BASE + 0x281cb400 + (i) * 0x20000)
#define VELCORE_Q2_Q3_HASH_FUNC_HASH_1(i) (VIRT_ADDR_BASE + 0x28203400 + (i) * 0x1c000)
#define VXE_HASH_FUNC_HASH_1(i)		  (VIRT_ADDR_BASE + 0x28277400 + (i) * 0x14000)
#define VXD_HASH_FUNC_HASH_1(i)		  (VIRT_ADDR_BASE + 0x28257400 + (i) * 0x14000)
#define VIDEO_IN_HASH_FUNC_HASH_1	  (VIRT_ADDR_BASE + 0x28237400)
#define VIDEO_OUT_HASH_FUNC_HASH_1	  (VIRT_ADDR_BASE + 0x2824b400)

#define AXI_COMM_US_DATA    (VIRT_ADDR_BASE + 0x1F40400C)
#define AXI_COMM_US_CTL	    (VIRT_ADDR_BASE + 0x1F404000)
#define AXI_COMM_DS_CTL	    (VIRT_ADDR_BASE + 0x1F404080)
#define AXI_COMM_US_BUF_STS (VIRT_ADDR_BASE + 0x1F404010)

#define IOMMU_GCFG_EN  BIT(0)
#define IOMMU_GCFG_OCR BIT(10)

#endif /* _REGS_SOLARIS_H */
