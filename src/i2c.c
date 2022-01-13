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

int i2c_ctrl_id_get(int ddr_ctrl_id)
{
	switch (ddr_ctrl_id) {
		case 0: return CONFIG_DDR0_I2C;
		case 1: return CONFIG_DDR1_I2C;
		case 2: return CONFIG_DDR2_I2C;
		case 3: return CONFIG_DDR3_I2C;
		default: return -EI2CCFG;
	}
}

int i2c_cfg(int i2c_ctrl_id, int ctrl_id)
{
	unsigned long base = platform_i2c_base_get(i2c_ctrl_id);
	int ret;
	uint32_t val;

	if (!base)
		return -EI2CCFG;

	ret = platform_i2c_cfg(i2c_ctrl_id);
	if (ret)
		return ret;

	write32(I2C_ENABLE(base), 0);
	val = I2C_CON_SLAVE_DISABLE | I2C_CON_RESTART_EN | I2C_CON_MASTER_MODE |
#ifdef CONFIG_I2C_SPEED_100
	      I2C_CON_SPEED_MASK_100;
#elif CONFIG_I2C_SPEED_400
	      I2C_CON_SPEED_MASK_FAST;
#endif
	write32(I2C_CON(base), val);

	val = CONFIG_I2C_FREQ / CONFIG_I2C_SPEED / 2;
#ifdef CONFIG_I2C_SPEED_100
	write32(I2C_SS_HCNT(base), val);
	write32(I2C_SS_LCNT(base), val);
#elif CONFIG_I2C_SPEED_400
	write32(I2C_FS_HCNT(base), val);
	write32(I2C_FS_LCNT(base), val);
#endif
	write32(I2C_TAR(base), i2c_addr_get(ctrl_id));
	write32(I2C_ENABLE(base), 1);

	return 0;
}

static int i2c_wait(unsigned long base, uint32_t bit_mask)
{
	uint32_t val = 0;

	return read32_poll_timeout(val, val & bit_mask, USEC, 8 * MSEC, I2C_STATUS(base));
}

static int i2c_read_reg(int i2c_ctrl_id, uint8_t reg, uint8_t *data)
{
	unsigned long base = platform_i2c_base_get(i2c_ctrl_id);
	int ret;

	if (!base)
		return -EI2CREAD;

	write32(I2C_DATA_CMD(base), reg);
	write32(I2C_DATA_CMD(base), 0x100);

	ret = i2c_wait(base, I2C_TX_EMPTY);
	if (ret == -ETIMEDOUT)
		return -EI2CREAD;

	ret = i2c_wait(base, I2C_RX_DATA_PRESENT);
	if (ret == -ETIMEDOUT)
		return -EI2CREAD;

	*data = read32(I2C_DATA_CMD(base));

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
