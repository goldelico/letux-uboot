/* u-boot driver for the TSC2007 connected to I2C2
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
#include "tsc2007.h"

#define TSC2007_BUS 2	// I2C2
#define TSC2007_ADDRESS 0x48

int tsc2007_init(void)
{
	unsigned char buf[16];
	uint addr=0;
	if(i2c_set_bus_num(TSC2007_BUS-1))
		{
			printf ("could not select I2C2\n");
			return -1;
		}
	
	if (i2c_read(TSC2007_ADDRESS, addr, 1, buf, sizeof(buf)) != 0)
		{
			printf ("Error reading the TSC.\n");
			return -1;
		}

	// initialize
	
	printf("did tsc2007_init()\n");

	return 0;
}

static int read_adc(int adcnum)
{
	// read value from given ADC
	return adcnum;
}

static void print_adc(void)
{
	printf("0: %03d 1:%03d 2:%03d 3:%03d 4: %03d 5:%03d 6:%03d 7:%03d",
		   read_adc(0),
		   read_adc(1),
		   read_adc(2),
		   read_adc(3),
		   read_adc(4),
		   read_adc(5),
		   read_adc(6),
		   read_adc(7));
}

