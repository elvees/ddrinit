obj-$(CONFIG_SPD_EEPROM) += i2c.c

ifeq ($(CONFIG_SPD_EEPROM)$(CONFIG_DRAM_TYPE_DDR4),yy)
	obj-y += ddrspd-ddr4.c
endif

obj-y += common.c ddrcfg.c ddrinit.c printf.c uart.c
