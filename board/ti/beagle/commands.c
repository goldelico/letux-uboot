/* u-boot extended commands for GTA04
 *
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

#include <common.h>
#include <command.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include "backlight.h"
#include "dssfb.h"
#include "jbt6k74.h"
#include "led.h"
#include "tsc2007.h"

/* LCM commands */

static int do_lcd_color(int argc, char *argv[])
{
	unsigned int color;
	if (argc < 3) {
		printf ("lcm color: missing color (0..ffffff).\n");
		return (-1);
	}
	color=simple_strtoul(argv[2], NULL, 16);
	omap3_set_color(color);
	return 0;
}

static int do_lcd_framebuffer(int argc, char *argv[])
{
	void *addr;
	if (argc < 3) {
		printf ("lcm fb: missing address.\n");
		return (-1);
	}
	addr=(void *) simple_strtoul(argv[2], NULL, 16);
	omap3_dss_set_fb(addr);
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

/** LED commands */

static int do_led_init(int argc, char *argv[])
{
	led_init();
	return 0;
}

static void print_buttons(int status)
{
	printf("AUX: %s Power: %s Antenna: %s Pen: %s", (status&0x01)?"on":"off", (status&0x04)?"on":"off", (status&0x02)?"EXT":"INT", (status&0x08)?"1":"0");
}

static int do_led_get(int argc, char *argv[])
{
	int status=led_get_buttons();
	printf("button status: %01x\n", status);
	print_buttons(status);
	printf("\n");
	return 0;
}

static int do_led_set(int argc, char *argv[])
{ // led set hh
	static int state;
	if(argc == 2)
		state++;
	else
		state=simple_strtoul(argv[2], NULL, 16);
	led_set_led(state);
	return 0;
}

static int do_led_loop(int argc, char *argv[])
{
	printf("mirroring buttons to LEDs.\n"
		   "Press any key to stop\n\n");
	while (!tstc())
		{
			int state=led_get_buttons();
			print_buttons(state);
			printf("\r");
			led_set_led(state);	// mirror to LEDs
			udelay(100000);	// 0.1 seconds
		}
	getc();
	printf("\n");
	return 0;
}

static int do_led_blink(int argc, char *argv[])
{
	int value=0;
	printf("blinking LEDs.\n"
		   "Press any key to stop\n\n");
	while (!tstc())
		{
			led_set_led(value++);	// mirror to LEDs
			udelay(500000);	// 0.5 seconds
		}
	getc();
	return 0;
}

static int do_led(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int len;
	
	if (argc < 2) {
		printf ("led: missing subcommand.\n");
		return (-1);
	}
	
	len = strlen (argv[1]);
	if (strncmp ("ge", argv[1], 2) == 0) {
		return do_led_get (argc, argv);
	} else if (strncmp ("se", argv[1], 2) == 0) {
		return do_led_set (argc, argv);
	} else if (strncmp ("mi", argv[1], 2) == 0) {
		return do_led_loop (argc, argv);
	} else if (strncmp ("bl", argv[1], 2) == 0) {
		return do_led_blink (argc, argv);
	} else if (strncmp ("in", argv[1], 2) == 0) {
		return do_led_init (argc, argv);
	} else {
		printf ("led: unknown operation: %s\n", argv[1]);
	}
	
	return (0);
}


U_BOOT_CMD(led, 3, 0, do_led, "LED and Buttons sub-system",
		   "init - initialize GPIOs\n"
		   "get - read button status\n"
		   "set value - set LEDs state\n"
		   "mirror - read buttons and mirror to LEDs\n"
		   "blink - blink LEDs\n"
		   );

/* TSC commands */

static int do_tsc_init(int argc, char *argv[])
{
	tsc2007_init();
	return 0;
}

static int do_tsc_get(int argc, char *argv[])
{
	print_adc();
	printf("\n");
	return 0;
}

static int do_tsc_loop(int argc, char *argv[])
{
	printf("permanently reading ADCs of TSC.\n"
		   "Press any key to stop\n\n");
	while (!tstc())
		{
			print_adc();
			printf("\r");
		}
	getc();
	printf("\n");
	return 0;
}

static int do_tsc_choose(int argc, char *argv[])
{
	int value=0;
	printf("choosing by waiting for touch.\n"
		   "Press any key to stop\n\n");
	while (!tstc())
		{
			int x=read_adc(0);
			int y=read_adc(1);
			// check if pressed - then print receptive field (e.g. 1 of 8 or 16) and return 0
			printf("did choose %d/%d\n", x, y);
			return 0;
		}
	getc();
	return 0;
}

static int do_tsc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int len;
	
	if (argc < 2) {
		printf ("led: missing subcommand.\n");
		return (-1);
	}
	
	len = strlen (argv[1]);
	if (strncmp ("ge", argv[1], 2) == 0) {
		return do_tsc_get (argc, argv);
	} else if (strncmp ("lo", argv[1], 2) == 0) {
		return do_tsc_loop (argc, argv);
	} else if (strncmp ("ch", argv[1], 2) == 0) {
		return do_tsc_choose (argc, argv);
	} else if (strncmp ("in", argv[1], 2) == 0) {
		return do_tsc_init (argc, argv);
	} else {
		printf ("tsc: unknown operation: %s\n", argv[1]);
	}
	
	return (0);
}


U_BOOT_CMD(tsc, 3, 0, do_tsc, "TSC2007 sub-system",
		   "in[it] - initialize TSC2007\n"
		   "ge[t] - read ADCs\n"
		   "lo[op] - loop and display x/y coordinates\n"
		   "ch[oose] - chosse item\n"
		   );
