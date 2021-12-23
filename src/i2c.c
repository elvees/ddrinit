// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#include <common.h>
#include <plat/plat.h>
#include <regs.h>
#include <i2c.h>

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

int i2c_cfg(int i2c_ctrl_id, uint8_t addr)
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
	write32(I2C_TAR(base), addr);
	write32(I2C_ENABLE(base), 1);

	return 0;
}

static int i2c_wait(unsigned long base, uint32_t bit_mask, uint32_t value)
{
	uint32_t val = 0;

	return read32_poll_timeout(val, (val & bit_mask) == value, USEC, 8 * MSEC,
				   I2C_STATUS(base));
}

static int i2c_read_reg(int i2c_ctrl_id, uint8_t reg, uint8_t *data)
{
	unsigned long base = platform_i2c_base_get(i2c_ctrl_id);
	int ret;

	if (!base)
		return -EI2CXFER;

	write32(I2C_DATA_CMD(base), reg);
	write32(I2C_DATA_CMD(base), 0x100);

	ret = i2c_wait(base, I2C_TX_EMPTY, I2C_TX_EMPTY);
	if (ret == -ETIMEDOUT)
		return -EI2CXFER;

	ret = i2c_wait(base, I2C_RX_DATA_PRESENT, I2C_RX_DATA_PRESENT);
	if (ret == -ETIMEDOUT)
		return -EI2CXFER;

	*data = read32(I2C_DATA_CMD(base));

	return 0;
}

/*
 * Write one byte to register on I2C device. Address of device was set by i2c_cfg().
 */
int i2c_write_reg(int i2c_ctrl_id, uint8_t reg, uint8_t data)
{
	unsigned long base = platform_i2c_base_get(i2c_ctrl_id);
	int ret;

	if (!base)
		return -EI2CXFER;

	write32(I2C_DATA_CMD(base), reg);
	write32(I2C_DATA_CMD(base), data);

	ret = i2c_wait(base, I2C_TX_EMPTY, I2C_TX_EMPTY);
	if (ret == -ETIMEDOUT)
		return -EI2CXFER;

	ret = i2c_wait(base, I2C_MST_ACTIVITY, 0);
	if (ret == -ETIMEDOUT)
		return -EI2CXFER;

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
