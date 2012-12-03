/* u-boot driver for the GTA04 shutdown
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
#include <twl4030.h>
#include <ns16550.h>
#include "systest.h"
#include "TD028TTEC1.h"
#include "panel.h"
#include "ulpi-phy.h"
#include "status.h"
#include "tsc2007.h"
#include "twl4030-additions.h"

#define TWL4030_I2C_BUS			(1-1)
#define SENSORS_I2C_BUS			(2-1)

int bt_hci(int msg)
{
#ifdef CONFIG_SYS_NS16550_COM1
#define MODE_X_DIV 16
	int baudrate=9600;
	int divisor=(CONFIG_SYS_NS16550_CLK + (baudrate * (MODE_X_DIV / 2))) / (MODE_X_DIV * baudrate);
	char send[]={
		0x01,	// command
#define OGF 0x01
#define OCF 0x0002	// HCI_Inquiry_Cancel
#define OP ((OGF<<10)+OCF)
		OP&0xff, OP>>8
	};
	NS16550_reinit((NS16550_t)CONFIG_SYS_NS16550_COM1, divisor);	// initialize UART
	int i;
	int bytes=0;
	long timer=(1*1000*1000)/40;	// 1 second total timeout
#if 1 // GPIO test for UART lines
	if(msg)
		{ // print states of UART lines
			MUX_VAL(CP(UART1_TX),		(IEN  | PTU | EN | M4)) /*UART1_TX -> Bluetooth HCI */\
			MUX_VAL(CP(UART1_RTS),		(IEN  | PTU | EN | M4)) /*UART1_RTS -> Bluetooth HCI */ \
			MUX_VAL(CP(UART1_CTS),		(IEN  | PTU | EN | M4)) /*UART1_CTS -> Bluetooth HCI */ \
			MUX_VAL(CP(UART1_RX),		(IEN  | PTU | EN | M4)) /*UART1_RX -> Bluetooth HCI */

			// FIXME: this is not 100% correct for BeagleBoard
			MUX_VAL(CP(UART2_CTS),		(IEN  | PTU | DIS | M4)) /*GPIO_144 - ext Ant */\
			MUX_VAL(CP(UART2_RTS),		(IEN  | PTD | DIS | M4)) /*GPIO_145 - GPS ON(0)/OFF(1)*/\
			MUX_VAL(CP(UART2_TX),		(IEN  | PTU | DIS | M4)) /*GPIO_146 - GPS_TX */\
			MUX_VAL(CP(UART2_RX),		(IEN  | PTU | DIS | M4)) /*GPIO_147 - GPS_RX */\
			
			udelay(100);
			
			omap_request_gpio(148);
			omap_request_gpio(149);
			omap_request_gpio(150);
			omap_request_gpio(151);
			omap_set_gpio_direction(148, 1);	// 1=input
			omap_set_gpio_direction(149, 1);	// 1=input
			omap_set_gpio_direction(150, 1);	// 1=input
			omap_set_gpio_direction(151, 1);	// 1=input
			printf("UART1 RTS: %d\n", omap_get_gpio_datain(149));
			printf("UART1 CTS: %d\n", omap_get_gpio_datain(150));
			printf("UART1 TX:  %d\n", omap_get_gpio_datain(148));
			printf("UART1 RX:  %d\n", omap_get_gpio_datain(151));
			
			omap_request_gpio(144);
			omap_request_gpio(145);
			omap_request_gpio(146);
			omap_request_gpio(147);
			omap_set_gpio_direction(144, 1);	// 1=input
			omap_set_gpio_direction(145, 1);	// 1=input
			omap_set_gpio_direction(146, 1);	// 1=input
			omap_set_gpio_direction(147, 1);	// 1=input
			printf("UART2 RTS: %d\n", omap_get_gpio_datain(145));
			printf("UART2 CTS: %d\n", omap_get_gpio_datain(144));
			printf("UART2 TX:  %d\n", omap_get_gpio_datain(146));
			printf("UART2 RX:  %d\n", omap_get_gpio_datain(147));
			
			MUX_VAL(CP(UART1_TX),		(IDIS | PTD | DIS | M0)) /*UART1_TX -> Bluetooth HCI */\
			MUX_VAL(CP(UART1_RTS),		(IDIS | PTD | DIS | M0)) /*UART1_RTS -> Bluetooth HCI */ \
			MUX_VAL(CP(UART1_CTS),		(IEN  | PTD | DIS | M0)) /*UART1_CTS -> Bluetooth HCI */ \
			MUX_VAL(CP(UART1_RX),		(IEN  | PTD | DIS | M0)) /*UART1_RX -> Bluetooth HCI */\

			MUX_VAL(CP(UART2_CTS),		(IEN  | PTU | DIS | M4)) /*GPIO_144 - ext Ant */\
			MUX_VAL(CP(UART2_RTS),		(IDIS | PTD | DIS | M4)) /*GPIO_145 - GPS ON(0)/OFF(1)*/\
			MUX_VAL(CP(UART2_TX),		(IDIS | PTU | DIS | M0)) /*GPIO_146 - GPS_TX */\
			MUX_VAL(CP(UART2_RX),		(IEN  | PTU | DIS | M0)) /*GPIO_147 - GPS_RX */\
			udelay(100);
		}
#endif
	if(msg)
		printf("HCI send:");
	for (i=0; i<sizeof(send); i++)
		{ // send bytes on HCI port
			if(msg)
				printf(" %02x", send[i]);
			NS16550_putc((NS16550_t)CONFIG_SYS_NS16550_COM1, send[i]);
		}
	if(msg)
		printf("\n");
	if(msg)
		printf("HCI receive:");
	while (timer-- > 0)
		{ //
			if(NS16550_tstc((NS16550_t)CONFIG_SYS_NS16550_COM1))
				{ // byte received
					int c=NS16550_getc((NS16550_t)CONFIG_SYS_NS16550_COM1);
					if(msg)
						printf(" %02x", c);	// from GPS to console
					bytes++;	// one more received
				}
			udelay(40);	// 115200 bit/s is approx. 86 us per byte 
		}
	if(bytes == 0)
		{
		if(msg)
			printf(" timed out\n");
		return 0;			
		}
	if(msg)
		printf("\n");
#endif
	return 1;
}

int systest(void)
{
	int r;
	i2c_set_bus_num(TWL4030_I2C_BUS);	// I2C1
	printf("TPS65950:      %s\n", !(r=i2c_probe(TWL4030_CHIP_USB))?"found":"-");	// responds on 4 addresses 0x48..0x4b
	if(!r)
		{ // was ok, ask for details
		u8 val;
		u8 val2;
		twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER, &val, TWL4030_PM_MASTER_STS_HW_CONDITIONS);
		printf("  STS_HW_CON: %02x", val);	// decode bits
		if(val & 0x80) printf(" VBUS");
		if(val & 0x08) printf(" NRESWARM");
		if(val & 0x04) printf(" USB");
		if(val & 0x02) printf(" CHG");
		if(val & 0x01) printf(" PWRON");
		printf("\n");
		twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER, &val, 0x2e);
		printf("  PWR_ISR1:   %02x", val);
		// decode bits
		printf("\n");
		twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER, &val, TWL4030_PM_MASTER_SC_DETECT1);
		printf("  SC_DETECT:  1:%02x", val);
		twl4030_i2c_read_u8(TWL4030_CHIP_PM_RECEIVER, &val, TWL4030_PM_MASTER_SC_DETECT1);
		printf(" 2:%02x\n", val);
		twl4030_i2c_read_u8(TWL4030_CHIP_MAIN_CHARGE, &val, 0x82);
		printf("  BCIMFSTS2:  %02x\n", val);
		twl4030_i2c_read_u8(TWL4030_CHIP_MAIN_CHARGE, &val, 0x83);
		printf("  BCIMFSTS3:  %02x\n", val);
		twl4030_i2c_read_u8(TWL4030_CHIP_MAIN_CHARGE, &val, 0x84);
		printf("  BCIMFSTS4:  %02x\n", val);
		twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val, 0x57);
		twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val2, 0x58);
		printf("  BTEMP:   %d\n", (val2<<2)+(val>>6));
		twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val, 0x59);
		twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val2, 0x5a);
		printf("  USBVBUS: %d\n", (val2<<2)+(val>>6));
		twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val, 0x5b);
		twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val2, 0x5c);
		printf("  ICHG:    %d\n", (val2<<2)+(val>>6));
		twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val, 0x5d);
		twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val2, 0x5e);
		printf("  VCHG:    %d\n", (val2<<2)+(val>>6));
		twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val, 0x5f);
		twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val2, 0x60);
		printf("  VBAT:    %d\n", (val2<<2)+(val>>6));
		}
	i2c_set_bus_num(SENSORS_I2C_BUS);	// I2C2
	printf("TSC2007:       %s\n", !i2c_probe(0x48)?"found":"-");
	printf("TCA6507:       %s\n", !i2c_probe(0x45)?"found":"-");
	printf("LIS302 TOP:    %s\n", !i2c_probe(0x1c)?"found":"-");
	printf("LIS302 BOTTOM: %s\n", !i2c_probe(0x1d)?"found":"-");
	printf("LSM303:        %s\n", !i2c_probe(0x19)?"found":"-");
	printf("HMC58xx:       %s\n", !i2c_probe(0x1e)?"found":"-");
	printf("BMP085:        %s\n", !i2c_probe(0x77)?"found":"-");
	printf("BMA180:        %s\n", !i2c_probe(0x41)?"found":"-");
	printf("BMC050 Accel:  %s\n", !i2c_probe(0x18)?"found":"-");
	printf("BMC050 Mag:    %s\n", !i2c_probe(0x10)?"found":"-");
	printf("ITG3200:       %s\n", !i2c_probe(0x68)?"found":"-");
	printf("Si47xx:        %s\n", !i2c_probe(0x11)?"found":"-");
	printf("TCA8418:       %s\n", !i2c_probe(0x34)?"found":"-");
	printf("OV9655:        %s\n", !i2c_probe(0x30)?"found":"-");
	printf("TPS61050:      %s\n", !i2c_probe(0x33)?"found":"-");
	printf("EEPROM:        %s\n", !i2c_probe(0x50)?"found":"-");
	printf("VCNL4000:      %s\n", !i2c_probe(0x13)?"found":"-");
	i2c_set_bus_num(2);	// I2C3
	/* nothing to check */
	i2c_set_bus_num(TWL4030_I2C_BUS);	// I2C1
	if(!panel_check())
	    printf("DISPLAY:       ok\n");
	else
		printf("DISPLAY:       failed\n");
#if HAVE_ULPI_TEST
	printf("PHY-WWAN:      %s\n", (ulpi_direct_access(1, 1, 0, 0) == 0x04)?"found":"-");	// read reg 1 (vendor id) of HSUSB2
#endif
	// LEDs
	// GPS/UART
	// RAM-Test
	// NAND-Test
	// Buttons
	// Power
	// real Display communication
	// PCM
	// OTG USB
	// Charger
	// internal USB
	// UMTS Module
	printf("BT HCI:        %s\n", bt_hci(0)?"found":"-");
	return (0);
}

/* audio test taken from BeagleBoard B validation U-Boot (1.3.3) */
/* http://code.google.com/p/beagleboard/wiki/BeagleSourceCode */

static ushort tone[] = {
	0x0ce4, 0x0ce4, 0x1985, 0x1985, 0x25A1, 0x25A1, 0x30FD, 0x30FE,
	0x3B56, 0x3B55, 0x447A, 0x447A, 0x4C3B, 0x4C3C, 0x526D, 0x526C,
	0x56F1, 0x56F1, 0x59B1, 0x59B1, 0x5A9E, 0x5A9D, 0x59B1, 0x59B2,
	0x56F3, 0x56F2, 0x526D, 0x526D, 0x4C3B, 0x4C3B, 0x447C, 0x447C,
	0x3B5A, 0x3B59, 0x30FE, 0x30FE, 0x25A5, 0x25A6, 0x1989, 0x198A,
	0x0CE5, 0x0CE3, 0x0000, 0x0000, 0xF31C, 0xF31C, 0xE677, 0xE676,
	0xDA5B, 0xDA5B, 0xCF03, 0xCF03, 0xC4AA, 0xC4AA, 0xBB83, 0xBB83,
	0xB3C5, 0xB3C5, 0xAD94, 0xAD94, 0xA90D, 0xA90E, 0xA64F, 0xA64E,
	0xA562, 0xA563, 0xA64F, 0xA64F, 0xA910, 0xA90F, 0xAD93, 0xAD94,
	0xB3C4, 0xB3C4, 0xBB87, 0xBB86, 0xC4AB, 0xC4AB, 0xCF03, 0xCF03,
	0xDA5B, 0xDA5A, 0xE67B, 0xE67B, 0xF31B, 0xF3AC, 0x0000, 0x0000,
	0x0CE4, 0x0CE4, 0x1985, 0x1985, 0x25A1, 0x25A1, 0x30FD, 0x30FE,
	0x3B56, 0x3B55, 0x447A, 0x447A, 0x4C3B, 0x4C3C, 0x526D, 0x526C,
	0x56F1, 0x56F1, 0x59B1, 0x59B1, 0x5A9E, 0x5A9D, 0x59B1, 0x59B2,
	0x56F3, 0x56F2, 0x526D, 0x526D, 0x4C3B, 0x4C3B, 0x447C, 0x447C,
	0x3B5A, 0x3B59, 0x30FE, 0x30FE, 0x25A5, 0x25A6, 0x1989, 0x198A,
	0x0CE5, 0x0CE3, 0x0000, 0x0000, 0xF31C, 0xF31C, 0xE677, 0xE676,
	0xDA5B, 0xDA5B, 0xCF03, 0xCF03, 0xC4AA, 0xC4AA, 0xBB83, 0xBB83,
	0xB3C5, 0xB3C5, 0xAD94, 0xAD94, 0xA90D, 0xA90E, 0xA64F, 0xA64E,
	0xA562, 0xA563, 0xA64F, 0xA64F, 0xA910, 0xA90F, 0xAD93, 0xAD94,
	0xB3C4, 0xB3C4, 0xBB87, 0xBB86, 0xC4AB, 0xC4AB, 0xCF03, 0xCF03,
	0xDA5B, 0xDA5A, 0xE67B, 0xE67B, 0xF31B, 0xF3AC, 0x0000, 0x0000,
	0x0CE4, 0x0CE4, 0x1985, 0x1985, 0x25A1, 0x25A1, 0x30FD, 0x30FE,
	0x3B56, 0x3B55, 0x447A, 0x447A, 0x4C3B, 0x4C3C, 0x526D, 0x526C,
	0x56F1, 0x56F1, 0x59B1, 0x59B1, 0x5A9E, 0x5A9D, 0x59B1, 0x59B2,
	0x56F3, 0x56F2, 0x526D, 0x526D, 0x4C3B, 0x4C3B, 0x447C, 0x447C,
	0x3B5A, 0x3B59, 0x30FE, 0x30FE, 0x25A5, 0x25A6, 0x1989, 0x198A,
	0x0CE5, 0x0CE3, 0x0000, 0x0000, 0xF31C, 0xF31C, 0xE677, 0xE676,
	0xDA5B, 0xDA5B, 0xCF03, 0xCF03, 0xC4AA, 0xC4AA, 0xBB83, 0xBB83,
	0xB3C5, 0xB3C5, 0xAD94, 0xAD94, 0xA90D, 0xA90E, 0xA64F, 0xA64E,
	0xA562, 0xA563, 0xA64F, 0xA64F, 0xA910, 0xA90F, 0xAD93, 0xAD94,
	0xB3C4, 0xB3C4, 0xBB87, 0xBB86, 0xC4AB, 0xC4AB, 0xCF03, 0xCF03,
	0xDA5B, 0xDA5A, 0xE67B, 0xE67B, 0xF31B, 0xF3AC, 0x0000, 0x0000
};

int audiotest_init(int channel)
{
	
	unsigned char byte;
	
	printf("Test Audio Tone on Speaker\n");
	printf("Initializing over I2C1");
	i2c_set_bus_num(TWL4030_I2C_BUS);	// I2C1
	
	// program Audio controller (see document SWCU050D)
	
	// ??? VAUX3 is not connected on BB ???
	// so what is the setting for 2.8V good for?
/*	
	byte = 0x20;						// RES_TYPE2=2, RES_TYPE=0
	i2c_write(0x4B, 0x7A, 1, &byte, 1);	// VAUX3_DEV_GRP
	byte = 0x03;						// no trim, 2.8V
	i2c_write(0x4B, 0x7D, 1, &byte, 1);	// VAUX3_DEDICATED

	
	byte = 0xE0;						// DEV_GRP belongs to all device groups
	i2c_write(0x4B, 0x8E, 1, &byte, 1);	// VPLL2_DEV_GRP
	byte = 0x05;						// 1.8V
	i2c_write(0x4B, 0x91, 1, &byte, 1);	// VPLL2_DEDICATED
 */
	
	byte = 0x03;						// 8 kHz, Codec on, Option 1:RX and TX stereo audio path
	i2c_write(0x49, 0x01, 1, &byte, 1);	// CODEC_MODE
	byte = 0xc0;						// Audio RX Right2 enable, Left 2 enable
	i2c_write(0x49, 0x02, 1, &byte, 1);	// OPTION
/*
	byte = 0x00;
	i2c_write(0x49, 0x03, 1, &byte, 1);	// ?
 */
	byte = 0x00;
	i2c_write(0x49, 0x04, 1, &byte, 1);	// MICBIAS_CTL
	byte = 0x00;
	i2c_write(0x49, 0x05, 1, &byte, 1);	// ANAMICL
	byte = 0x00;
	i2c_write(0x49, 0x06, 1, &byte, 1);	// ANAMICR
	byte = 0x00;
	i2c_write(0x49, 0x07, 1, &byte, 1);	// AVADC_CTL
	byte = 0x00;
	i2c_write(0x49, 0x08, 1, &byte, 1);	// ADCMICSEL
	byte = 0x00;
	i2c_write(0x49, 0x09, 1, &byte, 1);	// DIGMIXING
	byte = 0x00;
	i2c_write(0x49, 0x0a, 1, &byte, 1);	// ATXL1PGA
	byte = 0x00;
	i2c_write(0x49, 0x0b, 1, &byte, 1);	// ATXR1PGA
	byte = 0x00;
	i2c_write(0x49, 0x0c, 1, &byte, 1);	// AVTXL2PGA
	byte = 0x00;
	i2c_write(0x49, 0x0d, 1, &byte, 1);	// AVTXR2PGA
	byte = 0x01;						// TDM/CODEC master mode, 16 bit sample/word, codec mode format, clk256 disabled, Application mode
	i2c_write(0x49, 0x0e, 1, &byte, 1);	// AUDIO_IF
	byte = 0x00;
	i2c_write(0x49, 0x0f, 1, &byte, 1);	// VOICE_IF
	byte = 0x00;
	i2c_write(0x49, 0x10, 1, &byte, 1);	// ARXR1PGA
	byte = 0x00;
	i2c_write(0x49, 0x11, 1, &byte, 1);	// ARXL1PGA
	byte = 0x6c;						// ARXR2PGA_CGAIN=6 dB, ARXR2PGA_FGAIN=-31 dB
	i2c_write(0x49, 0x12, 1, &byte, 1);	// ARXR2PGA
	byte = 0x6c;						// ARXL2PGA_CGAIN=6 dB, ARXR2PGA_FGAIN=-31 dB
	i2c_write(0x49, 0x13, 1, &byte, 1);	// ARXL2PGA
	byte = 0x00;
	i2c_write(0x49, 0x14, 1, &byte, 1);	// VRXPGA
	byte = 0x00;
	i2c_write(0x49, 0x15, 1, &byte, 1);	// VSTPGA
	byte = 0x00;
	i2c_write(0x49, 0x16, 1, &byte, 1);	// VRX2ARXPGA
	byte = 0x0c;						// VDAC_EN off, ADACL2_EN & ADACR2_EN on, ADACL1_EN & ADACR1_EN off
	i2c_write(0x49, 0x17, 1, &byte, 1);	// AVDAC_CTL
	byte = 0x00;
	i2c_write(0x49, 0x18, 1, &byte, 1);	// ARX2VTXPGA
	byte = 0x00;
	i2c_write(0x49, 0x19, 1, &byte, 1);	// ARXL1_APGA_CTL
	byte = 0x00;
	i2c_write(0x49, 0x1a, 1, &byte, 1);	// ARXR1_APGA_CTL
	byte = 0x2b;						// 2 dB, no FM loop, Digital-Analog path enable, Analog PGA application mode
	i2c_write(0x49, 0x1b, 1, &byte, 1);	// ARXL2_APGA_CTL
	byte = 0x2b;						// 2 dB, no FM loop, Digital-Analog path enable, Analog PGA application mode
	i2c_write(0x49, 0x1c, 1, &byte, 1);	// ARXR2_APGA_CTL
	byte = 0x00;
	i2c_write(0x49, 0x1d, 1, &byte, 1);	// ATX2ARXPGA
	byte = 0x00;
	i2c_write(0x49, 0x1e, 1, &byte, 1);	// BT_IF
	byte = 0x00;
	i2c_write(0x49, 0x1f, 1, &byte, 1);	// BTPGA
	byte = 0x00;
	i2c_write(0x49, 0x20, 1, &byte, 1);	// BTSTPGA
	byte = 0x34;						// 0 dB, EAR_AL2_EN
	i2c_write(0x49, 0x21, 1, &byte, 1);	// EAR_CTL
	byte = 0x24;						// HSOR_AR2_EN, HSOL_AL2_EN
	i2c_write(0x49, 0x22, 1, &byte, 1);	// HS_SEL
	byte = 0x0a;						// HSR_GAIN & HSL_GAIN = 0 dB
	i2c_write(0x49, 0x23, 1, &byte, 1);	// HS_GAIN_SET
	byte = 0x42;						// VMID_EN enable, ramp down 20ms
	i2c_write(0x49, 0x24, 1, &byte, 1);	// HS_POPN_SET
	byte = 0x00;
	i2c_write(0x49, 0x25, 1, &byte, 1);	// PREDL_CTL
	byte = 0x00;
	i2c_write(0x49, 0x26, 1, &byte, 1);	// PREDR_CTL
	byte = 0x00;
	i2c_write(0x49, 0x27, 1, &byte, 1);	// PRECKL_CTL
	byte = 0x00;
	i2c_write(0x49, 0x28, 1, &byte, 1);	// PRECKR_CTL
	byte = 0x00;
	i2c_write(0x49, 0x29, 1, &byte, 1);	// HFL_CTL
	byte = 0x00;
	i2c_write(0x49, 0x2a, 1, &byte, 1);	// HFR_CTL
	byte = 0x00;
	i2c_write(0x49, 0x2b, 1, &byte, 1);	// ALC_CTL
	byte = 0x00;
	i2c_write(0x49, 0x2c, 1, &byte, 1);	// ALC_SET1
	byte = 0x00;
	i2c_write(0x49, 0x2d, 1, &byte, 1);	// ALC_SET2
	byte = 0x00;
	i2c_write(0x49, 0x2e, 1, &byte, 1);	// BOOST_CTL
	byte = 0x00;
	i2c_write(0x49, 0x2f, 1, &byte, 1);	// SOFTVOL_CTL
	byte = 0x00;
	i2c_write(0x49, 0x30, 1, &byte, 1);	// DTMF_FREQSEL
	byte = 0x00;
	i2c_write(0x49, 0x31, 1, &byte, 1);	// DTMF_TONEXT1H
	byte = 0x00;
	i2c_write(0x49, 0x32, 1, &byte, 1);	// DTMF_TONEXT1L
	byte = 0x00;
	i2c_write(0x49, 0x33, 1, &byte, 1);	// DTMF_TONEXT2H
	byte = 0x00;
	i2c_write(0x49, 0x34, 1, &byte, 1);	// DTMF_TONEXT2L
	byte = 0x00;
	i2c_write(0x49, 0x35, 1, &byte, 1);	// DTMF_TONOFF
	byte = 0x00;
	i2c_write(0x49, 0x36, 1, &byte, 1);	// DTMF_WANONOFF
	byte = 0x00;
	i2c_write(0x49, 0x37, 1, &byte, 1);	// I2S_RX_SCRAMBLE_H
	byte = 0x00;
	i2c_write(0x49, 0x38, 1, &byte, 1);	// I2S_RX_SCRAMBLE_M
	byte = 0x00;
	i2c_write(0x49, 0x39, 1, &byte, 1);	// I2S_RX_SCRAMBLE_L
//	byte = 0x15;						// APLL_EN enabled, 19.2 MHz
	byte = 0x16;						// APLL_EN enabled, 26 MHz
	i2c_write(0x49, 0x3a, 1, &byte, 1);	// APLL_CTL
	byte = 0x00;
	i2c_write(0x49, 0x3b, 1, &byte, 1);	// DTMF_CTL
	byte = 0x00;
	i2c_write(0x49, 0x3c, 1, &byte, 1);	// DTMF_PGA_CTL2
	byte = 0x00;
	i2c_write(0x49, 0x3d, 1, &byte, 1);	// DTMF_PGA_CTL1
	byte = 0x00;
	i2c_write(0x49, 0x3e, 1, &byte, 1);	// MISC_SET_1
	byte = 0x00;
	i2c_write(0x49, 0x3f, 1, &byte, 1);	// PCMBTMUX
/*
	byte = 0x00;
	i2c_write(0x49, 0x40, 1, &byte, 1);	// ?
	byte = 0x00;
	i2c_write(0x49, 0x41, 1, &byte, 1);	// ?
	byte = 0x00;
	i2c_write(0x49, 0x42, 1, &byte, 1);	// ?
	byte = 0x00;
	i2c_write(0x49, 0x43, 1, &byte, 1);	// ?
 */
	byte = 0x00;
	i2c_write(0x49, 0x44, 1, &byte, 1);	// RX_PATH_SEL
	byte = 0x00;
	i2c_write(0x49, 0x45, 1, &byte, 1);	// VDL_APGA_CTL
	byte = 0x00;
	i2c_write(0x49, 0x46, 1, &byte, 1);	// VIBRA_CTL
	byte = 0x00;
	i2c_write(0x49, 0x47, 1, &byte, 1);	// VIBRA_SET
	byte = 0x00;
	i2c_write(0x49, 0x48, 1, &byte, 1);	// ANAMIC_GAIN
	byte = 0x00;
	i2c_write(0x49, 0x49, 1, &byte, 1);	// MISC_SET_2

	// check for errors...
	
	// initialize McBSP2 and fill with data ???
	
	*((uint *) 0x4902208c) = 0x00000208;	// MCBSPLP_SYSCONFIG_REG
	*((uint *) 0x49022090) = 0x00000000;	// MCBSPLP_THRSH2_REG
	*((uint *) 0x49022094) = 0x00000000;	// MCBSPLP_THRSH1_REG
	*((uint *) 0x490220ac) = 0x00001008;	// MCBSPLP_XCCR_REG
	*((uint *) 0x490220b0) = 0x00000808;	// MCBSPLP_RCCR_REG
	*((uint *) 0x49022018) = 0x00000000;	// MCBSPLP_RCR2_REG
	*((uint *) 0x4902201c) = 0x00000000;	// MCBSPLP_RCR1_REG
	*((uint *) 0x49022020) = 0x00000000;	// MCBSPLP_XCR2_REG
	*((uint *) 0x49022024) = 0x00000000;	// MCBSPLP_XCR1_REG
	*((uint *) 0x49022028) = 0x00000000;	// MCBSPLP_SRGR2_REG
	*((uint *) 0x4902202c) = 0x00000000;	// MCBSPLP_SRGR1_REG
	*((uint *) 0x49022048) = 0x00000083;	// MCBSPLP_PCR_REG	/ SCLKME, CLKXP, CLKRP
	*((uint *) 0x49022010) = 0x00000200;	// MCBSPLP_SPCR2_REG / FREE, !XRST
	*((uint *) 0x49022014) = 0x00000000;	// MCBSPLP_SPCR1_REG / !RRST
//	*((uint *) 0x4902207c) = 0x00000023;	// MCBSPLP_REV_REG (is read only???)
	*((uint *) 0x49022010) = 0x00000201;	// MCBSPLP_SPCR2_REG / FREE, XRST
	*((uint *) 0x49022008) = 0x000056f3;	// MCBSPLP_DXR_REG - write first byte
	printf("  ... complete\n");
	return 0;

}

int audiotest_send(void)
{
	int count = 0;
	printf("Sending data");

	for (count = 0; count < 50; count++) {
		int bytes;
		for (bytes = 0; bytes < sizeof(tone) / 2; bytes++) {
			*((uint *) 0x49022008) = tone[bytes];	// MCBSPLP_DXR_REG
			udelay(100);
		}
	}
	printf("  ... complete\n");

	return 0;
}

int audiotest(int channel)
{
	return audiotest_init(channel) || audiotest_send();
}

#define RS232_ENABLE	13
#define RS232_RX		165	// UART3
#define RS232_TX		166	// UART3

int irdatest(void)
{
#ifdef CONFIG_OMAP3_GTA04
	printf("Stop through touch screen or AUX button - RS232 is disabled\n");
	udelay(50000);	// wait until RS232 is finished
	// switch RS232_ENABLE to IrDA through Pull-Down and RXTX to GPIO mode
	MUX_VAL(CP(UART3_RX_IRRX),	(IEN  | PTD | DIS | M4)); /*UART3_RX_IRRX*/
	MUX_VAL(CP(UART3_TX_IRTX),	(IDIS | PTD | DIS | M4)); /*UART3_TX_IRTX*/
	omap_request_gpio(RS232_RX);
	omap_set_gpio_direction(RS232_RX, 1);	// 1=input
	omap_request_gpio(RS232_TX);
	omap_set_gpio_direction(RS232_TX, 0);
	omap_set_gpio_dataout(RS232_TX, 0);	// set TX=low to select SIR mode
	udelay(10);
	MUX_VAL(CP(ETK_CTL_ES2),	(IEN  | PTD | EN  | M4)); /*GPIO_13 - RS232 enable*/
	udelay(1000);
	while(!pendown(NULL, NULL) && (status_get_buttons()&0x09) == 0)
		{
		// make IrDA blink
		omap_set_gpio_dataout(RS232_TX, 1);
		udelay(150);
		omap_set_gpio_dataout(RS232_TX, 0);
		udelay(150);
		if(!omap_get_gpio_datain(RS232_RX))	// RX is active low
			status_set_status(0x018);	// echo IrDA sensor status to red power button led
		else
			status_set_status(0x000);
		}
	// switch back to UART mode
	MUX_VAL(CP(UART3_RX_IRRX),	(IEN  | PTD | DIS | M0)); /*UART3_RX_IRRX*/
	MUX_VAL(CP(UART3_TX_IRTX),	(IDIS | PTD | DIS | M0)); /*UART3_TX_IRTX*/
	MUX_VAL(CP(ETK_CTL_ES2),	(IEN  | PTU | EN  | M4)); /*GPIO_13 - RS232 enable*/
	udelay(1000);
	printf("done.\n");
	return 0;
#else
	printf("n/a\n");
	return 1;
#endif
}

#ifdef CONFIG_OMAP3_GTA04
int wlanbtpower(void)
{ /* Bluetooth VAUX4 = 3.3V -- CHECKME: 3.3 V is not officially supported by TPS! We use 0x09 = 2.8V here*/
#define TCA6507_BUS			(2-1)	// I2C2
#define TCA6507_ADDRESS		0x45
	
	/* register numbers */
#define TCA6507_SELECT0						0
#define TCA6507_SELECT1						1
#define TCA6507_SELECT2						2
	
	printf("activate WiFi/BT reset\n");
	i2c_set_bus_num(TCA6507_BUS);	// write I2C2
	i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT0, 0);
	i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT1, 0);
	i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT2, 0x40);	// pull down reset for WLAN&BT chip
	i2c_set_bus_num(TWL4030_I2C_BUS);	// write I2C1
	printf("power on VAUX4\n");
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX4_DEDICATED,
							/* 2.8V: */ 0x09 /* 3.15V: 0x0c */,
							TWL4030_PM_RECEIVER_VAUX4_DEV_GRP,
							TWL4030_PM_RECEIVER_DEV_GRP_P1);
	udelay(20*1000);	// wait a little until power stabilizes
	printf("release WiFi/BT reset\n");
	i2c_set_bus_num(TCA6507_BUS);	// write I2C2
	i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT0, 0);
	i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT1, 0);
	i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT2, 0);	// release reset for WLAN&BT chip
	i2c_set_bus_num(TWL4030_I2C_BUS);	// write I2C1
	return 0;
}
#endif

int wlanbttest(int test)
{ /* Bluetooth VAUX4 = 3.3V -- CHECKME: 3.3 V is not officially supported! We use 0x09 = 2.8V here*/
#ifdef CONFIG_OMAP3_GTA04
	wlanbtpower();
	if(test)
		{
		// now, we should be able to test the UART for the BT part...
		return !bt_hci(1);
		}
	return 0;
#else
	printf("n/a\n");
	return 1;
#endif
}

int OTGchargepump(int enable)
{
	i2c_set_bus_num(TWL4030_I2C_BUS);	// I2C1
	if(enable)
		twl4030_i2c_write_u8(TWL4030_CHIP_USB, 0x20, TWL4030_USB_OTG_CTRL_SET);
	else
		twl4030_i2c_write_u8(TWL4030_CHIP_USB, 0x20, TWL4030_USB_OTG_CTRL_CLR);
	return 0;
}

// NOTE: this table is only for GTA04

static struct
{
	int gpio;
	int inputonly;		// can't switch to output mode (Table 2-24. General-Purpose IOs Signals Description)
	char *name;
	unsigned offset;	// PIN-MUX control address -> writew((VALUE), OMAP34XX_CTRL_BASE + (OFFSET));
} gpiotable[] = {
	{ 7, 0, "AUX button", CP(SYS_BOOT5) },
	
	{ 10, 0, "A3: KEYIRQ / A4: 3G_WAKE", CP(SYS_CLKOUT1) },
	
	{ 12, 0, "Display SCL", CP(ETK_CLK_ES2) },
	
	// CHECKME: doesn't this interfere with RS232 operation?
	//	{ 13, 1, "RS232 enable", CP(ETK_CTL_ES2) }, 
	
	{ 18, 0, "Display DR", CP(ETK_D4_ES2) },
	{ 19, 0, "Display XCS", CP(ETK_D5_ES2) },
	{ 20, 0, "Display DX", CP(ETK_D6_ES2) },
	{ 21, 0, "RS232 EXT", CP(ETK_D7_ES2) },
	
	{ 23, 0, "Video out en", CP(ETK_D9_ES2) },
	{ 24, 0, "HSUSB2_CLK", CP(ETK_D10_ES2) },
	{ 25, 0, "HSUSB2_STP", CP(ETK_D11_ES2) },
	{ 26, 0, "HSUSB2_DIR", CP(ETK_D12_ES2) },
	{ 27, 0, "HSUSB2_NXT", CP(ETK_D13_ES2) },
	{ 28, 0, "HSUSB2_DATA0", CP(ETK_D14_ES2) },
	{ 29, 0, "HSUSB2_DATA1", CP(ETK_D15_ES2) },
	
	{ 54, 0, "PoP Temp", CP(GPMC_NCS3) },
	{ 55, 0, "Audio out en", CP(GPMC_NCS4) },
	{ 56, 0, "Gyro int", CP(GPMC_NCS5) },	
	{ 57, 0, "Backlight en", CP(GPMC_NCS6) },
	
	{ 66, 0, "PCLK", CP(DSS_PCLK) },
	{ 67, 0, "HSYNC", CP(DSS_HSYNC) },
	{ 68, 0, "VSYNC", CP(DSS_VSYNC) },
	{ 69, 0, "DE", CP(DSS_ACBIAS) },
	{ 70, 0, "DSS0", CP(DSS_DATA0) },
	{ 71, 0, "DSS1", CP(DSS_DATA1) },
	{ 72, 0, "DSS2", CP(DSS_DATA2) },
	{ 73, 0, "DSS3", CP(DSS_DATA3) },
	{ 74, 0, "DSS4", CP(DSS_DATA4) },
	{ 75, 0, "DSS5", CP(DSS_DATA5) },
	{ 76, 0, "DSS6", CP(DSS_DATA6) },
	{ 77, 0, "DSS7", CP(DSS_DATA7) },
	{ 78, 0, "DSS8", CP(DSS_DATA8) },
	{ 79, 0, "DSS9", CP(DSS_DATA9) },
	{ 80, 0, "DSS10", CP(DSS_DATA10) },
	{ 81, 0, "DSS11", CP(DSS_DATA11) },
	{ 82, 0, "DSS12", CP(DSS_DATA12) },
	{ 83, 0, "DSS13", CP(DSS_DATA13) },
	{ 84, 0, "DSS14", CP(DSS_DATA14) },
	{ 85, 0, "DSS15", CP(DSS_DATA15) },
	{ 86, 0, "DSS16", CP(DSS_DATA16) },
	{ 87, 0, "DSS17", CP(DSS_DATA17) },
	{ 88, 0, "DSS18", CP(DSS_DATA18) },
	{ 89, 0, "DSS19", CP(DSS_DATA19) },
	{ 90, 0, "DSS20", CP(DSS_DATA20) },
	{ 91, 0, "DSS21", CP(DSS_DATA21) },
	{ 92, 0, "DSS22", CP(DSS_DATA22) },
	{ 93, 0, "DSS23", CP(DSS_DATA23) },
	{ 94, 0, "CAM_HS", CP(CAM_HS) },
	{ 95, 0, "CAM_VS", CP(CAM_VS) },
	{ 96, 0, "CAM_XCLKA", CP(CAM_XCLKA) },
	{ 97, 0, "CAM_PCLK", CP(CAM_PCLK) },
	{ 98, 0, "CAM_RESET", CP(CAM_FLD) },
	{ 99, 1, "CAM_D0", CP(CAM_D0) },
	{ 100, 1, "CAM_D1", CP(CAM_D1) },
	{ 101, 0, "CAM_D2", CP(CAM_D2) },
	{ 102, 0, "CAM_D3", CP(CAM_D3) },
	{ 103, 0, "CAM_D4", CP(CAM_D4) },
	{ 104, 0, "CAM_D5", CP(CAM_D5) },
	{ 105, 1, "CAM_D6", CP(CAM_D6) },
	{ 106, 1, "CAM_D7", CP(CAM_D7) },
	{ 107, 1, "CAM_D8", CP(CAM_D8) },
	{ 108, 1, "CAM_D9", CP(CAM_D9) },
	{ 109, 0, "CAM_D10", CP(CAM_D10) },
	{ 110, 0, "CAM_D11", CP(CAM_D11) },
	{ 111, 0, "CAM_XCLKB", CP(CAM_XCLKB) },
	{ 112, 1, "Compass rdy", CP(CSI2_DX0) },
	{ 113, 1, "Baro EOC", CP(CSI2_DY0) },
	{ 114, 1, "Accel2 int", CP(CSI2_DX1) },
	{ 115, 1, "Accel1 int", CP(CSI2_DY1) },
	{ 116, 0, "Audio_FSX", CP(MCBSP2_FSX) },
	{ 117, 0, "Audio_CLKX", CP(MCBSP2_CLKX) },
	{ 118, 0, "Audio_DR", CP(MCBSP2_DR) },
	{ 119, 0, "Audio_DX", CP(MCBSP2_DX) },

	/*	this are duplicate GPIOs with MMC1
	{ 120, 0, "HSUSB0_CLK", CP(HSUSB0_CLK) },
	{ 121, 0, "HSUSB0_STP", CP(HSUSB0_STP) },
	{ 122, 0, "HSUSB0_DIR", CP(HSUSB0_DIR) },
	
	{ 124, 0, "HSUSB0_NXT", CP(HSUSB0_NXT) },
	{ 125, 0, "HSUSB0_DATA0", CP(HSUSB0_DATA0) },
	 */

	{ 126, 0, "CAM_STROBE", CP(CAM_STROBE) },
	
	/*	this are duplicate GPIOs with MMC2
	 { 130, 0, "HSUSB0_DATA1", CP(HSUSB0_DATA1) },
	 { 131, 0, "HSUSB0_DATA2", CP(HSUSB0_DATA2) },
	 */
	
	{ 130, 0, "MMC2_CLK", CP(MMC2_CLK) },
	{ 131, 0, "MMC2_CMD", CP(MMC2_CMD) },
	{ 132, 0, "MMC2_DAT0", CP(MMC2_DAT0) },
	{ 133, 0, "MMC2_DAT1", CP(MMC2_DAT1) },
	{ 134, 0, "MMC2_DAT2", CP(MMC2_DAT2) },
	{ 135, 0, "MMC2_DAT3", CP(MMC2_DAT3) },
	{ 136, 0, "MMC2_DIR_DAT0", CP(MMC2_DAT4) },
	{ 137, 0, "MMC2_DIR_DAT1", CP(MMC2_DAT5) },
	{ 138, 0, "MMC2_DIR_CMD", CP(MMC2_DAT6) },
	{ 139, 0, "MMC2_DIR_CLKIN", CP(MMC2_DAT7) },	
	{ 140, 0, "BT_DX", CP(MCBSP3_DX) },
	{ 141, 0, "BT_DR", CP(MCBSP3_DR) },
	{ 142, 0, "BT_CLKX", CP(MCBSP3_CLKX) },
	{ 143, 0, "BT_FSX", CP(MCBSP3_FSX) },
	{ 144, 0, "Ext Ant", CP(UART2_CTS) },
//	{ 145, 0, "GPS enable", CP(UART2_RTS) },	// should remain disabled!
	{ 146, 0, "GPS_TX", CP(UART2_TX) },
	{ 147, 0, "GPS_RX", CP(UART2_RX) },
	{ 148, 0, "BT_TX", CP(UART1_TX) },
	{ 149, 0, "BT_RTS", CP(UART1_RTS) },
	{ 150, 0, "BT_CTS", CP(UART1_CTS) },
	{ 151, 0, "BT_RX", CP(UART1_RX) },
	{ 152, 0, "WWAN_CLKX", CP(MCBSP4_CLKX) },
	{ 153, 0, "WWAN_DR", CP(MCBSP4_DR) },
	{ 154, 0, "WWAN_DX", CP(MCBSP4_DX) },
	{ 155, 0, "WWAN_FSX", CP(MCBSP4_FSX) },
	{ 156, 0, "FM_CLKR", CP(MCBSP1_CLKR) },
	{ 157, 0, "FM_FSR", CP(MCBSP1_FSR) },
	{ 158, 0, "FM_DX", CP(MCBSP1_DX) },
	{ 159, 0, "FM_DR", CP(MCBSP1_DR) },
	{ 160, 0, "PENIRQ", CP(MCBSP_CLKS) },
	{ 161, 0, "FM_FSX", CP(MCBSP1_FSX) },
	{ 162, 0, "FM_CLKX", CP(MCBSP1_CLKX) },
		
	{ 167, 0, "CAM_PWDN", CP(CAM_WEN) },
	
	{ 169, 0, "HSUSB0_DATA3", CP(HSUSB0_DATA3) },

	{ 171, 0, "Revision 0x1", CP(MCSPI1_CLK) },
	{ 172, 0, "Revision 0x2", CP(MCSPI1_SIMO) },
	{ 173, 0, "Revision 0x4", CP(MCSPI1_SOMI) },
//	{ 174, 0, "USB3322 reset", CP(MCSPI1_CS0) },	// should remain in reset state
	{ 175, 0, "A3: HOST_WAKE / A4:spare", CP(MCSPI1_CS1) },
	{ 176, 0, "A3: 3G_WAKE / A4: KEYIRQ", CP(MCSPI1_CS2) },
	{ 177, 0, "HSUSB2_DATA2", CP(MCSPI1_CS3) },
	{ 178, 0, "HSUSB2_DATA7", CP(MCSPI2_CLK) },
	{ 179, 0, "HSUSB2_DATA4", CP(MCSPI2_SIMO) },
	{ 180, 0, "HSUSB2_DATA5", CP(MCSPI2_SOMI) },
	{ 181, 0, "HSUSB2_DATA6", CP(MCSPI2_CS0) },
	{ 182, 0, "HSUSB2_DATA3", CP(MCSPI2_CS1) },
	
	{ 186, 0, "A3: spare / A4: ON_KEY", CP(SYS_CLKOUT2) },
	
	{ 188, 0, "HSUSB0_DATA4", CP(HSUSB0_DATA4) },
	{ 189, 0, "HSUSB0_DATA5", CP(HSUSB0_DATA5) },
	{ 190, 0, "HSUSB0_DATA6", CP(HSUSB0_DATA6) },
	{ 191, 0, "HSUSB0_DATA7", CP(HSUSB0_DATA7) },
	
};

/*
 int i2c_read(u_int8_t chip, u_int32_t addr, int alen, u_int8_t *buf, int len)
 int i2c_write(u_int8_t chip, u_int32_t addr, int alen, u_int8_t *buf, int len)
 */

#define TCA8418_BUS (2-1)	// I2C1=0, I2C2=1, I2C3=2
#define TCA8418_ADDRESS 0x34

int keytest(void)
{
	unsigned char byte;
	printf("testing TCA8418 keyboard\n");
	if(i2c_set_bus_num(TCA8418_BUS))
		{
		printf ("could not select I2C%d\n", TCA8418_BUS+1);
		return 1;
		}
	
	if(i2c_probe(TCA8418_ADDRESS))
		{
		printf ("could not probe TCA8418 at 0x%02x\n", TCA8418_ADDRESS);
		return 1;		
		}
	
	byte = 0x01;
	i2c_write(TCA8418_ADDRESS, 0x01, 1, &byte, 1);	// enable keyboard IRQ
	byte = 0xff;
	i2c_write(TCA8418_ADDRESS, 0x1d, 1, &byte, 1);	// enable all lines as KP matrix
	i2c_write(TCA8418_ADDRESS, 0x1e, 1, &byte, 1);	// enable all lines as KP matrix
	i2c_write(TCA8418_ADDRESS, 0x1f, 1, &byte, 1);	// enable all lines as KP matrix
		
	while (!tstc())
		{
		int keyint=(status_get_buttons() & (1<<5)) != 0;	// get_button() knows how to read the KEYIRQ
#if 1
		if(!keyint)
		   continue;	// wait for key interrupt
#endif
		i2c_read(TCA8418_ADDRESS, 0x02, 1, &byte, 1);	// read int-status register
#if 0
		printf("keyint=%d intstat=%02x\r", keyint, byte);
#endif
		if(byte&1)
			{ // read matrix
			int i;
			unsigned char events;
			i2c_read(TCA8418_ADDRESS, 0x03, 1, &events, 1);	// read number of events in FIFO
			events &= 0x0f;	// up to 10
			for(i=0; i < events; i++)
				{
				i2c_read(TCA8418_ADDRESS, 0x04, 1, &byte, 1);	// read keypad event as often as there is an event
				if(byte & 0x80)
					printf("press   %2d\n", byte&0x7f);
				else
					printf("release %2d\n", byte&0x7f);
				}
			byte=0x01;
			i2c_write(TCA8418_ADDRESS, 0x02, 1, &byte, 1);	// write int-status register to reset keypad int
			}
		}
	if(tstc())
		getc();
	printf("\n");
	return 0;
}

int gpiotest(void)
{
	int i;
	printf("testing GPIOs for stuck-at, floating and shorts\n");
	omap_set_gpio_direction(174, 0);	// switch to output
	omap_set_gpio_dataout(174, 0); // activate reset of USB3322
	// Camera reset?
	// WLAN-Reset?
	// switch off GPS (if on)
	for(i=0; i<sizeof(gpiotable)/sizeof(gpiotable[0]); i++)
		{
		int g=gpiotable[i].gpio;
		omap_request_gpio(g);
		omap_set_gpio_direction(g, 1);	// make them all inputs
		if(gpiotable[i].offset)
			writew((IEN  | PTD | DIS | M4), OMAP34XX_CTRL_BASE + (gpiotable[i].offset));	// make input GPIO w/o pullup/down
		udelay(100);
		}
	for(i=0; i<sizeof(gpiotable)/sizeof(gpiotable[0]); i++)
		{ // loop over all inputs
			int g=gpiotable[i].gpio;
			char *n=gpiotable[i].name;
			int j;
			for(j=0; j<sizeof(gpiotable)/sizeof(gpiotable[0]); j++)
				{ // check for shorts with other GPIOs switched to output
					int gj=gpiotable[j].gpio;
					char *nj=gpiotable[j].name;
					int valdn, valup; 
					int stuck=0;	// if known to be stuck
					int follows=0;	// if known to follow
					if(!gpiotable[j].inputonly)
						{ // test with active I/O
							omap_set_gpio_direction(gj, 0);	// switch to output
							udelay(100);
							omap_set_gpio_dataout(gj, 0); // set other output to 0
							udelay(300);
							valdn=omap_get_gpio_datain(g);	// read value of input GPIO under test
							omap_set_gpio_dataout(gj, 1); // set other output to 1
							udelay(300);
							valup=omap_get_gpio_datain(g);	// read value of input GPIO under test
							omap_set_gpio_direction(gj, 1);	// switch back to input
							udelay(100);
							if(valdn == 0 && valup == 1)
								{ // our input follows the other in output mode
								if(i != j)
									printf("GPIO %d (%s) follows GPIO %d (%s)\n", g, n, gj, nj), follows=1;
								}
							else if(i == j)
								{ // output does *not* read back the input value -> shorted
									if(valdn == 0 && valup == 0)
										printf("GPIO %d (%s) stuck at 0\n", g, n), stuck=1;
									else if(valdn == 1 && valup == 1)
										printf("GPIO %d (%s) stuck at 1\n", g, n), stuck=1;
								}
						}
					writew((IEN  | PTD | EN | M4), OMAP34XX_CTRL_BASE + (gpiotable[j].offset));	// make other GPIO input with pulldown
					udelay(300);
					valdn=omap_get_gpio_datain(g);	// read value of GPIO under test
					writew((IEN  | PTU | EN | M4), OMAP34XX_CTRL_BASE + (gpiotable[j].offset));	// make other GPIO input with pullup
					udelay(300);
					valup=omap_get_gpio_datain(g);	// read value of GPIO under test
					writew((IEN  | PTD | DIS | M4), OMAP34XX_CTRL_BASE + (gpiotable[j].offset)); // make other GPIO w/o pullup/down
					udelay(100);
					if(valdn == 0 && valup == 1)
						{ // follows the other pull-up/down
							if(i == j)
								printf("GPIO %d (%s) floating\n", g, n);	// follows our own pull-up/down
							else if(!follows)
								printf("GPIO %d (%s) weakly follows GPIO %d (%s)%s\n", g, n, gj, nj, gpiotable[j].inputonly?" (strong not tested)":"");	// -> GPIOs are connected with low to high resistance
						}
					else if(i == j && !stuck)
						{
						if(valdn == 0 && valup == 0)
							printf("GPIO %d (%s) weak 0%s\n", g, n, gpiotable[j].inputonly?" (strong not tested)":"");
						else if(valdn == 1 && valup == 1)
							printf("GPIO %d (%s) weak 1%s\n", g, n, gpiotable[j].inputonly?" (strong not tested)":"");
						}
				}
		}											 
	return 0;
}
	
