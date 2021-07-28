/*
 * Copyright 2021 RnD Center "ELVEES", JSC
 */

#include <config.h>

.section ".text.boot"

.global _start

_start:
#ifdef CONFIG_BOOTMODE_JTAG
	ldr	x1, =0xfff0
	mov	sp, x1
#else
	sub	sp, sp, 16
	str	x30, [sp]
#endif

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

#ifdef CONFIG_BOOTMODE_JTAG
_jtag_mode_loop:
	ldr	x1, =CONFIG_BOOTMODE_JTAG_MAGIC_ADDRESS
	ldr	w1, [x1]
	ldr	w2, =0xdeadc0de
	cmp	w1, w2
	bne	_jtag_mode_loop
	ldr	x2, =CONFIG_BOOTMODE_JTAG_JUMP_ADDRESS
	br	x2
#endif

	ldr	x30, [sp]
	add	sp, sp, 16
	ret
