// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#ifndef _COMMON_H
#define _COMMON_H

#include <stddef.h>
#include <stdint.h>

#include <config.h>
#include <printf.h>

#define DRAM_TCK_1250 1600
#define DRAM_TCK_1333 1500
#define DRAM_TCK_1600 1250
#define DRAM_TCK_1866 1071
#define DRAM_TCK_2133 937
#define DRAM_TCK_2400 833
#define DRAM_TCK_2666 750
#define DRAM_TCK_2933 682
#define DRAM_TCK_3200 625

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))
#define ps2clk_rd(t, tck) ((t) / (tck))
#define ps2clk_ru(t, tck) DIV_ROUND_UP((t), (tck))

enum ddrinit_error_code {
	EPOWERUP = 1,
	ECLOCKCFG,
	EI2CREAD,
	ESPDCHECKSUM,
	EDIMMCFG,
	EFWTYPE,
	ETRAINFAIL,
};

void delay_us(int val);
void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *s, int c, size_t count);
uint32_t read32(unsigned long addr);
char *strcpy(char *dest, const char *src);
void write32(unsigned long addr, uint32_t val);
void write16(unsigned long addr, uint16_t val);

#endif
