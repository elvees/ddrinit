// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#include <phy.h>
#include <regs.h>

#ifdef CONFIG_DRAM_TYPE_DDR4
#include <dram/ddr4/fw-imem-1d.h>
#include <dram/ddr4/fw-dmem-1d.h>
#include <dram/ddr4/pie-cfg.h>
#endif

enum firmware_type {
	FW_IMEM_1D,
	FW_DMEM_1D,
	FW_IMEM_2D,
	FW_DMEM_2D,
};

static int firmware_load(int ctrl_id, enum firmware_type fwtype)
{
	uint8_t *fw;
	unsigned long write_offset;
	int i, fw_size, size, padding_size;

	switch (fwtype) {
	case FW_IMEM_1D:
		fw = &fw_imem_1d[0];
		write_offset = CONFIG_PHY_IMEM_OFFSET;
		fw_size = fw_imem_1d_size;
		padding_size = CONFIG_PHY_IMEM_SIZE - fw_size;
		break;
	case FW_DMEM_1D:
		fw = &fw_dmem_1d[0];
		write_offset = CONFIG_PHY_DMEM_OFFSET;
		fw_size = fw_dmem_1d_size;
		padding_size = 0;
		break;
	default:
		return -EFWTYPE;
	};

	size = (fw_size % 2) ? fw_size - 1 : fw_size;
	for (i = 0; i < size; i += 2) {
		phy_write16(ctrl_id, write_offset, (fw[i + 1] << 8) | fw[i]);
		write_offset += 4;
	}

	if (fw_size % 2) {
		phy_write16(ctrl_id, write_offset, (uint16_t)fw[size]);
		write_offset += 4;
		size += 2;
	}

	if (padding_size != 0) {
		for (i = size; i < fw_size + padding_size; i += 2) {
			phy_write16(ctrl_id, write_offset, 0);
			write_offset += 4;
		}
	}

	return 0;
}

static uint32_t mail_get(int ctrl_id)
{
	uint32_t mail;

	/*  Poll the UctWriteProtShadow, looking for 0 */
	while ((phy_read32(ctrl_id, PHY_UCT_SHADOW_REGS) & 0x1))
		continue;

	mail = phy_read32(ctrl_id, PHY_UCT_WRITE_ONLY_SHADOW);

	/* Acknowledge receipt of the message */
	phy_write32(ctrl_id, PHY_DCT_WRITE_PROT, 0);

	/*  Poll the UctWriteProtShadow, looking for 1 */
	while (!(phy_read32(ctrl_id, PHY_UCT_SHADOW_REGS) & 0x1))
		continue;

	/* Complete the protocol */
	phy_write32(ctrl_id, PHY_DCT_WRITE_PROT, 1);

	return mail;
}

static uint32_t stream_message_get(int ctrl_id)
{
	uint32_t lower, upper;

	while ((phy_read32(ctrl_id, PHY_UCT_SHADOW_REGS) & 0x1))
		continue;

	lower = phy_read32(ctrl_id, PHY_UCT_WRITE_ONLY_SHADOW);
	upper = phy_read32(ctrl_id, PHY_UCT_DAT_WRITE_ONLY_SHADOW);

	/* Acknowledge receipt of the message */
	phy_write32(ctrl_id, PHY_DCT_WRITE_PROT, 0);

	/*  Poll the UctWriteProtShadow, looking for 1 */
	while (!(phy_read32(ctrl_id, PHY_UCT_SHADOW_REGS) & 0x1))
		continue;

	/* Complete the protocol */
	phy_write32(ctrl_id, PHY_DCT_WRITE_PROT, 1);

	return (upper << 16) | lower;
}

static void stream_message_decode(int ctrl_id)
{
	uint32_t string_index, arg;
	int i = 0;

	string_index = stream_message_get(ctrl_id);

	print_dbg("stream_message_decode, index: 0x%x\n", string_index);

	while (i < (string_index & 0xffff)) {
		arg = stream_message_get(ctrl_id);
		print_dbg("arg[%d] = 0x%x\n", i, arg);
		i++;
	}
}

static int training_complete_wait(int ctrl_id)
{
	uint32_t mail;

	while (1) {
		mail = mail_get(ctrl_id);
		if (mail == 0x08) {
			stream_message_decode(ctrl_id);
		} else if (mail == 0x07) {
			return 0;
		} else if (mail == 0xff) {
			return -ETRAINFAIL;
		}
	}
}

static int pie_cfg_load(int ctrl_id, int tck)
{
	struct pie_cfg_record *r = &pie_cfg[0];
	int i;

	phy_write32(ctrl_id, PHY_SEQ0BDLY0, 500000 / 8 / tck);
	phy_write32(ctrl_id, PHY_SEQ0BDLY1, 1000000 / 8 / tck);
	phy_write32(ctrl_id, PHY_SEQ0BDLY2, 10000000 / 8 / tck);
	phy_write32(ctrl_id, PHY_SEQ0BDLY3, 44);

	for (i = 0; i < ARRAY_SIZE(pie_cfg); i++) {
		phy_write32(ctrl_id, r->reg * 4, r->val);
		r++;
	}

	return 0;
}

int phy_cfg(int ctrl_id, struct ddr_cfg *cfg)
{
	int ret;

	phy_init(ctrl_id, cfg);

	phy_write32(ctrl_id, PHY_MEMRESETL, 2);
	phy_write32(ctrl_id, PHY_MICROCONT_MUXSEL, 0);

	ret = firmware_load(ctrl_id, FW_IMEM_1D);
	if (ret)
		return ret;

	ret = firmware_load(ctrl_id, FW_DMEM_1D);
	if (ret)
		return ret;

	phy_training_params_load(ctrl_id, cfg);

	/* Execute firmware */
	phy_write32(ctrl_id, PHY_MICROCONT_MUXSEL, 0x1);
	phy_write32(ctrl_id, PHY_MICRO_RESET, 0x9);
	phy_write32(ctrl_id, PHY_MICRO_RESET, 0x1);
	phy_write32(ctrl_id, PHY_MICRO_RESET, 0x0);

	/* Wait for the training firmware to complete */
	ret = training_complete_wait(ctrl_id);
	if (ret)
		return ret;

	/* Halt the microcontroller. */
	phy_write32(ctrl_id, PHY_MICRO_RESET, 0x1);

	/* Read the Message Block results */
	phy_write32(ctrl_id, PHY_MICROCONT_MUXSEL, 0);
	phy_write32(ctrl_id, PHY_MICROCONT_MUXSEL, 1);
	phy_write32(ctrl_id, PHY_MICROCONT_MUXSEL, 0);

	ret = pie_cfg_load(ctrl_id, cfg->tck);
	if (ret)
		return ret;

	phy_write32(ctrl_id, PHY_MICROCONT_MUXSEL, 1);

	return 0;
}
