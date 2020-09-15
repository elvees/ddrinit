/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#ifndef _PRINTF_H
#define _PRINTF_H

int printf(const char *str, ...);

#ifdef CONFIG_DEBUG
#define print_dbg(fmt, ...) printf((fmt), ##__VA_ARGS__)
#else
#define print_dbg(fmt, ...)                                                                        \
	({                                                                                         \
		if (0)                                                                             \
			printf((fmt), ##__VA_ARGS__);                                              \
	})
#endif

#endif
