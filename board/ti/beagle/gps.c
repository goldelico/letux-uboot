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
#include "gps.h"

#define GPIO_GPSEXT		138		// external GPS antenna plugged in
#define GPIO_GPS_ON		156

int gps_init(void)
{
//	MUX_VAL(CP(UART2_CTS),		(IEN  | PTD | DIS | M4)) /*GPIO_144*/\
//	MUX_VAL(CP(UART2_RTS),		(IEN  | PTD | DIS | M4)) /*GPIO_145*/\
//	MUX_VAL(CP(UART2_TX),		(IEN  | PTD | DIS | M0)) /*GPIO_146*/\
//	MUX_VAL(CP(UART2_RX),		(IEN  | PTD | DIS | M0)) /*GPIO_147*/\
	
	omap_request_gpio(GPIO_GPS_ON);
	omap_set_gpio_direction(GPIO_GPS_ON, 0);		// output
	omap_request_gpio(GPIO_GPSEXT);
	omap_set_gpio_direction(GPIO_GPSEXT, 1);		// input
	return 0;
}

void gps_on(void)
{
	omap_set_gpio_dataout(GPIO_GPS_ON, 1);
}

void gps_off(void)
{
	omap_set_gpio_dataout(GPIO_GPS_ON, 0);
}

/*
 _serial_putc(const char c,const int port)
 {
 if (c == '\n')
 NS16550_putc(PORT, '\r');
 
 NS16550_putc(PORT, c);
 }
 
 void
 _serial_putc_raw(const char c,const int port)
 {
 NS16550_putc(PORT, c);
 }
 
 void
 _serial_puts (const char *s,const int port)
 {
 while (*s) {
 _serial_putc (*s++,port);
 }
 }
 
 
 int
 _serial_getc(const int port)
 {
 return NS16550_getc(PORT);
 }
 
 int
 _serial_tstc(const int port)
 {
 return NS16550_tstc(PORT);
 }
 
 void
 _serial_setbrg (const int port)
 {
 int clock_divisor;
 
 clock_divisor = calc_divisor(PORT);
 NS16550_reinit(PORT, clock_divisor);
 }
*/

void gps_echo(void)
{
	#define MODE_X_DIV 16
	int baudrate=9600;
	int divisor=(CONFIG_SYS_NS16550_CLK + (baudrate * (MODE_X_DIV / 2))) / (MODE_X_DIV * baudrate);
	if(omap_get_gpio_datain(GPIO_GPSEXT))
		printf("external antenna\n");
	else
		printf("internal antenna\n");
	NS16550_reinit((NS16550_t)CONFIG_SYS_NS16550_COM2, divisor);
	// initialize UART
	// echo in both directions
	// until we press ctl-C
	while (1)
		{
			if(NS16550_tstc((NS16550_t)CONFIG_SYS_NS16550_COM2))
				putc(NS16550_getc((NS16550_t)CONFIG_SYS_NS16550_COM2));	// from GPS to console
			if(tstc())
				break;	// NS16550_putc((NS16550_t)CONFIG_SYS_NS16550_COM2, getc());
		}
	getc();
	printf("\n");
	return 0;
}

