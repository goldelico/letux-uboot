/* u-boot driver for the tpo TD028TTEC1 LCM
 *
 * Copyright (C) 2006-2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 * All rights reserved.
 * Copyright (C) 2010 by Golden Delicious Computers GmbH&Co. KG
 * Author: H. Nikolaus Schaller <hns@goldelico.com>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

/* modified by hns@goldelico.com
 * now is just a SPI/GPIO driver to the serial interface of the TD028TTEC1
 
 *** should all this code be moved to drivers/misc or drivers/video ?
 
 */

#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include <asm/arch/dss.h>
#include "../gta04/dssfb.h"
#include "../gta04/panel.h"

int panel_display_onoff(int on)
{
	return 1;
}

int panel_enter_state(enum panel_state new_state)
{
	return 1;
}

const char *panel_state(void)
{
	return "no panel";
}

int board_video_init(GraphicDevice *pGD)
{
	return 0;
}

int displayColumns=0;
int displayLines=0;

int panel_check(void)
{ // check if we have connectivity
	return 1;
}

int panel_reg_init(void)
{
	return 1;
}


