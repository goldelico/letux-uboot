/*
 * Ingenic dorado lcd code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Huddy <hyli@ingenic.cn>
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

#include <regulator.h>
#include <asm/gpio.h>
#include <jz_lcd/jz_lcd_v1_2.h>

void board_set_lcd_power_on(void)
{
}

void board_set_lcd_power_off(void)
{
}

void board_set_backlight_init(int num)
{
}

struct jzfb_config_info jzfb1_init_data = {
#if defined(CONFIG_VIDEO_VIRTUAL)
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 16,

	.pixclk_falling_edge = 1,
	.date_enable_active_low = 0,

	.dither_enable = 0,
#else
#error "Please add the board data!!!"
#endif
};
