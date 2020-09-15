/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

OUTPUT_ARCH(mips)

ENTRY(__app_entry)

SECTIONS
{
	.text_init 0xffffffffbff14000 :
	AT(0x1ff14000) {
		*(.boot)
		*(.text)*(.text.*)
		. = ALIGN(16);
	} = 0

	.data : {
		*(.rodata)
		*(.rodata.*)
		*(.data)
		. = ALIGN(8);
		_gp = ABSOLUTE(. + 0x7ff0);
		*(.lit8)
		*(.lit4)
		*(.sdata)
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

	_end = . ;

	/DISCARD/ : {
		*(.reginfo)
	}

	_freemem = .;
}
