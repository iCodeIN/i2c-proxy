TARGET = local

ifeq ($(TARGET),local)
	KPATH = /lib/modules/$(shell uname -r)/build
	CROSS_COMPILE =
	INSTALL_MOD_PATH = $(CURDIR)/install_x86
	ARCH = x86
else ifeq ($(TARGET),remote-arm)
	KPATH = ~/Development/build-linux
	CROSS_COMPILE = arm-linux-gnueabihf-
	INSTALL_MOD_PATH = $(CURDIR)/install_arm
	ARCH = arm
else
	$(error Unrecognised target $(TARGET))
endif

obj-m := src/main.o

.PHONY: build clean modules_install

build:
	mkdir -p $(INSTALL_MOD_PATH)
	make -C $(KPATH) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
	make -C $(KPATH) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

clean:
	make -C $(KPATH) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	rm -rf $(INSTALL_MOD_PATH)
