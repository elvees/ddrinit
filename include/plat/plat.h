/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#ifndef _PLAT_H
#define _PLAT_H

#include <ddrcfg.h>

enum reset_type { PRESETN, CORERESETN };

enum reset_action { RESET_ASSERT, RESET_DEASSERT };

int platform_clk_cfg(int ctrl_id, struct ddr_cfg *cfg);
int platform_ddrcfg_get(int ctrl_id, struct ddr_cfg *cfg);
void platform_i2c_cfg(void);
int platform_power_up(int ctrl_id);
void platform_reset_ctl(int ctl_id, enum reset_type reset, enum reset_action action);
int platform_system_init(int init_mask);
void platform_uart_cfg(void);

/* TBD: Rename phy_* functions */
void phy_write32(int ctrl_id, unsigned long addr, uint32_t val);
void phy_write16(int ctrl_id, unsigned long addr, uint16_t val);
uint32_t phy_read32(int ctrl_id, unsigned long addr);

#endif /* _PLAT_H */
