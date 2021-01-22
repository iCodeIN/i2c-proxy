# Define the environment variables:
# - EXEC_PASSWORD
# - EXEC_TARGET
# in order to use the insmod, rmmod, log and shell recipes.

# Change this as needed.
TARGET = remote-arm

##########################################

ifeq ($(TARGET),local)
	KPATH = /lib/modules/$(shell uname -r)/build
	CROSS_COMPILE =
	INSTALL_MOD_PATH = $(CURDIR)/install_x86
	ARCH = x86
	VERSION = $(shell uname -r)
else ifeq ($(TARGET),remote-arm)
	KPATH = ~/Development/build-linux
	CROSS_COMPILE = arm-linux-gnueabihf-
	INSTALL_MOD_PATH = $(CURDIR)/install_arm
	ARCH = arm
	VERSION = 5.11.0-rc4
endif

KO_PATH = $(INSTALL_MOD_PATH)/lib/modules/$(VERSION)/extra/src/i2c_proxy.ko

VM_SSH_PREFIX = sshpass -p $(EXEC_PASSWORD) 
VM_CMD_PREFIX = sshpass -p $(EXEC_PASSWORD) ssh -o StrictHostKeyChecking=no root@$(EXEC_TARGET)

obj-m := src/i2c_proxy.o

.PHONY: build clean insmod rmmod log

build:
	mkdir -p $(INSTALL_MOD_PATH)
	make -C $(KPATH) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
	make -C $(KPATH) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

clean:
	make -C $(KPATH) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	rm -rf $(INSTALL_MOD_PATH)

insmod:
	$(VM_SSH_PREFIX) scp $(KO_PATH) root@$(EXEC_TARGET):~/i2c_proxy.ko
	$(VM_CMD_PREFIX) insmod i2c_proxy.ko

rmmod:
	$(VM_CMD_PREFIX) rmmod i2c_proxy.ko
	$(VM_CMD_PREFIX) rm i2c_proxy.ko

log:
	$(VM_CMD_PREFIX) journalctl -kr | head -n20

shell:
	$(VM_CMD_PREFIX)
