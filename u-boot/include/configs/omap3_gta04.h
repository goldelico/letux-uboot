/*
 * (C) Copyright 2010
 * Nikolaus Schaller <hns@goldelico.com>
 *
 * Configuration settings for the GTA04.
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

#define CONFIG_OMAP3_GTA04		y
/* enable to include patches and fixes for GTA04A2 */
/* #define CONFIG_OMAP3_GTA04A2	y */
 
#include "omap3_beagle.h"	/* share config */

#define CONFIG_CMD_UNZIP	1	/* for reducing size of splash image */

// #undef CONFIG_CMD_JFFS2
// #define CONFIG_CMD_JFFS2	1	/* to access the rootfs in NAND flash */

// FIXME: add configs for the partitions so that JFFS2 runs in the correct NAND partition

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		"GTA04 # "

#undef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY		1

#define CONFIG_SYS_NS16550_COM1		OMAP34XX_UART1	/* used for Bluetooth */

/* __CONFIG_H */
