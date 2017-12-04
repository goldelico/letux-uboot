#!/bin/sh
if [ $# -lt 2 ]; then
  echo Usage: $0 'nand|onenand' basedir
  exit
fi
set -e
NAND=$1
if [ "$NAND" = nand ]; then
  MLO=MLO
elif [ "$NAND" = onenand ]; then
  MLO=MLO.flash
else
  echo unknown nand "$2"
  exit 1
fi
TMPFILE=$(tempfile)
dd if=/dev/zero bs=$((0x1000)) count=$((0x1c0+0x80)) | tr '\0' '\377' >"$TMPFILE"
dd if="$2/$MLO" bs=$((0x1000)) of="$TMPFILE" conv=notrunc
dd if="$2/$MLO" bs=$((0x1000)) seek=$((0x20)) of="$TMPFILE" conv=notrunc
dd if="$2/$MLO" bs=$((0x1000)) seek=$((0x40)) of="$TMPFILE" conv=notrunc
dd if="$2/$MLO" bs=$((0x1000)) seek=$((0x60)) of="$TMPFILE" conv=notrunc

dd if="$2/u-boot.img" bs=$((0x1000)) seek=$((0x80)) of="$TMPFILE" conv=notrunc
dd if="$2/boot.scr" bs=$((0x1000)) seek=$((0x1b0)) of="$TMPFILE" conv=notrunc
dd if="$2/splash.rgb16z" bs=$((0x1000)) seek=$((0x1c0)) of="$TMPFILE" conv=notrunc
dd if="$2/menu.rgb16z" bs=$((0x1000)) seek=$((0x200)) of="$TMPFILE" conv=notrunc

dfu-util -a rd -D "$TMPFILE"
SCRIPT_CMD="$(tempfile)"
SCRIPT_SCR="$(tempfile)"

cat <<EOF >"$SCRIPT_CMD"
$NAND erase 0 240000
$NAND write \${loadaddrinitrd} 0 240000
poweroff
EOF
mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n 'Boot Script' -d "$SCRIPT_CMD" "$SCRIPT_SCR"
dfu-util -a script -D "$SCRIPT_SCR"
rm "$SCRIPT_SCR" "$SCRIPT_CMD" "$TMPFILE"
sleep 0.5
dfu-util -a script -e

