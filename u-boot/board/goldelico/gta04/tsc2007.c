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
#include <i2c.h>
#include "tsc2007.h"

#define TSC2007_BUS (2-1)	// I2C2
#define TSC2007_ADDRESS 0x48

// command byte definitions:
// channel selection and power down

#define TSC2007_TEMP0	0x00
#define TSC2007_AUX		0x20
#define TSC2007_TEMP1	0x40
#define TSC2007_ACTX	0x80
#define TSC2007_ACTY	0x90
#define TSC2007_ACTXY	0xA0
#define TSC2007_X		0xc0
#define TSC2007_Y		0xd0
#define TSC2007_Z1		0xe0
#define TSC2007_Z2		0xf0

#define TSC2007_POWER_DOWN		0x00	// must be sent once after power up
#define TSC2007_ADC_ON			0x04
#define TSC2007_ADC_OFF_PENIRQ	0x08

#define TSC2007_12Bit2MHz		0x00
#define TSC2007_8Bit4MHz		0x02

// setup command

#define TSC2007_SETUP		0xb0

#define TSC2007_USE_MAV		0x00
#define TSC2007_BYPASS_MAV	0x02

#define TSC2007_50kPUP		0x00
#define TSC2007_90kPUP		0x01

/*
 int i2c_read(u_int8_t chip, u_int32_t addr, int alen, u_int8_t *buf, int len)
 int i2c_write(u_int8_t chip, u_int32_t addr, int alen, u_int8_t *buf, int len)
*/

static int didNotInit=1;

int tsc2007_cmd(int cmd)
{ // send command
	unsigned char buf[16];
	buf[0]=cmd;
	if (i2c_write(TSC2007_ADDRESS, cmd, 1, buf, 0))	// write 1 byte command
		{
			printf ("Error writing the TSC.\n");
			return 1;
		}
	return 0;
}

int tsc2007_init(void)
{
	if(i2c_set_bus_num(TSC2007_BUS))
		{
			printf ("could not select I2C2\n");
			return 1;
		}
	
	if(i2c_probe(TSC2007_ADDRESS))
		{
		printf ("could not probe TSC2007\n");
		return 1;		
		}
	
	didNotInit = tsc2007_cmd(TSC2007_SETUP|TSC2007_USE_MAV|TSC2007_50kPUP);
	didNotInit |= tsc2007_cmd(TSC2007_POWER_DOWN);
	
	if(didNotInit)
		printf("did tsc2007_init() failed.\n");
	else
		printf("did tsc2007_init()\n");

	return 0;
}

int read_adc(int adcnum)
{ // read ADC and return value in range 0..4095
	unsigned char c;
	unsigned char buf[16];
	static int cmd[]={ 
		TSC2007_X,
		TSC2007_Y,
		TSC2007_Z1,
		TSC2007_Z2,
		TSC2007_TEMP0,
		TSC2007_TEMP1,
		TSC2007_AUX,
		TSC2007_AUX
	};
	if(didNotInit)
		return -1;
	if(i2c_get_bus_num() != TSC2007_BUS && i2c_set_bus_num(TSC2007_BUS))
		{
		printf ("could not select I2C2\n");
		return -1;
		}
	c=cmd[adcnum%8] | TSC2007_ADC_ON | TSC2007_12Bit2MHz;
//	printf("send %02x\n", c);
	if (i2c_read(TSC2007_ADDRESS, c, 1, buf, 1))
		{
			printf ("Error reading the TSC.\n");
			return -1;
		}
	return ((unsigned)buf[0]) << 4;	// read 1 byte only
	
//	return (buf[0]<<4)+(buf[1]>>4);	// 12 bit
}

void print_adc(void)
{
	printf("0:%04u 1:%04u 2:%04u 3:%04u 4:%04u 5:%04u 6:%04u 7:%04u",
		   read_adc(0),
		   read_adc(1),
		   read_adc(2),
		   read_adc(3),
		   read_adc(4),
		   read_adc(5),
		   read_adc(6),
		   read_adc(7));
}

int pendown(int *x, int *y)
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
	return z > 200;	// was pressed
#else
	// must be in PENIRQ mode...
	return (status_get_buttons() & (1 << 4)) == 0;
#endif
}

