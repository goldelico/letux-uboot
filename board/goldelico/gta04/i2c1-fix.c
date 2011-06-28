/* u-boot driver for the layout issue of the I2C1 on the GTA04A2 board
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
 * fragments taken from http://www.koders.com/c/fidDC48AC380BE8326E4C3F14F8195BBDB8ED713932.aspx?s=%22Cam%22#L4
 * see: // see http://www.cc5x.de/I2C.html
 *
 */


#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/i2c.h>
#include <asm/io.h>

#ifdef CONFIG_OMAP3_GTA04A2

/* #error this is for GTA04A2 only */

#include "../../../drivers/i2c/omap24xx_i2c.h"

/* this is a bit-bang driver for the I2C1
 since the GTA04A2 board has swapped SCL and SDA.
 We can access these lines in System Test Mode
 through the I2C_SYSTEST register in ST_EN mode
 */

#define I2C_SYSTEST_ST_EN	(1 << 15) /* System test enable */
#define I2C_SYSTEST_FREE	(1 << 14) /* Free running mode, on brkpoint) */
#define I2C_SYSTEST_TMODE_MASK	(3 << 12) /* Test mode select */
#define I2C_SYSTEST_TMODE_SHIFT	(12)	  /* Test mode select */
#define I2C_SYSTEST_SCL_I	(1 << 3)  /* SCL line sense input value */
#define I2C_SYSTEST_SCL_O	(1 << 2)  /* SCL line drive output value */
#define I2C_SYSTEST_SDA_I	(1 << 1)  /* SDA line sense input value */
#define I2C_SYSTEST_SDA_O	(1 << 0)  /* SDA line drive output value */

#define KBIT	100

static inline void delay_full_clock(void)
{
	udelay(1000/KBIT);
}
static inline void delay_half_clock(void)
{
	udelay(500/KBIT);
}
static inline void delay_quarter_clock(void)
{
	udelay(250/KBIT);
}

static inline void setscl(struct i2c *base, int state)
{
	if(base == (struct i2c *)I2C_BASE1)
		{ // swap SCL&SDA on I2C1 due to hardware bug
			if (state) {
				base->systest |= I2C_SYSTEST_SDA_O;
			} else {
				base->systest &= ~I2C_SYSTEST_SDA_O;
			}			
		}
	else
		{		
			if (state) {
				base->systest |= I2C_SYSTEST_SCL_O;
			} else {
				base->systest &= ~I2C_SYSTEST_SCL_O;
			}
		}
}

static inline void setsda(struct i2c *base, int state)
{
	if(base == (struct i2c *)I2C_BASE1)
		{ // swap SCL&SDA on I2C1 due to hardware bug
			if (state) {
				base->systest |= I2C_SYSTEST_SCL_O;
			} else {
				base->systest &= ~I2C_SYSTEST_SCL_O;
			}
		}
	else
		{
			if (state) {
				base->systest |= I2C_SYSTEST_SDA_O;
			} else {
				base->systest &= ~I2C_SYSTEST_SDA_O;
			}
		}
} 

static inline int getscl(struct i2c *base)
{
	if(base == (struct i2c *)I2C_BASE1)
		{ // swap SCL&SDA on I2C1 due to hardware bug
			return (base->systest & I2C_SYSTEST_SDA_I) ? 1 : 0;
		}
	else
		return (base->systest & I2C_SYSTEST_SCL_I) ? 1 : 0;
}

static inline int getsda(struct i2c *base)
{
	if(base == (struct i2c *)I2C_BASE1)
		{ // swap SCL&SDA on I2C1 due to hardware bug
			return (base->systest & I2C_SYSTEST_SCL_I) ? 1 : 0;			
		}
	else
		return (base->systest & I2C_SYSTEST_SDA_I) ? 1 : 0;
}

static inline void busidle(struct i2c *base)
{
	/*
	 * float the SCL and SDA lines. The lines have pull-ups
	 */
	setscl(base, 1);
	setsda(base, 1);
}

static inline void start(struct i2c *base)
{
	busidle(base);
	delay_full_clock();
	setsda(base, 0);	// switch data during SCL=H -> this is START
	delay_quarter_clock();
	setscl(base, 0);	// switch clock -> prepare for bits
	//	printf("start\n");
}

static inline void stop(struct i2c *base)
{
	setsda(base, 0);	// set next data bit
	delay_quarter_clock();
	setscl(base, 1);	// this is like sending a 0 bit
	delay_half_clock();
	setsda(base, 1);	// but switch data during clock -> STOP
	delay_quarter_clock();
}

static inline void write_bit(struct i2c *base, int bit)
{
//	printf("write bit %d\n", bit);
	setsda(base, bit);
	delay_quarter_clock();
	setscl(base, 1);
	delay_half_clock();
	setscl(base, 0);
	delay_quarter_clock();
}

static inline int read_bit(struct i2c *base)
{
	int bit;
	setsda(base, 1);	// so that we can read from the OC line...
	delay_quarter_clock();
	setscl(base, 1);
	delay_half_clock();
	setscl(base, 0);
	bit = getsda(base);
	delay_quarter_clock();
//	printf("read bit %d\n", bit);
	return bit;
}

static int write_byte(struct i2c *base, int byte)
{
	int i;
	
	for (i=7; i>=0; i--)
		write_bit(base, (byte & (1<<i)) != 0);
	return read_bit(base) == 0; // ok (1) if NACK is asserted
}

static int read_byte(struct i2c *base)
{
	int i;
	u8 byte=0;
		
	for (i=7; i>=0; i--)
		byte = (byte << 1) | read_bit(base);
	return byte;
}

int i2c_bitbang_probe(struct i2c *base, unchar devaddr)
{
//	printf("i2c probe %02x SDA %d SCL %d\n", devaddr, getsda(base), getscl(base));
	start(base);
	if (write_byte(base, (devaddr<<1))) { // write probe
		stop(base);	// send stop condition
		return 0;	// ok
	}
	stop(base);	// send stop condition
	return 1;	// fail
}

int i2c_bitbang_write_byte (struct i2c *base, u8 devaddr, u8 regoffset, u8 value)
{
//	printf("i2c-bb write dev=%02x regoffset=%02x value=%02x\n", devaddr, regoffset, value);
	start(base);
	if (write_byte(base, (devaddr<<1))) { // send chip address for write command
		if (write_byte(base, regoffset)) { // send register offset
			if (write_byte(base, value)) { // send value
				stop(base);	// send stop condition
				return 0;	// ok
			}
		}
	}
	stop(base);	// send stop condition
	return 1;	// fail
}

int i2c_bitbang_read_byte (struct i2c *base, u8 devaddr, u8 regoffset, u8 * value)
{
//	printf("i2c-bb read dev=%02x regoffset=%02x ", devaddr, regoffset);
	start(base);
	if (write_byte(base, (devaddr<<1))) { // send chip address for write command
		if (write_byte(base, regoffset)) { // send register offset - this defines where we want to read
			start(base);	// repeated start
			if (write_byte(base, (devaddr<<1) | 1)) { // send chip address for read command
				*value = read_byte(base);
//				printf("%02x\n", *value);
				write_bit(base, 1);	// NAK
				stop(base);	// send stop condition
				return 0;	// ok
			}
			else {
//				printf(" 2nd chip address error ");
			}
		}
		else {
//			printf(" offset error ");
		}
	}
	else {
//		printf(" chip address error ");
	}

	stop(base);	// send stop condition
	return 1;	// fail
}

void i2c_bitbang_init(struct i2c *base)
{
	writew(I2C_SYSTEST_ST_EN | I2C_SYSTEST_FREE | (3 << I2C_SYSTEST_TMODE_SHIFT), &base->systest);
	busidle(base);
//	printf("i2c-bb init base=%08x\n", base);
#if OLD
	printf("I2C_SYSTEST1 *0x4807003C = %04x\n", readw(0x4807003C));
	printf("I2C_SYSTEST2 *0x4807203C = %04x\n", readw(0x4807203C));
	printf("I2C_SYSTEST3 *0x4806003C = %04x\n", readw(0x4806003C));
	setscl(base, 0);
	delay_full_clock();
	printf("I2C_SYSTEST2 *0x4807203C = %04x\n", readw(0x4807203C));
	setscl(base, 1);
	delay_full_clock();
	printf("I2C_SYSTEST2 *0x4807203C = %04x\n", readw(0x4807203C));
#endif
}


void i2c_bitbang_close(struct i2c *base)
{
	busidle(base);	
	base->systest = 0; // get out of test mode
}

#else

/* not required */

#endif
