// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#include <common.h>
#include <plat/plat.h>
#include <regs.h>
#include <i2c.h>

static uint8_t i2c_addr_get(int ctrl_id)
{
	uint8_t i2c_addr[2];
	switch (ctrl_id) {
		case 0: i2c_addr[0] = CONFIG_DIMM0_I2C_ADDR;
			i2c_addr[1] = CONFIG_DIMM1_I2C_ADDR;
			break;
		case 1: i2c_addr[0] = CONFIG_DIMM2_I2C_ADDR;
			i2c_addr[1] = CONFIG_DIMM3_I2C_ADDR;
			break;
		case 2: i2c_addr[0] = CONFIG_DIMM4_I2C_ADDR;
			i2c_addr[1] = CONFIG_DIMM5_I2C_ADDR;
			break;
		case 3: i2c_addr[0] = CONFIG_DIMM6_I2C_ADDR;
			i2c_addr[1] = CONFIG_DIMM7_I2C_ADDR;
			break;
		default: return 0;
	}
	/* TODO: Use all DIMM slots for every controller */
	return i2c_addr[0];
}

void i2c_cfg(int i2c_ctrl_id, int ctrl_id)
{
	uint32_t val;

	platform_i2c_cfg();
	write32(I2C_ENABLE(i2c_ctrl_id), 0);
	val = I2C_CON_SLAVE_DISABLE | I2C_CON_RESTART_EN | I2C_CON_MASTER_MODE |
#ifdef CONFIG_I2C_SPEED_100
	      I2C_CON_SPEED_MASK_100;
#elif CONFIG_I2C_SPEED_400
	      I2C_CON_SPEED_MASK_FAST;
#endif
	write32(I2C_CON(i2c_ctrl_id), val);

	val = CONFIG_I2C_FREQ / CONFIG_I2C_SPEED / 2;
	write32(I2C_SS_HCNT(i2c_ctrl_id), val);
	write32(I2C_SS_LCNT(i2c_ctrl_id), val);
	write32(I2C_TAR(i2c_ctrl_id), i2c_addr_get(ctrl_id));
	write32(I2C_ENABLE(i2c_ctrl_id), 1);
}

static int i2c_wait(int i2c_ctrl_id, uint32_t bit_mask)
{
	uint32_t val = 0;

	return read32_poll_timeout(I2C_STATUS(i2c_ctrl_id), val, val & bit_mask, 1, 7500);
}

static int i2c_read_reg(int i2c_ctrl_id, uint8_t reg, uint8_t *data)
{
	int ret;

	write32(I2C_DATA_CMD(i2c_ctrl_id), reg);
	write32(I2C_DATA_CMD(i2c_ctrl_id), 0x100);

	ret = i2c_wait(i2c_ctrl_id, I2C_TX_EMPTY);
	if (ret == -ETIMEDOUT)
		return -EI2CREAD;

	ret = i2c_wait(i2c_ctrl_id, I2C_RX_DATA_PRESENT);
	if (ret == -ETIMEDOUT)
		return -EI2CREAD;

	*data = read32(I2C_DATA_CMD(i2c_ctrl_id));

	return 0;
}


int i2c_spd_read(int i2c_ctrl_id, uint8_t *buf, int len)
{
	int ret, max_byte = 256;
	for (int i = 0; (i < len) && (i < max_byte); i++){
		ret = i2c_read_reg(i2c_ctrl_id, i, buf);
		if (ret)
			return ret;
		buf++;
	}

	return 0;
}
