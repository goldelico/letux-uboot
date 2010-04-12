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
	printf("lcm state set to %s\n", jbt_state());	
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
	return board_video_init(NULL);
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

static int tsc_choice=0;

static int do_tsc_selection(int argc, char *argv[])
{ // tsc selection number
	if (argc != 3)
		{
			printf ("tsc selection: missing number of selection to check for.\n");
			return (-1);
		}
	return tsc_choice == simple_strtoul(argv[2], NULL, 10)?0:1;
}

static int pendown(int *x, int *y)
{
#if 1
	int z;
	int xx;
	int yy;
	xx=read_adc(0);
	yy=read_adc(1);
	z=read_adc(2);	// read Z
	if(z < 0)
		return 0;	// read error
#if 0
	printf("z=%04d x:%04d y:%04d\n", z, xx, yy);
#endif
	if(x) *x=xx;
	if(y) *y=yy;
	udelay(10000);	// reduce I2C traffic and debounce...
	return z > 50;	// was pressed
#else
	// must be in PENIRQ mode...
	return (led_get_buttons() & 0x08) == 0;
#endif
}

static int do_tsc_choose(int argc, char *argv[])
{ // tsc choose cols rows
	int cols;
	int rows;
	int x;
	int y;
	tsc_choice=0;	// reset choice
	if (argc != 4)
		{
			printf ("tsc choose: missing number of cols and rows.\n");
			return (-1);
		}
	cols=simple_strtoul(argv[2], NULL, 10);
	rows=simple_strtoul(argv[3], NULL, 10);
	printf("Choosing by waiting for touch.\n");
	for(y=0; y<rows; y++)
		for(x=0; x<cols; x++)
			printf("%d%s", 1+x+y*cols, (x+1==cols)?"\n":" ");
	printf("Press touch or any key to stop\n\n");
	while (!tstc())
		{
			if(pendown(NULL, NULL) && pendown(&x, &y))
				{ // still pressed - should now be stable
#if 0
					printf("xy: %d/%d\n", x, y);
					printf("xy: %d/%d\n", x*cols, y*rows);
#endif
					x=(x*cols)/4096;
					y=((4095-y)*rows)/4096;	// (0,0) is lower left corner in our hardware
					tsc_choice=1+x+y*cols;	// return 1..rows*cols
#if 0
					while(pendown(NULL, NULL))
						{ // wait for pen-up
							if(tstc())
								break;
						}
#endif
					if(tstc())
						break;
#if 1
					printf("did choose %d/%d -> %d\n", x, y, tsc_choice);
#endif
					return 0;
				}
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
	} else if (strncmp ("se", argv[1], 2) == 0) {
		return do_tsc_selection (argc, argv);
	} else if (strncmp ("in", argv[1], 2) == 0) {
		return do_tsc_init (argc, argv);
	} else {
		printf ("tsc: unknown operation: %s\n", argv[1]);
	}
	
	return (0);
}


U_BOOT_CMD(tsc, 4, 0, do_tsc, "TSC2007 sub-system",
		   "in[it] - initialize TSC2007\n"
		   "ge[t] - read ADCs\n"
		   "lo[op] - loop and display x/y coordinates\n"
		   "ch[oose] cols rows - choose item\n"
		   "se[lection] p - check if item p (1 .. cols*rows) was selected\n"
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

static int do_led_check(int argc, char *argv[])
{ // can be used in if construct
	int state=led_get_buttons();
	if (argc < 3)
		{
			printf ("led check: missing mask.\n");
			return (-1);
		}
	state &= simple_strtoul(argv[2], NULL, 16);
	return (state != 0)?0:1;
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
	while (!tstc() && !pendown(NULL, NULL))
		{
			int state=led_get_buttons();
			print_buttons(state);
			printf("\r");
			led_set_led(state);	// mirror to LEDs
			udelay(100000);	// 0.1 seconds
		}
	if(tstc())
		getc();
	printf("\n");
	return 0;
}

static int do_led_blink(int argc, char *argv[])
{
	int value=0;
	printf("blinking LEDs.\n"
		   "Press any key to stop\n\n");
	while (!tstc() && !pendown(NULL, NULL))
		{
			led_set_led(value++);	// mirror to LEDs
			udelay(500000);	// 0.5 seconds
		}
	if(tstc())
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
	} else if (strncmp ("ch", argv[1], 2) == 0) {
		return do_led_check (argc, argv);
	} else {
		printf ("led: unknown operation: %s\n", argv[1]);
	}
	
	return (0);
}


U_BOOT_CMD(led, 3, 0, do_led, "LED and Buttons sub-system",
		   "init - initialize GPIOs\n"
		   "get - print button status\n"
		   "check - check button status\n"
		   "set value - set LEDs state\n"
		   "mirror - read buttons and mirror to LEDs\n"
		   "blink - blink LEDs\n"
		   );

