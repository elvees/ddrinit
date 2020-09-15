/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#ifndef _DDR_SPD_H
#define _DDR_SPD_H

#include <ddrcfg.h>

/* From JEEC Standard No. 21-C release 23A */
struct ddr4_spd {
	/* General Section: Bytes 0-127 */
	uint8_t info_size_crc; /*  0 # bytes */
	uint8_t spd_rev; /*  1 Total # bytes of SPD */
	uint8_t mem_type; /*  2 Key Byte / mem type */
	uint8_t module_type; /*  3 Key Byte / Module Type */
	uint8_t density_banks; /*  4 Density and Banks	*/
	uint8_t addressing; /*  5 Addressing */
	uint8_t package_type; /*  6 Package type */
	uint8_t opt_feature; /*  7 Optional features */
	uint8_t thermal_ref; /*  8 Thermal and refresh */
	uint8_t oth_opt_features; /*  9 Other optional features */
	uint8_t res_10; /* 10 Reserved */
	uint8_t module_vdd; /* 11 Module nominal voltage */
	uint8_t organization; /* 12 Module Organization */
	uint8_t bus_width; /* 13 Module Memory Bus Width */
	uint8_t therm_sensor; /* 14 Module Thermal Sensor */
	uint8_t ext_type; /* 15 Extended module type */
	uint8_t res_16;
	uint8_t timebases; /* 17 MTb and FTB */
	uint8_t tck_min; /* 18 tCKAVGmin */
	uint8_t tck_max; /* 19 TCKAVGmax */
	uint8_t caslat_b1; /* 20 CAS latencies, 1st byte */
	uint8_t caslat_b2; /* 21 CAS latencies, 2nd byte */
	uint8_t caslat_b3; /* 22 CAS latencies, 3rd byte */
	uint8_t caslat_b4; /* 23 CAS latencies, 4th byte */
	uint8_t taa_min; /* 24 Min CAS Latency Time */
	uint8_t trcd_min; /* 25 Min RAS# to CAS# Delay Time */
	uint8_t trp_min; /* 26 Min Row Precharge Delay Time */
	uint8_t tras_trc_ext; /* 27 Upper Nibbles for tRAS and tRC */
	uint8_t tras_min_lsb; /* 28 tRASmin, lsb */
	uint8_t trc_min_lsb; /* 29 tRCmin, lsb */
	uint8_t trfc1_min_lsb; /* 30 Min Refresh Recovery Delay Time */
	uint8_t trfc1_min_msb; /* 31 Min Refresh Recovery Delay Time */
	uint8_t trfc2_min_lsb; /* 32 Min Refresh Recovery Delay Time */
	uint8_t trfc2_min_msb; /* 33 Min Refresh Recovery Delay Time */
	uint8_t trfc4_min_lsb; /* 34 Min Refresh Recovery Delay Time */
	uint8_t trfc4_min_msb; /* 35 Min Refresh Recovery Delay Time */
	uint8_t tfaw_msb; /* 36 Upper Nibble for tFAW */
	uint8_t tfaw_min; /* 37 tFAW, lsb */
	uint8_t trrds_min; /* 38 tRRD_Smin, MTB */
	uint8_t trrdl_min; /* 39 tRRD_Lmin, MTB */
	uint8_t tccdl_min; /* 40 tCCS_Lmin, MTB */
	uint8_t res_41[60 - 41]; /* 41 Rserved */
	uint8_t mapping[78 - 60]; /* 60~77 Connector to SDRAM bit map */
	uint8_t res_78[117 - 78]; /* 78~116, Reserved */
	int8_t fine_tccdl_min; /* 117 Fine offset for tCCD_Lmin */
	int8_t fine_trrdl_min; /* 118 Fine offset for tRRD_Lmin */
	int8_t fine_trrds_min; /* 119 Fine offset for tRRD_Smin */
	int8_t fine_trc_min; /* 120 Fine offset for tRCmin */
	int8_t fine_trp_min; /* 121 Fine offset for tRPmin */
	int8_t fine_trcd_min; /* 122 Fine offset for tRCDmin */
	int8_t fine_taa_min; /* 123 Fine offset for tAAmin */
	int8_t fine_tck_max; /* 124 Fine offset for tCKAVGmax */
	int8_t fine_tck_min; /* 125 Fine offset for tCKAVGmin */
	/* CRC: Bytes 126-127 */
	uint8_t crc[2]; /* 126-127 SPD CRC */

	/* Module-Specific Section: Bytes 128-255 */
	union {
		struct {
			/* 128 (Unbuffered) Module Nominal Height */
			uint8_t mod_height;
			/* 129 (Unbuffered) Module Maximum Thickness */
			uint8_t mod_thickness;
			/* 130 (Unbuffered) Reference Raw Card Used */
			uint8_t ref_raw_card;
			/* 131 (Unbuffered) Address Mapping from
			      Edge Connector to DRAM */
			uint8_t addr_mapping;
			/* 132~253 (Unbuffered) Reserved */
			uint8_t res_132[254 - 132];
			/* 254~255 CRC */
			uint8_t crc[2];
		} unbuffered;
		struct {
			/* 128 (Registered) Module Nominal Height */
			uint8_t mod_height;
			/* 129 (Registered) Module Maximum Thickness */
			uint8_t mod_thickness;
			/* 130 (Registered) Reference Raw Card Used */
			uint8_t ref_raw_card;
			/* 131 DIMM Module Attributes */
			uint8_t modu_attr;
			/* 132 RDIMM Thermal Heat Spreader Solution */
			uint8_t thermal;
			/* 133 Register Manufacturer ID Code, LSB */
			uint8_t reg_id_lo;
			/* 134 Register Manufacturer ID Code, MSB */
			uint8_t reg_id_hi;
			/* 135 Register Revision Number */
			uint8_t reg_rev;
			/* 136 Address mapping from register to DRAM */
			uint8_t reg_map;
			uint8_t ca_stren;
			uint8_t clk_stren;
			/* 139~253 Reserved */
			uint8_t res_137[254 - 139];
			/* 254~255 CRC */
			uint8_t crc[2];
		} registered;
		struct {
			/* 128 (Loadreduced) Module Nominal Height */
			uint8_t mod_height;
			/* 129 (Loadreduced) Module Maximum Thickness */
			uint8_t mod_thickness;
			/* 130 (Loadreduced) Reference Raw Card Used */
			uint8_t ref_raw_card;
			/* 131 DIMM Module Attributes */
			uint8_t modu_attr;
			/* 132 RDIMM Thermal Heat Spreader Solution */
			uint8_t thermal;
			/* 133 Register Manufacturer ID Code, LSB */
			uint8_t reg_id_lo;
			/* 134 Register Manufacturer ID Code, MSB */
			uint8_t reg_id_hi;
			/* 135 Register Revision Number */
			uint8_t reg_rev;
			/* 136 Address mapping from register to DRAM */
			uint8_t reg_map;
			/* 137 Register Output Drive Strength for CMD/Add*/
			uint8_t reg_drv;
			/* 138 Register Output Drive Strength for CK */
			uint8_t reg_drv_ck;
			/* 139 Data Buffer Revision Number */
			uint8_t data_buf_rev;
			/* 140 DRAM VrefDQ for Package Rank 0 */
			uint8_t vrefqe_r0;
			/* 141 DRAM VrefDQ for Package Rank 1 */
			uint8_t vrefqe_r1;
			/* 142 DRAM VrefDQ for Package Rank 2 */
			uint8_t vrefqe_r2;
			/* 143 DRAM VrefDQ for Package Rank 3 */
			uint8_t vrefqe_r3;
			/* 144 Data Buffer VrefDQ for DRAM Interface */
			uint8_t data_intf;
			/*
			 * 145 Data Buffer MDQ Drive Strength and RTT
			 * for data rate <= 1866
			 */
			uint8_t data_drv_1866;
			/*
			 * 146 Data Buffer MDQ Drive Strength and RTT
			 * for 1866 < data rate <= 2400
			 */
			uint8_t data_drv_2400;
			/*
			 * 147 Data Buffer MDQ Drive Strength and RTT
			 * for 2400 < data rate <= 3200
			 */
			uint8_t data_drv_3200;
			/* 148 DRAM Drive Strength */
			uint8_t dram_drv;
			/*
			 * 149 DRAM ODT (RTT_WR, RTT_NOM)
			 * for data rate <= 1866
			 */
			uint8_t dram_odt_1866;
			/*
			 * 150 DRAM ODT (RTT_WR, RTT_NOM)
			 * for 1866 < data rate <= 2400
			 */
			uint8_t dram_odt_2400;
			/*
			 * 151 DRAM ODT (RTT_WR, RTT_NOM)
			 * for 2400 < data rate <= 3200
			 */
			uint8_t dram_odt_3200;
			/*
			 * 152 DRAM ODT (RTT_PARK)
			 * for data rate <= 1866
			 */
			uint8_t dram_odt_park_1866;
			/*
			 * 153 DRAM ODT (RTT_PARK)
			 * for 1866 < data rate <= 2400
			 */
			uint8_t dram_odt_park_2400;
			/*
			 * 154 DRAM ODT (RTT_PARK)
			 * for 2400 < data rate <= 3200
			 */
			uint8_t dram_odt_park_3200;
			uint8_t res_155[254 - 155]; /* Reserved */
			/* 254~255 CRC */
			uint8_t crc[2];
		} loadreduced;
		uint8_t uc[128]; /* 128-255 Module-Specific Section */
	} mod_section;

	uint8_t res_256[320 - 256]; /* 256~319 Reserved */

	/* Module supplier's data: Byte 320~383 */
	uint8_t mmid_lsb; /* 320 Module MfgID Code LSB */
	uint8_t mmid_msb; /* 321 Module MfgID Code MSB */
	uint8_t mloc; /* 322 Mfg Location */
	uint8_t mdate[2]; /* 323~324 Mfg Date */
	uint8_t sernum[4]; /* 325~328 Module Serial Number */
	uint8_t mpart[20]; /* 329~348 Mfg's Module Part Number */
	uint8_t mrev; /* 349 Module Revision Code */
	uint8_t dmid_lsb; /* 350 DRAM MfgID Code LSB */
	uint8_t dmid_msb; /* 351 DRAM MfgID Code MSB */
	uint8_t stepping; /* 352 DRAM stepping */
	uint8_t msd[29]; /* 353~381 Mfg's Specific Data */
	uint8_t res_382[2]; /* 382~383 Reserved */

	uint8_t user[512 - 384]; /* 384~511 End User Programmable */
};

/*
 * Byte 2 Fundamental Memory Types.
 */
#define SPD_MEMTYPE_FPM (0x01)
#define SPD_MEMTYPE_EDO (0x02)
#define SPD_MEMTYPE_PIPE_NIBBLE (0x03)
#define SPD_MEMTYPE_SDRAM (0x04)
#define SPD_MEMTYPE_ROM (0x05)
#define SPD_MEMTYPE_SGRAM (0x06)
#define SPD_MEMTYPE_DDR (0x07)
#define SPD_MEMTYPE_DDR2 (0x08)
#define SPD_MEMTYPE_DDR2_FBDIMM (0x09)
#define SPD_MEMTYPE_DDR2_FBDIMM_PROBE (0x0A)
#define SPD_MEMTYPE_DDR3 (0x0B)
#define SPD_MEMTYPE_DDR4 (0x0C)

/* DIMM Type for DDR4 SPD */
#define DDR4_SPD_MODULETYPE_MASK (0x0f)
#define DDR4_SPD_MODULETYPE_EXT (0x00)
#define DDR4_SPD_MODULETYPE_RDIMM (0x01)
#define DDR4_SPD_MODULETYPE_UDIMM (0x02)
#define DDR4_SPD_MODULETYPE_SO_DIMM (0x03)
#define DDR4_SPD_MODULETYPE_LRDIMM (0x04)
#define DDR4_SPD_MODULETYPE_MINI_RDIMM (0x05)
#define DDR4_SPD_MODULETYPE_MINI_UDIMM (0x06)
#define DDR4_SPD_MODULETYPE_72B_SO_UDIMM (0x08)
#define DDR4_SPD_MODULETYPE_72B_SO_RDIMM (0x09)
#define DDR4_SPD_MODULETYPE_16B_SO_DIMM (0x0C)
#define DDR4_SPD_MODULETYPE_32B_SO_DIMM (0x0D)

int spd_get(int ctrl_id, struct ddr4_spd *spd);
int spd_parse(struct ddr4_spd *spd, struct ddr_cfg *cfg);

#endif /* _DDR_SPD_H */
