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

#define DDRCFG_OVERRIDE_MAGIC 0xdeadc0de

#define USEC 1ULL
#define MSEC 1000ULL
#define SEC 1000000ULL

#define read_poll_timeout(op, val, cond, sleep_us, timeout_us, args...)	\
({ \
	int timeout = timer_get_usec() + timeout_us; \
	for (;;) { \
		(val) = op(args); \
		if (cond) \
			break; \
		if (timer_get_usec() > timeout) { \
			(val) = op(args); \
			break; \
		} \
		if (sleep_us) \
			delay_usec(sleep_us); \
	} \
	(cond) ? 0 : -ETIMEDOUT; \
})

#define read32_poll_timeout(val, cond, sleep_us, timeout_us, addr) \
	read_poll_timeout(read32, val, cond, sleep_us, timeout_us, addr)

#define phy_read32_poll_timeout(val, cond, sleep_us, timeout_us, args...) \
	read_poll_timeout(phy_read32, val, cond, sleep_us, timeout_us, args)

extern unsigned long ddrcfg_override_start __attribute__((section(".text")));

struct ddrcfg_override {
	uint32_t magic;
	uint32_t xtal_freq;
};

enum ddrinit_error_code {
	EDDRMC0INITFAIL = 1,
	EPOWERUP,
	ECLOCKCFG,
	EI2CREAD,
	ESPDCHECKSUM,
	EDIMMCFG,
	EFWTYPE,
	ETRAINFAIL,
	EI2CCFG,
	ETIMEDOUT,
	ETRAINTIMEOUT,
	EUARTCFG,
};

void delay_usec(int usec);
void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *s, int c, size_t count);
uint32_t read32(unsigned long addr);
uint64_t read64(unsigned long addr);
char *strcpy(char *dest, const char *src);
int timer_get_usec(void);
void write16(unsigned long addr, uint16_t val);
void write32(unsigned long addr, uint32_t val);
void write64(unsigned long addr, uint64_t val);

#endif
