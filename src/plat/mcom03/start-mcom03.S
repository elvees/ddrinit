/*
 * Copyright 2021 RnD Center "ELVEES", JSC
 */

.section ".text.boot"

.global _start

_start:
	sub	sp, sp, 16
	str	x30, [sp]

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

	ldr	x30, [sp]
	add	sp, sp, 16
	ret
