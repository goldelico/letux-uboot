/*
 * X2000 common routines
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 * Based on: arch/mips/cpu/xburst/jz4780/jz4780.c
 *           Written by Paul Burton <paul.burton@imgtec.com>
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

#define DEBUG
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpm.h>
#include <asm/arch/wdt.h>
#include <spl.h>
#include <regulator.h>
#ifdef CONFIG_AUDIO_CAL_DIV
#include <generated/audio.h>
#endif
#ifdef CONFIG_SPL_BUILD

/* Pointer to as well as the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;
gd_t gdata __attribute__ ((section(".data")));

#ifndef CONFIG_BURNER
struct global_info ginfo __attribute__ ((section(".data"))) = {
	.extal		= CONFIG_SYS_EXTAL,
	.cpufreq	= CONFIG_SYS_CPU_FREQ,
	.ddrfreq	= CONFIG_SYS_MEM_FREQ,
	.uart_idx	= CONFIG_SYS_UART_INDEX,
	.baud_rate	= CONFIG_BAUDRATE,

};
#endif

extern void gpio_init(void);
extern void pll_init(void);
extern void sdram_init(void);
extern void ddr_test_refresh(unsigned int start_addr, unsigned int end_addr);
extern void flush_cache_all(void);

void release_soft_reset(void)
{
	cpm_outl(cpm_inl(CPM_CLKGR0) | CPM_CLKGR_OTG, CPM_CLKGR0);
	cpm_outl(cpm_inl(CPM_SRBC) & (~CPM_SLBC_OTG_SR), CPM_SRBC);
	cpm_outl(cpm_inl(CPM_CLKGR0) & (~CPM_CLKGR_OTG), CPM_CLKGR0);
}

void reallocate_cache(void)
{
	flush_cache_all();

	/* allcate L2 cache size */
	/***********************************
	  L2 cache size
	  reg addr: 0x12200060
	  bit   12 11 10
	  0   0  0   L2C=0KB
	  0   0  1   L2C=128KB
	  0   1  0   L2C=256KB
	  0   1  1   L2C=512KB
	  1   0  0   L2C=1024KB
	 ***********************************/
#if 1
	/* wait l2cache alloc ok */
	__asm__ volatile(
			".set push     \n\t"
			".set mips32r2 \n\t"
			"sync          \n\t"
			"lw $0,0(%0)   \n\t"
			".set pop      \n\t"
			::"r" (0xa0000000));
	*((volatile unsigned int *)(0xb2200060)) = 0x00000400;
	__asm__ volatile(
			".set push     \n\t"
			".set mips32r2 \n\t"
			"sync          \n\t"
			"lw $0,0(%0)   \n\t"
			".set pop      \n\t"
			::"r" (0xa0000000));
#endif

}

/*Improving the priority of ddrc in lcd processing*/
void change_lcd_ddrc_process_priority(void)
{
	*(unsigned int *)0x1301202c = 0x8840403e;   //DDRC-APB-CCHC3
	*(unsigned int *)0x13012038 = 0xff0000cd;   //DDRC-APB-CCHC6
}

void board_init_f(ulong dummy)
{
	/* Set global data pointer */
	gd = &gdata;

	/* Setup global info */
#ifndef CONFIG_BURNER
	gd->arch.gi = &ginfo;
#else
	burner_param_info();
#endif

	gpio_init();
#if 1
	/* OST clk gate set 0 */
	cpm_outl(cpm_inl(CPM_CLKGR1) & (~CPM_CLKGR1_SYS_OST), CPM_CLKGR1);
	/* wtd disable */
	writel(0, WDT_BASE + WDT_TCER);
	cpm_outl(cpm_inl(CPM_MESTSEL) | 0x7, CPM_MESTSEL);
#endif

	/* Init uart first */
#ifndef CONFIG_X2000_FPGA
	enable_uart_clk();
#endif

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
	printf("ERROR EPC %x\n", read_c0_errorepc());
#endif
#ifndef CONFIG_X2000_FPGA
	debug("Timer init\n");
	timer_init();

#ifndef CONFIG_BURNER
	debug("CLK stop\n");
	clk_prepare();
#endif

	debug("PLL init\n");
	pll_init();

	debug("CLK init\n");
	clk_init();
#endif

	debug("SDRAM init\n");
	sdram_init();

	change_lcd_ddrc_process_priority();

#ifdef CONFIG_DDR_AUTO_REFRESH_TEST
	ddr_test_refresh(0xa0000000, 0xa1000000);
#endif


#ifdef CONFIG_DDR_TEST
	ddr_basic_tests();
#endif

	reallocate_cache();

	/* Release otg soft reset. */
	release_soft_reset();

#ifndef CONFIG_BURNER
	/* Clear the BSS */
	memset(__bss_start, 0, (char *)&__bss_end - __bss_start);

	debug("board_init_r\n");
	board_init_r(NULL, 0);
#else
	debug("run start1 firmware finished\n");
	return;
#endif
}

#ifdef CONFIG_JZ_SECURE_SUPPORT
extern int secure_scboot (void *, void *);
static int secure_load_uboot(struct spl_image_info *spl_image)
{
	int ret = secure_scboot ((void *)spl_image->load_addr,
				 (void *)spl_image->entry_point);
	return ret;
}
#endif

void jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void  (*image_entry_noargs_t)(void);

#ifdef CONFIG_JZ_SECURE_SUPPORT
	flush_cache_all();
	int ret = secure_load_uboot(spl_image);
	if (ret) {
	  printf("Error spl secure load uboot.\n");
	  hang();
	}
	spl_image->entry_point += 2048;
#endif

	debug("image entry point: 0x%x\n", spl_image->entry_point);
	image_entry_noargs_t image_entry =
			(image_entry_noargs_t) spl_image->entry_point;


	flush_cache_all();
#if 0
	{
		int i;
		for (i = 0; i < 512; i = i + 4) {
		//	printf("0x%x : 0x%x\n", 0x80100000 + i, *(volatile unsigned int *)(0x80100000 + i));
			printf("0x%x : 0x%x\n", 0xa0100000 + i, *(volatile unsigned int *)(0xa0100000 + i));
		}
	}
#endif
	__asm__ volatile (
			".set push              \n\t"
			".set noreorder         \n\t"
			".set mips32r2          \n\t"
			"jr.hb %0              \n\t"
			"nop	\n\t"
			:
			:"r"(image_entry));

}

#endif /* CONFIG_SPL_BUILD */

/*
 * U-Boot common functions
 */

void enable_interrupts(void)
{
}

int disable_interrupts(void)
{
	return 0;
}
