/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2020 RnD Center "ELVEES", JSC */

#include <config.h>

OUTPUT_ARCH("mips")

ENTRY(__app_entry)

SECTIONS
{
	.text_init CONFIG_RAM_BASE : {
		*(.boot)
		*(.text)*(.text.*)
		. = ALIGN(16);
	} = 0

	.data : {
		*(.rodata)
		*(.rodata.*)
		*(.data)
		*(.data.*)
		. = ALIGN(8);
		_gp = ABSOLUTE(. + 0x7ff0);
		*(.lit8)
		*(.lit4)
		*(.sdata)
		*(.sdata.*)
		. = ALIGN(8);
	}

	_fbss = .;
	.sbss : {
		*(.sbss)
		*(.scommon)
	}

	.bss : {
		*(.bss)
		*(COMMON)
	}
	. = ALIGN(8);
	_end = . ;

	/DISCARD/ : {
		*(.reginfo)
	}

	_freemem = .;
}
