/*
 * JZ4775 common routines
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


//#define DEBUG
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpm.h>
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
extern void validate_cache(void);
extern void ddr_test_refresh(unsigned int start_addr, unsigned int end_addr);
extern void gpio_set_driver_strength(enum gpio_port gpio, int value, unsigned int pins);

extern int debug_icache_store(unsigned int start, unsigned int len);
extern int dump_icache_tag(unsigned int start, unsigned int len);
extern int debug_cache_2(void);
extern void dump_c0_regs(void);

#ifdef CONFIG_SPL_USB_BOOT
extern int spl_usb_boot;
#endif

void change_lcd_ddrc_process_priority(void)
{
	*(unsigned int *)0x13012024 = 0xFF404030;   //DDRC-APB-CCHC0
	*(unsigned int *)0x13012028 = 0xFF404030;   //DDRC-APB-CCHC1
}

void gpio_set_driver_strength_init(void)
{
#if 0
	/* set sfc pd06-pd11 driver strength as GPIO_DS_LEVEL_1*/
	gpio_set_driver_strength(GPIO_PORT_D, GPIO_DS_LEVEL_1, 0x3f <<6);
#endif
}

void board_init_f(ulong dummy)
{
	*(volatile unsigned int *)0xb363002c |= 1 << 16; // wdt disable.
	*(volatile unsigned int *)0xb3630004 &= ~(1 << 0); // wdt disable.

	/* Set global data pointer */
	gd = &gdata;


	/* Setup global info */
#ifndef CONFIG_BURNER
	gd->arch.gi = &ginfo;
#else
	burner_param_info();

#ifdef CONFIG_SPL_USB_BOOT
	if (!!spl_usb_boot) {
		timer_init();
		usb_boot_loop();
		return;
	}
#endif
#endif

	gpio_init();
	gpio_set_driver_strength_init();

	/* Init uart first */
	enable_uart_clk();

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#endif

	serial_debug("ERROR EPC %x\n", read_c0_errorepc());
	serial_debug("Reset status %x\n", *(volatile unsigned int *)0xb0000008);
	//dump_c0_regs();

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

	debug("SDRAM init\n");
	sdram_init();

	change_lcd_ddrc_process_priority();

#ifdef CONFIG_DDR_AUTO_REFRESH_TEST
	ddr_test_refresh(0xa0000000, 0xa1000000);
#endif

#ifdef CONFIG_DDR_TEST
	ddr_basic_tests();
#endif

#ifdef CONFIG_RUN_FIRMWARE_VIA_USB_BOOT
       serial_debug("run start1 firmware finished, return to bootrom!\n");
       usb_boot_loop();	// USB Stage2 protocal.
       return;
#endif

#ifndef CONFIG_BURNER
	/* Clear the BSS */
	memset(__bss_start, 0, (char *)&__bss_end - __bss_start);

	debug("board_init_r\n");
	board_init_r(NULL, 0);
#else
	debug("run start1 firmware finished\n");
#endif
}

extern void flush_cache_all(void);

#ifdef CONFIG_JZ_SECURE_SUPPORT
extern int secure_scboot (void *, void *);
static int secure_load_uboot(struct spl_image_info *spl_image)
{
	int ret = secure_scboot ((void *)spl_image->load_addr,
				 (void *)spl_image->entry_point);
	return ret;
}
#endif

void jump_to_entry_point(unsigned long entry_point)
{
	flush_cache_all();
	__asm__ volatile (
			".set push              \n\t"
			".set noreorder         \n\t"
			".set mips32r2          \n\t"
			"jr.hb %0              \n\t"
			"nop	\n\t"
			:
			:"r"(entry_point));
}

void jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void  (*image_entry_noargs_t)(void);

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

	jump_to_entry_point(image_entry);
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
