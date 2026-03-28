/*
 * Ingenic ad100 REG_BASE definitions
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
#define CCU_BASE        0xb2200000

/* APB BUS Devices Base */
#define	CPM_BASE	0xb0000000
#define	INTC_BASE	0xb0001000
#define	RTC_BASE	0xb0003000
#define	GPIO_BASE	0xb3601000
#define	UART0_BASE	0xb0030000
#define	UART1_BASE	0xb0031000
#define	UART2_BASE	0xb0032000
#define	UART3_BASE	0xb0033000
#define	UART4_BASE	0xb0034000

#define OTG_BASE        0xb3500000
#define OTGPHY_BASE	0xb0078000

#define G_OST_BASE	0xb2000000

#define DDRC_APB_BASE   0xb3012000

#define LCDC0_BASE	0xb3050000

#define MSC0_BASE	0xb3450000
#define MSC1_BASE	0xb3460000

#define GMAC0_BASE	0xb34b0000

#define SFC_BASE        0xb3440000
#define EFUSE_BASE      0xb3480000

/* AHB_MCU BUS Devices Base */
#define	TCU_BASE	0xb3630000
#define	WDT_BASE	0xb3630000

#endif /* __BASE_H__ */
