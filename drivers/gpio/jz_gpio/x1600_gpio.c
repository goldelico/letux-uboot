/*
 * Copyright (c) 2015 Ingenic Semiconductor Co.,Ltd
 * Author: Matthew <xu.guo@ingenic.com>
 * Based on: newxboot/modules/gpio/jz4775_gpio.c|jz4780_gpio.c
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

static struct jz_gpio_func_def uart_gpio_func[] = {
	[0] = { .port = GPIO_PORT_B, .func = GPIO_FUNC_0 | GPIO_PULL, .pins = 0x3 << 7},
	[1] = { .port = GPIO_PORT_B, .func = GPIO_FUNC_1 | GPIO_PULL, .pins = 0x3 << 2},
#ifdef CONFIG_UART2_PA
	[2] = { .port = GPIO_PORT_A, .func = GPIO_FUNC_2, .pins = 0x3 << 30},
#else
	[2] = { .port = GPIO_PORT_B, .func = GPIO_FUNC_1 | GPIO_PULL, .pins = 0x3 << 0},
#endif
	[3] = { .port = GPIO_PORT_D, .func = GPIO_FUNC_2 | GPIO_PULL, .pins = 0x3 << 4},
};

static struct jz_gpio_func_def gpio_func[] = {
#if defined(CONFIG_JZ_MMC_MSC0_PC_4BIT)
	{ .port = GPIO_PORT_C, .func = GPIO_FUNC_1, .pins = 0x3f << 17},
#endif

#if defined(CONFIG_JZ_MMC_MSC1_PD)
	{ .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x3f << 0},
#endif

#if defined(CONFIG_SPL_SFC_SUPPORT)
	{ .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x3f << 17},
#if defined(CONFIG_JZ_SFC_CE_PB04)
	{ .port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0x1 << 4},
#endif
#if defined(CONFIG_JZ_SFC_CE_PB31)
	{ .port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x1 << 31},
#endif
#endif
#if defined(CONFIG_HALLEY6_MAC_POWER_EN)
	{ .port = GPIO_PORT_B, .func = GPIO_OUTPUT1, .pins = 0x1 << 30},
#endif
};
