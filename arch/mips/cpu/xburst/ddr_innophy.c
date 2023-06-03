
/*
 * DDR driver for inno DDR PHY.
 * Used by x1xxx
 *
 * Copyright (C) 2017 Ingenic Semiconductor Co.,Ltd
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

/* #define DEBUG */
#include <config.h>
#include <common.h>
#include <ddr/ddr_common.h>
#include <asm/arch/clk.h>
#include "ddr_innophy.h"
#include "ddr_debug.h"
#ifndef CONFIG_BURNER
#include <generated/ddr_reg_values.h>
#else
#include "ddr_reg_data.h"
#endif

//#define CONFIG_DWC_DEBUG

#define ddr_hang() do{                                  \
	printf("%s %d\n",__FUNCTION__,__LINE__);        \
	hang();                                         \
}while(0)


/*#define CONFIG_DDRP_SOFTWARE_TRAINING*/

DECLARE_GLOBAL_DATA_PTR;
extern unsigned int sdram_size(int cs, struct ddr_params *p);


int current_ddr_type;


struct ddr_params *ddr_params_p = NULL;
extern void reset_dll(void);
#define BYPASS_ENABLE       1
#define BYPASS_DISABLE      0
#define IS_BYPASS_MODE(x)     (((x) & 1) == BYPASS_ENABLE)
#define DDR_TYPE_MODE(x)     (((x) >> 1) & 0xf)

static void dump_ddr_params(void)
{
#ifdef CONFIG_DWC_DEBUG
	printf("DDRC_CFG_VALUE		    0x%x\n",DDRC_CFG_VALUE		    );
	printf("DDRC_CTRL_VALUE	       	0x%x\n",DDRC_CTRL_VALUE	    	);
	printf("DDRC_DLMR_VALUE	       	0x%x\n",DDRC_DLMR_VALUE	    	);
	printf("DDRC_DDLP_VALUE	       	0x%x\n",DDRC_DDLP_VALUE	    	);
	printf("DDRC_MMAP0_VALUE       	0x%x\n",DDRC_MMAP0_VALUE    	);
	printf("DDRC_MMAP1_VALUE       	0x%x\n",DDRC_MMAP1_VALUE    	);
	printf("DDRC_REFCNT_VALUE      	0x%x\n",DDRC_REFCNT_VALUE   	);
	printf("DDRC_TIMING1_VALUE     	0x%x\n",DDRC_TIMING1_VALUE  	);
	printf("DDRC_TIMING2_VALUE     	0x%x\n",DDRC_TIMING2_VALUE  	);
	printf("DDRC_TIMING3_VALUE     	0x%x\n",DDRC_TIMING3_VALUE  	);
	printf("DDRC_TIMING4_VALUE     	0x%x\n",DDRC_TIMING4_VALUE  	);
	printf("DDRC_TIMING5_VALUE     	0x%x\n",DDRC_TIMING5_VALUE  	);
	printf("DDRC_TIMING6_VALUE     	0x%x\n",DDRC_TIMING6_VALUE  	);
	printf("DDRC_AUTOSR_EN_VALUE   	0x%x\n",DDRC_AUTOSR_EN_VALUE	);
	printf("DDRP_MEMCFG_VALUE      	0x%x\n",DDRP_MEMCFG_VALUE   	);
	printf("DDRP_CL_VALUE          	0x%x\n",DDRP_CL_VALUE       	);
	printf("DDRP_CWL_VALUE	       	0x%x\n",DDRP_CWL_VALUE	    	);
	printf("DDR_MR0_VALUE	       	0x%x\n",DDR_MR0_VALUE	    	);
	printf("DDR_MR1_VALUE	       	0x%x\n",DDR_MR1_VALUE	    	);
	printf("DDR_MR2_VALUE	       	0x%x\n",DDR_MR2_VALUE	    	);
	printf("DDR_MR3_VALUE	       	0x%x\n",DDR_MR3_VALUE	    	);
	printf("DDR_MR10_VALUE	       	0x%x\n",DDR_MR10_VALUE	    	);
//	printf("DDR_MR11_VALUE	       	0x%x\n",DDR_MR11_VALUE	    	);
	printf("DDR_MR63_VALUE	       	0x%x\n",DDR_MR63_VALUE	    	);
	printf("DDR_CHIP_0_SIZE	       	0x%x\n",DDR_CHIP_0_SIZE	    	);
	printf("DDR_CHIP_1_SIZE	       	0x%x\n",DDR_CHIP_1_SIZE	    	);
	printf("REMMAP_ARRAY0          	0x%x\n",REMMAP_ARRAY[0]     	);
	printf("REMMAP_ARRAY1          	0x%x\n",REMMAP_ARRAY[1]     	);
	printf("REMMAP_ARRAY2          	0x%x\n",REMMAP_ARRAY[2]     	);
	printf("REMMAP_ARRAY3          	0x%x\n",REMMAP_ARRAY[3]     	);
	printf("REMMAP_ARRAY4          	0x%x\n",REMMAP_ARRAY[4]     	);
#endif
}


static void dump_ddrc_register(void)
{
#ifdef CONFIG_DWC_DEBUG
	printf("DDRC_STATUS         0x%x\n", ddr_readl(DDRC_STATUS));
	printf("DDRC_CFG            0x%x\n", ddr_readl(DDRC_CFG));
	printf("DDRC_CTRL           0x%x\n", ddr_readl(DDRC_CTRL));
	printf("DDRC_LMR            0x%x\n", ddr_readl(DDRC_LMR));
	printf("DDRC_DLP            0x%x\n", ddr_readl(DDRC_DLP));
	printf("DDRC_TIMING1        0x%x\n", ddr_readl(DDRC_TIMING(1)));
	printf("DDRC_TIMING2        0x%x\n", ddr_readl(DDRC_TIMING(2)));
	printf("DDRC_TIMING3        0x%x\n", ddr_readl(DDRC_TIMING(3)));
	printf("DDRC_TIMING4        0x%x\n", ddr_readl(DDRC_TIMING(4)));
	printf("DDRC_TIMING5        0x%x\n", ddr_readl(DDRC_TIMING(5)));
	printf("DDRC_TIMING6        0x%x\n", ddr_readl(DDRC_TIMING(6)));
	printf("DDRC_REFCNT         0x%x\n", ddr_readl(DDRC_REFCNT));
	printf("DDRC_MMAP0          0x%x\n", ddr_readl(DDRC_MMAP0));
	printf("DDRC_MMAP1          0x%x\n", ddr_readl(DDRC_MMAP1));
	printf("DDRC_REMAP1         0x%x\n", ddr_readl(DDRC_REMAP(1)));
	printf("DDRC_REMAP2         0x%x\n", ddr_readl(DDRC_REMAP(2)));
	printf("DDRC_REMAP3         0x%x\n", ddr_readl(DDRC_REMAP(3)));
	printf("DDRC_REMAP4         0x%x\n", ddr_readl(DDRC_REMAP(4)));
	printf("DDRC_REMAP5         0x%x\n", ddr_readl(DDRC_REMAP(5)));
	printf("DDRC_AUTOSR_EN      0x%x\n", ddr_readl(DDRC_AUTOSR_EN));
	printf("INNO_DQ_WIDTH   :%X\n",phy_readl(INNO_DQ_WIDTH));
	printf("INNO_PLL_FBDIV  :%X\n",phy_readl(INNO_PLL_FBDIV));
	printf("INNO_PLL_PDIV   :%X\n",phy_readl(INNO_PLL_PDIV));
	printf("INNO_MEM_CFG    :%X\n",phy_readl(INNO_MEM_CFG));
	printf("INNO_PLL_CTRL   :%X\n",phy_readl(INNO_PLL_CTRL));
	printf("INNO_CHANNEL_EN :%X\n",phy_readl(INNO_CHANNEL_EN));
	printf("INNO_CWL        :%X\n",phy_readl(INNO_CWL));
	printf("INNO_CL         :%X\n",phy_readl(INNO_CL));
#endif
}

static void reset_controller(void)
{
	ddr_writel(0xf << 20, DDRC_CTRL);
	mdelay(5);
	ddr_writel(0x8 << 20, DDRC_CTRL);
	mdelay(5);
}

static void ddrc_post_init(void)
{
	ddr_writel(DDRC_REFCNT_VALUE, DDRC_REFCNT);
	debug("DDRC_STATUS: %x\n",ddr_readl(DDRC_STATUS));
	ddr_writel(DDRC_CTRL_VALUE, DDRC_CTRL);
}

static void ddrc_prev_init(void)
{
	dwc_debug("DDR Controller init\n");
//	ddr_writel(DDRC_CTRL_CKE | DDRC_CTRL_ALH, DDRC_CTRL);
//	ddr_writel(0, DDRC_CTRL);
	/* DDRC CFG init*/
//	ddr_writel(DDRC_CFG_VALUE, DDRC_CFG);
	/* DDRC timing init*/
	ddr_writel(DDRC_TIMING1_VALUE, DDRC_TIMING(1));
	ddr_writel(DDRC_TIMING2_VALUE, DDRC_TIMING(2));
	ddr_writel(DDRC_TIMING3_VALUE, DDRC_TIMING(3));
	ddr_writel(DDRC_TIMING4_VALUE, DDRC_TIMING(4));
	ddr_writel(DDRC_TIMING5_VALUE, DDRC_TIMING(5));
	ddr_writel(DDRC_TIMING6_VALUE, DDRC_TIMING(6));

	/* DDRC memory map configure*/
	ddr_writel(DDRC_MMAP0_VALUE, DDRC_MMAP0);
	ddr_writel(DDRC_MMAP1_VALUE, DDRC_MMAP1);
//	ddr_writel(DDRC_CTRL_CKE | DDRC_CTRL_ALH, DDRC_CTRL);
//	ddr_writel(DDRC_REFCNT_VALUE, DDRC_REFCNT);
	ddr_writel(DDRC_CTRL_VALUE & 0xffff8fff, DDRC_CTRL);
}



#ifdef CONFIG_X1600
void ddr_inno_phy_init(void)
{
	u32 val = 0;
	/*
	 * ddr phy pll initialization
	 */
	ddr_writel(DDRP_DQ_WIDTH_DQ_H | DDRP_DQ_WIDTH_DQ_L, DDRP_INNOPHY_DQ_WIDTH);

	ddr_writel(DDRP_MEMCFG_VALUE, DDRP_INNOPHY_MEM_CFG);

	val = ddr_readl(DDRP_INNOPHY_CL);
	val &= ~(0xf);
	val |= DDRP_CL_VALUE;
	ddr_writel(val, DDRP_INNOPHY_CL);

	val = ddr_readl(DDRP_INNOPHY_CWL);
	val &= ~(0xf);
	val |= DDRP_CWL_VALUE;
	ddr_writel(val, DDRP_INNOPHY_CWL);

	val = ddr_readl(DDRP_INNOPHY_AL);
	val &= ~(0xf);
	ddr_writel(val, DDRP_INNOPHY_AL);

	/* ????   reserve   read only   ???? */
	val = ddr_readl(DDRC_CTRL);
	val &= ~ (1 << 20);
	ddr_writel(val, DDRC_CTRL);


	val = ddr_readl(DDRP_INNOPHY_PLL_FBDIV);
	val &= ~(0xff);
//	val |= 0x14;
	val |= 0x10;
	ddr_writel(val, DDRP_INNOPHY_PLL_FBDIV);

	ddr_writel(ddr_readl(DDRP_INNOPHY_PLL_CTRL) | DDRP_PLL_CTRL_PLLPDEN, DDRP_INNOPHY_PLL_CTRL);

	val = ddr_readl(DDRP_INNOPHY_PLL_PDIV);
	val &= ~(0xff);
//	val |= 0x5;
	val |= 0x24;
	ddr_writel(val, DDRP_INNOPHY_PLL_PDIV);

	udelay(5);
	/* pll reset */
	ddr_writel(ddr_readl(DDRP_INNOPHY_PLL_CTRL) & ~DDRP_PLL_CTRL_PLLPDEN, DDRP_INNOPHY_PLL_CTRL);
	udelay(5);

	while(!(ddr_readl(DDRP_INNOPHY_PLL_LOCK) & (1 << 3)));
	udelay(5);
	while(!((*(volatile unsigned int *)0xb301208c) & (1 << 2)));
	udelay(5);



}
#else
void ddr_inno_phy_init(void)
{
	u32 reg = 0;
	/*
	 * ddr phy pll initialization
	 */
#ifdef CONFIG_X1021
	phy_writel(0x8, INNO_PLL_FBDIV);
	phy_writel(0x2, INNO_PLL_PDIV);
#else
	phy_writel(0x14, INNO_PLL_FBDIV);
	phy_writel(0x5, INNO_PLL_PDIV);
#endif
	phy_writel(0x1a, INNO_PLL_CTRL);
	phy_writel(0x18, INNO_PLL_CTRL);
#ifndef CONFIG_FASTBOOT
	printf("ddrp pll lock 0x%x\n", phy_readl(INNO_PLL_LOCK));
#endif
	while(!(readl(DDR_APB_PHY_INIT) & (1<<2))); //polling pll lock

	/*
	 * ddr phy register cfg
	 */
	phy_writel(0x3, INNO_DQ_WIDTH);

	if(current_ddr_type == DDR3) {
		phy_writel(DDRP_MEMCFG_VALUE, INNO_MEM_CFG);
		phy_writel(DDRP_CWL_VALUE, INNO_CWL);
		phy_writel(DDRP_CL_VALUE, INNO_CL);

	} else if(current_ddr_type == DDR2) {
		phy_writel(0x11,INNO_MEM_CFG);  // MEMSEL  =  DDR2  ,    BURSEL = burst8
		phy_writel(0x0d,INNO_CHANNEL_EN);
		phy_writel(((DDR_MR0_VALUE&0xf0)>>4)-1, INNO_CWL);
		reg = ((DDR_MR0_VALUE&0xf0)>>4);
		phy_writel(reg, INNO_CL);
	}
	phy_writel(0x0, INNO_AL);

#ifndef CONFIG_FASTBOOT
	printf("CWL = 0x%x\n", phy_readl(INNO_CWL));
	printf("CL = 0x%x\n", phy_readl(INNO_CL));
	printf("AL = 0x%x\n", phy_readl(INNO_AL));
#endif
}
#endif

void ddrc_dfi_init(void)
{
	u32 reg = 0;

	writel(1, DDR_APB_PHY_INIT); //start high
	writel(0, DDR_APB_PHY_INIT); //start low
	while(!(readl(DDR_APB_PHY_INIT) & (1<<1))); //polling dfi init comp
#ifndef CONFIG_FASTBOOT
	printf("ddr_inno_phy_init ..! 11:  %X\n", readl(DDR_APB_PHY_INIT));
#endif

#ifdef CONFIG_X1600
	reg = ddr_readl(DDRC_CTRL);
	reg |= (1 << 23);
	ddr_writel(reg, DDRC_CTRL); //set dfi_reset_n high

	ddr_writel(DDRC_CFG_VALUE, DDRC_CFG);

	ddr_writel(DDRC_CTRL_CKE, DDRC_CTRL); // set CKE to high

	ddr_writel(7 << 8 | 1 << 0, DDRC_LMR); //Send All bank precharge.
	mdelay(1);
#else
	ddr_writel(0, DDRC_CTRL);
	ddr_writel(DDRC_CFG_VALUE, DDRC_CFG);
	ddr_writel(0x2, DDRC_CTRL);
#endif


	if(current_ddr_type == DDR3) {
#define DDRC_LMR_MR(n)						\
	DDRC_DLMR_VALUE | 0x1 | (2 << 3) |			\
		((DDR_MR##n##_VALUE & 0xffff) << 12) |		\
		(((DDR_MR##n##_VALUE >> 16) & 0x7) << 8)

	printf("MR0 : 0x%x\n", DDRC_LMR_MR(0));
	printf("MR1 : 0x%x\n", DDRC_LMR_MR(1));
	printf("MR2 : 0x%x\n", DDRC_LMR_MR(2));
	printf("MR3 : 0x%x\n", DDRC_LMR_MR(3));
	printf("ZQCL : 0x%x\n", DDRC_DLMR_VALUE | (0x4 << 3) | 0x1);

	ddr_writel(DDRC_LMR_MR(0)/*0x1a30011*/, DDRC_LMR); //MR0
	ddr_writel(DDRC_LMR_MR(1)/*0x6111*/, DDRC_LMR); //MR1
	ddr_writel(DDRC_LMR_MR(2)/*0x8211*/, DDRC_LMR); //MR2
	ddr_writel(DDRC_LMR_MR(3)/*0x311*/, DDRC_LMR); //MR3
	ddr_writel(DDRC_DLMR_VALUE | (0x4 << 3) | 0x1/*0x19*/, DDRC_LMR);

#undef DDRC_LMR_MR
	} else if(current_ddr_type == LPDDR2) {
#define DDRC_LMR_MR(n)										\
		DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |	\
			((DDR_MR##n##_VALUE & 0xff) << 24) |						\
			(((DDR_MR##n##_VALUE >> 8) & 0xff) << (16))
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
		printf("mr1 = 0x%x\n", DDRC_LMR_MR(1));
		printf("mr2 = 0x%x\n", DDRC_LMR_MR(2));
		printf("mr3 = 0x%x\n", DDRC_LMR_MR(3));
		printf("mr10 = 0x%x\n", DDRC_LMR_MR(10));
		printf("mr63 = 0x%x\n", DDRC_LMR_MR(63));
#undef DDRC_LMR_MR
	} else {
		/*DDR2*/
#ifdef CONFIG_X1600
#define DDRC_LMR_MR(n)										\
		DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |	\
			(((DDR_MR##n##_VALUE  >> 13) & 0x3) << 8) |						\
			(((DDR_MR##n##_VALUE ) & 0x1fff) << (12))

		ddr_writel(DDRC_LMR_MR(0), DDRC_LMR);
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR);
		mdelay(1);
		printf("mr0 = 0x%x\n", DDRC_LMR_MR(0));
		printf("mr1 = 0x%x\n", DDRC_LMR_MR(1));
#undef DDRC_LMR_MR
#else
		ddr_writel(0x211,DDRC_LMR);
#ifndef CONFIG_FASTBOOT
		printf("DDRC_LMR: %x\n",ddr_readl(DDRC_LMR));
#endif
		ddr_writel(0,DDRC_LMR);

		ddr_writel(0x311,DDRC_LMR);
#ifndef CONFIG_FASTBOOT
		printf("DDRC_LMR: %x\n", ddr_readl(DDRC_LMR));
#endif
		ddr_writel(0,DDRC_LMR);

		ddr_writel(0x111,DDRC_LMR);
#ifndef CONFIG_FASTBOOT
		printf("DDRC_LMR: %x\n", ddr_readl(DDRC_LMR));
#endif
		ddr_writel(0,DDRC_LMR);

		reg = ((DDR_MR0_VALUE)<<12)|0x011;
		ddr_writel(reg, DDRC_LMR);
#ifndef CONFIG_FASTBOOT
		printf("DDRC_LMR, MR0: %x\n", reg);
#endif
		ddr_writel(0,DDRC_LMR);
#endif

	}
}

void ddrp_wl_training(void)
{

	if(current_ddr_type == DDR3) {
		//write level
		printf("WL_MODE1 : 0x%x\n", DDR_MR1_VALUE & 0xff);
		phy_writel(DDR_MR1_VALUE & 0xff, INNO_WL_MODE1);
		phy_writel(0x40, INNO_WL_MODE2);
		phy_writel(0xa4, INNO_TRAINING_CTRL);
		while (0x3 != phy_readl(INNO_WL_DONE));
		phy_writel(0xa1, INNO_TRAINING_CTRL);
	}

	/* ???? */
	if(current_ddr_type == DDR3)
		phy_writel(0x50, INNO_MEM_CFG);
	else
		phy_writel(0x51, INNO_MEM_CFG);

	writel(0x24,0xb3011028);
}

/*
 * Name     : phy_calibration()
 * Function : control the RX DQS window delay to the DQS
 * */
void phy_calibration(void)
{
	int m = phy_readl(INNO_TRAINING_CTRL);
#ifndef CONFIG_FASTBOOT
	printf("INNO_TRAINING_CTRL 1: %x\n", phy_readl(INNO_TRAINING_CTRL));
#endif
	m = 0xa1;
	phy_writel(m,INNO_TRAINING_CTRL);
#ifndef CONFIG_FASTBOOT
	printf("INNO_TRAINING_CTRL 2: %x\n", phy_readl(INNO_TRAINING_CTRL));
#endif
	while (0x3 != phy_readl(INNO_CALIB_DONE));
#ifndef CONFIG_FASTBOOT
	printf("calib done: %x\n", phy_readl(INNO_CALIB_DONE));
#endif
	phy_writel(0xa0,INNO_TRAINING_CTRL);
#ifndef CONFIG_FASTBOOT
	printf("INNO_TRAINING_CTRL 3: %x\n", phy_readl(INNO_TRAINING_CTRL));
#endif
}
#if defined(CONFIG_BURNER) || defined(CONFIG_X1600)
static enum ddr_type get_ddr_type(void)
{
	int type;
	ddrc_cfg_t ddrc_cfg;
	ddrc_cfg.d32 = DDRC_CFG_VALUE;
	switch(ddrc_cfg.b.TYPE){
	case 3:
		type = LPDDR;
		debug("DDR type is : LPDDR\n");
		break;
	case 4:
		type = DDR2;
		debug("DDR type is : DDR2\n");
		break;
	case 5:
		type = LPDDR2;
		debug("DDR type is : LPDDR2\n");
		break;
	case 6:
		type = DDR3;
		debug("DDR type is : DDR3\n");
		break;
	case 7:
		type = LPDDR3;
		debug("DDR type is : LPDDR3\n");
		break;
	default:
		type = UNKOWN;
		debug("unsupport ddr type!\n");
		ddr_hang();
	}
	return type;
}
#else
int get_ddr_type(void)
{
	int type;
#ifndef CONFIG_MULT_DDR_PARAMS_CREATOR
#ifdef CONFIG_DDR_TYPE_DDR3
	type = DDR3;
#else /*CONFIG_DDR_TYPE_DDR2*/
	type = DDR2;
#endif

#else

#define EFUSE_BASE	0xb3540000
#define EFUSE_CTRL		0x0
#define EFUSE_STATE		0x8
#define EFUSE_DATA		0xC
#define DDR_INFO_ADDR	0xE

	unsigned int val, data;

	writel(0, EFUSE_BASE + EFUSE_STATE);

	val = DDR_INFO_ADDR << 21 | 1;
	writel(val, EFUSE_BASE + EFUSE_CTRL);

	while(!(writel(val, EFUSE_BASE + EFUSE_STATE) & 1));
	data = readl(EFUSE_BASE + EFUSE_DATA);
	val = data & 0xFFFF;

	if(val == 0x1111)
		type = DDR2;
	else if(val == 0x2222)
		type = DDR3;
	else
		type = DDR3;

	get_ddr_params(type);
#endif

	return type;

}
#endif

#ifdef CONFIG_X1600
struct ddrp_calib {
	union{
		uint8_t u8;
		struct{
			unsigned dllsel:3;
			unsigned ophsel:2;
			unsigned cyclesel:3;
		}b;
	}bypass;
	union{
		uint8_t u8;
		struct{
			unsigned reserved:5;
			unsigned rx_dll:3;
		}b;
	}rx_dll;
};

struct ddrp_calib calib_val[8 * 4 * 8 * 5];

static void ddrp_software_calibration(void)
{

	int x, y, z;
	int c, o, d =0, r = 0;
	unsigned int addr = 0xa0100000, val;
	unsigned int i, n, m = 0;
	unsigned int reg;

	reg = ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	reg |= (DDRP_TRAINING_CTRL_DSCSE_BP);
	reg &= ~(DDRP_TRAINING_CTRL_DSACE_START);
	ddr_writel(reg, DDRP_INNOPHY_TRAINING_CTRL);

	for(c = 0; c < 8; c ++) {
		for(o = 0; o < 2; o++) {
			for (d = 0; d < 8; d++) {
				x = c << 4 | o << 3 | d;
				y = c << 4 | o << 3 | d;
				ddr_writel(x, DDRP_INNOPHY_CALIB_BYPASS_AL);
				ddr_writel(y, DDRP_INNOPHY_CALIB_BYPASS_AH);

				for(r = 0; r < 4; r++) {
					unsigned int val1,val2;

					val1 = ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AL) & (~0x3);
					ddr_writel((r << 0) | val1, DDRP_INNOPHY_RXDLL_DELAY_AL);
					val1 = ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AH) & (~0x3);
					ddr_writel((r << 0)|val1, DDRP_INNOPHY_RXDLL_DELAY_AH);


					for(i = 0; i < 0xff; i++) {
						val = 0;
						for(n = 0; n < 4; n++ ) {
							val |= i<<(n * 8);
						}
						*(volatile unsigned int *)(addr + i * 4) = val;
						volatile unsigned int val1;
						val1 = *(volatile unsigned int *)(addr + i * 4);
						if(val1 != val) {
//						printf("val1 : 0x%x   val : 0x%x\n", val1, val);
							break;
						}
					}
					if(i == 0xff) {
						calib_val[m].bypass.b.cyclesel = c;
						calib_val[m].bypass.b.ophsel = o;
						calib_val[m].bypass.b.dllsel = d;
						calib_val[m].rx_dll.b.rx_dll = r;
						m++;
					}
				}
			}
		}
	}

	if(!m) {
		printf("calib bypass fail\n");
		return ;
	}


	m = m  / 2;
	c = calib_val[m].bypass.b.cyclesel;
	o = calib_val[m].bypass.b.ophsel;
	d = calib_val[m].bypass.b.dllsel;
	r = calib_val[m].rx_dll.b.rx_dll;

	printf("m = %d   c = %d   o = %d   d = %d  r = %d\n", m, c, o, d, r);

	x = c << 4 | o << 3 | d;
	y = c << 4 | o << 3 | d;
	z = r << 4 | r << 0;
	ddr_writel(x, DDRP_INNOPHY_CALIB_BYPASS_AL);
	ddr_writel(y, DDRP_INNOPHY_CALIB_BYPASS_AH);

	ddr_writel(ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AL) | r << 0, DDRP_INNOPHY_RXDLL_DELAY_AL);
	ddr_writel(ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AH) | r << 0, DDRP_INNOPHY_RXDLL_DELAY_AH);

	{
		struct ddrp_calib b_al, b_ah, r_al,r_ah;
		b_al.bypass.u8 = ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AL);
		printf("bypass :CALIB_AL: dllsel %x, ophsel %x, cyclesel %x\n",b_al.bypass.b.dllsel, b_al.bypass.b.ophsel, b_al.bypass.b.cyclesel);
		b_ah.bypass.u8 = ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AH);
		printf("bypass:CAHIB_AH: dllsel %x, ophsel %x, cyclesel %x\n", b_ah.bypass.b.dllsel, b_ah.bypass.b.ophsel, b_ah.bypass.b.cyclesel);
		printf("rxdll delay al : %x\n", ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AL));
		printf("rxdll delay ah : %x\n", ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AH));

	}
}

void ddrp_hardware_calibration(void)
{
	unsigned int reg;
	unsigned int timeout = 0xffffff;
	unsigned int al, ah;

	reg = ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	reg &= ~(DDRP_TRAINING_CTRL_DSCSE_BP);
	ddr_writel(reg, DDRP_INNOPHY_TRAINING_CTRL);

	reg = ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	reg |= (DDRP_TRAINING_CTRL_DSACE_START);
	ddr_writel(reg, DDRP_INNOPHY_TRAINING_CTRL);

	while(!((ddr_readl(DDRP_INNOPHY_CALIB_DONE) & 0x13) == 3) && --timeout) {
		udelay(1);
		printf("DDRP_INNOPHY_CALIB_DELAY_AL : %x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL));
		printf("DDRP_INNOPHY_CALIB_DELAY_AH : %x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH));
		printf("DDRP_INNOPHY_CALIB_DONE : %x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE));
	}

	if(!timeout) {
		debug("ddrp_auto_calibration failed!\n");
	}

	mdelay(5);
	reg = ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	reg &= ~(1);
	ddr_writel(reg, DDRP_INNOPHY_TRAINING_CTRL);

	debug("ddrp_auto_calibration success!\n");
	al = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL);
	ah = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH);
	debug("left  c : %d, o : %d,  d : %d\n", (al >> 4) & 7, (al >> 3) & 1, al & 7);
	debug("right c : %d, o : %d,  d : %d\n", (ah >> 4) & 7, (ah >> 3) & 1, ah & 7);

	{
		unsigned int cycsel, tmp;
		unsigned int read_data0, read_data1;
		unsigned int c0, c1;
		unsigned int max;

		read_data0 = *(volatile unsigned int *)(0xb3011000 + (0x74 << 2));
		read_data1 = *(volatile unsigned int *)(0xb3011000 + (0x75 << 2));
		c0 = (read_data0 >> 4) & 0x7;
		c1 = (read_data1 >> 4) & 0x7;

		max = max(c0, c1);

		cycsel = max + 1;

		tmp = *(volatile unsigned int *)(0xb3011000 + (0xa << 2));
		tmp &= ~(7 << 1);
		tmp |= cycsel << 1;
		*(volatile unsigned int *)(0xb3011000 + (0xa << 2)) = tmp;


		tmp = *(volatile unsigned int *)(0xb3011000 + 0x4);
		tmp |= 1 << 6;
		*(volatile unsigned int *)(0xb3011000 + (0x1 << 2)) = tmp;
	}

	printf("ddr calib finish\n");

}
#endif




/* DDR sdram init */
void sdram_init(void)
{

	unsigned int mode;
	unsigned int bypass = 0;
	unsigned int rate;

	dwc_debug("sdram init start\n");

	current_ddr_type = get_ddr_type();
//	dump_ddr_params();

	clk_set_rate(DDR, CONFIG_SYS_MEM_FREQ);
	reset_dll();
	rate = clk_get_rate(DDR);
	if(rate != CONFIG_SYS_MEM_FREQ)
		dwc_debug("sdram set ddr freq failed\n");

	reset_controller();

#ifdef CONFIG_DDR_AUTO_SELF_REFRESH
	ddr_writel(0x0 ,DDRC_AUTOSR_EN);
#endif

	ddr_inno_phy_init();
	ddrc_dfi_init();

#ifndef CONFIG_X1600
	ddrp_wl_training();
#endif
        /* DDR Controller init*/
	ddrc_prev_init();
#ifndef CONFIG_X1600
	phy_calibration();
#endif
	dwc_debug("DDR PHY init OK\n");
	ddrc_post_init();

#ifndef CONFIG_X1600
	ddr_writel(ddr_readl(DDRC_STATUS) & ~DDRC_DSTATUS_MISS, DDRC_STATUS);
#endif

#ifdef CONFIG_X1600
#ifdef CONFIG_DDRP_SOFTWARE_TRAINING
	ddrp_software_calibration();
#else
	ddrp_hardware_calibration();
#endif
#endif

#ifdef CONFIG_DDR_AUTO_SELF_REFRESH
#ifndef CONFIG_X1600
	if(!bypass)
		ddr_writel(0 , DDRC_DLP);
#endif
	ddr_writel(0x1 ,DDRC_AUTOSR_EN);
#ifdef CONFIG_X1600
	ddr_writel(CONFIG_DDR_AUTO_SELF_REFRESH_CNT ,DDRC_AUTOSR_CNT);
#endif
#endif

#ifdef CONFIG_X1600
	{
		unsigned int dlp = 0;
		dlp = ddr_readl(DDRC_DLP);
		dlp |= DDRC_DDLP_FSR | DDRC_DDLP_FPD | DDRC_DDLP_LPEN;
		ddr_writel(dlp, DDRC_DLP);
	}
#else
	ddr_writel(0 , DDRC_DLP);
#endif



	dump_ddrc_register();
	dwc_debug("sdram init finished\n");
#undef DDRTYPE
	(void)rate;(void)bypass;(void)mode;
}

phys_size_t initdram(int board_type)
{
#ifndef EMC_LOW_SDRAM_SPACE_SIZE
#define EMC_LOW_SDRAM_SPACE_SIZE 0x10000000 /* 256M */
#endif /* EMC_LOW_SDRAM_SPACE_SIZE */
        unsigned int ram_size;

	get_ddr_type();

        ram_size = (unsigned int)(DDR_CHIP_0_SIZE) + (unsigned int)(DDR_CHIP_1_SIZE);
        if (ram_size > EMC_LOW_SDRAM_SPACE_SIZE)
                ram_size = EMC_LOW_SDRAM_SPACE_SIZE;

        return ram_size;
}
