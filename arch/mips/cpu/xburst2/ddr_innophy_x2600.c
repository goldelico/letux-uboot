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
#include <asm/io.h>
#include <asm/arch/clk.h>


#define DEBUG_TX_RX_TRAINING	0
#define OPTIMIZE_TX_RX_TRAINING 0	// save training time.

/*#define CONFIG_DWC_DEBUG 0*/
#define ddr_hang() do{						\
		debug("%s %d\n",__FUNCTION__,__LINE__);	\
		hang();						\
	}while(0)

DECLARE_GLOBAL_DATA_PTR;
extern struct ddr_reg_value *global_reg_value __attribute__ ((section(".data")));

static struct phy_drvodt_config *drvodt = NULL;
static struct phy_deskew_config *deskew = NULL;

static char tx_pass_cmd_skew[0x3f];
static char tx_rx_pass_dqs_skew[0x3f];
static char tx_rx_left_dq[0x3f];
static char tx_rx_right_dq[0x3f];
#ifdef CONFIG_DDR_DRVODT_DEBUG
unsigned int pass_count = 0;
#endif

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

static void ddrp_set_ck_drv(struct phy_drvodt_config *drvodt)
{
	unsigned int pu = drvodt->phy_pu_drv_ck;
        unsigned int pd = drvodt->phy_pd_drv_ck;

        ddr_writel(pu, DDRP_INNOPHY_PU_DRV_CK);
	ddr_writel(pd, DDRP_INNOPHY_PD_DRV_CK);
}
/*

	该函数的作用主要用于校准 drv 和 odt 的pull up/down电阻.

	对于drv pull up/down电阻使用的是40欧姆标定.
	对于odt pull up/down电阻使用的是160欧姆标定.

	实际设置的 ODT阻值与寄存器里面有偏差:

	相关寄存器:

	setting =
	DDRP_INNOPHY_PD_DRV_DQ7_0
	DDRP_INNOPHY_PU_DRV_DQ7_0
	DDRP_INNOPHY_PD_DRV_DQ15_8
	DDRP_INNOPHY_PU_DRV_DQ15_8

	real = setting * (DDRP_INNOPHY_ZQ_CALIB_PU_ODT / 0x7)

	0x7: 对应的是160欧姆的寄存器值.


	DDRP_INNOPHY_PD_ODT_DQ7_0
	DDRP_INNOPHY_PU_ODT_DQ7_0
	DDRP_INNOPHY_PD_ODT_DQ15_8
	DDRP_INNOPHY_PU_ODT_DQ15_8

	real = setting * (DDRP_INNOPHY_ZQ_CALIB_PD_DRV  / 0x16)
	或
	real = setting * (DDRP_INNOPHY_ZQ_CALIB_PU_DRV  / 0x16)
	0x16: 是对应41.4欧姆的寄存器值.


	DDRP_INNOPHY_PU_DRV_CMD:  0000000e
	DDRP_INNOPHY_PU_DRV_DQ7_0: 00000014
	DDRP_INNOPHY_PU_DRV_DQ15_8: 00000014
	DDRP_INNOPHY_PD_DRV_DQ7_0: 00000014
	DDRP_INNOPHY_PD_DRV_DQ15_8: 00000014
	DDRP_INNOPHY_PU_ODT_DQ7_0: 00000005
	DDRP_INNOPHY_PU_ODT_DQ15_8: 00000005
	DDRP_INNOPHY_PD_ODT_DQ7_0: 00000005
	DDRP_INNOPHY_PD_ODT_DQ15_8: 00000005

*/

static void ddrp_zq_calibration(int bypass, struct phy_drvodt_config *drvodt)
{
	unsigned tmp;
	unsigned int pu_drv = 0;
	unsigned int pd_drv = 0;
	unsigned int pu_odt = 0;
	unsigned int pd_odt = 0;

	debug("DDRP_INNOPHY_PU_DRV_CMD:  %x\n", ddr_readl(DDRP_INNOPHY_PU_DRV_CMD));
	debug("DDRP_INNOPHY_PU_DRV_DQ7_0: %x\n", ddr_readl(DDRP_INNOPHY_PU_DRV_DQ7_0));
	debug("DDRP_INNOPHY_PU_DRV_DQ15_8: %x\n", ddr_readl(DDRP_INNOPHY_PU_DRV_DQ15_8));
	debug("DDRP_INNOPHY_PD_DRV_DQ7_0: %x\n", ddr_readl(DDRP_INNOPHY_PD_DRV_DQ7_0));
	debug("DDRP_INNOPHY_PD_DRV_DQ15_8: %x\n", ddr_readl(DDRP_INNOPHY_PU_DRV_DQ15_8));
	debug("DDRP_INNOPHY_PU_ODT_DQ7_0: %x\n", ddr_readl(DDRP_INNOPHY_PU_ODT_DQ7_0));
	debug("DDRP_INNOPHY_PU_ODT_DQ15_8: %x\n", ddr_readl(DDRP_INNOPHY_PU_ODT_DQ15_8));
	debug("DDRP_INNOPHY_PD_ODT_DQ7_0: %x\n", ddr_readl(DDRP_INNOPHY_PD_ODT_DQ7_0));
	debug("DDRP_INNOPHY_PD_ODT_DQ15_8: %x\n", ddr_readl(DDRP_INNOPHY_PD_ODT_DQ15_8));



	if(!bypass) {
		ddr_writel(0, DDRP_INNOPHY_ZQ_CALIB_EN);
		ddr_writel(1 << 5, DDRP_INNOPHY_ZQ_CALIB_EN);
		do{
			tmp = ddr_readl(DDRP_INNOPHY_ZQ_CALIB_DONE);
		}while(tmp != 1);

		ddr_writel(0, DDRP_INNOPHY_ZQ_CALIB_EN);
		pd_drv = ddr_readl(DDRP_INNOPHY_ZQ_CALIB_PD_DRV_6C);
		pu_drv = ddr_readl(DDRP_INNOPHY_ZQ_CALIB_PU_DRV_6D);
		pd_odt = ddr_readl(DDRP_INNOPHY_ZQ_CALIB_PD_ODT_6E);
		pu_odt = ddr_readl(DDRP_INNOPHY_ZQ_CALIB_PU_ODT_6F);

		// Choose ZQCAL value?
		ddr_writel(3 << 4, DDRP_INNOPHY_ZQ_CALIB_AL);
		ddr_writel(3 << 4, DDRP_INNOPHY_ZQ_CALIB_AH);

		tmp = ddr_readl(DDRP_INNOPHY_ZQ_CALIB_CMD);
		tmp |= 1 << 7;
		ddr_writel(tmp, DDRP_INNOPHY_ZQ_CALIB_CMD);	//Choose CMD pull up/down resistance. choose ZQCALIB value.

	} else {

		// Register value, 怎么补偿的？
		// 默认pu/pd 配置为相同值.

		ddrp_set_dq_odt(drvodt);
		ddrp_set_dq_drv(drvodt);
		ddrp_set_cmd_drv(drvodt);
		ddrp_set_ck_drv(drvodt);
	}


	debug("DRP_INNOPHY_ZQ_CALIB_DONE : %x\n", ddr_readl(DDRP_INNOPHY_ZQ_CALIB_DONE));
	debug("DRP_INNOPHY_ZQ_CALIB_AL: %x\n", ddr_readl(DDRP_INNOPHY_ZQ_CALIB_AL));
	debug("DRP_INNOPHY_ZQ_CALIB_AH: %x\n", ddr_readl(DDRP_INNOPHY_ZQ_CALIB_AH));
	debug("DRP_INNOPHY_ZQ_CALIB_PD_DRV_6C: %x\n", ddr_readl(DDRP_INNOPHY_ZQ_CALIB_PD_DRV_6C));
	debug("DRP_INNOPHY_ZQ_CALIB_PU_DRV_6D: %x\n", ddr_readl(DDRP_INNOPHY_ZQ_CALIB_PU_DRV_6D));
	debug("DRP_INNOPHY_ZQ_CALIB_PD_ODT_6E: %x\n", ddr_readl(DDRP_INNOPHY_ZQ_CALIB_PD_ODT_6E));
	debug("DRP_INNOPHY_ZQ_CALIB_PU_ODT_6F: %x\n", ddr_readl(DDRP_INNOPHY_ZQ_CALIB_PU_ODT_6F));
	debug("DRP_INNOPHY_ZQ_CALIB_CMD: %x\n", ddr_readl(DDRP_INNOPHY_ZQ_CALIB_CMD));

	debug("DDRP_INNOPHY_PU_DRV_CMD:  %x\n", ddr_readl(DDRP_INNOPHY_PU_DRV_CMD));
	debug("DDRP_INNOPHY_PU_DRV_DQ7_0: %x\n", ddr_readl(DDRP_INNOPHY_PU_DRV_DQ7_0));
	debug("DDRP_INNOPHY_PU_DRV_DQ15_8: %x\n", ddr_readl(DDRP_INNOPHY_PU_DRV_DQ15_8));
	debug("DDRP_INNOPHY_PD_DRV_DQ7_0: %x\n", ddr_readl(DDRP_INNOPHY_PD_DRV_DQ7_0));
	debug("DDRP_INNOPHY_PD_DRV_DQ15_8: %x\n", ddr_readl(DDRP_INNOPHY_PU_DRV_DQ15_8));
	debug("DDRP_INNOPHY_PU_ODT_DQ7_0: %x\n", ddr_readl(DDRP_INNOPHY_PU_ODT_DQ7_0));
	debug("DDRP_INNOPHY_PU_ODT_DQ15_8: %x\n", ddr_readl(DDRP_INNOPHY_PU_ODT_DQ15_8));
	debug("DDRP_INNOPHY_PD_ODT_DQ7_0: %x\n", ddr_readl(DDRP_INNOPHY_PD_ODT_DQ7_0));
	debug("DDRP_INNOPHY_PD_ODT_DQ15_8: %x\n", ddr_readl(DDRP_INNOPHY_PD_ODT_DQ15_8));
}

void ddrp_wl_calibration(void)
{
	unsigned tmp = 0;
	int wl_count = 0;

	unsigned int mr1 = global_reg_value->DDR_MR1_VALUE;


	mr1 = ((mr1 >> 16) & 0x7) << 13 | (mr1 & 0x1ffe); // 高三位BA2,BA1,BA0 低位地址线, 确保DLL ON， 否则后面会卡死.

	/*
		说明:
		这里做write leveling ，
		需要确保MR1寄存器里面的ZQ的设置和正常读写的一致，否则即使writeleveling 完成，
		但是实际上用的是不同的ZQ 配置，会影响后面系统的稳定性.!
	*/

	/*
		WL_MODE1:
		[7:0]: load mode [7:0];
	*/
	tmp = mr1 & 0x7f; // keep bit7 0. others mr1.
	ddr_writel(tmp, DDRP_INNOPHY_WL_MODE1);

	/*
		WL_MODE2:
		[7:6]: load mode select[1:0], 00:MR0, 01:MR1
		[5:0]: load mode [13:8]
	*/
	tmp = 0;
	tmp = 1 << 6 | ((mr1 >> 8) & 0x1f);	// 去掉高3位bank信息.取低5位.

	ddr_writel(tmp, DDRP_INNOPHY_WL_MODE2);


retry_wl:
	wl_count ++;
	tmp = 2 << 6 | 1 << 2;
	ddr_writel(tmp, DDRP_INNOPHY_TRAINING_CTRL);

	do{
		tmp = ddr_readl(DDRP_INNOPHY_WL_DONE);
	}while(tmp != 0x3);

#if 1
//	ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	ddr_writel(0, DDRP_INNOPHY_TRAINING_CTRL);
#endif

#if 0
#define MAX_WL_RETRY_CNT	3
	if(((ddr_readl(DDRP_INNOPHY_WL_L) == 0x3f) || (ddr_readl(DDRP_INNOPHY_WL_H) == 0x3f)) && wl_count <= MAX_WL_RETRY_CNT) {
		/* 不一定是错，这里进行 retry，只是在调试阶段debug使用.*/
		serial_debug("***ERROR WL FOUND retry <%d/%d>**** ", wl_count, MAX_WL_RETRY_CNT);
		serial_debug("DDRP_INNOPHY_WL_L: 0x%x ", ddr_readl(DDRP_INNOPHY_WL_L));
		serial_debug("DDRP_INNOPHY_WL_H: 0x%x\n", ddr_readl(DDRP_INNOPHY_WL_H));
		goto retry_wl;
	}
#endif

#if 0
	if(ddr_readl(DDRP_INNOPHY_WL_L) == 0x3f) {
		serial_debug("******** ");
	}
		serial_debug("DDRP_INNOPHY_WL_L: 0x%x ", ddr_readl(DDRP_INNOPHY_WL_L));
		serial_debug("DDRP_INNOPHY_WL_H: 0x%x\n", ddr_readl(DDRP_INNOPHY_WL_H));
#endif

}

int tx_re_training(unsigned int cmd_skew)
{
	unsigned tmp, wl_l, wl_h;

	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A0);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A2);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A3);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A4);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A5);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A6);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A7);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A8);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A9);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A10);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A11);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A12);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A13);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A14);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A15);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_WEB);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CASB);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_BA0);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_BA1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_BA2);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_BG1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CKE);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CK0);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CKB0);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CSB0);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_ODT0);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_RESETN);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_RASB);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CSB1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_ODT1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CKE1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CK1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CKB1);

	tmp = ddr_readl(DDRC_CGUC1);
	tmp &= ~(0xf << 4);
	ddr_writel(tmp, DDRC_CGUC1);

	ddr_writel(7 << 9 | 1 << 0, DDRC_LMR);
	udelay(200);

	tmp = 0 << 7 | 6;
	ddr_writel(tmp, DDRP_INNOPHY_WL_MODE1);
	tmp = 0x40;
	ddr_writel(tmp, DDRP_INNOPHY_WL_MODE2);

	tmp = ddr_readl(DDRC_CGUC1);
	tmp |= 0xf << 4;
	ddr_writel(tmp, DDRC_CGUC1);

	ddr_writel(2 << 6 | 1 << 2, DDRP_INNOPHY_TRAINING_CTRL);

	do{
		tmp = ddr_readl(DDRP_INNOPHY_WL_DONE);
	}while(tmp != 3);

	ddr_readl( DDRP_INNOPHY_TRAINING_CTRL);
	ddr_writel(0, DDRP_INNOPHY_TRAINING_CTRL);

	wl_l = ddr_readl(DDRP_INNOPHY_WL_L);
	wl_h = ddr_readl(DDRP_INNOPHY_WL_H);
	serial_debug("write leveling low : 0x%x\n", ddr_readl(DDRP_INNOPHY_WL_L));
	serial_debug("write leveling high: 0x%x\n", ddr_readl(DDRP_INNOPHY_WL_H));

	if((wl_h == 0x3f) && (wl_l == 0x3f))
		return -1;
	else
		return 0;
}


static void tx_soft_training_set_pb_ck_skew(unsigned int ck_skew)
{
	ddr_writel(ck_skew, DDRP_INNOPHY_PBDS_CK0);
	ddr_writel(ck_skew, DDRP_INNOPHY_PBDS_CKB0);
}

static void tx_soft_training_set_pb_cmd_skew(unsigned int cmd_skew)
{
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A0);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A2);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A3);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A4);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A5);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A6);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A7);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A8);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A9);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A10);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A11);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A12);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A13);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A14);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_A15);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_WEB);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CASB);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_BA0);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_BA1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_BA2);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_BG1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CKE);
#if 0
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CK0);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CKB0);
#endif
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CSB0);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_ODT0);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_RESETN);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_RASB);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CSB1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_ODT1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CKE1);
#if 0
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CK1);
	ddr_writel(cmd_skew, DDRP_INNOPHY_PBDS_CKB1);
#endif
}

static void tx_soft_training_set_pb_dq_skew(unsigned int dq_skew)
{
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ0);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ1);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ2);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ3);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ4);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ5);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ6);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ7);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ8);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ9);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ10);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ11);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ12);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ13);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ14);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQ15);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DM0);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DM1);
#if 0
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQS0);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQSB0);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQS1);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_TX_DQSB1);
#endif
}

static void tx_soft_training_set_pb_dqs0_skew(unsigned int dqs_skew)
{

	ddr_writel(dqs_skew, DDRP_INNOPHY_PBDS_TX_DQS0);
	ddr_writel(dqs_skew, DDRP_INNOPHY_PBDS_TX_DQSB0);
}

static void tx_soft_training_set_pb_dqs1_skew(unsigned int dqs_skew)
{

	ddr_writel(dqs_skew, DDRP_INNOPHY_PBDS_TX_DQS1);
	ddr_writel(dqs_skew, DDRP_INNOPHY_PBDS_TX_DQSB1);
}

static void ddrp_wl_bypass()
{
	unsigned int reg;
	reg = ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	reg |= (DDRP_TRAINING_CTRL_WL_BP);
	reg &= ~(DDRP_TRAINING_CTRL_WL_START);
	ddr_writel(reg, DDRP_INNOPHY_TRAINING_CTRL);
}


static int check_wr_data(int start)
{
	//unsigned int *p = 0xa5a5a5a0;
	unsigned int *p = 0xa0000000;
	int i = 0;
	unsigned int pattern = 0;



	for(i = 0; i < 16; i++) {
		p[i] = 0xffff0000; //pattern;
	}
	for(i = 0; i < 16; i++) {
		if(p[i] != 0xffff0000) {
			return -1;
		}
	}

	return 0;
}

void tx_soft_training()
{
	int cmd_skew = 0;
	int pass = -1;
	int i = 0;
	int j = 0;
	int left_dq = 0;
	int right_dq = 0;
	int max_pass_count = 0;
	char selected_dq_skew = 0;
	char selected_cmd_skew = 0;

	int left_cmd_skew = -1;
	int right_cmd_skew = -1;

	int first_left_dq = -1;

	int pass_index = 0;


#if DEBUG_TX_RX_TRAINING
	serial_debug("=============tx pb deskew from 0 to 63 ==============\n");
#endif
	for(cmd_skew = 0;  cmd_skew <= 0x3F; cmd_skew++) {
//	for(cmd_skew = 0x3f;  cmd_skew >= 0; cmd_skew--) {
		/* 1. 每次修改 cmd_skew之后，需要做wl leveling使 cmd 和DQS 对齐.*/
		tx_soft_training_set_pb_ck_skew(cmd_skew);
		tx_soft_training_set_pb_cmd_skew(cmd_skew);

		/* 2. 做一次 wl ，对齐 DQS 和 CMD */
		ddrp_wl_calibration();
		/*3. 将 wl 设置为bypass，使 tx dqs skew 可以使用 bypass 的值. */
		ddrp_wl_bypass();

		/*4. 将 wl 的结果，写入 tx dqs skew. */
		unsigned int wl_l = ddr_readl(DDRP_INNOPHY_WL_L);
		tx_soft_training_set_pb_dqs0_skew(wl_l);
		unsigned int wl_h = ddr_readl(DDRP_INNOPHY_WL_H);
		tx_soft_training_set_pb_dqs1_skew(wl_h);

#if DEBUG_TX_RX_TRAINING
		serial_debug("WL_L(DQS0):%x, WL_H(DQS1):%x ",wl_l, wl_h);
#endif

#if DEBUG_TX_RX_TRAINING
		if(cmd_skew < 10) {
			serial_debug("cmd_skew:0%d ", cmd_skew);
		} else {
			serial_debug("cmd_skew:%d ", cmd_skew);
		}
#endif

		pass = -1;
		left_dq = 0;
		right_dq = 0;


		for(i = 0; i < 0x3f; i++) {
			tx_soft_training_set_pb_dq_skew(i);

			*(volatile unsigned int *)0xb0000000;

			int check_wr = check_wr_data(i);
			if(!check_wr && (pass == -1)) {
				left_dq = i;
				pass = 1;

			} else if(check_wr && (pass == 1)) {
				right_dq = i;
				pass = 0;
			}

#if DEBUG_TX_RX_TRAINING
			if(!check_wr) {
				if(pass) {
					putchar('1');
				} else {
					putchar('x'); // 读写测试通过，但是没有被选中，可能处于边缘，不稳定.
				}
			} else {
				putchar('0');
			}
#endif
		}

		int count = right_dq - left_dq;
		if(count >= 5 && left_cmd_skew == -1) {
			left_cmd_skew = cmd_skew;
			right_cmd_skew = cmd_skew;

			max_pass_count = count;
			first_left_dq = left_dq;

#if DEBUG_TX_RX_TRAINING
			serial_debug(" -> * ");
#endif
		}
#if 0
else if(count == 0){
			//break; // TEST.
		}
#endif

		if((first_left_dq == left_dq) && (count >= max_pass_count)) {

#if DEBUG_TX_RX_TRAINING
			serial_debug(" * <- ");
#endif
			right_cmd_skew = cmd_skew;
			max_pass_count = count;

			tx_pass_cmd_skew[pass_index]	= cmd_skew;
			tx_rx_left_dq[pass_index] 	= left_dq;
			tx_rx_right_dq[pass_index] 	= right_dq;
			pass_index++;
		}

#if DEBUG_TX_RX_TRAINING
		putchar('\n');
#endif

	}

	left_dq = tx_rx_left_dq[pass_index / 2];
	right_dq = tx_rx_right_dq[pass_index / 2];
	max_pass_count = tx_rx_right_dq[pass_index - 1] - tx_rx_left_dq[pass_index - 1];

	selected_dq_skew = left_dq + (right_dq - left_dq) - max_pass_count / 2;
	selected_cmd_skew = tx_pass_cmd_skew[pass_index / 2];

	serial_debug("selected tx cmd: %d\n", tx_pass_cmd_skew[pass_index / 2]);
	serial_debug("selected tx left dq: %d right dq: %d\n", tx_rx_left_dq[pass_index / 2], tx_rx_right_dq[pass_index / 2]);
	serial_debug("max pass tx dq: %d\n", tx_rx_right_dq[pass_index - 1] - tx_rx_left_dq[pass_index - 1]);
	serial_debug("selected dq: %d\n", selected_dq_skew);


	if(max_pass_count == 0) {
		serial_debug("tx deskew tuning error, no skew found!\n");
	} else {

#define RIGHT_CMD_SKEW_ADJUST	3	// 去掉右侧可能出现的假PASS。

#if 0
		right_cmd_skew -= right_cmd_skew > RIGHT_CMD_SKEW_ADJUST ? RIGHT_CMD_SKEW_ADJUST : 0;
		selected_cmd_skew = left_cmd_skew + (right_cmd_skew - left_cmd_skew) / 2;
#endif

		//1. 找到合适的cmd_skew， 重新设置到.
		tx_soft_training_set_pb_ck_skew(selected_cmd_skew);
		tx_soft_training_set_pb_cmd_skew(selected_cmd_skew);

		//2. 重新做一次 wl_calibration，使DQS 和 CMD 对齐.
		ddrp_wl_calibration();
		//3. 设置wl 为bypass 模式，此时PHY 会使用 wl result 作为DQS tx perbit skew value.
		//ddr_writel(reg, DDRP_INNOPHY_TRAINING_CTRL);
		ddrp_wl_bypass();

		unsigned int wl_l = ddr_readl(DDRP_INNOPHY_WL_L);
		tx_soft_training_set_pb_dqs0_skew(wl_l);
		unsigned int wl_h = ddr_readl(DDRP_INNOPHY_WL_H);
		tx_soft_training_set_pb_dqs1_skew(wl_h);


		serial_debug("================== Final TX deskew Result ===============\n");
		serial_debug("DDRP_INNOPHY_TRAINING_CTRL: %x\n", ddr_readl(DDRP_INNOPHY_TRAINING_CTRL));
		serial_debug("TX_DQS0 deskew: %x ",ddr_readl(DDRP_INNOPHY_PBDS_TX_DQS0));
		serial_debug("TX_DQSB0 deskew: %x \n", ddr_readl(DDRP_INNOPHY_PBDS_TX_DQSB0));
		serial_debug("TX_DQS1 deskew: %x ", ddr_readl(DDRP_INNOPHY_PBDS_TX_DQS1));
		serial_debug("TX_DQSB1 deskew: %x\n", ddr_readl(DDRP_INNOPHY_PBDS_TX_DQSB1));

		serial_debug("DDRP_INNOPHY_WL_L: 0x%x ", ddr_readl(DDRP_INNOPHY_WL_L));
		serial_debug("DDRP_INNOPHY_WL_H: 0x%x\n", ddr_readl(DDRP_INNOPHY_WL_H));

		// 4. 使用上面扫描记录中的最佳dq_skew.
		tx_soft_training_set_pb_dq_skew(selected_dq_skew);
		*(volatile unsigned int *)0xb0000000;

#if 0
		//4. 重新校准一次 perbit DQ skew，找到左右窗口的中心点作为最终值.
		pass = -1;
		left_dq = 0;
		right_dq = 0;

		for(i = 0; i <= 0x3f; i++) {
			tx_soft_training_set_pb_dq_skew(i);
			*(volatile unsigned int *)0xb0000000;
			int check_wr = check_wr_data(i);
			if(!check_wr && (pass == -1)) {
				left_dq = i;
				pass = 1;

			} else if(check_wr && (pass == 1)) {
				right_dq = i;
				pass = 0;

			}

#if DEBUG_TX_RX_TRAINING
			if(!check_wr) {
				putchar('1');
			} else {
				putchar('0');
			}
#endif
		}

#if DEBUG_TX_RX_TRAINING
		putchar('\n');
#endif

		selected_dq_skew = left_dq + (right_dq - left_dq) / 2;

		tx_soft_training_set_pb_dq_skew(selected_dq_skew);
		*(volatile unsigned int *)0xb0000000;

		serial_debug("tx deskew tuning done, %d found, tuned tx dq_skew: %d, tx_cmd_skew: %d\n", max_pass_count, selected_dq_skew, selected_cmd_skew);

#endif
	}


}



void tx_soft_training1()
{
	unsigned int addr = 0xa1000000, val, reg;
	unsigned int dq_skew[64] = {0};
	unsigned int i, j, n, m = 0, finish = 0, de_skew;
	unsigned int cmd_skew = 32;
	unsigned int cnt = 256;
	unsigned int cmd_test_all = 0;
	volatile unsigned int val1 = 0;
	int ret;

	reg = ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	reg |= (DDRP_TRAINING_CTRL_WL_BP);
	reg &= ~(DDRP_TRAINING_CTRL_WL_START);
	ddr_writel(reg, DDRP_INNOPHY_TRAINING_CTRL);


	do{
		for(i = 0; i < 64; i++){
			/* serial_debug("i = %d-----\n", i); */
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ0);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ1);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ2);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ3);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ4);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ5);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ6);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ7);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ8);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ9);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ10);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ11);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ12);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ13);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ14);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DQ15);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DM0);
			ddr_writel(i, DDRP_INNOPHY_PBDS_TX_DM1);

			for (j = 0; j < cnt; j++) {
				volatile unsigned int val1;
				val = 0x12345678;
				*(volatile unsigned int *)(addr + j * 4) = val;
				val1 = *(volatile unsigned int *)(addr + j * 4);

				if (val1 != val) {
					/*serial_debug("%s  val = 0x%x val1 = 0x%x addr = 0x%x\n", __func__, val, val1, addr + j * 4);*/
					break;
				}
			}

			if (j == cnt) {
				dq_skew[m] = i;
				m++;
			}

		}

		if ((m == 1) && (dq_skew[0] == 0)) {
			cmd_skew++;
			m = 0;
			tx_re_training(cmd_skew);
			if ((cmd_skew > 64) || (cmd_skew < 0))
				finish = 1;
			else
				finish = 0;
		} else if ((m == 1) && (dq_skew[0] == 63)) {
			cmd_skew--;
			m = 0;
			tx_re_training(cmd_skew);
			if ((cmd_skew > 64) || (cmd_skew < 0))
				finish = 1;
			else
				finish = 0;
		} else if (m == 0) {
			serial_debug("%s no_data_found cmd_skew = %d\n", __func__, cmd_skew);
			if (cmd_test_all == 0) {
				cmd_skew = 0;
				cmd_test_all = 1;
			}
			if (cmd_test_all == 1) {
				cmd_skew++;
			}

			m = 0;
			do  {
				ret = tx_re_training(cmd_skew);
				if (ret)
					cmd_skew++;
			} while (ret);
			finish = 0;
		} else {
			de_skew = dq_skew[m / 2];
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ0);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ1);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ2);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ3);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ4);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ5);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ6);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ7);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ8);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ9);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ10);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ11);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ12);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ13);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ14);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DQ15);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DM0);
			ddr_writel(de_skew, DDRP_INNOPHY_PBDS_TX_DM1);
         finish = 1;
      }

   } while(!finish);

}

static void rx_soft_training_set_pb_dqs_skew(struct phy_deskew_config *deskew)
{
        unsigned int dqs0_skew = deskew->phy_deskew_rx_dqs0;
        unsigned int dqs1_skew = deskew->phy_deskew_rx_dqs1;

	ddr_writel(dqs0_skew, DDRP_INNOPHY_PBDS_RX_DQS0);
	ddr_writel(dqs0_skew, DDRP_INNOPHY_PBDS_RX_DQSB0);
	ddr_writel(dqs1_skew, DDRP_INNOPHY_PBDS_RX_DQS1);
	ddr_writel(dqs1_skew, DDRP_INNOPHY_PBDS_RX_DQSB1);

}

static void rx_soft_training_set_pb_dq_skew(struct phy_deskew_config *deskew)
{
        unsigned int dq_skew = deskew->phy_deskew_rx_dq7_0;

	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ0);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ1);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ2);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ3);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ4);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ5);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ6);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ7);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ8);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ9);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ10);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ11);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ12);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ13);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ14);
	ddr_writel(dq_skew, DDRP_INNOPHY_PBDS_RX_DQ15);
}


static int check_pb_dq_pattern(int dq)
{
	unsigned int *p = 0xa0000000;

	int i = 0;
	unsigned int val = 0;
	unsigned int short data_low = 0;
	unsigned int short data_high = 0;
	// 1 个burst 的数据.

	//p[i] == 0xffff0000 is pass.

	for(i = 0; i < 16; i++) {
		//val = p[i];

		data_low = p[i] & 0xffff;
		data_high = (p[i] >> 16) & 0xffff;

		if(!!(data_low & (1 << dq)) == 1) {
			return -1;
		}

		if(!!(data_high & (1 << dq)) == 0) {
			return -1;
		}
	}

#if 0
	serial_debug("dq%d: %x\n", dq, p[0]);

	serial_debug("p[0] & (1 << dq): %d\n", p[0] & (1 << dq));
#endif

	return 0;

}

static void rx_soft_training_pb_dqx(unsigned int reg_dqx, int dq)
{
	/*调整每个DQ，使DQ 和DQS 处于最佳采样点. 窗口最大,
	这种tuning效率太低. 但是带来的稳定性相对较高.*/

	int i = 0;

	int left_dq = 0;
	int right_dq = 0;
	int pass = 0;
	int selected_dq_skew = 0;
	int count = 0;


#if DEBUG_TX_RX_TRAINING
	if(dq < 10) {
		serial_debug("DQ: 0%d ", dq);
	} else {
		serial_debug("DQ: %d ", dq);
	}
#endif

	for(i = 0; i < 0x3f; i++) {

		/*1. 调整pb_dq的值.*/
		ddr_writel(i, reg_dqx);

		int check_ret = check_pb_dq_pattern(dq);

		if(!check_ret && (pass == 0)) {
			left_dq = i;
			pass = 1;
		} else if(check_ret && (pass == 1)) {
			right_dq = i;
			pass = 0;
		}

		if(pass) {
			count ++;
		}

#if DEBUG_TX_RX_TRAINING
		if(!check_ret) {
			putchar('1');
		} else {
			putchar('0');
		}
#endif

	}

	selected_dq_skew = left_dq + count / 2;
#if DEBUG_TX_RX_TRAINING
	serial_debug(" - pass: %d, dq_skew: %d\n", count, selected_dq_skew);
#endif

	ddr_writel(selected_dq_skew, reg_dqx);
}

static void rx_soft_training_pb_dq_all(void)
{
#if DEBUG_TX_RX_TRAINING
	serial_debug("============tuning dq @ dqs_skew: \n");
#endif
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ0, 0);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ1, 1);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ2, 2);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ3, 3);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ4, 4);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ5, 5);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ6, 6);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ7, 7);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ8, 8);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ9, 9);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ10, 10);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ11, 11);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ12, 12);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ13, 13);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ14, 14);
	rx_soft_training_pb_dqx(DDRP_INNOPHY_PBDS_RX_DQ15, 15);
}



static inline void ddr3_enable_mpr(int enable)
{
	unsigned int mr3 = global_reg_value->DDR_MR3_VALUE;

	unsigned int tmp, tmp1;

	// clear before load MR registers.

	mr3 = DDRC_LMR_START | DDRC_LMR_CMD_LMR	\
		| ((mr3 & 0xffff | (!!enable << 2)) << DDRC_LMR_DDR_ADDR_BIT) \
		| (((mr3 >> 16) & 0x7) << DDRC_LMR_BA_BIT) ;

	ddr_writel(mr3, DDRC_LMR);
	udelay(1);
#if 0
	int i = 0;
	unsigned int *p = 0xa0000000;
	for(i = 0; i < 4; i++) {
		serial_debug("%x\n", p[i]);
	}
#endif

	// restore.
	//ddr_writel(tmp, DDRC_CGUC1);
	//udelay(10);
}

static int print_read_pattern(int dq_skew)
{
	unsigned int *p = 0xa0000000;

	int i = 0;

	serial_debug("==dq_skew: %d ", dq_skew);
	serial_debug("%x\n", p[0]);


}
static int check_read_pattern(void)
{
	unsigned int *p = 0xa0000000;
	int i = 0;

#if 0
	if((p[0] == 0xffff0000) && (p[1] == 0xffff0000)  && (p[2] == 0xffff0000) && (p[3] == 0xffff0000)) {
		return 0;
	} else {
		return -1;
	}
#endif

#if 1
	for(i = 0; i < 8; i++) {
		unsigned int d = p[i];
		if(d != 0xffff0000) {
			return -1;
		}
	}
	return 0;
#endif
}




static void rx_soft_training(void)
{
	unsigned int i, j, n, m = 0, finish = 0, de_skew;
	unsigned int dqs_skew = 0;
	unsigned int cnt = 256;
	unsigned int dqs_test_all = 0;
	unsigned int *p = 0xa0000000;
	int pass = 0;
	int ret;

	int left_dq = 0;
	int right_dq = 0;

	int max_pass_count = 0;
	char selected_dq_skew = 0;
	char selected_dqs_skew = 0;

	int left_dqs = -1;
	int right_dqs = -1;

	int first_left_dq = -1;

	int pass_index = 0;


	ddr3_enable_mpr(1);

#if DEBUG_TX_RX_TRAINING
	serial_debug("==============rx dq pb deskew from 0 to 63 ============\n");
#endif
	for(dqs_skew = 0; dqs_skew <= 0x3f; dqs_skew++) {
		//这里假定了所有的DQS skew 相同, 找到最大的pass区间.
                deskew->phy_deskew_rx_dqs0 = selected_dqs_skew;
                deskew->phy_deskew_rx_dqs1 = selected_dqs_skew;
		rx_soft_training_set_pb_dqs_skew(deskew);
		pass = -1;
		left_dq = 0;
		right_dq = 0;
#if DEBUG_TX_RX_TRAINING
		if(dqs_skew < 10) {
			serial_debug("dqs_skew:0%d ", dqs_skew);
		} else {
			serial_debug("dqs_skew:%d ", dqs_skew);
		}


#endif
		for(i = 0; i <= 0x3f; i++) {
			//这里假定了所有的DQ skew 相同.
                        deskew->phy_deskew_rx_dq7_0  = i;
                        deskew->phy_deskew_rx_dq15_8 = i;
			rx_soft_training_set_pb_dq_skew(deskew);
			*(volatile unsigned int *)0xb0000000;	// 读一下总线，确保寄存器已经写入DDRPHY. 否则下面的读可能会出错.



			/*
				pass windows:
				ffff0000	->	0000ffff	偏移了一个cycle.
				0000ffff	->	ffff0000	从错误到正确，不可能偏移两个cycle.
			*/

			int check_ret = check_read_pattern();

			/* 禁止出现 PFP 的情况
			   只查找 FPF的情况.
			   如果left_dq < right_dq 了，说明right_
			*/
			if(!check_ret && (pass == -1)) {
				left_dq = i;
				pass = 1;
			} else if(check_ret && (pass == 1)) {
				right_dq = i;
				pass = 0;
			}

#if DEBUG_TX_RX_TRAINING
			if(!check_ret) {
				if(pass) {
					putchar('1');
				} else {
					putchar('x');
				}
			} else {
				putchar('0');
			}

//			print_read_pattern(i);
#endif
		}

		int count = right_dq - left_dq;

/*
	定义必须在 某个DQS skew 下，DQ 必须PASS 的count数，值太小，可能会导致找到的值不是最佳，偏小.
*/
#define DQS_MIN_PASS_COUNT	5
		if(count >= DQS_MIN_PASS_COUNT && left_dqs == -1) {
			left_dqs = dqs_skew;
			first_left_dq = left_dq;
#if DEBUG_TX_RX_TRAINING
			serial_debug(" -> * ");
#endif
		}
		if((first_left_dq == left_dq) && (count >= max_pass_count)) {

			right_dqs = dqs_skew;
			/* count 从小到大.*/
			selected_dq_skew = left_dq + (right_dq - left_dq) / 2;
			max_pass_count = count;

			tx_rx_pass_dqs_skew[pass_index]	= dqs_skew;
			tx_rx_left_dq[pass_index] 	= left_dq;
			tx_rx_right_dq[pass_index] 	= right_dq;
			pass_index++;

#if DEBUG_TX_RX_TRAINING
			serial_debug(" * <- ");
#endif

//			selected_dqs_skew = dqs_skew;
		} else {

#if DEBUG_TX_RX_TRAINING
			//serial_debug(" -> * ");
#endif
			/* count 从大到小变化, dqs 开始偏移. 结束tuning*/
//			break;
		}

#if DEBUG_TX_RX_TRAINING
		putchar('\n');
#endif
		//serial_debug("---found pass count %d @ dqs deskew: %d, max_pass_count: %d, selected_dq_skew: %d, selected_dqs_skew: %d\n", count, dqs_skew, max_pass_count, selected_dq_skew, selected_dqs_skew);

	}

	selected_dqs_skew = tx_rx_pass_dqs_skew[pass_index / 2];
	left_dq = tx_rx_left_dq[pass_index / 2];
	right_dq = tx_rx_right_dq[pass_index / 2];
	max_pass_count = tx_rx_right_dq[pass_index - 1] - tx_rx_left_dq[pass_index - 1];

	selected_dq_skew = left_dq + (right_dq - left_dq) - max_pass_count / 2;

	serial_debug("selected rx dqs: %d\n", tx_rx_pass_dqs_skew[pass_index / 2]);
	serial_debug("selected rx left dq: %d right dq: %d\n", tx_rx_left_dq[pass_index / 2], tx_rx_right_dq[pass_index / 2]);
	serial_debug("max pass rx dq: %d\n", tx_rx_right_dq[pass_index - 1] - tx_rx_left_dq[pass_index - 1]);
	serial_debug("selected dq: %d\n", selected_dq_skew);

	if(max_pass_count == 0) {
		serial_debug("rx deskew tuning error, no skew found!\n");
	} else {
                deskew->phy_deskew_rx_dqs0  = selected_dqs_skew;
                deskew->phy_deskew_rx_dqs1  = selected_dqs_skew;
                deskew->phy_deskew_rx_dq7_0  = selected_dq_skew;
                deskew->phy_deskew_rx_dq15_8 = selected_dq_skew;
		rx_soft_training_set_pb_dqs_skew(deskew);
		rx_soft_training_set_pb_dq_skew(deskew);
		*(volatile unsigned int *)0xb0000000;	// 读一下总线，确保寄存器已经写入DDRPHY. 否则下面的读可能会出错.


#if 0
		selected_dqs_skew = left_dqs + (right_dqs - left_dqs) / 2;
                deskew->phy_deskew_rx_dqs0 = selected_dqs_skew;
                deskew->phy_deskew_rx_dqs1 = selected_dqs_skew;

		// 选定一个dqs，重新tuning dq的窗口.
		rx_soft_training_set_pb_dqs_skew(deskew);

		//

#if 1
	// 一起，重新tuning 一遍.
		pass = -1;
		left_dq = 0;
		right_dq = 0;
		for(i = 0; i <= 0x3f; i++) {
                        deskew->phy_deskew_rx_dq7_0  = i;
                        deskew->phy_deskew_rx_dq15_8 = i;
                        rx_soft_training_set_pb_dq_skew(deskew);

			*(volatile unsigned int *)0xb0000000;	// 读一下总线，确保寄存器已经写入DDRPHY. 否则下面的读可能会出错.

			int check_ret = check_read_pattern();

			if(!check_ret && (pass == -1)) {
				left_dq = i;
				pass = 1;
			} else if(check_ret && (pass == 1)) {
				right_dq = i;
				pass = 0;
			}

#if DEBUG_TX_RX_TRAINING
			if(!check_ret) {
				putchar('1');
			} else {
				putchar('0');
			}
#endif

		}

#if DEBUG_TX_RX_TRAINING
		putchar('\n');
#endif
		selected_dq_skew = left_dq + (right_dq - left_dq) / 2;
                deskew->phy_deskew_rx_dq7_0  = selected_dq_skew;
                deskew->phy_deskew_rx_dq15_8 = selected_dq_skew;
		rx_soft_training_set_pb_dq_skew(deskew);
		*(volatile unsigned int *)0xb0000000;	// 读一下总线，确保寄存器已经写入DDRPHY. 否则下面的读可能会出错.
		serial_debug("rx deskew tuning done, %d found, tuned rx dq_skew: %d, rx_dqs_skew: %d\n", max_pass_count, selected_dq_skew, selected_dqs_skew);
#else
		// 每个DQ 重新tuning一遍.
		rx_soft_training_pb_dq_all();
		*(volatile unsigned int *)0xb0000000;	// 读一下总线，确保寄存器已经写入DDRPHY. 否则下面的读可能会出错.
#endif

#endif
	}

	ddr3_enable_mpr(0);
}

void ddrp_pll_init(void)
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

void ddrp_cfg(struct ddr_reg_value *global_reg_value)
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

	debug("DDRP_INNOPHY_CL:   	%x\n", ddr_readl(DDRP_INNOPHY_CL));
	debug("DRP_INNOPHY_CWL:  	%x\n", ddr_readl(DDRP_INNOPHY_CWL));
	debug("DRP_INNOPHY_AL:   	%x\n", ddr_readl(DDRP_INNOPHY_AL));
	debug("DDRP_INNOPHY_MEM_CFG:	%x\n", ddr_readl(DDRP_INNOPHY_MEM_CFG));
}

/*
 * Name     : ddrp_calibration()
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



static int do_whole_chip_scan(void)
{
	int i = 0;
	unsigned int *p = 0xa0000000;

	for(i = 0; i < 16*1024*1024/4; i++) {
		//p[i] = &p[i];
		p[i] = 0x01010101;
	}

	for(i = 0; i < 16*1024*1024/4; i++) {
		//p[i] = &p[i];
		//p[i] = 0x12345a5a;

		p[i] = (i&0xff) | (i&0xff) << 8 | (i&0xff) << 16 | (i&0xff) << 24;//(i&0xff) | ((i << 8) & 0xff) | ((i << 16)& 0xff) | ((i << 24) & 0xff);

	}
	serial_debug("after write!\n");
	for(i = 0; i < 16*1024*1024/4; i++) {

		unsigned short low16bit = p[i] & 0xffff;
		unsigned short high16bit = (p[i] >> 16) & 0xffff;

		unsigned char byte0 = p[i] & 0xff;
		unsigned char byte1 = (p[i] >> 8) & 0xff;
		unsigned char byte2 = (p[i] >> 16) & 0xff;
		unsigned char byte3 = (p[i] >> 24) & 0xff;
		int err = 0;

#if 1
		if(byte0 != (i & 0xff)) {
			err = 1;
		}
#endif
#if 1
		if(byte1 != (i & 0xff)) {
			err = 1;
		}
#endif
#if 1
		if(byte2 != (i & 0xff)) {
			err = 1;
		}
#endif
#if 1
		if(byte3 != (i & 0xff)) {
			err = 1;
		}
#endif

		if(err) {
			serial_debug("err:%x:%x, i: %x\n", &p[i], p[i], i);
			serial_debug("byte0: %x, byte3: %x\n", byte0, byte3);
			return -1;
		}
	}

	return 0;
}

unsigned char pass_invdelay[0x1f];
static void _ddrp_training_invdelay(void)
{

	int i = 0;
	int ret = 0;
	int count = 0;


	serial_debug("DDR_PHY_OFFSET_0x8: %x\n", ddr_readl(DDR_PHY_OFFSET + 0x20));
	/*TODO:*/
	for(i = 0; i < 0x1f; i++) {
		ddr_writel(i, DDRP_INNOPHY_INVDELAYSEL_DQCMD);

		serial_debug("--- loop: %d\n", i);

		ret = do_whole_chip_scan();
		if(!ret) {
			serial_debug("pass!\n");
			pass_invdelay[count++] = i;

			if(count >= 0x1f) {
				serial_debug("pass_invdelay overflow, force done\n");
				break;
			}
		}
	}

	if(!count) {
		return;
	}


	for(i = 0; i < count; i++) {
		serial_debug("passed delay: %d\n", pass_invdelay[i]);
	}

	//ddr_writel(pass_invdelay[count / 2], DDRP_INNOPHY_INVDELAYSEL_DQCMD);
	//ddr_writel(20, DDRP_INNOPHY_INVDELAYSEL_DQCMD);
}

static void _ddrp_post_init(void)
{

}
#ifdef CONFIG_DDR_DRVODT_DEBUG
static int do_whole_chip_test(void)
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
struct debug_param {
	unsigned int drv_value;
	unsigned int odt_value;
	unsigned int restart_count;
	unsigned int debug_value;
	unsigned char date_eye[32][32];
};
struct debug_param *debug_drvodt = (struct debug_param *) 0xb2400000;
void debug_date_eye(void) {
        unsigned char default_drv = 0x14;
        unsigned char default_odt = 0x5;
#ifdef CONFIG_BURNER
        int x=0,y=0;
	debug_drvodt->drv_value = 0;
	debug_drvodt->odt_value = 0;
	debug_drvodt->debug_value = 1;
	for(x=0;x<32;x++){
		for(y=0;y<32;y++){
			debug_drvodt->date_eye[x][y]=1;
		}
	}
#else
        int mem_count = 2;
	int restart_count_max = 2;
	int i=0;
	serial_debug("drv_value  is %x odt_value is %x\n",debug_drvodt->drv_value,debug_drvodt->odt_value);

        drvodt->phy_pu_drv_cmd    = debug_drvodt->drv_value;
        drvodt->phy_pd_drv_cmd    = debug_drvodt->drv_value;
        drvodt->phy_pu_drv_ck     = debug_drvodt->drv_value;
        drvodt->phy_pd_drv_ck     = debug_drvodt->drv_value;
        drvodt->phy_pu_drv_dq7_0  = debug_drvodt->drv_value;
        drvodt->phy_pd_drv_dq7_0  = debug_drvodt->drv_value;
        drvodt->phy_pu_drv_dq15_8 = debug_drvodt->drv_value;
        drvodt->phy_pd_drv_dq15_8 = debug_drvodt->drv_value;

        drvodt->phy_pu_odt_dq7_0  = debug_drvodt->odt_value;
        drvodt->phy_pd_odt_dq7_0  = debug_drvodt->odt_value;
        drvodt->phy_pu_odt_dq15_8 = debug_drvodt->odt_value;
        drvodt->phy_pd_odt_dq15_8 = debug_drvodt->odt_value;

        ddrp_zq_calibration(1, drvodt);
	ddrp_rx_dqs_auto_calibration();
	int j=0;
	pass_count = 0;
        if (debug_drvodt->debug_value > 0){
		debug_drvodt->restart_count = 0;
		debug_drvodt->debug_value = 0;
		_machine_restart();
	} else if(debug_drvodt->debug_value == 0) {
		unsigned int restart_count = 0;
		unsigned int drv_value = 0;
		unsigned int odt_value = 0;

		restart_count = debug_drvodt->restart_count;

		if(restart_count < restart_count_max){

			for(i = 0;i < mem_count;i++){
				do_whole_chip_test();
			}

			if(pass_count != mem_count){
				debug_drvodt->date_eye[debug_drvodt->drv_value][debug_drvodt->odt_value] = 0;
				restart_count = restart_count_max ;
			}
			restart_count += 1;
			debug_drvodt->restart_count = restart_count;
			_machine_restart();
		}else {
			odt_value = debug_drvodt->odt_value;
			odt_value += 1;
			debug_drvodt->odt_value = odt_value;
			if(debug_drvodt->odt_value > 0x1f){
				drv_value = debug_drvodt->drv_value;
				drv_value += 1;
				debug_drvodt->drv_value  = drv_value;
			        debug_drvodt->odt_value = 0;
			}
			debug_drvodt->debug_value = 1;
		}
		if(debug_drvodt->drv_value <= 0x1f) {
			_machine_restart();
		}else{
			/*ddr inno phy default drv_value is 0x14,odt_value is 0x5*/
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
		}
	}
#endif
}
void debug_date_eye_printf(void) {
	int i=0;
	int j=0;
        for(i=0;i<32;i++){
		  if(i<10){
		  serial_debug("drv is %d                     ",i);
		  }
		  else{
		  serial_debug("drv is %d                    ",i);
		  }
	          for(j=0;j<32;j++){
			serial_debug("%d",debug_drvodt->date_eye[i][j]);
		  }
		  serial_debug("\n");
	}
}
#endif
extern void (*ddrp_post_init)(void);
void ddrp_auto_calibration(void)
{
        drvodt = &global_reg_value->phy_drvodt;
        deskew = &global_reg_value->phy_deskew;
        ddrp_post_init = _ddrp_post_init;

#ifdef CONFIG_DDR_DRVODT_DEBUG
	debug_date_eye();
	debug_date_eye_printf();
#endif
        if (!drvodt->use_drvodt_config) {
                drvodt->phy_pu_drv_cmd    = 0xc;
                drvodt->phy_pd_drv_cmd    = 0xc;
                drvodt->phy_pu_drv_ck     = 0xc;
                drvodt->phy_pd_drv_ck     = 0xc;
                drvodt->phy_pu_drv_dq7_0  = 0xc;
                drvodt->phy_pd_drv_dq7_0  = 0xc;
                drvodt->phy_pu_drv_dq15_8 = 0xc;
                drvodt->phy_pd_drv_dq15_8 = 0xc;

                drvodt->phy_pu_odt_dq7_0  = 0x2;
                drvodt->phy_pd_odt_dq7_0  = 0x2;
                drvodt->phy_pu_odt_dq15_8 = 0x2;
                drvodt->phy_pd_odt_dq15_8 = 0x2;
        }

	//ddrp_zq_calibration(1, 0xe, 0x14, 0x14, 0x5);	 // 0xe, 0x14, 0x14, 0x5 is default value.
        ddrp_zq_calibration(1, drvodt);	// 1, bypass. drv:0xc 38.4 欧姆, odt:0x2, 282 欧姆.
	ddrp_rx_dqs_auto_calibration();

#if 0
	//ddrp_wl_calibration();	// 低频可以只做WL. 不做tx rx training.
#else
	/*如果频率低，可以不用tx rx training， 以减少启动时间.*/


	/* 利用了DDR3 的固定pattern 做training，不依赖写数据，为了防止dfi 的影响，在auto refresh之前进行rx training.*/
	//rx_soft_training();
	/*
	说明:
	tx_soft_training 过程中有 write_leveling过程 和 数据读写过程.
	write_leveling 过程 不能开启ddrc auto refresh 功能，否则对结果有影响.
	数据读写过程理论上又要求控制器的auto refresh 功能打开，否则读写的数据可能不能保持.
		--> 如果读写的数据量很小的情况下，CPU 的频率足够块，理论上可以维持数据.
	*/
	//tx_soft_training();

        if (!deskew->use_deskew_config) {
                deskew->phy_deskew_cmd       = 0x3;

                deskew->phy_deskew_rx_dm0    = 0x3;
                deskew->phy_deskew_tx_dm0    = 0x3;
                deskew->phy_deskew_rx_dq7_0  = 0x3;
                deskew->phy_deskew_tx_dq7_0  = 0x3;
                deskew->phy_deskew_rx_dqs0   = 0x3;
                deskew->phy_deskew_tx_dqs0   = 0x3;

                deskew->phy_deskew_rx_dm1    = 0x3;
                deskew->phy_deskew_tx_dm1    = 0x3;
                deskew->phy_deskew_rx_dq15_8 = 0x3;
                deskew->phy_deskew_tx_dq15_8 = 0x3;
                deskew->phy_deskew_rx_dqs1   = 0x3;
                deskew->phy_deskew_tx_dqs1   = 0x3;
        }
	rx_soft_training_set_pb_dq_skew(deskew);
#endif
}

#ifdef CONFIG_DDRP_SOFTWARE_TRAINING

//#define DDR_CHOOSE_PARAMS	0
#ifdef DDR_CHOOSE_PARAMS
static int atoi(char *pstr)
{
	int value = 0;
	int sign = 1;
	int radix;

	if(*pstr == '-'){
		sign = -1;
		pstr++;
	}
	if(*pstr == '0' && (*(pstr+1) == 'x' || *(pstr+1) == 'X')){
		radix = 16;
		pstr += 2;
	}
	else
		radix = 10;
	while(*pstr){
		if(radix == 16){
			if(*pstr >= '0' && *pstr <= '9')
				value = value * radix + *pstr - '0';
			else if(*pstr >= 'A' && *pstr <= 'F')
				value = value * radix + *pstr - 'A' + 10;
			else if(*pstr >= 'a' && *pstr <= 'f')
				value = value * radix + *pstr - 'a' + 10;
		}
		else
			value = value * radix + *pstr - '0';
		pstr++;
	}
	return sign*value;
}


static int choose_params(int m)
{
	char buf[16];
	char ch;
	char *p = buf;
	int select_m;

	debug("Please select from [0 to %d]\n", m);

	debug(">>  ");

	while((ch = getc()) != '\r') {
		putc(ch);
		*p++ = ch;

		if((p - buf) > 16)
			break;
	}
	*p = '\0';

	debug("\n");

	select_m = atoi(buf);
	debug("slected: %d\n", select_m);

	return select_m;
}
#endif

struct ddrp_calib {
	union{
		uint8_t u8;
		struct{
			unsigned cyclesel:3;
			unsigned reserved:5;
		}b;
	}bypass_c;
	union{
		uint8_t u8;
		struct{
			unsigned ophsel:3;
			unsigned dllsel:5;
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

/* struct ddrp_calib calib_val[8 * 4 * 8 * 5]; */
/*
 * Name     : ddrp_calibration_manual()
 * Function : control the RX DQS window delay to the DQS
 *
 * a_low_8bit_delay	= al8_2x * clk_2x + al8_1x * clk_1x;
 * a_high_8bit_delay	= ah8_2x * clk_2x + ah8_1x * clk_1x;
 *
 * */
struct ddrp_calib calib_val[8*8*8];

static void ddrp_software_calibration(void)
{

	int x, y, z, x1, y1;
	int c, o, d =0, r = 0;
	unsigned int addr = 0xa1000000, val;
	unsigned int i, n, m = 0;
	unsigned int reg;
	unsigned int tmp;
	volatile unsigned int val1 = 0;

	ddr_writel(0x20, DDRP_INNOPHY_TRAINING_CTRL);
	ddr_writel(0x0, DDRP_INNOPHY_CALIB_MODE);
	reg = ddr_readl(DDRP_INNOPHY_TRAINING_CTRL);
	reg |= (DDRP_TRAINING_CTRL_DSCSE_BP);
	reg &= ~(DDRP_TRAINING_CTRL_DSACE_START);
	ddr_writel(reg, DDRP_INNOPHY_TRAINING_CTRL);

	for(c = 0; c < 8; c ++) {
		for(o = 0; o < 8; o++) {
			for (d = 0; d < 32; d++) {
				x = d << 3 | o;
				x1 = c;
				y = d << 3 | o;
				y1 = c;
				ddr_writel(x, DDRP_INNOPHY_CALIB_BYPASS_AL);
				ddr_writel(x1, DDRP_INNOPHY_CALIB_BYPASS_AL_C);
				ddr_writel(y, DDRP_INNOPHY_CALIB_BYPASS_AH);
				ddr_writel(y1, DDRP_INNOPHY_CALIB_BYPASS_AH_C);

				for(i = 0; i < 0xff; i++) {
					val = 0;
					for(n = 0; n < 4; n++ ) {
						val |= (i+n)<<(n * 8);
					}

					*(volatile unsigned int *)(addr + i * 4) = val;
					val1 = *(volatile unsigned int *)(addr + i * 4);

					if(val1 != val) {
						break;
					}
#if 0
					serial_debug("val1 : 0x%x val : 0x%x, addr = 0x%x, bypass_l_c : %x bypass_l :%x bypass_h_c : %x bypass: %x\n", val1, val, addr + i * 4, \
						ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AL_C),\
						ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AL), \
						ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AH_C),\
						ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AH));
#endif
				}
				if(i == 0xff) {
					calib_val[m].bypass_c.b.cyclesel = c;
					calib_val[m].bypass.b.ophsel = o;
					calib_val[m].bypass.b.dllsel = d;
					calib_val[m].rx_dll.b.rx_dll = r;
					m++;
				}
			}
		}
	}

	if(!m) {
		serial_debug("calib bypass fail\n");
		return ;
	}


	m = m  / 2;
	c = calib_val[m].bypass_c.b.cyclesel;
	o = calib_val[m].bypass.b.ophsel;
	d = calib_val[m].bypass.b.dllsel;
	r = calib_val[m].rx_dll.b.rx_dll;

	serial_debug("m = %d   c = %d   o = %d   d = %d  r = %d\n", m, c, o, d, r);

	x = d << 3 | o;
	x1 = c;
	y = d << 3 | o;
	y1 = c;
	ddr_writel(x, DDRP_INNOPHY_CALIB_BYPASS_AL);
	ddr_writel(y, DDRP_INNOPHY_CALIB_BYPASS_AH);
	ddr_writel(x1, DDRP_INNOPHY_CALIB_BYPASS_AL_C);
	ddr_writel(y1, DDRP_INNOPHY_CALIB_BYPASS_AH_C);

}


#endif


