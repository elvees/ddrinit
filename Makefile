# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2020 RnD Center "ELVEES", JSC

CONFIG_HEADER_PATH := $(CURDIR)/include/config.h
SRC_DIR := $(CURDIR)/src
TARGET := $(SRC_DIR)/ddrinit

CROSS_COMPILE ?=
override CFLAGS += -I$(CURDIR)/include -Wall -ffreestanding -fno-stack-protector -Os \
		-fstack-usage -fdump-ipa-cgraph -fdata-sections -ffunction-sections
override LDFLAGS += -nostdlib -nostartfiles -Wl,--gc-sections -Wl,-Map=$(TARGET).map

# Don't traverse above current directory while searching for .git
BUILD_COMMIT := $(shell git log --pretty=tformat:"%h" -n1 .)

ifneq ($(BUILD_COMMIT),)
override CFLAGS += -DVERSION=\"$(BUILD_COMMIT)\"
endif

## Build artifacts
all: $(TARGET).bin $(TARGET).dis

ifneq ($(FRAGMENTS),)
KCONFIG_CONFIG := .config
PLATFORM := $$(sed -n 's/^CONFIG_PLATFORM_\(.*\)=y/\L\1/p' $(KCONFIG_CONFIG))
endif

## Run interactive Python based configurator
menuconfig:
	menuconfig Kconfig

## Resolve any unresolved symbols in .config
oldconfig:
	oldconfig Kconfig

## Save a minimal configuration file as defconfig
savedefconfig:
	savedefconfig

## Apply minimal configuration from configs/<board>_defconfig
%_defconfig:
	defconfig $(CURDIR)/configs/$@

ifneq ($(FRAGMENTS),)
	KCONFIG_CONFIG=$(KCONFIG_CONFIG) scripts/merge_config.sh \
		-m $(KCONFIG_CONFIG) $(foreach f,$(subst :, ,$(FRAGMENTS)), \
			$(CURDIR)/fragments/$(PLATFORM)/$(f).fragment)
	olddefconfig Kconfig
endif

$(CONFIG_HEADER_PATH): .config
	genconfig --header-path $(CONFIG_HEADER_PATH) Kconfig

-include .config

include $(shell find $(SRC_DIR) -name Makefile)
srcs := $(shell for f in $(obj-y); do find $(SRC_DIR) -name $$f; done)
objs := $(srcs:.c=.o)
objs := $(objs:.S=.o)
objs := $(sort $(objs))

ifneq ($(linkfile),)
linkfile := $(shell find $(SRC_DIR) -name '$(linkfile)')
endif

%.o : %.c $(CONFIG_HEADER_PATH)
	$(CROSS_COMPILE)gcc -c $(CFLAGS) $< -o $@

%.o : %.S $(CONFIG_HEADER_PATH)
	$(CROSS_COMPILE)gcc -c $(CFLAGS) $< -o $@

$(TARGET).ld : $(linkfile) $(CONFIG_HEADER_PATH)
	$(CROSS_COMPILE)gcc $(CFLAGS) -E $< -P -o $@

%.bin : %.elf
	$(CROSS_COMPILE)objcopy -O binary $< $@

%.dis : %.elf
	$(CROSS_COMPILE)objdump -D  $< > $@

$(TARGET).elf: $(TARGET).ld $(objs)
	$(CROSS_COMPILE)gcc -T$^ $(LDFLAGS) -o $@

## Check stack usage
check-stack: $(TARGET).elf
	find . -name '*.cgraph' | grep -v stack-usage-log | xargs cat > stack-usage-log.cgraph
	find . -name '*.su'     | grep -v stack-usage-log | xargs cat > stack-usage-log.su
	python2 scripts/stack-usage.py --csv stack-usage.csv --json stack-usage.json > $(TARGET).su
	#TODO: Add desired stack size in defconfig
	#TODO: Compare stack size with one defined in defconfig

## Remove build artifacts
clean:
	find $(SRC_DIR) -name *.o | xargs $(RM) -f
	find . -name *.su | xargs $(RM) -f
	find . -name *.cgraph | xargs $(RM) -f
	$(RM) -f stack-usage.json stack-usage.csv
	$(RM) -f $(TARGET).elf $(TARGET).bin $(TARGET).dis $(TARGET).map $(TARGET).ld $(CONFIG_HEADER_PATH)

## Remove build artifacts and .config file
clean-all: clean
	$(RM) -f .config

## Show available targets
help: Makefile
	# Inspired by https://gist.github.com/klmr/575726c7e05d8780505a
	@echo "$$(tput bold)Available rules:$$(tput sgr0)";echo;sed -ne"/^## /{h;s/.*//;:d" \
		-e"H;n;s/^## //;td" -e"s/:.*//;G;s/\\n## /---/;s/\\n/ /g;p;}" \
		${MAKEFILE_LIST}|LC_ALL='C' sort -f|awk -F --- -v n=$$(tput cols) -v i=19 -v \
		a="$$(tput setaf 6)" -v z="$$(tput sgr0)" '{printf"%s%*s%s ",a,-i,$$1,z; \
		m=split($$2,w," ");l=n-i;for(j=1;j<=m;j++){l-=length(w[j])+1;if(l<= 0) \
		{l=n-i-length(w[j])-1;printf"\n%*s ",-i," ";}printf"%s ",w[j];}printf"\n";}' \
		|more $(shell test $(shell uname) == Darwin && echo '-Xr')

## Install build artifacts to $DESTDIR directory
install:
	install -D -m 0755 $(TARGET).bin $(DESTDIR)/ddrinit.bin
	install -D -m 0755 $(TARGET).elf $(DESTDIR)/ddrinit.elf
	install -D -m 0755 $(TARGET).map $(DESTDIR)/ddrinit.map
	install -D -m 0755 $(TARGET).ld $(DESTDIR)/ddrinit.ld
ifneq ($(wildcard $(TARGET).su),)
	install -D -m 0755 $(TARGET).su $(DESTDIR)/ddrinit.su
endif

.PHONY: all help clean clean-all menuconfig oldconfig savedefconfig install check-stack
