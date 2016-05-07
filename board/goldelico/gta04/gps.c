/* u-boot driver for the GTA04 LEDs and Buttons
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
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include <ns16550.h>
#include <twl4030.h>
#include "gps.h"

#ifdef CONFIG_OMAP3_GTA04

#define GPIO_GPS_EXT	144		// external GPS antenna plugged in
#define GPIO_GPS_ON		145		// reset for GPS module
#define GPIO_GPS_PPS	114		// Pulse per Second interrupt

#else /* Beagle Hybrid */

#define GPIO_GPS_EXT	144		// external GPS antenna plugged in
#define GPIO_GPS_ON		156
#define GPIO_GPS_PPS	138		// Pulse per Second interrupt

#endif

#define TWL4030_I2C_BUS		(1-1)	// I2C1

int gps_init(void)
{
#ifdef CONFIG_OMAP3_GTA04
	i2c_set_bus_num(TWL4030_I2C_BUS);
	/* ext. GPS Ant VSIM = 2.8 V (3.0V) */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VSIM_DEDICATED,
							/*TWL4030_PM_RECEIVER_VSIM_VSEL_28*/ 0x04 /* 0x05 */,
							TWL4030_PM_RECEIVER_VSIM_DEV_GRP,
							TWL4030_PM_RECEIVER_DEV_GRP_P1);
	udelay(5000);
#endif
	
	omap_request_gpio(GPIO_GPS_ON);
	omap_set_gpio_direction(GPIO_GPS_ON, 0);		// output
	omap_request_gpio(GPIO_GPS_EXT);
	omap_set_gpio_direction(GPIO_GPS_EXT, 1);		// input
	omap_request_gpio(GPIO_GPS_PPS);
	omap_set_gpio_direction(GPIO_GPS_PPS, 1);		// input
	return 0;
}

void gps_on(void)
{
	omap_set_gpio_dataout(GPIO_GPS_ON, 1);
	if(omap_get_gpio_datain(GPIO_GPS_EXT))
		printf("external antenna\n");
	else
		printf("internal antenna\n");
}

void gps_off(void)
{
	omap_set_gpio_dataout(GPIO_GPS_ON, 0);
}

static int lastant=-1;
static int lastpps=-1;
static long timer;

#define TIMEOUT 2	// in seconds

void gps_echo(void)
{
	#define MODE_X_DIV 16
	int baudrate=9600;
	int divisor=(CONFIG_SYS_NS16550_CLK + (baudrate * (MODE_X_DIV / 2))) / (MODE_X_DIV * baudrate);
	NS16550_reinit((NS16550_t)CONFIG_SYS_NS16550_COM2, divisor);	// initialize UART
	while (1)
		{ // echo in both directions
			int ant=omap_get_gpio_datain(GPIO_GPS_EXT);
			int pps=omap_get_gpio_datain(GPIO_GPS_PPS);
			if(ant != lastant)
				{ // changed
				if(ant)
					printf("external antenna\n");
				else
					printf("internal antenna\n");
				lastant=ant;
				}
			if(pps != lastpps)
				{ // comes only with >= 5 satellites
				if(lastpps >= 0)
					printf("PPS\n");
				lastpps=pps;
				}
			if(NS16550_tstc((NS16550_t)CONFIG_SYS_NS16550_COM2))
				{
				putc(NS16550_getc((NS16550_t)CONFIG_SYS_NS16550_COM2));	// from GPS to console
				timer=0;	// data received
				}
			if(tstc())
				{
				int c=getc();
				if(c == 0x03)	// ctrl-C
					break;
				// NS16550_putc((NS16550_t)CONFIG_SYS_NS16550_COM2, c);
				break;
				}
			if(timer++ > 2*10000)
				{ // timeout - try to wakeup/reset the chip
					printf("no data: on-off impulse\n");
					omap_set_gpio_dataout(GPIO_GPS_ON, 1);
					udelay(5000);
					omap_set_gpio_dataout(GPIO_GPS_ON, 0);
					timer=0;
				}
			udelay(100);	// 10 kHz @ 9 kbit/s
		}
	printf("\n");
}

