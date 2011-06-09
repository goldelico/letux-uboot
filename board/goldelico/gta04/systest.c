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
#include "systest.h"
#include "TD028TTEC1.h"

#define TWL4030_I2C_BUS			(1-1)

int systest(void)
{ // do mixture of gps_echo, tsc_loop, status mirror status blink
	int r;
	i2c_set_bus_num(TWL4030_I2C_BUS);	// I2C1
	printf("TPS65950:      %s\n", !(r=i2c_probe(TWL4030_CHIP_USB))?"found":"-");	// responds on 4 addresses 0x48..0x4b
	if(!r)
		{ // was ok, ask for details
		u8 val;
		u8 val2;
		twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER, &val, TWL4030_PM_MASTER_STS_HW_CONDITIONS);
		printf("  STS_HW_CND: %02x", val);	// decode bits
		if(val & 0x80) printf(" VBUS");
		if(val & 0x08) printf(" NRESWARM");
		if(val & 0x04) printf(" USB");
		if(val & 0x02) printf(" CHG");
		if(val & 0x01) printf(" PWRON");
		printf("\n");
		twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER, &val, 0x2e);
		printf("  PWR_ISR:    %02x", val);
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
	i2c_set_bus_num(1);	// I2C2
	printf("TSC2007:       %s\n", !i2c_probe(0x48)?"found":"-");
	printf("TCA6507:       %s\n", !i2c_probe(0x45)?"found":"-");
	printf("LIS302 TOP:    %s\n", !i2c_probe(0x1c)?"found":"-");
	printf("LIS302 BOTTOM: %s\n", !i2c_probe(0x1d)?"found":"-");
	printf("LSM303:        %s\n", !i2c_probe(0x19)?"found":"-");
	printf("HMC58xx:       %s\n", !i2c_probe(0x1e)?"found":"-");
	printf("BMA180:        %s\n", !i2c_probe(0x41)?"found":"-");
	printf("BMP085:        %s\n", !i2c_probe(0x77)?"found":"-");
	printf("ITG3200:       %s\n", !i2c_probe(0x68)?"found":"-");
	printf("Si47xx:        %s\n", !i2c_probe(0x21)?"found":"-");
	printf("TCA8418:       %s\n", !i2c_probe(0x64)?"found":"-");
	printf("OV9655:        %s\n", !i2c_probe(0x30)?"found":"-");
	printf("TPS61050:      %s\n", !i2c_probe(0x33)?"found":"-");
	printf("EEPROM:        %s\n", !i2c_probe(0x50)?"found":"-");
	printf("VCNL4000:      %s\n", !i2c_probe(0x13)?"found":"-");
	i2c_set_bus_num(2);	// I2C3
	/* nothing to check */
	i2c_set_bus_num(TWL4030_I2C_BUS);	// I2C1
	if(!jbt_check())
	    printf("DISPLAY:       ok\n");
	else
		printf("DISPLAY:       failed\n");
	
	// LEDs
	// GPS UART
	// RAM-Test
	// NAND-Test
	// Buttons
	// Power
	// Display communication
	// PCM
	// OTG USB
	// Charger
	// internal USB
	// UMTS Module
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

int audiotest(int channel)
{
	
	unsigned char byte;
	int count = 0;
	
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
