ifeq ($(CONFIG_PLATFORM_MCOM03),y)
ifeq ($(CONFIG_ARCH_AARCH64),y)
obj-y += mcom03.c start-aarch64.S
linkfile := link-aarch64.lds

CFLAGS += -march=armv8-a -mtune=cortex-a53 -mstrict-align
LDFLAGS += -march=armv8-a -mtune=cortex-a53
else
obj-y += mcom03.c start-mips32.S
linkfile := link-mips32.lds

CFLAGS += -mips32
LDFLAGS += -mips32
endif
endif
