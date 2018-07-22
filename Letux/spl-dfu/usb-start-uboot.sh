#!/bin/bash
UBOOT=u-boot.img
if [ $# -ge 1 ]; then
  if [ "$1" = --help ]; then
    echo Usage: $0 [u-boot.img]
    exit 0
  fi
  UBOOT="$1"
fi

set -e
echo Power on you gta04 with aux button pressed
pusb -f spl -d 0xd00e
# wait for dfu to appear
sleep 1
dfu-util -a uboot -D "$UBOOT"
sleep 0.5
dfu-util -a uboot -e
