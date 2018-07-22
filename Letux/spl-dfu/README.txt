Scripts and output files to boot a gta04 from usb

Required for all: dfu-util
The dfu-gta04.rules has to be copied into /etc/udev/rules.d
if it should work as ordinary user

usb-start-uboot.sh:

boots the device from usb. It requires additionally pusb.
No data is needed on mmc or nand

usb-start-kernel.sh:
expects an uboot waiting for dfu usb input and loads a kernel
optionally including a ramdisk

usb-flash-uboot-gta04.sh
flashes spl+uboot+images into nand

