# SPDX-License-Identifier: GPL-2.0-or-later

ifeq ($(CONFIG_PLATFORM_MCOM03),y)

ifeq ($(CONFIG_BOOTLOADER_BOOTROM),y)
obj-y += start-mips32-bootrom.S
else
obj-y += start-mips32-sbl.S
endif

obj-y += vmmu.c mcom03.c
linkfile := link-mips32.ld.S

override CFLAGS += -mips32
override LDFLAGS += -mips32
endif
