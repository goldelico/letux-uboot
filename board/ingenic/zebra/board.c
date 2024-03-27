/*
 * Ingenic x2000 setup code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: cxtan <chenxi.tan@ingenic.cn>
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
#include <asm/arch/clk.h>
#include <asm/arch/mmc.h>

#ifdef CONFIG_BOOT_ANDROID
extern void boot_mode_select(void);
#endif

int board_early_init_f(void)
{
	return 0;
}

int board_early_init_r(void)
{

#ifdef CONFIG_REGULATOR
	regulator_init();
#endif
	return 0;

}

#ifdef CONFIG_REGULATOR
int regulator_init(void)
{
	int ret;
#ifdef CONFIG_PMU_RICOH6x
	ret = ricoh61x_regulator_init();
#endif
	return ret;
}
#endif /* CONFIG_REGULATOR */


int misc_init_r(void)
{
#ifdef CONFIG_BOOT_ANDROID
	boot_mode_select();
#endif
	return 0;
}



#ifdef CONFIG_MMC
extern void jz_mmc_init(void);
int board_mmc_init(bd_t *bd)
{
	jz_mmc_init();
	return 0;
}
#endif

#ifdef CONFIG_SYS_NAND_SELF_INIT
void board_nand_init(void)
{
	return;
}
#endif

int board_eth_init(bd_t *bis)
{
	int rv;
#ifndef  CONFIG_USB_ETHER
	/* reset grus DM9000 */
	rv = jz_net_initialize(bis);
#else
	rv = usb_eth_initialize(bis);
#endif
	return rv;
}

#ifdef CONFIG_SPL_NOR_SUPPORT
int spl_start_uboot(void)
{
	return 1;
}
#endif

/* U-Boot common routines */
int checkboard(void)
{
	puts("Board: zebra (Ingenic XBurst2 X2000-V12 SoC)\n");
	return 0;
}

#ifdef CONFIG_SPL_BUILD

void spl_board_init(void)
{
}

#endif /* CONFIG_SPL_BUILD */
