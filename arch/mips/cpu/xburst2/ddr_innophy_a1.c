/*
 * DDR driver for Synopsys DWC DDR PHY.
 * Used by Jz4775, JZ4780...
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

//#define DEBUG
/*#define DEBUG_READ_WRITE */
//#define CONFIG_DDR_DEBUG 1
#include <config.h>
#include <common.h>
#include <ddr/ddr_common.h>
#include <generated/ddr_reg_values.h>
#include <asm/cacheops.h>
#include <asm/dma-default.h>

#include <asm/io.h>
#include <asm/arch/clk.h>
#include "ddr_debug.h"
#define ddr_hang() do{						\
		serial_debug("%s %d\n",__FUNCTION__,__LINE__);	\
		hang();						\
	}while(0)

DECLARE_GLOBAL_DATA_PTR;

#ifdef  CONFIG_DDR_DEBUG
#define FUNC_ENTER() serial_debug("%s enter.\n",__FUNCTION__);
#define FUNC_EXIT() serial_debug("%s exit.\n",__FUNCTION__);

static void dump_ddrc_register(void)
{
	serial_debug("DDRC_STATUS         0x%x\n", ddr_readl(DDRC_STATUS));
	serial_debug("DDRC_CFG            0x%x\n", ddr_readl(DDRC_CFG));
	serial_debug("DDRC_CTRL           0x%x\n", ddr_readl(DDRC_CTRL));
	serial_debug("DDRC_LMR            0x%x\n", ddr_readl(DDRC_LMR));
	serial_debug("DDRC_DLP            0x%x\n", ddr_readl(DDRC_DLP));
	serial_debug("DDRC_TIMING1        0x%x\n", ddr_readl(DDRC_TIMING(1)));
	serial_debug("DDRC_TIMING2        0x%x\n", ddr_readl(DDRC_TIMING(2)));
	serial_debug("DDRC_TIMING3        0x%x\n", ddr_readl(DDRC_TIMING(3)));
	serial_debug("DDRC_TIMING4        0x%x\n", ddr_readl(DDRC_TIMING(4)));
	serial_debug("DDRC_TIMING5        0x%x\n", ddr_readl(DDRC_TIMING(5)));
	serial_debug("DDRC_REFCNT         0x%x\n", ddr_readl(DDRC_REFCNT));
	serial_debug("DDRC_AUTOSR_CNT     0x%x\n", ddr_readl(DDRC_AUTOSR_CNT));
	serial_debug("DDRC_AUTOSR_EN      0x%x\n", ddr_readl(DDRC_AUTOSR_EN));
	serial_debug("DDRC_MMAP0          0x%x\n", ddr_readl(DDRC_MMAP0));
	serial_debug("DDRC_MMAP1          0x%x\n", ddr_readl(DDRC_MMAP1));
	serial_debug("DDRC_REMAP1         0x%x\n", ddr_readl(DDRC_REMAP(1)));
	serial_debug("DDRC_REMAP2         0x%x\n", ddr_readl(DDRC_REMAP(2)));
	serial_debug("DDRC_REMAP3         0x%x\n", ddr_readl(DDRC_REMAP(3)));
	serial_debug("DDRC_REMAP4         0x%x\n", ddr_readl(DDRC_REMAP(4)));
	serial_debug("DDRC_REMAP5         0x%x\n", ddr_readl(DDRC_REMAP(5)));
	serial_debug("DDRC_DWCFG          0x%x\n", ddr_readl(DDRC_DWCFG));
	serial_debug("DDRC_DWSTATUS          0x%x\n", ddr_readl(DDRC_DWSTATUS));

	serial_debug("DDRC_HREGPRO        0x%x\n", ddr_readl(DDRC_HREGPRO));
	serial_debug("DDRC_PREGPRO        0x%x\n", ddr_readl(DDRC_PREGPRO));
	serial_debug("DDRC_CGUC0          0x%x\n", ddr_readl(DDRC_CGUC0));
	serial_debug("DDRC_CGUC1          0x%x\n", ddr_readl(DDRC_CGUC1));

}

static void dump_ddrp_register(void)
{
#if 1
	serial_debug("DDRP_INNO_PHY_RST    0x%x\n", ddr_readl(DDRP_INNOPHY_INNO_PHY_RST	));
	serial_debug("DDRP_MEM_CFG         0x%x\n", ddr_readl(DDRP_INNOPHY_MEM_CFG		));
	serial_debug("DDRP_TRAINING_CTRL   0x%x\n", ddr_readl(DDRP_INNOPHY_TRAINING_CTRL	));
	serial_debug("DDRP_CALIB_DELAY_AL  0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL1));
	serial_debug("DDRP_CALIB_DELAY_AH  0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH1));
	serial_debug("DDRP_CALIB_DELAY_BL  0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL1));
	serial_debug("DDRP_CALIB_DELAY_BH  0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH1));
	serial_debug("DDRP_CL              0x%x\n", ddr_readl(DDRP_INNOPHY_CL				));
	serial_debug("DDRP_CWL             0x%x\n", ddr_readl(DDRP_INNOPHY_CWL			));
	serial_debug("DDRP_WL_DONE         0x%x\n", ddr_readl(DDRP_INNOPHY_WL_DONE		));
	serial_debug("DDRP_CALIB_DONE      0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE		));
	serial_debug("DDRP_PLL_LOCK        0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_LOCK		));
	serial_debug("DDRP_PLL_FBDIV       0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_FBDIV		));
	serial_debug("DDRP_PLL_CTRL        0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_CTRL		));
	serial_debug("DDRP_PLL_PDIV        0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_PDIV		));

	serial_debug("DDRP_TRAINING_2c     0x%x\n", ddr_readl(DDRP_INNOPHY_TRAINING_2c	));
	serial_debug("DDRP_TRAINING_3c     0x%x\n", ddr_readl(DDRP_INNOPHY_TRAINING_3c	));
	serial_debug("DDRP_TRAINING_4c     0x%x\n", ddr_readl(DDRP_INNOPHY_TRAINING_4c	));
	serial_debug("DDRP_TRAINING_5c     0x%x\n", ddr_readl(DDRP_INNOPHY_TRAINING_5c	));
#endif
#if 0
	unsigned int offset;
	serial_debug("------------------ ddr phy common register ------------------\n");
	for (offset = 0; offset <= 0x136; offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	serial_debug("------------------ ddr phy specail register ------------------\n");

	serial_debug("-------- DRIVE STRENGTH --------\n");
	for (offset = 0x130; offset <= (0x130+3); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = 0x140; offset <= (0x140+3); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = 0x150; offset <= (0x150+3); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = 0x160; offset <= (0x160+3); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = 0x170; offset <= (0x170+3); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	serial_debug("-------- PER BIT DE-SKEW --------\n");
	for (offset = 0x340; offset <= (0x340+0x32); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = 0x1c0; offset <= (0x1c0+0x2b); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = 0x220; offset <= (0x220+0x2b); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	serial_debug("-------- RX CALIBRATION --------\n");
	for (offset = (0x70+0x2b); offset <= (0x70+0x2e); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = (0xa0+0x2b); offset <= (0xa0+0x2e); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = (0x70+0x00); offset <= (0x70+0x01); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = (0x70+0x10); offset <= (0x70+0x11); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = (0x70+0x02); offset <= (0x70+0x03); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = (0x70+0x12); offset <= (0x70+0x13); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = (0xa0+0x00); offset <= (0xa0+0x01); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = (0xa0+0x10); offset <= (0xa0+0x11); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = (0xa0+0x02); offset <= (0xa0+0x03); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
	for (offset = (0xa0+0x12); offset <= (0xa0+0x13); offset++) {
		serial_debug("ddr phy offset %x, addr %x, value %x\n", offset, 0xb3011000+offset*4, readl(0xb3011000+offset*4));
	}
#endif
}

static void dump_ddrp_driver_strength_register(void)
{
	serial_debug("-------------------------------------------------------\n");
	serial_debug("cmd strenth pull_down                = 0x%x\n", readl(0xb3011000+4*0x130));
	serial_debug("cmd strenth pull_up                  = 0x%x\n", readl(0xb3011000+4*0x131));
	serial_debug("clk strenth pull_down                = 0x%x\n", readl(0xb3011000+4*0x132));
	serial_debug("clk strenth pull_up                  = 0x%x\n", readl(0xb3011000+4*0x133));
	serial_debug("data A io ODT DQ[7:0]  pull_down               = 0x%x\n", readl(0xb3011000+4*0x140));
	serial_debug("data A io ODT DQ[7:0]  pull_up                 = 0x%x\n", readl(0xb3011000+4*0x141));
	serial_debug("data A io ODT DQ[15:8] pull_down               = 0x%x\n", readl(0xb3011000+4*0x150));
	serial_debug("data A io ODT DQ[15:8] pull_up                 = 0x%x\n", readl(0xb3011000+4*0x151));
	serial_debug("data B io ODT DQ[7:0]  pull_down               = 0x%x\n", readl(0xb3011000+4*0x160));
	serial_debug("data B io ODT DQ[7:0]  pull_up                 = 0x%x\n", readl(0xb3011000+4*0x161));
	serial_debug("data B io ODT DQ[15:8] pull_down               = 0x%x\n", readl(0xb3011000+4*0x170));
	serial_debug("data B io ODT DQ[15:8] pull_up                 = 0x%x\n", readl(0xb3011000+4*0x171));
	serial_debug("data A io strenth DQ[7:0]  pull_down    = 0x%x\n", readl(0xb3011000+4*0x142));
	serial_debug("data A io strenth DQ[7:0]  pull_up      = 0x%x\n", readl(0xb3011000+4*0x143));
	serial_debug("data A io strenth DQ[15:8] pull_down    = 0x%x\n", readl(0xb3011000+4*0x152));
	serial_debug("data A io strenth DQ[15:8] pull_up      = 0x%x\n", readl(0xb3011000+4*0x153));
	serial_debug("data B io strenth DQ[7:0]  pull_down    = 0x%x\n", readl(0xb3011000+4*0x162));
	serial_debug("data B io strenth DQ[7:0]  pull_up      = 0x%x\n", readl(0xb3011000+4*0x163));
	serial_debug("data B io strenth DQ[15:8] pull_down    = 0x%x\n", readl(0xb3011000+4*0x172));
	serial_debug("data B io strenth DQ[15:8] pull_up      = 0x%x\n", readl(0xb3011000+4*0x173));
#if 0
	writel(0x1c, 0xb3011000+4*0x130);
	writel(0x1c, 0xb3011000+4*0x131);
	writel(0x1c, 0xb3011000+4*0x132);
	writel(0x1c, 0xb3011000+4*0x133);
#endif
#if 0
	//pull down
	writel(0x1, 0xb3011000+4*0x140);
	writel(0x1, 0xb3011000+4*0x150);
	writel(0x1, 0xb3011000+4*0x160);
	writel(0x1, 0xb3011000+4*0x170);
	//pull up
	writel(0x0, 0xb3011000+4*0x141);
	writel(0x0, 0xb3011000+4*0x151);
	writel(0x0, 0xb3011000+4*0x161);
	writel(0x0, 0xb3011000+4*0x171);
#endif
#if 0
	writel(0x1c, 0xb3011000+4*0x142);
	writel(0x1c, 0xb3011000+4*0x143);
	writel(0x1c, 0xb3011000+4*0x152);
	writel(0x1c, 0xb3011000+4*0x153);
	writel(0x1c, 0xb3011000+4*0x162);
	writel(0x1c, 0xb3011000+4*0x163);
	writel(0x1c, 0xb3011000+4*0x172);
	writel(0x1c, 0xb3011000+4*0x173);
#endif
#if 0
	serial_debug("-------------------------------------------------------\n");
	serial_debug("cmd strenth pull_down                = 0x%x\n", readl(0xb3011000+4*0x130));
	serial_debug("cmd strenth pull_up                  = 0x%x\n", readl(0xb3011000+4*0x131));
	serial_debug("clk strenth pull_down                = 0x%x\n", readl(0xb3011000+4*0x132));
	serial_debug("clk strenth pull_up                  = 0x%x\n", readl(0xb3011000+4*0x133));
	serial_debug("data A io ODT DQ[7:0]  pull_down               = 0x%x\n", readl(0xb3011000+4*0x140));
	serial_debug("data A io ODT DQ[7:0]  pull_up                 = 0x%x\n", readl(0xb3011000+4*0x141));
	serial_debug("data A io ODT DQ[15:8] pull_down               = 0x%x\n", readl(0xb3011000+4*0x150));
	serial_debug("data A io ODT DQ[15:8] pull_up                 = 0x%x\n", readl(0xb3011000+4*0x151));
	serial_debug("data B io ODT DQ[7:0]  pull_down               = 0x%x\n", readl(0xb3011000+4*0x160));
	serial_debug("data B io ODT DQ[7:0]  pull_up                 = 0x%x\n", readl(0xb3011000+4*0x161));
	serial_debug("data B io ODT DQ[15:8] pull_down               = 0x%x\n", readl(0xb3011000+4*0x170));
	serial_debug("data B io ODT DQ[15:8] pull_up                 = 0x%x\n", readl(0xb3011000+4*0x171));
	serial_debug("data A io strenth DQ[7:0]  pull_down    = 0x%x\n", readl(0xb3011000+4*0x142));
	serial_debug("data A io strenth DQ[7:0]  pull_up      = 0x%x\n", readl(0xb3011000+4*0x143));
	serial_debug("data A io strenth DQ[15:8] pull_down    = 0x%x\n", readl(0xb3011000+4*0x152));
	serial_debug("data A io strenth DQ[15:8] pull_up      = 0x%x\n", readl(0xb3011000+4*0x153));
	serial_debug("data B io strenth DQ[7:0]  pull_down    = 0x%x\n", readl(0xb3011000+4*0x162));
	serial_debug("data B io strenth DQ[7:0]  pull_up      = 0x%x\n", readl(0xb3011000+4*0x163));
	serial_debug("data B io strenth DQ[15:8] pull_down    = 0x%x\n", readl(0xb3011000+4*0x172));
	serial_debug("data B io strenth DQ[15:8] pull_up      = 0x%x\n", readl(0xb3011000+4*0x173));
#endif

}

static void dump_ddrp_per_bit_de_skew(void)
{
	int channel = 0;
	unsigned int reg_offset[2] = {0x1c0, 0x220};
	unsigned int offset;
	serial_debug("---------------------------------------------------------------\n");
	for (channel = 0; channel < 2; channel++) {
		offset = reg_offset[channel];
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x0)*4, readl(0xb3011000+(offset+0x0)*4));//RX DM0
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x2)*4, readl(0xb3011000+(offset+0x2)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x4)*4, readl(0xb3011000+(offset+0x4)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x6)*4, readl(0xb3011000+(offset+0x6)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x8)*4, readl(0xb3011000+(offset+0x8)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0xa)*4, readl(0xb3011000+(offset+0xa)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x8)*4, readl(0xb3011000+(offset+0x8)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0xe)*4, readl(0xb3011000+(offset+0xe)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x10)*4, readl(0xb3011000+(offset+0x10)*4));//RX DQ7
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x12)*4, readl(0xb3011000+(offset+0x12)*4));//RX DQS0
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x15)*4, readl(0xb3011000+(offset+0x15)*4));//RX DM1
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x17)*4, readl(0xb3011000+(offset+0x17)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x19)*4, readl(0xb3011000+(offset+0x19)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x1b)*4, readl(0xb3011000+(offset+0x1b)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x1d)*4, readl(0xb3011000+(offset+0x1d)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x1f)*4, readl(0xb3011000+(offset+0x1f)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x21)*4, readl(0xb3011000+(offset+0x21)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x23)*4, readl(0xb3011000+(offset+0x23)*4));
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x25)*4, readl(0xb3011000+(offset+0x25)*4));//RX DQ15
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x27)*4, readl(0xb3011000+(offset+0x27)*4));//RX DQS1
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x2a)*4, readl(0xb3011000+(offset+0x2a)*4));//RX DQSB0
		serial_debug("phy de skew chn %d, reg offset %x, reg addr %x, reg value %x\n", channel, offset, 0xb3011000+(offset+0x2b)*4, readl(0xb3011000+(offset+0x2b)*4));//RX DQSB1
	}
}
#else
#define FUNC_ENTER()
#define FUNC_EXIT()
#define dump_ddrc_register()
#define dump_ddrp_register()
#define dump_ddrp_driver_strength_register()
#define dump_ddrp_per_bit_de_skew()
#endif

static void ddrp_set_drv_odt(void)
{

	int odt_pu,odt_pd;
	int drvcmd;
	int drval_pu,drval_pd;
	int drvah_pu,drvah_pd;
	int drvbl_pu,drvbl_pd;
	int drvbh_pu,drvbh_pd;

	odt_pd = 1;
	odt_pu = 0;
	drvcmd = 0xe;
#ifdef CONFIG_A1N
	drval_pd = 0x14; drval_pu = 0x14;
	drvah_pd = 0x14; drvah_pu = 0x14;
	drvbl_pd = 0x14; drvbl_pu = 0x08;
	drvbh_pd = 0x14; drvbh_pu = 0x08;
#elif defined(CONFIG_A1NT)
	drval_pd = 0x06; drval_pu = 0x03;
	drvah_pd = 0x06; drvah_pu = 0x03;
	drvbl_pd = 0xa; drvbl_pu = 0x06;
	drvbh_pd = 0xa; drvbh_pu = 0x06;
#elif defined(CONFIG_A1X)
	drval_pd = 0x14; drval_pu = 0x14;
	drvah_pd = 0x14; drvah_pu = 0x14;
	drvbl_pd = 0x14; drvbl_pu = 0x08;
	drvbh_pd = 0x14; drvbh_pu = 0x08;
#endif

#if 0
	serial_debug("input drv,odt(odt_pd,odt_pu,drvcmd):\n");
	if (3 == scanf("%d,%d,%d\n",
				&odt_pd, &odt_pu,
				&drvcmd)) {
		;
	} else {
	}
	serial_debug("input drv,odt(drval_pd,drval_pu,drvah_pd,drvah_pu):\n");
	if (4 == scanf("%d,%d,%d,%d\n",
				&drval_pd, &drval_pu,
				&drvah_pd, &drvah_pu)) {
		;
	} else {
	}
	serial_debug("input drv,odt(drvbl_pd,drvbl_pu,drvbh_pd,drvbh_pu):\n");
	if (4 == scanf("%d,%d,%d,%d\n",
				&drvbl_pd, &drvbl_pu,
				&drvbh_pd, &drvbh_pu)) {
		;
	} else {
	}

	serial_debug("dump drv,odt:\n"
			"odt_pd = %x, odt_pu = %x\n"
			"drvcmd = %x\n"
			"drval_pd = %x, drval_pu = %x\n"
			"drvah_pd = %x, drvah_pu = %x\n"
			"drvbl_pd = %x, drvbl_pu = %x\n"
			"drvbh_pd = %x, drvbh_pu = %x\n",
			odt_pd, odt_pu,
			drvcmd,
			drval_pd, drval_pu,
			drvah_pd, drvah_pu,
			drvbl_pd, drvbl_pu,
			drvbh_pd, drvbh_pu
		  );
#endif

	//odt
	//pull down
	writel(odt_pd, 0xb3011000+4*0x140);
	writel(odt_pd, 0xb3011000+4*0x150);
	writel(odt_pd, 0xb3011000+4*0x160);
	writel(odt_pd, 0xb3011000+4*0x170);
	//pull up
	writel(odt_pu, 0xb3011000+4*0x141);
	writel(odt_pu, 0xb3011000+4*0x151);
	writel(odt_pu, 0xb3011000+4*0x161);
	writel(odt_pu, 0xb3011000+4*0x171);

	//driver
	writel(drvcmd, 0xb3011000+4*0x130);
	writel(drvcmd, 0xb3011000+4*0x131);
	writel(drvcmd, 0xb3011000+4*0x132);
	writel(drvcmd, 0xb3011000+4*0x133);

	writel(drval_pd, 0xb3011000+4*0x142);
	writel(drval_pu, 0xb3011000+4*0x143);
	writel(drvah_pd, 0xb3011000+4*0x152);
	writel(drvah_pu, 0xb3011000+4*0x153);

	writel(drvbl_pd, 0xb3011000+4*0x162);
	writel(drvbl_pu, 0xb3011000+4*0x163);
	writel(drvbh_pd, 0xb3011000+4*0x172);
	writel(drvbh_pu, 0xb3011000+4*0x173);
}

static void mem_remap(void)
{
	int i;
	unsigned int remap_array[] = REMMAP_ARRAY;
	for(i = 0;i < ARRAY_SIZE(remap_array);i++)
	{
		ddr_writel(remap_array[i],DDRC_REMAP(i+1));
	}
}

//#define ddr_writel(value, reg)	writel((value), DDRC_BASE + reg)

static void ddrp_reg_set_range(u32 offset, u32 startbit, u32 bitscnt, u32 value)
{
	u32 reg = 0;
	u32 mask = 0;
	mask = ((0xffffffff>>startbit)<<(startbit))&((0xffffffff<<(32-startbit-bitscnt))>>(32-startbit-bitscnt));
	reg = readl(DDRC_BASE+DDR_PHY_OFFSET+(offset*4));
	reg = (reg&(~mask))|((value<<startbit)&mask);
	//serial_debug("value = %x, reg = %x, mask = %x", value, reg, mask);
	writel(reg, DDRC_BASE+DDR_PHY_OFFSET+(offset*4));
}

static u32 ddrp_reg_get(u32 offset)
{
	return  readl(DDRC_BASE+DDR_PHY_OFFSET+(offset*4));
}

static void ddrc_post_init(void)
{
	ddr_writel(DDRC_REFCNT_VALUE, DDRC_REFCNT);
#ifdef CONFIG_DDR_TYPE_DDR3
	mem_remap();
#endif
	ddr_writel(DDRC_CTRL_VALUE, DDRC_CTRL);

	ddr_writel(DDRC_CGUC0_VALUE, DDRC_CGUC0);
	ddr_writel(DDRC_CGUC1_VALUE, DDRC_CGUC1);

}

static void ddrc_prev_init(void)
{
	FUNC_ENTER();
	/* DDRC CFG init*/
	/* /\* DDRC CFG init*\/ */
	/* ddr_writel(DDRC_CFG_VALUE, DDRC_CFG); */

    	/* DDRC timing init*/
	ddr_writel(DDRC_TIMING1_VALUE, DDRC_TIMING(1));
	ddr_writel(DDRC_TIMING2_VALUE, DDRC_TIMING(2));
	ddr_writel(DDRC_TIMING3_VALUE, DDRC_TIMING(3));
	ddr_writel(DDRC_TIMING4_VALUE, DDRC_TIMING(4));
	ddr_writel(DDRC_TIMING5_VALUE, DDRC_TIMING(5));

	/* DDRC memory map configure*/
	ddr_writel(DDRC_MMAP0_VALUE, DDRC_MMAP0);
	ddr_writel(DDRC_MMAP1_VALUE, DDRC_MMAP1);

	/* ddr_writel(DDRC_CTRL_CKE, DDRC_CTRL); */
	ddr_writel(DDRC_CTRL_VALUE & ~(7 << 12), DDRC_CTRL);

	FUNC_EXIT();
}

static enum ddr_type get_ddr_type(void)
{
	int type;
	ddrc_cfg_t ddrc_cfg;
	ddrc_cfg.d32 = DDRC_CFG_VALUE;
	switch(ddrc_cfg.b.TYPE){
	case 3:
		type = LPDDR;
		break;
	case 4:
		type = DDR2;
		break;
	case 5:
		type = LPDDR2;
		break;
	case 6:
		type = DDR3;
		break;
	default:
		type = UNKOWN;
		debug("unsupport ddr type!\n");
		ddr_hang();
	}
	return type;
}

static void ddrc_reset_phy(void)
{
	FUNC_ENTER();
	ddr_writel(0xf << 20, DDRC_CTRL);
	mdelay(1);
	ddr_writel(0x8 << 20, DDRC_CTRL);  //dfi_reset_n low for innophy
	mdelay(1);
	FUNC_EXIT();
}

static struct jzsoc_ddr_hook *ddr_hook = NULL;
void register_ddr_hook(struct jzsoc_ddr_hook * hook)
{
	ddr_hook = hook;
}

static void ddrp_pll_init(void)
{
	unsigned int val;
	unsigned int timeout = 1000000;

	/* fbdiv={reg21[0],reg20[7:0]};
	 * pre_div = reg22[4:0];
	 * post_div = 2^reg22[7:5];
	 * fpll_4xclk = fpll_refclk * fbdiv / ( post_div * pre_div);
	 *			  = fpll_refclk * 16 / ( 1 * 4 ) = fpll_refclk * 4;
	 * fpll_1xclk = fpll_2xclk / 2 = fpll_refclk;
	 * Use: IN:fpll_refclk, OUT:fpll_4xclk */

	/* reg20; bit[7:0]fbdiv M[7:0] */
#if 0
	val = ddr_readl(DDRP_INNOPHY_PLL_FBDIV);
	val &= ~(0xff);
	val |= 0x6a;
	ddr_writel(val, DDRP_INNOPHY_PLL_FBDIV);

	/* reg21; bit[7:2]reserved; bit[1]PLL reset; bit[0]fbdiv M8 */
	//val = ddr_readl(DDRP_INNOPHY_PLL_CTRL);
	//val &= ~(0xff);
	//val |= 0x1a;
	//ddr_writel(val, DDRP_INNOPHY_PLL_CTRL);

	/* reg22; bit[7:5]post_div; bit[4:0]pre_div */
	val = ddr_readl(DDRP_INNOPHY_PLL_PDIV);
	val &= ~(0xff);
	val |= 0x42;
	ddr_writel(val, DDRP_INNOPHY_PLL_PDIV);

	/* reg21; bit[7:2]reserved; bit[1]PLL reset; bit[0]fbdiv M8 */
	val = ddr_readl(DDRP_INNOPHY_PLL_CTRL);
	val &= ~(0xff);
	val |= 0x4;
	ddr_writel(val, DDRP_INNOPHY_PLL_CTRL);

	val = ddr_readl(DDRP_INNOPHY_PLL_PD);
	val &= ~(0xff);
	val |= 0x0;
	ddr_writel(val, DDRP_INNOPHY_PLL_PD);
#endif
	val = ddr_readl(DDRP_INNOPHY_MEM_CFG);
	val &= ~(0xff);
	val |= 0xa;
	ddr_writel(val, DDRP_INNOPHY_MEM_CFG);

	/* reg1f; bit[7:4]reserved; bit[3:0]DQ width
	 * DQ width bit[3:0]:0x1:8bit,0x3:16bit,0x7:24bit,0xf:32bit */
	val = ddr_readl(DDRP_INNOPHY_DQ_WIDTH);
	val &= ~(0xf);
#if CONFIG_DDR_DW32
	val |= 0xf; /* 0x0f:32bit */
#else
	val |= DDRP_DQ_WIDTH_DQ_H | DDRP_DQ_WIDTH_DQ_L; /* 0x3:16bit */
#endif
	ddr_writel(val, DDRP_INNOPHY_DQ_WIDTH);

#if 0
	/* reg1; bit[7:5]reserved; bit[4]burst type; bit[3:2]reserved; bit[1:0]DDR mode */
	val = ddr_readl(DDRP_INNOPHY_MEM_CFG);
	val &= ~(0xff);
#ifdef CONFIG_DDR_TYPE_DDR3
	val |= 0x10; /* burst 8 */
#elif defined(CONFIG_DDR_TYPE_DDR2)
	val |= 0x11; /* burst 8 ddr2 */
#endif
	ddr_writel(val, DDRP_INNOPHY_MEM_CFG);
#endif

	/* bit[3]reset digital core; bit[2]reset analog logic; bit[0]Reset Initial status
	 * other reserved */
	val = ddr_readl(DDRP_INNOPHY_INNO_PHY_RST);
	val &= ~(0xff);
	val |= 0x0d;
	ddr_writel(val, DDRP_INNOPHY_INNO_PHY_RST);

	/* CWL */
	val = ddr_readl(DDRP_INNOPHY_CWL);
	val &= ~(0xf);
	val |= DDRP_CWL_VALUE;
	ddr_writel(val, DDRP_INNOPHY_CWL);

	/* CL */
	val = ddr_readl(DDRP_INNOPHY_CL);
	val &= ~(0xf);
	val |= DDRP_CL_VALUE;
	ddr_writel(val, DDRP_INNOPHY_CL);

	/* AL */
	val = ddr_readl(DDRP_INNOPHY_AL);
	val &= ~(0xf);
	val = 0x0;
	ddr_writel(val, DDRP_INNOPHY_AL);

	val = ddr_readl(DDRP_INNOPHY_PLL_PD);
	val &= ~(0xff);
	val |= 0x0;
	ddr_writel(val, DDRP_INNOPHY_PLL_PD);

	while(!(ddr_readl(DDRP_INNOPHY_PLL_LOCK) & 1 << 2) && timeout--);
	if(!timeout) {
		serial_debug("DDRP_INNOPHY_PLL_LOCK time out!!!\n");
		hang();
	}
	//mdelay(20);
    //dump_ddrp_driver_strength_register();
}

static void ddrp_software_writeleveling_tx(void)
{
    int k,i,j,a,b,middle;
    int ret = -1;
    unsigned int addr = 0xa2000000,val;
    unsigned int reg_val;
    unsigned int ddbuf[8] = {0};
    unsigned int pass_invdelay_total[64] = {0};
    unsigned int pass_cnt = 0;
    unsigned int offset_addr_tx[4] = {0xb3011484,0xb30114b0,0xb3011684,0xb30116b0}; //A_DQ0 , A_DQ8, B_DQ0, B_DQ8 rx invdelay control register

    //ddr_writel((0x8|ddr_readl(DDRP_INNOPHY_TRAINING_CTRL)), DDRP_INNOPHY_TRAINING_CTRL);

    writel(0x8, 0xb30114a4); //A_DQS0
    writel(0x8, 0xb30114d0); //A_DQS1
    writel(0x8, 0xb30116a4); //B_DQS0
    writel(0x8, 0xb30116d0); //B_DQS1

    writel(0x8, 0xb30114a8); //A_DQSB0
    writel(0x8, 0xb30114d4); //A_DQSB1
    writel(0x8, 0xb30116a8); //B_DQSB0
    writel(0x8, 0xb30116d4); //B_DQSB1

    for (j=0; j<=0x3; j++) {
        for(k=0; k<=0x7; k++) { // DQ0~DQ7
            pass_cnt = 0;
            for(i=0; i<=0x4; i++) { //invdelay from 0 to 3f

                reg_val = readl(offset_addr_tx[j]+k*4) & ~0x3f;
                writel((i | reg_val) , (offset_addr_tx[j]+k*4)); // set invdelay, i is invdelay value , offset_addr_tx[j]+k*2 is the addr

                //mdelay(5);
			    for(a = 0; a < 0xff; a++) {

                    val = 0x12345678 + (j + k + i + a)*4;

                    *(volatile unsigned int *)(addr + a * 4) = val;

                    if(((*(volatile unsigned int *)(addr + a * 4)) & 0xff000000) != (val & 0xff000000) && ((*(volatile unsigned int *)(addr + a * 4)) & 0x00ff0000) != (val & 0x00ff0000) && ((*(volatile unsigned int *)(addr + a * 4)) & 0x0000ff00) != (val & 0x0000ff00) && ((*(volatile unsigned int *)(addr + a * 4)) & 0x000000ff) != (val & 0x000000ff)) {
                        serial_debug(" AL fail VALUE 0x%x  sVALUE 0x%x error \n", val ,(*(volatile unsigned int *)(addr + i * 4)));
                        break;
                    }
                }

                if(a == 0xff) {
                    pass_invdelay_total[pass_cnt] = i;
                    pass_cnt++;
                }
            }

            middle = pass_cnt / 2;
            writel((pass_invdelay_total[middle] | reg_val), (offset_addr_tx[j]+k*4));    //this means step 3
            serial_debug("\nTX  chan%d DQ%d small_value = %d big value %d  size = 0x%x middle = %x\n",j, k, pass_invdelay_total[0], pass_invdelay_total[pass_cnt-1], (pass_invdelay_total[pass_cnt-1] - pass_invdelay_total[0]), pass_invdelay_total[middle]);
        }
    }
}
static void ddrp_software_writeleveling_rx(void)
{
    int k,i,j,a,b,c,d,e,middle;
    int ret = -1;
    unsigned int addr = 0xa1000000;
    unsigned int val[0xff],value,tmp;
    unsigned int reg_val;
    unsigned int ddbuf[8] = {0};
    unsigned int pass_invdelay_total[64] = {0};
    unsigned int pass_dqs_total[64] = {0};
    unsigned int pass_dqs_value[64] = {0};
    unsigned int dq[8],middle_dq=0,max_dqs = 0;
    unsigned int pass_cnt = 0, pass_cnt_dqs = 0;
    unsigned int offset_addr_rx[4] = {0xb3011504,0xb3011530,0xb3011704,0xb3011730}; //A_DQ0 , A_DQ8, B_DQ0, B_DQ8 rx invdelay control register
    unsigned int offset_addr_dqs[4] = {0xb3011524,0xb3011550,0xb3011724,0xb3011750}; //A_DQS0 , A_DQS1, B_DQS0, B_DQS1 rx invdelay control register


    ddr_writel((0x8|ddr_readl(DDRP_INNOPHY_TRAINING_CTRL)), DDRP_INNOPHY_TRAINING_CTRL);
#if 1

    /* this is set default value */
    value = 0x1f;
	ddr_writel(value, DDR_PHY_OFFSET + (0x120+0x20)*4);///DM0
	ddr_writel(value, DDR_PHY_OFFSET + (0x120+0x2b)*4);///DM1

    ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+0x20)*4);///DM0
	ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+0x2b)*4);///DM1
    //RX DQ
    for (i = 0x21; i <= 0x28;i++) {
	    ddr_writel(value, DDR_PHY_OFFSET + (0x120+i)*4);//DQ0-DQ15
	    ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+i)*4);//DQ15-DQ31
    }

    //DQS
	ddr_writel(value, DDR_PHY_OFFSET + (0x120+0x29)*4);///DQS0
	ddr_writel(value, DDR_PHY_OFFSET + (0x120+0x34)*4);///DQS1

	ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+0x29)*4);///DQS0
	ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+0x34)*4);///DQS1
    for (i = 0x2c; i <= 0x33;i++) {
	    ddr_writel(value, DDR_PHY_OFFSET + (0x120+i)*4);//DQ0-DQ15
	    ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+i)*4);//DQ15-DQ31
    }

#endif
#if 1
    for(j=0; j<=0x3; j++) {
        for(k=0; k<=0x7; k++) { // DQ0~DQ7
            pass_cnt = 0;

            for(i=0x20; i<=0x3f; i++) { //invdelay from 0 to 63
                reg_val = 0;
                reg_val = readl(offset_addr_rx[j]+k*4) & ~0x3f;
                writel((i | reg_val), (offset_addr_rx[j]+k*4)); // set invdelay, i is invdelay value , offset_addr_rx[j]+k*4 is the addr

                //mdelay(5);
                for(a = 0; a < 0xff; a ++) {
                    val[a] = 0x12345678 + (j + k + i + a)*4;
                    *(volatile unsigned int *)(addr + a * 4) = val[a];
                    *(volatile unsigned int *)(addr + (a + 0xff) * 4) = val[a];
                    *(volatile unsigned int *)(addr + (a + 0xff*2) * 4) = val[a];
                    *(volatile unsigned int *)(addr + (a + 0xff*3) * 4) = val[a];
                    *(volatile unsigned int *)(addr + (a + 0xff*4) * 4) = val[a];
                    *(volatile unsigned int *)(addr + (a + 0xff*5) * 4) = val[a];
                    *(volatile unsigned int *)(addr + (a + 0xff*6) * 4) = val[a];
                }

                //flush_dcache_range(addr, addr+(0xff*4));
		        //flush_l2cache_range(addr, addr+(0xff*4));

                for(a = 0; a < 0xff; a ++) {
                    if(((*(volatile unsigned int *)(addr + a * 4)) != val[a]) && ((*(volatile unsigned int *)(addr + (a + 0xff*3) * 4)) != val[a])) {
                        //serial_debug("want = 0x%x value1 = 0x%x  value2 = 0x%x value3 = 0x%x value4 = 0x%x \n", val1, tmp, tmp1, tmp2, tmp3);
                        break;
                    }
                }

                if(a == 0xff) {
                    pass_invdelay_total[pass_cnt] = i;
                    //serial_debug("\nRX  chan%d DQ%d pass_value = %d \n",j, k, i);
                    pass_cnt++;
                } else {
                    serial_debug("##### this is the big :2222 pass_cnt = %d\n", pass_cnt);
                    break;
                }

            }
            for(i=0x1f; i>=0x0; i--) { //invdelay from 0 to 63
                reg_val = 0;
                reg_val = readl(offset_addr_rx[j]+k*4) & ~0x3f;
                writel((i | reg_val), (offset_addr_rx[j]+k*4)); // set invdelay, i is invdelay value , offset_addr_rx[j]+k*4 is the addr

                //mdelay(5);
                for(a = 0; a < 0xff; a ++) {
                    val[a] = 0x12345678 + (j + k + i + a)*4;
                    *(volatile unsigned int *)(addr + a * 4) = val[a];
                }
                //flush_dcache_range(addr, addr+(0xff*4));
		        //flush_l2cache_range(addr, addr+(0xff*4));

                for(a = 0; a < 0xff; a ++) {
                    if((*(volatile unsigned int *)(addr + a * 4)) != val[a]) {
                        //serial_debug("want = 0x%x value1 = 0x%x  value2 = 0x%x value3 = 0x%x value4 = 0x%x \n", val1, tmp, tmp1, tmp2, tmp3);
                        break;
                    }
                }

                if(a == 0xff) {
                    pass_invdelay_total[pass_cnt] = i;
                    //serial_debug("\nRX  chan%d DQ%d pass_value = %d \n",j, k, i);
                    pass_cnt++;
                } else {
                    serial_debug("##### this is the small:2222 pass_cnt = %d\n", pass_cnt);
                    break;
                }
            }

            for(a=0; a<pass_cnt-1; a++) {
                for(b=0; b<pass_cnt-a-1; b++) {
                    if(pass_invdelay_total[b] > pass_invdelay_total[b+1]) {
                        tmp= pass_invdelay_total[b];
                        pass_invdelay_total[b] = pass_invdelay_total[b+1];
                        pass_invdelay_total[b+1] = tmp;
                    }
                }
            }
            for(a=0; a<pass_cnt;a++) {
                serial_debug("##### pass_invdelay_total[%d] value %d\n", a, pass_invdelay_total[a]);
            }
            serial_debug("##### this is the small:3333\n");
            middle = pass_cnt / 2;
            writel(pass_invdelay_total[middle], (offset_addr_rx[j]+k*4));    //this means step 3
            serial_debug("\nRX  chan%d DQ%d small_value = %d big value %d  size = 0x%x middle = %x\n",j, k, pass_invdelay_total[0], pass_invdelay_total[pass_cnt-1], (pass_invdelay_total[pass_cnt-1] - pass_invdelay_total[0]), pass_invdelay_total[middle]);
        }
    }

#endif
}

static void ddrp_write_leveling_calibration(void)
{
#ifdef CONFIG_DDR_TYPE_DDR3
	unsigned int timeout = 1000000;
	ddrp_writel_byidx(DDR_MR1_VALUE&0xff, DDRP_REG_OFFSET_WR_LEVEL1);
	ddrp_writel_byidx((1<<6)|((DDR_MR1_VALUE>>8)&0x3f), DDRP_REG_OFFSET_WR_LEVEL2);
	ddrp_reg_set_range(DDRP_REG_OFFSET_RAINING_CTRL, 2, 1, 1);
	while (((ddrp_readl_byidx(DDRP_REG_OFFSET_WL_DONE) & 0xf) != 0xf) && timeout--);
	if(!timeout) {
		serial_debug("timeout:INNOPHY_WL_DONE %x\n", ddrp_readl_byidx(DDRP_REG_OFFSET_WL_DONE));
		hang();
	}
	ddrp_reg_set_range(DDRP_REG_OFFSET_RAINING_CTRL, 2, 1, 0);
	serial_debug("ddrp write leveling calibration:\n");
	serial_debug("A chn low,  reg offset %x value %x\n", DDRP_REG_OFFSET_WRLEVEL_RESULT_LOW_A, ddrp_readl_byidx(DDRP_REG_OFFSET_WRLEVEL_RESULT_LOW_A));
	serial_debug("A chn high, reg offset %x value %x\n", DDRP_REG_OFFSET_WRLEVEL_RESULT_HIGH_A, ddrp_readl_byidx(DDRP_REG_OFFSET_WRLEVEL_RESULT_HIGH_A));
	serial_debug("B chn low,  reg offset %x value %x\n", DDRP_REG_OFFSET_WRLEVEL_RESULT_LOW_B, ddrp_readl_byidx(DDRP_REG_OFFSET_WRLEVEL_RESULT_LOW_B));
	serial_debug("B chn high, reg offset %x value %x\n", DDRP_REG_OFFSET_WRLEVEL_RESULT_HIGH_B, ddrp_readl_byidx(DDRP_REG_OFFSET_WRLEVEL_RESULT_HIGH_B));
#endif
}

static void ddrp_hardware_calibration(void)
{
	unsigned int val;
	unsigned int timeout = 1000000;

	//ddrp_write_leveling_calibration();
	ddr_writel(0, DDRP_INNOPHY_TRAINING_CTRL);
	ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	debug("ddr training into!\n");
	ddr_writel(1, DDRP_INNOPHY_TRAINING_CTRL);
#if CONFIG_DDR_DW32
	do
	{
		val = ddr_readl(DDRP_INNOPHY_CALIB_DONE);
	} while (((val & 0xf) != 0xf) && timeout--);/*32bit ddr mode*/
#else
	do
	{
		val = ddr_readl(DDRP_INNOPHY_CALIB_DONE);
	} while (((val & 0xf) != 0x3) && timeout--);/*16bit ddr mode*/
#endif
	if(!timeout) {
		serial_debug("timeout:INNOPHY_CALIB_DONE %x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE));
		hang();
	}

	ddr_writel(ddr_readl(DDRP_INNOPHY_TRAINING_CTRL)&(~0x1), DDRP_INNOPHY_TRAINING_CTRL);
	{
		int reg1, reg2;
		serial_debug("ddrp rx hard calibration:\n");
		reg1 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL_RESULT1);
		reg2 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL_RESULT2);
		serial_debug("CALIB_AL: cyc %d, oph %d, dll %d\n", reg1, reg2&0x3, reg2>>3&0x1f);
		reg1 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH_RESULT1);
		reg2 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH_RESULT2);
		serial_debug("CALIB_AH: cyc %d, oph %d, dll %d\n", reg1, reg2&0x3, reg2>>3&0x1f);
		reg1 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL_RESULT1);
		reg2 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL_RESULT2);
		serial_debug("CALIB_BL: cyc %d, oph %d, dll %d\n", reg1, reg2&0x3, reg2>>3&0x1f);
		reg1 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH_RESULT1);
		reg2 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH_RESULT2);
		serial_debug("CALIB_BH: cyc %d, oph %d, dll %d\n", reg1, reg2&0x3, reg2>>3&0x1f);
	}
}

static void ddr_phy_cfg_driver_odt(void)
{
	/* ddr phy driver strength and  ODT config */

#ifdef CONFIG_T40A
	/* cmd */
	ddrp_reg_set_range(0xb0, 0, 5, 0x15);
	ddrp_reg_set_range(0xb1, 0, 5, 0x15);
	/* ck */
	ddrp_reg_set_range(0xb2, 0, 5, 0x18); //pull down
	ddrp_reg_set_range(0xb3, 0, 5, 0x18);//pull up

	/* DQ ODT config */
	u32 dq_odt = 0x6;
	/* Channel A */
	ddrp_reg_set_range(0xc0, 0, 5, dq_odt);
	ddrp_reg_set_range(0xc1, 0, 5, dq_odt);
	ddrp_reg_set_range(0xd0, 0, 5, dq_odt);
	ddrp_reg_set_range(0xd1, 0, 5, dq_odt);
	/* Channel B */
	ddrp_reg_set_range(0xe0, 0, 5, dq_odt);
	ddrp_reg_set_range(0xe1, 0, 5, dq_odt);
	ddrp_reg_set_range(0xf0, 0, 5, dq_odt);
	ddrp_reg_set_range(0xf1, 0, 5, dq_odt);

	/* driver strength config */
    	u32 dq_ds = 0x15;
	ddrp_reg_set_range(0xc2, 0, 5, dq_ds);//pull down
	ddrp_reg_set_range(0xc3, 0, 5, dq_ds);//pull up
	ddrp_reg_set_range(0xd2, 0, 5, dq_ds);
	ddrp_reg_set_range(0xd3, 0, 5, dq_ds);

	ddrp_reg_set_range(0xe2, 0, 5, dq_ds);
	ddrp_reg_set_range(0xe3, 0, 5, dq_ds);
	ddrp_reg_set_range(0xf2, 0, 5, dq_ds);
	ddrp_reg_set_range(0xf3, 0, 5, dq_ds);

#elif defined (CONFIG_T40XP)

	/* cmd */
	ddrp_reg_set_range(0xb0, 0, 5, 0xf);
	ddrp_reg_set_range(0xb1, 0, 5, 0xf);
	/* ck */
	ddrp_reg_set_range(0xb2, 0, 5, 0x11);//pull down
	ddrp_reg_set_range(0xb3, 0, 5, 0x11);//pull up

	/* DQ ODT config */
	u32 dq_odt = 0x3;
	/* Channel A */
	ddrp_reg_set_range(0xc0, 0, 5, dq_odt);
	ddrp_reg_set_range(0xc1, 0, 5, dq_odt);
	ddrp_reg_set_range(0xd0, 0, 5, dq_odt);
	ddrp_reg_set_range(0xd1, 0, 5, dq_odt);
	/* Channel B */
	ddrp_reg_set_range(0xe0, 0, 5, dq_odt);
	ddrp_reg_set_range(0xe1, 0, 5, dq_odt);
	ddrp_reg_set_range(0xf0, 0, 5, dq_odt);
	ddrp_reg_set_range(0xf1, 0, 5, dq_odt);

	/* driver strength config */
	u32 dq_ds = 0x11;
	ddrp_reg_set_range(0xc2, 0, 5, dq_ds);//pull down
	ddrp_reg_set_range(0xc3, 0, 5, dq_ds);//pull up
	ddrp_reg_set_range(0xd2, 0, 5, dq_ds);
	ddrp_reg_set_range(0xd3, 0, 5, dq_ds);

	ddrp_reg_set_range(0xe2, 0, 5, dq_ds);
	ddrp_reg_set_range(0xe3, 0, 5, dq_ds);
	ddrp_reg_set_range(0xf2, 0, 5, dq_ds);
	ddrp_reg_set_range(0xf3, 0, 5, dq_ds);
#elif defined (CONFIG_T40N)
	//driver cmd and ck
	//cmd
	ddrp_reg_set_range(0xb0, 0, 4, 0x5); //0x4
	ddrp_reg_set_range(0xb1, 0, 4, 0x5);
	//ck
	ddrp_reg_set_range(0xb2, 0, 4, 0x5); //pull down,0x5
	ddrp_reg_set_range(0xb3, 0, 4, 0x5);//pull up
	//dq ODT
	//ch A
	ddrp_reg_set_range(0xc0, 0, 5, 0x3);
	ddrp_reg_set_range(0xc1, 0, 5, 0x3);
	ddrp_reg_set_range(0xd0, 0, 5, 0x3);
	ddrp_reg_set_range(0xd1, 0, 5, 0x3);
	//chb B
	ddrp_reg_set_range(0xe0, 0, 4, 0x3);
	ddrp_reg_set_range(0xe1, 0, 4, 0x3);
	ddrp_reg_set_range(0xf0, 0, 4, 0x3);
	ddrp_reg_set_range(0xf1, 0, 4, 0x3);
	// driver strength
	ddrp_reg_set_range(0xc2, 0, 4, 0x5);//pull down
	ddrp_reg_set_range(0xc3, 0, 4, 0x5);//pull up
	ddrp_reg_set_range(0xd2, 0, 4, 0x5);
	ddrp_reg_set_range(0xd3, 0, 4, 0x5);

	ddrp_reg_set_range(0xe2, 0, 4, 0x5);
	ddrp_reg_set_range(0xe3, 0, 4, 0x5);
	ddrp_reg_set_range(0xf2, 0, 4, 0x5);
	ddrp_reg_set_range(0xf3, 0, 4, 0x5);
#endif
}

static void ddr_phy_init(void)
{
	u32 idx;
	u32  val;
#if 0
	/* bit[3]reset digital core; bit[2]reset analog logic; bit[0]Reset Initial status
	 * other reserved */
	val = ddr_readl(DDRP_INNOPHY_INNO_PHY_RST);
	val &= ~(0xff);
	mdelay(2);
	val |= 0x0d;
	ddr_writel(val, DDRP_INNOPHY_INNO_PHY_RST);
	ddr_phy_cfg_driver_odt();

	writel(0xc, 0xb3011000 + (0x15)*4);//default 0x4 cmd
	writel(0x1, 0xb3011000 + (0x16)*4);//default 0x0 ck

	writel(0xc, 0xb3011000 + (0x54)*4);//default 0x4  byte0 dq dll
	writel(0xc, 0xb3011000 + (0x64)*4);//default 0x4  byte1 dq dll
	writel(0xc, 0xb3011000 + (0x84)*4);//default 0x4  byte2 dq dll
	writel(0xc, 0xb3011000 + (0x94)*4);//default 0x4  byte3 dq dll

	writel(0x1, 0xb3011000 + (0x55)*4);//default 0x0  byte0 dqs dll
	writel(0x1, 0xb3011000 + (0x65)*4);//default 0x0  byte1 dqs dll
	writel(0x1, 0xb3011000 + (0x85)*4);//default 0x0  byte2 dqs dll
	writel(0x1, 0xb3011000 + (0x95)*4);//default 0x0  byte3 dqs dll

#ifdef CONFIG_DDR_TYPE_DDR3
	unsigned int i = 0;
	u32 value = 8;
	/* write leveling dq delay time config */
	//cmd
	for (i = 0; i <= 0x1e;i++) {
		ddr_writel(value, DDR_PHY_OFFSET + (0x100+i)*4);///cmd
	}
	//tx DQ
	for (i = 0; i <= 0x8;i++) {
		ddr_writel(value, DDR_PHY_OFFSET + (0x120+i)*4);//DQ0-DQ15
		ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+i)*4);//DQ15-DQ31
	}
	//tx DQ
	for (i = 0xb; i <= 0x13;i++) {
		ddr_writel(value, DDR_PHY_OFFSET + (0x120+i)*4);//DQ0-DQ15
		ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+i)*4);//DQ15-DQ31
	}
	/* rx dqs */
	ddr_writel(5, DDR_PHY_OFFSET + (0x120+0x29)*4);//DQS0-A
	ddr_writel(5, DDR_PHY_OFFSET + (0x1a0+0x29)*4);//DQS0-B
	ddr_writel(5, DDR_PHY_OFFSET + (0x120+0x34)*4);//DQS1-A
	ddr_writel(5, DDR_PHY_OFFSET + (0x1a0+0x34)*4);//DQS1-B
	//enable bypass write leveling
	//open manual per bit de-skew
	//serial_debug("PHY REG-02 :  0x%x \n",readl(0xb3011008));
	writel((readl(0xb3011008))|(0x8), 0xb3011008);
	//serial_debug("PHY REG-02 :  0x%x \n",readl(0xb3011008));

	dump_ddr_phy_cfg_per_bit_de_skew_register();

#ifdef CONFIG_DDR_DEBUG
	//writel(0x73, 0xb3011000+(0x88*4));
	//writel(0x73, 0xb3011000+(0x98*4));
	serial_debug("--------inno a dll reg:0x58 = 0x%x\n", readl(0xb3011000+(0x58*4)));
	serial_debug("--------inno a dll reg:0x68 = 0x%x\n", readl(0xb3011000+(0x68*4)));
	serial_debug("--------inno b dll reg:0x88 = 0x%x\n", readl(0xb3011000+(0x88*4)));
	serial_debug("--------inno b dll reg:0x98 = 0x%x\n", readl(0xb3011000+(0x98*4)));
#endif
#elif defined(CONFIG_DDR_TYPE_DDR2)
	unsigned int i = 0;
	u32 value = 5;
	/* write leveling dq delay time config */
	//cmd
	for (i = 0; i <= 0x1e;i++) {
		ddr_writel(value, DDR_PHY_OFFSET + (0x100+i)*4);///cmd
	}
	//tx DQ
	for (i = 0; i <= 0x8;i++) {
		ddr_writel(value, DDR_PHY_OFFSET + (0x120+i)*4);//DQ0-DQ15
		ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+i)*4);//DQ15-DQ31
	}
	//tx DQ
	for (i = 0xb; i <= 0x13;i++) {
		ddr_writel(value, DDR_PHY_OFFSET + (0x120+i)*4);//DQ0-DQ15
		ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+i)*4);//DQ15-DQ31
	}
	ddr_writel(2, DDR_PHY_OFFSET + (0x120+0x9)*4);//DQS0-A
	ddr_writel(2, DDR_PHY_OFFSET + (0x1a0+0x9)*4);//DQS0-B
	ddr_writel(2, DDR_PHY_OFFSET + (0x120+0xa)*4);//DQS0B-A
	ddr_writel(2, DDR_PHY_OFFSET + (0x1a0+0xa)*4);//DQS0B-B
	ddr_writel(2, DDR_PHY_OFFSET + (0x120+0x14)*4);//DQS1-A
	ddr_writel(2, DDR_PHY_OFFSET + (0x1a0+0x14)*4);//DQS1-B
	ddr_writel(2, DDR_PHY_OFFSET + (0x120+0x15)*4);//DQS1B-A
	ddr_writel(2, DDR_PHY_OFFSET + (0x1a0+0x15)*4);//DQS1B-B

	writel((readl(0xb3011008))|(0x8), 0xb3011008);
	//serial_debug("PHY REG-02 :  0x%x \n",readl(0xb3011008));
#endif
#endif
	ddrp_pll_init();
}
/*
 * Name     : ddrp_calibration_manual()
 * Function : control the RX DQS window delay to the DQS
 *
 * a_low_8bit_delay	= al8_2x * clk_2x + al8_1x * clk_1x;
 * a_high_8bit_delay	= ah8_2x * clk_2x + ah8_1x * clk_1x;
 *
 * */
static void ddrp_software_calibration(void)
{
	int x, y;
	int w, z;
	int c, o, d;
	unsigned int addr = 0xa1000000, val;
	unsigned int i, n, m = 0;
	unsigned int sel = 0;
	int ret = -1;
	int j, k, q;

	unsigned int d0 = 0, d1 = 0, d2 = 0, d3 = 0, d4 = 0, d5 = 0, d6 = 0, d7 = 0;
	unsigned int ddbuf[8] = {0};
	unsigned short calv[0x3ff] = {0};
	ddrp_reg_set_range(0x2, 1, 1, 1);
	serial_debug("BEFORE A CALIB\n");
	serial_debug("CALIB DELAY AL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL1));
	serial_debug("CALIB DELAY AL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL2));
	serial_debug("CALIB DELAY AH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH1));
	serial_debug("CALIB DELAY AH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH2));
	serial_debug("CALIB DELAY BL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL1));
	serial_debug("CALIB DELAY BL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL2));
	serial_debug("CALIB DELAY BH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH1));
	serial_debug("CALIB DELAY BH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH2));
#if 1
	for(c = 0; c <=0x3ff; c++) {
		ddr_writel((c>>7)&0x7, DDRP_INNOPHY_CALIB_DELAY_AL1);
		ddr_writel((c&0x1f)<<3 | ((c>>5)&0x3), DDRP_INNOPHY_CALIB_DELAY_AL2);
		ddr_writel((c>>7)&0x7, DDRP_INNOPHY_CALIB_DELAY_AH1);
		ddr_writel((c&0x1f)<<3 | ((c>>5)&0x3), DDRP_INNOPHY_CALIB_DELAY_AH2);
		unsigned int value = 0x12345678;
		for(i = 0; i < 4 * 1024; i += 8) {
			*(volatile unsigned int *)(addr + (i + 0) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 1) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 2) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 3) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 4) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 5) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 6) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 7) * 4) = value;
		}
		for(i = 0; i < 4 * 1024; i += 8) {
			ddbuf[0] = *(volatile unsigned int *)(addr + (i + 0) * 4);
			ddbuf[1] = *(volatile unsigned int *)(addr + (i + 1) * 4);
			ddbuf[2] = *(volatile unsigned int *)(addr + (i + 2) * 4);
			ddbuf[3] = *(volatile unsigned int *)(addr + (i + 3) * 4);
			ddbuf[4] = *(volatile unsigned int *)(addr + (i + 4) * 4);
			ddbuf[5] = *(volatile unsigned int *)(addr + (i + 5) * 4);
			ddbuf[6] = *(volatile unsigned int *)(addr + (i + 6) * 4);
			ddbuf[7] = *(volatile unsigned int *)(addr + (i + 7) * 4);
			for(q = 0; q < 8; q++) {
				if ((ddbuf[q]&0xffff0000) != (value&0xffff0000)) {
					;//serial_debug("#####################################   high error want 0x%x get 0x%x\n", value, ddbuf[q]);
				}
				if ((ddbuf[q]&0xffff) != (value&0xffff)) {
					//serial_debug("SET AL,AH %x q[%d] fail want 0x%x  get 0x%x \n", c, q, value, ddbuf[q]);
					ret = -1;
					break;
				} else {
					//serial_debug("SET AL,AH %x q[%d] pass want 0x%x  get 0x%x \n", c, q, value, ddbuf[q]);
					//serial_debug("SET %d  AL[%d] pass want 0x%x  get 0x%x \n", c, q, value, ddbuf[q]);
					ret = 0;
				}
			}
			if (ret) {
				break;
			}
		}

		if(i == 4 * 1024) {
			calv[m] = c;
			m++;
			serial_debug("calib a once idx = %d,  value = %x\n", m, c);
		}
	}

	if(!m) {
		serial_debug("####################### AL a calib bypass fail\n");
		ddr_writel(0x1c, DDRP_INNOPHY_CALIB_DELAY_AL1);
		ddr_writel(0x1c, DDRP_INNOPHY_CALIB_DELAY_AH1);
		//		return;
	} else {
		/* choose the middle parameter */
		sel = m * 1 / 2;
		ddr_writel((calv[sel]>>7)&0x7, DDRP_INNOPHY_CALIB_DELAY_AL1);
		ddr_writel((calv[sel]&0x1f)<<3 | ((calv[sel]>>5)&0x3), DDRP_INNOPHY_CALIB_DELAY_AL2);
		ddr_writel((calv[sel]>>7)&0x7, DDRP_INNOPHY_CALIB_DELAY_AH1);
		ddr_writel((calv[sel]&0x1f)<<3 | ((calv[sel]>>5)&0x3), DDRP_INNOPHY_CALIB_DELAY_AH2);
	}
	serial_debug("calib a done range = %d, value = %x\n", m, calv[sel]);
	serial_debug("AFTER A CALIB\n");
	serial_debug("CALIB DELAY AL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL1));
	serial_debug("CALIB DELAY AL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL2));
	serial_debug("CALIB DELAY AH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH1));
	serial_debug("CALIB DELAY AH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH2));
	serial_debug("CALIB DELAY BL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL1));
	serial_debug("CALIB DELAY BL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL2));
	serial_debug("CALIB DELAY BH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH1));
	serial_debug("CALIB DELAY BH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH2));
#endif
#if CONFIG_DDR_DW32
#if 1
	m = 0;
	for(c = 0; c < 0x3ff; c ++) {
		ddr_writel((c>>7)&0x7, DDRP_INNOPHY_CALIB_DELAY_BL1);
		ddr_writel((c&0x1f)<<3 | ((c>>5)&0x3), DDRP_INNOPHY_CALIB_DELAY_BL2);
		ddr_writel((c>>7)&0x7, DDRP_INNOPHY_CALIB_DELAY_BH1);
		ddr_writel((c&0x1f)<<3 | ((c>>5)&0x3), DDRP_INNOPHY_CALIB_DELAY_BH2);
		unsigned int value = 0xf0f0f0f0;
		for(i = 0; i < 4 * 1024; i += 8) {
			*(volatile unsigned int *)(addr + (i + 0) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 1) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 2) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 3) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 4) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 5) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 6) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 7) * 4) = value;
			//flush_cache((unsigned int *)(addr + i * 4), 32);
		}

		for(i = 0; i < 4 * 1024; i += 8) {
			ddbuf[0] = *(volatile unsigned int *)(addr + (i + 0) * 4);
			ddbuf[1] = *(volatile unsigned int *)(addr + (i + 1) * 4);
			ddbuf[2] = *(volatile unsigned int *)(addr + (i + 2) * 4);
			ddbuf[3] = *(volatile unsigned int *)(addr + (i + 3) * 4);
			ddbuf[4] = *(volatile unsigned int *)(addr + (i + 4) * 4);
			ddbuf[5] = *(volatile unsigned int *)(addr + (i + 5) * 4);
			ddbuf[6] = *(volatile unsigned int *)(addr + (i + 6) * 4);
			ddbuf[7] = *(volatile unsigned int *)(addr + (i + 7) * 4);

			for(q = 0; q < 8; q++) {

				if ((ddbuf[q]&0xffff0000) != (value&0xffff0000)) {
					//serial_debug("SET BL,BH 0x%x q[%d] fail want 0x%x  get 0x%x \n", c, q, value, ddbuf[q]);
					ret = -1;
					break;
				} else {
					//serial_debug("SET BL,BH 0x%x q[%d] pass want 0x%x  get 0x%x \n", c, q, value, ddbuf[q]);
					ret = 0;
				}
			}
			if (ret) {
				break;
			}
		}

		if(i == 4 * 1024) {
			calv[m] = c;
			m++;
			serial_debug("calib b once idx = %d,  value = %x\n", m, c);
		}
	}

	if(!m) {
		serial_debug("####################### AL calib bypass fail\n");
		ddr_writel(0x1c, DDRP_INNOPHY_CALIB_DELAY_BL1);
		ddr_writel(0x1c, DDRP_INNOPHY_CALIB_DELAY_BH1);

		//return;
	} else {
		/* choose the middle parameter */
		sel = m/2;
		ddr_writel((calv[sel]>>7)&0x7, DDRP_INNOPHY_CALIB_DELAY_BL1);
		ddr_writel((calv[sel]&0x1f)<<3 | ((calv[sel]>>5)&0x3), DDRP_INNOPHY_CALIB_DELAY_BL2);
		ddr_writel((calv[sel]>>7)&0x7, DDRP_INNOPHY_CALIB_DELAY_BH1);
		ddr_writel((calv[sel]&0x1f)<<3 | ((calv[sel]>>5)&0x3), DDRP_INNOPHY_CALIB_DELAY_BH2);
	}
	serial_debug("calib b done range = %d, value = %x\n", m, calv[sel]);
	serial_debug("AFTER B CALIB\n");

	serial_debug("CALIB DELAY AL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL1));
	serial_debug("CALIB DELAY AL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL2));
	serial_debug("CALIB DELAY AH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH1));
	serial_debug("CALIB DELAY AH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH2));
	serial_debug("CALIB DELAY BL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL1));
	serial_debug("CALIB DELAY BL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL2));
	serial_debug("CALIB DELAY BH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH1));
	serial_debug("CALIB DELAY BH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH2));

	{
		int reg1, reg2;
		serial_debug("ddrp rx soft calibration:\n");
		reg1 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL1);
		reg2 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL2);
		serial_debug("CALIB_AL: cyc %d, oph %d, dll %d\n", reg1, reg2&0x3, reg2>>3&0x1f);
		reg1 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH1);
		reg2 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH2);
		serial_debug("CALIB_AH: cyc %d, oph %d, dll %d\n", reg1, reg2&0x3, reg2>>3&0x1f);
		reg1 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL1);
		reg2 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL2);
		serial_debug("CALIB_BL: cyc %d, oph %d, dll %d\n", reg1, reg2&0x3, reg2>>3&0x1f);
		reg1 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH1);
		reg2 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH2);
		serial_debug("CALIB_BH: cyc %d, oph %d, dll %d\n", reg1, reg2&0x3, reg2>>3&0x1f);
	}
#endif
#endif/*end if CONFIG_DDR_DW32 */
}

static void ddrc_dfi_init(enum ddr_type type, int bypass)
{
	FUNC_ENTER();
#ifdef CONFIG_DDR_BUSWIDTH_32
	ddr_writel((DDRC_DWCFG_DFI_INIT_START | 1), DDRC_DWCFG); // dfi_init_start high 0x09
	ddr_writel(1, DDRC_DWCFG); // set buswidth 32bit
#else
	ddr_writel((DDRC_DWCFG_DFI_INIT_START), DDRC_DWCFG); // dfi_init_start high ddr buswidth 16bit
	ddr_writel(0, DDRC_DWCFG); // set buswidth 16bit
#endif
	while(!(ddr_readl(DDRC_DWSTATUS) & DDRC_DWSTATUS_DFI_INIT_COMP)); //polling dfi_init_complete

	udelay(50);
	ddr_writel(0, DDRC_CTRL); //set dfi_reset_n high
	ddr_writel(DDRC_CFG_VALUE, DDRC_CFG);
	udelay(500);
	ddr_writel(DDRC_CTRL_CKE, DDRC_CTRL); // set CKE to high
	udelay(10);

	switch(type) {
		case DDR2:
#define DDRC_LMR_MR(n)												\
			1 << 1 | DDRC_LMR_START | DDRC_LMR_CMD_LMR |		\
			((DDR_MR##n##_VALUE & 0x1fff) << DDRC_LMR_DDR_ADDR_BIT) |	\
			(((DDR_MR##n##_VALUE >> 13) & 0x3) << DDRC_LMR_BA_BIT)
			while (ddr_readl(DDRC_LMR) & (1 << 0));
			ddr_writel(0x400003, DDRC_LMR);
			udelay(100);
			ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //MR2
			udelay(5);
			ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //MR3
			udelay(5);
			ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //MR1
			udelay(5);
			ddr_writel(DDRC_LMR_MR(0), DDRC_LMR); //MR0
			udelay(5 * 1000);
			ddr_writel(0x400003, DDRC_LMR);
			udelay(100);
			ddr_writel(0x43, DDRC_LMR);
			udelay(5);
			ddr_writel(0x43, DDRC_LMR);
			udelay(5 * 1000);
#undef DDRC_LMR_MR
			break;
		case LPDDR2:
#define DDRC_LMR_MR(n)													\
		1 << 1| DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |	\
			((DDR_MR##n##_VALUE & 0xff) << 24) |						\
			(((DDR_MR##n##_VALUE >> 8) & 0xff) << (16))
		ddr_writel(DDRC_LMR_MR(63), DDRC_LMR); //set MRS reset
		ddr_writel(DDRC_LMR_MR(10), DDRC_LMR); //set IO calibration
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //set MR1
		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //set MR2
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //set MR3
#undef DDRC_LMR_MR
		break;
	case DDR3:
#define DDRC_LMR_MR(n)												\
		DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |		\
		((DDR_MR##n##_VALUE & 0xffff) << DDRC_LMR_DDR_ADDR_BIT) |	\
			(((DDR_MR##n##_VALUE >> 16) & 0x7) << DDRC_LMR_BA_BIT)

		ddr_writel(0, DDRC_LMR);udelay(5);
		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //MR2
		serial_debug("mr2: %x\n", DDRC_LMR_MR(2));
		udelay(5);
		ddr_writel(0, DDRC_LMR);udelay(5);
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //MR3
		serial_debug("mr3: %x\n", DDRC_LMR_MR(3));
		udelay(5);
		ddr_writel(0, DDRC_LMR);udelay(5);
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //MR1
		serial_debug("mr1: %x\n", DDRC_LMR_MR(1));
		udelay(5);
		ddr_writel(0, DDRC_LMR);udelay(5);
		ddr_writel(DDRC_LMR_MR(0), DDRC_LMR); //MR0
		serial_debug("mr0: %x\n", DDRC_LMR_MR(0));
		udelay(5);
		//ddr_writel(DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_ZQCL_CS0, DDRC_LMR); //ZQCL
		udelay(5);

#undef DDRC_LMR_MR
		break;
	default:
		ddr_hang();
	}

	FUNC_EXIT();
}

struct ddr_calib_value {
	unsigned int rate;
	unsigned int refcnt;
	unsigned char bypass_al;
	unsigned char bypass_ah;
};

#define REG32(addr) *(volatile unsigned int *)(addr)
#define CPM_DDRCDR (0xb000002c)

static void ddr_calibration(struct ddr_calib_value *dcv, int div)
{
	unsigned int val;

	// Set change_en
	val = REG32(CPM_DDRCDR);
	val |= ((1 << 29) | (1 << 25));
	REG32(CPM_DDRCDR) = val;
	while((REG32(CPM_DDRCDR) & (1 << 24)))
		;
	/* // Set clock divider */
	val = REG32(CPM_DDRCDR);
	val &= ~(0xf);
	val |= div;
	REG32(CPM_DDRCDR) = val;

	// Polling PHY_FREQ_DONE
	while(((ddr_readl(DDRC_DWSTATUS) & (1 << 3 | 1 << 1)) & 0xf) != 0xa);
	ddrp_hardware_calibration();
	/* ddrp_software_calibration(); */

	dcv->bypass_al = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL1);
	/* serial_debug("auto :CALIB_AL: dcv->bypss_al %x\n", dcv->bypass_al); */
	dcv->bypass_ah = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH1);
	/* serial_debug("auto:CAHIB_AH: dcv->bypss_ah %x\n", dcv->bypass_ah); */

	// Set Controller Freq Exit
	val = ddr_readl(DDRC_DWCFG);
	val |= (1 << 2);
	ddr_writel(val, DDRC_DWCFG);

	// Clear Controller Freq Exit
	val = ddr_readl(DDRC_DWCFG);
	val &= ~(1 << 2);
	ddr_writel(val, DDRC_DWCFG);

	val = REG32(CPM_DDRCDR);
	val &= ~((1 << 29) | (1 << 25));
	REG32(CPM_DDRCDR) = val;
}

static void get_dynamic_calib_value(unsigned int rate)
{
	struct ddr_calib_value *dcv;
	unsigned int drate = 0;
	int div, n, cur_div;
#define CPU_TCSM_BASE (0xb2400000)
	dcv = (struct ddr_calib_value *)(CPU_TCSM_BASE + 2048);
	cur_div = REG32(CPM_DDRCDR) & 0xf;
	div = cur_div + 1;
	do {
		drate = rate / (div + 1);
		if(drate < 100000000) {
			dcv[cur_div].rate = rate;
			dcv[cur_div].refcnt = get_refcnt_value(cur_div);
			ddr_calibration(&dcv[cur_div], cur_div);
			break;
		}
		dcv[div].rate = drate;
		dcv[div].refcnt = get_refcnt_value(div);
		ddr_calibration(&dcv[div], div);
		div ++;
	} while(1);

	/* for(div = 6, n = 0; div > 0; div--, n++) { */
	/* 	dcv[div - 1].rate = rate / div; */
	/* 	if(dcv[div - 1].rate < 100000000) */
	/* 		break; */
	/* 	dcv[div - 1].refcnt = get_refcnt_value(div); */
	/* 	get_calib_value(&dcv[div - 1], div); */
	/* } */
}

static void ddrp_set_per_bit_de_skew(void)
{
	writel(0x14, 0xb3011000+((0x1c0+0x12)*4));//RX DQS0
	writel(0x14, 0xb3011000+((0x1c0+0x27)*4));//RX DQS1
	writel(0x14, 0xb3011000+((0x1c0+0x2a)*4));//RX DQSB0
	writel(0x14, 0xb3011000+((0x1c0+0x2b)*4));//RX DQSB1

	writel(0x14, 0xb3011000+((0x220+0x12)*4));//RX DQS0
	writel(0x14, 0xb3011000+((0x220+0x27)*4));//RX DQS1
	writel(0x14, 0xb3011000+((0x220+0x2a)*4));//RX DQSB0
	writel(0x14, 0xb3011000+((0x220+0x2b)*4));//RX DQSB1
}

static void ddrp_try_per_bit_de_skew(void)
{
	int value = 0;
	int channel = 0;
	unsigned int reg_offset[4] = {0x1c0, 0x220};
	unsigned int offset;;
    dump_ddrp_per_bit_de_skew();
	for (value = 0; value < 0x3f; value++) {
		for (channel = 0; channel < 2; channel++) {
			offset = reg_offset[channel];
#if 0
			writel(value, 0xb3011000+((offset+0x0)*4));//RX DM0
			writel(value, 0xb3011000+((offset+0x2)*4));
			writel(value, 0xb3011000+((offset+0x4)*4));
			writel(value, 0xb3011000+((offset+0x6)*4));
			writel(value, 0xb3011000+((offset+0x8)*4));
			writel(value, 0xb3011000+((offset+0xa)*4));
			writel(value, 0xb3011000+((offset+0x8)*4));
			writel(value, 0xb3011000+((offset+0xe)*4));
			writel(value, 0xb3011000+((offset+0x10)*4));//RX DQ7

#endif
			writel(value, 0xb3011000+((offset+0x12)*4));//RX DQS0
#if 0
			writel(value, 0xb3011000+((offset+0x15)*4));//RX DM1
			writel(value, 0xb3011000+((offset+0x17)*4));
			writel(value, 0xb3011000+((offset+0x19)*4));
			writel(value, 0xb3011000+((offset+0x1b)*4));
			writel(value, 0xb3011000+((offset+0x1d)*4));
			writel(value, 0xb3011000+((offset+0x1f)*4));
			writel(value, 0xb3011000+((offset+0x21)*4));
			writel(value, 0xb3011000+((offset+0x23)*4));
			writel(value, 0xb3011000+((offset+0x25)*4));//RX DQ15
#endif
			writel(value, 0xb3011000+((offset+0x27)*4));//RX DQS1
			writel(value, 0xb3011000+((offset+0x2a)*4));//RX DQSB0
			writel(value, 0xb3011000+((offset+0x2b)*4));//RX DQSB1
		}
		{
			int j = 0;
			unsigned int addr;
			unsigned int value_get;
			unsigned int data;
			unsigned int test_size =  64;
			unsigned int test_data =  0xffffffff;
			serial_debug("try rx skew %x\n", value);
			int isok = 1;
			for (j = 0; j < test_size; j+=4) {
				data = (j/4)%2?test_data:0;
				*(volatile unsigned int *)(addr+j) = data;
			}
			addr = 0xa1000000;
			for (j = 0; j < test_size; j+=4) {
				data = (j/4)%2?test_data:0;
				value_get = *(volatile unsigned int *)(addr+j);
				if (value_get != data) {
					serial_debug("memtest rx skew %x  addr 0x%x  want 0x%x get 0x%x error 0x%x\n",
							value, addr+j, data, value_get, data^value_get);
					isok = 0;
				}
			}
			if (isok)
				serial_debug("try rx skew %x success.\n", value);
			else
				serial_debug("try rx skew %x failed.\n", value);
		}
	}

    dump_ddrp_per_bit_de_skew();
}


int ddrp_set_rfifo()
{
	ddrp_reg_set_range(0x1, 5, 1, 1);
	ddrp_reg_set_range(0xe, 0, 3, 3);
	return 0;
}

int ddrp_set_rx_vref()
{
    serial_debug("ddrp vref AL: %x\n", ddrp_readl_byidx(0x140+0x07));
    serial_debug("ddrp vref AH: %x\n", ddrp_readl_byidx(0x140+0x17));
    serial_debug("ddrp vref BL: %x\n", ddrp_readl_byidx(0x160+0x07));
    serial_debug("ddrp vref BH: %x\n", ddrp_readl_byidx(0x160+0x07));

	//A_L
	ddrp_reg_set_range(0x140+0x07, 0, 8, 0x80);
	//A_H
	ddrp_reg_set_range(0x140+0x17, 0, 8, 0x80);
	//B_L
	ddrp_reg_set_range(0x160+0x07, 0, 8, 0x80);
	//B_H
	ddrp_reg_set_range(0x160+0x17, 0, 8, 0x80);

    serial_debug("ddrp vref AL: %x\n", ddrp_readl_byidx(0x140+0x07));
    serial_debug("ddrp vref AH: %x\n", ddrp_readl_byidx(0x140+0x17));
    serial_debug("ddrp vref BL: %x\n", ddrp_readl_byidx(0x160+0x07));
    serial_debug("ddrp vref BH: %x\n", ddrp_readl_byidx(0x160+0x07));
	return 0;
}

/* DDR sdram init */
void sdram_init(void)
{
	enum ddr_type type;
	unsigned int rate;
	int bypass = 0;

	type = get_ddr_type();
	debug("ddr type is =%d\n",type);
#ifndef CONFIG_FPGA
	clk_set_rate(DDR, gd->arch.gi->ddrfreq);
	/*if(ddr_hook && ddr_hook->prev_ddr_init)*/
		/*ddr_hook->prev_ddr_init(type);*/
	rate = clk_get_rate(DDR);
#else
	rate = gd->arch.gi->ddrfreq;
#endif

#ifndef CONFIG_FAST_BOOT
    serial_debug("DDR clk rate: %d\n\n", rate);
#endif

	ddrc_reset_phy();
	//mdelay(20);
	ddr_phy_init();
	ddrc_dfi_init(type, bypass);
	ddrc_prev_init();
#ifdef CONFIG_DDR_HARDWARE_TRAINING
	ddrp_hardware_calibration();
#endif
#ifdef	CONFIG_DDR_SOFT_TRAINING
	ddrp_software_calibration();
#endif

#ifdef CONFIG_DDR_DEBUG
    dump_ddrp_driver_strength_register();

	serial_debug("DDRP_INNOPHY_INNO_PHY 0X1d0    0x%x\n", ddr_readl(DDR_PHY_OFFSET + 0x1d0));
	serial_debug("DDRP_INNOPHY_INNO_PHY 0X1d4    0x%x\n", ddr_readl(DDR_PHY_OFFSET + 0x1d4));
	serial_debug("DDRP_INNOPHY_INNO_PHY 0X290    0x%x\n", ddr_readl(DDR_PHY_OFFSET + 0x290));
	serial_debug("DDRP_INNOPHY_INNO_PHY 0X294    0x%x\n", ddr_readl(DDR_PHY_OFFSET + 0x294));
	serial_debug("DDRP_INNOPHY_INNO_PHY 0X1c0    0x%x\n", ddr_readl(DDR_PHY_OFFSET + 0x1c0));
	serial_debug("DDRP_INNOPHY_INNO_PHY 0X280    0x%x\n", ddr_readl(DDR_PHY_OFFSET + 0x280));
#endif

	ddrc_post_init();

	/*get_dynamic_calib_value(rate);*/

	if(DDRC_AUTOSR_EN_VALUE) {
		ddr_writel(DDRC_AUTOSR_CNT_VALUE, DDRC_AUTOSR_CNT);
		ddr_writel(1, DDRC_AUTOSR_EN);
	} else {
		ddr_writel(DDRC_AUTOSR_CNT_VALUE, DDRC_AUTOSR_CNT);
		ddr_writel(0, DDRC_AUTOSR_EN);
	}

	ddr_writel(DDRC_HREGPRO_VALUE, DDRC_HREGPRO);
	ddr_writel(DDRC_PREGPRO_VALUE, DDRC_PREGPRO);

	ddrp_set_drv_odt();
	//ddrp_set_rx_vref();
	ddrp_set_rfifo();

#ifdef CONFIG_DDR_DEBUG
	dump_ddrc_register();
	dump_ddrp_register();
	dump_ddrp_driver_strength_register();
#endif

#if 0
	serial_debug("reselect ddr type\n");
#ifdef CONFIG_DDR_TYPE_DDR3
	//serial_debug("PHY REG-01 :  0x%x \n", ddrp_reg_get(0x1));
	ddrp_reg_set_range(0x1, 6, 1, 1);
	//serial_debug("PHY REG-01 :  0x%x \n", ddrp_reg_get(0x1));
#elif defined(CONFIG_DDR_TYPE_DDR2)
	//serial_debug("PHY REG-01 :  0x%x \n",readl(0xb3011004));
	writel(0x51,0xb3011004);
	//serial_debug("PHY REG-01 :  0x%x \n",readl(0xb3011004));
#endif
	//fifo need set reg 0x01 bit6  to 1
	//serial_debug("PHY REG-0xa :  0x%x \n", ddrp_reg_get(0xa));
	ddrp_reg_set_range(0xa, 1, 3, 3);
	//serial_debug("PHY REG-0xa :  0x%x \n", ddrp_reg_get(0xa));

	//TX Write Pointer adjust
	//serial_debug("PHY REG-0x8 :  0x%x \n", ddrp_reg_get(0x8));
	ddrp_reg_set_range(0x8, 0, 2, 3);
	//serial_debug("PHY REG-0x8 :  0x%x \n", ddrp_reg_get(0x8));

	//extend rx dqs gateing window
	//ddrp_reg_set_range(0x9, 7, 1, 1);

	//ddrp_software_writeleveling_rx();
#endif
	//ddrp_try_per_bit_de_skew();
    //dump_ddrp_per_bit_de_skew();
	debug("sdram init finished\n");
}


phys_size_t initdram(int board_type)
{
	/* SDRAM size was calculated when compiling. */
#ifndef EMC_LOW_SDRAM_SPACE_SIZE
#define EMC_LOW_SDRAM_SPACE_SIZE 0x20000000 /* 512M */
#endif /* EMC_LOW_SDRAM_SPACE_SIZE */

	unsigned int ram_size;
	ram_size = (unsigned int)(DDR_CHIP_0_SIZE) + (unsigned int)(DDR_CHIP_1_SIZE);

	if (ram_size > EMC_LOW_SDRAM_SPACE_SIZE)
		ram_size = EMC_LOW_SDRAM_SPACE_SIZE;

	return ram_size;
}
