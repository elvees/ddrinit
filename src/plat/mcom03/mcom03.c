// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#include <config.h>
#include <plat.h>

int power_up(int ctrl_id)
{
	return 0;
}

void reset_ctl(int ctl_id, enum RESET_TYPE reset, enum RESET_ACTION action)
{
	urb_t *urb = (urb_t *)URB_BASE;
	uint32_t value = (action == ASSERT) ? 0x10 : 0;

	if (ctl_id == 0) {
		if (action == PRESETN)
			urb->ddr0presetn_ppolicy = value;
		else
			urb->ddr0resetn_ppolicy = value;
	} else {
		if (action == PRESETN)
			urb->ddr1presetn_ppolicy = value;
		else
			urb->ddr1resetn_ppolicy = value;
	}

	/* TODO: Add waiting for action completion */
}

static void setup_plls()
{
	urb_t *urb = (urb_t *)URB_BASE;

	urb->pllcfg0.sel = 1;
	urb->pllcfg0.man = 1;
	urb->pllcfg0.nf = CONFIG_PLLCFG0_NF;
	urb->pllcfg0.nr = CONFIG_PLLCFG0_NR;
	urb->pllcfg0.od = CONFIG_PLLCFG0_OD;

	while (!urb->pllcfg0.lock)
		continue;

	urb->pllcfg1.sel = 1;
	urb->pllcfg1.man = 1;
	urb->pllcfg1.nf = CONFIG_PLLCFG1_NF;
	urb->pllcfg1.nr = CONFIG_PLLCFG1_NR;
	urb->pllcfg1.od = CONFIG_PLLCFG1_OD;

	while (!urb->pllcfg1.lock)
		continue;
}

int clocks_cfg(int ctrl_id)
{
	setup_plls

		/* TBD */

		return 0;
}

void ddrinit_exit(void)
{
	/* TBD */
}

int platform_ddrcfg_get(int ctrl_id, struct ddrcfg *cfg)
{
	/* TBD */
}
