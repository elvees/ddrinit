/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#ifndef _PHY_H
#define _PHY_H

#include <common.h>
#include <ddrcfg.h>
#include <regs.h>

static inline void phy_write32(int ctrl_id, unsigned long addr, uint32_t val)
{
	write32(PHY_BASE(ctrl_id) + addr, val);
}

static inline void phy_write16(int ctrl_id, unsigned long addr, uint16_t val)
{
	write16(PHY_BASE(ctrl_id) + addr, val);
}

static inline uint32_t phy_read32(int ctrl_id, unsigned long addr)
{
	return read32(PHY_BASE(ctrl_id) + addr);
}

int phy_cfg(int ctrl_id, struct ddr_cfg *cfg);
void phy_init(int ctrl_id, struct ddr_cfg *cfg);
void phy_training_params_load(int ctrl_id, struct ddr_cfg *cfg);

#endif /* _PHY_H */
