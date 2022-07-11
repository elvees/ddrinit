/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2019 RnD Center "ELVEES", JSC */

#ifndef _I2C_H
#define _I2C_H

#include <stdint.h>

int i2c_ctrl_id_get(int ddr_ctrl_id);
int i2c_cfg(int i2c_ctrl_id, uint8_t addr);
int i2c_write_reg(int i2c_ctrl_id, uint8_t reg, uint8_t data);
int i2c_spd_read(int i2c_ctrl_id, uint8_t *buf, int len);

#endif /* _I2C_H */
