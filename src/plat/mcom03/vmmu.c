// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2021 RnD Center "ELVEES", JSC

#include <config.h>
#include <string.h>
#include <common.h>
#include <plat/mcom03/regs.h>
#include <plat/mcom03/vmmu.h>

#define VMMU_TABLE_COUNT       (3)
#define VMMU_TABLE_ENTRY_COUNT (512)
#define VMMU_TABLE_ENTRY_SIZE  (sizeof(uint64_t))

#define VMMU_TABLE_SIZE (VMMU_TABLE_ENTRY_COUNT * VMMU_TABLE_ENTRY_SIZE)
#define VMMU_TOTAL_SIZE (VMMU_TABLE_SIZE * VMMU_TABLE_COUNT)

#define VMMU_1GB (0x40000000ULL)
#define VMMU_2MB (0x00200000ULL)

#define VMMU_2MB_MASK (0x1FFFFFULL)

#define VMMU_Ith_GB(x) ((x)*VMMU_1GB)

#define VMMU_1GB_SLOT_IDX  (CONFIG_VMMU_VIRT_BASE_START / VMMU_1GB)
#define VMMU_1GB_SLOT_ADDR (VMMU_Ith_GB(VMMU_1GB_SLOT_IDX))

#define VMMU_2MB_SLOT_MIN ((CONFIG_VMMU_VIRT_BASE_START - VMMU_1GB_SLOT_ADDR) / VMMU_2MB)
#define VMMU_2MB_SLOT_MAX (VMMU_2MB_SLOT_MIN + CONFIG_VMMU_VIRT_SLOT_COUNT)

/**
 * @brief Get pointer of struct of registers VMMU
 *
 * @return pointer of struct of registers VMMU
 */
vmmu_t *vmmu_get_registers(void)
{
	return (vmmu_t *)SERVICE_SUBS_VMMU_BASE;
}

/**
 * @brief Physical to Virtual Address Conversion function
 *        for MIPS with Fixed Map Memory setting
 *
 * @param addr               physical address
 *
 * @return                   virtual address
 */
static uintptr_t convert_pa_to_va(uintptr_t addr)
{
	if (addr < 0x40000000UL)
		return (addr + 0xA0000000UL);
	if (addr < 0xC0000000UL)
		return (addr - 0x40000000UL);
	return (addr);
}

/**
 * @brief Initialize VMMU
 *
 * @param dev                pointer to struct of VMMU registers
 * @param ptw_base_addr      physical address of translation table
 */
void vmmu_init(vmmu_t *dev, uintptr_t ptw_base_addr)
{
	vmmu_pte_t *pte_p;
	unsigned int reg_val = 0UL;
	unsigned int i;

	//TODO: check align

	uintptr_t pAddrTab512Gb = ptw_base_addr;
	uintptr_t pAddrTab1Gb = ptw_base_addr + VMMU_TABLE_SIZE;
	uintptr_t pAddrTab2Mb = ptw_base_addr + 2UL * VMMU_TABLE_SIZE;

	uintptr_t vAddrTab512Gb = convert_pa_to_va(pAddrTab512Gb);
	uintptr_t vAddrTab1Gb = convert_pa_to_va(pAddrTab1Gb);
	uintptr_t vAddrTab2Mb = convert_pa_to_va(pAddrTab2Mb);

	/* Clean whole table */
	memset((void *)vAddrTab512Gb, 0, VMMU_TOTAL_SIZE);

	/* Fill 512GB table */
	pte_p = (vmmu_pte_t *)vAddrTab512Gb;
	pte_p[0] = (1ULL << VMMU_PTE_V_POS) | (VMMU_PTE_TYPE_REF << VMMU_PTE_TYPE_POS) |
		   ((pAddrTab1Gb >> 12ULL) << VMMU_PTE_PPN0_POS);

	/* Fill 1GB table */
	pte_p = (vmmu_pte_t *)vAddrTab1Gb;
	for (i = 0; i < 4; ++i) {
		if (i != VMMU_1GB_SLOT_IDX) {
			pte_p[i] = (1ULL << VMMU_PTE_V_POS) |
				   (VMMU_PTE_TYPE_RWX << VMMU_PTE_TYPE_POS) |
				   ((VMMU_Ith_GB(i) >> 12ULL) << VMMU_PTE_PPN0_POS);
		} else {
			pte_p[i] = (1ULL << VMMU_PTE_V_POS) |
				   (VMMU_PTE_TYPE_REF << VMMU_PTE_TYPE_POS) |
				   ((pAddrTab2Mb >> 12ULL) << VMMU_PTE_PPN0_POS);
		}
	}

	/* Fill 2MB table */
	pte_p = (vmmu_pte_t *)vAddrTab2Mb;
	for (i = 0; i < VMMU_TABLE_ENTRY_COUNT; ++i) {
		pte_p[i] = (1ULL << VMMU_PTE_V_POS) | (VMMU_PTE_TYPE_RWX << VMMU_PTE_TYPE_POS) |
			   (((VMMU_1GB_SLOT_ADDR + i * VMMU_2MB) >> 12ULL) << VMMU_PTE_PPN0_POS);
	}

	/* Set address for xlat table */
	dev->PTW_PBA_L = (unsigned int)pAddrTab512Gb;
	dev->PTW_PBA_H = 0x00UL; /* Always Null */

	/* Enable translation */
	reg_val = dev->PTW_CFG;
	reg_val &= ~(VMMU_PTW_CFG_A_PROT_MASK << VMMU_PTW_CFG_A_PROT_OFFSET);
	reg_val |= (VMMU_ENABLE_XLAT << VMMU_PTW_CFG_STATUS_VM_OFFSET);
	dev->PTW_CFG = reg_val;

	/* Disable skipping translation for all channel TLB */
	for (i = 0; i < VMMU_TLB_MAX_NUM; i++)
		dev->TLB_CTRL[i] = VMMU_TLB_CTRL_ALL_BIT_RESET;

	/* Check disabling range without conversion */
	if (dev->MAPSEG_ENABLE != 0UL)
		dev->MAPSEG_ENABLE = 0UL;

	/* Enable ARPROT/AWPROT bus AXI */
	reg_val = dev->PTW_CFG;
	reg_val |= ((VMMU_PTW_CFG_A_PROT_DEFAULT) << VMMU_PTW_CFG_A_PROT_OFFSET);
	dev->PTW_CFG = reg_val;
}

/**
 * @brief Deinitialize VMMU (Set the default value in the registers)
 *
 * @param dev                pointer to struct of VMMU registers
 */
void vmmu_deinit(vmmu_t *dev)
{
	unsigned int reg_val = 0UL;
	unsigned int i;

	/* Disable translation */
	reg_val = dev->PTW_CFG;
	reg_val &= ~(VMMU_PTW_CFG_A_PROT_MASK << VMMU_PTW_CFG_A_PROT_OFFSET);
	reg_val &= ~(0xAUL << VMMU_PTW_CFG_STATUS_VM_OFFSET);
	dev->PTW_CFG = reg_val;

	/* Disable skipping translation for all channel TLB */
	for (i = 0UL; i < VMMU_TLB_MAX_NUM; i++) {
		dev->TLB_CTRL[i] = VMMU_PASSTHROUGH_TLB_WRITE_CHANNEL |
				   VMMU_PASSTHROUGH_TLB_READ_CHANNEL |
				   VMMU_PASSTHROUGH_TLB_EXEC_CHANNEL;
	}

	/* Reset address for xlat table */
	dev->PTW_PBA_L = 0x00UL;
	dev->PTW_PBA_H = 0x00UL; /* Always Null */

	/* Check disabling range without conversion */
	if (dev->MAPSEG_ENABLE != 0UL) {
		dev->MAPSEG_ENABLE = 0UL;
	}

	/* Enable ARPROT/AWPROT bus AXI */
	reg_val = dev->PTW_CFG;
	reg_val |= ((VMMU_PTW_CFG_A_PROT_DEFAULT) << VMMU_PTW_CFG_A_PROT_OFFSET);

	/* Invalidate cache */
	reg_val |= (0x1UL << VMMU_PTW_CFG_INVALIDATE_OFFSET);
	dev->PTW_CFG = reg_val;
}

/**
 * @brief Deinitialize VMMU (Set the default value in the registers)
 *
 * @param dev                pointer to struct of VMMU registers
 * @param desired_addr32     desired 32bit address
 * @param base_address64     base 64bit address
 *
 * @return                   (0) - success; (-1) - failed
 */
int vmmu_map_64bit_address(vmmu_t *dev, uintptr_t desired_addr32, uint64_t base_address64)
{
	vmmu_pte_t *pte_p;
	int16_t slot;

	uintptr_t pAddrTab2Mb = dev->PTW_PBA_L + 2UL * VMMU_TABLE_SIZE;
	uintptr_t vAddrTab2Mb = convert_pa_to_va(pAddrTab2Mb);
	pte_p = (vmmu_pte_t *)vAddrTab2Mb;

	/* Check desired 32bit address */
	if ((desired_addr32 == 0ULL) || ((desired_addr32 & VMMU_2MB_MASK) != 0ULL))
		return -EVMMUCFG;

	/* Get slot in 2Mb table */
	slot = (desired_addr32 - CONFIG_VMMU_VIRT_BASE_START) / VMMU_2MB;
	if ((slot < VMMU_2MB_SLOT_MIN) || (slot >= VMMU_2MB_SLOT_MAX))
		return -EVMMUCFG;

	/* Check that slot is empty */
	if (((pte_p[slot] & VMMU_PTE_USER_DEF_MASK) >> VMMU_PTE_USER_DEF_POS) != 0ULL)
		return -EVMMUCFG;

	/* Setup translation window for base_address64 */
	pte_p[slot] = 0ULL;
	pte_p[slot] = (1ULL << VMMU_PTE_V_POS) | (VMMU_PTE_TYPE_RWX << VMMU_PTE_TYPE_POS) |
		      ((base_address64 >> 12ULL) << VMMU_PTE_PPN0_POS);
	pte_p[slot] |= (1ULL << VMMU_PTE_USER_DEF_POS);

	/* Invalidate cache */
	unsigned int reg_val = dev->PTW_CFG;
	reg_val |= (0x1UL << VMMU_PTW_CFG_INVALIDATE_OFFSET);
	dev->PTW_CFG = reg_val;

	return 0;
}
