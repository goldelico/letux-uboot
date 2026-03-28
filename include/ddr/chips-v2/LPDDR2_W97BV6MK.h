/*
 * =====================================================================================
 *
 *       Filename:  LPDDR2_W97BV6MK.h
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

#ifndef __LPDDR2_W97BV6MK_H__
#define __LPDDR2_W97BV6MK_H__

/*
 * CL:3,50M ~ 166M
 * CL:3,166M ~ 200M
 * CL:4,200M ~ 266M
 * CL:4,266M ~ 333M
 * CL:5,333M ~ 400M
 * CL:5,400M ~ 466M
 * CL:7,466M ~ 533M
 *
 * */

#ifndef CONFIG_LPDDR2_W97BV6MK_MEM_FREQ
#define CONFIG_LPDDR2_W97BV6MK_MEM_FREQ CONFIG_SYS_MEM_FREQ
#endif

#if (CONFIG_DDR_SEL_PLL == MPLL)
#define CONFIG_SYS_PLL_FREQ CONFIG_SYS_MPLL_FREQ
#else
#define CONFIG_SYS_PLL_FREQ CONFIG_SYS_APLL_FREQ
#endif

#define CONFIG_DDR_DATA_RATE (CONFIG_LPDDR2_W97BV6MK_MEM_FREQ * 2)

#if((CONFIG_SYS_PLL_FREQ % CONFIG_LPDDR2_W97BV6MK_MEM_FREQ) ||\
	(CONFIG_SYS_PLL_FREQ / CONFIG_LPDDR2_W97BV6MK_MEM_FREQ < 0) ||\
	(CONFIG_SYS_PLL_FREQ / CONFIG_LPDDR2_W97BV6MK_MEM_FREQ > 15))
#error DDR memoryclock division ratio should be an integer between 1 and 16, check CONFIG_SYS_MPLL_FREQ and CONFIG_LPDDR2_W97BV6MK_MEM_FREQ
#endif

#if	((CONFIG_DDR_DATA_RATE > 50000000) &&\
		(CONFIG_DDR_DATA_RATE <= 333000000))
#define CONFIG_DDR_RL	3
#define CONFIG_DDR_WL	1
#elif((CONFIG_DDR_DATA_RATE > 333000000) &&\
		(CONFIG_DDR_DATA_RATE <= 400000000))
#define CONFIG_DDR_RL	3
#define CONFIG_DDR_WL	1
#elif((CONFIG_DDR_DATA_RATE > 400000000) &&\
		(CONFIG_DDR_DATA_RATE <= 533000000))
#define CONFIG_DDR_RL	4
#define CONFIG_DDR_WL	2
#elif((CONFIG_DDR_DATA_RATE > 533000000) &&\
		(CONFIG_DDR_DATA_RATE <= 667000000))
#define CONFIG_DDR_RL	5
#define CONFIG_DDR_WL	2
#elif((CONFIG_DDR_DATA_RATE > 667000000) &&\
		(CONFIG_DDR_DATA_RATE <= 800000000))
#define CONFIG_DDR_RL	6
#define CONFIG_DDR_WL	3
#elif((CONFIG_DDR_DATA_RATE > 800000000) &&\
		(CONFIG_DDR_DATA_RATE <= 933000000))
#define CONFIG_DDR_RL	7
#define CONFIG_DDR_WL	4
#elif((CONFIG_DDR_DATA_RATE > 933000000) &&\
		(CONFIG_DDR_DATA_RATE <= 1066000000))
#define CONFIG_DDR_RL	8
#define CONFIG_DDR_WL	4
#else
#define CONFIG_DDR_RL	-1
#define CONFIG_DDR_WL	-1
#endif

#if(-1 == CONFIG_DDR_RL)
#error CONFIG_LPDDR2_W97BV6MK_MEM_FREQ don't support, check data_rate range
#endif


#if !defined(CONFIG_LPDDR2_W97BV6MK_KGD_CONFIG) && \
        defined(CONFIG_DDR2_KGD_CONFIG)
        #define CONFIG_LPDDR2_W97BV6MK_KGD_CONFIG            CONFIG_DDR2_KGD_CONFIG
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_KGD_MR3_DS) && \
        defined(CONFIG_DDR2_KGD_MR3_DS)
        #define CONFIG_LPDDR2_W97BV6MK_KGD_MR3_DS            CONFIG_DDR2_KGD_MR3_DS
#endif

#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DRVODT_CONFIG) && \
        defined(CONFIG_PHY_DRVODT_CONFIG)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DRVODT_CONFIG     CONFIG_PHY_DRVODT_CONFIG
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_PU_DRV_CMD) && \
        defined(CONFIG_PHY_PU_DRV_CMD)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_PU_DRV_CMD        CONFIG_PHY_PU_DRV_CMD
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_PD_DRV_CMD) && \
        defined(CONFIG_PHY_PD_DRV_CMD)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_PD_DRV_CMD        CONFIG_PHY_PD_DRV_CMD
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_PU_DRV_CK) && \
        defined(CONFIG_PHY_PU_DRV_CK)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_PU_DRV_CK         CONFIG_PHY_PU_DRV_CK
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_PD_DRV_CK) && \
        defined(CONFIG_PHY_PD_DRV_CK)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_PD_DRV_CK         CONFIG_PHY_PD_DRV_CK
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_PU_DRV_DQ7_0) && \
        defined(CONFIG_PHY_PU_DRV_DQ7_0)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_PU_DRV_DQ7_0      CONFIG_PHY_PU_DRV_DQ7_0
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_PD_DRV_DQ7_0) && \
        defined(CONFIG_PHY_PD_DRV_DQ7_0)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_PD_DRV_DQ7_0      CONFIG_PHY_PD_DRV_DQ7_0
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_PU_DRV_DQ15_8) && \
        defined(CONFIG_PHY_PU_DRV_DQ15_8)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_PU_DRV_DQ15_8     CONFIG_PHY_PU_DRV_DQ15_8
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_PD_DRV_DQ15_8) && \
        defined(CONFIG_PHY_PD_DRV_DQ15_8)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_PD_DRV_DQ15_8     CONFIG_PHY_PD_DRV_DQ15_8
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_PU_ODT_DQ7_0) && \
        defined(CONFIG_PHY_PU_ODT_DQ7_0)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_PU_ODT_DQ7_0      CONFIG_PHY_PU_ODT_DQ7_0
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_PD_ODT_DQ7_0) && \
        defined(CONFIG_PHY_PD_ODT_DQ7_0)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_PD_ODT_DQ7_0      CONFIG_PHY_PD_ODT_DQ7_0
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_PU_ODT_DQ15_8) && \
        defined(CONFIG_PHY_PU_ODT_DQ15_8)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_PU_ODT_DQ15_8     CONFIG_PHY_PU_ODT_DQ15_8
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_PD_ODT_DQ15_8) && \
        defined(CONFIG_PHY_PD_ODT_DQ15_8)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_PD_ODT_DQ15_8     CONFIG_PHY_PD_ODT_DQ15_8
#endif

#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_CONFIG) && \
        defined(CONFIG_PHY_DESKEW_CONFIG)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_CONFIG     CONFIG_PHY_DESKEW_CONFIG
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_CMD) && \
        defined(CONFIG_PHY_DESKEW_CMD)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_CMD        CONFIG_PHY_DESKEW_CMD
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DM0) && \
        defined(CONFIG_PHY_DESKEW_RX_DM0)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DM0     CONFIG_PHY_DESKEW_RX_DM0
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DM0) && \
        defined(CONFIG_PHY_DESKEW_TX_DM0)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DM0     CONFIG_PHY_DESKEW_TX_DM0
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DQ7_0) && \
        defined(CONFIG_PHY_DESKEW_RX_DQ7_0)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DQ7_0   CONFIG_PHY_DESKEW_RX_DQ7_0
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DQ7_0) && \
        defined(CONFIG_PHY_DESKEW_TX_DQ7_0)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DQ7_0   CONFIG_PHY_DESKEW_TX_DQ7_0
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DQS0) && \
        defined(CONFIG_PHY_DESKEW_RX_DQS0)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DQS0    CONFIG_PHY_DESKEW_RX_DQS0
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DQS0) && \
        defined(CONFIG_PHY_DESKEW_TX_DQS0)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DQS0    CONFIG_PHY_DESKEW_TX_DQS0
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DM1) && \
        defined(CONFIG_PHY_DESKEW_RX_DM1)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DM1     CONFIG_PHY_DESKEW_RX_DM1
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DM1) && \
        defined(CONFIG_PHY_DESKEW_TX_DM1)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DM1     CONFIG_PHY_DESKEW_TX_DM1
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DQ15_8) && \
        defined(CONFIG_PHY_DESKEW_RX_DQ15_8)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DQ15_8  CONFIG_PHY_DESKEW_RX_DQ15_8
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DQ15_8) && \
        defined(CONFIG_PHY_DESKEW_TX_DQ15_8)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DQ15_8  CONFIG_PHY_DESKEW_TX_DQ15_8
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DQS1) && \
        defined(CONFIG_PHY_DESKEW_RX_DQS1)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DQS1    CONFIG_PHY_DESKEW_RX_DQS1
#endif
#if !defined(CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DQS1) && \
        defined(CONFIG_PHY_DESKEW_TX_DQS1)
        #define CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DQS1    CONFIG_PHY_DESKEW_TX_DQS1
#endif

#if defined(CONFIG_DDR_DRIVER_STRENGTH)
        #define CONFIG_LPDDR2_W97BV6MK_KGD_CONFIG            0x1
        #define CONFIG_LPDDR2_W97BV6MK_KGD_MR3_DS            CONFIG_DDR_DRIVER_STRENGTH
#endif


static inline void LPDDR2_W97BV6MK_init(void *data)
{
	struct ddr_chip_info *c = (struct ddr_chip_info *)data;


	c->DDR_ROW  		= 14,
	c->DDR_ROW1 		= 14,
	c->DDR_COL  		= 10,
	c->DDR_COL1 		= 10,
	c->DDR_BANK8 		= 1,
	c->DDR_BL	   	= 8,
	c->DDR_RL	   	= CONFIG_DDR_RL;
	c->DDR_WL	   	= CONFIG_DDR_WL;

	c->DDR_tMRW  		= DDR__tck(5);
	c->DDR_tDQSCK 		= DDR__ps(2000);
	c->DDR_tDQSCKMAX 	= DDR__ps(10000);
	c->DDR_tRAS  		= DDR_SELECT_MAX__tCK_ps(3, 42 * 1000);
	c->DDR_tRTP  		= DDR_SELECT_MAX__tCK_ps(2, 7500);
	c->DDR_tRP   		= DDR_SELECT_MAX__tCK_ps(3, 21 * 1000);
	c->DDR_tRCD  		= DDR_SELECT_MAX__tCK_ps(3, 18 * 1000);
	c->DDR_tRC   		= c->DDR_tRAS + c->DDR_tRP;
	c->DDR_tRRD  		= DDR_SELECT_MAX__tCK_ps(2, 10 * 1000);
	c->DDR_tWR   		= DDR_SELECT_MAX__tCK_ps(3, 15 * 1000);
	c->DDR_tWTR  		= DDR_SELECT_MAX__tCK_ps(2, 7500);
	c->DDR_tCCD  		= DDR__tck(2);
	c->DDR_tFAW  		= DDR_SELECT_MAX__tCK_ps(8, 50 * 1000);

	c->DDR_tRFC  		= DDR__ns(130);
	c->DDR_tREFI 		= DDR__ns(3900);

	c->DDR_tCKE  		= DDR__tck(3);
	c->DDR_tCKESR 		= DDR_SELECT_MAX__tCK_ps(3, DDR__ns(150));
	c->DDR_tXSR  		= DDR_SELECT_MAX__tCK_ps(2, c->DDR_tRFC + DDR__ns(10));
	c->DDR_tXP  		= DDR_SELECT_MAX__tCK_ps(2, 7500);

#ifdef CONFIG_LPDDR2_W97BV6MK_KGD_CONFIG
        struct lpddr2_mr_config *mr_cfg    = &c->kgd_config.mr_config;
        c->kgd_config.use_kgd_config       = CONFIG_LPDDR2_W97BV6MK_KGD_CONFIG;
        mr_cfg->kgd_mr3_ds                 = CONFIG_LPDDR2_W97BV6MK_KGD_MR3_DS;
#endif
#ifdef CONFIG_LPDDR2_W97BV6MK_PHY_DRVODT_CONFIG
        c->phy_drvodt.use_drvodt_config    = CONFIG_LPDDR2_W97BV6MK_PHY_DRVODT_CONFIG;
        c->phy_drvodt.phy_pu_drv_cmd       = CONFIG_LPDDR2_W97BV6MK_PHY_PU_DRV_CMD   ;
        c->phy_drvodt.phy_pd_drv_cmd       = CONFIG_LPDDR2_W97BV6MK_PHY_PD_DRV_CMD   ;
        c->phy_drvodt.phy_pu_drv_ck        = CONFIG_LPDDR2_W97BV6MK_PHY_PU_DRV_CK    ;
        c->phy_drvodt.phy_pd_drv_ck        = CONFIG_LPDDR2_W97BV6MK_PHY_PD_DRV_CK    ;
        c->phy_drvodt.phy_pu_drv_dq7_0     = CONFIG_LPDDR2_W97BV6MK_PHY_PU_DRV_DQ7_0 ;
        c->phy_drvodt.phy_pd_drv_dq7_0     = CONFIG_LPDDR2_W97BV6MK_PHY_PD_DRV_DQ7_0 ;
        c->phy_drvodt.phy_pu_drv_dq15_8    = CONFIG_LPDDR2_W97BV6MK_PHY_PU_DRV_DQ15_8;
        c->phy_drvodt.phy_pd_drv_dq15_8    = CONFIG_LPDDR2_W97BV6MK_PHY_PD_DRV_DQ15_8;
        c->phy_drvodt.phy_pu_odt_dq7_0     = CONFIG_LPDDR2_W97BV6MK_PHY_PU_ODT_DQ7_0 ;
        c->phy_drvodt.phy_pd_odt_dq7_0     = CONFIG_LPDDR2_W97BV6MK_PHY_PD_ODT_DQ7_0 ;
        c->phy_drvodt.phy_pu_odt_dq15_8    = CONFIG_LPDDR2_W97BV6MK_PHY_PU_ODT_DQ15_8;
        c->phy_drvodt.phy_pd_odt_dq15_8    = CONFIG_LPDDR2_W97BV6MK_PHY_PD_ODT_DQ15_8;
#endif
#ifdef CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_CONFIG
        c->phy_deskew.use_deskew_config    = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_CONFIG   ;
        c->phy_deskew.phy_deskew_cmd       = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_CMD      ;
        c->phy_deskew.phy_deskew_rx_dm0    = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DM0   ;
        c->phy_deskew.phy_deskew_tx_dm0    = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DM0   ;
        c->phy_deskew.phy_deskew_rx_dq7_0  = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DQ7_0 ;
        c->phy_deskew.phy_deskew_tx_dq7_0  = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DQ7_0 ;
        c->phy_deskew.phy_deskew_rx_dqs0   = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DQS0  ;
        c->phy_deskew.phy_deskew_tx_dqs0   = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DQS0  ;
        c->phy_deskew.phy_deskew_rx_dm1    = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DM1   ;
        c->phy_deskew.phy_deskew_tx_dm1    = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DM1   ;
        c->phy_deskew.phy_deskew_rx_dq15_8 = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DQ15_8;
        c->phy_deskew.phy_deskew_tx_dq15_8 = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DQ15_8;
        c->phy_deskew.phy_deskew_rx_dqs1   = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_RX_DQS1  ;
        c->phy_deskew.phy_deskew_tx_dqs1   = CONFIG_LPDDR2_W97BV6MK_PHY_DESKEW_TX_DQS1  ;
#endif
}

#define LPDDR2_W97BV6MK {					\
	.name 	= "W97BV6MK",					\
	.id	= DDR_CHIP_ID(VENDOR_WINBOND, TYPE_LPDDR2, MEM_256M),	\
	.type	= LPDDR2,						\
	.freq	= CONFIG_LPDDR2_W97BV6MK_MEM_FREQ,			\
	.size	= 256,						\
	.init	= LPDDR2_W97BV6MK_init,				\
}


#endif
