// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#include <common.h>
#include <plat/plat.h>
#include <regs.h>
#include <i2c.h>

uint8_t i2c_addr_get(int ctrl_id)
{
	return 0;
}

void i2c_cfg(int ctrl_id)
{
	uint32_t val;

	platform_i2c_cfg();

	write32(I2C_ENABLE(0), 0);

	val = I2C_CON_SLAVE_DISABLE | I2C_CON_RESTART_EN | I2C_CON_SPEED_MASK_100 |
	      I2C_CON_MASTER_MODE;
	write32(I2C_CON(0), val);

	val = CONFIG_I2C_FREQ / 100000 / 2;
	write32(I2C_SS_HCNT(0), val);
	write32(I2C_SS_LCNT(0), val);

	write32(I2C_TAR(0), i2c_addr_get(ctrl_id));

	write32(I2C_ENABLE(0), 0);
}

int i2c_spd_read(uint8_t chip_addr, uint16_t addr, int alen, uint8_t *buf, int len)
{
	return 0;
}
