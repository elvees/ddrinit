/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2019 RnD Center "ELVEES", JSC */

#ifndef _DDRMC_H
#define _DDRMC_H

#include <ddrcfg.h>
#include <regs.h>

uint16_t ddr3_mr0_get(struct ddr_cfg *cfg);
uint16_t ddr3_mr1_get(struct ddr_cfg *cfg);
uint16_t ddr3_mr2_get(struct ddr_cfg *cfg);

uint16_t ddr4_mr0_cas_get(uint8_t taa);
uint16_t ddr4_mr0_get(struct ddr_cfg *cfg);
uint16_t ddr4_mr1_get(struct ddr_cfg *cfg);
uint16_t ddr4_mr2_get(struct ddr_cfg *cfg);
uint16_t ddr4_mr3_get(struct ddr_cfg *cfg);
uint16_t ddr4_mr4_get(struct ddr_cfg *cfg);
uint16_t ddr4_mr5_get(struct ddr_cfg *cfg);
uint16_t ddr4_mr6_get(struct ddr_cfg *cfg);

uint8_t lpddr4_mr1_get(struct ddr_cfg *cfg);
uint8_t lpddr4_mr2_get(struct ddr_cfg *cfg);
uint8_t lpddr4_mr3_get(struct ddr_cfg *cfg);
uint8_t lpddr4_mr11_get(struct ddr_cfg *cfg);
uint8_t lpddr4_mr22_get(struct ddr_cfg *cfg);

void ddrmc_cfg(int ctrl_id, struct ddr_cfg *cfg);

#endif /* _DDRMC_H */
