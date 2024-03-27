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
/* #define DEBUG_READ_WRITE */
#include <config.h>
#include <common.h>
#include <ddr/ddr_common.h>
#ifndef CONFIG_BURNER
//#include <generated/ddr_reg_values.h>
extern struct ddr_reg_value supported_ddr_reg_values[];
#endif

#include <asm/io.h>
#include <asm/arch/clk.h>

/*#define CONFIG_DWC_DEBUG 0*/
#define ddr_hang() do{						\
		debug("%s %d\n",__FUNCTION__,__LINE__);	\
		hang();						\
	}while(0)

DECLARE_GLOBAL_DATA_PTR;
extern struct ddr_reg_value *global_reg_value __attribute__ ((section(".data")));

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


#else
#define FUNC_ENTER()
#define FUNC_EXIT()

#define dump_ddrc_register()
#define dump_ddrp_register()
#endif


void ddrp_pll_init(void)
{
	unsigned int val;

	val = ddr_readl(DDRP_INNOPHY_PLL_FBDIV);
	val &= ~(0xff);
	val |= 0x8;
	ddr_writel(val, DDRP_INNOPHY_PLL_FBDIV);

	val = ddr_readl(DDRP_INNOPHY_PLL_PDIV);
	val &= ~(0xff);
	val |= 4;
	ddr_writel(val, DDRP_INNOPHY_PLL_PDIV);

	ddr_writel(ddr_readl(DDRP_INNOPHY_PLL_CTRL) | DDRP_PLL_CTRL_PLLPDEN, DDRP_INNOPHY_PLL_CTRL);
	debug("DDRP_INNOPHY_PLL_LOCK: %x\n", ddr_readl(DDRP_INNOPHY_PLL_LOCK));
	ddr_writel(ddr_readl(DDRP_INNOPHY_PLL_CTRL) & ~DDRP_PLL_CTRL_PLLPDEN, DDRP_INNOPHY_PLL_CTRL);
	debug("DDRP_INNOPHY_PLL_LOCK: %x\n", ddr_readl(DDRP_INNOPHY_PLL_LOCK));

	while(!(ddr_readl(DDRP_INNOPHY_PLL_LOCK) & (1 << 3)));
	debug("DDRP_INNOPHY_PLL_LOCK: %x\n", ddr_readl(DDRP_INNOPHY_PLL_LOCK));

}

void ddrp_cfg(struct ddr_reg_value *global_reg_value)
{
	unsigned int val;
#ifdef DEBUG_READ_WRITE
	val = ddr_readl(DDRP_INNOPHY_DQ_WIDTH);
	val &= ~(0x3);
	val |= DDRP_DQ_WIDTH_DQ_H | DDRP_DQ_WIDTH_DQ_L;
	ddr_writel(val, DDRP_INNOPHY_DQ_WIDTH);

	val = ddr_readl(DDRP_INNOPHY_MEM_CFG);
	val &= ~(0x3 | 1 << 4);
	val |= 1 << 4 | 3;
	ddr_writel(val, DDRP_INNOPHY_MEM_CFG);

	debug("ddr_readl(DDRP_INNOPHY_CL)  %x\n", ddr_readl(DDRP_INNOPHY_CL));
	debug("ddr_readl(DDRP_INNOPHY_CWL)  %x\n", ddr_readl(DDRP_INNOPHY_CWL));
#else
	ddr_writel(DDRP_DQ_WIDTH_DQ_H | DDRP_DQ_WIDTH_DQ_L, DDRP_INNOPHY_DQ_WIDTH);
	ddr_writel(global_reg_value->DDRP_MEMCFG_VALUE, DDRP_INNOPHY_MEM_CFG);
#endif

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

	debug("ddr_readl(DDRP_INNOPHY_CL)   %x\n", ddr_readl(DDRP_INNOPHY_CL));
	debug("ddr_readl(DDRP_INNOPHY_CWL)  %x\n", ddr_readl(DDRP_INNOPHY_CWL));
	debug("ddr_readl(DDRP_INNOPHY_AL)   %x\n", ddr_readl(DDRP_INNOPHY_AL));
}

/*
 * Name     : ddrp_calibration()
 * Function : control the RX DQS window delay to the DQS
 *
 * a_low_8bit_delay	= al8_2x * clk_2x + al8_1x * clk_1x;
 * a_high_8bit_delay	= ah8_2x * clk_2x + ah8_1x * clk_1x;
 *
 * */
#if 0
static void ddrp_calibration(int al8_1x,int ah8_1x,int al8_2x,int ah8_2x)
{
	ddr_writel(ddr_readl(DDRP_INNOPHY_TRAINING_CTRL) | DDRP_TRAINING_CTRL_DSCSE_BP, DDRP_INNOPHY_TRAINING_CTRL);

	int x = ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AL);
	int y = ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AH);
	x = (x & ~(0xf << 3)) | (al8_1x << DDRP_CALIB_BP_CYCLESELBH_BIT) | (al8_2x << DDRP_CALIB_BP_OPHCSELBH_BIT);
	y = (y & ~(0xf << 3)) | (ah8_1x << DDRP_CALIB_BP_CYCLESELBH_BIT) | (ah8_2x << DDRP_CALIB_BP_OPHCSELBH_BIT);
	ddr_writel(x, DDRP_INNOPHY_CALIB_BYPASS_AL);
	ddr_writel(y, DDRP_INNOPHY_CALIB_BYPASS_AH);
}
#endif
void ddrp_auto_calibration(void)
{
	unsigned int reg_val = ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	unsigned int timeout = 0xffffff;
	unsigned int wait_cal_done = DDRP_CALIB_DONE_HDQCFA | DDRP_CALIB_DONE_LDQCFA;

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


#if 0
	{
		unsigned int cycsel, tmp;
		unsigned int read_data0, read_data1;
		unsigned int c0, c1;
		unsigned int max;

		read_data0 = *(volatile unsigned int *)(0xb3011000 + (0x64 << 2));
		read_data1 = *(volatile unsigned int *)(0xb3011000 + (0x65 << 2));
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

	printf("ddr calib test finish\n");
#endif

}


struct ddrp_calib {
	union{
		uint8_t u8;
		struct{
			uint8_t dllsel:3;
			uint8_t ophsel:1;
			uint8_t cyclesel:3;
		}b;
	}bypass;
	union{
		uint8_t u8;
		struct{
			uint8_t reserved:5;
			uint8_t rx_dll:3;
		}b;
	}rx_dll;
};

/*
 * Name     : ddrp_calibration_manual()
 * Function : control the RX DQS window delay to the DQS
 *
 * a_low_8bit_delay	= al8_2x * clk_2x + al8_1x * clk_1x;
 * a_high_8bit_delay	= ah8_2x * clk_2x + ah8_1x * clk_1x;
 *
 * */


#ifdef CONFIG_DDRP_SOFTWARE_TRAINING
void ddrp_software_calibration(void)
{

	int x, y, z;
	int c, o, d =0, r = 0;
	unsigned int addr = 0xa0100000, val;
	unsigned int i, n, m = 0;
	struct ddrp_calib calib_val[8 * 2 * 8 * 5];

	unsigned int reg_val = ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	unsigned int timeout = 0xffffff;
	unsigned int wait_cal_done = DDRP_CALIB_DONE_HDQCFA | DDRP_CALIB_DONE_LDQCFA;

	reg_val |= (DDRP_TRAINING_CTRL_DSCSE_BP);
	reg_val &= ~DDRP_TRAINING_CTRL_DSACE_START;
	ddr_writel(reg_val, DDRP_INNOPHY_TRAINING_CTRL);
	debug("-----DDRP_INNOPHY_TRAINING_CTRL: %x\n", ddr_readl(DDRP_INNOPHY_TRAINING_CTRL));
	debug("before trainning %x %x %x %x\n",ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AL),
	ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AH),ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AL),ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AH));


	for(c = 0; c < 8; c ++) {
		for(o = 0; o < 2; o++) {
			for(d = 0; d < 8; d++)
			{
				x = c << 4 | o << 3 | d;
				y = c << 4 | o << 3 | d;
				unsigned int val1 = ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AL) & (~0x7f);
				ddr_writel(x | val1, DDRP_INNOPHY_CALIB_BYPASS_AL);
				unsigned int val2 = ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AH) & (~0x7f);
				ddr_writel(y | val2, DDRP_INNOPHY_CALIB_BYPASS_AH);

				for(r = 0; r < 4; r++) {
					unsigned int val1,val2;

#if 0
					val1 = ddr_readl(DDRP_INNOPHY_FPGA_RXDLL_DELAY) & (~(0x7 << 4)) & ((~(0x7 << 0))) ;
					ddr_writel(r << 0 | r << 4 | val1, DDRP_INNOPHY_FPGA_RXDLL_DELAY);
#else
					val1 = ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AL) & (~0x3);
					ddr_writel((r << 0) | val1, DDRP_INNOPHY_RXDLL_DELAY_AL);
					val1 = ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AH) & (~0x3);
					ddr_writel((r << 0)|val1, DDRP_INNOPHY_RXDLL_DELAY_AH);

#endif

					for(i = 0; i < 256; i++) {
						val = 0;
						for(n = 0; n < 4; n++ ) {
							val |= i<<(n * 8);
						}
						*(volatile unsigned int *)(addr + i * 4) = val;
						volatile unsigned int val1;
						val1 = *(volatile unsigned int *)(addr + i * 4);
//						printf("c: %d, o: %d, d: %d, r: %d val1: %x, val: %x\n", c, o, d, r, val1, val);
#if 1
						if(val1 != val)
							break;
#endif
					}
					if(i == 256) {
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
		debug("calib bypass fail\n");
		return ;
	}

	debug("total trainning pass params: %d\n", m);
#ifdef DDR_CHOOSE_PARAMS
	m = choose_params(m);
#else
	m = m  / 2;
#endif


	c = calib_val[m].bypass.b.cyclesel;
	o = calib_val[m].bypass.b.ophsel;
	d = calib_val[m].bypass.b.dllsel;
	r = calib_val[m].rx_dll.b.rx_dll;

	x = c << 4 | o << 3 | d;
	y = c << 4 | o << 3 | d;
	z = r << 4 | r << 0;
	ddr_writel(x, DDRP_INNOPHY_CALIB_BYPASS_AL);
	ddr_writel(y, DDRP_INNOPHY_CALIB_BYPASS_AH);

#if 0
	unsigned int rxdll = ddr_readl(DDRP_INNOPHY_FPGA_RXDLL_DELAY) & (~(0x7 << 4)) & ((~(0x7 << 0))) ;
	ddr_writel(z | rxdll, DDRP_INNOPHY_FPGA_RXDLL_DELAY);
#else
	//unsigned int rxdll = ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AL) & 0x3;
	//rxdll = ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AH) & 0x3;
	ddr_writel(ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AL) | r << 0, DDRP_INNOPHY_RXDLL_DELAY_AL);
	ddr_writel(ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AH) | r << 0, DDRP_INNOPHY_RXDLL_DELAY_AH);
#endif

	{
		struct ddrp_calib b_al, b_ah, r_al,r_ah;
		b_al.bypass.u8 = ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AL);
		debug("bypass :CALIB_AL: dllsel %x, ophsel %x, cyclesel %x\n",b_al.bypass.b.dllsel, b_al.bypass.b.ophsel, b_al.bypass.b.cyclesel);
		b_ah.bypass.u8 = ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AH);
		debug("bypass:CAHIB_AH: dllsel %x, ophsel %x, cyclesel %x\n", b_ah.bypass.b.dllsel, b_ah.bypass.b.ophsel, b_ah.bypass.b.cyclesel);
		//debug("fpga rxdll delay %x\n", ddr_readl(DDRP_INNOPHY_FPGA_RXDLL_DELAY));

	}
	ddr_writel(0, DDRP_INNOPHY_TRAINING_CTRL);

}
#endif


