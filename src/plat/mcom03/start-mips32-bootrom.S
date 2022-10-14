/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2021-2022 RnD Center "ELVEES", JSC */

#include <plat/mcom03/asm.h>
#include <config.h>

.section .text.boot

.set     noat
.set     noreorder
.set     mips32

LEAF(_start)
	la      $sp, _stack
	la      $gp, _gp

	la      $k0, __bss_start
	la      $k1, __bss_length
	li      $v0, 0

1:	blez    $k1, 2f
	nop
	sw      $v0, 0($k0)
	addiu   $k0, $k0, 4
	addiu   $k1, $k1, -4
	b       1b
	nop

2:	SAVE_REGS
	la      $k0, main
	jalr    $k0
	nop
	RESTORE_REGS

	/* Allocate address for a JTAG breakpoint */
	.org    0x188
	nop

3:	lui     $k0, 0x9fa0
	lui     $k1, 0x0100
	sw      $k1, 312($k0)
	li      $k0, 0xbfc00500
	jalr    $k0
	nop
	b       3b
	nop

	b        .
	nop
END(_start)

.set at
.set reorder
