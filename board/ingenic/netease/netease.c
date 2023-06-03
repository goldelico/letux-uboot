/*
 * Ingenic dorado setup code
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
#include <asm/arch/nand.h>
#include <asm/arch/mmc.h>
#include <asm/arch/clk.h>

struct cgu_clk_src cgu_clk_src[] = {
	{VPU, MPLL},
	{OTG, EXCLK},
	{LCD, MPLL},
	{MSC, MPLL},
	{SSI, MPLL},
	{CIM, MPLL},
	{PCM, MPLL},
	{GPU, MPLL},
	{ISP, MPLL},
	{BCH, MPLL},
	{I2S, EXCLK},
	{SRC_EOF,SRC_EOF}
};

struct pmu_init {
	char * id;
	unsigned int value;
} init_value [] = {
	{ "RICOH619_DC1",   1100000},
	{ "RICOH619_DC2",   0},
	{ "RICOH619_DC3",   1800000},
	{ "RICOH619_DC4",   1800000},
	{ "RICOH619_DC5",   3300000},
	{ "RICOH619_LDO1",  0},
	{ "RICOH619_LDO2",  3300000},
	{ "RICOH619_LDO3",  3000000},
	{ "RICOH619_LDO4",  0},
	{ "RICOH619_LDO5",  2500000},
	{ "RICOH619_LDO6",  0},
	{ "RICOH619_LDO7",  0},
	{ "RICOH619_LDO8",  0},
	{ "RICOH619_LDO9",  0},
	{ "RICOH619_LDO10", 2800000},
};

int board_early_init_r(void)
{
	int i;
	for (i = 0; i < sizeof(init_value) / sizeof(struct pmu_init); i++) {
		struct regulator * lcd_regulator = regulator_get(init_value[i].id);
		if (lcd_regulator == NULL) {
			printf("Fail to enable %s - %d\n", init_value[i].id, init_value[i].value);
			continue;
		}

		if (init_value[i].value == 0) {
			regulator_disable(lcd_regulator);
		} else {
			regulator_set_voltage(lcd_regulator, init_value[i].value, init_value[i].value);
			regulator_enable(lcd_regulator);
		}
	}

	return 0;
}

#ifdef CONFIG_PMU_RICOH6x
extern int ricoh61x_regulator_init(void);
#endif

int board_early_init_f(void)
{
#ifdef CONFIG_PMU_RICOH6x
	ricoh61x_regulator_init();
#endif
	return 0;
}

#ifdef CONFIG_USB_GADGET
int jz_udc_probe(void);
void board_usb_init(void)
{
	jz_udc_probe();
}
#endif /* CONFIG_USB_GADGET */

#ifdef CONFIG_BOOT_ANDROID
extern void boot_mode_select(void);
#endif

int misc_init_r(void)
{
#ifdef CONFIG_BOOT_ANDROID
	boot_mode_select();
#endif

#if defined(CONFIG_CMD_BATTERYDET) && defined(CONFIG_BATTERY_INIT_GPIO)
	battery_init_gpio();
#endif
	return 0;
}

int board_nand_init(struct nand_chip *nand)
{
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
	return 0;
}


#ifdef CONFIG_SPL_NAND_SUPPORT
void nand_init(void)
{
}

int nand_spl_load_image(uint32_t offs, unsigned int size, void *dst)
{
	return 0;
}

void nand_deselect(void)
{
}
#endif

#ifdef CONFIG_SPL_NOR_SUPPORT
int spl_start_uboot(void)
{
	return 1;
}
#endif
/* U-Boot common routines */
int checkboard(void)
{
	puts("Board: netease (Ingenic XBurst M200 SoC)\n");
	return 0;
}

#ifdef CONFIG_SPL_BUILD

void spl_board_init(void)
{
}

#endif /* CONFIG_SPL_BUILD */
