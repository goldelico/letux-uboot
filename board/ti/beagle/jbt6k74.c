/* u-boot driver for the tpo JBT6K74-AS LCM ASIC
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

/* modified by hns@goldelico.com
 * to separate jbt commands from communication (throgh SPI or GPIOs)
 */

#include <common.h>
#include <command.h>
#include <spi.h>
#include <video_fb.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include <asm/arch/dss.h>
#include <twl4030.h>
#include "jbt6k74.h"

#if 1
#define DEBUGP(x, args...) printf("%s: " x, __FUNCTION__, ## args);
#define DEBUGPC(x, args...) printf(x, ## args);
#else
#define DEBUGP(x, args...) do { } while (0)
#define DEBUGPC(x, args...) do { } while (0)
#endif


enum jbt_register {
	JBT_REG_SLEEP_IN		= 0x10,
	JBT_REG_SLEEP_OUT		= 0x11,

	JBT_REG_DISPLAY_OFF		= 0x28,
	JBT_REG_DISPLAY_ON		= 0x29,

	JBT_REG_RGB_FORMAT		= 0x3a,
	JBT_REG_QUAD_RATE		= 0x3b,

	JBT_REG_POWER_ON_OFF		= 0xb0,
	JBT_REG_BOOSTER_OP		= 0xb1,
	JBT_REG_BOOSTER_MODE		= 0xb2,
	JBT_REG_BOOSTER_FREQ		= 0xb3,
	JBT_REG_OPAMP_SYSCLK		= 0xb4,
	JBT_REG_VSC_VOLTAGE		= 0xb5,
	JBT_REG_VCOM_VOLTAGE		= 0xb6,
	JBT_REG_EXT_DISPL		= 0xb7,
	JBT_REG_OUTPUT_CONTROL		= 0xb8,
	JBT_REG_DCCLK_DCEV		= 0xb9,
	JBT_REG_DISPLAY_MODE1		= 0xba,
	JBT_REG_DISPLAY_MODE2		= 0xbb,
	JBT_REG_DISPLAY_MODE		= 0xbc,
	JBT_REG_ASW_SLEW		= 0xbd,
	JBT_REG_DUMMY_DISPLAY		= 0xbe,
	JBT_REG_DRIVE_SYSTEM		= 0xbf,

	JBT_REG_SLEEP_OUT_FR_A		= 0xc0,
	JBT_REG_SLEEP_OUT_FR_B		= 0xc1,
	JBT_REG_SLEEP_OUT_FR_C		= 0xc2,
	JBT_REG_SLEEP_IN_LCCNT_D	= 0xc3,
	JBT_REG_SLEEP_IN_LCCNT_E	= 0xc4,
	JBT_REG_SLEEP_IN_LCCNT_F	= 0xc5,
	JBT_REG_SLEEP_IN_LCCNT_G	= 0xc6,

	JBT_REG_GAMMA1_FINE_1		= 0xc7,
	JBT_REG_GAMMA1_FINE_2		= 0xc8,
	JBT_REG_GAMMA1_INCLINATION	= 0xc9,
	JBT_REG_GAMMA1_BLUE_OFFSET	= 0xca,

	JBT_REG_BLANK_CONTROL		= 0xcf,
	JBT_REG_BLANK_TH_TV		= 0xd0,
	JBT_REG_CKV_ON_OFF		= 0xd1,
	JBT_REG_CKV_1_2			= 0xd2,
	JBT_REG_OEV_TIMING		= 0xd3,
	JBT_REG_ASW_TIMING_1		= 0xd4,
	JBT_REG_ASW_TIMING_2		= 0xd5,

	JBT_REG_HCLOCK_VGA		= 0xec,
	JBT_REG_HCLOCK_QVGA		= 0xed,

};

static const char *jbt_state_names[] = {
	[JBT_STATE_DEEP_STANDBY]	= "deep-standby",
	[JBT_STATE_SLEEP]		= "sleep",
	[JBT_STATE_NORMAL]		= "normal",
};


static struct jbt_info _jbt, *jbt = &_jbt;

static int jbt_init_regs(struct jbt_info *jbt)
{
	int rc;

	DEBUGP("entering\n");

	rc = jbt_reg_write(jbt, JBT_REG_DISPLAY_MODE1, 0x01);
	rc |= jbt_reg_write(jbt, JBT_REG_DISPLAY_MODE2, 0x00);
	rc |= jbt_reg_write(jbt, JBT_REG_RGB_FORMAT, 0x60);
	rc |= jbt_reg_write(jbt, JBT_REG_DRIVE_SYSTEM, 0x10);
	rc |= jbt_reg_write(jbt, JBT_REG_BOOSTER_OP, 0x56);
	rc |= jbt_reg_write(jbt, JBT_REG_BOOSTER_MODE, 0x33);
	rc |= jbt_reg_write(jbt, JBT_REG_BOOSTER_FREQ, 0x11);
	rc |= jbt_reg_write(jbt, JBT_REG_BOOSTER_FREQ, 0x11);
	rc |= jbt_reg_write(jbt, JBT_REG_OPAMP_SYSCLK, 0x02);
	rc |= jbt_reg_write(jbt, JBT_REG_VSC_VOLTAGE, 0x2b);
	rc |= jbt_reg_write(jbt, JBT_REG_VCOM_VOLTAGE, 0x40);
	rc |= jbt_reg_write(jbt, JBT_REG_EXT_DISPL, 0x03);
	rc |= jbt_reg_write(jbt, JBT_REG_DCCLK_DCEV, 0x04);
	/*
	 * default of 0x02 in JBT_REG_ASW_SLEW responsible for 72Hz requirement
	 * to avoid red / blue flicker
	 */
	rc |= jbt_reg_write(jbt, JBT_REG_ASW_SLEW, 0x04);
	rc |= jbt_reg_write(jbt, JBT_REG_DUMMY_DISPLAY, 0x00);

	rc |= jbt_reg_write(jbt, JBT_REG_SLEEP_OUT_FR_A, 0x11);
	rc |= jbt_reg_write(jbt, JBT_REG_SLEEP_OUT_FR_B, 0x11);
	rc |= jbt_reg_write(jbt, JBT_REG_SLEEP_OUT_FR_C, 0x11);
	rc |= jbt_reg_write16(jbt, JBT_REG_SLEEP_IN_LCCNT_D, 0x2040);
	rc |= jbt_reg_write16(jbt, JBT_REG_SLEEP_IN_LCCNT_E, 0x60c0);
	rc |= jbt_reg_write16(jbt, JBT_REG_SLEEP_IN_LCCNT_F, 0x1020);
	rc |= jbt_reg_write16(jbt, JBT_REG_SLEEP_IN_LCCNT_G, 0x60c0);

	rc |= jbt_reg_write16(jbt, JBT_REG_GAMMA1_FINE_1, 0x5533);
	rc |= jbt_reg_write(jbt, JBT_REG_GAMMA1_FINE_2, 0x00);
	rc |= jbt_reg_write(jbt, JBT_REG_GAMMA1_INCLINATION, 0x00);
	rc |= jbt_reg_write(jbt, JBT_REG_GAMMA1_BLUE_OFFSET, 0x00);
	rc |= jbt_reg_write(jbt, JBT_REG_GAMMA1_BLUE_OFFSET, 0x00);

	rc |= jbt_reg_write16(jbt, JBT_REG_HCLOCK_VGA, 0x1f0);
	rc |= jbt_reg_write(jbt, JBT_REG_BLANK_CONTROL, 0x02);
	rc |= jbt_reg_write16(jbt, JBT_REG_BLANK_TH_TV, 0x0804);
	rc |= jbt_reg_write16(jbt, JBT_REG_BLANK_TH_TV, 0x0804);

	rc |= jbt_reg_write(jbt, JBT_REG_CKV_ON_OFF, 0x01);
	rc |= jbt_reg_write16(jbt, JBT_REG_CKV_1_2, 0x0000);

	rc |= jbt_reg_write16(jbt, JBT_REG_OEV_TIMING, 0x0d0e);
	rc |= jbt_reg_write16(jbt, JBT_REG_ASW_TIMING_1, 0x11a4);
	rc |= jbt_reg_write(jbt, JBT_REG_ASW_TIMING_2, 0x0e);

#if 0
	rc |= jbt_reg_write16(jbt, JBT_REG_HCLOCK_QVGA, 0x00ff);
	rc |= jbt_reg_write16(jbt, JBT_REG_HCLOCK_QVGA, 0x00ff);
#endif

	printf("did jbt_init_regs()\n");
	return rc;
}

static int standby_to_sleep(struct jbt_info *jbt)
{
	int rc;

	DEBUGP("entering\n");

	/* three times command zero */
	rc = jbt_reg_write_nodata(jbt, 0x00);
	udelay(1000);
	rc = jbt_reg_write_nodata(jbt, 0x00);
	udelay(1000);
	rc = jbt_reg_write_nodata(jbt, 0x00);
	udelay(1000);

	/* deep standby out */
	rc |= jbt_reg_write(jbt, JBT_REG_POWER_ON_OFF, 0x17);

	return rc;
}

static int sleep_to_normal(struct jbt_info *jbt)
{
	int rc;
	DEBUGP("entering\n");

	/* RGB I/F on, RAM wirte off, QVGA through, SIGCON enable */
	rc = jbt_reg_write(jbt, JBT_REG_DISPLAY_MODE, 0x80);

	/* Quad mode off */
	rc |= jbt_reg_write(jbt, JBT_REG_QUAD_RATE, 0x00);

	/* AVDD on, XVDD on */
	rc |= jbt_reg_write(jbt, JBT_REG_POWER_ON_OFF, 0x16);

	/* Output control */
	rc |= jbt_reg_write16(jbt, JBT_REG_OUTPUT_CONTROL, 0xfff9);

	/* Sleep mode off */
	rc |= jbt_reg_write_nodata(jbt, JBT_REG_SLEEP_OUT);

	/* at this point we have like 50% grey */

	/* initialize register set */
	rc |= jbt_init_regs(jbt);
	return rc;
}

static int normal_to_sleep(struct jbt_info *jbt)
{
	int rc;
	DEBUGP("entering\n");

	rc = jbt_reg_write_nodata(jbt, JBT_REG_DISPLAY_OFF);
	rc |= jbt_reg_write16(jbt, JBT_REG_OUTPUT_CONTROL, 0x8002);
	rc |= jbt_reg_write_nodata(jbt, JBT_REG_SLEEP_IN);

	return rc;
}

static int sleep_to_standby(struct jbt_info *jbt)
{
	DEBUGP("entering\n");
	return jbt_reg_write(jbt, JBT_REG_POWER_ON_OFF, 0x00);
}

/* frontend function */
int jbt6k74_enter_state(enum jbt_state new_state)
{
	int rc = -EINVAL;

	DEBUGP("entering(old_state=%u, new_state=%u)\n", jbt->state, new_state);

	switch (jbt->state) {
	case JBT_STATE_DEEP_STANDBY:
		switch (new_state) {
		case JBT_STATE_DEEP_STANDBY:
			rc = 0;
			break;
		case JBT_STATE_SLEEP:
			rc = standby_to_sleep(jbt);
			break;
		case JBT_STATE_NORMAL:
			/* first transition into sleep */
			rc = standby_to_sleep(jbt);
			/* then transition into normal */
			rc |= sleep_to_normal(jbt);
			break;
		}
		break;
	case JBT_STATE_SLEEP:
		switch (new_state) {
		case JBT_STATE_SLEEP:
			rc = 0;
			break;
		case JBT_STATE_DEEP_STANDBY:
			rc = sleep_to_standby(jbt);
			break;
		case JBT_STATE_NORMAL:
			rc = sleep_to_normal(jbt);
			break;
		}
		break;
	case JBT_STATE_NORMAL:
		switch (new_state) {
		case JBT_STATE_NORMAL:
			rc = 0;
			break;
		case JBT_STATE_DEEP_STANDBY:
			/* first transition into sleep */
			rc = normal_to_sleep(jbt);
			/* then transition into deep standby */
			rc |= sleep_to_standby(jbt);
			break;
		case JBT_STATE_SLEEP:
			rc = normal_to_sleep(jbt);
			break;
		}
		break;
	}

	return rc;
}

int jbt6k74_display_onoff(int on)
{
	DEBUGP("entering\n");
	if (on)
		return jbt_reg_write_nodata(jbt, JBT_REG_DISPLAY_ON);
	else
		return jbt_reg_write_nodata(jbt, JBT_REG_DISPLAY_OFF);
}

#define DVI_BACKGROUND_COLOR		0x00fadc29	// rgb(250, 220, 41)

// configure beagle board DSS for the TD28TTEC1

#define DSS1_FCLK	432000000	// see figure 15-65
#define PIXEL_CLOCK	22000000	// approx. 22 MHz (will be divided from 432 MHz)

// all values are min ratings

#define VDISP	640				// vertical active area
#define VFP		4				// vertical front porch
#define VS		2				// VSYNC pulse width (negative going)
#define VBP		2				// vertical back porch
#define VDS		(VS+VBP)		// vertical data start
#define VBL		(VS+VBP+VFP)	// vertical blanking period
#define VP		(VDISP+VBL)		// vertical cycle

#define HDISP	480				// horizontal active area
#define HFP		24				// horizontal front porch
#define HS		8				// HSYNC pulse width (negative going)
#define HBP		8				// horizontal back porch
#define HDS		(HS+HBP)		// horizontal data start
#define HBL		(HS+HBP+HFP)	// horizontal blanking period
#define HP		(HDISP+HBL)		// horizontal cycle

static const struct panel_config lcm_cfg = 
{
	.timing_h	= ((HBP-1)<<20) | ((HFP-1)<<8) | ((HS-1)<<0), /* Horizantal timing */
	.timing_v	= ((VBP+0)<<20) | ((VFP+0)<<8) | ((VS-1)<<0), /* Vertical timing */
	.pol_freq	= (1<<17)|(0<<16)|(0<<15)|(1<<14)|(1<<13)|(1<<12)|0x28,    /* Pol Freq */
	.divisor	= (0x0001<<16)|(DSS1_FCLK/PIXEL_CLOCK), /* Pixel Clock divisor from dss1_fclk */
	.lcd_size	= ((HDISP-1)<<0) | ((VDISP-1)<<16), /* as defined by LCM */
	.panel_type	= 0x01, /* TFT */
	.data_lines	= 0x03, /* 24 Bit RGB */
	.load_mode	= 0x02, /* Frame Mode */
	.panel_color	= DVI_BACKGROUND_COLOR
};

void omap3_dss_go(void)
{ // push changes from shadow register to display controller
	struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
	u32 l = 0;
	
	l = readl(&dispc->control);
	l |= GO_LCD | GO_DIG;
	writel(l, &dispc->control);
}

void board_video_init(GraphicDevice *pGD)
{
	omap3_dss_panel_config(&lcm_cfg);	// set new config
	omap3_dss_enable();	// and (re)enable
	
	jbt_reg_init();		// initialize SPI
	backlight_init();	// initialize backlight
	
#if !defined(_BEAGLE_)
	S3C24X0_LCD * const lcd = S3C24X0_GetBase_LCD();

	lcd->LCDCON1 = 0x00000178; /* CLKVAL=1, BPPMODE=16bpp, TFT, ENVID=0 */

	lcd->LCDCON2 = 0x019fc3c1;
	lcd->LCDCON3 = 0x0039df67;
	lcd->LCDCON4 = 0x00000007;
	lcd->LCDCON5 = 0x0001cf09;
	lcd->LPCSEL  = 0x00000000;
#endif
	printf("did board_video_init()\n");
}

static int do_lcd_color(int argc, char *argv[])
{
	struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
	unsigned int color;
	if (argc < 3) {
		printf ("lcm color: missing color (0..ffffff).\n");
		return (-1);
	}
	color=simple_strtoul(argv[2], NULL, 16);
	writel(color, &dispc->default_color0);
	omap3_dss_go();
	return 0;
}

static int do_lcd_framebuffer(int argc, char *argv[])
{
	if (argc < 3) {
		printf ("lcm fb: missing address.\n");
		return (-1);
	}
	return 0;
}

static int do_lcd_backlight(int argc, char *argv[])
{
	unsigned char level;
	if (argc < 3) {
		printf ("lcm backlight: missing level (0..255).\n");
		return (-1);
	}
	level=simple_strtoul(argv[2], NULL, 10);
	backlight_set_level(level);
	printf("lcm backlight level set to %d (0..255)\n", level);
	return 0;
}

static int do_lcd_power(int argc, char *argv[])
{
	int state=JBT_STATE_NORMAL;
	if (argc < 3)
		{
			printf ("lcm power: missing state (0..2).\n");
			return (-1);
		}
	state=simple_strtoul(argv[2], NULL, 10);
	if(state > 2)
		{
			printf ("lcm power: invalid state (0..2).\n");
			return (-1);
		}
	jbt6k74_enter_state(state);
	printf("lcm state set to %s\n", jbt_state_names[state]);
	
	return 0;
}

static int do_lcd_onoff(int argc, char *argv[], int flag)
{
	jbt6k74_display_onoff(flag);
	printf("display power %s\n", flag?"on":"off");
	return 0;
}

static int do_lcd_init(int argc, char *argv[])
{
	board_video_init(NULL);
	return 0;
}

static int do_lcd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int len;
	
	if (argc < 2) {
		printf ("lcm: missing subcommand.\n");
		return (-1);
	}
	
	len = strlen (argv[1]);
	if (strncmp ("ba", argv[1], 2) == 0) {
		return do_lcd_backlight (argc, argv);
	} else if (strncmp ("po", argv[1], 2) == 0) {
		return do_lcd_power (argc, argv);
	} else if (strncmp ("of", argv[1], 2) == 0) {
		return do_lcd_onoff (argc, argv, 0);
	} else if (strncmp ("on", argv[1], 2) == 0) {
		return do_lcd_onoff (argc, argv, 1);
	} else if (strncmp ("in", argv[1], 2) == 0) {
		return do_lcd_init (argc, argv);
	} else if (strncmp ("co", argv[1], 2) == 0) {
		return do_lcd_color (argc, argv);
	} else if (strncmp ("fb", argv[1], 2) == 0) {
		return do_lcd_framebuffer (argc, argv);
	} else {
		printf ("lcm: unknown operation: %s\n", argv[1]);
	}
	
	return (0);
}

U_BOOT_CMD(lcm, 3, 0, do_lcd, "LCM sub-system",
		   "init - initialize DSS, GPIOs and LCM controller\n"
		   "backlight level - set backlight level\n"
		   "off - switch off\n"
		   "on - switch on\n"
		   "power mode - set power mode\n"
		   "color hhhhhh - switch color (can be used without init)\n"
		   "fb address - set framebuffer address (can be used without init)\n"
		   );
