/*
 * X2000 cpm definitions
 *
 * Copyright (c) 2016 Ingenic Semiconductor Co.,Ltd
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

#define CPM_CPCCR		(0x00)
#define CPM_CPCSR		(0xD4)
#define CPM_DDRCDR		(0x2C)
#define CPM_MACPHYCDR		(0x54)
#define CPM_MACTXCDR0		(0x58)
#define CPM_MACTXCDR1		(0xDC)
#define CPM_MACPTPCDR		(0x4c)
#define CPM_I2S0CDR		(0x60)
#define CPM_I2S0CDR1		(0x70)
#define CPM_I2S1CDR		(0x7C)
#define CPM_I2S1CDR1		(0x80)
#define CPM_I2S2CDR		(0x84)
#define CPM_I2S2CDR1		(0x88)
#define CPM_I2S3CDR		(0x8C)
#define CPM_I2S3CDR1		(0xA0)
#define CPM_AUDIOCR		(0xAC)
#define CPM_LPCDR		(0x64)
#define CPM_MSC0CDR		(0x68)
#define CPM_MSC1CDR		(0xA4)
#define CPM_MSC2CDR		(0xA8)
#define CPM_SFCCDR		(0x74)
#define CPM_SSICDR		(0x5C)
#define CPM_CIMCDR		(0x78)
#define CPM_PWMCDR		(0x6C)
#define CPM_ISPCDR		(0x30)
#define CPM_RSACDR		(0x50)
#define CPM_MACPHYC0		(0xE4)
#define CPM_MACPHYC1		(0xE8)
#define CPM_INTR		(0xB0)
#define CPM_INTRE		(0xB4)
#define CPM_SFTINT		(0xBC)
#define CPM_DRCG		(0xD0)
#define CPM_CPSPR		(0x34)
#define CPM_CPSPPR		(0x38)
#define CPM_USBPCR		(0x3C)
#define CPM_USBRDT		(0x40)
#define CPM_USBVBFIL		(0x44)
#define CPM_USBPCR1		(0x48)
#define CPM_CPPCR		(0x0C)
#define CPM_CPAPCR		(0x10)
#define CPM_CPMPCR		(0x14)
#define CPM_CPEPCR		(0x18)

#define CPM_LCR			(0x04)
#define CPM_PSWC0ST		(0x90)
#define CPM_PSWC1ST		(0x94)
#define CPM_PSWC2ST		(0x98)
#define CPM_PSWC3ST		(0x9C)
#define CPM_SRBC		(0xC4)
#define CPM_SLBC		(0xC8)
#define CPM_SLPC		(0xCC)
#define CPM_CLKGR0		(0x20)
#define CPM_CLKGR1		(0x28)
#define CPM_MEMPD0		(0xF8)
#define CPM_MEMPD1		(0xFC)
#define CPM_OPCR		(0x24)
#define CPM_MESTSEL		(0xEC)
#define CPM_EXCLK_DS		(0xE0)


#define LCR_LPM_MASK		(0x3)
#define LCR_LPM_SLEEP		(0x1)

#define CPM_CLKGR_DDR		(1 << 31)
#define CPM_CLKGR_IPU		(1 << 30)
#define CPM_CLKGR_AHB0		(1 << 29)
#define CPM_CLKGR_APB0		(1 << 28)
#define CPM_CLKGR_RTC		(1 << 27)
#define CPM_CLKGR_SSI1		(1 << 26)
#define CPM_CLKGR_RSA		(1 << 25)
#define CPM_CLKGR_AES		(1 << 24)
#define CPM_CLKGR_LCD		(1 << 23)
#define CPM_CLKGR_CIM		(1 << 22)
#define CPM_CLKGR_PDMA		(1 << 21)
#define CPM_CLKGR_OST		(1 << 20)
#define CPM_CLKGR_SSI0		(1 << 19)
#define CPM_CLKGR_TCU		(1 << 18)
#define CPM_CLKGR_DTRNG		(1 << 17)
#define CPM_CLKGR_UART2		(1 << 16)
#define CPM_CLKGR_UART1		(1 << 15)
#define CPM_CLKGR_UART0		(1 << 14)
#define CPM_CLKGR_SADC		(1 << 13)
#define CPM_CLKGR_HELIX		(1 << 12)
#define CPM_CLKGR_AUDIO		(1 << 11)
#define CPM_CLKGR_I2C3	        (1 << 10)
#define CPM_CLKGR_I2C2		(1 << 9)
#define CPM_CLKGR_I2C1		(1 << 8)
#define CPM_CLKGR_I2C0		(1 << 7)
#define CPM_CLKGR_SCC		(1 << 6)
#define CPM_CLKGR_MSC1		(1 << 5)
#define CPM_CLKGR_MSC0		(1 << 4)
#define CPM_CLKGR_OTG		(1 << 3)
#define CPM_CLKGR_SFC		(1 << 2)
#define CPM_CLKGR_EFUSE		(1 << 1)
#define CPM_CLKGR_NEMC		(1 << 0)

#define CPM_CLKGR_MIPI_DSI	(1 << 29)
#define CPM_CLKGR_MIPI_CSI	(1 << 28)
#define CPM_CLKGR_INTC		(1 << 26)
#define CPM_CLKGR_MSC2		(1 << 25)
#define CPM_CLKGR_GMAC1		(1 << 24)
#define CPM_CLKGR_GMAC0		(1 << 23)
#define CPM_CLKGR_UART9		(1 << 22)
#define CPM_CLKGR_UART8		(1 << 21)
#define CPM_CLKGR_UART7		(1 << 20)
#define CPM_CLKGR_UART6		(1 << 19)
#define CPM_CLKGR_UART5		(1 << 18)
#define CPM_CLKGR_UART4		(1 << 17)
#define CPM_CLKGR_UART3		(1 << 16)
#define CPM_CLKGR_SPDIF		(1 << 14)
#define CPM_CLKGR_DMIC		(1 << 13)
#define CPM_CLKGR_PCM		(1 << 12)
#define CPM_CLKGR_I2S3		(1 << 11)
#define CPM_CLKGR_I2S2		(1 << 10)
#define CPM_CLKGR_I2S1		(1 << 9)
#define CPM_CLKGR_I2S0		(1 << 8)
#define CPM_CLKGR_ROT		(1 << 7)
#define CPM_CLKGR_HASH		(1 << 6)
#define CPM_CLKGR_PWM		(1 << 5)
#define CPM_CLKGR_FELIX		(1 << 4)
#define CPM_CLKGR_ISP1		(1 << 3)
#define CPM_CLKGR_ISP0		(1 << 2)
#define CPM_CLKGR_I2C5		(1 << 1)
#define CPM_CLKGR_I2C4		(1 << 0)

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
#define USBPCR1_WORD_IF_16_30	(1 << 19)

#define OPCR_SPENDN		(1 << 7)

#ifndef BIT
#define BIT(nr)  (1UL << nr)
#endif

/*USB Parameter Control Register*/
#define USBPCR_USB_MODE                 BIT(31)
#define USBPCR_AVLD_REG                 BIT(30)
#define USBPCR_IDPULLUP_MASK_BIT        28
#define USBPCR_IDPULLUP_MASK		(0x3 << USBPCR_IDPULLUP_MASK_BIT)
#define USBPCR_IDPULLUP_OTG             (0x0 << USBPCR_IDPULLUP_MASK_BIT)
#define USBPCR_IDPULLUP_ALWAYS_SUSPEND  (0x1 << USBPCR_IDPULLUP_MASK_BIT)
#define USBPCR_IDPULLUP_ALWAYS          (0x2 << USBPCR_IDPULLUP_MASK_BIT)
#define USBPCR_INCR_MASK                BIT(27)

/*USB Reset Detect Timer Register*/
#define USBRDT_RESUME_INTEEN		BIT(31)	/*RW*/
#define USBRDT_RESUME_INTERCLR		BIT(30)	/*W0*/
#define USBRDT_RESUME_SPEED_BIT		28	/*RW*/
#define USBRDT_RESUME_SPEED_MSK		(0x3 << USBRDT_RESUME_SPEED_BIT)
#define USBRDT_RESUME_SPEED_HIGH	(0x0 << USBRDT_RESUME_SPEED_BIT)
#define USBRDT_RESUME_SPEED_FULL	(0x1 << USBRDT_RESUME_SPEED_BIT)
#define USBRDT_RESUME_SPEED_LOW		(0x2 << USBRDT_RESUME_SPEED_BIT)
#define USBRDT_RESUME_STATUS		BIT(27)	/*RO*/
#define USBRDT_HB_MASK                  BIT(26)
#define USBRDT_VBFIL_LD_EN              BIT(25)
#define USBRDT_IDDIG_EN			BIT(24)
#define USBRDT_IDDIG_REG                BIT(23)
#define USBRDT_USBRDT_MSK               (0x7fffff)
#define USBRDT_USBRDT(x)                ((x) & USBRDT_USBRDT_MSK)

/*USB VBUS Jitter Filter Register*/
#define USBVBFIL_USBVBFIL(x)		((x) & 0xffff)
#define USBVBFIL_IDDIGFIL(x)		((x) & (0xffff << 16))

/*USB Parameter Control Register1*/
#define USBPCR1_BVLD_REG        BIT(31)
#define USBPCR1_DPPULLDOWN	BIT(29)
#define USBPCR1_DMPULLDOWN	BIT(28)
#define USBPCR1_PORT_RST	BIT(21)

/*Oscillator and Power Control Register*/
#define OPCR_USB_SPENDN		BIT(7)
#define OPCR_USB_PHY_GATE	BIT(23)


#define LCR_LPM_MASK		(0x3)
#define LCR_LPM_SLEEP		(0x1)

#define OPCR_ERCS		(0x1<<2)
#define OPCR_PD			(0x1<<3)
#define OPCR_IDLE		(0x1<<31)

/* MSC EXTCLK enable BIT */
#define MSCCDR_EXCK_E		(1 << 21)
#define MSCCDR_MPCS		(30)
#define MSCCDR_MPCS_MASK	(3 << MSCCDR_MPCS)
#define MSCCDR_MPCS_EXCLK	(2 << MSCCDR_MPCS)

#define cpm_inl(off)		readl(CPM_BASE + (off))
#define cpm_outl(val,off)	writel(val,CPM_BASE + (off))
#define cpm_clear_bit(val,off)	do{cpm_outl((cpm_inl(off) & ~(1<<(val))),off);}while(0)
#define cpm_set_bit(val,off)	do{cpm_outl((cpm_inl(off) |  (1<<val)),off);}while(0)
#define cpm_test_bit(val,off)	(cpm_inl(off) & (0x1<<val))

/* CPM scratch pad protected register(CPSPPR) */
#define CPSPPR_CPSPR_WRITABLE   (0x00005a5a)
#define RECOVERY_SIGNATURE      (0x1a1a)        /* means "RECY" */
#define RECOVERY_SIGNATURE_SEC  0x800           /* means "RECY" */
#define FASTBOOT_SIGNATURE      (0x0666)        /* means "FASTBOOT" */

#define cpm_get_scrpad()        readl(CPM_BASE + CPM_CPSPR)
#define cpm_set_scrpad(data)                    \
do {                                            \
	volatile int i = 0x3fff;                \
	writel(0x00005a5a,CPM_BASE + CPM_CPSPPR);		\
	while(i--);				\
	writel(data,CPM_BASE + CPM_CPSPR);			\
	writel(0x0000a5a5,CPM_BASE + CPM_CPSPPR);      	\
} while (0)

#endif /* __CPM_H__ */
