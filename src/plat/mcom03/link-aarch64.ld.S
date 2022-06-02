/*
 * Copyright 2021 RnD Center "ELVEES", JSC
 */

#include <config.h>

ENTRY(_start)

SECTIONS
{
	. = CONFIG_RAM_BASE;

	.text : {
		*(.text.boot*)
		*(.text*)
	}

	. = ALIGN(8);

	.data : {
		*(.rodata)
		*(.data)
	}

	. = ALIGN(8);

	.bss : {
		__bss_start__ = . ;
		*(.bss)
		__bss_end__ = . ;
	}

	/DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) *(.debug*) }
	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
}
