/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2021 RnD Center "ELVEES", JSC
 */

#ifndef _PHY_CFG_H
#define _PHY_CFG_H

#include <config.h>

struct phy_cfg_record {
	unsigned int reg;
	unsigned int val;
};

static struct phy_cfg_record phy_cfg_array[] = {
	{ 0x4017c, 0x1ff },
	{ 0x4057c, 0x1ff },
	{ 0x4417c, 0x1ff },
	{ 0x4457c, 0x1ff },
	{ 0x4817c, 0x1ff },
	{ 0x4857c, 0x1ff },
	{ 0x4c17c, 0x1ff },
	{ 0x4c57c, 0x1ff },

	{ 0x154, 0x1ff },
	{ 0x4154, 0x1ff },
	{ 0x8154, 0x1ff },
	{ 0xc154, 0x1ff },
	{ 0x10154, 0x1ff },
	{ 0x14154, 0x1ff },
	{ 0x18154, 0x1ff },
	{ 0x1c154, 0x1ff },
	{ 0x20154, 0x1ff },
	{ 0x24154, 0x1ff },

	{ 0x80314, 0x6 },
	{ 0x800b8, 0x1 },
	{ 0x240810, 0x0 },
	{ 0x80090, 0xe3 },
	{ 0x800e8, 0x2 },
	{ 0x801f4, 0x212 },
	{ 0x801f0, 0x61 },
	{ 0x80158, 0xa },

	{ 0x40134, 0x600 },
	{ 0x40534, 0x600 },
	{ 0x44134, 0x600 },
	{ 0x44534, 0x600 },
	{ 0x48134, 0x600 },
	{ 0x48534, 0x600 },
	{ 0x4c134, 0x600 },
	{ 0x4c534, 0x600 },

	{ 0x40124, 0x618 },
	{ 0x40524, 0x618 },
	{ 0x44124, 0x618 },
	{ 0x44524, 0x618 },
	{ 0x48124, 0x618 },
	{ 0x48524, 0x618 },
	{ 0x4c124, 0x618 },
	{ 0x4c524, 0x618 },

	{ 0x10c, 0x3ff },
	{ 0x410c, 0x3ff },
	{ 0x810c, 0x3ff },
	{ 0xc10c, 0x3ff },
	{ 0x1010c, 0x3ff },
	{ 0x1410c, 0x3ff },
	{ 0x1810c, 0x3ff },
	{ 0x1c10c, 0x3ff },
	{ 0x2010c, 0x3ff },
	{ 0x2410c, 0x3ff },

	{ 0x80060, 0x3 },
	{ 0x801d4, 0x4 },
	{ 0x80140, 0x0 },
	{ 0x80020, 0x10b },
	{ 0x80220, 0x9 },
	{ 0x802c8, 0x104 },

	{ 0x4010c, 0x5a1 },
	{ 0x4050c, 0x5a1 },
	{ 0x4410c, 0x5a1 },
	{ 0x4450c, 0x5a1 },
	{ 0x4810c, 0x5a1 },
	{ 0x4850c, 0x5a1 },
	{ 0x4c10c, 0x5a1 },
	{ 0x4c50c, 0x5a1 },

	{ 0x803e8, 0x1 },
	{ 0x80064, 0x1 },
	{ 0x803c0, 0x0 },
	{ 0x803c4, 0x0 },
	{ 0x803c8, 0x4444 },
	{ 0x803cc, 0x8888 },
	{ 0x803d0, 0x5555 },
	{ 0x803d4, 0x0 },
	{ 0x803d8, 0x0 },
	{ 0x803dc, 0xf000 },

	{ 0x40128, 0x500 },
	{ 0x44128, 0x500 },
	{ 0x48128, 0x500 },
	{ 0x4c128, 0x500 },

	{ 0x80094, 0x0 },
	{ 0x800b4, 0x0 },
	{ 0x800b0, 0x0 },
};

static struct phy_cfg_record train_param_array[] = {
	{ 0x150000, 0xe0 },
	{ 0x150004, 0x0 },
	{ 0x150008, 0x0 },
	{ 0x15000c, 0x42a },
	{ 0x150010, 0x2 },
	{ 0x150014, 0x0 },
	{ 0x150018, 0x14 },
	{ 0x15001c, 0x0 },
#ifdef CONFIG_DISABLE_CA_TRAINING
	{ 0x150020, 0x31f },
#else
	{ 0x150020, 0x131f },
#endif
	{ 0x150024, 0x05 },
	{ 0x150028, 0x0 },
	{ 0x15002c, 0x2 },
	{ 0x150030, 0x0 },
	{ 0x150034, 0x0 },
	{ 0x150038, 0x0 },
	{ 0x15003c, 0x100 },
	{ 0x150040, 0x0 },
	{ 0x150044, 0x0 },
	{ 0x150048, 0x310 },
	{ 0x15004c, 0x0 },
	{ 0x150050, 0x0 },
	{ 0x150054, 0x0 },
	{ 0x150058, 0x0 },
	{ 0x15005c, 0x0 },
	{ 0x150060, 0x0 },
	{ 0x150064, 0x914 },
	{ 0x150068, 0x33 },
	{ 0x15006c, 0x4d64 },
	{ 0x150070, 0x4f20 },
	{ 0x150074, 0x0 },
	{ 0x150078, 0x4 },
	{ 0x15007c, 0x914 },
	{ 0x150080, 0x33 },
	{ 0x150084, 0x4d64 },
	{ 0x150088, 0x4f20 },
	{ 0x15008c, 0x0 },
	{ 0x150090, 0x4 },
	{ 0x150094, 0x0 },
	{ 0x150098, 0x0 },
	{ 0x15009c, 0x0 },
	{ 0x1500a0, 0x0 },
	{ 0x1500a4, 0x0 },
	{ 0x1500a8, 0x0 },
	{ 0x1500ac, 0x1000 },
	{ 0x1500b0, 0x3 },
	{ 0x1500b4, 0x0 },
	{ 0x1500b8, 0x0 },
	{ 0x1500bc, 0x0 },
	{ 0x1500c0, 0x0 },
	{ 0x1500c4, 0x0 },
	{ 0x1500c8, 0x1400 },
	{ 0x1500cc, 0x3309 },
	{ 0x1500d0, 0x6400 },
	{ 0x1500d4, 0x204d },
	{ 0x1500d8, 0x4f },
	{ 0x1500dc, 0x400 },
	{ 0x1500e0, 0x1400 },
	{ 0x1500e4, 0x3309 },
	{ 0x1500e8, 0x6400 },
	{ 0x1500ec, 0x204d },
	{ 0x1500f0, 0x4f },
	{ 0x1500f4, 0x400 },
	{ 0x1500f8, 0x0 },
	{ 0x1500fc, 0x0 },
	{ 0x150100, 0x0 },
	{ 0x150104, 0x0 },
	{ 0x150108, 0x0 },
	{ 0x15010c, 0x0 },
	{ 0x150110, 0x0 },
};

#endif
