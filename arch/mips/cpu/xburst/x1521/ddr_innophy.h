#ifndef T30_PHY
#define T30_PHY
#include <asm/io.h>

#define PHY_BASE						(0xb3011000)
#define APB_BASE						(0xb3012000)
#define AHB_BASE						(0xb34f0000)

/**********          PHY ADDR  ********************/
#define INNO_CHANNEL_EN 0x0
#define INNO_MEM_CFG    0x04
#define INNO_TRAINING_CTRL 0x08
#define INNO_WL_MODE1   0x0C
#define INNO_WL_MODE2   0x10
#define INNO_CL         0x14
#define INNO_AL         0x18

#define INNO_CWL        0x1C
#define INNO_DQ_WIDTH   0x7C

#define INNO_PLL_FBDIV  0x80
#define INNO_PLL_CTRL   0x84
#define INNO_PLL_PDIV   0x88

#define INNO_WL_DONE    0xC0
#define INNO_PLL_LOCK    0xC8
#define INNO_CALIB_DONE 0xCC

#define INNO_INIT_COMP  0xD0

#define DDRP_TRAINING_CTRL_WL_BP    (1 << 3)
#define DDRP_TRAINING_CTRL_WL_START (1 << 2)
#define DDRP_TRAINING_CTRL_DSCSE_BP (1 << 1)
#define DDRP_TRAINING_CTRL_DSACE_START  (1 << 0)


/*
 *  * DDR Innophy registers
 *   * */
#define DDRP_INNOPHY_PHY_RST        (DDR_PHY_OFFSET + 0x000)
#define DDRP_INNOPHY_MEM_CFG        (DDR_PHY_OFFSET + 0x004)
#define DDRP_INNOPHY_DQ_WIDTH       (DDR_PHY_OFFSET + 0x07c)
#define DDRP_INNOPHY_CL         (DDR_PHY_OFFSET + 0x014)
#define DDRP_INNOPHY_CWL        (DDR_PHY_OFFSET + 0x01c)
#define DDRP_INNOPHY_AL         (DDR_PHY_OFFSET + 0x018)
#define DDRP_INNOPHY_PLL_FBDIV      (DDR_PHY_OFFSET + 0x080)
#define DDRP_INNOPHY_PLL_CTRL       (DDR_PHY_OFFSET + 0x084)
#define DDRP_INNOPHY_PLL_PDIV       (DDR_PHY_OFFSET + 0x088)
#define DDRP_INNOPHY_PLL_LOCK       (DDR_PHY_OFFSET + 0xc8)
#define DDRP_INNOPHY_TRAINING_CTRL  (DDR_PHY_OFFSET + 0x008)
#define DDRP_INNOPHY_CALIB_DONE     (DDR_PHY_OFFSET + 0xcc)
#define DDRP_INNOPHY_CALIB_DELAY_AL (DDR_PHY_OFFSET + 0x190)
#define DDRP_INNOPHY_CALIB_DELAY_AH (DDR_PHY_OFFSET + 0x194)
#define DDRP_INNOPHY_CALIB_BYPASS_AL    (DDR_PHY_OFFSET + 0x118)
#define DDRP_INNOPHY_CALIB_BYPASS_AH    (DDR_PHY_OFFSET + 0x158)
#define DDRP_INNOPHY_WL_MODE1       (DDR_PHY_OFFSET + 0x00c)
#define DDRP_INNOPHY_WL_MODE2       (DDR_PHY_OFFSET + 0x010)
#define DDRP_INNOPHY_WL_DONE        (DDR_PHY_OFFSET + 0x0c0)
#define DDRP_INNOPHY_INIT_COMP      (DDR_PHY_OFFSET + 0x0d0)


#define T30_CHANNEL_EN	       (PHY_BASE + 0x0)
#define T30_MEM_CFG            (PHY_BASE + 0x04)
#define T30_TRANING_CTRL       (PHY_BASE + 0x08)
#define T30_WRITE_LEVEL_MODE1  (PHY_BASE + 0x0C)
#define T30_WRITE_LEVEL_MODE2  (PHY_BASE + 0x10)
#define T30_CL                 (PHY_BASE + 0x14)
#define T30_AL                 (PHY_BASE + 0x18)

#define T30_CWL		           (PHY_BASE + 0x1C)
#define T30_DQ_WIDTH           (PHY_BASE + 0x7C)

#define T30_PLL_FBDIV          (PHY_BASE + 0x80)
#define T30_PLL_CTRL           (PHY_BASE + 0x84)
#define T30_PLL_PDIV           (PHY_BASE + 0x88)

#define T30_WL_DONE             (PHY_BASE + 0xc0)
#define T30_DDR_PLL_LOCK             (PHY_BASE + 0xc8)
#define T30_CALIB_DONE          (PHY_BASE + 0xcc)

#define T30_INIT_COMP           (PHY_BASE + 0xd0)
/**************        PHY ADDR  END    ****************/

/**************        PHY 附加寄存器   ****************/
#define T30_DQS_DELAY_L			(PHY_BASE + 0x120)
#define T30_DQS_DELAY_H			(PHY_BASE + 0x160)

//#define T30_REG02				(PHY_BASE + 0x8)
//#define T30_REG08				(PHY_BASE + 0x20)
#define T30_REG46				(PHY_BASE + 0x118)
#define T30_REG56				(PHY_BASE + 0x158)

#define DDR_APB_PHY_INIT				(APB_BASE + 0x8c)

#define REG_DDR_CTRL					(AHB_BASE + 0x008)
#define REG_DDR_CFG					(AHB_BASE + 0x004)
#define REG_DDR_LMR					(AHB_BASE + 0x00c)
/*************       PHY END         *******************/

/***********                  PHY REGISTER  BEGIN                 ****************/
#define  PHY_CHANNEL_EN    (*(volatile unsigned int *)T30_CHANNEL_EN       )
#define  PHY_RST           (*(volatile unsigned int *)T30_PHY_RST          )
#define  PHY_MEM_CFG       (*(volatile unsigned int *)T30_MEM_SEL          )
#define  PHY_TRANING_CTRL  (*(volatile unsigned int *)T30_TRANING_CTRL     )
#define PHY_WL_MODE1       (*(volatile unsigned int *)T30_WRITE_LEVEL_MODE1)
#define PHY_WL_MODE2       (*(volatile unsigned int *)T30_WRITE_LEVEL_MODE2)

#define  PHY_CL            (*(volatile unsigned int *)T30_CL               )
#define  PHY_AL            (*(volatile unsigned int *)T30_AL               )
#define  PHY_CWL            (*(volatile unsigned int *)T30_CWL             )

#define  PHY_DQ_WIDTH      (*(volatile unsigned int *)T30_DQ_WIDTH         )
#define  PHY_PLL_FBDIV 	   (*(volatile unsigned int *)T30_PLL_FBDIV        )
#define  PHY_PLL_CTRL      (*(volatile unsigned int *)T30_PLL_CTRL         )
#define  PHY_PLL_PDIV      (*(volatile unsigned int *)T30_PLL_PDIV         )

#define PHY_WL_DONE        (*(volatile unsigned int *)T30_WL_DONE          )
#define PHY_CALIB_DONE     (*(volatile unsigned int *)T30_CALIB_DONE   )

#define PHY_INIT_COMP      (*(volatile unsigned int *)T30_INIT_COMP   )
/***********                  PHY REGISTER  END                    ****************/


#define phy_writel(value, reg)  writel((value), PHY_BASE+reg)
#define phy_readl(reg)          readl(PHY_BASE+reg)


#endif

