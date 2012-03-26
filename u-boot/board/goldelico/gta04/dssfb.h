/* u-boot driver for the DSS and framebuffer
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

#include <asm/arch/dss.h>

void dssfb_init(const struct panel_config *lcm_cfg);

void omap3_dss_go(void);

int omap3_dss_enable_fb(int flag);

int omap3_dss_set_fb(void *addr);	// set framebuffer address

int omap3_set_color(u32 color);	// set background color
