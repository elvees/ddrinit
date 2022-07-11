# SPDX-License-Identifier: GPL-2.0-or-later

ifeq ($(CONFIG_PLATFORM_MCOM03),y)
ifeq ($(CONFIG_ARCH_AARCH64),y)
obj-y += mcom03.c start-aarch64.S
linkfile := link-aarch64.lds

override CFLAGS += -march=armv8-a -mtune=cortex-a53 -mstrict-align
override LDFLAGS += -march=armv8-a -mtune=cortex-a53
else
obj-y += mcom03.c start-mips32.S
linkfile := link-mips32.lds

override CFLAGS += -mips32
override LDFLAGS += -mips32
endif
endif
