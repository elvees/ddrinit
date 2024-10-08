/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2020 RnD Center "ELVEES", JSC */

#ifndef _COMMON_H
#define _COMMON_H

#include <stddef.h>
#include <stdint.h>

#include <config.h>
#include <printf.h>

#define DRAM_TCK_266  7504
#define DRAM_TCK_533  3748
#define DRAM_TCK_1066 1876
#define DRAM_TCK_1250 1600
#define DRAM_TCK_1333 1500
#define DRAM_TCK_1600 1250
#define DRAM_TCK_1866 1071
#define DRAM_TCK_2133 937
#define DRAM_TCK_2400 833
#define DRAM_TCK_2666 750
#define DRAM_TCK_2933 682
#define DRAM_TCK_3200 625

/*
 * Getting something that works in C and CPP for an arg that may or may
 * not be defined is tricky.  Here, if we have "#define CONFIG_BOOGER 1"
 * we match on the placeholder define, insert the "0," for arg1 and generate
 * the triplet (0, 1, 0).  Then the last step cherry picks the 2nd arg (a one).
 * When CONFIG_BOOGER is not defined, we generate a (... 1, 0) pair, and when
 * the last step cherry picks the 2nd arg, we get a zero.
 */
#define __ARG_PLACEHOLDER_1		       0,
#define config_enabled(cfg)		       _config_enabled(cfg)
#define _config_enabled(value)		       __config_enabled(__ARG_PLACEHOLDER_##value)
#define __config_enabled(arg1_or_junk)	       ___config_enabled(arg1_or_junk 1, 0)
#define ___config_enabled(__ignored, val, ...) val

/*
 * IS_ENABLED(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'y',
 * 0 otherwise.
 */
#define IS_ENABLED(option) (config_enabled(option))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define DIV_ROUND_CLOSEST(x, divisor)                                             \
	({                                                                        \
		typeof(x) __x = x;                                                \
		typeof(divisor) __d = divisor;                                    \
		(((typeof(x))-1) > 0 || ((typeof(divisor))-1) > 0 || (__x) > 0) ? \
			(((__x) + ((__d) / 2)) / (__d)) :                         \
			(((__x) - ((__d) / 2)) / (__d));                          \
	})

#define ps2clk_rd(t, tck) ((t) / (tck))
#define ps2clk_ru(t, tck) DIV_ROUND_UP((t), (tck))

#define DDRCFG_OVERRIDE_MAGIC 0xdeadc0de

#define USEC 1ULL
#define MSEC 1000ULL
#define SEC  1000000ULL

#define read_poll_timeout(op, val, cond, sleep_us, timeout_us, args...) \
	({                                                              \
		int timeout = timer_get_usec() + timeout_us;            \
		for (;;) {                                              \
			(val) = op(args);                               \
			if (cond)                                       \
				break;                                  \
			if (timer_get_usec() > timeout) {               \
				(val) = op(args);                       \
				break;                                  \
			}                                               \
			if (sleep_us)                                   \
				delay_usec(sleep_us);                   \
		}                                                       \
		(cond) ? 0 : -ETIMEDOUT;                                \
	})

#define read32_poll_timeout(val, cond, sleep_us, timeout_us, addr) \
	read_poll_timeout(read32, val, cond, sleep_us, timeout_us, addr)

#define phy_read32_poll_timeout(val, cond, sleep_us, timeout_us, args...) \
	read_poll_timeout(phy_read32, val, cond, sleep_us, timeout_us, args)

#define max(x, y)                              \
	({                                     \
		typeof(x) _max1 = (x);         \
		typeof(y) _max2 = (y);         \
		(void)(&_max1 == &_max2);      \
		_max1 > _max2 ? _max1 : _max2; \
	})

#define min(x, y)                              \
	({                                     \
		typeof(x) _min1 = (x);         \
		typeof(y) _min2 = (y);         \
		(void)(&_min1 == &_min2);      \
		_min1 < _min2 ? _min1 : _min2; \
	})

extern unsigned long ddrcfg_override_start __attribute__((section(".text")));

struct ddrcfg_override {
	uint32_t magic;
	uint32_t xtal_freq;
};

enum ddrinit_error_code {
	EDDRMC0INITFAIL = 1,
	EPOWERUP,
	ECLOCKCFG,
	EI2CXFER,
	ESPDCHECKSUM,
	EDIMMCFG,
	EFWTYPE,
	ETRAINFAIL,
	EI2CCFG,
	ETIMEDOUT,
	ETRAINTIMEOUT,
	EUARTCFG,
	EVMMUCFG,
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
void write32_with_dbg(unsigned long addr, uint32_t val);
void write64(unsigned long addr, uint64_t val);

#endif
