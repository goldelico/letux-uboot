/*
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Huddy <hyli@ingenic.cn>
 * Based on: xboot/boot/lcd/jz4775_android_lcd.h
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

#ifndef _LCD_MA0060_H__
#define _LCD_MA0060_H__
#include <jz_lcd/jz_dsim.h>

extern void panel_pin_init(void);
extern void open_backlight(void);
extern void panel_power_on(void);
extern void panel_poer_off(void);
extern void ma0060_sleep_in(struct dsi_device *dsi);
extern void ma0060_sleep_out(struct dsi_device *dsi);
extern void ma0060_display_on(struct dsi_device *dsi);
extern void ma0060_display_off(struct dsi_device *dsi);
extern void ma0060_panel_init(struct dsi_device *dsi);


#endif
