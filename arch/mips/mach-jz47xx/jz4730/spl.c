// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4730 Secondary Program Load.
 *
 * Copyright (C) 2020 Lubomir Rintel <lkundrak@v3.sk>
 *
 * Based on code that's (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 */

#include <asm/io.h>
#include <asm/sections.h>
#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <init.h>
#include <linux/bitops.h>
#include <spl.h>

#define C0_CONFIG7_BTBV			BIT(1)

static void early_init(void)
{
	/*
	 * Enable and clear the branch target buffer
	 */
	write_c0_config7(read_c0_config7() | C0_CONFIG7_BTBV);

	/*
	 * Clear the BSS
	 */
	memset(__bss_start, 0, (char *)&__bss_end - __bss_start);
}

#define EMC_BASE			(void __iomem *)0xb3010000
#define EMC_BCR				0x00
#define EMC_DMCR			0x80
#define EMC_RTCSR			0x84
#define EMC_RTCNT			0x88
#define EMC_RTCOR			0x8c
#define EMC_DMAR1			0x90
#define EMC_DMAR2			0x94

#define EMC_BCR_BRE			BIT(1)

#define EMC_DMCR_BW_SHIFT		31
#define EMC_DMCR_CA_SHIFT		26
#define EMC_DMCR_RFSH			BIT(24)
#define EMC_DMCR_MRSET			BIT(23)
#define EMC_DMCR_RA_SHIFT		20
#define EMC_DMCR_BA_SHIFT		19
#define EMC_DMCR_EPIN			BIT(17)
#define EMC_DMCR_TRAS_SHIFT		13
#define EMC_DMCR_RCD_SHIFT		11
#define EMC_DMCR_TPC_SHIFT		8
#define EMC_DMCR_TRWL_SHIFT		5
#define EMC_DMCR_TRC_SHIFT		2
#define EMC_DMCR_TCL_SHIFT		0

#define EMC_RTCSR_CKS_SHIFT		0

#define EMC_SDMR_CAS_SHIFT		4
#define EMC_SDMR_BL_SHIFT		0

#define EMC_SDMR0			0xa000
#define EMC_SDMR1			0xb000

void sdram_init(void)
{
	register unsigned int dmcr, sdmode, tmp, ns;

	/*
	 * Enable SPLIT
	 */
	writel(EMC_BCR_BRE, EMC_BASE + EMC_BCR);

	writew(0 << EMC_RTCSR_CKS_SHIFT, EMC_BASE + EMC_RTCSR);
	writew(0, EMC_BASE + EMC_RTCOR);
	writew(0, EMC_BASE + EMC_RTCNT);

	/*
	 * Megabytes per bank
	 */
	tmp = 0x20f0;
	tmp |= CONFIG_JZ4730_SDRAM_BANK_SIZE / 2;
	writel(tmp, EMC_BASE + EMC_DMAR1);
	tmp |= (CONFIG_JZ4730_SDRAM_BANK_SIZE / 2) << 8;
	writel(tmp, EMC_BASE + EMC_DMAR2);

	/*
	 * Basic DMCR register value
	 */
	dmcr = (CONFIG_JZ4730_SDRAM_ROW - 11) << EMC_DMCR_RA_SHIFT;
	dmcr |= (CONFIG_JZ4730_SDRAM_ROW - 11) << EMC_DMCR_RA_SHIFT;
	dmcr |= (CONFIG_JZ4730_SDRAM_COL - 8) << EMC_DMCR_CA_SHIFT;
	dmcr |= (CONFIG_JZ4730_SDRAM_BANKS_PER_CHIP / 2 - 1) << EMC_DMCR_BA_SHIFT;
	dmcr |= (CONFIG_JZ4730_SDRAM_WIDTH == 32 ? 0 : 1) << EMC_DMCR_BW_SHIFT;
	dmcr |= EMC_DMCR_EPIN;
	dmcr |= (CONFIG_JZ4730_SDRAM_CAS - 1) << EMC_DMCR_TCL_SHIFT;

	/*
	 * SDRAM timimg parameters
	 */
	ns = 1000000000 / (CONFIG_CPU_FREQ_HZ / 3);
	tmp = CONFIG_JZ4730_SDRAM_TRAS / ns;
	if (tmp < 4)
		tmp = 4;
	if (tmp > 11)
		tmp = 11;
	dmcr |= (tmp - 4) << EMC_DMCR_TRAS_SHIFT;
	tmp = CONFIG_JZ4730_SDRAM_RCD / ns;
	if (tmp > 3)
		tmp = 3;
	dmcr |= tmp << EMC_DMCR_RCD_SHIFT;
	tmp = CONFIG_JZ4730_SDRAM_TPC / ns;
	if (tmp > 7)
		tmp = 7;
	dmcr |= tmp << EMC_DMCR_TPC_SHIFT;
	tmp = CONFIG_JZ4730_SDRAM_TRW / ns;
	if (tmp > 3)
		tmp = 3;
	dmcr |= tmp << EMC_DMCR_TRWL_SHIFT;
	tmp = (CONFIG_JZ4730_SDRAM_TRAS + CONFIG_JZ4730_SDRAM_TPC) / ns;
	if (tmp > 14)
		tmp = 14;
	dmcr |= ((tmp + 1) >> 1) << EMC_DMCR_TRC_SHIFT;

	/*
	 * SDRAM mode values
	 */
	sdmode = (generic_ffs(4) - 1) << EMC_SDMR_BL_SHIFT;
	sdmode |= CONFIG_JZ4730_SDRAM_CAS << EMC_SDMR_CAS_SHIFT;
	sdmode <<= (CONFIG_JZ4730_SDRAM_WIDTH == 32) ? 2 : 1;

	/*
	 * Precharge phase
	 */
	writel(dmcr, EMC_BASE + EMC_DMCR);

	/*
	 * Set refresh registers
	 */
	tmp = CONFIG_JZ4730_SDRAM_TREF / ns;
	tmp = tmp / 64 + 1;
	if (tmp > 0xff)
		tmp = 0xff;

	writew(tmp, EMC_BASE + EMC_RTCOR);

	/*
	 * CKO Divisor
	 */
	switch (CONFIG_JZ4730_SDRAM_CKS) {
	case 4:
		writew(1 << EMC_RTCSR_CKS_SHIFT, EMC_BASE + EMC_RTCSR);
		break;
	case 16:
		writew(2 << EMC_RTCSR_CKS_SHIFT, EMC_BASE + EMC_RTCSR);
		break;
	case 64:
		writew(3 << EMC_RTCSR_CKS_SHIFT, EMC_BASE + EMC_RTCSR);
		break;
	case 256:
		writew(4 << EMC_RTCSR_CKS_SHIFT, EMC_BASE + EMC_RTCSR);
		break;
	case 1024:
		writew(5 << EMC_RTCSR_CKS_SHIFT, EMC_BASE + EMC_RTCSR);
		break;
	case 2048:
		writew(6 << EMC_RTCSR_CKS_SHIFT, EMC_BASE + EMC_RTCSR);
		break;
	case 4096:
		writew(7 << EMC_RTCSR_CKS_SHIFT, EMC_BASE + EMC_RTCSR);
		break;
	}

	/*
	 * Precharge all chip-selects
	 */
	writeb(0, EMC_BASE + (EMC_SDMR0 | sdmode));
	writeb(0, EMC_BASE + (EMC_SDMR1 | sdmode));

	/*
	 * Wait for precharge, > 200us
	 */
	tmp = (CONFIG_CPU_FREQ_HZ / 1000000) * 200;
	while (tmp--)
		;

	/*
	 * Enable refresh and set SDRAM mode
	 */
	writel(dmcr | EMC_DMCR_RFSH | EMC_DMCR_MRSET, EMC_BASE + EMC_DMCR);

	/*
	 * Write sdram mode register for each chip-select
	 */
	writeb(0, EMC_BASE + (EMC_SDMR0 | sdmode));
	writeb(0, EMC_BASE + (EMC_SDMR1 | sdmode));
}

#define CPM_BASE			(void __iomem *)0xb0000000
#define CPM_CFCR			0x00
#define CPM_PLCR1			0x10

#define CPM_CFCR_CCLK_PLL_SHIFT		0
#define CPM_CFCR_HCLK_PLL_SHIFT		4
#define CPM_CFCR_PCLK_PLL_SHIFT		8
#define CPM_CFCR_LCD_PLL_SHIFT		12
#define CPM_CFCR_MCLK_PLL_SHIFT		16
#define CPM_CFCR_MCLK_COKEN1		BIT(22)
#define CPM_CFCR_UHC_IN_SHIFT		25

#define CPM_PLCR1_PLL_STABLE_TIME_SHIFT	0
#define CPM_PLCR1_PLL_ENABLE		BIT(8)
#define CPM_PLCR1_PLL_OD_SHIFT		16
#define CPM_PLCR1_PLL_N_SHIFT		18
#define CPM_PLCR1_PLL_M_SHIFT		23

static void clock_init(void)
{
	u32 val;

	/*
	 * Clock divisors
	 */
	val = CPM_CFCR_MCLK_COKEN1;
	val |= 0 << CPM_CFCR_CCLK_PLL_SHIFT;
	val |= 2 << CPM_CFCR_HCLK_PLL_SHIFT;
	val |= 2 << CPM_CFCR_PCLK_PLL_SHIFT;
	val |= 2 << CPM_CFCR_LCD_PLL_SHIFT;
	val |= 2 << CPM_CFCR_MCLK_PLL_SHIFT;
	val |= (CONFIG_CPU_FREQ_HZ / 48000000 - 1) << CPM_CFCR_UHC_IN_SHIFT;
	writel(val, CPM_BASE + CPM_CFCR);

	/*
	 * Main PLL
	 */
	val = (CONFIG_CPU_FREQ_HZ * 2 / CONFIG_SYS_CLK - 2) << CPM_PLCR1_PLL_M_SHIFT;
	val |= 0 << CPM_PLCR1_PLL_N_SHIFT;
	val |= 0 << CPM_PLCR1_PLL_OD_SHIFT;
	val |= 0x20 << CPM_PLCR1_PLL_STABLE_TIME_SHIFT;
	val |= CPM_PLCR1_PLL_ENABLE;
	writel(val, CPM_BASE + CPM_PLCR1);
}

#define PINCTRL_BASE	(void __iomem *)0xb0010000
#define PINCTRL_ALR(n)	(0x10 + (n) * 0x30)
#define PINCTRL_AUR(n)	(0x14 + (n) * 0x30)

static void pinmux_init(void)
{
	/*
	 * NAND Read Enable function on GP79
	 */
	clrbits_32(PINCTRL_BASE + PINCTRL_ALR(2), 0xc0000000);
	setbits_32(PINCTRL_BASE + PINCTRL_ALR(2), 0x40000000);

	/*
	 * NAND Write Enable on GP80, NAND Ready/Busy on GP81 and
	 * SDRAM CS1 on GP82
	 */
	clrbits_32(PINCTRL_BASE + PINCTRL_AUR(2), 0x0000003f);
	setbits_32(PINCTRL_BASE + PINCTRL_AUR(2), 0x00000015);

	/*
	 * UART0 RX/TX on GP126/GP127
	 */
	clrbits_32(PINCTRL_BASE + PINCTRL_AUR(3), 0xf0000000);
	setbits_32(PINCTRL_BASE + PINCTRL_AUR(3), 0x50000000);
}

void __noreturn board_init_f(ulong dummy)
{
	early_init();

	spl_init();

	pinmux_init();

	clock_init();

	sdram_init();

	board_init_r(NULL, 0);

	hang();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NAND;
}
