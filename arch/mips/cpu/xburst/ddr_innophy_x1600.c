
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
#include <asm/io.h>
#include "ddr_innophy.h"
#include "ddr_debug.h"
#ifndef CONFIG_BURNER
#include <generated/ddr_reg_values.h>
#endif

#define ddr_hang() do{						\
		debug("%s %d\n",__FUNCTION__,__LINE__);	\
		hang();						\
	}while(0)

DECLARE_GLOBAL_DATA_PTR;

struct ddr_reg_value *global_reg_value __attribute__ ((section(".data")));

/*#define CONFIG_DDRP_SOFTWARE_TRAINING*/

extern struct ddr_reg_value supported_ddr_reg_values[];


int current_ddr_type;


struct ddr_params *ddr_params_p = NULL;
extern void reset_dll(void);
#define BYPASS_ENABLE       1
#define BYPASS_DISABLE      0
#define IS_BYPASS_MODE(x)     (((x) & 1) == BYPASS_ENABLE)
#define DDR_TYPE_MODE(x)     (((x) >> 1) & 0xf)


/*#define CONFIG_DWC_DEBUG*/
#ifdef  CONFIG_DWC_DEBUG
static void dump_ddrc_register(void)
{
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
}

void dump_generated_reg(struct ddr_reg_value *reg)
{
	int i;
	printf("name		      = %s\n", reg->h.name);
	printf("id		      = %x\n", reg->h.id);
	printf("type		      = %x\n", reg->h.type);
	printf("freq		      = %x\n", reg->h.freq);
	printf("DDRC_CFG_VALUE        = %x\n", reg->DDRC_CFG_VALUE);
	printf("DDRC_CTRL_VALUE       = %x\n", reg->DDRC_CTRL_VALUE);
	printf("DDRC_DLMR_VALUE       = %x\n", reg->DDRC_DLMR_VALUE);
	printf("DDRC_DDLP_VALUE       = %x\n", reg->DDRC_DDLP_VALUE);
	printf("DDRC_MMAP0_VALUE      = %x\n", reg->DDRC_MMAP0_VALUE);
	printf("DDRC_MMAP1_VALUE      = %x\n", reg->DDRC_MMAP1_VALUE);
	printf("DDRC_REFCNT_VALUE     = %x\n", reg->DDRC_REFCNT_VALUE);
	printf("DDRC_TIMING1_VALUE    = %x\n", reg->DDRC_TIMING1_VALUE);
	printf("DDRC_TIMING2_VALUE    = %x\n", reg->DDRC_TIMING2_VALUE);
	printf("DDRC_TIMING3_VALUE    = %x\n", reg->DDRC_TIMING3_VALUE);
	printf("DDRC_TIMING4_VALUE    = %x\n", reg->DDRC_TIMING4_VALUE);
	printf("DDRC_TIMING5_VALUE    = %x\n", reg->DDRC_TIMING5_VALUE);
	printf("DDRP_MEMCFG_VALUE     = %x\n", reg->DDRP_MEMCFG_VALUE);
	printf("DDRP_CL_VALUE         = %x\n", reg->DDRP_CL_VALUE);
	printf("DDRP_CWL_VALUE        = %x\n", reg->DDRP_CWL_VALUE);
	printf("DDR_MR0_VALUE         = %x\n", reg->DDR_MR0_VALUE);
	printf("DDR_MR1_VALUE         = %x\n", reg->DDR_MR1_VALUE);
	printf("DDR_MR2_VALUE         = %x\n", reg->DDR_MR2_VALUE);
	printf("DDR_MR3_VALUE         = %x\n", reg->DDR_MR3_VALUE);
	printf("DDR_MR10_VALUE        = %x\n", reg->DDR_MR10_VALUE);
	printf("DDR_MR11_VALUE        = %x\n", reg->DDR_MR11_VALUE);
	printf("DDR_MR63_VALUE        = %x\n", reg->DDR_MR63_VALUE);
	printf("DDR_CHIP_0_SIZE       = %x\n", reg->DDR_CHIP_0_SIZE);
	printf("DDR_CHIP_1_SIZE       = %x\n", reg->DDR_CHIP_1_SIZE);
	for(i = 0; i < 5; i++) {
		printf("REMMAP_ARRAY[%d] = %x\n", i, reg->REMMAP_ARRAY[i]);
	}
}

#else

#define dump_ddrc_register()
void dump_generated_reg(struct ddr_reg_value *reg){}
#endif

void ddrp_pll_init(void)
{
	unsigned int val;

	val = ddr_readl(DDRP_INNOPHY_PLL_FBDIV);
	val &= ~(0xff);
	val |= 0x10;
	ddr_writel(val, DDRP_INNOPHY_PLL_FBDIV);

	ddr_writel(ddr_readl(DDRP_INNOPHY_PLL_CTRL) | DDRP_PLL_CTRL_PLLPDEN, DDRP_INNOPHY_PLL_CTRL);
	debug("DDRP_INNOPHY_PLL_LOCK: %x\n", ddr_readl(DDRP_INNOPHY_PLL_LOCK));

	val = ddr_readl(DDRP_INNOPHY_PLL_PDIV);
	val &= ~(0xff);
	val |= 0x24;
	ddr_writel(val, DDRP_INNOPHY_PLL_PDIV);

	udelay(5);
	ddr_writel(ddr_readl(DDRP_INNOPHY_PLL_CTRL) & ~DDRP_PLL_CTRL_PLLPDEN, DDRP_INNOPHY_PLL_CTRL);
	debug("DDRP_INNOPHY_PLL_LOCK: %x\n", ddr_readl(DDRP_INNOPHY_PLL_LOCK));
	udelay(5);

	while(!(ddr_readl(DDRP_INNOPHY_PLL_LOCK) & (1 << 3)));
	udelay(5);
	while(!((*(volatile unsigned int *)0xb301208c) & (1 << 2)));
	udelay(5);
	debug("DDRP_INNOPHY_PLL_LOCK: %x\n", ddr_readl(DDRP_INNOPHY_PLL_LOCK));
	udelay(5);
}

void ddrp_cfg(struct ddr_reg_value *global_reg_value)
{
	unsigned int val;
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

	/* ????   reserve   read only   ???? */
	val = ddr_readl(DDRC_CTRL);
	val &= ~ (1 << 20);
	ddr_writel(val, DDRC_CTRL);


	debug("ddr_readl(DDRP_INNOPHY_CL)   %x\n", ddr_readl(DDRP_INNOPHY_CL));
	debug("ddr_readl(DDRP_INNOPHY_CWL)  %x\n", ddr_readl(DDRP_INNOPHY_CWL));
	debug("ddr_readl(DDRP_INNOPHY_AL)   %x\n", ddr_readl(DDRP_INNOPHY_AL));
}

void ddrp_auto_calibration(void)
{
	unsigned int reg_val = ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	unsigned int timeout = 0xffffff;
	unsigned int wait_cal_done = DDRP_CALIB_DONE_HDQCFA | DDRP_CALIB_DONE_LDQCFA;
	unsigned int al, ah;

	reg_val &= ~(DDRP_TRAINING_CTRL_DSCSE_BP);
	reg_val |= DDRP_TRAINING_CTRL_DSACE_START;
	ddr_writel(reg_val, DDRP_INNOPHY_TRAINING_CTRL);

	while(!((ddr_readl(DDRP_INNOPHY_CALIB_DONE) & 0x13) == 3) && --timeout) {

		udelay(1);
		printf("DDRP_INNOPHY_CALIB_DELAY_AL:%x\n", ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AL));
		printf("DDRP_INNOPHY_CALIB_DELAY_AH:%x\n", ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AH));
		printf("-----ddr_readl(DDRP_INNOPHY_CALIB_DONE): %x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE));
	}

	if(!timeout) {
		debug("ddrp_auto_calibration failed!\n");
	}
	ddr_writel(0, DDRP_INNOPHY_TRAINING_CTRL);
	debug("ddrp_auto_calibration success!\n");

	mdelay(5);
	reg_val = ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	reg_val &= ~(1);
	ddr_writel(reg_val, DDRP_INNOPHY_TRAINING_CTRL);

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

static void mem_remap(void)
{
	int i;
	unsigned int *remap;

	remap = global_reg_value->REMMAP_ARRAY;


	for(i = 0;i < ARRAY_SIZE(global_reg_value->REMMAP_ARRAY);i++){
		ddr_writel(remap[i], DDRC_REMAP(i+1));
	}
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
	mem_remap();
	ddr_writel(global_reg_value->DDRC_REFCNT_VALUE, DDRC_REFCNT);
	debug("DDRC_STATUS: %x\n",ddr_readl(DDRC_STATUS));
	ddr_writel(global_reg_value->DDRC_CTRL_VALUE, DDRC_CTRL);
}

static void ddrc_prev_init(void)
{
	dwc_debug("DDR Controller init\n");
	/* DDRC timing init*/
	ddr_writel(global_reg_value->DDRC_TIMING1_VALUE, DDRC_TIMING(1));
	ddr_writel(global_reg_value->DDRC_TIMING2_VALUE, DDRC_TIMING(2));
	ddr_writel(global_reg_value->DDRC_TIMING3_VALUE, DDRC_TIMING(3));
	ddr_writel(global_reg_value->DDRC_TIMING4_VALUE, DDRC_TIMING(4));
	ddr_writel(global_reg_value->DDRC_TIMING5_VALUE, DDRC_TIMING(5));
	ddr_writel(global_reg_value->DDRC_TIMING6_VALUE, DDRC_TIMING(6));

	/* DDRC memory map configure*/
	ddr_writel(global_reg_value->DDRC_MMAP0_VALUE, DDRC_MMAP0);
	ddr_writel(global_reg_value->DDRC_MMAP1_VALUE, DDRC_MMAP1);
	ddr_writel(global_reg_value->DDRC_CTRL_VALUE & 0xffff8fff, DDRC_CTRL);
}

void ddrc_dfi_init(void)
{
	u32 reg = 0;

	writel(1, DDR_APB_PHY_INIT); //start high
	writel(0, DDR_APB_PHY_INIT); //start low
	while(!(readl(DDR_APB_PHY_INIT) & (1<<1))); //polling dfi init comp
#ifndef CONFIG_FASTBOOT
	printf("ddr_inno_phy_init ..! 11:  %X\n", readl(DDR_APB_PHY_INIT));
#endif

	reg = ddr_readl(DDRC_CTRL);
	reg |= (1 << 23);
	ddr_writel(reg, DDRC_CTRL); //set dfi_reset_n high

	ddr_writel(global_reg_value->DDRC_CFG_VALUE, DDRC_CFG);

	ddr_writel(DDRC_CTRL_CKE, DDRC_CTRL); // set CKE to high

	ddr_writel(7 << 8 | 1 << 0, DDRC_LMR); //Send All bank precharge.
	mdelay(1);

	if(current_ddr_type == DDR3) {
#define DDRC_LMR_MR(n)						\
	global_reg_value->DDRC_DLMR_VALUE | 0x1 | (2 << 3) |			\
		((global_reg_value->DDR_MR##n##_VALUE & 0xffff) << 12) |		\
		(((global_reg_value->DDR_MR##n##_VALUE >> 16) & 0x7) << 8)

	printf("MR0 : 0x%x\n", DDRC_LMR_MR(0));
	printf("MR1 : 0x%x\n", DDRC_LMR_MR(1));
	printf("MR2 : 0x%x\n", DDRC_LMR_MR(2));
	printf("MR3 : 0x%x\n", DDRC_LMR_MR(3));
	printf("ZQCL : 0x%x\n", global_reg_value->DDRC_DLMR_VALUE | (0x4 << 3) | 0x1);

	ddr_writel(DDRC_LMR_MR(0)/*0x1a30011*/, DDRC_LMR); //MR0
	ddr_writel(DDRC_LMR_MR(1)/*0x6111*/, DDRC_LMR); //MR1
	ddr_writel(DDRC_LMR_MR(2)/*0x8211*/, DDRC_LMR); //MR2
	ddr_writel(DDRC_LMR_MR(3)/*0x311*/, DDRC_LMR); //MR3
	ddr_writel(global_reg_value->DDRC_DLMR_VALUE | (0x4 << 3) | 0x1/*0x19*/, DDRC_LMR);

#undef DDRC_LMR_MR
	} else if(current_ddr_type == LPDDR2) {
#define DDRC_LMR_MR(n)										\
		global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |	\
			((global_reg_value->DDR_MR##n##_VALUE & 0xff) << 24) |						\
			(((global_reg_value->DDR_MR##n##_VALUE >> 8) & 0xff) << (16))
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
#define DDRC_LMR_MR(n)										\
		global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |	\
			(((global_reg_value->DDR_MR##n##_VALUE  >> 13) & 0x3) << 8) |						\
			(((global_reg_value->DDR_MR##n##_VALUE ) & 0x1fff) << (12))

		ddr_writel(DDRC_LMR_MR(0), DDRC_LMR);
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR);
		mdelay(1);
		printf("mr0 = 0x%x\n", DDRC_LMR_MR(0));
		printf("mr1 = 0x%x\n", DDRC_LMR_MR(1));
#undef DDRC_LMR_MR
	}
}

void ddrp_wl_training(void)
{

	if(current_ddr_type == DDR3) {
		//write level
		printf("WL_MODE1 : 0x%x\n", global_reg_value->DDR_MR1_VALUE & 0xff);
		phy_writel(global_reg_value->DDR_MR1_VALUE & 0xff, INNO_WL_MODE1);
		phy_writel(0x40, INNO_WL_MODE2);
		phy_writel(0xa4, INNO_TRAINING_CTRL);
		while (0x3 != phy_readl(INNO_WL_DONE));
		phy_writel(0xa1, INNO_TRAINING_CTRL);
	}

	/* ???? */
		phy_writel(0x50, INNO_MEM_CFG);
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

int get_ddr_type(void)
{
	int type;

	type = global_reg_value->h.type;

	switch(global_reg_value->h.type){

		case DDR3:
			printf("DDR: %s type is : DDR3\n", global_reg_value->h.name);
			break;
		case LPDDR:
			printf("DDR: %s type is : LPDDR\n", global_reg_value->h.name);
			break;
		case LPDDR2:
			printf("DDR: %s type is : LPDDR2\n", global_reg_value->h.name);
			break;
		case LPDDR3:
			printf("DDR: %s type is : LPDDR3\n", global_reg_value->h.name);
			break;
		case DDR2:
			printf("DDR: %s type is : DDR2\n", global_reg_value->h.name);
			break;
		default:
			type = UNKOWN;
			printf("unsupport ddr type!\n");
			ddr_hang();
	}

	return type;
}
#ifdef CONFIG_DDRP_SOFTWARE_TRAINING
struct ddrp_calib {
	union{
		uint8_t u8;
		struct{
			unsigned dllsel:3;
			unsigned ophsel:1;
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
	struct ddrp_calib calib_val[8 * 2 * 8 * 5];

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
#endif

#ifndef CONFIG_BURNER
void get_ddr_params_normal(void)
{
	int found = 0;
	int size = 0;
	int i;
	unsigned int burned_ddr_id = *(volatile unsigned int *)(CONFIG_SPL_TEXT_BASE + 128);

	if((burned_ddr_id & 0xffff) != (burned_ddr_id >> 16)) {
		printf("invalid burned ddr id\n");
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
	dump_generated_reg(global_reg_value);

}

unsigned int get_ddr_size(void)
{
#ifdef CONFIG_X1660_DDR
	return 32;
#else
	return 64;
#endif
}

/* DDR sdram init */
void sdram_init(void)
{

	unsigned int mode;
	unsigned int bypass = 0;
	unsigned int rate;
	unsigned int reg_val;

	dwc_debug("sdram init start\n");

	get_ddr_params();

	current_ddr_type = get_ddr_type();

	clk_set_rate(DDR, CONFIG_SYS_MEM_FREQ);
	reset_dll();
	rate = clk_get_rate(DDR);
	if(rate != CONFIG_SYS_MEM_FREQ)
		dwc_debug("sdram set ddr freq failed\n");

	reset_controller();

#ifdef CONFIG_DDR_AUTO_SELF_REFRESH
	ddr_writel(0x0 ,DDRC_AUTOSR_EN);
#endif

	ddrp_cfg(global_reg_value);
	ddrp_pll_init();
	ddrc_dfi_init();

        /* DDR Controller init*/
	ddrc_prev_init();
	dwc_debug("DDR PHY init OK\n");
	ddrc_post_init();

#ifdef CONFIG_DDRP_SOFTWARE_TRAINING
	ddrp_software_calibration();
#else
	ddrp_auto_calibration();
#endif

#ifdef CONFIG_DDR_AUTO_SELF_REFRESH
	ddr_writel(0x1 ,DDRC_AUTOSR_EN);
	ddr_writel(CONFIG_DDR_AUTO_SELF_REFRESH_CNT ,DDRC_AUTOSR_CNT);
#endif
	{
		unsigned int dlp = 0;
		dlp = ddr_readl(DDRC_DLP);
		dlp |= DDRC_DDLP_FSR | DDRC_DDLP_FPD | DDRC_DDLP_LPEN;
		ddr_writel(dlp, DDRC_DLP);
	}

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

	get_ddr_params();

        ram_size = (unsigned int)(global_reg_value->DDR_CHIP_0_SIZE) + (unsigned int)(global_reg_value->DDR_CHIP_1_SIZE);
        if (ram_size > EMC_LOW_SDRAM_SPACE_SIZE)
               ram_size = EMC_LOW_SDRAM_SPACE_SIZE;

	return ram_size;
}
