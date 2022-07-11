/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2021 RnD Center "ELVEES", JSC */

#include <config.h>

.section ".text.boot"

.global _start

_start:
	ldr	x1, =0xfff0
	mov	sp, x1

_clean_bss:
	mov	w0, #0
	ldr	x1, =__bss_start__
	ldr	x2, =__bss_end__

_bss_clean_loop:
	str	w0, [x1]
	add	x1, x1, #4
	cmp	x1, x2
	ble	_bss_clean_loop

	bl	main

_ddrinit_loop:
	ldr	x1, =CONFIG_DDRINIT_JTAG_MAGIC_ADDRESS
	ldr	w1, [x1]
	ldr	w2, =0xdeadc0de
	cmp	w1, w2
	bne	_ddrinit_loop
	ldr	x2, =CONFIG_DDRINIT_RETURN_ADDRESS
	br	x2

	ldr	x30, [sp]
	add	sp, sp, 16
	ret
