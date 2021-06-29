ifeq ($(CONFIG_PLATFORM_MCOM03),y)
obj-y += mcom03.c start-mcom03.S
linkfile := link-mcom03.lds
CFLAGS += -march=armv8-a -mtune=cortex-a53
LDFLAGS += -march=armv8-a -mtune=cortex-a53
endif
