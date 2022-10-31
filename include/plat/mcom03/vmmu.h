/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2022 RnD Center "ELVEES", JSC */

#ifndef _VMMU_H_
#define _VMMU_H_

#include <stdint.h>
#include <bitops.h>
#include <common.h>

#define VMMU_ENABLE_XLAT (0xAU)

#define VMMU_TLB_WR_RISC (0)
#define VMMU_TLB_RD_RISC (1)
#define VMMU_TLB_WR_QDMA (2)
#define VMMU_TLB_RD_QDMA (3)
#define VMMU_TLB_MAX_NUM (4U)

#define VMMU_IRQ_TLB0_MASK (0x01)
#define VMMU_IRQ_TLB1_MASK (0x02)
#define VMMU_IRQ_TLB2_MASK (0x04)
#define VMMU_IRQ_TLB3_MASK (0x08)

#define VMMU_TLB_ERROR_SUCCESS	 (0x00)
#define VMMU_TLB_ERROR_INVALID	 (0x01)
#define VMMU_TLB_ERROR_WR_PROT	 (0x02)
#define VMMU_TLB_ERROR_RD_PROT	 (0x03)
#define VMMU_TLB_ERROR_EXEC_PROT (0x04)

#define VMMU_TLB_CTRL_MASK	    (0xFF)
#define VMMU_TLB_CTRL_ALL_BIT_RESET (0)

#define VMMU_PASSTHROUGH_TLB_WRITE_CHANNEL    BIT(0)
#define VMMU_PASSTHROUGH_TLB_READ_CHANNEL     BIT(1)
#define VMMU_PASSTHROUGH_TLB_EXEC_CHANNEL     BIT(2)
#define VMMU_XPCT_RETRY_TLB_WRITE_CHANNEL     BIT(3)
#define VMMU_XPCT_RETRY_TLB_READ_CHANNEL      BIT(4)
#define VMMU_XPCT_DUMMY_TLB_WRITE_CHANNEL     BIT(5)
#define VMMU_XPCT_DUMMY_TLB_READ_CHANNEL      BIT(6)
#define VMMU_XPCT_AUTODUMMY_TLB_WRITE_CHANNEL BIT(7)
#define VMMU_XPCT_AUTODUMMY_TLB_READ_CHANNEL  BIT(8)

#define VMMU_PTW_CFG_INVALIDATE_OFFSET	(0U)
#define VMMU_PTW_CFG_STATUS_VM_OFFSET	(1U)
#define VMMU_PTW_CFG_STATUS_MPRV_OFFSET (6U)
#define VMMU_PTW_CFG_STATUS_PRV1_OFFSET (7U)
#define VMMU_PTW_CFG_STATUS_PRV_OFFSET	(9U)
#define VMMU_PTW_CFG_STATUS_IRQ_OFFSET	(11U)
#define VMMU_PTW_CFG_STATUS_IRQ_MASK	(0xFU)
#define VMMU_PTW_CFG_A_CACHE_OFFSET	(15U)
#define VMMU_PTW_CFG_A_PROT_OFFSET	(19U)
#define VMMU_PTW_CFG_A_PROT_DEFAULT	(0x2U)
#define VMMU_PTW_CFG_A_PROT_MASK	(0x7U)
#define VMMU_PTW_CFG_PREFETCH_OFFSET	(22U)
#define VMMU_PTW_CFG_FETCHTWO_OFFSET	(23U)

#define VMMU_PTE_V_POS	       (0ULL)
#define VMMU_PTE_V_MASK	       (0b1ULL << VMMU_PTE_V_POS)
#define VMMU_PTE_TYPE_POS      (1ULL)
#define VMMU_PTE_TYPE_MASK     (0b111ULL << VMMU_PTE_TYPE_POS)
#define VMMU_PTE_R_POS	       (5ULL)
#define VMMU_PTE_R_MASK	       (0b1ULL << VMMU_PTE_R_POS)
#define VMMU_PTE_D_POS	       (6ULL)
#define VMMU_PTE_D_MASK	       (0b1ULL << VMMU_PTE_D_POS)
#define VMMU_PTE_USER_DEF_POS  (7ULL)
#define VMMU_PTE_USER_DEF_MASK (0b111ULL << VMMU_PTE_USER_DEF_POS)
#define VMMU_PTE_PPN0_POS      (10ULL)
#define VMMU_PTE_PPN0_MASK     (0b111111111ULL << VMMU_PTE_PPN0_POS)
#define VMMU_PTE_PPN1_POS      (19ULL)
#define VMMU_PTE_PPN1_MASK     (0b111111111ULL << VMMU_PTE_PPN1_POS)
#define VMMU_PTE_PPN2_POS      (28ULL)
#define VMMU_PTE_PPN2_MASK     (0b111111111ULL << VMMU_PTE_PPN2_POS)
#define VMMU_PTE_PPN3_POS      (37ULL)
#define VMMU_PTE_PPN3_MASK     (0b111111111111ULL << VMMU_PTE_PPN3_POS)
#define VMMU_PTE_RSVD_POS      (46ULL)
#define VMMU_PTE_RSVD_MASK     (0b111111111111111111ULL << VMMU_PTE_RSVD_POS)
#define VMMU_PTE_TYPE_REF      (0b1)
#define VMMU_PTE_TYPE_RWX      (0b111)

/**
 * @brief Struct of registers VMMU
 *
 */
typedef struct {
	volatile unsigned int PTW_PBA_L;
	volatile unsigned int PTW_PBA_H;
	volatile unsigned int PTW_CFG;
	volatile unsigned int TLBXCPT_NUM;
	volatile unsigned int TLBXCPT_ADDR;
	volatile unsigned int TLBXCPT_TYPE;
	volatile unsigned int MAPSEG_START_L;
	volatile unsigned int MAPSEG_START_H;
	volatile unsigned int MAPSEG_END_L;
	volatile unsigned int MAPSEG_END_H;
	volatile unsigned int MAPSEG_ENABLE;
	volatile unsigned int RESERVED[5];
	volatile unsigned int TLB_CTRL[4];
} __attribute__((packed, __aligned__(4))) vmmu_t;

typedef uint64_t vmmu_pte_t;

vmmu_t *vmmu_get_registers(void);
void vmmu_init(vmmu_t *dev, uintptr_t ptw_base_addr);
void vmmu_deinit(vmmu_t *dev);

/* Function shall be used with critical section */
int vmmu_map_64bit_address(vmmu_t *dev, uintptr_t desired_addr32, uint64_t base_address64);
int vmmu_map_range_64bit_address(vmmu_t *dev, uintptr_t base32_start, ptrdiff_t base32_size,
				 uint64_t base64_start);

#endif /* _VMMU_H_ */
