/*
 * ad100 cpm definitions
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

/*
 *  * CGU Register
 *   * */
#define CPM_CPCCR       (0x00)
#define CPM_CPCSR       (0xd4)

#define CPM_DDRCDR      (0x2c)
#define CPM_MACPHYCDR   (0x54)
#define CPM_I2SCDR      (0x60)
#define CPM_I2SCDR1     (0x70)
#define CPM_PCMCDR      (0xc0)
#define CPM_PCMCDR1     (0xdc)
#define CPM_DMICCR      (0xd8)
#define CPM_LPCDR       (0x64)
#define CPM_MSC0CDR     (0x68)
#define CPM_MSC1CDR     (0xa4)
#define CPM_SFCCDR      (0x74)
#define CPM_SSICDR      (0x5c)
#define CPM_G2DCDR      (0x30)
#define CPM_PWMCDR      (0x6c)
#define CPM_CAN0CDR     (0xa0)
#define CPM_CAN1CDR     (0xa8)
#define CPM_SADCCDR     (0xac)

#define CPM_INTR        (0xb0)
#define CPM_INTRE       (0xb4)
#define CPM_SFTINT      (0xbc)
#define CPM_DRCG        (0xd0)
#define CPM_CPSPR       (0x34)
#define CPM_CPSPPR      (0x38)

#define CPM_USBPCR      (0x3c)
#define CPM_USBRDT      (0x40)
#define CPM_USBVBFIL    (0x44)
#define CPM_USBPCR1     (0x48)
#define CPM_CPPCR       (0x0c)
#define CPM_CPAPCR      (0x10)
#define CPM_CPAPACR     (0x84)
#define CPM_CPMPCR      (0x14)
#define CPM_CPMPACR     (0x88)
#define CPM_CPEPCR      (0x18)
#define CPM_CPEPACR     (0x8c)

#define CPM_CPCSR		(0xd4)
#define CPM_DDRCDR		(0x2c)
#define CPM_SLPC        (0xCC)

#define cpm_inl(off)		readl(CPM_BASE + (off))
#define cpm_outl(val,off)	writel(val,CPM_BASE + (off))
#define cpm_clear_bit(val,off)	do{cpm_outl((cpm_inl(off) & ~(1<<(val))),off);}while(0)
#define cpm_set_bit(val,off)	do{cpm_outl((cpm_inl(off) |  (1<<val)),off);}while(0)
#define cpm_test_bit(val,off)	(cpm_inl(off) & (0x1<<val))
#define cpm_writel(val,off)   writel(val,CPM_BASE + (off))
#define cpm_readl(off)        readl(CPM_BASE + (off))

/************power/reset management*********/
#define CPM_LCR     (0x04)
#define CPM_CLKGR0  (0x20)
#define CPM_OPCR    (0x24)
#define CPM_CLKGR1  (0x28)
#define CPM_SRBC	(0xC4)

/**********CLKGR0 0x20**************/
#define CPM_CLKGR_NEMC	(1 << 31)
#define CPM_CLKGR_INTC	(1 << 30)
#define CPM_CLKGR_RTC	(1 << 29)
#define CPM_CLKGR_OST	(1 << 28)
#define CPM_CLKGR_DTRNG (1 << 27)
#define CPM_CLKGR_I2S   (1 << 26)
#define CPM_CLKGR_DMIC  (1 << 25)
#define CPM_CLKGR_AIC   (1 << 24)
#define CPM_CLKGR_MIPI_DSI (1 << 23)
#define CPM_CLKGR_SSISLV   (1 << 22)
#define CPM_CLKGR_SSI1	   (1 << 21)
#define CPM_CLKGR_SSI0	   (1 << 20)
#define CPM_CLKGR_UART7	   (1 << 19)
#define CPM_CLKGR_UART6    (1 << 18)
#define CPM_CLKGR_UART5    (1 << 17)
#define CPM_CLKGR_UART4    (1 << 16)
#define CPM_CLKGR_UART3    (1 << 15)
#define CPM_CLKGR_UART2    (1 << 14)
#define CPM_CLKGR_UART1    (1 << 13)
#define CPM_CLKGR_UART0    (1 << 12)
#define CPM_CLKGR_I2C3     (1 << 11)
#define CPM_CLKGR_I2C2     (1 << 9)
#define CPM_CLKGR_I2C1     (1 << 8)
#define CPM_CLKGR_I2C0     (1 << 7)
#define CPM_CLKGR_USB      (1 << 5)
#define CPM_CLKGR_OTG      (1 << 4)
#define CPM_CLKGR_MSC1     (1 << 3)
#define CPM_CLKGR_MSC0     (1 << 2)
#define CPM_CLKGR_SFC      (1 << 1)
#define CPM_CLKGR_EFUSE	   (1 << 0)

/**********CLKGR1 0x28**************/
#define CPM_CLKGR1_AHB0  (1 << 31)
#define CPM_CLKGR1_APB0  (1 << 30)
#define CPM_CLKGR1_AHB2  (1 << 29)
#define CPM_CLKGR1_APB   (1 << 28)
#define CPM_CLKGR1_ARB   (1 << 27)
#define CPM_CLKGR1_DDR   (1 << 23)
#define CPM_CLKGR1_PCM   (1 << 21)
#define CPM_CLKGR1_BUS_MON (1 <<20)
#define CPM_CLKGR1_JPEGE   (1 <<19)
#define CPM_CLKGR1_JPEGD   (1 <<18)
#define CPM_CLKGR1_FELIX   (1 <<17)
#define CPM_CLKGR1_ROT     (1 <<16)
#define CPM_CLKGR1_G2D     (1 <<15)
#define CPM_CLKGR1_LCD     (1 <<14)
#define CPM_CLKGR1_CIM     (1 <<13)
#define CPM_CLKGR1_TCSM    (1 <<11)
#define CPM_CLKGR1_SADC    (1 <<10)
#define CPM_CLKGR1_TCU1    (1 <<9)
#define CPM_CLKGR1_TCU0    (1 <<8)
#define CPM_CLKGR1_PWM     (1 <<6)
#define CPM_CLKGR1_CAN1    (1 <<5)
#define CPM_CLKGR1_CAN0    (1 <<4)
#define CPM_CLKGR1_HASH    (1 <<3)
#define CPM_CLKGR1_AES     (1 <<2)
#define CPM_CLKGR1_PDMA1   (1 <<1)
#define CPM_CLKGR1_PDMA    (1 <<0)

/*USBRDT*/
#define USBRDT_IDDIG_EN		(1 << 24)
#define USBRDT_IDDIG_REG        (1 << 23)

/* MSC EXTCLK enable BIT */
#define MSCCDR_EXCK_E           (1 << 21)
#define MSCCDR_MPCS             (30)
#define MSCCDR_MPCS_MASK        (3 << MSCCDR_MPCS)
#define MSCCDR_MPCS_EXCLK       (3 << MSCCDR_MPCS)


/* CPM scratch pad protected register(CPSPPR) */
#define RECOVERY_SIGNATURE      (0x1a1a)        /* means "RECY" */
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
