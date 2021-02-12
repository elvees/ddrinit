/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2019 RnD Center "ELVEES", JSC
 */
#ifndef _I2C_H
#define _I2C_H

#include <ddrcfg.h>

void i2c_cfg(int i2c_ctrl_id, int ctrl_id);
int i2c_spd_read(int i2c_ctrl_id, uint8_t *buf, int len);

#endif /* _I2C_H */
