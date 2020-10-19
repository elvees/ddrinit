/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#ifndef _PLAT_H
#define _PLAT_H

#include <ddrcfg.h>

enum reset_type { PRESETN, CORERESETN };

enum reset_action { RESET_ASSERT, RESET_DEASSERT };

int clocks_cfg(int ctrl_id, struct ddr_cfg *cfg);
int platform_ddrcfg_get(int ctrl_id, struct ddr_cfg *cfg);
void platform_i2c_cfg(void);
void platform_uart_cfg(void);
int power_up(int ctrl_id);
void reset_ctl(int ctl_id, enum reset_type reset, enum reset_action action);

#endif /* _PLAT_H */
