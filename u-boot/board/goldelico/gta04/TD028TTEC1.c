/* u-boot driver for the tpo TD028TTEC1 LCM
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
 * now is just a SPI/GPIO driver to the serial interface of the TD028TTEC1
 
 *** should all this code be moved to drivers/misc or drivers/video ?
 
 */

#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include "TD028TTEC1.h"

#ifdef CONFIG_OMAP3_BEAGLE

#ifdef CONFIG_OMAP3_GTA04

#define GPIO_CS		19
#define GPIO_SCL	12
#define GPIO_DIN	18
#define GPIO_DOUT	20

#else	/* Beagle Hybrid */

#define GPIO_CS		161
#define GPIO_SCL	162
#define GPIO_DIN	159
#define GPIO_DOUT	158

#endif

#define SPI_READ()      (omap_get_gpio_datain(GPIO_DIN))
#define SPI_CS(bit) 	(omap_set_gpio_dataout(GPIO_CS, bit))
#define SPI_SDA(bit)    (omap_set_gpio_dataout(GPIO_DOUT, bit))
#define SPI_SCL(bit)    (omap_set_gpio_dataout(GPIO_SCL, bit))

#elif !defined(CONFIG_GTA02_REVISION)	/* GTA01 */

#define GTA01_SCLK	(1 << 7) 	/* GPG7 */
#define GTA01_MOSI	(1 << 6)	/* GPG6 */
#define GTA01_MISO	(1 << 5)	/* GPG5 */
#define GTA01_CS	(1 << 3)	/* GPG3 */

#define SPI_READ        ((immr->GPGDAT & GTA01_MISO) != 0)

#define SPI_CS(bit) 	if (bit) gpio->GPGDAT |= GTA01_CS; \
			else gpio->GPGDAT &= ~GTA01_CS

#define SPI_SDA(bit)    if (bit) gpio->GPGDAT |=  GTA01_MOSI; \
			else    gpio->GPGDAT &= ~GTA01_MOSI

#define SPI_SCL(bit)    if (bit) gpio->GPGDAT |=  GTA01_SCLK; \
			else    gpio->GPGDAT &= ~GTA01_SCLK

#else /* GTA02 */

extern void smedia3362_spi_cs(int);
extern void smedia3362_spi_sda(int);
extern void smedia3362_spi_scl(int);
extern void smedia3362_lcm_reset(int);

#define SPI_READ	$not$implemented$
#define SPI_CS(b)   smedia3362_spi_cs(b)
#define SPI_SDA(b)  smedia3362_spi_sda(b)
#define SPI_SCL(b)  smedia3362_spi_scl(b)
   
#endif


/* 150uS minimum clock cycle, we have two of this plus our other
 * instructions */

#define SPI_DELAY()	udelay(150)

static int jbt_spi_xfer(int wordnum, int bitlen, u_int16_t *dout)
{
#if !defined(_BEAGLE_)
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
#endif
	u_int16_t tmpdout = 0;
	int   i, j;

//	DEBUGP("spi_xfer: dout %08X wordnum %u bitlen %d\n",
//		*(uint *)dout, wordnum, bitlen);

	SPI_CS(0);

	for (i = 0; i < wordnum; i ++) {
		tmpdout = dout[i];

		for (j = 0; j < bitlen; j++) {
			SPI_SCL(0);
			if (tmpdout & (1 << (bitlen-1))) {
				SPI_SDA(1);
//				DEBUGPC("1");
//				VERIFY(1);
			} else {
				SPI_SDA(0);
//				DEBUGPC("0");
//				VERIFY(0);
			}
			SPI_DELAY();
			SPI_SCL(1);
			SPI_DELAY();
			tmpdout <<= 1;
		}
//		DEBUGPC(" ");
	}
//	DEBUGPC("\n");

	SPI_CS(1);

	return 0;
}

#define JBT_COMMAND	0x000
#define JBT_DATA	0x100

int jbt_reg_write_nodata(struct jbt_info *jbt, u_int8_t reg)
{
	int rc;
	
	jbt->tx_buf[0] = JBT_COMMAND | reg;
	
	rc = jbt_spi_xfer(1, 9, jbt->tx_buf);
	
	return rc;
}


int jbt_reg_write(struct jbt_info *jbt, u_int8_t reg, u_int8_t data)
{
	int rc;
	
	jbt->tx_buf[0] = JBT_COMMAND | reg;
	jbt->tx_buf[1] = JBT_DATA | data;
	
	rc = jbt_spi_xfer(2, 9, jbt->tx_buf);
	
	return rc;
}

int jbt_reg_write16(struct jbt_info *jbt, u_int8_t reg, u_int16_t data)
{
	int rc;
	
	jbt->tx_buf[0] = JBT_COMMAND | reg;
	jbt->tx_buf[1] = JBT_DATA | (data >> 8);
	jbt->tx_buf[2] = JBT_DATA | (data & 0xff);
	
	rc = jbt_spi_xfer(3, 9, jbt->tx_buf);
	
	return rc;
}

int jbt_check(void)
{ // check if we have connectivity
#if defined(_BEAGLE_)
	int err;
	int i;
	int failed=0;
	int cnt0 = 0;
	int cnt1 = 0;

#if 0
	printf("jbt_reg_init()\n");
#endif
	err = omap_request_gpio(GPIO_CS);
	SPI_CS(1);	// unselect
	err |= omap_request_gpio(GPIO_SCL);
	SPI_SCL(1);	// default
	err |= omap_request_gpio(GPIO_DOUT);
	SPI_SDA(0);
	err |= omap_request_gpio(GPIO_DIN);
	if(err)
		{
		printf("jbt_reg_init() - could not get GPIOs\n");
		return 1;
		}
#if 1		// should have already been done by MUX settings!
	omap_set_gpio_direction(GPIO_CS, 0);	// output
	omap_set_gpio_direction(GPIO_SCL, 0);	// output
	omap_set_gpio_direction(GPIO_DOUT, 0);	// output
	omap_set_gpio_direction(GPIO_DIN, 1);	// input (for reading back)
#endif
	
	//	omap_free_gpio(GPIO_DIN);
	//	omap_free_gpio(GPIO_DOUT);
	//	omap_free_gpio(GPIO_CS);
	//	omap_free_gpio(GPIO_SCL);
	
	for(i=0; i<16; i++)
		{ // check for connection between GPIO158 -> GPIO159; since we have 10 kOhm pse. make sure that the PUP/PDN is disabled on DIN in the MUX config!
			int bit=i&1;
			SPI_SDA(bit);	// write bit
			SPI_DELAY();
#if 0
			printf("bit: %d out: %d in: %d (%d)\n", bit, omap_get_gpio_datain(GPIO_DOUT), omap_get_gpio_datain(GPIO_DIN), SPI_READ());
#endif
			if(SPI_READ() != bit)	// did not read back
				failed++;
			if(SPI_READ())
				cnt1++;
			else
				cnt0++;
		}	
	if(failed > 0)
		{
		printf("DISPLAY:    ");
		if(cnt0 == 0)
			printf("DIN (GPIO%d) stuck at 0\n", GPIO_DIN);
		else if(cnt1 == 0)
			printf("DIN (GPIO%d) stuck at 1\n", GPIO_DIN);
		else 
			printf("DIN-DOUT (GPIO%d- (GPIO%d)) connetion broken\n", GPIO_DIN, GPIO_DOUT);
		return 1;
		}
#endif
	return 0;
}

int jbt_reg_init(void)
{
	
	if(jbt_check())
		return 1;	// some error
	/* according to data sheet: wait 50ms (Tpos of LCM). However, 50ms
	 * seems unreliable with later LCM batches, increasing to 90ms */
	udelay(90000);
	printf("did jbt_reg_init()\n");
	return 0;
}


