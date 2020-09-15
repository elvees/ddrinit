// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#include <common.h>

void write32(unsigned long addr, uint32_t val)
{
	*((volatile uint32_t *)addr) = val;
}

void write16(unsigned long addr, uint16_t val)
{
	*((volatile uint16_t *)addr) = val;
}

uint32_t read32(unsigned long addr)
{
	return *((volatile uint32_t *)addr);
}

void delay_us(int val)
{
	/* TODO: Use timers */
	volatile int i;

	for (i = 0; i < val * 1000; i++)
		__asm__ volatile("nop");
}

void *memcpy(void *dest, const void *src, size_t count)
{
	unsigned long *dl = (unsigned long *)dest, *sl = (unsigned long *)src;
	uint8_t *d8, *s8;

	if (src == dest)
		return dest;

	/* While all data is aligned (common case), copy a word at a time */
	if ((((unsigned long)dest | (unsigned long)src) & (sizeof(*dl) - 1)) == 0) {
		while (count >= sizeof(*dl)) {
			*dl++ = *sl++;
			count -= sizeof(*dl);
		}
	}

	/* Copy the rest one byte at a time */
	d8 = (uint8_t *)dl;
	s8 = (uint8_t *)sl;

	while (count--)
		*d8++ = *s8++;

	return dest;
}

void *memset(void *s, int c, size_t count)
{
	unsigned long *sl = (unsigned long *)s;
	char *s8;
	unsigned long cl = 0;
	int i;

	/* Do it one word at a time (32 bits or 64 bits) while possible */
	if (((unsigned long)s & (sizeof(*sl) - 1)) == 0) {
		for (i = 0; i < sizeof(*sl); i++) {
			cl <<= 8;
			cl |= c & 0xff;
		}
		while (count >= sizeof(*sl)) {
			*sl++ = cl;
			count -= sizeof(*sl);
		}
	}

	/* Fill 8 bits at a time */
	s8 = (char *)sl;

	while (count--)
		*s8++ = c;

	return s;
}
