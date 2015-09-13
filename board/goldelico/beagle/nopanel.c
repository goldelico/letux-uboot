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
//	printf("no panel board_video_init\n");
	return 1;
}

int displayColumns=0;
int displayLines=0;

int panel_check(void)
{ // check if we have connectivity
	return 1;
}

int panel_reg_init(void)
{
//	printf("no panel panel_reg_init\n");
	return 1;
}

void backlight_set_level(int level) { return ; }
int backlight_init(void) { return 0; }

void dssfb_init(const struct panel_config *lcm_cfg) { return ; }
void omap3_dss_go(void) { return ; }
int omap3_dss_enable_fb(int flag) { return 0; }
int omap3_dss_set_fb(void *addr) { return 0; }
int omap3_set_color(u32 color) { return 0; }

int gps_init(void) { return 0; }
void gps_on(void) { return ; }
void gps_off(void) { return ; }
void gps_echo(void) { return ; }

int systest(void) { return 0; }
int audiotest_init(int channel) { return 0; }
int audiotest_send(void) { return 0; }
int audiotest(int channel) { return 0; }
int irdatest(void) { return 0; }
int wlanbttest(int serial) { return 0; }
int OTGchargepump(int enable) { return 0; }
int gpiotest(void) { return 0; }
int keytest(void) { return 0; }

int tsc2007_init(void) { return 0; }
int read_adc(int adcnum) { return 0; }
void print_adc(void) { return ; }
int pendown(int *x, int *y) { return 0; }


