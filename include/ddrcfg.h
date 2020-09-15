/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#ifndef _DDRCFG_H
#define _DDRCFG_H

#include <stdint.h>

struct ddr_cfg {
	char mpart[19];

	/* DIMM parameters */
	uint8_t ranks;
	uint32_t die_size;
	uint64_t rank_size;
	uint64_t full_size;
	uint8_t full_sdram_width;
	uint8_t primary_sdram_width;
	uint8_t ecc_sdram_width;
	uint8_t registered_dimm;
	uint8_t mirrored_dimm;
	uint8_t package_3ds;
	uint8_t device_width;

	/* SDRAM parameters */
	uint8_t row_addr_bits;
	uint8_t col_addr_bits;
	uint8_t bank_addr_bits;
	uint8_t bank_group_bits;

	/* DIMM timing parameters */
	uint16_t tck;
	uint16_t tckmin;
	uint16_t tckmax;
	uint8_t taa;
	uint8_t tfaw;
	uint8_t trcd;
	uint8_t trp;
	uint8_t trasmin;
	uint16_t trfc1;
	uint16_t trfc2;
	uint16_t trfc4;
	uint8_t trrds;
	uint8_t trrdl;
	uint8_t tccdl;
	uint16_t trc;

	uint32_t dq_mapping[18];
	uint32_t dq_mapping_ors;

	struct sysinfo *sysinfo;
};

struct sysinfo {
	uint64_t dram_size;
	uint8_t interleaving_enabled;
	int speed[CONFIG_DDRMC_MAX_NUMBER];
};

/* Convert picoseconds into clocks and round according to algorithm
 * described in section 13.5 of JESD79-4B
 */
static inline int ps2clk_jedec(uint32_t ps, uint32_t tck)
{
	return (ps * 1000 / tck + 974) / 1000;
}

#endif
