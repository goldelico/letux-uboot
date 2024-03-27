/*
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Huddy <shaobo.zhang@ingenic.cn>
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
#include <jz_lcd/jz_lcd_v14.h>
#include <linux/fb.h>
#include <jz_lcd/lcd_y88249.h>

struct jzfb_tft_config y88249_cfg = {
    .pix_clk_inv            = 0,
    .de_dl                  = 0,
    .sync_dl                = 0,
    .color_even             = TFT_LCD_COLOR_EVEN_RGB,
    .color_odd              = TFT_LCD_COLOR_ODD_RGB,
    .mode                   = TFT_LCD_MODE_PARALLEL_24B,
    .fb_copy_type           = FB_COPY_TYPE_NONE,
};

struct jzfb_config_info jzfb1_init_data = {
        .modes = &jzfb1_videomode,
        .lcd_type = LCD_TYPE_TFT,
        .bpp = 24,

        .tft_config = &y88249_cfg,
        .pixclk_falling_edge = 0,
        .date_enable_active_low = 0,

        .dither_enable          = 1,
        .dither.dither_red      = 1,
        .dither.dither_green    = 1,
        .dither.dither_blue     = 1,
};

struct lcd_y88249_data lcd_y88249_pdata = {
	.gpio_lcd_rd  = CONFIG_GPIO_LCD_RD,
	.gpio_lcd_rst = CONFIG_GPIO_LCD_RST,
	.gpio_lcd_cs  = CONFIG_GPIO_LCD_CS,
	.gpio_lcd_bl  = CONFIG_GPIO_LCD_BL,
};
