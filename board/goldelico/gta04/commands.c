/* u-boot extended commands for flash
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
#include "status.h"
#include "gps.h"
#include "tsc2007.h"
#include "shutdown.h"
#include "systest.h"
#include "twl4030-additions.h"

/* LCM commands */

static int do_lcd_color(int argc, char *const argv[])
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

static int do_lcd_framebuffer(int argc, char *const argv[])
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

static int do_lcd_backlight(int argc, char *const argv[])
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

static char *lcdmodel="td028";

static int do_lcd_power(int argc, char *const argv[])
{
	if(strcmp(lcdmodel, "td028") == 0)
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
		}
	return 0;
}

static int do_lcd_onoff(int argc, char *const argv[], int flag)
{
	if(strcmp(lcdmodel, "td028") == 0)
		jbt6k74_display_onoff(flag);
	else
		jbt6k74_display_onoff(flag);
	printf("display power %s\n", flag?"on":"off");
	return 0;
}

static int do_lcd_init(int argc, char *const argv[])
{
	// check argv for user specified lcdmodel
	return board_video_init(NULL);
}

static int do_lcd_start(int argc, char *const argv[])
{
	// check argv for user specified lcdmodel
	if(board_video_init(NULL))
		return 1;
	if(strcmp(lcdmodel, "td028") == 0)
		{
		jbt6k74_enter_state(2);
		jbt6k74_display_onoff(1);
		backlight_set_level(255);		
		}
	return 0;
}

static int do_lcd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
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
	} else if (strncmp ("st", argv[1], 2) == 0) {
		return do_lcd_start (argc, argv);
	} else {
		printf ("lcm: unknown operation: %s\n", argv[1]);
	}
	
	return (0);
}

U_BOOT_CMD(lcm, 3, 0, do_lcd, "LCM sub-system",
		   "init [model] - initialize DSS, GPIOs and LCM controller\n"
		   "backlight level - set backlight level\n"
		   "off - switch off\n"
		   "on - switch on\n"
		   "power mode - set power mode\n"
		   "start [model] - initialize, switch on power and enable backlight\n"
		   "color hhhhhh - switch color (can be used without init)\n"
		   "fb address - set framebuffer address (can be used without init)\n"
		   );

/* TSC commands */

static int do_tsc_init(int argc, char *const argv[])
{
	tsc2007_init();
	return 0;
}

static int do_tsc_get(int argc, char *const argv[])
{
	print_adc();
	printf("\n");
	return 0;
}

static int do_tsc_loop(int argc, char *const argv[])
{
	printf("permanently reading ADCs of TSC.\n"
		   "Press any key to stop\n\n");
	while (!tstc() && (status_get_buttons()&0x09) == 0)
		{
			print_adc();
			printf("\r");
		}
	if(tstc())
		getc();
	printf("\n");
	return 0;
}

static int do_tsc_gloop(int argc, char *const argv[])
{
	unsigned short *fb=(void *) 0x81000000;	// base address to be used as RGB16 framebuffer
	printf("permanently reading ADCs of TSC to framebuffer.\n"
		   "Press any key to stop\n\n");
	omap3_dss_set_fb(fb);
	while (!tstc() && (status_get_buttons()&0x09) == 0)
		{
		int i;
		for(i=0; i<8; i++)
			{
			int val=(480*read_adc(i))/4096;
			int x, y;
			printf("%d: %d\n", i, val);
			for(y=16*i; y<16*i+16; y++)
				{ // draw colored bar depending on current value
					for(x=0; x<480; x++)
						fb[x+480*y]=(x < val)?0xfc00:0x03ff;
				}
			}
		}
	if(tstc())
		getc();
	printf("\n");
	return 0;
}



static int tsc_choice=0;

static int do_tsc_selection(int argc, char *const argv[])
{ // tsc selection number
	if (argc != 3)
		{
			printf ("tsc selection: missing number of selection to check for.\n");
			return (-1);
		}
	return tsc_choice == simple_strtoul(argv[2], NULL, 10)?0:1;
}

static int do_tsc_choose(int argc, char *const argv[])
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
	if(tstc())
		getc();
	return 0;
}

static int do_tsc(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int len;
	
	if (argc < 2) {
		printf ("tsc: missing subcommand.\n");
		return (-1);
	}
	
	len = strlen (argv[1]);
	if (strncmp ("ge", argv[1], 2) == 0) {
		return do_tsc_get (argc, argv);
	} else if (strncmp ("lo", argv[1], 2) == 0) {
		return do_tsc_loop (argc, argv);
	} else if (strncmp ("gl", argv[1], 2) == 0) {
		return do_tsc_gloop (argc, argv);
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
		   "gl[oop] - loop and draw to framebuffer\n"
		   "ch[oose] cols rows - choose item\n"
		   "se[lection] p - check if item p (1 .. cols*rows) was selected\n"
		   );

/** Status commands */

static int do_status_init(int argc, char *const argv[])
{
	status_init();
	return 0;
}

static void print_buttons(int status)
{
	printf("AUX: %s Power: %s Antenna: %s Pen: %s", (status&0x01)?"on":"off", (status&0x08)?"on":"off", (status&0x02)?"EXT":"INT", (status&0x10)?"1":"0");
}

static int do_status_check(int argc, char *const argv[])
{ // can be used in if construct
	int state=status_get_buttons();
	if (argc < 3)
		{
			printf ("status check: missing mask.\n");
			return (-1);
		}
	state &= simple_strtoul(argv[2], NULL, 16);
	return (state != 0)?0:1;
}

static int do_status_get(int argc, char *const argv[])
{
	int status=status_get_buttons();
	printf("button status: %02x\n", status);
	print_buttons(status);
	printf("\n");
	return 0;
}

static int do_status_set(int argc, char *const argv[])
{ // status set hh
	static int state;
	if(argc == 2)
		state++;
	else
		state=simple_strtoul(argv[2], NULL, 16);
	status_set_status(state);
	return 0;
}

static int do_status_loop(int argc, char *const argv[])
{
	printf("mirroring buttons to LEDs.\n"
		   "Press any key to stop\n\n");
	while (!tstc() && !pendown(NULL, NULL))
		{
			int state=status_get_buttons();
			print_buttons(state);
			printf("\r");
			status_set_status(state);	// mirror to LEDs
			udelay(100000);	// 0.1 seconds
		}
	if(tstc())
		getc();
	printf("\n");
	return 0;
}

static int do_status_blink(int argc, char *const argv[])
{
	int value=0;
	printf("blinking LEDs.\n"
		   "Press any key to stop\n\n");
	while (!tstc() && !pendown(NULL, NULL))
		{
			status_set_status(value++);	// mirror to LEDs
			udelay(100000);	// 0.1 seconds
		}
	if(tstc())
		getc();
	return 0;
}

static int do_status_flash(int argc, char *const argv[])
{
	int len;
	
	if (argc < 3) {
		printf ("status flash: missing subcommand.\n");
		return (-1);
	}
	
	len = strlen (argv[2]);
	if (strncmp ("of", argv[2], 2) == 0) {
		return status_set_flash (0);
	} else if (strncmp ("on", argv[2], 2) == 0) {
		return status_set_flash (1);
	} else if (strncmp ("fl", argv[2], 2) == 0) {
		return status_set_flash (2);
	} else {
		printf ("status: unknown operation: %s\n", argv[1]);
		return 1;
	}
	return (0);
}

static int do_status_vibra(int argc, char *const argv[])
{
	static int value;
	if (argc < 3) {
		printf ("status vibra: missing value.\n");
		return (-1);
	}
	value=simple_strtol(argv[2], NULL, 10);
	return status_set_vibra(value);
}

static int do_status(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int len;
	
	if (argc < 2) {
		printf ("status: missing subcommand.\n");
		return (-1);
	}
	
	len = strlen (argv[1]);
	if (strncmp ("ge", argv[1], 2) == 0) {
		return do_status_get (argc, argv);
	} else if (strncmp ("se", argv[1], 2) == 0) {
		return do_status_set (argc, argv);
	} else if (strncmp ("mi", argv[1], 2) == 0) {
		return do_status_loop (argc, argv);
	} else if (strncmp ("bl", argv[1], 2) == 0) {
		return do_status_blink (argc, argv);
	} else if (strncmp ("in", argv[1], 2) == 0) {
		return do_status_init (argc, argv);
	} else if (strncmp ("ch", argv[1], 2) == 0) {
		return do_status_check (argc, argv);
	} else if (strncmp ("fl", argv[1], 2) == 0) {
		return do_status_flash (argc, argv);
	} else if (strncmp ("vi", argv[1], 2) == 0) {
		return do_status_vibra (argc, argv);
	} else {
		printf ("status: unknown operation: %s\n", argv[1]);
	}
	
	return (0);
}


U_BOOT_CMD(status, 3, 0, do_status, "LED and Buttons sub-system",
		   "in[it] - initialize GPIOs\n"
		   "ge[t] - print button status\n"
		   "ch[eck] - check button status\n"
		   "se[t] value - set LEDs state\n"
		   "mi[rror] - read buttons and mirror to LEDs\n"
		   "bl[ink] - blink LEDs\n"
		   "fl[ash] of[f] | on | fl[ash] - control torch / flashlight\n"
		   "vi[bra] -256..256 - run vibracall motor\n"
		   );

/** GPS commands */

static int do_gps_init(int argc, char *const argv[])
{
	return gps_init();
}

static int do_gps_on(int argc, char *const argv[])
{
	// should we better send a single ongoing pulse of at least 2 32 kHz cycles?
	gps_on();
	printf("GPS on\n");
	return 0;
}

static int do_gps_off(int argc, char *const argv[])
{
	gps_off();
	printf("GPS off\n");
	return 0;
}

static int do_gps_echo(int argc, char *const argv[])
{
	gps_echo();
	return 0;
}

// FIXME: "gps cmd" to send a string

static int do_gps(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int len;
	
	if (argc < 2) {
		printf ("gps: missing subcommand.\n");
		return (-1);
	}
	
	len = strlen (argv[1]);
	if (strncmp ("on", argv[1], 2) == 0) {
		return do_gps_on (argc, argv);
	} else if (strncmp ("of", argv[1], 2) == 0) {
		return do_gps_off (argc, argv);
	} else if (strncmp ("in", argv[1], 2) == 0) {
		return do_gps_init (argc, argv);
	} else if (strncmp ("ec", argv[1], 2) == 0) {
		return do_gps_echo (argc, argv);
	} else {
		printf ("gps: unknown operation: %s\n", argv[1]);
	}
	
	return (0);
}


U_BOOT_CMD(gps, 3, 0, do_gps, "GPS sub-system",
		   "init - initialize GPIOs\n"
		   "on - enable GPS\n"
		   "off - disable GPS\n"
		   "cmd string - send string\n"
		   "echo - echo GPS out to console\n"
		   );

#include "ulpi-phy.h"

static int do_systest_all(int argc, char *const argv[])
{
	unsigned short *fb=(void *) 0x81000000;	// base address to be used as RGB16 framebuffer
	printf("permanently doing complete systest.\n"
		   "Press any key to stop\n\n");
	omap3_dss_set_fb(fb);
	audiotest_init(0);
	while (!tstc() && (status_get_buttons()&0x09) == 0)
		{
		int i;
		for(i=0; i<8; i++)
			{
			int val=(480*read_adc(i))/4096;
			int x, y;
			printf("%d: %d\n", i, val);
			for(y=16*i; y<16*i+16; y++)
				{ // draw colored bar depending on current value
					for(x=0; x<480; x++)
						fb[x+480*y]=(x < val)?0xfc00:0x03ff;
				}
			}
		// show hardware test results (chip availability)
		// continue to play some sound every loop...
		}
	if(tstc())
		getc();
	printf("\n");
	return 0;
}

static int do_systest(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if(argc >= 2) {
		if (strncmp ("au", argv[1], 2) == 0) {
			return audiotest(0);
		}
		if (strncmp ("al", argv[1], 2) == 0) {
			return do_systest_all (argc, argv);
		}
		if (strncmp ("ir", argv[1], 2) == 0) {
			return irdatest();
		}
		if (strncmp ("wl", argv[1], 2) == 0) {
			return wlanbttest(1);
		}
		if (strncmp ("wp", argv[1], 2) == 0) {
			return wlanbttest(0);	// just power on
		}
		if (strncmp ("ch", argv[1], 2) == 0) {
			return twl4030_init_battery_charging();
		}
		if (strncmp ("gp", argv[1], 2) == 0) {
			return gpiotest();
		}
	}
	if(argc == 3) {
		if (strncmp ("ot", argv[1], 2) == 0) {
			return OTGchargepump(simple_strtoul(argv[2], NULL, 10));			
		}
		if (strncmp ("ul", argv[1], 2) == 0) {
			int port=simple_strtoul(argv[2], NULL, 10);	/* 0, 1, ... */
			int reg;
			for(reg=0; reg <= 0x3f; reg++)
				printf("ulpi reg %02x: %02x\n", reg, ulpi_direct_access(port, reg, 0, 0));
		}
	}
	return systest();
}

U_BOOT_CMD(systest, 3, 0, do_systest, "System Test",
		   "al[l] - graphical test mode\n"
		   "au[dio] - test audio\n"
		   "ir[da] - test IrDA\n"
		   "gp[io] - test some GPIOs\n"
		   "wl[anbt] - test WLAN/BT module\n"
		   "wp - apply power to WLAN/BT module\n"
		   "ch[arging] - init and test BCI/BKBAT\n"
		   "ot[g] n - enable/disable OTG charge pump\n"
		   "ul[pi] n - read ULPI register n\n"
		   "<no args> - test presence of I2C devices and some TPS65950 registers\n"
		   );


static int do_poweroff(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	backlight_set_level(0);
	jbt6k74_enter_state(0);
	jbt6k74_display_onoff(0);
	shutdown();	// finally shut down power
	printf ("failed to power down\n");
	return (0);
}

U_BOOT_CMD(poweroff, 2, 0, do_poweroff, "Poweroff",
		   "");

static int do_suspend(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	backlight_set_level(0);
	jbt6k74_enter_state(0);
	jbt6k74_display_onoff(0);
	suspend();	// put CPU in sleep mode so that it can be waked up by pressing the AUX button or other events
	printf ("suspend finished\n");
	return (0);
}

U_BOOT_CMD(suspend, 2, 0, do_suspend, "Suspend",
		   "");


static int do_mux(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int cols=0;
	char *addr=(char *) 0x48002030;
	printf("PADCONF [0:15] [16:31]\n");
	while(addr < (char *) 0x48002A28) {
		u16 mux=*(u16 *) addr;
		if(cols == 0)
			printf("%08x", (unsigned int) addr);			
		printf(" %c%d%c", (mux&8)?((mux&0x10?'U':'D')):' ', (mux&7), (mux&0x100)?'I':'O');
		addr+=2;
		if(addr == (char *) 0x48002262)
			addr= (char *) 0x480025D8, cols=1;	// skip block and force new line
		if(addr == (char *) 0x480025FC)
			addr= (char *) 0x48002A00, cols=1;	// skip block and force new line
		if(++cols == 2) {
			printf("\n");
			cols=0;
		}
	}
	if(cols != 0)
		printf("\n");
	return (0);
}

U_BOOT_CMD(mux, 2, 0, do_mux, "Pinmux", "");


static int do_gpio(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int g;
	if(argc == 3) {
		if (strncmp ("on", argv[1], 2) == 0) {
			g=simple_strtoul(argv[2], NULL, 10);
			omap_request_gpio(g);
			omap_set_gpio_dataout(g, 1);
			//			omap_free_gpio(g); - switches back to input
			return 0;
		}
		else if (strncmp ("of", argv[1], 2) == 0) {
			g=simple_strtoul(argv[2], NULL, 10);
			omap_request_gpio(g);
			omap_set_gpio_dataout(g, 0);
			//			omap_free_gpio(g); - switches back to input
			return 0;
		}
		else if (strncmp ("in", argv[1], 2) == 0) {
			g=simple_strtoul(argv[2], NULL, 10);
			omap_request_gpio(g);
			omap_set_gpio_direction(g, 1);
			//			omap_free_gpio(g); - switches back to input
			return 0;
		}
		else if (strncmp ("ou", argv[1], 2) == 0) {
			g=simple_strtoul(argv[2], NULL, 10);
			omap_request_gpio(g);
			omap_set_gpio_direction(g, 0);
			//			omap_free_gpio(g); - switches back to input
			return 0;
		}
	}
	if(argc == 1 || argc == 3)
		{ // no arguments or from..to
			int i=0;
			int end=6*32;
			int n=10;	// number of columns
			int col=0;
			if(argc == 3) {
				i=simple_strtoul(argv[1], NULL, 10);
				end=simple_strtoul(argv[2], NULL, 10)+1;	// include
			}
			for(; i<end; i++)
				{
				if(col == 0)
					printf("%03d", i);
				printf(" %d", omap_get_gpio_datain(i));
				if(++col == n)
					printf("\n"), col=0;
				}
			if(col != 0)
				printf("\n");	// last line		
		}
	else if(argc == 2) { // n only
		if(omap_get_gpio_datain(simple_strtoul(argv[1], NULL, 10)))
			{
			printf("1\n");
			return 1;
			}
		else
			{
			printf("0\n");
			return 0;
			}
		}
	else {
		printf ("gpio: unknown subcommand.\n");
		return (-1);
	}

	return (0);
}

U_BOOT_CMD(gpio, 3, 0, do_gpio, "GPIO sub-system",
		   " - print all\n"
		   "n - print and return state\n"
		   "m n - print state in given range\n"
		   "on n - set to 1\n"
		   "of[f] n - set to 0\n"
		   "in n - switch to input\n"
		   "ou[t] n - switch to out (does not change pinmux!)\n"
		   );
