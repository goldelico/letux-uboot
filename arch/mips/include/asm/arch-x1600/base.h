/*
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
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
 */

#ifndef __BASE_H__
#define __BASE_H__

/*
 * Define the module base addresses
 */

/* AHB0 BUS Devices Base */
#define DDRC_BASE	0xb34f0000
#define DDRC_APB_OFFSET (-0x4e0000 + 0x2000)
#define DDR_PHY_OFFSET	(-0x4e0000 + 0x1000)
#define LCDC_BASE	0xb3050000
#define CIM_BASE	0xb3060000

/* AHB2 BUS Devices Base */
#define NEMC_BASE	0xb3410000
#define PDMA_BASE	0xb3420000
#define AES_BASE	0xb3430000
#define SFC_BASE	0xb3440000
#define MSC0_BASE	0xb3450000
#define MSC1_BASE	0xb3460000
#define HASH_IOBASE	0xb3480000
#define MAC_IOBASE	0xb34b0000
#define PWM_IOBASE	0xb34c0000
#define OTG_BASE	0xb3500000
#define EFUSE_BASE	0xb3540000


/* APB BUS Devices Base */
#define	CPM_BASE	0xb0000000
#define	INTC_BASE	0xb0001000
#define	TCU_BASE	0xb0002000
#define	RTC_BASE	0xb0003000
#define	GPIO_BASE	0xb0010000
#define	AIC_BASE	0xb0079000
#define	UART0_BASE	0xb0030000
#define	UART1_BASE	0xb0031000
#define	UART2_BASE	0xb0032000
#define	SSI0_BASE	0xb0043000
#define	SSI_SLV0_BASE	0xb0045000
#define	I2C0_BASE	0xb0050000
#define	I2C1_BASE	0xb0051000
#define	WDT_BASE	0xb0002000
#define SADC_BASE	0xb0070000
#define DTRNG_BASE	0xb0072000
#define USB_PHY_BASE	0xb0060000

#define	OST_BASE	0xb2000000

/* NAND CHIP Base Address*/
#define NEMC_CS1_BASE 0xbb000000
#define NEMC_CS2_BASE 0xba000000

#endif /* __BASE_H__ */
