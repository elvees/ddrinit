/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2019 RnD Center "ELVEES", JSC
 */
#ifndef _DDRMC_H
#define _DDRMC_H

#include <ddrcfg.h>
#include <regs.h>

#define max(x, y)                                                                                  \
	({                                                                                         \
		typeof(x) _max1 = (x);                                                             \
		typeof(y) _max2 = (y);                                                             \
		(void)(&_max1 == &_max2);                                                          \
		_max1 > _max2 ? _max1 : _max2;                                                     \
	})

uint16_t ddr4_mr0_get(struct ddr_cfg *cfg);
uint16_t ddr4_mr1_get(struct ddr_cfg *cfg);
uint16_t ddr4_mr2_get(struct ddr_cfg *cfg);
uint16_t ddr4_mr3_get(struct ddr_cfg *cfg);
uint16_t ddr4_mr4_get(struct ddr_cfg *cfg);
uint16_t ddr4_mr5_get(struct ddr_cfg *cfg);
uint16_t ddr4_mr6_get(struct ddr_cfg *cfg);
void ddrmc_cfg(int ctrl_id, struct ddr_cfg *cfg);

#endif /* _DDRMC_H */
