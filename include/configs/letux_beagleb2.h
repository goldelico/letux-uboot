/*
 * (C) Copyright 2010
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

#define CONFIG_GOLDELICO_EXPANDER_B2	1	/* working with BEAGLE and B2 Expander board */

#include "letux_beagle.h"	/* share config */

#define CONFIG_CMD_SPI	1

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		"OGTA04@Beagle B2 # "


/* __CONFIG_H */
