#Copyright 2020 RnD Center "ELVEES", JSC

CROSS_COMPILE ?=
CFLAGS := -I$(CURDIR)/include -Wall -nostdlib -ffreestanding -fno-stack-protector -Os
LDFLAGS := -nostdlib
CONFIG_HEADER_PATH := $(CURDIR)/include/config.h
SRC_DIR := $(CURDIR)/src
TARGET := $(SRC_DIR)/ddrinit

## Build artifacts
all: $(TARGET).bin $(TARGET).dis

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

$(CONFIG_HEADER_PATH): .config
	genconfig --header-path $(CONFIG_HEADER_PATH) Kconfig

-include .config

include $(shell find $(SRC_DIR) -name Makefile)
srcs := $(shell for f in $(obj-y); do find $(SRC_DIR) -name $$f; done)
objs := $(srcs:.c=.o)
objs := $(objs:.S=.o)

ifneq ($(linkfile),)
linkfile := $(shell find $(SRC_DIR) -name '$(linkfile)')
endif

%.o : %.c $(CONFIG_HEADER_PATH)
	$(CROSS_COMPILE)gcc -c $(CFLAGS) $< -o $@

%.o : %.S $(CONFIG_HEADER_PATH)
	$(CROSS_COMPILE)gcc -c $(CFLAGS) $< -o $@

%.bin : %.elf
	$(CROSS_COMPILE)objcopy -O binary $< $@

%.dis : %.elf
	$(CROSS_COMPILE)objdump -D  $< > $@

$(TARGET).elf: $(objs)
	$(CROSS_COMPILE)gcc -T$(linkfile) $(LDFLAGS) $^ -o $@

## Remove build artifacts
clean:
	find $(SRC_DIR) -name *.o | xargs $(RM) -f
	$(RM) -f $(TARGET).elf $(TARGET).bin $(TARGET).dis $(CONFIG_HEADER_PATH)

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

.PHONY: all help clean clean-all menuconfig oldconfig savedefconfig install
