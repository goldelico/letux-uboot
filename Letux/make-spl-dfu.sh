#!/bin/sh
set -e
if test $# -lt 1 -o "$1" = --help ; then
  echo Usage: $0 'configname  [parameters to make]'
  echo e.g.
  echo  ./make-spl-dfu.sh letux_gta04 ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
  exit 0
fi
set -x
cd ..
rm -rf spl-dfu-build
mkdir spl-dfu-build
(
cat configs/$1_defconfig | grep -v CONFIG_SPL_
cat <<EOF
CONFIG_SPL_ENV_SUPPORT=y
CONFIG_SPL_YMODEM_SUPPORT=y
CONFIG_SPL_DFU_SUPPORT=y
CONFIG_SPL_HASH_SUPPORT=y
CONFIG_SPL_MUSB_NEW_SUPPORT=y
CONFIG_SPL_OS_BOOT=y
CONFIG_CMD_DFU=y
CONFIG_DFU_RAM=y
CONFIG_SPL_EXT_SUPPORT=n
CONFIG_SPL_FAT_SUPPORT=n
CONFIG_SPL_LIBDISK_SUPPORT=n
CONFIG_SPL_MMC_SUPPORT=n
CONFIG_SPL_NAND_SUPPORT=n
EOF
) > spl-dfu-build/.config
shift
make olddefconfig O=spl-dfu-build "$@"
make O=spl-dfu-build "$@"
cp spl-dfu-build/u-boot.img Letux/spl-dfu
cp spl-dfu-build/spl/u-boot-spl.bin Letux/spl-dfu/spl
