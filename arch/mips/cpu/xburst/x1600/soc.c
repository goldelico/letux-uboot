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
#ifdef CONFIG_GPIO_SPI_TO_UART
extern int gpio_spi_to_uart_init(void);
#endif

#ifdef CONFIG_SPL_USB_BOOT
extern int spl_usb_boot;
#endif

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
	*(volatile unsigned int *)0xb000202c |= 1 << 16; // wdt disable.
	*(volatile unsigned int *)0xb0002004 &= ~(1 << 0); // wdt disable.

	/* Set global data pointer */
	gd = &gdata;

	/* Setup global info */
#ifndef CONFIG_BURNER
	gd->arch.gi = &ginfo;
#else
	burner_param_info();

#ifdef CONFIG_SPL_USB_BOOT
	if (!!spl_usb_boot) {
		usb_boot_loop();
		return;
	}
#endif
#endif

	gpio_init();

#ifndef CONFIG_FPGA
	/* Init uart first */
	enable_uart_clk();
#endif

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#endif

#ifdef CONFIG_GPIO_SPI_TO_UART
	gpio_spi_to_uart_init();
#endif
#ifndef CONFIG_DDR_DRVODT_DEBUG
	serial_debug("ERROR EPC %x\n", read_c0_errorepc());
#endif
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

#ifdef CONFIG_HW_WATCHDOG
	debug("WATCHDOG init\n");
	hw_watchdog_init();
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

extern void flush_cache_all(void);

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);

#ifdef CONFIG_JZ_SECURE_SUPPORT
	flush_cache_all();
	int ret = secure_load_uboot(spl_image);
	if (ret) {
	  serial_debug("Error spl secure load uboot.\n");
	  hang();
	}
	spl_image->entry_point += 2048;
#endif

	debug("image entry point: 0x%x\n", spl_image->entry_point);
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
