/* u-boot driver for the GTA04 backlight
 *
 * Copyright (C) 2010 by Golden Delicious Computers GmbH&Co. KG
 * Author: H. Nikolaus Schaller <hns@goldelico.com>
 * All rights reserved.
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
 *
 */

#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include "backlight.h"

#ifdef CONFIG_OMAP3_GTA04
#define GPIO_BACKLIGHT		57	/* = GPT11_PWM */
#define GPT_BACKLIGHT		OMAP34XX_GPT11
#else /* Beagle Hybrid */
#define GPIO_BACKLIGHT		145
#define GPT_BACKLIGHT		OMAP34XX_GPT10
#endif

#define USE_PWM	0

void backlight_set_level(int level)	// 0..255
{
#if USE_PWM
	struct gptimer *gpt_base = (struct gptimer *)GPT_BACKLIGHT;
	// 	writel(value, &gpt_base->registername);
#else
	omap_set_gpio_dataout(GPIO_BACKLIGHT, level >= 128);	// for simplicity we just have on/off
	level=(level >= 128)?255:0;
#endif
	printf("lcm backlight level set to %d (0..255)\n", level);
}

int backlight_init(void)
{
#if USE_PWM
	struct gptimer *gpt_base = (struct gptimer *)GPT_BACKLIGHT;
	MUX_VAL(CP(UART2_RTS),		(IEN  | PTD | DIS | M2)) /* switch to GPT10 */
	// 	writel(value, &gpt_base->registername);
	// program registers
#error todo
#else
#ifndef CONFIG_OMAP3_GTA04
	MUX_VAL(CP(UART2_RTS),		(IEN  | PTD | DIS | M4)) /*GPIO_145*/
#endif
	omap_request_gpio(GPIO_BACKLIGHT);
	omap_set_gpio_direction(GPIO_BACKLIGHT, 0);		// output

	//	omap_free_gpio(GPIO_BACKLIGHT);
#endif
	printf("did backlight_init()\n");

	return 0;
}


