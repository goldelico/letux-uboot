/*
 * X2000 GPIO definitions
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

static struct jz_gpio_func_def uart_gpio_func[] = {
	[0] = {.port = GPIO_PORT_D, .func = GPIO_FUNC_2 | GPIO_PULL, .pins = 0x3 << 23},
	[1] = {.port = GPIO_PORT_C, .func = GPIO_FUNC_1 | GPIO_PULL, .pins = 0x3 << 23},
	[2] = {.port = GPIO_PORT_D, .func = GPIO_FUNC_0 | GPIO_PULL, .pins = 0x3 << 30},
	[3] = {.port = GPIO_PORT_C, .func = GPIO_FUNC_0 | GPIO_PULL, .pins = 0x3 << 25},
};

static struct jz_gpio_func_def gpio_func[] = {
#if defined(CONFIG_JZ_MMC_MSC0_PD)
	{ .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x3ff << 17},
#endif

#if defined(CONFIG_JZ_MMC_MSC1_PD)
	{ .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x3f << 8},
#endif

#if defined(CONFIG_JZ_MMC_MSC2_PE)
	{ .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x3f << 0},
#endif

#ifdef CONFIG_JZ_SFC_PD_4BIT
	{ .port = GPIO_PORT_D, .func = GPIO_FUNC_1 , .pins = 0x3f << 17, },
#endif

#ifdef CONFIG_JZ_SFC_PD_8BIT
	{ .port = GPIO_PORT_D, .func = GPIO_FUNC_1 , .pins = 0x3ff << 17, },
#endif

#ifdef CONFIG_JZ_SFC_PD_8BIT_PULL
	{ .port = GPIO_PORT_D, .func = GPIO_FUNC_1 | GPIO_PULL, .pins = 0x3ff << 17, },
#endif

#ifdef CONFIG_JZ_PMU_SLP_OUTPUT1
	{ .port = GPIO_PORT_C, .func = GPIO_OUTPUT1, .pins = 0x1 <<28, },
#endif

#ifdef CONFIG_JZ_SFC_PE
	{ .port = GPIO_PORT_E, .func = GPIO_FUNC_0 | GPIO_PULL, .pins = 0x3f << 16, },
#endif
};
