// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2021 RnD Center "ELVEES", JSC

#include <bitops.h>
#include <common.h>
#include <ddrcfg.h>
#include <ddrspd.h>
#include <plat/plat.h>

#ifdef CONFIG_DEBUG_DDRCFG_PRINT
void ddrcfg_print(struct ddr_cfg *cfg)
{
	printf("Ranks:                                                %d\n", cfg->ranks);
	printf("Die size:                                             %d\n", cfg->die_size);
	printf("Rank size:                                            %d MB\n",
	       cfg->rank_size / 1024 / 1024);
	printf("Full size:                                            %d MB\n",
	       cfg->full_size / 1024 / 1024);
	printf("Full SDRAM Width:                                     %d bits\n",
	       cfg->full_sdram_width);
	printf("Primary SDRAM Width:                                  %d bits\n",
	       cfg->primary_sdram_width);
	printf("ECC SDRAM Width:                                      %d bits\n",
	       cfg->ecc_sdram_width);
	printf("Device Width:                                         %d bits\n",
	       cfg->device_width);

	printf("Number of Row Addresses:                              %d\n",
	       BIT(cfg->row_addr_bits));
	printf("Number of Column Addresses:                           %d\n",
	       BIT(cfg->col_addr_bits));
	printf("Number of Bank Addresses:                             %d\n",
	       BIT(cfg->bank_addr_bits));
	printf("Bank Group Addressing:                                %d\n",
	       BIT(cfg->bank_group_bits));

	printf("Clock Cycle Time (tCK):                               %d ps\n", cfg->tck);
	printf("Minimum Clock Cycle Time (tCK min):                   %d ps\n", cfg->tckmin);
	printf("Maximum Clock Cycle Time (tCK max):                   %d ps\n", cfg->tckmax);
	printf("CAS# Latency Time (tAA min):                          %d ps\n",
	       cfg->taa * cfg->tck);
	printf("Minimum Four Activate Windows Delay (tFAW):           %d ps\n",
	       cfg->tfaw * cfg->tck);
	printf("RAS to CAS Delay Time (tRCD min):                     %d ps\n",
	       cfg->trcd * cfg->tck);
	printf("Row Precharge Delay Time (tRP min):                   %d ps\n",
	       cfg->trp * cfg->tck);
	printf("Active to Precharge Delay Time (tRAS min):            %d ps\n",
	       cfg->trasmin * cfg->tck);
	printf("Normal Refresh Recovery Delay Time (tRFC1 min):       %d ps\n",
	       cfg->trfc1 * cfg->tck);
	printf("2x Mode Refresh Recovery Delay Time (tRFC2 min):      %d ps\n",
	       cfg->trfc2 * cfg->tck);
	printf("4x Mode Refresh Recovery Delay Time (tRFC4 min):      %d ps\n",
	       cfg->trfc4 * cfg->tck);
	printf("Short Row Active to Row Active Delay (tRRD_S min):    %d ps\n",
	       cfg->trrds * cfg->tck);
	printf("Long Row Active to Row Active Delay (tRRD_L min):     %d ps\n",
	       cfg->trrdl * cfg->tck);
	printf("Long CAS to CAS Delay Time (tCCD_L min):              %d ps\n",
	       cfg->tccdl * cfg->tck);
	printf("Minimum Active to Auto-Refresh Delay (tRC):           %d ps\n\n",
	       cfg->trc * cfg->tck);
}
#endif

int ddrcfg_get(int ctrl_id, struct ddr_cfg *cfg)
{
	int ret;
#ifdef CONFIG_SPD_EEPROM
#ifdef CONFIG_DRAM_TYPE_DDR4
	struct ddr4_spd spd;
#endif

	ret = spd_get(ctrl_id, &spd);
	if (ret)
		return ret;

	ret = spd_parse(&spd, cfg);

#ifdef CONFIG_DEBUG_SPD_DUMP
	spd_dump((uint8_t *)&spd);
#endif

#else
	ret = platform_ddrcfg_get(ctrl_id, cfg);
#endif

#ifdef CONFIG_DEBUG_DDRCFG_PRINT
	ddrcfg_print(cfg);
#endif
	return ret;
}
