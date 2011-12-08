/* u-boot driver for the OrtusTech COM37H3M05DTC LCM
 *
 * Copyright (C) 2006-2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
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

#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include <twl4030.h>
#include "../gta04/dssfb.h"
#include "../gta04/jbt6k74.h"	// FIXME: change to display.h and make it more generic
#include "COM37H3M05DTC.h"

#ifdef CONFIG_OMAP3_BEAGLE_EXPANDER
#define GPIO_STBY 158
#else
#define GPIO_STBY 20
#endif

// configure beagle board DSS for the COM37H3M05DTC

#define DVI_BACKGROUND_COLOR		0x00fadc29	// rgb(250, 220, 41)

#define DSS1_FCLK	432000000	// see figure 15-65
#define PIXEL_CLOCK	22400000	// approx. 22.4 MHz (will be divided from 432 MHz)

// all values are min ratings

#define VDISP	640				// vertical active area
#define VFP		2				// vertical front porch
#define VS		1				// VSYNC pulse width (negative going)
#define VBP		3				// vertical back porch
#define VDS		(VS+VBP)		// vertical data start
#define VBL		(VS+VBP+VFP)	// vertical blanking period
#define VP		(VDISP+VBL)		// vertical cycle

#define HDISP	480				// horizontal active area
#define HFP		2				// horizontal front porch
#define HS		2				// HSYNC pulse width (negative going)
#define HBP		9				// horizontal back porch
#define HDS		(HS+HBP)		// horizontal data start
#define HBL		(HS+HBP+HFP)	// horizontal blanking period
#define HP		(HDISP+HBL)		// horizontal cycle

static const struct panel_config lcm_cfg = 
{
	.timing_h	= ((HBP-1)<<20) | ((HFP-1)<<8) | ((HS-1)<<0), /* Horizantal timing */
	.timing_v	= ((VBP+0)<<20) | ((VFP+0)<<8) | ((VS-1)<<0), /* Vertical timing */
	// negative clock edge
	// negative sync pulse
	// positive DE pulse
	.pol_freq	= (1<<17)|(1<<16)|(0<<15)|(0<<14)|(1<<13)|(1<<12)|0x28,    /* Pol Freq */
	.divisor	= (0x0001<<16)|(DSS1_FCLK/PIXEL_CLOCK), /* Pixel Clock divisor from dss1_fclk */
	.lcd_size	= ((HDISP-1)<<0) | ((VDISP-1)<<16), /* as defined by LCM */
	.panel_type	= 0x01, /* TFT */
	.data_lines	= 0x03, /* 24 Bit RGB */
	.load_mode	= 0x02, /* Frame Mode */
	.panel_color	= DVI_BACKGROUND_COLOR
};

int jbt_reg_init(void)
{
	omap_request_gpio(GPIO_STBY);
	omap_set_gpio_direction(GPIO_STBY, 0);		// output
	return 0;
}

int jbt_check(void)
{
	return 0;
}

const char *jbt_state(void)
{
	return "?";
}

/* frontend function */
int jbt6k74_enter_state(enum jbt_state new_state)
{
	return 0;
}

int jbt6k74_display_onoff(int on)
{
	omap_set_gpio_dataout(GPIO_STBY, on?1:0);	// on = no STBY
	return 0;
}

int board_video_init(GraphicDevice *pGD)
{
	extern int get_board_revision(void);
	
	// FIXME: here we should pass down the GPIO(s)
	
	backlight_init();	// initialize backlight
#define REVISION_XM 0
	if(get_board_revision() == REVISION_XM) {
		/* Set VAUX1 to 3.3V for GTA04E display board */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX1_DEDICATED,
								/*TWL4030_PM_RECEIVER_VAUX1_VSEL_33*/ 0x07,
								TWL4030_PM_RECEIVER_VAUX1_DEV_GRP,
								TWL4030_PM_RECEIVER_DEV_GRP_P1);
		udelay(5000);
	}
	
	// FIXME: here we should init the TSC and pass down the GPIO numbers and resistance values
	
	if(jbt_reg_init())	// initialize SPI
		{
		printf("No LCM connected\n");
		return 1;
		}
	
	dssfb_init(&lcm_cfg);
	
	printf("did board_video_init()\n");
	return 0;
}

