/*
 * Copyright 2022 RnD Center "ELVEES", JSC
 */

#include <config.h>

OUTPUT_ARCH("mips:isa32")
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
		__text_end = ABSOLUTE(.);
	}

	. = ALIGN(8);

	.data : {
		__data_start = ABSOLUTE(.);
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
		__data_end = ABSOLUTE(.);
	}

	. = ALIGN(8);

	.sbss : {
		__bss_start = ABSOLUTE(.);
		*(.sbss)
		*(.sbss.*)
		*(.scommon)
	}

	.bss : {
		*(.bss)
		*(.bss.*)
		*(COMMON)
		__bss_end = ABSOLUTE(.);
		__bss_length = ABSOLUTE(.) - __bss_start;
	}

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
