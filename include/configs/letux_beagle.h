/*
 * (C) Copyright 2010-2017
 * Nikolaus Schaller <hns@goldelico.com>
 *
 * Configuration settings for the TI OMAP3530 Beagle board with
 *               Openmoko Hybrid Display extension.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "omap3_beagle.h"	/* share config */

/*
 * allow either NAND or OneNAND variant
 * select by defining CONFIG_CMD_ONENAND in defconfig
 */

#undef MTDIDS_DEFAULT
#undef MTDPARTS_DEFAULT

#if defined(CONFIG_CMD_ONENAND)

#undef CONFIG_CMD_NAND
#undef CONFIG_NAND_OMAP_GPMC
#undef CONFIG_ENV_IS_IN_NAND
#undef CONFIG_SPL_NAND_SUPPORT

#define CONFIG_CMD_MTDPARTS
#define CONFIG_SYS_ONENAND_BASE	ONENAND_MAP
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS	/* required for UBI partition support */

#if !defined(CONFIG_ENV_IS_NOWHERE)
#define CONFIG_ENV_IS_IN_ONENAND
#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_OFFSET		0x240000
#undef CONFIG_ENV_ADDR
#define CONFIG_ENV_ADDR		CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE		(256 << 10)	/* 256 KiB */
#endif	/* CONFIG_ENV_IS_NOWHERE */
#define MTDIDS_DEFAULT			"onenand0=onenand"
#define MTDPARTS_DEFAULT		"mtdparts=onenand:"\
					"512k(x-loader),"\
					"1792k(u-boot),"\
					"256k(u-boot-env),"\
					"6m(kernel),"\
					"-(fs)"

/* needed to be able to load u-boot from OneNAND partition */

#define CONFIG_SYS_ONENAND_U_BOOT_OFFS 0x80000
#define CONFIG_SYS_ONENAND_PAGE_SIZE 4096

#else	/* CONFIG_CMD_ONENAND */

#define CONFIG_ENV_IS_IN_NAND		1
#define CONFIG_SPL_NAND_SUPPORT	1

#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_OFFSET		0x240000
#undef CONFIG_ENV_ADDR
#define CONFIG_ENV_ADDR		CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE		(256 << 10)	/* 256 KiB */
#define MTDIDS_DEFAULT			"nand0=nand"
#define MTDPARTS_DEFAULT		"mtdparts=nand:"\
					"512k(x-loader),"\
					"1792k(u-boot),"\
					"256k(u-boot-env),"\
					"6m(kernel),"\
					"-(fs)"

#endif	/* CONFIG_CMD_ONENAND */

#define CONFIG_CMD_UNZIP	1	/* for reducing size of splash image */

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		"Letux Beagle # "

#define CONFIG_SYS_MEMTEST_START	0x90000000
#define CONFIG_SYS_MEMTEST_END		0xb8000000

/* __CONFIG_H */
