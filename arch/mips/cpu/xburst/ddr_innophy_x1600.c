
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
#include <asm/arch/cpm.h>
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

static struct phy_drvodt_config *drvodt = NULL;
static struct phy_deskew_config *deskew = NULL;

/* #define CONFIG_DDRP_SOFTWARE_TRAINING */

extern struct ddr_reg_value supported_ddr_reg_values[];

#ifdef CONFIG_DDR_DRVODT_DEBUG
#include <asm/spl.h>
#include <netdev.h>
#include <command.h>
#include <asm/reboot.h>
#include <asm/mipsregs.h>
#include <asm/cacheops.h>
#include <asm/arch/wdt.h>
#include <asm/jz_cache.h>
int slpc = 0;
int pass_value=0;
char drv_value = 0;
char odt_value = 0;
char restart_count = 0;
char fail_flag = 0;
unsigned int pass_count = 0;

void drv_odt_value_ergodic(void){
	drv_value = (slpc & 0xff);
	odt_value = (slpc >> 8) & 0xff;
	pass_value = 0;
	serial_debug("drv value is %x odt value is %x pass_value is %x\n",drv_value,odt_value,pass_value);
	odt_value += 1;

	if(odt_value > 0x1f){
		drv_value += 1;
		odt_value = 0;
	}
	if(drv_value <= 0x1f) {
		restart_count = 0;
		slpc = drv_value | (odt_value << 8) | (restart_count << 16);
		cpm_outl(slpc,CPM_SLPC);
	}
}

void  _machine_restart(void){
	if(fail_flag == -1){
		drv_odt_value_ergodic();
	}

        int time = RTC_FREQ / WDT_DIV * RESET_DELAY_MS / 1000;

	if(time > 65535)
		time = 65535;
	writel(TSCR_WDTSC, TCU_BASE + TCU_TSCR);

	writel(0, WDT_BASE + WDT_TCNT);
	writel(time, WDT_BASE + WDT_TDR);
	writel(TCSR_PRESCALE | TCSR_RTC_EN
#if (defined(CONFIG_X1600))
			| TCSR_CLRZ
#endif
			, WDT_BASE + WDT_TCSR);
	writel(0,WDT_BASE + WDT_TCER);

	/*serial_debug("reset in %dms\n", RESET_DELAY_MS);*/
	writel(TCER_TCEN,WDT_BASE + WDT_TCER);
	mdelay(1000);
}
#endif

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
	serial_debug("DDRC_TIMING6        0x%x\n", ddr_readl(DDRC_TIMING(6)));
	serial_debug("DDRC_REFCNT         0x%x\n", ddr_readl(DDRC_REFCNT));
	serial_debug("DDRC_MMAP0          0x%x\n", ddr_readl(DDRC_MMAP0));
	serial_debug("DDRC_MMAP1          0x%x\n", ddr_readl(DDRC_MMAP1));
	serial_debug("DDRC_REMAP1         0x%x\n", ddr_readl(DDRC_REMAP(1)));
	serial_debug("DDRC_REMAP2         0x%x\n", ddr_readl(DDRC_REMAP(2)));
	serial_debug("DDRC_REMAP3         0x%x\n", ddr_readl(DDRC_REMAP(3)));
	serial_debug("DDRC_REMAP4         0x%x\n", ddr_readl(DDRC_REMAP(4)));
	serial_debug("DDRC_REMAP5         0x%x\n", ddr_readl(DDRC_REMAP(5)));
	serial_debug("DDRC_AUTOSR_EN      0x%x\n", ddr_readl(DDRC_AUTOSR_EN));
	serial_debug("INNO_DQ_WIDTH   :%X\n",phy_readl(INNO_DQ_WIDTH));
	serial_debug("INNO_PLL_FBDIV  :%X\n",phy_readl(INNO_PLL_FBDIV));
	serial_debug("INNO_PLL_PDIV   :%X\n",phy_readl(INNO_PLL_PDIV));
	serial_debug("INNO_MEM_CFG    :%X\n",phy_readl(INNO_MEM_CFG));
	serial_debug("INNO_PLL_CTRL   :%X\n",phy_readl(INNO_PLL_CTRL));
	serial_debug("INNO_CHANNEL_EN :%X\n",phy_readl(INNO_CHANNEL_EN));
	serial_debug("INNO_CWL        :%X\n",phy_readl(INNO_CWL));
	serial_debug("INNO_CL         :%X\n",phy_readl(INNO_CL));
}

void dump_generated_reg(struct ddr_reg_value *reg)
{
	int i;
	serial_debug("name		      = %s\n", reg->h.name);
	serial_debug("id		      = %x\n", reg->h.id);
	serial_debug("type		      = %x\n", reg->h.type);
	serial_debug("freq		      = %x\n", reg->h.freq);
	serial_debug("DDRC_CFG_VALUE        = %x\n", reg->DDRC_CFG_VALUE);
	serial_debug("DDRC_CTRL_VALUE       = %x\n", reg->DDRC_CTRL_VALUE);
	serial_debug("DDRC_DLMR_VALUE       = %x\n", reg->DDRC_DLMR_VALUE);
	serial_debug("DDRC_DDLP_VALUE       = %x\n", reg->DDRC_DDLP_VALUE);
	serial_debug("DDRC_MMAP0_VALUE      = %x\n", reg->DDRC_MMAP0_VALUE);
	serial_debug("DDRC_MMAP1_VALUE      = %x\n", reg->DDRC_MMAP1_VALUE);
	serial_debug("DDRC_REFCNT_VALUE     = %x\n", reg->DDRC_REFCNT_VALUE);
	serial_debug("DDRC_TIMING1_VALUE    = %x\n", reg->DDRC_TIMING1_VALUE);
	serial_debug("DDRC_TIMING2_VALUE    = %x\n", reg->DDRC_TIMING2_VALUE);
	serial_debug("DDRC_TIMING3_VALUE    = %x\n", reg->DDRC_TIMING3_VALUE);
	serial_debug("DDRC_TIMING4_VALUE    = %x\n", reg->DDRC_TIMING4_VALUE);
	serial_debug("DDRC_TIMING5_VALUE    = %x\n", reg->DDRC_TIMING5_VALUE);
	serial_debug("DDRP_MEMCFG_VALUE     = %x\n", reg->DDRP_MEMCFG_VALUE);
	serial_debug("DDRP_CL_VALUE         = %x\n", reg->DDRP_CL_VALUE);
	serial_debug("DDRP_CWL_VALUE        = %x\n", reg->DDRP_CWL_VALUE);
	serial_debug("DDR_MR0_VALUE         = %x\n", reg->DDR_MR0_VALUE);
	serial_debug("DDR_MR1_VALUE         = %x\n", reg->DDR_MR1_VALUE);
	serial_debug("DDR_MR2_VALUE         = %x\n", reg->DDR_MR2_VALUE);
	serial_debug("DDR_MR3_VALUE         = %x\n", reg->DDR_MR3_VALUE);
	serial_debug("DDR_MR10_VALUE        = %x\n", reg->DDR_MR10_VALUE);
	serial_debug("DDR_MR11_VALUE        = %x\n", reg->DDR_MR11_VALUE);
	serial_debug("DDR_MR63_VALUE        = %x\n", reg->DDR_MR63_VALUE);
	serial_debug("DDR_CHIP_0_SIZE       = %x\n", reg->DDR_CHIP_0_SIZE);
	serial_debug("DDR_CHIP_1_SIZE       = %x\n", reg->DDR_CHIP_1_SIZE);
	for(i = 0; i < 5; i++) {
		serial_debug("REMMAP_ARRAY[%d] = %x\n", i, reg->REMMAP_ARRAY[i]);
	}
}

#else

#define dump_ddrc_register()
void dump_generated_reg(struct ddr_reg_value *reg){}
#endif

#ifdef CONFIG_DDR_DRVODT_DEBUG
#define DDRP_INNOPHY_PD_DRV_CMD         (DDR_PHY_OFFSET + (0xb0<<2))        //0x130
#define DDRP_INNOPHY_PU_DRV_CMD         (DDR_PHY_OFFSET + (0xb1<<2))        //0x131
#define DDRP_INNOPHY_PD_ODT_DQ7_0       (DDR_PHY_OFFSET + (0xc0<<2))        //0x140
#define DDRP_INNOPHY_PU_ODT_DQ7_0       (DDR_PHY_OFFSET + (0xc1<<2))        //0x141
#define DDRP_INNOPHY_PD_DRV_DQ7_0       (DDR_PHY_OFFSET + (0xc2<<2))        //0x142
#define DDRP_INNOPHY_PU_DRV_DQ7_0       (DDR_PHY_OFFSET + (0xc3<<2))        //0x143
#define DDRP_INNOPHY_PD_ODT_DQ15_8      (DDR_PHY_OFFSET + (0xd0<<2))        //0X150
#define DDRP_INNOPHY_PU_ODT_DQ15_8      (DDR_PHY_OFFSET + (0xd1<<2))        //0x151
#define DDRP_INNOPHY_PD_DRV_DQ15_8      (DDR_PHY_OFFSET + (0xd2<<2))        //0x152
#define DDRP_INNOPHY_PU_DRV_DQ15_8      (DDR_PHY_OFFSET + (0xd3<<2))        //0x153

static void ddrp_set_dq_odt(struct phy_drvodt_config *drvodt)
{
	unsigned int pu = drvodt->phy_pu_odt_dq7_0;
        unsigned int pd = drvodt->phy_pd_odt_dq7_0;

	ddr_writel(pu, DDRP_INNOPHY_PU_ODT_DQ7_0);
	ddr_writel(pu, DDRP_INNOPHY_PU_ODT_DQ15_8);
	ddr_writel(pd, DDRP_INNOPHY_PD_ODT_DQ7_0);
	ddr_writel(pd, DDRP_INNOPHY_PD_ODT_DQ15_8);

}
static void ddrp_set_dq_drv(struct phy_drvodt_config *drvodt)
{
	unsigned int pu = drvodt->phy_pu_drv_dq7_0;
        unsigned int pd = drvodt->phy_pd_drv_dq7_0;

        ddr_writel(pu, DDRP_INNOPHY_PU_DRV_DQ7_0);
	ddr_writel(pu, DDRP_INNOPHY_PU_DRV_DQ15_8);
	ddr_writel(pd, DDRP_INNOPHY_PD_DRV_DQ7_0);
	ddr_writel(pd, DDRP_INNOPHY_PD_DRV_DQ15_8);
}
static void ddrp_set_cmd_drv(struct phy_drvodt_config *drvodt)
{
	unsigned int pu = drvodt->phy_pu_drv_cmd;
        unsigned int pd = drvodt->phy_pd_drv_cmd;

        ddr_writel(pu, DDRP_INNOPHY_PU_DRV_CMD);
	ddr_writel(pd, DDRP_INNOPHY_PD_DRV_CMD);
}

static void ddrp_zq_calibration(int bypass, struct phy_drvodt_config *drvodt)
{
	unsigned tmp;
	unsigned int pu_drv = 0;
	unsigned int pd_drv = 0;
	unsigned int pu_odt = 0;
	unsigned int pd_odt = 0;
	ddrp_set_dq_odt(drvodt);
	ddrp_set_dq_drv(drvodt);
	ddrp_set_cmd_drv(drvodt);
#if 0
	serial_debug("DDRP_INNOPHY_PU_DRV_CMD:  %x\n", ddr_readl(DDRP_INNOPHY_PU_DRV_CMD));
	serial_debug("DDRP_INNOPHY_PU_DRV_DQ7_0: %x\n", ddr_readl(DDRP_INNOPHY_PU_DRV_DQ7_0));
	serial_debug("DDRP_INNOPHY_PU_DRV_DQ15_8: %x\n", ddr_readl(DDRP_INNOPHY_PU_DRV_DQ15_8));
	serial_debug("DDRP_INNOPHY_PD_DRV_DQ7_0: %x\n", ddr_readl(DDRP_INNOPHY_PD_DRV_DQ7_0));
	serial_debug("DDRP_INNOPHY_PD_DRV_DQ15_8: %x\n", ddr_readl(DDRP_INNOPHY_PU_DRV_DQ15_8));
	serial_debug("DDRP_INNOPHY_PU_ODT_DQ7_0: %x\n", ddr_readl(DDRP_INNOPHY_PU_ODT_DQ7_0));
	serial_debug("DDRP_INNOPHY_PU_ODT_DQ15_8: %x\n", ddr_readl(DDRP_INNOPHY_PU_ODT_DQ15_8));
	serial_debug("DDRP_INNOPHY_PD_ODT_DQ7_0: %x\n", ddr_readl(DDRP_INNOPHY_PD_ODT_DQ7_0));
	serial_debug("DDRP_INNOPHY_PD_ODT_DQ15_8: %x\n", ddr_readl(DDRP_INNOPHY_PD_ODT_DQ15_8));
#endif
}
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


/*
	ZQ校准，
	1. 用于校准drive 的 pull up/pull down校准.
	2. 用于校准rx ODT的 pull up/pull down校准.

	一般不使用自动校准，使用寄存器的值设置阻值.
*/
static void ddrp_zqcalibration(void)
{
	unsigned int timeout = 0xffffffff;
	unsigned int val = 0;


	//1. Clear ZQCal module to initial state.
	val = ddr_readl(DDR_PHY_OFFSET + (0x80 << 2));
	val |= (1 << 6);
	ddr_writel(val, DDR_PHY_OFFSET + (0x80 << 2));

	//2. enable ZQCal module
	val = ddr_readl(DDR_PHY_OFFSET + (0x80 << 2));
	val &= ~(1 << 6);
	val |= (1 << 5);
	ddr_writel(val, DDR_PHY_OFFSET + (0x80 << 2));


	while(!((ddr_readl(DDR_PHY_OFFSET + (0x88 << 2)) & 1) == 1) && --timeout) {
		udelay(1);
		unsigned int pd_resist = ddr_readl(DDR_PHY_OFFSET + (0x89 << 2));
		unsigned int pu_resist = ddr_readl(DDR_PHY_OFFSET + (0x8a << 2));
		unsigned int pd_odt = ddr_readl(DDR_PHY_OFFSET + (0x8b << 2));
		unsigned int pu_odt = ddr_readl(DDR_PHY_OFFSET + (0x8c << 2));


		serial_debug("--pd: %x, pu: %x, pd_odt: %x, pu_odt:%x\n", pd_resist, pu_resist, pd_odt, pu_odt);
	}


	//DQ ODT & Drive choose calib value.
	val = ddr_readl(DDR_PHY_OFFSET + (0xc7 << 2));
	val |= (0xf << 0);
	ddr_writel(val, DDR_PHY_OFFSET + (0xc7 << 2));


	//CMD choose calib value
	val = ddr_readl(DDR_PHY_OFFSET + (0xb4 << 2));
	val |= (0x1 << 0);
	ddr_writel(val, DDR_PHY_OFFSET + (0xb4 << 2));
}

void ddrp_cfg(struct ddr_reg_value *global_reg_value)
{
	unsigned int val;
	/*reset digital and analog core.*/

	val = ddr_readl(DDRP_INNOPHY_PHY_RST);
	ddr_writel(0, DDRP_INNOPHY_PHY_RST);
	mdelay(1);
	ddr_writel(val, DDRP_INNOPHY_PHY_RST);
	mdelay(1);

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


	if(1) {
		// bypass sdll.
		val = ddr_readl(DDR_PHY_OFFSET + (0x15 << 2));
		val &= ~0xff;
		val |= 0x18;
		ddr_writel(val, DDR_PHY_OFFSET + (0x15 << 2));

		val = ddr_readl(DDR_PHY_OFFSET + (0x14 << 2));
		val |= 0x10;
		ddr_writel(val, DDR_PHY_OFFSET + (0x14 << 2));
	}

	//ddrp_zqcalibration();

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
#ifdef CONFIG_DDR_DRVODT_DEBUG
	timeout = 0x500;
#endif
	unsigned int wait_cal_done = DDRP_CALIB_DONE_HDQCFA | DDRP_CALIB_DONE_LDQCFA;
	unsigned int al, ah;

        drvodt = &global_reg_value->phy_drvodt;
        deskew = &global_reg_value->phy_deskew;

        reg_val &= ~(DDRP_TRAINING_CTRL_DSCSE_BP);
	reg_val |= DDRP_TRAINING_CTRL_DSACE_START;
	ddr_writel(reg_val, DDRP_INNOPHY_TRAINING_CTRL);

	while(!((ddr_readl(DDRP_INNOPHY_CALIB_DONE) & 0x13) == 3) && --timeout) {

		udelay(1);
#ifndef CONFIG_DDR_DRVODT_DEBUG
		serial_debug("DDRP_INNOPHY_CALIB_DELAY_AL:%x\n", ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AL));
		serial_debug("DDRP_INNOPHY_CALIB_DELAY_AH:%x\n", ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AH));
		serial_debug("-----ddr_readl(DDRP_INNOPHY_CALIB_DONE): %x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE));
#endif
	}

	if(!timeout) {
#ifdef CONFIG_DDR_DRVODT_DEBUG
		fail_flag = -1;
		_machine_restart();
#endif
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
#ifndef CONFIG_DDR_DRVODT_DEBUG
	serial_debug("ddr calib finish\n");
#endif
}
#ifdef CONFIG_DDR_DRVODT_DEBUG
static int do_whole_chip_scan(void)
{
	int i = 0;
	unsigned int *p = 0x81000000;

//	serial_debug("doing whole chip w/r test!\n");

#define MAX_WR_TEST_SIZE	(4*1024*1024/4)
	for(i = 0; i < MAX_WR_TEST_SIZE; i++) {
		p[i] = &p[i];
		//p[i] = 0x01010101;
	}
	for(i = 0; i < MAX_WR_TEST_SIZE; i++) {
		p[i] = &p[i];
		//p[i] = 0x12345a5a;

		//p[i] = (i&0xff) | (i&0xff) << 8 | (i&0xff) << 16 | (i&0xff) << 24;//(i&0xff) | ((i << 8) & 0xff) | ((i << 16)& 0xff) | ((i << 24) & 0xff);

	}
	for(i = 0; i < MAX_WR_TEST_SIZE; i++) {

		if(p[i] != &p[i]) {
			serial_debug("---------------------------------------------------------err:%x:%x\n", &p[i], p[i]);
			return -1;
		}
	}
	pass_count ++;
	return 0;
}
void debug_date_eye(void) {
#ifdef CONFIG_BURNER
        cpm_outl(0,CPM_SLPC);
#else
	int i,j=0;
	pass_count = 0;
	int mem_count = 1;
	int restart_count_max = 3;

        unsigned char default_drv = 0x6;
        unsigned char default_odt = 0x5;

	slpc = cpm_inl(CPM_SLPC);
	drv_value = (slpc & 0xff);
	odt_value = (slpc >> 8) & 0xff;
	restart_count = (slpc >> 16) & 0xff;



	if(restart_count >= 0 && restart_count < restart_count_max) {
		if(drv_value <= 0x1f && odt_value <= 0x1f){

                        drvodt->phy_pu_drv_cmd    = drv_value;
                        drvodt->phy_pd_drv_cmd    = drv_value;
                        drvodt->phy_pu_drv_ck     = drv_value;
                        drvodt->phy_pd_drv_ck     = drv_value;
                        drvodt->phy_pu_drv_dq7_0  = drv_value;
                        drvodt->phy_pd_drv_dq7_0  = drv_value;
                        drvodt->phy_pu_drv_dq15_8 = drv_value;
                        drvodt->phy_pd_drv_dq15_8 = drv_value;

                        drvodt->phy_pu_odt_dq7_0  = odt_value;
                        drvodt->phy_pd_odt_dq7_0  = odt_value;
                        drvodt->phy_pu_odt_dq15_8 = odt_value;
                        drvodt->phy_pd_odt_dq15_8 = odt_value;

                        ddrp_zq_calibration(1, drvodt);
			ddrp_auto_calibration();

			for(i = 0;i < mem_count;i++){
				do_whole_chip_scan();
				if(pass_count != 1){
					pass_value = 0;
					restart_count = 0;
					serial_debug("drv value is %x odt value is %x pass_value is %x\n",drv_value,odt_value,pass_value);
					odt_value += 1;
					if(odt_value > 0x1f){
						drv_value += 1;
						odt_value = 0;
					}
					if(drv_value <= 0x1f) {
						slpc = drv_value | (odt_value << 8) | (restart_count << 16);
						cpm_outl(slpc,CPM_SLPC);
						_machine_restart();
					}else {
                                                /*inno phy default drv value is 0x6 odt value is 0x5*/
                                                drvodt->phy_pu_drv_cmd    = default_drv;
                                                drvodt->phy_pd_drv_cmd    = default_drv;
                                                drvodt->phy_pu_drv_ck     = default_drv;
                                                drvodt->phy_pd_drv_ck     = default_drv;
                                                drvodt->phy_pu_drv_dq7_0  = default_drv;
                                                drvodt->phy_pd_drv_dq7_0  = default_drv;
                                                drvodt->phy_pu_drv_dq15_8 = default_drv;
                                                drvodt->phy_pd_drv_dq15_8 = default_drv;

                                                drvodt->phy_pu_odt_dq7_0  = default_odt;
                                                drvodt->phy_pd_odt_dq7_0  = default_odt;
                                                drvodt->phy_pu_odt_dq15_8 = default_odt;
                                                drvodt->phy_pd_odt_dq15_8 = default_odt;

						ddrp_zq_calibration(1, drvodt);
						ddrp_auto_calibration();
						slpc = drv_value | (odt_value << 8) | (restart_count << 16);
						cpm_outl(slpc,CPM_SLPC);
						_machine_restart();
					}
					pass_count = 0;
				}
				if(drv_value <= 0x1f){
					drv_value = (slpc & 0xff);
					odt_value = (slpc >> 8) & 0xff;
					restart_count = (slpc >> 16) & 0xff;
					restart_count += 1;
					slpc = drv_value | (odt_value << 8) | (restart_count << 16);
					cpm_outl(slpc,CPM_SLPC);
					_machine_restart();
				}
			}
		}
	}else {
		odt_value = (slpc >> 8) & 0xff;
		pass_value = 1;
		serial_debug("drv value is %x odt value is %x pass_value is %x\n",drv_value,odt_value,pass_value);
		odt_value += 1;
		if(odt_value > 0x1f){
			drv_value += 1;
			odt_value = 0;
		}
		if(drv_value <= 0x1f) {
			restart_count = 0;
			slpc = drv_value | (odt_value << 8) | (restart_count << 16);
			cpm_outl(slpc,CPM_SLPC);
			_machine_restart();
		} else {
			/*inno phy default drv value is 0x6 odt value is 0x5*/
                        drvodt->phy_pu_drv_cmd    = default_drv;
                        drvodt->phy_pd_drv_cmd    = default_drv;
                        drvodt->phy_pu_drv_ck     = default_drv;
                        drvodt->phy_pd_drv_ck     = default_drv;
                        drvodt->phy_pu_drv_dq7_0  = default_drv;
                        drvodt->phy_pd_drv_dq7_0  = default_drv;
                        drvodt->phy_pu_drv_dq15_8 = default_drv;
                        drvodt->phy_pd_drv_dq15_8 = default_drv;

                        drvodt->phy_pu_odt_dq7_0  = default_odt;
                        drvodt->phy_pd_odt_dq7_0  = default_odt;
                        drvodt->phy_pu_odt_dq15_8 = default_odt;
                        drvodt->phy_pd_odt_dq15_8 = default_odt;

                        ddrp_zq_calibration(1, drvodt);
			ddrp_auto_calibration();
			drv_value = (slpc & 0xff);
			odt_value = (slpc >> 8) & 0xff;
		}
	}

#endif
}
#endif
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
	/* reg value 1 keeps dfi_rst_n low.*/
	ddr_writel(0x8 << 20, DDRC_CTRL);
	mdelay(5);
}

static void ddrc_post_init(void)
{
	unsigned reg = 0;
	debug("DDRC_STATUS: %x\n",ddr_readl(DDRC_STATUS));


	/* auto refresh 需要在 auto training之后开启.*/
	ddr_writel(global_reg_value->DDRC_REFCNT_VALUE, DDRC_REFCNT);

	/*控制寄存器应该只修改配置相关的内容，需要读后写操作.*/
	reg = ddr_readl(DDRC_CTRL);
	reg |= global_reg_value->DDRC_CTRL_VALUE & (1 << 2 | 1 << 11 | 0xf << 12);
	ddr_writel(reg, DDRC_CTRL);
}

static void ddrc_prev_init(void)
{
	dwc_debug("DDR Controller init\n");
	/*1. 先初始化DDR 控制器，主要一些timing 参数，在初始化ddr 颗粒的时候需要用到.*/
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

	/* 初始化时，关闭DDR自刷新功能. */
	ddr_writel(0 ,DDRC_AUTOSR_EN);
	ddr_writel(0, DDRC_AUTOSR_CNT);
	ddr_writel(0, DDRC_REFCNT);

	mem_remap();
}

void ddrc_dfi_init(void)
{
	u32 reg = 0;

	writel(1, DDR_APB_PHY_INIT); //start high
	writel(0, DDR_APB_PHY_INIT); //start low
	while(!(readl(DDR_APB_PHY_INIT) & (1<<1))); //polling dfi init comp
#ifndef CONFIG_FASTBOOT
#ifndef CONFIG_DDR_DRVODT_DEBUG
	serial_debug("ddr_inno_phy_init ..! 11:  %X\n", readl(DDR_APB_PHY_INIT));
#endif
#endif

	reg = ddr_readl(DDRC_CTRL);
	reg &= ~(1 << 23);
	ddr_writel(reg, DDRC_CTRL); //set reg value 0, keeps dfi_reset_n high

	udelay(500);	//DDR3 needs 500us delay before CKE High, LPDDR2/LPDDR3 needs 100ns.

	ddr_writel(global_reg_value->DDRC_CFG_VALUE, DDRC_CFG);

	ddr_writel(DDRC_CTRL_CKE, DDRC_CTRL); // set CKE to high

	if(current_ddr_type == DDR3) {
#define DDRC_LMR_MR(n)						\
	global_reg_value->DDRC_DLMR_VALUE | 0x1 | (2 << 3) |			\
		((global_reg_value->DDR_MR##n##_VALUE & 0xffff) << 12) |		\
		(((global_reg_value->DDR_MR##n##_VALUE >> 16) & 0x7) << 8)

	serial_debug("MR0 : 0x%x\n", DDRC_LMR_MR(0));
	serial_debug("MR1 : 0x%x\n", DDRC_LMR_MR(1));
	serial_debug("MR2 : 0x%x\n", DDRC_LMR_MR(2));
	serial_debug("MR3 : 0x%x\n", DDRC_LMR_MR(3));
	serial_debug("ZQCL : 0x%x\n", global_reg_value->DDRC_DLMR_VALUE | (0x4 << 3) | 0x1);

	/*从CKE High 到第一个MR命令需要延时tXPR,*/
	udelay(100);
	ddr_writel(DDRC_LMR_MR(0)/*0x1a30011*/, DDRC_LMR); //MR0
	udelay(100);
	ddr_writel(DDRC_LMR_MR(1)/*0x6111*/, DDRC_LMR); //MR1
	udelay(100);
	ddr_writel(DDRC_LMR_MR(2)/*0x8211*/, DDRC_LMR); //MR2
	udelay(100);
	ddr_writel(DDRC_LMR_MR(3)/*0x311*/, DDRC_LMR); //MR3
	udelay(100);
	ddr_writel(global_reg_value->DDRC_DLMR_VALUE | (0x4 << 3) | 0x1/*0x19*/, DDRC_LMR);
	udelay(100);

#undef DDRC_LMR_MR
	} else if(current_ddr_type == LPDDR2) {

		/*CKE High 到第一个命令之间需要延时200us.*/
		udelay(200);
		/*对于LPDDR2， 发送All bank precharge命令是可选的.*/
		ddr_writel(7 << 8 | 1 << 0, DDRC_LMR); //Send All bank precharge.
		mdelay(1);

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
#ifndef CONFIG_DDR_DRVODT_DEBUG
		serial_debug("mr1 = 0x%x\n", DDRC_LMR_MR(1));
		serial_debug("mr2 = 0x%x\n", DDRC_LMR_MR(2));
		serial_debug("mr3 = 0x%x\n", DDRC_LMR_MR(3));
		serial_debug("mr10 = 0x%x\n", DDRC_LMR_MR(10));
		serial_debug("mr63 = 0x%x\n", DDRC_LMR_MR(63));
#endif
#undef DDRC_LMR_MR
	} else {
		/*DDR2*/
#define DDRC_LMR_MR(n)										\
		global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |	\
			(((global_reg_value->DDR_MR##n##_VALUE  >> 13) & 0x3) << 8) |						\
			(((global_reg_value->DDR_MR##n##_VALUE ) & 0x1fff) << (12))

		udelay(1);
		/*CKE High 延时400ns, 发送All bank precharge*/
		ddr_writel(7 << 8 | 1 << 0, DDRC_LMR); //Send All bank precharge.

		if(global_reg_value->h.id == 0x65){
			/*LVDDR2_A3L28E40BGD*/
			ddr_writel(DDRC_LMR_MR(0), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MR(1) | 0x1000000, DDRC_LMR);
			mdelay(1);
			ddr_writel(0x80211, DDRC_LMR);
			mdelay(1);
			ddr_writel(0x1fff211, DDRC_LMR);
			mdelay(1);
		}
		else{
			ddr_writel(DDRC_LMR_MR(1), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MR(0), DDRC_LMR);
			mdelay(1);
		}
		/*MR 寄存器设置完成之后，需要All Bank Precharge.*/
		ddr_writel(7 << 8 | 1 << 0, DDRC_LMR); //Send All bank precharge.
		udelay(100);

		/*发送2个或者多个auto refresh 命令.*/
		ddr_writel(1 << 3 | 1 << 0, DDRC_LMR); // send auto refresh.
		udelay(100);
		ddr_writel(1 << 3 | 1 << 0, DDRC_LMR); // send auto refresh.
		udelay(100);

		serial_debug("mr0 = 0x%x\n", DDRC_LMR_MR(0));
		serial_debug("mr1 = 0x%x\n", DDRC_LMR_MR(1));
#undef DDRC_LMR_MR
	}
}

void ddrp_wl_training(void)
{

	if(current_ddr_type == DDR3) {
		//write level
		serial_debug("WL_MODE1 : 0x%x\n", global_reg_value->DDR_MR1_VALUE & 0xff);
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


int get_ddr_type(void)
{
	int type;

	type = global_reg_value->h.type;

	switch(global_reg_value->h.type){
#ifndef CONFIG_DDR_DRVODT_DEBUG
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
#endif
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
//						serial_debug("val1 : 0x%x   val : 0x%x\n", val1, val);
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
		serial_debug("calib bypass fail\n");
		return ;
	}


	m = m  / 2;
	c = calib_val[m].bypass.b.cyclesel;
	o = calib_val[m].bypass.b.ophsel;
	d = calib_val[m].bypass.b.dllsel;
	r = calib_val[m].rx_dll.b.rx_dll;

	serial_debug("m = %d   c = %d   o = %d   d = %d  r = %d\n", m, c, o, d, r);

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
		serial_debug("bypass :CALIB_AL: dllsel %x, ophsel %x, cyclesel %x\n",b_al.bypass.b.dllsel, b_al.bypass.b.ophsel, b_al.bypass.b.cyclesel);
		b_ah.bypass.u8 = ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AH);
		serial_debug("bypass:CAHIB_AH: dllsel %x, ophsel %x, cyclesel %x\n", b_ah.bypass.b.dllsel, b_ah.bypass.b.ophsel, b_ah.bypass.b.cyclesel);
		serial_debug("rxdll delay al : %x\n", ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AL));
		serial_debug("rxdll delay ah : %x\n", ddr_readl(DDRP_INNOPHY_RXDLL_DELAY_AH));

	}
}
#endif

#ifndef CONFIG_BURNER

__weak unsigned int check_socid(void)
{
        return -1;
}

int get_ddr_params_socid(void)
{
	int i;
	int found = 0;
	uint32_t ddrid = 0;
	uint32_t mask = ~(7 << 3);

	ddrid = check_socid();
	if ((int)ddrid < 0) {
		printf("Check socid return invalid ddr id\n");
		return -1;
	}

	for(i = 0; i < ARRAY_SIZE(supported_ddr_reg_values); i++) {
		global_reg_value = &supported_ddr_reg_values[i];
		if((ddrid & mask) == (global_reg_value->h.id & mask)) {
			found = 1;
			break;
		}
	}

	if(found == 0) {
		printf("Check socid not match to %x\n",ddrid);
		return -1;
	}

	return 0;
}

void get_ddr_params_normal(void)
{
	int found = 0;
	int size = 0;
	int i;
	unsigned int burned_ddr_id = *(volatile unsigned int *)(CONFIG_SPL_TEXT_BASE + 128);
	uint32_t mask = ~(7 << 3);

	if((burned_ddr_id & 0xffff) != (burned_ddr_id >> 16)) {
		printf("invalid burned ddr id\n");
	}

	burned_ddr_id &= 0xffff;

	for(i = 0; i < ARRAY_SIZE(supported_ddr_reg_values); i++) {
		global_reg_value = &supported_ddr_reg_values[i];
		if((burned_ddr_id & mask) == (global_reg_value->h.id & mask)) {
			found = 1;
			break;
		}
	}

	if(found == 0) {
		printf("No match to %x\n",burned_ddr_id);
	}

}
#else
void get_ddr_params_burner(void)
{
	/* keep ddr_reg_value inc ddr_innophy.h
	 * with ddr_registers the same
	 * */
	global_reg_value = g_ddr_param;
        memset(&global_reg_value->phy_drvodt, 0, sizeof(struct phy_drvodt_config));
        memset(&global_reg_value->phy_deskew, 0, sizeof(struct phy_deskew_config));
}
#endif

void get_ddr_params(void)
{
#ifndef CONFIG_BURNER
	if(ARRAY_SIZE(supported_ddr_reg_values) == 1)
		global_reg_value = &supported_ddr_reg_values[0];
	else if (get_ddr_params_socid() < 0)
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

	clk_set_rate(DDR, global_reg_value->h.freq);
	reset_dll();
	rate = clk_get_rate(DDR);
	if(rate != global_reg_value->h.freq)
		dwc_debug("sdram set ddr freq failed\n");

	reset_controller();

#ifdef CONFIG_DDR_AUTO_SELF_REFRESH
	ddr_writel(0x0 ,DDRC_AUTOSR_EN);
#endif

	ddrp_cfg(global_reg_value);
	ddrp_pll_init();

	ddrc_prev_init();

	ddrc_dfi_init();

        /* DDR Controller init*/
	dwc_debug("DDR PHY init OK\n");

#ifdef CONFIG_DDRP_SOFTWARE_TRAINING

	/* Soft training, 依赖 ddr 控制器 dfi接口，training 之前需要先初始化好ddr 控制器相关timing.*/
	ddrc_post_init();
	ddrp_software_calibration();
#else
	/*
	 * auto training, 是phy本身的行为，与ddr控制器,dfi接口无关，并且必须保持dfi静默。
	 * 在进行auto training 之前，需要关闭ddr 控制器的auto refresh, auto self refresh等会自动发命令的功能.
	*/
#ifdef CONFIG_DDR_DRVODT_DEBUG
	debug_date_eye();
#endif
	ddrp_auto_calibration();
	ddrc_post_init();
#endif

#ifdef CONFIG_DDR_AUTO_SELF_REFRESH
	ddr_writel(CONFIG_DDR_AUTO_SELF_REFRESH_CNT ,DDRC_AUTOSR_CNT);
	ddr_writel(0x1 ,DDRC_AUTOSR_EN);
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
