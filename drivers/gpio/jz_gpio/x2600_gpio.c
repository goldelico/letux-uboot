/*
 * Ingenic X2600 GPIO definitions
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
	[0] = { .port = GPIO_PORT_E, .func = GPIO_FUNC_1 | GPIO_PULLUP, .pins = 0xf << 9},
#ifdef CONFIG_SYS_UART1_PC
	[1] = { .port = GPIO_PORT_C, .func = GPIO_FUNC_2 | GPIO_PULLUP, .pins = 0x3 << 2},
#else/* CONFIG_SYS_UART1_PB */
	[1] = { .port = GPIO_PORT_B, .func = GPIO_FUNC_1 | GPIO_PULLUP, .pins = 0xf << 0},
#endif

	[2] = {.port = GPIO_PORT_D,  .func = GPIO_FUNC_0 | GPIO_PULLUP, .pins = 0x3 << 12},
#ifdef CONFIG_SYS_UART3_PC
	[3] = {.port = GPIO_PORT_C,  .func = GPIO_FUNC_3 | GPIO_PULLUP, .pins = 0x3 << 15},
#else/* CONFIG_SYS_UART3_PB */
	[3] = {.port = GPIO_PORT_B,  .func = GPIO_FUNC_1 | GPIO_PULLUP, .pins = 0x3 << 22},
#endif

#ifdef CONFIG_SYS_UART4_PC
	[4] = {.port = GPIO_PORT_C,  .func = GPIO_FUNC_3 | GPIO_PULLUP, .pins = 0x3 << 17},
#else/* CONFIG_SYS_UART4_PB */
	[4] = {.port = GPIO_PORT_B,  .func = GPIO_FUNC_1 | GPIO_PULLUP, .pins = 0x3 << 24},
#endif

#ifdef CONFIG_SYS_UART5_PC
	[5] = {.port = GPIO_PORT_C,  .func = GPIO_FUNC_3 | GPIO_PULLUP, .pins = 0x3 << 7},
#else/* CONFIG_SYS_UART5_PB */
	[5] = {.port = GPIO_PORT_B,  .func = GPIO_FUNC_1 | GPIO_PULLUP, .pins = 0x3 << 26},
#endif
	[6] = {.port = GPIO_PORT_C,  .func = GPIO_FUNC_2 | GPIO_PULLUP, .pins = 0x3 << 0},
};

static struct jz_gpio_func_def gpio_func[] = {
#if defined(CONFIG_JZ_MMC_MSC0_PD)
	{ .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 0x3f << 0},
#endif

#if defined(CONFIG_JZ_MMC_MSC0_PE)
	{ .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0xf << 1},
#endif

#if defined(CONFIG_JZ_MMC_MSC1_PD)
	{ .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0x3f << 6},
#elif defined(CONFIG_JZ_MMC_MSC1_PC)
	{ .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x3f << 25},
#endif

#if defined(CONFIG_JZ_MMC_MSC2_PE)
	{ .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 0x3f << 0},
#endif

#ifdef CONFIG_JZ_SFC_PD
//	{ .port = GPIO_PORT_D, .func = GPIO_FUNC_1 | GPIO_PULL, .pins = 0x3ff <<17, },
//	{ .port = GPIO_PORT_D, .func = GPIO_FUNC_1 , .pins = 0x3ff <<17, },
#ifdef CONFIG_JZ_SFC_PD0
	{ .port = GPIO_PORT_D, .func = GPIO_FUNC_1 , .pins = 0x3f},
#else 
	{ .port = GPIO_PORT_D, .func = GPIO_FUNC_0 , .pins = 0x3f <<6},
#endif 
#endif

#ifdef CONFIG_LCD
/* DPU_TFT_18BIT */
	{.port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x0fffffff}
/* DPU_SLCD_8BIT */
//	{.port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0x0e0000ff}

/* DPU_SLCD_TE_ONLY */
//	{.port = GPIO_PORT_B, .func = GPIO_FUNC_2, .pins = 0x08000000}

/* DPU_SLCD_SPI */
//	{.port = GPIO_PORT_C, .func = GPIO_FUNC_2, .pins = 0x18000001}

#endif
};
