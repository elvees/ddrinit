# SPDX-License-Identifier: GPL-2.0-or-later

ifeq ($(CONFIG_PLATFORM_SOLARIS),y)
obj-y += solaris.c start-solaris.S
linkfile := link-solaris.ld.S
override CFLAGS += -EL -mips64r6 -march=i6400 -mabi=64 -DR6 -fno-builtin-malloc
override LDFLAGS += -EL -mips64r6 -mabi=64
endif
