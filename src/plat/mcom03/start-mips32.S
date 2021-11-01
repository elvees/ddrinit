/*
 * Copyright 2021-2022 RnD Center "ELVEES", JSC
 */

#include "machine/asm.h"
#include "machine/regdef.h"
#include "m32c0.h"
#include <config.h>

.set    noreorder
.section .text.boot

_start:
/* In noBoot mode the stack pointer is set up to CRAM by BootROM.
 * In QSPI0 XIP boot mode ddrinit retains the stack pointer set up by SBL.
 */
store_regs:
	addiu   sp, sp, -88
	sw      ra, 0(sp)
	sw      gp, 8(sp)
	sw      s0, 16(sp)
	sw      s1, 24(sp)
	sw      s2, 32(sp)
	sw      s3, 40(sp)
	sw      s4, 48(sp)
	sw      s5, 56(sp)
	sw      s6, 64(sp)
	sw      s7, 72(sp)
	sw      s8, 80(sp)
	la      gp, _gp

clean_bss:
	la      a0, __bss_start
	la      a1, __bss_length
	li      v0, 0
bss_clean_loop:
	blez    a1, bss_clean_done
	nop
	sw      v0, 0(a0)
	addiu   a0, a0, 4
	addiu   a1, a1, -4
	b       bss_clean_loop
	nop
bss_clean_done:
	/* Map 16 MiB at CONFIG_MEM_REGIONS_PHYS_ADDR to
	 * CONFIG_MEM_REGIONS_VIRT_ADDR to save
	 * initialized memory info.
	 */
	li      a0, 1
	mtc0    a0, C0_INDEX

	li      a1, CONFIG_MEM_REGIONS_VIRT_ADDR
	mtc0    a1, C0_ENTRYHI

	li      a1, (CONFIG_MEM_REGIONS_PHYS_ADDR>>6|0x17)
	mtc0    a1, C0_ENTRYLO0
	mtc0    a1, C0_ENTRYLO1
	ehb
	tlbwi
	nop

	/* Switch to TLB mapping */
	li      a0, 0xbfd08000
	li      v0, 0
	sw      v0, 0(a0)

	la      a2, main
	jalr    a2
	nop

	/* Switch to fixed mapping */
	li      a0, 0xbfd08000
	li      v0, 0x1
	sw      v0, 0(a0)

	/* Jump to previously stored return address */
	lw      ra, 0(sp)
	lw      gp, 8(sp)
	lw      s0, 16(sp)
	lw      s1, 24(sp)
	lw      s2, 32(sp)
	lw      s3, 40(sp)
	lw      s4, 48(sp)
	lw      s5, 56(sp)
	lw      s6, 64(sp)
	lw      s7, 72(sp)
	lw      s8, 80(sp)
	addiu   sp, sp, 88
	/* Allocate address for a JTAG breakpoint */
	.org 0x100
	nop
	jr      ra
