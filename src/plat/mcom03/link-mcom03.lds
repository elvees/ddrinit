/*
 * Copyright 2021 RnD Center "ELVEES", JSC
 */

ENTRY(_start)

SPRAM_BASE_ADDR = 0x0;

SECTIONS
{
	. = SPRAM_BASE_ADDR;

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
