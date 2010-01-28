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

#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include <asm/arch/dss.h>

#define DVI_BACKGROUND_COLOR		0x00fadc29	// rgb(250, 220, 41)

// configure beagle board DSS for the TD28TTEC1

#define DSS1_FCLK	432000000	// see figure 15-65
#define PIXEL_CLOCK	22000000	// approx. 22 MHz (will be divided from 432 MHz)

// all values are min ratings

#define VDISP	640				// vertical active area
#define VFP		4				// vertical front porch
#define VS		2				// VSYNC pulse width (negative going)
#define VBP		2				// vertical back porch
#define VDS		(VS+VBP)		// vertical data start
#define VBL		(VS+VBP+VFP)	// vertical blanking period
#define VP		(VDISP+VBL)		// vertical cycle

#define HDISP	480				// horizontal active area
#define HFP		24				// horizontal front porch
#define HS		8				// HSYNC pulse width (negative going)
#define HBP		8				// horizontal back porch
#define HDS		(HS+HBP)		// horizontal data start
#define HBL		(HS+HBP+HFP)	// horizontal blanking period
#define HP		(HDISP+HBL)		// horizontal cycle

static const struct panel_config lcm_cfg = 
{
.timing_h	= ((HBP-1)<<20) | ((HFP-1)<<8) | ((HS-1)<<0), /* Horizantal timing */
.timing_v	= ((VBP+0)<<20) | ((VFP+0)<<8) | ((VS-1)<<0), /* Vertical timing */
.pol_freq	= (1<<17)|(0<<16)|(0<<15)|(1<<14)|(1<<13)|(1<<12)|0x28,    /* Pol Freq */
.divisor	= (0x0001<<16)|(DSS1_FCLK/PIXEL_CLOCK), /* Pixel Clock divisor from dss1_fclk */
.lcd_size	= ((HDISP-1)<<0) | ((VDISP-1)<<16), /* as defined by LCM */
.panel_type	= 0x01, /* TFT */
.data_lines	= 0x03, /* 24 Bit RGB */
.load_mode	= 0x02, /* Frame Mode */
.panel_color	= DVI_BACKGROUND_COLOR
};

void omap3_dss_go(void)
{ // push changes from shadow register to display controller
	struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
	u32 l = 0;
	l = readl(&dispc->control);
	l |= GO_LCD | GO_DIG;
	writel(l, &dispc->control);
	while((readl(&dispc->control) & (GO_LCD | GO_DIG)) != 0)
		udelay(1000);	// udelay(1000) until the bit(s) are reset by Hardware!
	DEBUGP("omap3_dss_go() dispc_control: %08x\n", readl(&dispc->control));
}

struct gfx_regs 
{
	u32 gfx_ba[2];				/* 80 */
	u32 gfx_position;			/* 88 */
	u32 gfx_size;				/* 8c */
	u32 reserved1[4];			
	u32 gfx_attributes;			/* a0 */
#define GFX_ENABLE 0x0001
	u32 gfx_fifo_threshold;		/* a4 */
	u32 gfx_fifo_size_status;	/* a8 */
	u32 gfx_row_inc;			/* ac */
	u32 gfx_pixel_inc;			/* b0 */
	u32 gfx_window_skip;		/* b4 */
	u32 gfx_table_ba;			/* b8 */
};

#define OMAP3_GFX_BASE (0x48050480)

int omap3_dss_enable_fb(int flag)
{
	struct gfx_regs *gfx = (struct gfx_regs *) OMAP3_GFX_BASE;
	u32 l = readl(&gfx->gfx_attributes);
	if(flag)
		l |= GFX_ENABLE;
	else
		l &= ~GFX_ENABLE;
	DEBUGP("write %x to gfx_attibutes: %08x\n", l, &gfx->gfx_attributes);
	writel(l, &gfx->gfx_attributes);
	omap3_dss_go();
	DEBUGP("framebuffer enabled: %d\n", flag);
	DEBUGP("gfx_attibutes: %08x\n", readl(&gfx->gfx_attributes));
	return 0;
}

int omap3_dss_set_fb(void *addr)
{ // set framebuffer address
	struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
	struct gfx_regs *gfx = (struct gfx_regs *) OMAP3_GFX_BASE;
	if(addr != NULL)
		{
			//			u32 l = readl(&dispc->control);
			//			l |= GO_LCD | GO_DIG;
			//			writel(l, &dispc->control);
			//			printf("write %x to gfx_attibutes: %08x\n", l, &gfx->gfx_attributes);
			//			writel(l, &gfx->gfx_attributes);
			//			printf("gfx_ba[0]: %08x\n", &gfx->gfx_ba[0]);
			writel((u32) addr, &gfx->gfx_ba[0]);
			writel((u32) addr, &gfx->gfx_ba[1]);
			//			printf("framebuffer address: %08x\n", addr);
			writel(0, &gfx->gfx_position);
			//			printf("size_lcd: %08x\n", readl(&dispc->size_lcd));
			writel(readl(&dispc->size_lcd), &gfx->gfx_size);
			writel(0x008c, &gfx->gfx_attributes);	// 16x32 bit bursts + RGB16?
			writel(((0x3fc << 16) + (0x3bc)), &gfx->gfx_fifo_threshold);	// high & low
			writel(1024, &gfx->gfx_fifo_size_status);	// FIFO size in bytes
			writel(1, &gfx->gfx_row_inc);
			writel(1, &gfx->gfx_pixel_inc);
			writel(0, &gfx->gfx_window_skip);
			writel(0x807ff000, &gfx->gfx_table_ba);
			omap3_dss_enable_fb(1);
#if DEBUG
			{
				u32 addr;
				for(addr=0x48050010; addr <= 0x48050010; addr+=4)
					printf("%08x: %08x\n", addr, readl(addr));
				for(addr=0x48050040; addr <= 0x48050058; addr+=4)
					printf("%08x: %08x\n", addr, readl(addr));
				for(addr=0x48050410; addr <= 0x48050414; addr+=4)
					printf("%08x: %08x\n", addr, readl(addr));
				for(addr=0x48050444; addr <= 0x4805048c; addr+=4)
					printf("%08x: %08x\n", addr, readl(addr));
				for(addr=0x480504a0; addr <= 0x480504b8; addr+=4)
					printf("%08x: %08x\n", addr, readl(addr));
			}
#endif
		}
	else
		{ // disable
			omap3_dss_enable_fb(0);
		}
	return 0;
}

int omap3_set_color(u32 color)
{
	struct dispc_regs *dispc = (struct dispc_regs *) OMAP3_DISPC_BASE;
	writel(color, &dispc->default_color0);
	omap3_dss_go();
	printf("background color: %06x\n", color);
	return 0;
}

