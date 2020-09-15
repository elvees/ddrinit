/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2019 RnD Center "ELVEES", JSC
 */
#ifndef _I2C_H
#define _I2C_H

#include <ddrcfg.h>

void i2c_cfg(int ctrl_id);
uint8_t i2c_addr_get(int ctrl_id);
int i2c_spd_read(uint8_t chip_addr, uint16_t addr, int alen, uint8_t *buf, int len);

#endif /* _I2C_H */
