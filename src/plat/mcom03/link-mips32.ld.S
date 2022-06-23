/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2022 RnD Center "ELVEES", JSC */

#include <config.h>

OUTPUT_ARCH("mips")
TARGET("elf32-littlemips")
GROUP(-lc -lgcc)

ENTRY(_start)

MEMORY
{
	ram (rwx)  : ORIGIN = CONFIG_RAM_BASE, LENGTH = CONFIG_RAM_SIZE
}

SECTIONS
{
	. = CONFIG_RAM_BASE;

	.text : {
		*(.text.boot*)
		*(.text*)
		__text_end = .;
	} > ram

	.data : {
		. = ALIGN(8);
		__data_start = .;
		*(.rodata)
		*(.rodata.*)
		*(.data)
		*(.data.*)
		. = ALIGN(4);
		_gp = ALIGN(16);  /* Base of small data (for MIPS) */
		*(.lit8)
		*(.lit4)
		*(.sdata)
		*(.sdata.*)
		__data_end = .;
	} > ram

	.sbss : {
		. = ALIGN(8);
		__bss_start = .;
		*(.sbss)
		*(.sbss.*)
		*(.scommon)
	} > ram

	.bss : {
		*(.bss)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(8);
		__bss_end = .;
	} > ram
	__bss_length = __bss_end - __bss_start;

#ifdef CONFIG_BOOTLOADER_BOOTROM
	_stack = ORIGIN(ram) + LENGTH(ram) - 0x20;
#endif

	end = .;

	ASSERT(end < (CONFIG_RAM_BASE + CONFIG_RAM_SIZE), "ddrinit image has exceeded RAM limit.")

	/DISCARD/ : {
		*(.reginfo)
		*(.MIPS.abiflags)
		*(.comment)
		*(.note*)
		*(.eh_frame*)
		*(.debug*)
		*(.dynsym)
		*(.dynstr*)
		*(.dynamic*)
		*(.plt*)
		*(.interp*)
		*(.gnu*)
	}
}
