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
#if defined(CONFIG_BOOTSTAGE_ENABLE)
	bootstage_f (rw)  :  ORIGIN = CONFIG_BOOTSTAGE_BASE_F, LENGTH = CONFIG_BOOTSTAGE_SIZE_F
	bootstage_r (rw)  :  ORIGIN = CONFIG_BOOTSTAGE_BASE_R, LENGTH = CONFIG_BOOTSTAGE_SIZE_R
#endif
}

SECTIONS
{
	. = CONFIG_RAM_BASE;

	.text : {
		*(.text.boot)
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

#if defined(CONFIG_BOOTSTAGE_ENABLE)
	. = ORIGIN(bootstage_f);
	ASSERT(. == ALIGN(4), "ORIGIN(bootstage_f) address is not aligned on a word boundary.")

	.bootstage_f (NOLOAD) : {
		__bs_start_f = ABSOLUTE(.);
		. = . + CONFIG_BOOTSTAGE_SIZE_F;
		. = ALIGN(4);
		__bs_end_f = ABSOLUTE(.);
	} > bootstage_f

	. = ORIGIN(bootstage_r);
	ASSERT(. == ALIGN(4), "ORIGIN(bootstage_r) address is not aligned on a word boundary.")

	.bootstage_r (NOLOAD) : {
		__bs_start_r = ABSOLUTE(.);
		. = . + CONFIG_BOOTSTAGE_SIZE_R;
		. = ALIGN(4);
		__bs_end_r = ABSOLUTE(.);
	} > bootstage_r
#endif

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
