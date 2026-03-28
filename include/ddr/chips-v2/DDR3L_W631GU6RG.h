/*
 * =====================================================================================
 *
 *       Filename:  DDR3L_W631GU6RG.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2020年09月21日 17时55分13秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef __DDR3L_W631GU6RG_H__
#define __DDR3L_W631GU6RG_H__



/*
 * CL:5, CWL:5  303Mhz ~ 330Mhz
 * CL:6, CWL:5	330Mhz ~ 400Mhz
 * CL:8, CWL:6	400Mhz ~ 533Mhz
 * CL:10, CWL:7 533Mhz ~ 667Mhz
 * CL:11, CWL:8 667Mhz ~ 800Mhz
 * CL:13, CWL:9 800Mhz ~ 933Mhz
 * CL:14, CWL:10
 *
 * */

#ifndef CONFIG_DDR3L_W631GU6RG_MEM_FREQ
#define CONFIG_DDR3L_W631GU6RG_MEM_FREQ CONFIG_SYS_MEM_FREQ
#endif

#if (CONFIG_DDR_SEL_PLL == MPLL)
#define CONFIG_SYS_PLL_FREQ CONFIG_SYS_MPLL_FREQ
#else
#define CONFIG_SYS_PLL_FREQ CONFIG_SYS_APLL_FREQ
#endif

#define CONFIG_DDR_DATA_RATE (CONFIG_DDR3L_W631GU6RG_MEM_FREQ * 2)

#if((CONFIG_SYS_PLL_FREQ % CONFIG_DDR3L_W631GU6RG_MEM_FREQ) ||\
	(CONFIG_SYS_PLL_FREQ / CONFIG_DDR3L_W631GU6RG_MEM_FREQ < 0) ||\
	(CONFIG_SYS_PLL_FREQ / CONFIG_DDR3L_W631GU6RG_MEM_FREQ > 15))
#error DDR memoryclock division ratio should be an integer between 1 and 16, check CONFIG_SYS_MPLL_FREQ and CONFIG_DDR3L_W631GU6RG_MEM_FREQ
#endif

#if ((CONFIG_DDR_DATA_RATE >= 606000000) && (CONFIG_DDR_DATA_RATE <= 660000000))
#define CONFIG_DDR_CL	5
#define CONFIG_DDR_CWL	5
#elif((CONFIG_DDR_DATA_RATE > 660000000) && (CONFIG_DDR_DATA_RATE <= 800000000))
#define CONFIG_DDR_CL	6
#define CONFIG_DDR_CWL	5
#elif((CONFIG_DDR_DATA_RATE > 800000000) && (CONFIG_DDR_DATA_RATE <= 1066000000))
#define CONFIG_DDR_CL	8
#define CONFIG_DDR_CWL	6
#elif((CONFIG_DDR_DATA_RATE > 1066000000) && (CONFIG_DDR_DATA_RATE <= 1333000000))
#define CONFIG_DDR_CL	10
#define CONFIG_DDR_CWL	7
/*This frequency band is not labeled in the KGD manual and has not undergone high and low temperature testing*/
#elif((CONFIG_DDR_DATA_RATE > 1333000000) && (CONFIG_DDR_DATA_RATE <= 1600000000))
#define CONFIG_DDR_CL	11
#define CONFIG_DDR_CWL	8
#elif((CONFIG_DDR_DATA_RATE > 1600000000) && (CONFIG_DDR_DATA_RATE <= 1866000000))
#define CONFIG_DDR_CL	13
#define CONFIG_DDR_CWL	9
#else
#define CONFIG_DDR_CL	14
#define CONFIG_DDR_CWL	10
#endif

#if(-1 == CONFIG_DDR_CL)
#error CONFIG_DDR3L_W631GU6RG_MEM_FREQ don't support, check data_rate range
#endif

#if !defined(CONFIG_DDR3_W631GU6RG_KGD_CONFIG) && \
        defined(CONFIG_DDR3_KGD_CONFIG)
        #define CONFIG_DDR3_W631GU6RG_KGD_CONFIG            CONFIG_DDR3_KGD_CONFIG
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_KGD_MR0_DLL_RST) && \
        defined(CONFIG_DDR3_KGD_MR0_DLL_RST)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR0_DLL_RST       CONFIG_DDR3_KGD_MR0_DLL_RST
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_KGD_MR0_PD) && \
        defined(CONFIG_DDR3_KGD_MR0_PD)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR0_PD            CONFIG_DDR3_KGD_MR0_PD
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_KGD_MR1_DLL_EN) && \
        defined(CONFIG_DDR3_KGD_MR1_DLL_EN)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR1_DLL_EN        CONFIG_DDR3_KGD_MR1_DLL_EN
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_KGD_MR1_DIC) && \
        defined(CONFIG_DDR3_KGD_MR1_DIC)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR1_DIC           CONFIG_DDR3_KGD_MR1_DIC
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_KGD_MR1_RTT_NOM) && \
        defined(CONFIG_DDR3_KGD_MR1_RTT_NOM)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR1_RTT_NOM       CONFIG_DDR3_KGD_MR1_RTT_NOM
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_KGD_MR2_RTT_WR) && \
        defined(CONFIG_DDR3_KGD_MR2_RTT_WR)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR2_RTT_WR        CONFIG_DDR3_KGD_MR2_RTT_WR
#endif

#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DRVODT_CONFIG) && \
        defined(CONFIG_PHY_DRVODT_CONFIG)
        #define CONFIG_DDR3_W631GU6RG_PHY_DRVODT_CONFIG     CONFIG_PHY_DRVODT_CONFIG
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_PU_DRV_CMD) && \
        defined(CONFIG_PHY_PU_DRV_CMD)
        #define CONFIG_DDR3_W631GU6RG_PHY_PU_DRV_CMD        CONFIG_PHY_PU_DRV_CMD
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_PD_DRV_CMD) && \
        defined(CONFIG_PHY_PD_DRV_CMD)
        #define CONFIG_DDR3_W631GU6RG_PHY_PD_DRV_CMD        CONFIG_PHY_PD_DRV_CMD
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_PU_DRV_CK) && \
        defined(CONFIG_PHY_PU_DRV_CK)
        #define CONFIG_DDR3_W631GU6RG_PHY_PU_DRV_CK         CONFIG_PHY_PU_DRV_CK
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_PD_DRV_CK) && \
        defined(CONFIG_PHY_PD_DRV_CK)
        #define CONFIG_DDR3_W631GU6RG_PHY_PD_DRV_CK         CONFIG_PHY_PD_DRV_CK
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_PU_DRV_DQ7_0) && \
        defined(CONFIG_PHY_PU_DRV_DQ7_0)
        #define CONFIG_DDR3_W631GU6RG_PHY_PU_DRV_DQ7_0      CONFIG_PHY_PU_DRV_DQ7_0
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_PD_DRV_DQ7_0) && \
        defined(CONFIG_PHY_PD_DRV_DQ7_0)
        #define CONFIG_DDR3_W631GU6RG_PHY_PD_DRV_DQ7_0      CONFIG_PHY_PD_DRV_DQ7_0
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_PU_DRV_DQ15_8) && \
        defined(CONFIG_PHY_PU_DRV_DQ15_8)
        #define CONFIG_DDR3_W631GU6RG_PHY_PU_DRV_DQ15_8     CONFIG_PHY_PU_DRV_DQ15_8
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_PD_DRV_DQ15_8) && \
        defined(CONFIG_PHY_PD_DRV_DQ15_8)
        #define CONFIG_DDR3_W631GU6RG_PHY_PD_DRV_DQ15_8     CONFIG_PHY_PD_DRV_DQ15_8
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_PU_ODT_DQ7_0) && \
        defined(CONFIG_PHY_PU_ODT_DQ7_0)
        #define CONFIG_DDR3_W631GU6RG_PHY_PU_ODT_DQ7_0      CONFIG_PHY_PU_ODT_DQ7_0
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_PD_ODT_DQ7_0) && \
        defined(CONFIG_PHY_PD_ODT_DQ7_0)
        #define CONFIG_DDR3_W631GU6RG_PHY_PD_ODT_DQ7_0      CONFIG_PHY_PD_ODT_DQ7_0
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_PU_ODT_DQ15_8) && \
        defined(CONFIG_PHY_PU_ODT_DQ15_8)
        #define CONFIG_DDR3_W631GU6RG_PHY_PU_ODT_DQ15_8     CONFIG_PHY_PU_ODT_DQ15_8
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_PD_ODT_DQ15_8) && \
        defined(CONFIG_PHY_PD_ODT_DQ15_8)
        #define CONFIG_DDR3_W631GU6RG_PHY_PD_ODT_DQ15_8     CONFIG_PHY_PD_ODT_DQ15_8
#endif

#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_CONFIG) && \
        defined(CONFIG_PHY_DESKEW_CONFIG)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_CONFIG     CONFIG_PHY_DESKEW_CONFIG
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_CMD) && \
        defined(CONFIG_PHY_DESKEW_CMD)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_CMD        CONFIG_PHY_DESKEW_CMD
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DM0) && \
        defined(CONFIG_PHY_DESKEW_RX_DM0)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DM0     CONFIG_PHY_DESKEW_RX_DM0
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DM0) && \
        defined(CONFIG_PHY_DESKEW_TX_DM0)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DM0     CONFIG_PHY_DESKEW_TX_DM0
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DQ7_0) && \
        defined(CONFIG_PHY_DESKEW_RX_DQ7_0)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DQ7_0   CONFIG_PHY_DESKEW_RX_DQ7_0
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DQ7_0) && \
        defined(CONFIG_PHY_DESKEW_TX_DQ7_0)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DQ7_0   CONFIG_PHY_DESKEW_TX_DQ7_0
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DQS0) && \
        defined(CONFIG_PHY_DESKEW_RX_DQS0)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DQS0    CONFIG_PHY_DESKEW_RX_DQS0
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DQS0) && \
        defined(CONFIG_PHY_DESKEW_TX_DQS0)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DQS0    CONFIG_PHY_DESKEW_TX_DQS0
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DM1) && \
        defined(CONFIG_PHY_DESKEW_RX_DM1)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DM1     CONFIG_PHY_DESKEW_RX_DM1
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DM1) && \
        defined(CONFIG_PHY_DESKEW_TX_DM1)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DM1     CONFIG_PHY_DESKEW_TX_DM1
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DQ15_8) && \
        defined(CONFIG_PHY_DESKEW_RX_DQ15_8)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DQ15_8  CONFIG_PHY_DESKEW_RX_DQ15_8
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DQ15_8) && \
        defined(CONFIG_PHY_DESKEW_TX_DQ15_8)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DQ15_8  CONFIG_PHY_DESKEW_TX_DQ15_8
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DQS1) && \
        defined(CONFIG_PHY_DESKEW_RX_DQS1)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DQS1    CONFIG_PHY_DESKEW_RX_DQS1
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DQS1) && \
        defined(CONFIG_PHY_DESKEW_TX_DQS1)
        #define CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DQS1    CONFIG_PHY_DESKEW_TX_DQS1
#endif


#if defined(CONFIG_DDR_DLL_RESET_EN) || defined(CONFIG_DDR_DLL_OFF)  || \
        defined(CONFIG_DDR_DRIVER_OUT_STRENGTH) || defined(CONFIG_OPEN_KGD_DRIVER_STRENGTH)
        #define CONFIG_DDR3_W631GU6RG_KGD_CONFIG            0x1

#if defined(CONFIG_DDR_DLL_RESET_EN)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR0_DLL_RST       0x1
#elif !defined(CONFIG_DDR3_W631GU6RG_KGD_MR0_DLL_RST)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR0_DLL_RST       0x1
#endif
#if defined(CONFIG_DDR_DLL_OFF)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR1_DLL_EN        0x1
        #define CONFIG_DDR3_W631GU6RG_KGD_MR0_PD            0x0
#else
#if !defined(CONFIG_DDR3_W631GU6RG_KGD_MR1_DLL_EN)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR1_DLL_EN        0x0
#endif
#if !defined(CONFIG_DDR3_W631GU6RG_KGD_MR0_PD)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR0_PD            0x1
#endif
#endif
#if defined(CONFIG_DDR_DRIVER_OUT_STRENGTH)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR1_DIC       \
                ((CONFIG_DDR_DRIVER_OUT_STRENGTH_1 << 1) | CONFIG_DDR_DRIVER_OUT_STRENGTH_0)
#elif !defined(CONFIG_DDR3_W631GU6RG_KGD_MR1_DIC)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR1_DIC           0x1
#endif
#if defined(CONFIG_DDR_CHIP_ODT_VAL) || defined(CONFIG_DDR_CHIP_ODT)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR1_RTT_NOM       \
                ((CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_9 << 2) |   \
                 (CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_6 << 1) |   \
                 CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_2)
#elif !defined(CONFIG_DDR3_W631GU6RG_KGD_MR1_RTT_NOM)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR1_RTT_NOM       0x1
#endif
#if defined(CONFIG_DDR_CHIP_ODT_VAL_RTT_WR)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR2_RTT_WR        CONFIG_DDR_CHIP_ODT_VAL_RTT_WR
#elif !defined(CONFIG_DDR3_W631GU6RG_KGD_MR2_RTT_WR)
        #define CONFIG_DDR3_W631GU6RG_KGD_MR2_RTT_WR        0x0
#endif

#endif

static inline void DDR3L_W631GU6RG_init(void *data)
{
	struct ddr_chip_info *c = (struct ddr_chip_info *)data;


	c->DDR_ROW  		= 13,
	c->DDR_ROW1 		= 13,
	c->DDR_COL  		= 10,
	c->DDR_COL1 		= 10,
	c->DDR_BANK8 		= 1,
	c->DDR_BL	   	= 8,
	c->DDR_CL	   	= CONFIG_DDR_CL,
	c->DDR_CWL	   	= CONFIG_DDR_CWL,

	c->DDR_RL	   	= DDR__tck(c->DDR_CL),
	c->DDR_WL	   	= DDR__tck(c->DDR_CWL),

	c->DDR_tRAS  		= DDR__ns(34);
	c->DDR_tRTP  		= DDR_SELECT_MAX__tCK_ps(4, 7500);
	c->DDR_tRP   		= DDR__ns(14);
	c->DDR_tRCD  		= DDR__ns(14);
	c->DDR_tRC   		= c->DDR_tRAS + c->DDR_tRP;
	c->DDR_tRRD  		= DDR_SELECT_MAX__tCK_ps(4, 6000);
	c->DDR_tWR   		= DDR__ns(15);
	c->DDR_tWTR  		= DDR_SELECT_MAX__tCK_ps(4, 7500);
	c->DDR_tCCD  		= DDR__tck(4);
	c->DDR_tFAW  		= DDR__ns(35);

	c->DDR_tRFC  		= DDR__ns(110);
	c->DDR_tREFI 		= DDR__ns(3900);

	c->DDR_tCKE  		= DDR_SELECT_MAX__tCK_ps(3, 5000);
	c->DDR_tCKESR 		= c->DDR_tCKE + DDR__tck(1);
	c->DDR_tCKSRE 		= DDR_SELECT_MAX__tCK_ps(5, 10000);
	c->DDR_tXP  		= DDR_SELECT_MAX__tCK_ps(3, 6000);
	c->DDR_tMRD		= DDR__tck(4);
	c->DDR_tXSDLL		= DDR__tck(512);
	c->DDR_tMOD   		= DDR_SELECT_MAX__tCK_ps(12, 15 * 1000);
	c->DDR_tXPDLL 		= DDR_SELECT_MAX__tCK_ps(10, 24 * 1000);

#ifdef CONFIG_DDR3_W631GU6RG_KGD_CONFIG
        struct ddr3_mr_config *mr_cfg      = &c->kgd_config.mr_config;
        c->kgd_config.use_kgd_config       = CONFIG_DDR3_W631GU6RG_KGD_CONFIG     ;
        mr_cfg->kgd_mr0_dll_rst            = CONFIG_DDR3_W631GU6RG_KGD_MR0_DLL_RST;
        mr_cfg->kgd_mr0_pd                 = CONFIG_DDR3_W631GU6RG_KGD_MR0_PD     ;
        mr_cfg->kgd_mr1_dll_en             = CONFIG_DDR3_W631GU6RG_KGD_MR1_DLL_EN ;
        mr_cfg->kgd_mr1_dic                = CONFIG_DDR3_W631GU6RG_KGD_MR1_DIC    ;
        mr_cfg->kgd_mr1_rtt_nom            = CONFIG_DDR3_W631GU6RG_KGD_MR1_RTT_NOM;
        mr_cfg->kgd_mr2_rtt_wr             = CONFIG_DDR3_W631GU6RG_KGD_MR2_RTT_WR ;
#endif
#ifdef CONFIG_DDR3_W631GU6RG_PHY_DRVODT_CONFIG
        c->phy_drvodt.use_drvodt_config    = CONFIG_DDR3_W631GU6RG_PHY_DRVODT_CONFIG;
        c->phy_drvodt.phy_pu_drv_cmd       = CONFIG_DDR3_W631GU6RG_PHY_PU_DRV_CMD   ;
        c->phy_drvodt.phy_pd_drv_cmd       = CONFIG_DDR3_W631GU6RG_PHY_PD_DRV_CMD   ;
        c->phy_drvodt.phy_pu_drv_ck        = CONFIG_DDR3_W631GU6RG_PHY_PU_DRV_CK    ;
        c->phy_drvodt.phy_pd_drv_ck        = CONFIG_DDR3_W631GU6RG_PHY_PD_DRV_CK    ;
        c->phy_drvodt.phy_pu_drv_dq7_0     = CONFIG_DDR3_W631GU6RG_PHY_PU_DRV_DQ7_0 ;
        c->phy_drvodt.phy_pd_drv_dq7_0     = CONFIG_DDR3_W631GU6RG_PHY_PD_DRV_DQ7_0 ;
        c->phy_drvodt.phy_pu_drv_dq15_8    = CONFIG_DDR3_W631GU6RG_PHY_PU_DRV_DQ15_8;
        c->phy_drvodt.phy_pd_drv_dq15_8    = CONFIG_DDR3_W631GU6RG_PHY_PD_DRV_DQ15_8;
        c->phy_drvodt.phy_pu_odt_dq7_0     = CONFIG_DDR3_W631GU6RG_PHY_PU_ODT_DQ7_0 ;
        c->phy_drvodt.phy_pd_odt_dq7_0     = CONFIG_DDR3_W631GU6RG_PHY_PD_ODT_DQ7_0 ;
        c->phy_drvodt.phy_pu_odt_dq15_8    = CONFIG_DDR3_W631GU6RG_PHY_PU_ODT_DQ15_8;
        c->phy_drvodt.phy_pd_odt_dq15_8    = CONFIG_DDR3_W631GU6RG_PHY_PD_ODT_DQ15_8;
#endif
#ifdef CONFIG_DDR3_W631GU6RG_PHY_DESKEW_CONFIG
        c->phy_deskew.use_deskew_config    = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_CONFIG   ;
        c->phy_deskew.phy_deskew_cmd       = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_CMD      ;
        c->phy_deskew.phy_deskew_rx_dm0    = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DM0   ;
        c->phy_deskew.phy_deskew_tx_dm0    = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DM0   ;
        c->phy_deskew.phy_deskew_rx_dq7_0  = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DQ7_0 ;
        c->phy_deskew.phy_deskew_tx_dq7_0  = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DQ7_0 ;
        c->phy_deskew.phy_deskew_rx_dqs0   = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DQS0  ;
        c->phy_deskew.phy_deskew_tx_dqs0   = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DQS0  ;
        c->phy_deskew.phy_deskew_rx_dm1    = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DM1   ;
        c->phy_deskew.phy_deskew_tx_dm1    = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DM1   ;
        c->phy_deskew.phy_deskew_rx_dq15_8 = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DQ15_8;
        c->phy_deskew.phy_deskew_tx_dq15_8 = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DQ15_8;
        c->phy_deskew.phy_deskew_rx_dqs1   = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_RX_DQS1  ;
        c->phy_deskew.phy_deskew_tx_dqs1   = CONFIG_DDR3_W631GU6RG_PHY_DESKEW_TX_DQS1  ;
#endif
}

#define DDR3L_W631GU6RG {					\
	.name 	= "W631GU6RG",					\
	.id	= DDR_CHIP_ID(VENDOR_WINBOND, TYPE_DDR3, MEM_128M),	\
	.type	= DDR3,						\
	.freq	= CONFIG_DDR3L_W631GU6RG_MEM_FREQ,			\
	.size	= 128,						\
	.init	= DDR3L_W631GU6RG_init,				\
}


#endif
