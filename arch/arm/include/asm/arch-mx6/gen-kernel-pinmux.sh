#!/bin/sh
PINSRC="$1/arch/arm/boot/dts/imx6sl-pinfunc.h"
if [ ! -f $PINSRC ] ; then
	echo "Usage: $0 path-to-kernel"
	exit 1
fi
awk '/#define M/ {print $2" = IOMUX_PAD("$4","$3","$6","$5","$7",0)," }' <$PINSRC >mx6sl-pins-kernel.h
