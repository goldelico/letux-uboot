/*
 * T21  cpm definitions
 *
 * Copyright (c) 2017 Ingenic Semiconductor Co.,Ltd
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

#ifndef __CPM_H__
#define __CPM_H__
#include <asm/arch/base.h>
/**********CGU registers Configuration*****/
#define CPM_CPCCR		(0x00)	/* clock control */
#define CPM_RSR			(0x08)	/* Reset status clock divider */
#define CPM_CPPCR		(0x0c)	/* pll control */
#define CPM_CPAPCR		(0x10)	/* apll control */
#define CPM_CPMPCR		(0x14)	/* mpll control */
#define CPM_CPEPCR		(0x18)	/* epll control */
#define CPM_CPVPCR		(0x1c)	/* vpll control */
#define CPM_CPAPACR		(0x20)	/* apll FRAC */
#define CPM_CPMPACR		(0x24)	/* mpll FRAC */
#define CPM_CPEPACR		(0x28)	/* epll FRAC */
#define CPM_CPVPACR		(0x2c)	/* vpll FRAC */
#define CPM_DDRCDR		(0x3c)	/* ddr memory clock divder */
#define CPM_SOFTAPP		(0x40)	/* soft used in application register */
#define CPM_CPSPR		(0x44)	/* cpm scratch pad register */
#define CPM_CPSPPR		(0x48)	/* cpm scratch pad protected register */
#define CPM_USB0PCR		(0x50)	/* usb0 parameter control register 0 */
#define CPM_USB0RDT		(0x54)	/* usb0 reset detect timer register */
#define CPM_USB0VBFIL		(0x58)	/* usb0 vbus jitter filter */
#define CPM_USB0PCR1		(0x5c)	/* usb0 parameter control register 1 */
#define CPM_USB1PCR		(0x60)	/* usb1 parameter control register 0 */
#define CPM_USB1RDT		(0x64)	/* usb1 reset detect timer register */
#define CPM_USB1VBFIL		(0x68)	/* usb1 vbus jitter filter */
#define CPM_USB1PCR1		(0x6c)	/* usb1 parameter control register 0 */
#define CPM_USB2PCR		(0x70)	/* usb2 parameter control register 0 */
#define CPM_USB2RDT		(0x74)	/* usb2 reset detect timer register */
#define CPM_USB2VBFIL		(0x78)	/* usb2 vbus jitter filter */
#define CPM_USB2PCR1		(0x7c)	/* usb2 parameter control register 0 */

//#define CPM_RSACDR		(0x80)	/* RSA clock divider */
#define CPM_AHB1CDR		(0x84)	/* AHB1 clock divider */
#define CPM_SSICDR		(0x8c)
#define CPM_SFC0CDR		(0x90)
#define CPM_SFC1CDR		(0x94)
#define CPM_MSC0CDR		(0x98)
#define CPM_MSC1CDR		(0x9c)
#define CPM_I2S0TCDR 		(0xa0)	/* I2S0 transmit clock divider */
#define CPM_I2S0TCDR1		(0xa4)
#define CPM_I2S0RCDR 		(0xa8)	/* I2S0 receive clock divider */
#define CPM_I2S0RCDR1		(0xac)
#define CPM_VDEVCDR		(0xb0)	/* VDE video output clock divider, dsc output clock */
#define CPM_VDEACDR		(0xb4)	/* VDE AXI clock divider */
#define CPM_VDEMCDR		(0xb8)	/* VDE main clock divider */
#define CPM_IPUMCDR		(0xbc)	/* IPU clock divider */
#define CPM_MAC0CDR		(0xc0)	/* MAC0 clock divider */
#define CPM_MAC0TXCDR		(0xc4)	/* MAC0 TX PHYclock divider */
#define CPM_MAC0PHYC		(0xc8)	/* MAC0 PHY control clock divider */
#define CPM_SATACDR		(0xcc)	/* SATA clock divider */
#define CPM_MAC1CDR		(0xd0)	/* MAC1 clock divider */
#define CPM_MAC1TXCDR		(0xd4)	/* MAC1 TX PHYclock divider */
#define CPM_MAC1PHYC		(0xd8)	/* MAC1 PHY control clock divider */

#define CPM_INTR		(0xe0)
#define CPM_INTRE		(0xe4)
#define CPM_DRCG		(0xe8)	/* DDR clock gate register */
#define CPM_CPCSR		(0xec)	/* clock status register */

#define CPM_BT0CDR		(0xf8)	/* BT0 clock divider */
#define CPM_PWMCDR		(0xfc)	/* BT0 clock divider */
#define CPM_I2S1TCDR 		(0x100)	/* I2S1 transmit clock divider */
#define CPM_I2S1TCDR1		(0x104)
#define CPM_I2S1RCDR 		(0x108)	/* I2S1 receive clock divider */
#define CPM_I2S1RCDR1		(0x10c)

/* power and clock gate */
#define CPM_LCR			(0x04)	/* low power control */
#define CPM_CLKGR0		(0x30)	/* clock gate register 0 */
#define CPM_OPCR		(0x34)	/* special control register, oscillator and power control register*/
#define CPM_CLKGR1		(0x38)	/* clock gate register 1*/
#define CPM_OSCCTRL		(0x4c)	/* oscillator control */
#define CPM_SRBC0		(0xf0)	/* soft reset and bus control register */
#define CPM_MESTSEL		(0xf4)	/* CPM metastable state sel register */

/******************************************/

/****************CLKGR0******************/

#define CPM_CLKGR_DMIC		(1 << 31)
#define CPM_CLKGR_AIC		(1 << 30)
#define CPM_CLKGR_IPU		(1 << 29)
#define CPM_CLKGR_VDE		(1 << 28)
#define CPM_CLKGR_HDMI		(1 << 27)
#define CPM_CLKGR_VGA		(1 << 26)
#define CPM_CLKGR_SFC1		(1 << 25)
#define CPM_CLKGR_SFC0		(1 << 24)
#define CPM_CLKGR_AIC1		(1 << 23)
#define CPM_CLKGR_RTC		(1 << 22)
#define CPM_CLKGR_I2C1		(1 << 21)
#define CPM_CLKGR_I2C0		(1 << 20)
#define CPM_CLKGR_AHB1		(1 << 19)
#define CPM_CLKGR_AIP		(1 << 18)
#define CPM_CLKGR_SSI1		(1 << 17)
#define CPM_CLKGR_SSI0		(1 << 16)
#define CPM_CLKGR_MSC1		(1 << 15)
#define CPM_CLKGR_MSC0		(1 << 14)
#define CPM_CLKGR_OTG2		(1 << 13)
#define CPM_CLKGR_OTG1		(1 << 12)
#define CPM_CLKGR_OTG0		(1 << 11)
#define CPM_CLKGR_UART2		(1 << 10)
#define CPM_CLKGR_UART1		(1 << 9)
#define CPM_CLKGR_UART0		(1 << 8)
#define CPM_CLKGR_NEMC		(1 << 7)
#define CPM_CLKGR_OST		(1 << 6)
#define CPM_CLKGR_TCU		(1 << 5)
#define CPM_CLKGR_EFUSE		(1 << 4)
#define CPM_CLKGR_DDR		(1 << 3)
#define CPM_CLKGR_AHB0		(1 << 2)
#define CPM_CLKGR_APB0		(1 << 1)
#define CPM_CLKGR_CPU		(1 << 0)
/***************CLKGR1 ********************/
#define CPM_CLKGR_PWM		(1 << 15)
#define CPM_CLKGR_VO		(1 << 14)
#define CPM_CLKGR_JPEG		(1 << 13)
#define CPM_CLKGR_BUS_MONITOR	(1 << 12)
#define CPM_CLKGR_SATA		(1 << 11)
#define CPM_CLKGR_GMAC1		(1 << 10)
#define CPM_CLKGR_AES		(1 << 9)
#define CPM_CLKGR_GMAC0		(1 << 8)
#define CPM_CLKGR_DTRNG		(1 << 7)
#define CPM_CLKGR_VC8000D	(1 << 6)
#define CPM_CLKGR_DES		(1 << 5)
// #define CPM_CLKGR_RSA		(1 << 4)
#define CPM_CLKGR_DMAC		(1 << 3)
#define CPM_CLKGR_HASH		(1 << 2)
// #define CPM_CLKGR_PCM		(1 << 1)
// #define CPM_CLKGR_SADC		(1 << 0)


#define CPM_RSR_HR		(1 << 3)
#define CPM_RSR_P0R		(1 << 2)
#define CPM_RSR_WR		(1 << 1)
#define CPM_RSR_PR		(1 << 0)

#define OPCR_ERCS		(0x1<<2)
#define OPCR_PD			(0x1<<3)
#define OPCR_IDLE		(0x1<<31)

#define cpm_readl(off)          readl(CPM_BASE + (off))
#define cpm_writel(val,off)     writel(val, CPM_BASE + (off))
#define cpm_inl(off)		readl(CPM_BASE + (off))
#define cpm_outl(val,off)	writel(val, CPM_BASE + (off))
#define cpm_test_bit(bit,off)	(cpm_inl(off) & 0x1<<(bit))
#define cpm_set_bit(bit,off)	(cpm_outl((cpm_inl(off) | 0x1<<(bit)),off))
#define cpm_clear_bit(bit,off)	(cpm_outl(cpm_inl(off) & ~(0x1 << bit), off))

/* MSC EXTCLK enable BIT */
#define MSCCDR_EXCK_E           (1 << 21)
#define MSCCDR_MPCS             (30)
#define MSCCDR_MPCS_MASK        (3 << MSCCDR_MPCS)
#define MSCCDR_MPCS_EXCLK       (3 << MSCCDR_MPCS)

/*USBRDT*/
#define USBRDT_IDDIG_EN		0
#define USBRDT_IDDIG_REG        0

/*USBCDR*/
#define USBCDR_UCS_PLL		(1 << 31)
#define USBCDR_UPCS_MPLL	(1 << 30)
#define USBCDR_CE_USB		(1 << 29)
#define USBCDR_USB_BUSY		(1 << 28)
#define USBCDR_USB_STOP		(1 << 27)
#define USBCDR_USB_DIS		(1 << 26)
#define USBCDR_MIPI_CS		(1 << 25)
#define USBCDR_USBCDR_MSK	(0xff)

/*USBPCR*/
#define USBPCR_USB_MODE_ORG	(1 << 31)
#define USBPCR_VBUSVLDEXT	(1 << 24)
#define USBPCR_VBUSVLDEXTSEL	(1 << 23)
#define USBPCR_POR		(1 << 22)
#define USBPCR_OTG_DISABLE	(1 << 20)

/*USBPCR1*/
#define USBPCR1_REFCLKSEL_BIT	(26)
#define USBPCR1_REFCLKSEL_MSK	(0x3 << USBPCR1_REFCLKSEL_BIT)
#define USBPCR1_REFCLKSEL_CORE	(0x2 << USBPCR1_REFCLKSEL_BIT)
#define USBPCR1_REFCLKSEL_EXT	(0x1 << USBPCR1_REFCLKSEL_BIT)
#define USBPCR1_REFCLKSEL_CSL	(0x0 << USBPCR1_REFCLKSEL_BIT)
#define USBPCR1_REFCLKDIV_BIT	(24)
#define USBPCR1_REFCLKDIV_MSK	(0X3 << USBPCR1_REFCLKDIV_BIT)
#define USBPCR1_REFCLKDIV_19_2M	(0x3 << USBPCR1_REFCLKDIV_BIT)
#define USBPCR1_REFCLKDIV_48M	(0x2 << USBPCR1_REFCLKDIV_BIT)
#define USBPCR1_REFCLKDIV_24M	(0x1 << USBPCR1_REFCLKDIV_BIT)
#define USBPCR1_REFCLKDIV_12M	(0x0 << USBPCR1_REFCLKDIV_BIT)
#define USBPCR1_WORD_IF0_16_30	(1 << 19)

/*OPCR*/
#define OPCR_SPENDN0		(1 << 7)

/* CPM scratch pad protected register(CPSPPR) */
#define CPSPPR_CPSPR_WRITABLE   (0x00005a5a)
#define RECOVERY_SIGNATURE      (0x1a1a)        /* means "RECY" */
#define RECOVERY_SIGNATURE_SEC  0x800           /* means "RECY" */
#define FASTBOOT_SIGNATURE      (0x0666)        /* means "FASTBOOT" */

#define cpm_get_scrpad()        readl(CPM_BASE + CPM_CPSPR)
#define cpm_set_scrpad(data)                    \
do {                                            \
	volatile int i = 0x3fff;                \
	writel(0x5a5a,CPM_BASE + CPM_CPSPPR);		\
	while(i--);				\
	writel(data,CPM_BASE + CPM_CPSPR);			\
	writel(0xa5a5,CPM_BASE + CPM_CPSPPR);      	\
} while (0)

#endif /* __CPM_H__ */
