/*
 * Ingenic burner setup code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

#include <common.h>
#include <nand.h>
#include <net.h>
#include <netdev.h>
#include <asm/gpio.h>
#include <asm/arch/cpm.h>
#include <asm/arch/mmc.h>
#include <asm/jz_uart.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;
struct global_info ginfo __attribute__ ((section(".data")));
extern struct jz_uart *uart;

extern void burner_param_info(void);
extern int jz_udc_probe(void);
extern void jz_mmc_init(void);

struct cgu_clk_src cgu_clk_src[] = {
	{MSC,	MPLL},
	{MSC1,	MPLL},
	{SFC0,	MPLL},
	{SFC1,	MPLL},
	{SRC_EOF,SRC_EOF}
};

int board_early_init_f(void)
{
	burner_param_info();
	uart = (struct jz_uart *)(UART0_BASE + gd->arch.gi->uart_idx * 0x1000);
	return 0;
}

int misc_init_r(void)
{
       return 0;
}

void board_usb_init(void)
{
	jz_udc_probe();
}

int board_mmc_init(bd_t *bd)
{
	jz_mmc_init();
	return 0;
}

#ifdef CONFIG_SYS_NAND_SELF_INIT
void board_nand_init(void)
{
}
#endif

int checkboard(void)
{
	return 0;
}

