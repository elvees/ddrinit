/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#ifndef _PHY_H
#define _PHY_H

#include <common.h>
#include <ddrcfg.h>

int phy_cfg(int ctrl_id, struct ddr_cfg *cfg);
void phy_init(int ctrl_id, struct ddr_cfg *cfg);
void phy_training_params_load(int ctrl_id, struct ddr_cfg *cfg);
void phy_write32_with_dbg(int ctrl_id, unsigned long addr, uint32_t val);

#endif /* _PHY_H */
