# I2C Proxy Linux Kernel Module
Created to learn about and demonstrate the usage of the Linux kernel device model.

This module allows the user to send a byte of data as the I2C master upon writing to the device file. You can set the I2C bus to use, slave address, and value to send using the sysfs device attributes created. It will attempt to send the byte of data as soon as any data is written to the device file.

Tested against a BeagleBone Black running Linux Kernel version 5.11.0-rc4, in conjunction with the [stm32-i2c-proxy](https://github.com/marcosatti/stm32-i2c-proxy) companion project.

## Prerequisites
- Development Tools
    - On x86_64 Fedora, run `sudo dnf groupinstall "Development Tools"`.
- ARM embedded toolchain
    - On x86_64 Fedora, see [here](https://copr.fedorainfracloud.org/coprs/lantw44/arm-linux-gnueabihf-toolchain/) for a pre-packaged toolchain.
    - This is only required if you intend to build/deploy to an ARM device such as the BeagleBone Black.
- Linux Kernel Source
    - Can either be sourced through your package manager, or downloaded and built manually.

## Typical Usage
This all assumes the root user where needed.

1. `make` to compile the module - see the Makefile for options available regarding building.
2. `make insmod` to copy the module and insert it into the kernel. Make sure the `EXEC_PASSWORD` and `EXEC_TARGET` environment variables are set.
3. `make log` to check the module was inserted ok.
4. On the remote machine (you can use `make shell` to launch a SSH session), setup the `i2c_proxy` device as required, eg:
    - `echo 0x20 > /sys/devices/platform/i2c_proxy/i2c_address`
    - `echo 2 > /sys/devices/platform/i2c_proxy/i2c_bus`
    - `echo 0x99 > /sys/devices/platform/i2c_proxy/i2c_data`
5. Write to the device file created for you: `echo 1 > /dev/i2c_proxy`.
6. `make rmmod` to remove the module from the kernel.

## Licence
GPLv3+  
See LICENCE.md for a full copy of the licence text.
