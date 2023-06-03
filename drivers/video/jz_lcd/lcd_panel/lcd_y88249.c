/*
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

#include <config.h>
#include <serial.h>
#include <common.h>
#include <lcd.h>
#include <linux/list.h>
#include <linux/fb.h>
#include <asm/types.h>
#include <asm/arch-x1830/tcu.h>
#include <asm/arch-x1830/lcdc.h>
#include <asm/arch-x1830/gpio.h>
#include <regulator.h>
#include <jz_lcd/lcd_y88249.h>
#include <jz_lcd/jz_lcd_v14.h>

extern struct jzfb_config_info lcd_config_info;
struct lcd_y88249_data lcd_y88249_pdata;

vidinfo_t panel_info = { 640, 480, LCD_BPP, };

void panel_pin_init(void)
{
	int ret = 0;
	ret = gpio_request(lcd_y88249_pdata.gpio_lcd_cs, "lcd_cs");
	if(ret){
		/*printf("canot request gpio lcd_cs\n");*/
	}

	ret = gpio_request(lcd_y88249_pdata.gpio_lcd_rd, "lcd_rd");
	if(ret){
		/*printf("canot request gpio lcd_rd\n");*/
	}

        ret = gpio_request(lcd_y88249_pdata.gpio_lcd_rst, "lcd_rst");
	if(ret){
		/*printf("canot request gpio lcd_rst\n");*/
	}

	ret = gpio_request(lcd_y88249_pdata.gpio_lcd_bl, "lcd_bl");
	if(ret){
		/*printf("canot request gpio lcd_bl\n");*/
	}
	serial_puts("lcd_y88249 panel display pin init\n");
}

void panel_power_on(void)
{

	/*power reset*/
        gpio_direction_output(lcd_y88249_pdata.gpio_lcd_rst, 1);
	mdelay(1);
	gpio_direction_output(lcd_y88249_pdata.gpio_lcd_rst, 0);
	mdelay(10);
        gpio_direction_output(lcd_y88249_pdata.gpio_lcd_rst, 1);
	mdelay(500);

	gpio_direction_output(lcd_y88249_pdata.gpio_lcd_bl, 1);

	serial_puts("lcd_y88249 panel display on\n");
}

void panel_power_off(void)
{
	gpio_direction_output(lcd_y88249_pdata.gpio_lcd_cs, 0);
	gpio_direction_output(lcd_y88249_pdata.gpio_lcd_rd, 0);
	gpio_direction_output(lcd_y88249_pdata.gpio_lcd_rst, 0);
	gpio_direction_output(lcd_y88249_pdata.gpio_lcd_bl, 0);
	serial_puts("lcd_y88249 panel display off\n");
}

struct fb_videomode jzfb1_videomode = {
	.name = "640x480",
	//.refresh = 30,
	.refresh = 0,
	.xres = 640,
	.yres = 480,
	.pixclock = KHZ2PICOS(33264),
	.left_margin = 20,
	.right_margin = 20,
	.upper_margin = 6,
	.lower_margin = 12,
	.hsync_len = 2,
	.vsync_len = 2,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
//	.vmode = FB_VMODE_NONINTERLACED,
//	.flag = 0,

};
