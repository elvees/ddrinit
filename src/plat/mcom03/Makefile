# SPDX-License-Identifier: GPL-2.0-or-later

ifeq ($(CONFIG_PLATFORM_MCOM03),y)

ifeq ($(CONFIG_BOOTLOADER_BOOTROM),y)

IMG_ALIGN ?= 16

obj-y += start-mips32-bootrom.S

.PHONY: ddrinit-aligned
ddrinit-aligned: $(TARGET).bin
	@echo "  ALIGN      $<"
	@truncate -s %$(IMG_ALIGN) $<

all: ddrinit-aligned

else
obj-y += start-mips32-sbl.S
endif

ifeq ($(CONFIG_BOOTSTAGE_ENABLE),y)
obj-y += bootstage.c
endif

obj-y += vmmu.c mcom03.c
linkfile := link-mips32.ld.S

override CFLAGS += -mips32 -mabi=32 -mno-gpopt -G0
override LDFLAGS += -mips32 -mabi=32 -G0
endif
