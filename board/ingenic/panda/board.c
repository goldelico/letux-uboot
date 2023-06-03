/*
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Matthew <xu.guo@ingenic.com>
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
#include <asm/arch/nand.h>
#include <asm/arch/mmc.h>
#include <asm/arch/clk.h>

extern int jz_net_initialize(bd_t *bis);
#ifdef CONFIG_BOOT_ANDROID
extern void boot_mode_select(void);
#endif

int board_early_init_f(void)
{
	return 0;
}

#ifdef CONFIG_REGULATOR
int regulator_init(void)
{
	return 0;
}
#endif /* CONFIG_REGULATOR */

int board_early_init_r(void)
{
#ifdef CONFIG_REGULATOR
	regulator_init();
#endif
	return 0;
}

#ifdef CONFIG_USB_GADGET
int jz_udc_probe(void);
void board_usb_init(void)
{
	printf("USB_udc_probe\n");
	jz_udc_probe();
}
#endif /* CONFIG_USB_GADGET */

int misc_init_r(void)
{
#if 0 /* TO DO */
	uint8_t mac[6] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc };

	/* set MAC address */
	eth_setenv_enetaddr("ethaddr", mac);
#endif
#ifdef CONFIG_BOOT_ANDROID
	boot_mode_select();
#endif

#if defined(CONFIG_CMD_BATTERYDET) && defined(CONFIG_BATTERY_INIT_GPIO)
	battery_init_gpio();
#endif
	return 0;
}

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bd)
{
	jz_mmc_init();
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_NET_X1600
	int rv;
#ifndef  CONFIG_USB_ETHER
	/* reset grus DM9000 */
	rv = jz_net_initialize(bis);
#else
	rv = usb_eth_initialize(bis);
#endif
	return rv;
#endif
}

#ifdef CONFIG_SYS_NAND_SELF_INIT
void board_nand_init(void)
{
	return 0;
}
#endif

/* U-Boot common routines */
int checkboard(void)
{
	puts("Board: fpga-X1600 (Ingenic XBurst X1600  SoC)\n");
	return 0;
}
#ifdef CONFIG_SPL_NOR_SUPPORT
int spl_start_uboot(void)
{
	return 1;
}
#endif

#ifdef CONFIG_SPL_BUILD

void spl_board_init(void)
{
}

#endif /* CONFIG_SPL_BUILD */

#if (CONFIG_BOOTARGS_AUTO_MODIFY == 1)
unsigned char linux_argp[200];
static char* linux_cmdline_set(char *arg, const char *value, size_t len)
{
	memset(linux_argp, 0, sizeof(linux_argp));
	memcpy(linux_argp, arg, strlen(arg));
	strcat(linux_argp, " ");
	strcat(linux_argp, value);

	return linux_argp;
}


static char *board_process_mem_arg(char *arg)
{
	unsigned long ram_size = initdram(0) >> 20;

	if(ram_size == 32){
#ifdef CONFIG_BOOTARGS_MEM_32M
		arg = linux_cmdline_set(arg, CONFIG_BOOTARGS_MEM_32M, strlen(CONFIG_BOOTARGS_MEM_32M));
#endif
	}else if(ram_size == 64) {
#ifdef CONFIG_BOOTARGS_MEM_64M
		arg = linux_cmdline_set(arg, CONFIG_BOOTARGS_MEM_64M, strlen(CONFIG_BOOTARGS_MEM_64M));
#endif
	} else if(ram_size == 128) {
#ifdef CONFIG_BOOTARGS_MEM_128M
		arg = linux_cmdline_set(arg, CONFIG_BOOTARGS_MEM_128M, strlen(CONFIG_BOOTARGS_MEM_128M));
#endif
	}

	return arg;

}
#endif


char *spl_board_process_bootargs(char *arg)
{
#if (CONFIG_BOOTARGS_AUTO_MODIFY == 1)
	arg = board_process_mem_arg(arg);
#endif

	return arg;
}

