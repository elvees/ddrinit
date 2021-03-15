// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#include <common.h>
#include <plat/plat.h>

void write16(unsigned long addr, uint16_t val)
{
	*((volatile uint16_t *)addr) = val;
}

void write32(unsigned long addr, uint32_t val)
{
	*((volatile uint32_t *)addr) = val;
}

void write64(unsigned long addr, uint64_t val)
{
	*((volatile uint64_t *)addr) = val;
}

uint32_t read32(unsigned long addr)
{
	return *((volatile uint32_t *)addr);
}

uint64_t read64(unsigned long addr)
{
	return *((volatile uint64_t *)addr);
}

static uint64_t usec_to_tick(int usec)
{
	return (uint64_t)usec * CONFIG_TIMER_FREQ / SEC;
}

static uint64_t get_ticks(void)
{
	static uint32_t timebase_h, timebase_l;
	uint32_t now = platform_get_timer_count();

	/* Increment tbh if tbl has rolled over */
	if (now < timebase_l)
		timebase_h++;
	timebase_l = now;
	return ((uint64_t)timebase_h << 32) | timebase_l;
}

void delay_usec(int usec)
{
	uint64_t tmp;

	/* Get current timestamp */
	tmp = get_ticks() + usec_to_tick(usec);

	/* Loop till event */
	while (get_ticks() < tmp + 1)
		 /*NOP*/;
}

int timer_get_usec(void)
{
	return get_ticks() * SEC / CONFIG_TIMER_FREQ;
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

char *strcpy(char *dest, const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}
