/*
 * X2580 common routines
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

#define DEBUG
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpm.h>
#include <asm/arch/wdt.h>
#include <asm/jz_cache.h>
#include <spl.h>



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
extern void sdram_init(int pll_sel);
extern void gpio_set_driver_strength(enum gpio_port gpio, int value, unsigned int pins);

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
}

static inline void zboost_is_run()
{
    __asm__ volatile (
        ".set push     \n\t"
        ".set mips32r2 \n\t"
        "sync \n\t"
        "nop \n\t"
        "li      $2, 0xb0000008 \n\t"
        "lw      $3, 0($2) \n\t"
        "andi    $4, $3, 0x2 \n\t"
        "bne     $4, 0, _start_boot \n\t"
        "nop \n\t"
        "andi    $4, $3, (0x1<<4) \n\t"            /* FASTBOOT and HR(hibernate reset) */
        "beq     $4, (0x1<<4), _fastboot \n\t"     /*if only fastboot !=0*/
        "nop \n\t"
        "bne     $4, (0x1<<4), _start_boot \n\t"  /*if WR && PR && FASTBOOT = 0*/
        "nop \n\t"
        "_fastboot: \n\t"
        "li      $4, 0xb0004000 \n\t"          /* RTC Reserved Memory 4KBytes. */
        "jalr.hb $4 \n\t"
        "nop \n\t"
        "_start_boot:\n\t"
        "nop \n\t"
		"lw $0,0(%0)   \n\t"
        ".set pop      \n\t"
		::"r" (0xa0000000));
}

void gpio_set_driver_strength_init(void)
{
#if 0
	/* set sfc PA23~28 driver strength as GPIO_DS_LEVEL_1*/
	gpio_set_driver_strength(GPIO_PORT_A, GPIO_DS_LEVEL_1, 0x3f << 23);
#endif
}

static int ddr_mem_test12(void)
{
	int i = 0;
	int j = 0;
	int ret = 8;
	unsigned int addr;
	unsigned int value_get;
	unsigned int data;
	unsigned int test_size =  64;
	unsigned int test_data =  0xffffffff;
	//serial_debug("mem test12 start!\n");
	for(i = 0; i < 1; i++) {


		addr = 0x81000000;
		for (j = 0; j < test_size; j+=4) {
			data = (j/4)%2?test_data:0;
			*(volatile unsigned int *)(addr+j) = data;
		}

		//flush cache
		flush_dcache_range(addr, addr+test_size);
		flush_scache_range(addr, addr+test_size);

		//read cached
		addr = 0x81000000;
		for (j = 0; j < test_size; j+=4) {
			value_get = *(volatile unsigned int *)(addr+j);
		}
#if 0
		//write uncached
		addr = 0xa1000000;
		for (j = 0; j < test_size; j+=4) {
			data = (j/4)%2?test_data:0;
			*(volatile unsigned int *)(addr+j) = data;
		}
#endif
		//invalid cache
		addr = 0x81000000;
		invalid_dcache_range(addr, addr+test_size);
		invalid_scache_range(addr, addr+test_size);
		//read cached
		addr = 0x81000000;
		serial_debug("mem data:\n");
		for (j = 0; j < test_size; j+=4) {
			value_get = *(volatile unsigned int *)(addr+j);
			if(j/4%4%2)
				if(test_data == value_get)
					ret -= 1;
			if (0 == j/4%4)
				serial_debug("%x: ", addr+j);
			serial_debug("  %x", value_get);
			if (3 == j/4%4)
				serial_debug("\n");
		}
	}
	//serial_debug("mem test12 end!\n");
	return ret;
}

static int ddr_mem_test4(void)
{
	volatile u32 tmp = 0;
	u32 tmp_data = 0;

#ifdef CONFIG_DDR_TYPE_DDR2
	for (tmp = 0xa0000000; tmp < 0xa4000000; tmp+=0x104) {
		u32 td = 0x5a5a5a5a;
		*(u32*)tmp = td;
		tmp_data = *(u32*)tmp;
		if (tmp_data != td) {
			serial_debug("\n##### addr = %p, want = %x, get = %x\n", tmp, td, tmp_data);
			return -1;
		}
	}
#else
	for (tmp = 0xa0000000; tmp < 0xa8000000; tmp+=0x104) {
		u32 td = 0x5a5a5a5a;
		*(u32*)tmp = td;
		tmp_data = *(u32*)tmp;
		if (tmp_data != td) {
			serial_debug("\n##### addr = %p, want = %x, get = %x\n", tmp, td, tmp_data);
			return -1;
		}
	}
#endif
	return 0;
}

void board_init_f(ulong dummy)
{
	unsigned int ccu_value = 0;
	int pll_sel = 0;

	zboost_is_run();

	/* Set global data pointer */
	gd = &gdata;

    /* close ccu IFU simple Loop */
	ccu_value = *((volatile unsigned int *)(0xb2200fe0));
	*((volatile unsigned int *)(0xb2200fe0)) = (ccu_value | 0x00000078);

	/* Setup global info */
#ifndef CONFIG_BURNER
	gd->arch.gi = &ginfo;
#else
	burner_param_info();
#endif

	gpio_init();
	gpio_set_driver_strength_init();
#if 1
	/* OST clk gate set 0 */
	cpm_outl(cpm_inl(CPM_CLKGR1) & (~CPM_CLKGR1_SYS_OST), CPM_CLKGR1);
	/* wtd disable */
	writel(0, WDT_BASE + WDT_TCER);
	cpm_outl(cpm_inl(CPM_MESTSEL) | 0x7, CPM_MESTSEL);
#endif

	/* Init uart first */
#ifndef CONFIG_X2580_FPGA
	enable_uart_clk();
#endif

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
	serial_debug("ERROR EPC %x\n", read_c0_errorepc());
#endif
#ifndef CONFIG_X2580_FPGA
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

	sdram_init(0);

	while(ddr_mem_test4())
	{
		pll_sel++;
		sdram_init(pll_sel);
	}

	serial_debug("SDRAM init\n");
#ifdef CONFIG_DDR_AUTO_REFRESH_TEST
	ddr_test_refresh(0xa0000000, 0xa1000000);
#endif

#ifdef CONFIG_DDR_TEST
	ddr_basic_tests();
#endif

	reallocate_cache();

	if(ddr_mem_test12())
	{
		serial_debug("ddr error!\n");
		do_reset(NULL, 0, 0, NULL);
	}

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

extern void flush_cache_all(void);

void jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void  (*image_entry_noargs_t)(void);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t) spl_image->entry_point;

	flush_cache_all();

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
