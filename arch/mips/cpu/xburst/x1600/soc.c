/*
 * T15G common routines
 *
 * Copyright (c) 2015 Ingenic Semiconductor Co.,Ltd
 * Author: Elvis <huan.wang@ingenic.com>
 * Based on: arch/mips/cpu/xburst/jz4775/jz4775.c
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

/* #define DEBUG */
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpm.h>
#include <spl.h>
#include <asm/mipsregs.h>

//#define CONFIG_SIMULATION
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

extern void pll_init(void);
extern void sdram_init(void);
extern void validate_cache(void);

#ifdef CONFIG_SIMULATION
volatile noinline void hello_word(void)
{
	while(1);
}
#endif

void release_soft_reset(void)
{
	cpm_outl(cpm_inl(CPM_CLKGR0) | CPM_CLKGR_OTG, CPM_CLKGR0);
	cpm_outl(cpm_inl(CPM_SRBC) & (~CPM_SLBC_OTG_SR), CPM_SRBC);
	cpm_outl(cpm_inl(CPM_CLKGR0) & (~CPM_CLKGR_OTG), CPM_CLKGR0);
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

	*(volatile unsigned int *)0xb000202c |= 1 << 16; // wdt disable.
	*(volatile unsigned int *)0xb0002004 &= ~(1 << 0); // wdt disable.

#ifndef CONFIG_FPGA
	/* Init uart first */
	enable_uart_clk();
#endif

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#endif
	printf("ERROR EPC %x\n", read_c0_errorepc());

#ifndef CONFIG_FPGA
	debug("Timer init\n");
	timer_init();

#ifdef CONFIG_SPL_REGULATOR_SUPPORT
	debug("regulator set\n");
	spl_regulator_set();
#endif

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
	debug("SDRAM init ok\n");

#ifdef CONFIG_SIMULATION
	{
		hello_word();
	}
#endif
#ifdef CONFIG_DDR_TEST
	ddr_basic_tests();
#endif

	/* Release otg soft reset. */
	release_soft_reset();

#ifndef CONFIG_BURNER
	/* Clear the BSS */
	memset(__bss_start, 0, (char *)&__bss_end - __bss_start);
#ifdef CONFIG_PALLADIUM
	{
		debug("Going to palladium kernel......\n");
		void (*kernel)(void);
		kernel = (void (*)(void))0x80010000;
		(*kernel)();
	}
#endif
	debug("board_init_r\n");
	board_init_r(NULL, 0);
#else
	debug("run firmware finished\n");
	return ;
#endif
}

extern void flush_cache_all(void);

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);

	image_entry_noargs_t image_entry =
			(image_entry_noargs_t) spl_image->entry_point;

	flush_cache_all();

	debug("image entry point: 0x%X\n", spl_image->entry_point);

	image_entry();
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
