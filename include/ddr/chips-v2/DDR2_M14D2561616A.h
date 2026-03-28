#ifndef __DDR2_M14D2561616A_CONFIG_H__
#define __DDR2_M14D2561616A_CONFIG_H__

/*
 * CL:7,50M ~ 533M
 *
 * */

#ifndef CONFIG_DDR2_M14D2561616A_MEM_FREQ
#define CONFIG_DDR2_M14D2561616A_MEM_FREQ CONFIG_SYS_MEM_FREQ
#endif

#if (CONFIG_DDR_SEL_PLL == MPLL)
#define CONFIG_SYS_PLL_FREQ CONFIG_SYS_MPLL_FREQ
#else
#define CONFIG_SYS_PLL_FREQ CONFIG_SYS_APLL_FREQ
#endif

#define CONFIG_DDR_DATA_RATE (CONFIG_DDR2_M14D2561616A_MEM_FREQ * 2)

#if((CONFIG_SYS_PLL_FREQ % CONFIG_DDR2_M14D2561616A_MEM_FREQ) ||\
	(CONFIG_SYS_PLL_FREQ / CONFIG_DDR2_M14D2561616A_MEM_FREQ < 0) ||\
	(CONFIG_SYS_PLL_FREQ / CONFIG_DDR2_M14D2561616A_MEM_FREQ > 15))
#error DDR memoryclock division ratio should be an integer between 1 and 16, check CONFIG_SYS_MPLL_FREQ and CONFIG_DDR2_M14D2561616A_MEM_FREQ
#endif

#if ((CONFIG_DDR_DATA_RATE > 100000000) &&\
		(CONFIG_DDR_DATA_RATE <= 1066000000))
#define CONFIG_DDR_CL	7
#else
#define CONFIG_DDR_CL	-1
#endif

#define CONFIG_DDR_AL	0

#if(-1 == CONFIG_DDR_CL)
#error CONFIG_DDR2_M14D2561616A_MEM_FREQ don't support, check data_rate range
#endif


#if !defined(CONFIG_DDR2_M14D2561616A_KGD_CONFIG) && \
        defined(CONFIG_DDR2_KGD_CONFIG)
        #define CONFIG_DDR2_M14D2561616A_KGD_CONFIG            CONFIG_DDR2_KGD_CONFIG
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_KGD_MR0_DLL_RST) && \
        defined(CONFIG_DDR2_KGD_MR0_DLL_RST)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR0_DLL_RST       CONFIG_DDR2_KGD_MR0_DLL_RST
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_KGD_MR0_PD) && \
        defined(CONFIG_DDR2_KGD_MR0_PD)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR0_PD            CONFIG_DDR2_KGD_MR0_PD
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_KGD_MR1_DLL_EN) && \
        defined(CONFIG_DDR2_KGD_MR1_DLL_EN)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR1_DLL_EN        CONFIG_DDR2_KGD_MR1_DLL_EN
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_KGD_MR1_DIC) && \
        defined(CONFIG_DDR2_KGD_MR1_DIC)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR1_DIC           CONFIG_DDR2_KGD_MR1_DIC
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_KGD_MR1_RTT_NOM) && \
        defined(CONFIG_DDR2_KGD_MR1_RTT_NOM)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR1_RTT_NOM       CONFIG_DDR2_KGD_MR1_RTT_NOM
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_KGD_MR1_OCD) && \
        defined(CONFIG_DDR2_KGD_MR1_OCD)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR1_OCD           CONFIG_DDR2_KGD_MR1_OCD
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_KGD_MR2_DCC_EN) && \
        defined(CONFIG_DDR2_KGD_MR2_DCC_EN)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR2_DCC_EN        CONFIG_DDR2_KGD_MR2_DCC_EN
#endif

#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DRVODT_CONFIG) && \
        defined(CONFIG_PHY_DRVODT_CONFIG)
        #define CONFIG_DDR2_M14D2561616A_PHY_DRVODT_CONFIG     CONFIG_PHY_DRVODT_CONFIG
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_PU_DRV_CMD) && \
        defined(CONFIG_PHY_PU_DRV_CMD)
        #define CONFIG_DDR2_M14D2561616A_PHY_PU_DRV_CMD        CONFIG_PHY_PU_DRV_CMD
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_PD_DRV_CMD) && \
        defined(CONFIG_PHY_PD_DRV_CMD)
        #define CONFIG_DDR2_M14D2561616A_PHY_PD_DRV_CMD        CONFIG_PHY_PD_DRV_CMD
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_PU_DRV_CK) && \
        defined(CONFIG_PHY_PU_DRV_CK)
        #define CONFIG_DDR2_M14D2561616A_PHY_PU_DRV_CK         CONFIG_PHY_PU_DRV_CK
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_PD_DRV_CK) && \
        defined(CONFIG_PHY_PD_DRV_CK)
        #define CONFIG_DDR2_M14D2561616A_PHY_PD_DRV_CK         CONFIG_PHY_PD_DRV_CK
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_PU_DRV_DQ7_0) && \
        defined(CONFIG_PHY_PU_DRV_DQ7_0)
        #define CONFIG_DDR2_M14D2561616A_PHY_PU_DRV_DQ7_0      CONFIG_PHY_PU_DRV_DQ7_0
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_PD_DRV_DQ7_0) && \
        defined(CONFIG_PHY_PD_DRV_DQ7_0)
        #define CONFIG_DDR2_M14D2561616A_PHY_PD_DRV_DQ7_0      CONFIG_PHY_PD_DRV_DQ7_0
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_PU_DRV_DQ15_8) && \
        defined(CONFIG_PHY_PU_DRV_DQ15_8)
        #define CONFIG_DDR2_M14D2561616A_PHY_PU_DRV_DQ15_8     CONFIG_PHY_PU_DRV_DQ15_8
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_PD_DRV_DQ15_8) && \
        defined(CONFIG_PHY_PD_DRV_DQ15_8)
        #define CONFIG_DDR2_M14D2561616A_PHY_PD_DRV_DQ15_8     CONFIG_PHY_PD_DRV_DQ15_8
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_PU_ODT_DQ7_0) && \
        defined(CONFIG_PHY_PU_ODT_DQ7_0)
        #define CONFIG_DDR2_M14D2561616A_PHY_PU_ODT_DQ7_0      CONFIG_PHY_PU_ODT_DQ7_0
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_PD_ODT_DQ7_0) && \
        defined(CONFIG_PHY_PD_ODT_DQ7_0)
        #define CONFIG_DDR2_M14D2561616A_PHY_PD_ODT_DQ7_0      CONFIG_PHY_PD_ODT_DQ7_0
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_PU_ODT_DQ15_8) && \
        defined(CONFIG_PHY_PU_ODT_DQ15_8)
        #define CONFIG_DDR2_M14D2561616A_PHY_PU_ODT_DQ15_8     CONFIG_PHY_PU_ODT_DQ15_8
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_PD_ODT_DQ15_8) && \
        defined(CONFIG_PHY_PD_ODT_DQ15_8)
        #define CONFIG_DDR2_M14D2561616A_PHY_PD_ODT_DQ15_8     CONFIG_PHY_PD_ODT_DQ15_8
#endif

#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_CONFIG) && \
        defined(CONFIG_PHY_DESKEW_CONFIG)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_CONFIG     CONFIG_PHY_DESKEW_CONFIG
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_CMD) && \
        defined(CONFIG_PHY_DESKEW_CMD)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_CMD        CONFIG_PHY_DESKEW_CMD
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DM0) && \
        defined(CONFIG_PHY_DESKEW_RX_DM0)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DM0     CONFIG_PHY_DESKEW_RX_DM0
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DM0) && \
        defined(CONFIG_PHY_DESKEW_TX_DM0)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DM0     CONFIG_PHY_DESKEW_TX_DM0
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DQ7_0) && \
        defined(CONFIG_PHY_DESKEW_RX_DQ7_0)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DQ7_0   CONFIG_PHY_DESKEW_RX_DQ7_0
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DQ7_0) && \
        defined(CONFIG_PHY_DESKEW_TX_DQ7_0)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DQ7_0   CONFIG_PHY_DESKEW_TX_DQ7_0
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DQS0) && \
        defined(CONFIG_PHY_DESKEW_RX_DQS0)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DQS0    CONFIG_PHY_DESKEW_RX_DQS0
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DQS0) && \
        defined(CONFIG_PHY_DESKEW_TX_DQS0)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DQS0    CONFIG_PHY_DESKEW_TX_DQS0
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DM1) && \
        defined(CONFIG_PHY_DESKEW_RX_DM1)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DM1     CONFIG_PHY_DESKEW_RX_DM1
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DM1) && \
        defined(CONFIG_PHY_DESKEW_TX_DM1)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DM1     CONFIG_PHY_DESKEW_TX_DM1
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DQ15_8) && \
        defined(CONFIG_PHY_DESKEW_RX_DQ15_8)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DQ15_8  CONFIG_PHY_DESKEW_RX_DQ15_8
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DQ15_8) && \
        defined(CONFIG_PHY_DESKEW_TX_DQ15_8)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DQ15_8  CONFIG_PHY_DESKEW_TX_DQ15_8
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DQS1) && \
        defined(CONFIG_PHY_DESKEW_RX_DQS1)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DQS1    CONFIG_PHY_DESKEW_RX_DQS1
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DQS1) && \
        defined(CONFIG_PHY_DESKEW_TX_DQS1)
        #define CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DQS1    CONFIG_PHY_DESKEW_TX_DQS1
#endif

#if defined(CONFIG_DDR_DLL_RESET_EN) || defined(CONFIG_DDR_DLL_OFF)  || \
        defined(DDR2_CHIP_DRIVER_OUT_STRENGHT) || defined(CONFIG_OPEN_KGD_DRIVER_STRENGTH)
        #define CONFIG_DDR2_M14D2561616A_KGD_CONFIG            0x1

#if defined(CONFIG_DDR_DLL_RESET_EN)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR0_DLL_RST       0x1
#elif !defined(CONFIG_DDR2_M14D2561616A_KGD_MR0_DLL_RST)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR0_DLL_RST       0x1
#endif
#if defined(CONFIG_DDR_DLL_OFF)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR1_DLL_EN        0x1
#elif !defined(CONFIG_DDR2_M14D2561616A_KGD_MR1_DLL_EN)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR1_DLL_EN        0x0
#endif
#if defined(CONFIG_DDR_DRIVER_OUT_STRENGTH)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR1_DIC       \
                ((CONFIG_DDR_DRIVER_OUT_STRENGTH_1 << 1) | CONFIG_DDR_DRIVER_OUT_STRENGTH_0)
#elif defined(DDR2_CHIP_DRIVER_OUT_STRENGTH)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR1_DIC           DDR2_CHIP_DRIVER_OUT_STRENGTH
#elif !defined(CONFIG_DDR2_M14D2561616A_KGD_MR1_DIC)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR1_DIC           0x1
#endif
#if defined(CONFIG_DDR_CHIP_ODT)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR1_RTT_NOM       \
                ((CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_6 << 1) | CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_2)
#elif !defined(CONFIG_DDR2_M14D2561616A_KGD_MR1_RTT_NOM)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR1_RTT_NOM       0x1
#endif

#if !defined(CONFIG_DDR2_M14D2561616A_KGD_MR0_PD)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR0_PD            0x0
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_KGD_MR1_OCD)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR1_OCD           0x0
#endif
#if !defined(CONFIG_DDR2_M14D2561616A_KGD_MR2_DCC_EN)
        #define CONFIG_DDR2_M14D2561616A_KGD_MR2_DCC_EN        0x0
#endif

#endif

static inline void DDR2_M14D2561616A_init(void *data)
{
	struct ddr_chip_info *c = (struct ddr_chip_info *)data;
	unsigned int RL = CONFIG_DDR_CL + CONFIG_DDR_AL;

	c->DDR_ROW = 13;
	c->DDR_COL = 9;
	c->DDR_ROW1 = 13;
	c->DDR_COL1 = 9;

	c->DDR_BANK8 = 0;
	c->DDR_CL = CONFIG_DDR_CL;

	c->DDR_tRAS = DDR__ns(45);
	c->DDR_tRTP = DDR__ns(8);
	c->DDR_tRP = DDR__ps(13125);
	c->DDR_tRCD = DDR__ps(13125);
	c->DDR_tRC = DDR__ps(58125);
	c->DDR_tRRD = DDR__ns(8);
	c->DDR_tWR = DDR__ns(15);
	c->DDR_tWTR = DDR__ps(7500);
	c->DDR_tRFC = DDR__ns(75);
	c->DDR_tMINSR = DDR__ns(60);
	c->DDR_tXP = DDR__tck(3);
	c->DDR_tMRD = DDR__tck(2);

	c->DDR_BL = 8;
	c->DDR_RL = DDR__tck(RL);
	c->DDR_WL = DDR__tck(RL - 1);
	c->DDR_tCCD = DDR__tck(2);
	c->DDR_tRTW = (((c->DDR_BL > 4) ? 6 : 4) + 1);
	c->DDR_tFAW = DDR__ns(45);
	c->DDR_tCKE = DDR__tck(3);
	c->DDR_tCKESR = DDR__tck(3);
	c->DDR_tRDLAT = DDR__tck(7);
	c->DDR_tWDLAT = DDR__tck(3);
	c->DDR_tXARD = DDR__tck(3);
	c->DDR_tXARDS = DDR__tck(10);

	c->DDR_tXSNR = (c->DDR_tRFC + DDR__ns(10));
	c->DDR_tXSRD = DDR__tck(200);
	c->DDR_tCKSRE = DDR__ns(10000);
	c->DDR_tREFI = DDR__ns(3900);

	c->DDR_CLK_DIV = 1;

#ifdef CONFIG_DDR2_M14D2561616A_KGD_CONFIG
        struct ddr2_mr_config *mr_cfg      = &c->kgd_config.mr_config;
        c->kgd_config.use_kgd_config       = CONFIG_DDR2_M14D2561616A_KGD_CONFIG     ;
        mr_cfg->kgd_mr0_dll_rst            = CONFIG_DDR2_M14D2561616A_KGD_MR0_DLL_RST;
        mr_cfg->kgd_mr0_pd                 = CONFIG_DDR2_M14D2561616A_KGD_MR0_PD     ;
        mr_cfg->kgd_mr1_dll_en             = CONFIG_DDR2_M14D2561616A_KGD_MR1_DLL_EN ;
        mr_cfg->kgd_mr1_dic                = CONFIG_DDR2_M14D2561616A_KGD_MR1_DIC    ;
        mr_cfg->kgd_mr1_rtt_nom            = CONFIG_DDR2_M14D2561616A_KGD_MR1_RTT_NOM;
        mr_cfg->kgd_mr1_ocd                = CONFIG_DDR2_M14D2561616A_KGD_MR1_OCD    ;
        mr_cfg->kgd_mr2_dcc_en             = CONFIG_DDR2_M14D2561616A_KGD_MR2_DCC_EN ;
#endif
#ifdef CONFIG_DDR2_M14D2561616A_PHY_DRVODT_CONFIG
        c->phy_drvodt.use_drvodt_config    = CONFIG_DDR2_M14D2561616A_PHY_DRVODT_CONFIG;
        c->phy_drvodt.phy_pu_drv_cmd       = CONFIG_DDR2_M14D2561616A_PHY_PU_DRV_CMD   ;
        c->phy_drvodt.phy_pd_drv_cmd       = CONFIG_DDR2_M14D2561616A_PHY_PD_DRV_CMD   ;
        c->phy_drvodt.phy_pu_drv_ck        = CONFIG_DDR2_M14D2561616A_PHY_PU_DRV_CK    ;
        c->phy_drvodt.phy_pd_drv_ck        = CONFIG_DDR2_M14D2561616A_PHY_PD_DRV_CK    ;
        c->phy_drvodt.phy_pu_drv_dq7_0     = CONFIG_DDR2_M14D2561616A_PHY_PU_DRV_DQ7_0 ;
        c->phy_drvodt.phy_pd_drv_dq7_0     = CONFIG_DDR2_M14D2561616A_PHY_PD_DRV_DQ7_0 ;
        c->phy_drvodt.phy_pu_drv_dq15_8    = CONFIG_DDR2_M14D2561616A_PHY_PU_DRV_DQ15_8;
        c->phy_drvodt.phy_pd_drv_dq15_8    = CONFIG_DDR2_M14D2561616A_PHY_PD_DRV_DQ15_8;
        c->phy_drvodt.phy_pu_odt_dq7_0     = CONFIG_DDR2_M14D2561616A_PHY_PU_ODT_DQ7_0 ;
        c->phy_drvodt.phy_pd_odt_dq7_0     = CONFIG_DDR2_M14D2561616A_PHY_PD_ODT_DQ7_0 ;
        c->phy_drvodt.phy_pu_odt_dq15_8    = CONFIG_DDR2_M14D2561616A_PHY_PU_ODT_DQ15_8;
        c->phy_drvodt.phy_pd_odt_dq15_8    = CONFIG_DDR2_M14D2561616A_PHY_PD_ODT_DQ15_8;
#endif
#ifdef CONFIG_DDR2_M14D2561616A_PHY_DESKEW_CONFIG
        c->phy_deskew.use_deskew_config    = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_CONFIG   ;
        c->phy_deskew.phy_deskew_cmd       = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_CMD      ;
        c->phy_deskew.phy_deskew_rx_dm0    = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DM0   ;
        c->phy_deskew.phy_deskew_tx_dm0    = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DM0   ;
        c->phy_deskew.phy_deskew_rx_dq7_0  = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DQ7_0 ;
        c->phy_deskew.phy_deskew_tx_dq7_0  = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DQ7_0 ;
        c->phy_deskew.phy_deskew_rx_dqs0   = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DQS0  ;
        c->phy_deskew.phy_deskew_tx_dqs0   = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DQS0  ;
        c->phy_deskew.phy_deskew_rx_dm1    = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DM1   ;
        c->phy_deskew.phy_deskew_tx_dm1    = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DM1   ;
        c->phy_deskew.phy_deskew_rx_dq15_8 = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DQ15_8;
        c->phy_deskew.phy_deskew_tx_dq15_8 = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DQ15_8;
        c->phy_deskew.phy_deskew_rx_dqs1   = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_RX_DQS1  ;
        c->phy_deskew.phy_deskew_tx_dqs1   = CONFIG_DDR2_M14D2561616A_PHY_DESKEW_TX_DQS1  ;
#endif
}


#define DDR2_M14D2561616A {					\
	.name = "M14D2561616A",					\
	.id = DDR_CHIP_ID(VENDOR_ESMT, TYPE_DDR2, MEM_32M),	\
	.type = DDR2,					\
	.freq = CONFIG_DDR2_M14D2561616A_MEM_FREQ,	\
	.size = 32,					\
	.init = DDR2_M14D2561616A_init,			\
}
#endif /* __DDR2_CONFIG_H */
