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
#include <config.h>
#include <common.h>
#include <ddr/ddr_common.h>
#ifndef CONFIG_BURNER
#include <generated/ddr_reg_values.h>
#endif

#include <asm/io.h>
#include <asm/arch/clk.h>
#include "efuse_ddr.h"

/*#define CONFIG_DWC_DEBUG 0*/
#define ddr_hang() do{						\
		debug("%s %d\n",__FUNCTION__,__LINE__);	\
		hang();						\
	}while(0)

DECLARE_GLOBAL_DATA_PTR;
struct ddr_reg_value *global_reg_value __attribute__ ((section(".data")));

#ifdef  CONFIG_DWC_DEBUG
#define FUNC_ENTER() debug("%s enter.\n",__FUNCTION__);
#define FUNC_EXIT() debug("%s exit.\n",__FUNCTION__);


static void dump_ddrp_register(void)
{
	debug("DDRP_INNOPHY_PHY_RST		0x%x\n", ddr_readl(DDRP_INNOPHY_PHY_RST));
	debug("DDRP_INNOPHY_MEM_CFG		0x%x\n", ddr_readl(DDRP_INNOPHY_MEM_CFG));
	debug("DDRP_INNOPHY_DQ_WIDTH		0x%x\n", ddr_readl(DDRP_INNOPHY_DQ_WIDTH));
	debug("DDRP_INNOPHY_CL			0x%x\n", ddr_readl(DDRP_INNOPHY_CL));
	debug("DDRP_INNOPHY_CWL		0x%x\n", ddr_readl(DDRP_INNOPHY_CWL));
	debug("DDRP_INNOPHY_PLL_FBDIV		0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_FBDIV));
	debug("DDRP_INNOPHY_PLL_CTRL		0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_CTRL));
	debug("DDRP_INNOPHY_PLL_PDIV		0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_PDIV));
	debug("DDRP_INNOPHY_PLL_LOCK		0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_LOCK));
	debug("DDRP_INNOPHY_TRAINING_CTRL	0x%x\n", ddr_readl(DDRP_INNOPHY_TRAINING_CTRL));
	debug("DDRP_INNOPHY_CALIB_DONE		0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE));
	debug("DDRP_INNOPHY_CALIB_DELAY_AL	0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL));
	debug("DDRP_INNOPHY_CALIB_DELAY_AH	0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH));
	debug("DDRP_INNOPHY_CALIB_BYPASS_AL	0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AL));
	debug("DDRP_INNOPHY_CALIB_BYPASS_AH	0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AH));
	debug("DDRP_INNOPHY_INIT_COMP		0x%x\n", ddr_readl(DDRP_INNOPHY_INIT_COMP));
}

static void dump_ddrc_register(void)
{
	debug("DDRC_STATUS		 0x%x\n", ddr_readl(DDRC_STATUS));
	debug("DDRC_CFG			0x%x\n", ddr_readl(DDRC_CFG));
	debug("DDRC_CTRL		   0x%x\n", ddr_readl(DDRC_CTRL));
	debug("DDRC_LMR			0x%x\n", ddr_readl(DDRC_LMR));
	debug("DDRC_DLP			0x%x\n", ddr_readl(DDRC_DLP));
	debug("DDRC_TIMING1		0x%x\n", ddr_readl(DDRC_TIMING(1)));
	debug("DDRC_TIMING2		0x%x\n", ddr_readl(DDRC_TIMING(2)));
	debug("DDRC_TIMING3		0x%x\n", ddr_readl(DDRC_TIMING(3)));
	debug("DDRC_TIMING4		0x%x\n", ddr_readl(DDRC_TIMING(4)));
	debug("DDRC_TIMING5		0x%x\n", ddr_readl(DDRC_TIMING(5)));
	debug("DDRC_REFCNT		 0x%x\n", ddr_readl(DDRC_REFCNT));
	debug("DDRC_AUTOSR_CNT	 0x%x\n", ddr_readl(DDRC_AUTOSR_CNT));
	debug("DDRC_AUTOSR_EN	  0x%x\n", ddr_readl(DDRC_AUTOSR_EN));
	debug("DDRC_MMAP0		  0x%x\n", ddr_readl(DDRC_MMAP0));
	debug("DDRC_MMAP1		  0x%x\n", ddr_readl(DDRC_MMAP1));
	debug("DDRC_REMAP1		 0x%x\n", ddr_readl(DDRC_REMAP(1)));
	debug("DDRC_REMAP2		 0x%x\n", ddr_readl(DDRC_REMAP(2)));
	debug("DDRC_REMAP3		 0x%x\n", ddr_readl(DDRC_REMAP(3)));
	debug("DDRC_REMAP4		 0x%x\n", ddr_readl(DDRC_REMAP(4)));
	debug("DDRC_REMAP5		 0x%x\n", ddr_readl(DDRC_REMAP(5)));
	debug("DDRC_REMAP6		 0x%x\n", ddr_readl(DDRC_REMAP(6)));
	debug("DDRC_DWCFG		  0x%x\n", ddr_readl(DDRC_DWCFG));
	debug("DDRC_HREGPRO		0x%x\n", ddr_readl(DDRC_HREGPRO));
	debug("DDRC_PREGPRO		0x%x\n", ddr_readl(DDRC_PREGPRO));
	debug("DDRC_CGUC0		  0x%x\n", ddr_readl(DDRC_CGUC0));
	debug("DDRC_CGUC1		  0x%x\n", ddr_readl(DDRC_CGUC1));
}

#else
#define FUNC_ENTER()
#define FUNC_EXIT()

#define dump_ddrc_register()
#define dump_ddrp_register()
#endif

static void ddrp_set_drv_odt(unsigned int *ddr_drv_config)
{
	ddr_writel(ddr_drv_config[ODT_PD], DDRP_INNOPHY_PD_ODT_DQ7_0 );	// [7:0]ODT pull down
	ddr_writel(ddr_drv_config[ODT_PU], DDRP_INNOPHY_PU_ODT_DQ7_0 );	// [7:0]ODT pull up
	ddr_writel(ddr_drv_config[ODT_PD], DDRP_INNOPHY_PD_ODT_DQ15_8);	// [15:8]ODT pull down
	ddr_writel(ddr_drv_config[ODT_PU], DDRP_INNOPHY_PU_ODT_DQ15_8);	// [15:8]ODT pull up

	ddr_writel(ddr_drv_config[CMD_RC_PD], DDRP_INNOPHY_PD_DRV_CMD);	// CMD pull down
	ddr_writel(ddr_drv_config[CMD_RC_PU], DDRP_INNOPHY_PU_DRV_CMD);	// CMD pull up

	ddr_writel(ddr_drv_config[CLK_RC_PD], DDRP_INNOPHY_PD_DRV_CK);	 // CLK pull down
	ddr_writel(ddr_drv_config[CLK_RC_PU], DDRP_INNOPHY_PU_DRV_CK);	 // CLK pull up

	ddr_writel(ddr_drv_config[DQX_RC_PD], DDRP_INNOPHY_PD_DRV_DQ7_0 ); // [7:0]DQ pull down
	ddr_writel(ddr_drv_config[DQX_RC_PU], DDRP_INNOPHY_PU_DRV_DQ7_0 ); // [7:0]DQ pull up
	ddr_writel(ddr_drv_config[DQX_RC_PD], DDRP_INNOPHY_PD_DRV_DQ15_8); // [15:8]DQ pull down
	ddr_writel(ddr_drv_config[DQX_RC_PU], DDRP_INNOPHY_PU_DRV_DQ15_8); // [15:8]DQ pull up
}

static void ddr_set_vref_skew(unsigned int *ddr_drv_config)
{
	int DQxOFFSET_rx[] = {0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe, 0x10, 0x17, 0x19, 0x1b, 0x1d, 0x1f, 0x21, 0x23, 0x25}; // DQxxx RX寄存器偏移
	int DQxOFFSET_tx[] = {0x3, 0x5, 0x7, 0x9, 0xb, 0xd, 0xf, 0x11, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26}; // DQxxx TX寄存器偏移

	int i, wl;

	if (0 == ddr_drv_config[SKEW_TRX])
		return ;
	serial_debug("AD100 InnoPhy skew Settings...\n");

	// VREF A_L
	// ddrp_reg_set_range(0x147, 0, 8, ddr_drv_config[VREF]);
	writel(ddr_drv_config[VREF], 0xb3011000 + 0x147 * 4);
	// VREF A_H
	writel(ddr_drv_config[VREF], 0xb3011000 + 0x157 * 4);
	// ddrp_reg_set_range(0x157, 0, 8, ddr_drv_config[VREF]);

	/* RX */
	if (ddr_drv_config[SKEW_TRX] & 0x1)
	{
		writel(ddr_drv_config[SKEW_DQS0R], 0xb3011000 + (0x1c0 + 0x00) * 4); // RX DM0
		writel(ddr_drv_config[SKEW_DQS0R], 0xb3011000 + (0x1c0 + 0x12) * 4); // RX DQS0
		writel(ddr_drv_config[SKEW_DQS0R], 0xb3011000 + (0x1c0 + 0x2a) * 4); // RX DQSB0
		writel(ddr_drv_config[SKEW_DQS1R], 0xb3011000 + (0x1c0 + 0x15) * 4); // RX DM1
		writel(ddr_drv_config[SKEW_DQS1R], 0xb3011000 + (0x1c0 + 0x27) * 4); // RX DQS1
		writel(ddr_drv_config[SKEW_DQS1R], 0xb3011000 + (0x1c0 + 0x2b) * 4); // RX DQSB1

		for (i = 0; i < 16; i++)
		{
			writel(ddr_drv_config[SKEW_DQRX], 0xb3011000 + (0x1c0 + DQxOFFSET_rx[i]) * 4); // RX DQX
		}

	}
	/* TX */
	if (ddr_drv_config[SKEW_TRX] & 0x2)
	{
		/* TX BYPASS FUNCTION */
		wl = readl(0xb3011000 + (0x2) * 4);
		wl |= 0x8;
		writel(wl, 0xb3011000 + (0x2) * 4);

		writel(ddr_drv_config[SKEW_DQS0T], 0xb3011000 + (0x1c0 + 0x01) * 4); // TX DM0
		writel(ddr_drv_config[SKEW_DQS0T], 0xb3011000 + (0x1c0 + 0x13) * 4); // TX DQS0
		writel(ddr_drv_config[SKEW_DQS0T], 0xb3011000 + (0x1c0 + 0x14) * 4); // TX DQSB0
		writel(ddr_drv_config[SKEW_DQS1T], 0xb3011000 + (0x1c0 + 0x16) * 4); // TX DM1
		writel(ddr_drv_config[SKEW_DQS1T], 0xb3011000 + (0x1c0 + 0x28) * 4); // TX DQS1
		writel(ddr_drv_config[SKEW_DQS1T], 0xb3011000 + (0x1c0 + 0x29) * 4); // TX DASB1

		// cmd
		for (i = 0; i < 0x1d; i++)
		{
			writel(ddr_drv_config[SKEW_DQS0T], 0xb3011000 + (0x340 + i) * 4);
		}
		writel(ddr_drv_config[SKEW_DQS0T], 0xb3011000 + (0x340 + 0x1e) * 4);
		writel(ddr_drv_config[SKEW_DQS0T], 0xb3011000 + (0x340 + 0x1f) * 4);

		for (i = 0; i < 16; i++)
		{
			writel(ddr_drv_config[SKEW_DQTX], 0xb3011000 + (0x1c0 + DQxOFFSET_tx[i]) * 4);
		}
	}
}

static void ddrp_pll_init(void)
{
	ddr_writel(20, DDRP_INNOPHY_INVDELAYSEL_DQCMD);

	ddr_writel(0x0, DDRP_INNOPHY_PLL_FBDIV);
	ddr_writel(0x86, DDRP_INNOPHY_PLL_FBDIV_H);
	ddr_writel(0x41, DDRP_INNOPHY_PLL_PDIV);
	ddr_writel(0x20, DDRP_INNOPHY_PLL_CTRL);

	while(! (ddr_readl(DDRP_INNOPHY_PLL_LOCK) & (1 << 2)));

	debug("DDRP_INNOPHY_PLL_FBDIV_50	0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_FBDIV));
	debug("DDRP_INNOPHY_PLL_FBDIV_H_51	0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_FBDIV_H));
	debug("DDRP_INNOPHY_PLL_CTRL_53	0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_CTRL));
	debug("DDRP_INNOPHY_PLL_PDIV_52	0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_PDIV));
	debug("DDRP_INNOPHY_PLL_LOCK_60	0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_LOCK));
}



static void ddrp_reset(void)
{
	serial_debug("DDRP_INNOPHY_PHY_RST: %x\n", ddr_readl(DDRP_INNOPHY_PHY_RST));
	unsigned int val = ddr_readl(DDRP_INNOPHY_PHY_RST);
	ddr_writel(0, DDRP_INNOPHY_PHY_RST);
	udelay(10);
	ddr_writel(val, DDRP_INNOPHY_PHY_RST);
	udelay(10);
}

static void ddrp_cfg(struct ddr_reg_value *global_reg_value)
{
	unsigned int val;

	//ddrp_reset();

	ddr_writel(0, DDRP_INNOPHY_DQ_WIDTH_H);
	ddr_writel(DDRP_DQ_WIDTH_DQ_H | DDRP_DQ_WIDTH_DQ_L, DDRP_INNOPHY_DQ_WIDTH);
	ddr_writel(global_reg_value->DDRP_MEMCFG_VALUE, DDRP_INNOPHY_MEM_CFG);


	val = ddr_readl(DDRP_INNOPHY_CL);
	val &= ~(0xf);
	val |= global_reg_value->DDRP_CL_VALUE;
	ddr_writel(val, DDRP_INNOPHY_CL);

	val = ddr_readl(DDRP_INNOPHY_CWL);
	val &= ~(0xf);
	val |= global_reg_value->DDRP_CWL_VALUE;
	ddr_writel(val, DDRP_INNOPHY_CWL);

	val = ddr_readl(DDRP_INNOPHY_AL);
	val &= ~(0xf);
	ddr_writel(val, DDRP_INNOPHY_AL);

	debug("DDRP_INNOPHY_CL:	   %x\n", ddr_readl(DDRP_INNOPHY_CL));
	debug("DRP_INNOPHY_CWL:	  %x\n", ddr_readl(DDRP_INNOPHY_CWL));
	debug("DRP_INNOPHY_AL:	   %x\n", ddr_readl(DDRP_INNOPHY_AL));
	debug("DDRP_INNOPHY_MEM_CFG:	%x\n", ddr_readl(DDRP_INNOPHY_MEM_CFG));
}

/*
 * Name	 : ddrp_calibration()
 * Function : control the RX DQS window delay to the DQS
 *
 * a_low_8bit_delay	= al8_2x * clk_2x + al8_1x * clk_1x;
 * a_high_8bit_delay	= ah8_2x * clk_2x + ah8_1x * clk_1x;
 *
 * */
static void ddrp_rx_dqs_auto_calibration(void)
{
	unsigned int reg_val = ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	unsigned int timeout = 0xffffff;
#ifdef CONFIG_DDR_DRVODT_DEBUG
	timeout =0x50;
#endif
	debug("DDRP_INNOPHY_CALIB_MODE:	%x\n", ddr_readl(DDRP_INNOPHY_CALIB_MODE));

	reg_val &= ~(DDRP_TRAINING_CTRL_DSCSE_BP);
	reg_val |= DDRP_TRAINING_CTRL_DSACE_START;
	ddr_writel(reg_val, DDRP_INNOPHY_TRAINING_CTRL);

	while(!((ddr_readl(DDRP_INNOPHY_CALIB_DONE) & 0x3) == 3) && --timeout) {

		udelay(1);
#ifndef CONFIG_DDR_DRVODT_DEBUG
		debug("-----ddr_readl(DDRP_INNOPHY_CALIB_DONE): %x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE));
#endif
	}

	if(!timeout) {
		debug("ddrp_auto_calibration failed!\n");
#ifndef CONFIG_DDR_DRVODT_DEBUG
		while(1);
#endif
	}

	debug("ddrp_auto_calibration success!\n");

	debug("DDRP_INNOPHY_CALIB_DONE_61: %x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE));
	debug("DDRP_INNOPHY_CALIB_ERR_69:	%X\n", ddr_readl(DDRP_INNOPHY_CALIB_ERR));
	debug("DDRP_INNOPHY_CALIB_L_C_9b: %x\n", ddr_readl(DDRP_INNOPHY_CALIB_L_C));
	debug("DDRP_INNOPHY_CALIB_L_DO_9c: %x\n", ddr_readl(DDRP_INNOPHY_CALIB_L_DO));
	debug("DDRP_INNOPHY_CALIB_R_C_9d: %x\n", ddr_readl(DDRP_INNOPHY_CALIB_R_C));
	debug("DDRP_INNOPHY_CALIB_R_DO_9e: %x\n", ddr_readl(DDRP_INNOPHY_CALIB_R_DO));
	debug("DDRP_INNOPHY_CALIB_MODE:	%x\n", ddr_readl(DDRP_INNOPHY_CALIB_MODE));

	if(ddr_readl(DDRP_INNOPHY_CALIB_ERR) & (1 << 6)) {
		serial_debug("ddr pass but with error!\n");
		while(1);
	}

	ddr_writel(0, DDRP_INNOPHY_TRAINING_CTRL);
}


static void ddrp_post_init(void)
{

}

void ddrp_auto_calibration(unsigned int *ddr_drv_config)
{
    ddrp_set_drv_odt(ddr_drv_config);
    ddrp_rx_dqs_auto_calibration();
    ddr_set_vref_skew(ddr_drv_config);
}

// from ddr_innophy.c
static void mem_remap(void)
{
	int i;
	unsigned int *remap;

	remap = global_reg_value->REMMAP_ARRAY;


	for(i = 0;i < ARRAY_SIZE(global_reg_value->REMMAP_ARRAY);i++)
	{
		ddr_writel(remap[i], DDRC_REMAP(i+1));
	}
}

static enum ddr_type get_ddr_type(void)
{
	int type;

	type = global_reg_value->h.type;
	switch(global_reg_value->h.type){

		case DDR3:
			serial_debug("DDR: %s type is : DDR3\n", global_reg_value->h.name);
			break;
		case LPDDR:
			serial_debug("DDR: %s type is : LPDDR\n", global_reg_value->h.name);
			break;
		case LPDDR2:
			serial_debug("DDR: %s type is : LPDDR2\n", global_reg_value->h.name);
			break;
		case LPDDR3:
			serial_debug("DDR: %s type is : LPDDR3\n", global_reg_value->h.name);
			break;
		case DDR2:
			serial_debug("DDR: %s type is : DDR2\n", global_reg_value->h.name);
			break;
		default:
			type = UNKOWN;
			serial_debug("unsupport ddr type!\n");
			ddr_hang();
	}

	return type;
}
#if 0
static void ddrc_reset_phy(void)
{
	FUNC_ENTER();
	ddr_writel(0xf << 20, DDRC_CTRL);
	mdelay(1);
	ddr_writel(0x8 << 20, DDRC_CTRL);  //dfi_reset_n low for innophy
	mdelay(1);
	FUNC_EXIT();
}
#endif

static struct jzsoc_ddr_hook *ddr_hook = NULL;

void register_ddr_hook(struct jzsoc_ddr_hook * hook)
{
	ddr_hook = hook;
}


void ddrc_dfi_init(enum ddr_type type, unsigned int kgd_rtt_dic)
{
	unsigned int reg_val;
	FUNC_ENTER();

	reg_val = ddr_readl(DDRC_DWCFG);
	reg_val &= ~(1 << 3);
	ddr_writel(reg_val, DDRC_DWCFG); // set dfi_init_start low, and buswidth 16bit
	while(!(ddr_readl(DDRC_DWSTATUS) & DDRC_DWSTATUS_DFI_INIT_COMP)); //polling dfi_init_complete

	reg_val = ddr_readl(DDRC_CTRL);
	reg_val &= ~(1 << 23);
	ddr_writel(reg_val, DDRC_CTRL); //set dfi_reset_n high

	udelay(500);
	ddr_writel(global_reg_value->DDRC_CFG_VALUE, DDRC_CFG);
	ddr_writel(DDRC_CTRL_CKE, DDRC_CTRL); // set CKE to high
	udelay(500);


	switch(type) {
	case LPDDR2:
		udelay(200);
		ddr_writel(7 << 9 | 1 << 0, DDRC_LMR); //Send All bank precharge.
		mdelay(1);

#define DDRC_LMR_MR(n)														  \
				(global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |		\
		((global_reg_value->DDR_MR##n##_VALUE & 0xff) << 24)  |						   \
		(((global_reg_value->DDR_MR##n##_VALUE >> 8) & 0xff) << (16)))
		ddr_writel(DDRC_LMR_MR(63), DDRC_LMR); //set MRS reset
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(10), DDRC_LMR); //set IO calibration
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //set MR1
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //set MR2
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //set MR3
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(11), DDRC_LMR); //set MR11
		mdelay(1);

		break;

	case LPDDR3:
		udelay(200);	//200us delay before RST.
#define DDRC_LMR_MR(n)														  \
				(global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |		\
		((global_reg_value->DDR_MR##n##_VALUE & 0xff) << 24)  |						   \
		(((global_reg_value->DDR_MR##n##_VALUE >> 8) & 0xff) << (16)))
		ddr_writel(DDRC_LMR_MR(63), DDRC_LMR); //set MRS reset
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(10), DDRC_LMR); //set IO calibration
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //set MR1
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //set MR2
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //set MR3
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(11), DDRC_LMR); //set MR11
		mdelay(1);

		break;

	case DDR3:
		mdelay(1);
#define DDRC_LMR_MR(n)								\
		(global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |		   \
			((global_reg_value->DDR_MR##n##_VALUE & 0xffff) << DDRC_LMR_DDR_ADDR_BIT) |	   \
			(((global_reg_value->DDR_MR##n##_VALUE >> 16) & 0x7) << DDRC_LMR_BA_BIT))

		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //MR2
		udelay(10);
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //MR3
		udelay(10);
		ddr_writel((DDRC_LMR_MR(1) & (~0x266000)) | (kgd_rtt_dic << 12), DDRC_LMR); //MR1
		udelay(10);
		ddr_writel(DDRC_LMR_MR(0), DDRC_LMR); //MR0
		udelay(10);

#if CONFIG_DDR_CS0
		ddr_writel(global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_ZQCL_CS0, DDRC_LMR); //ZQCL
		mdelay(1);
#endif

#if CONFIG_DDR_CS1
		ddr_writel(global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_ZQCL_CS1, DDRC_LMR); //ZQCL
		mdelay(1);
#endif

#undef DDRC_LMR_MR
		break;

	case DDR2:

		udelay(200);
		ddr_writel(7 << 9 | 1 << 0, DDRC_LMR); //Send All bank precharge.
		mdelay(1);

#define DDRC_LMR_MR(n)											\
		(global_reg_value->DDRC_DLMR_VALUE | 1 << 1 | DDRC_LMR_START | DDRC_LMR_CMD_LMR |	\
			((global_reg_value->DDR_MR##n##_VALUE & 0x1fff) << DDRC_LMR_DDR_ADDR_BIT) |		\
			(((global_reg_value->DDR_MR##n##_VALUE >> 13) & 0x3) << DDRC_LMR_BA_BIT))

		while (ddr_readl(DDRC_LMR) & (1 << 0));
		ddr_writel(0x400003, DDRC_LMR);
		udelay(100);
		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //MR2
		udelay(5);
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //MR3
		udelay(5);
		ddr_writel((DDRC_LMR_MR(1) & (~0x46000)) | (kgd_rtt_dic << 12), DDRC_LMR); //MR1
		udelay(5);
		ddr_writel(DDRC_LMR_MR(0), DDRC_LMR); //MR0
		udelay(5 * 1000);
		ddr_writel(0x400003, DDRC_LMR);
		udelay(100);
		ddr_writel(0x43, DDRC_LMR);
		udelay(5);
		ddr_writel(0x43, DDRC_LMR);
		udelay(5 * 1000);

		udelay(200);
		ddr_writel(7 << 9 | 1 << 0, DDRC_LMR); //Send All bank precharge.
		mdelay(1);

		ddr_writel(1 << 3 | 1 << 0, DDRC_LMR); // send auto refresh.
		udelay(100);
		ddr_writel(1 << 3 | 1 << 0, DDRC_LMR); // send auto refresh.
		udelay(100);

#undef DDRC_LMR_MR
		break;

	default:
		ddr_hang();
	}
	FUNC_EXIT();
}

static void ddrc_prev_init(void)
{
	FUNC_ENTER();
	ddr_writel(global_reg_value->DDRC_TIMING1_VALUE, DDRC_TIMING(1));
	ddr_writel(global_reg_value->DDRC_TIMING2_VALUE, DDRC_TIMING(2));
	ddr_writel(global_reg_value->DDRC_TIMING3_VALUE, DDRC_TIMING(3));
	ddr_writel(global_reg_value->DDRC_TIMING4_VALUE, DDRC_TIMING(4));
	ddr_writel(global_reg_value->DDRC_TIMING5_VALUE, DDRC_TIMING(5));

	/* DDRC memory map configure*/
	ddr_writel(global_reg_value->DDRC_MMAP0_VALUE, DDRC_MMAP0);
	ddr_writel(global_reg_value->DDRC_MMAP1_VALUE, DDRC_MMAP1);
	mem_remap();

	/* 初始化时，关闭DDR自刷新功能. */
	ddr_writel(0, DDRC_AUTOSR_EN);
	ddr_writel(0, DDRC_AUTOSR_CNT);
	ddr_writel(0, DDRC_REFCNT);
	FUNC_EXIT();
}

static void ddrc_post_init(void)
{
	unsigned int reg = 0;
	FUNC_ENTER();

	debug("DDRC_STATUS: %x\n",ddr_readl(DDRC_STATUS));

	ddr_writel(global_reg_value->DDRC_AUTOSR_CNT_VALUE, DDRC_AUTOSR_CNT);
	ddr_writel(global_reg_value->DDRC_REFCNT_VALUE, DDRC_REFCNT);

	/*控制寄存器应该只修改配置相关的内容，需要读后写操作.*/
	reg = ddr_readl(DDRC_CTRL);
	/*X2000/X2500/X1600 该寄存器有差异. */
	reg |= global_reg_value->DDRC_CTRL_VALUE & (0xf << 12);
	ddr_writel(reg, DDRC_CTRL);

	reg = ddr_readl(DDRC_DWCFG);
	reg |= (1 << 5);	//PORT_OB_EN, 优化选项.
	ddr_writel(reg, DDRC_DWCFG);

	FUNC_EXIT();
}

void dump_generated_reg(struct ddr_reg_value *reg)
{
	int i;
	serial_debug("name			  = %s\n", reg->h.name);
	serial_debug("id			  = %x\n", reg->h.id);
	serial_debug("type			  = %x\n", reg->h.type);
	serial_debug("freq			  = %x\n", reg->h.freq);
	serial_debug("DDRC_CFG_VALUE		= %x\n", reg->DDRC_CFG_VALUE);
	serial_debug("DDRC_CTRL_VALUE	   = %x\n", reg->DDRC_CTRL_VALUE);
	serial_debug("DDRC_DLMR_VALUE	   = %x\n", reg->DDRC_DLMR_VALUE);
	serial_debug("DDRC_DDLP_VALUE	   = %x\n", reg->DDRC_DDLP_VALUE);
	serial_debug("DDRC_MMAP0_VALUE	  = %x\n", reg->DDRC_MMAP0_VALUE);
	serial_debug("DDRC_MMAP1_VALUE	  = %x\n", reg->DDRC_MMAP1_VALUE);
	serial_debug("DDRC_REFCNT_VALUE	 = %x\n", reg->DDRC_REFCNT_VALUE);
	serial_debug("DDRC_TIMING1_VALUE	= %x\n", reg->DDRC_TIMING1_VALUE);
	serial_debug("DDRC_TIMING2_VALUE	= %x\n", reg->DDRC_TIMING2_VALUE);
	serial_debug("DDRC_TIMING3_VALUE	= %x\n", reg->DDRC_TIMING3_VALUE);
	serial_debug("DDRC_TIMING4_VALUE	= %x\n", reg->DDRC_TIMING4_VALUE);
	serial_debug("DDRC_TIMING5_VALUE	= %x\n", reg->DDRC_TIMING5_VALUE);
	serial_debug("DDRC_AUTOSR_CNT_VALUE = %x\n", reg->DDRC_AUTOSR_CNT_VALUE);
	serial_debug("DDRC_AUTOSR_EN_VALUE  = %x\n", reg->DDRC_AUTOSR_EN_VALUE);
	serial_debug("DDRC_HREGPRO_VALUE	= %x\n", reg->DDRC_HREGPRO_VALUE);
	serial_debug("DDRC_PREGPRO_VALUE	= %x\n", reg->DDRC_PREGPRO_VALUE);
	serial_debug("DDRC_CGUC0_VALUE	  = %x\n", reg->DDRC_CGUC0_VALUE);
	serial_debug("DDRC_CGUC1_VALUE	  = %x\n", reg->DDRC_CGUC1_VALUE);
	serial_debug("DDRP_MEMCFG_VALUE	 = %x\n", reg->DDRP_MEMCFG_VALUE);
	serial_debug("DDRP_CL_VALUE		 = %x\n", reg->DDRP_CL_VALUE);
	serial_debug("DDRP_CWL_VALUE		= %x\n", reg->DDRP_CWL_VALUE);
	serial_debug("DDR_MR0_VALUE		 = %x\n", reg->DDR_MR0_VALUE);
	serial_debug("DDR_MR1_VALUE		 = %x\n", reg->DDR_MR1_VALUE);
	serial_debug("DDR_MR2_VALUE		 = %x\n", reg->DDR_MR2_VALUE);
	serial_debug("DDR_MR3_VALUE		 = %x\n", reg->DDR_MR3_VALUE);
	serial_debug("DDR_MR10_VALUE		= %x\n", reg->DDR_MR10_VALUE);
	serial_debug("DDR_MR11_VALUE		= %x\n", reg->DDR_MR11_VALUE);
	serial_debug("DDR_MR63_VALUE		= %x\n", reg->DDR_MR63_VALUE);
	serial_debug("DDR_CHIP_0_SIZE	   = %x\n", reg->DDR_CHIP_0_SIZE);
	serial_debug("DDR_CHIP_1_SIZE	   = %x\n", reg->DDR_CHIP_1_SIZE);
	for(i = 0; i < 5; i++) {
		serial_debug("REMMAP_ARRAY[%d] = %x\n", i, reg->REMMAP_ARRAY[i]);
	}

}

#ifndef CONFIG_BURNER
void get_ddr_params_normal(void)
{
	int found = 0;
	int size = 0;
	int i;
	unsigned int burned_ddr_id = *(volatile unsigned int *)(CONFIG_SPL_TEXT_BASE + 128);

	if((burned_ddr_id & 0xffff) != (burned_ddr_id >> 16)) {
		serial_debug("invalid burned ddr id\n");
	}

	burned_ddr_id &= 0xffff;

	size = ARRAY_SIZE(supported_ddr_reg_values);

	if(size == 1) {
		found = 1;
		global_reg_value = &supported_ddr_reg_values[0];
	} else {
		for(i = 0; i < ARRAY_SIZE(supported_ddr_reg_values); i++) {
			global_reg_value = &supported_ddr_reg_values[i];
			if(burned_ddr_id == global_reg_value->h.id) {
				found = 1;
				break;
			}
		}
	}
}
#else
void get_ddr_params_burner(void)
{
	/* keep ddr_reg_value inc ddr_innophy.h
	 * with ddr_registers the same
	 * */
	global_reg_value = g_ddr_param;
}
#endif

void get_ddr_params(void)
{
#ifndef CONFIG_BURNER
	get_ddr_params_normal();
#else
	get_ddr_params_burner();
#endif
	//dump_generated_reg(global_reg_value);

}

unsigned int get_ddr_size(void)
{
	return (global_reg_value->DDR_CHIP_0_SIZE) >> 20;
}

void sdram_init(void)
{
	enum ddr_type type;
	unsigned int rate;
	unsigned int reg_val;
	unsigned int ddr_drv_config[INDEX_EN] = {0};

	debug("sdram init start\n");
	serial_debug("DDR clk rate %d\n", CONFIG_SYS_MEM_FREQ);
	soc_ddr_init();

	get_ddr_params();
	type = get_ddr_type();
	clk_set_rate(DDR, global_reg_value->h.freq);

	if(ddr_hook && ddr_hook->prev_ddr_init)
		ddr_hook->prev_ddr_init(type);
	rate = clk_get_rate(DDR);
	debug("DDR clk rate %d\n", rate);

	ddr_writel(0x1 << 20 | 1 << 23, DDRC_CTRL);  /* ddrc_reset_phy , keep dfi_rst_n low*/
	mdelay(1);

	ddrp_cfg(global_reg_value);

	reg_val = ddr_readl(DDRC_CTRL);
	reg_val &= ~ (1 << 20);
	ddr_writel(reg_val, DDRC_CTRL); /*ddrc_reset_phy clear*/
	mdelay(1);

	ddrp_pll_init();

	/* DDR Controller init*/
	ddrc_prev_init();
	get_ddr_par(ddr_drv_config, sizeof(ddr_drv_config));
	ddrc_dfi_init(type, ddr_drv_config[KGD_RTT_DIC]);

	/*auto training 需要在training之后，初始化控制器功能，防止控制器自动刷新对training结果造成影响.*/
	ddrp_auto_calibration(ddr_drv_config);
	ddrc_post_init(); //配置自刷新、power down、port ob buffer func

	ddrp_post_init();

	/*在完全初始化完ddr phy之后，再开启自动时钟控制.*/
	ddr_writel(global_reg_value->DDRC_CGUC0_VALUE, DDRC_CGUC0);
	ddr_writel(global_reg_value->DDRC_CGUC1_VALUE, DDRC_CGUC1);

	/*一些数据访问相关的配置，自动控制的配置，应该在training之后，防止training过程中出现干扰.*/
	if(global_reg_value->DDRC_AUTOSR_EN_VALUE) {
		/* ddr_writel(DDRC_AUTOSR_CNT_VALUE, DDRC_AUTOSR_CNT); */
		ddr_writel(1, DDRC_AUTOSR_EN);
	} else {
		ddr_writel(0, DDRC_AUTOSR_EN);
	}

	if(ddr_hook && ddr_hook->post_ddr_init)
		ddr_hook->post_ddr_init(type);


	dump_ddrc_register();

	debug("DDR size is : %d MByte\n", (global_reg_value->DDR_CHIP_0_SIZE + global_reg_value->DDR_CHIP_1_SIZE) / 1024 /1024);
	/* DDRC address remap configure*/
	debug("sdram init finished\n");
}

phys_size_t initdram(int board_type)
{
	/* SDRAM size was calculated when compiling. */
#ifndef EMC_LOW_SDRAM_SPACE_SIZE
#define EMC_LOW_SDRAM_SPACE_SIZE 0x10000000 /* 256M */
#endif /* EMC_LOW_SDRAM_SPACE_SIZE */

	unsigned int ram_size;

	/*init ddr params in uboot env. */
	get_ddr_params();
	ram_size = (unsigned int)(global_reg_value->DDR_CHIP_0_SIZE) + (unsigned int)(global_reg_value->DDR_CHIP_1_SIZE);
	debug("ram_size=%x\n", ram_size);


	return ram_size;
}

