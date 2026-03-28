#ifndef __X2100_DDR__
#define __X2100_DDR__


#define CONFIG_DDR_INNOPHY
#define CONFIG_DDR_PARAMS_CREATOR
#define CONFIG_DDR_HOST_CC
#define CONFIG_DDR_CS0                      1    /* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1                      0    /* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32                     0    /* 1-32bit-width, 0-16bit-width */

#define CONFIG_DDR_TYPE_LPDDR2

#ifdef CONFIG_DDR_TYPE_LPDDR2

	/*X2100*/
	#define CONFIG_X2100_LPDDR2
        #define CONFIG_X2100_LPDDR2_64M_MEM_FREQ		500000000

	/*X2100L*/
	#define CONFIG_LPDDR2_SCB4BL256160AFL19GI
        #define CONFIG_LPDDR2_SCB4BL256160AFL19GI_MEM_FREQ      500000000

        #define CONFIG_LPDDR2_KGD_CONFIG             0x1
        #define CONFIG_LPDDR2_KGD_MR3_DS             0x2
#endif


#define CONFIG_PHY_DRVODT_CONFIG             0x0
#define CONFIG_PHY_PU_DRV_CMD                0x8
#define CONFIG_PHY_PD_DRV_CMD                0x8
#define CONFIG_PHY_PU_DRV_CK                 0x8
#define CONFIG_PHY_PD_DRV_CK                 0x8
#define CONFIG_PHY_PU_DRV_DQ7_0              0x8
#define CONFIG_PHY_PD_DRV_DQ7_0              0x8
#define CONFIG_PHY_PU_DRV_DQ15_8             0x8
#define CONFIG_PHY_PD_DRV_DQ15_8             0x8
#define CONFIG_PHY_PU_ODT_DQ7_0              0x5
#define CONFIG_PHY_PD_ODT_DQ7_0              0x5
#define CONFIG_PHY_PU_ODT_DQ15_8             0x5
#define CONFIG_PHY_PD_ODT_DQ15_8             0x5

#define CONFIG_PHY_DESKEW_CONFIG             0x0
#define CONFIG_PHY_DESKEW_CMD                0x3
#define CONFIG_PHY_DESKEW_RX_DM0             0x3
#define CONFIG_PHY_DESKEW_TX_DM0             0x3
#define CONFIG_PHY_DESKEW_RX_DQ7_0           0x3
#define CONFIG_PHY_DESKEW_TX_DQ7_0           0x3
#define CONFIG_PHY_DESKEW_RX_DQS0            0x3
#define CONFIG_PHY_DESKEW_TX_DQS0            0x3
#define CONFIG_PHY_DESKEW_RX_DM1             0x3
#define CONFIG_PHY_DESKEW_TX_DM1             0x3
#define CONFIG_PHY_DESKEW_RX_DQ15_8          0x3
#define CONFIG_PHY_DESKEW_TX_DQ15_8          0x3
#define CONFIG_PHY_DESKEW_RX_DQS1            0x3
#define CONFIG_PHY_DESKEW_TX_DQS1            0x3

#define CONFIG_DDR_PHY_IMPEDANCE             40
#define CONFIG_DDR_PHY_ODT_IMPEDANCE         120

#define CONFIG_DDR_AUTO_SELF_REFRESH
#define CONFIG_DDR_AUTO_SELF_REFRESH_CNT     257

#define CONFIG_BOOTARGS_MEM_32M			"mem=32M@0x0"	/* customize bootargs for default env.*/
#define CONFIG_BOOTARGS_MEM_64M			"mem=64M@0x0"	/* customize bootargs for default env.*/
#define CONFIG_BOOTARGS_MEM_128M		"mem=128M@0x0"
#define CONFIG_BOOTARGS_MEM_256M		"mem=256M@0x0"
#define CONFIG_BOOTARGS_MEM_512M		"mem=256M@0x0 mem=256M@0x30000000"

/*
#define CONFIG_DDR_CHIP_ODT
#define CONFIG_DDR_PHY_ODT
#define CONFIG_DDR_PHY_DQ_ODT
#define CONFIG_DDR_PHY_DQS_ODT
#define CONFIG_DDR_PHY_IMPED_PULLUP		0xe
#define CONFIG_DDR_PHY_IMPED_PULLDOWN           0xe
*/

/*
#define CONFIG_DDR_TEST_CPU
#define CONFIG_DDR_TEST
#define CONFIG_DDR_TEST_DATALINE
#define CONFIG_DDR_TEST_ADDRLINE
*/

/*#define CONFIG_DDR_DRVODT_DEBUG*/
/*#define CONFIG_DDRP_SOFTWARE_TRAINING*/

#endif
