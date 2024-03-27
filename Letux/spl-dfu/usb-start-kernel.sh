#!/bin/bash
set -e
if [ $# -lt 4 ] ; then
  echo Usage $0 'uImage ramdisk|- fdt|-  [args...]'
fi
KERNEL="$1"
INITRD="$2"
FDT="$3"
shift
shift
shift
dfu-util -a kernel -D "$KERNEL"
[ "$INITRD" != "-" ] && dfu-util -a rd -D "$INITRD"
[ "$FDT" != "-" ] && dfu-util -a fdt -D "$FDT"

SCRIPT_CMD="$(tempfile)"
(
echo setenv bootargs "$*"
echo -n 'bootm ${loadaddr}'
if [ "$INITRD" != "-" ] ; then
  echo -n ' ${loadaddrinitrd}'
else
  echo -n ' -'
fi
if [ "$FDT" != "-" ] ; then
  echo  ' ${loadaddrfdt}'
else
  echo ' -'
fi
) >> "$SCRIPT_CMD"
SCRIPT_SCR="$(tempfile)"
mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n 'Boot Script' -d "$SCRIPT_CMD" "$SCRIPT_SCR"

dfu-util -a script -D "$SCRIPT_SCR"
rm "$SCRIPT_SCR" "$SCRIPT_CMD"
sleep 0.5
dfu-util -a script -e
