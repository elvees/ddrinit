// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 RnD Center "ELVEES", JSC
 */

#include <string.h>
#include <common.h>
#include <ddrmc.h>
#include <phy.h>
#include <regs.h>
#include <plat/plat.h>

#include <dram/lpddr4/phy-cfg.h>

void phy_init(int ctrl_id, struct ddr_cfg *cfg)
{
	struct phy_cfg_record *r = &phy_cfg_array[0];
	int i;

	for (i = 0; i < ARRAY_SIZE(phy_cfg_array); i++) {
		phy_write32(ctrl_id, r->reg, r->val);
		r++;
	}
}

void phy_training_params_load(int ctrl_id, struct ddr_cfg *cfg)
{
	struct phy_cfg_record *r = &train_param_array[0];
	int i;

	for (i = 0; i < ARRAY_SIZE(train_param_array); i++) {
		phy_write32(ctrl_id, r->reg, r->val);
		r++;
	}
}
