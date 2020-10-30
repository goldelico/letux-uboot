/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Lubomir Rintel <lkundrak@v3.sk>
 *
 * Derived from work that's Copyright (c) 2003
 * Wolfgang Denk <wd@denx.de>
 */

#include <config.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/asm.h>

LEAF(lowlevel_init)

        /* Invalidate BTB */
        mfc0    t0, CP0_CONFIG, 7
        nop
        ori     t0, 2
        mtc0    t0, CP0_CONFIG, 7
        nop

        /*
         * CU0=UM=EXL=IE=0, BEV=ERL=1, IP2~7=1
         */
        li      t0, 0x0040FC04
        mtc0    t0, CP0_STATUS

        /* CAUSE register */
        /* IV=1, use the specical interrupt vector (0x200) */
        li      t1, 0x00800000
        mtc0    t1, CP0_CAUSE

        jr      ra
         nop

	END(lowlevel_init)
