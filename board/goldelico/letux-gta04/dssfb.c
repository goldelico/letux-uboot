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
#include <asm/arch/clocks.h>
#include <asm/arch/clocks_omap3.h>
#include "dssfb.h"

#if 0
#define DEBUGP(x, args...) printf("%s: " x, __FUNCTION__, ## args);
#define DEBUGPC(x, args...) printf(x, ## args);
#define VERIFY(VAL) if(SPI_READ() != (VAL)) { printf("expected: %d found: %d\n", VAL, SPI_READ()); return 1; }
#else
#define DEBUGP(x, args...) do { } while (0)
#define DEBUGPC(x, args...) do { } while (0)
#define VERIFY(VAL) if(SPI_READ() != (VAL)) { return 1; }
#endif

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

/*
 * Display Configuration
 */

#define DVI_BEAGLE_ORANGE_COL		0x00FF8000
#define VENC_HEIGHT			0x00ef
#define VENC_WIDTH			0x027f

/*
 * Configure VENC in DSS for Beagle to generate Color Bar
 *
 * Kindly refer to OMAP TRM for definition of these values.
 */
static const struct venc_regs venc_config_std_tv = {
	.status					= 0x0000001B,
	.f_control				= 0x00000040,
	.vidout_ctrl				= 0x00000000,
	.sync_ctrl				= 0x00008000,
	.llen					= 0x00008359,
	.flens					= 0x0000020C,
	.hfltr_ctrl				= 0x00000000,
	.cc_carr_wss_carr			= 0x043F2631,
	.c_phase				= 0x00000024,
	.gain_u					= 0x00000130,
	.gain_v					= 0x00000198,
	.gain_y					= 0x000001C0,
	.black_level				= 0x0000006A,
	.blank_level				= 0x0000005C,
	.x_color				= 0x00000000,
	.m_control				= 0x00000001,
	.bstamp_wss_data			= 0x0000003F,
	.s_carr					= 0x21F07C1F,
	.line21					= 0x00000000,
	.ln_sel					= 0x00000015,
	.l21__wc_ctl				= 0x00001400,
	.htrigger_vtrigger			= 0x00000000,
	.savid__eavid				= 0x069300F4,
	.flen__fal				= 0x0016020C,
	.lal__phase_reset			= 0x00060107,
	.hs_int_start_stop_x			= 0x008D034E,
	.hs_ext_start_stop_x			= 0x000F0359,
	.vs_int_start_x				= 0x01A00000,
	.vs_int_stop_x__vs_int_start_y		= 0x020501A0,
	.vs_int_stop_y__vs_ext_start_x		= 0x01AC0024,
	.vs_ext_stop_x__vs_ext_start_y		= 0x020D01AC,
	.vs_ext_stop_y				= 0x00000006,
	.avid_start_stop_x			= 0x03480079,
	.avid_start_stop_y			= 0x02040024,
	.fid_int_start_x__fid_int_start_y	= 0x0001008A,
	.fid_int_offset_y__fid_ext_start_x	= 0x01AC0106,
	.fid_ext_start_y__fid_ext_offset_y	= 0x01060006,
	.tvdetgp_int_start_stop_x		= 0x00140001,
	.tvdetgp_int_start_stop_y		= 0x00010001,
	.gen_ctrl				= 0x00FF0000,
	.output_control				= 0x0000000D,
	.dac_b__dac_c				= 0x00000000
};

/*
 * Configure Timings for DVI D
 */
static const struct panel_config dvid_cfg = {
	.timing_h	= 0x0ff03f31, /* Horizantal timing */
	.timing_v	= 0x01400504, /* Vertical timing */
	.pol_freq	= 0x00007028, /* Pol Freq */
	.divisor	= 0x00010006, /* 72Mhz Pixel Clock */
	.lcd_size	= 0x02ff03ff, /* 1024x768 */
	.panel_type	= 0x01, /* TFT */
	.data_lines	= 0x03, /* 24 Bit RGB */
	.load_mode	= 0x02, /* Frame Mode */
	.panel_color	= DVI_BEAGLE_ORANGE_COL /* ORANGE */
};

void dssfb_init(const struct panel_config *lcm_cfg)
{
#ifdef CONFIG_OMAP3_GTA04A2	/* delayed on GTA04A2 */
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	printf("prcm base = %p\n", (void *) prcm_base);
	printf("ick_dss_on\n");
	sr32(&prcm_base->iclken_dss, 0, 32, ICK_DSS_ON);
	sdelay(1000);
	printf("fck_dss_on\n");
	sr32(&prcm_base->fclken_dss, 0, 32, FCK_DSS_ON);
	sdelay(1000);
//	printf("fck_cam_on\n");
//	sr32(&prcm_base->fclken_cam, 0, 32, FCK_CAM_ON);
//	printf("ick_cam_on\n");
//	sr32(&prcm_base->iclken_cam, 0, 32, ICK_CAM_ON);
	sdelay(1000);
#else
	// FIXME: restore original code
#endif
	printf("dss panel config\n");	
	omap3_dss_panel_config(lcm_cfg);	// set new config
	printf("dss enable\n");
	omap3_dss_enable();	// and (re)enable
}

/*
 * Configure DSS to display background color on DVID
 * Configure VENC to display color bar on S-Video
 */
void display_init(void)
{
	omap3_dss_venc_config(&venc_config_std_tv, VENC_HEIGHT, VENC_WIDTH);
	omap3_dss_panel_config(&dvid_cfg);
}

