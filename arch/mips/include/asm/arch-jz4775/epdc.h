/*
 *
 * JZ4775 EPD register definition.
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 */

#ifndef __CHIP_EPD_H__
#define __CHIP_EPD_H__

#define REG32(addr)	*((volatile unsigned int *)(addr))

#define	EPD_BASE	0xb30c0000
/*************************************************************************
 * EPDC:EPD Controller
 *************************************************************************/

/* EPD Register Address*/
#define EPD_CTRL	(EPD_BASE + 0x0)
#define EPD_CFG		(EPD_BASE + 0x4)
#define EPD_STA		(EPD_BASE + 0x8)
#define EPD_ISRC	(EPD_BASE + 0xc)
#define EPD_DIS0	(EPD_BASE + 0x10)
#define EPD_DIS1	(EPD_BASE + 0x14)
#define EPD_SIZE	(EPD_BASE + 0x18)
#define EPD_VAT		(EPD_BASE + 0x20)
#define EPD_DAV		(EPD_BASE + 0x24)
#define EPD_DAH		(EPD_BASE + 0x28)
#define EPD_VSYNC	(EPD_BASE + 0x2c)
#define EPD_HSYNC	(EPD_BASE + 0x30)
#define EPD_GDCLK	(EPD_BASE + 0x34)
#define EPD_GDOE	(EPD_BASE + 0x38)
#define EPD_GDSP	(EPD_BASE + 0x3c)
#define EPD_SDOE	(EPD_BASE + 0x40)
#define EPD_SDSP	(EPD_BASE + 0x44)
#define EPD_PMGR0	(EPD_BASE + 0x50)
#define EPD_PMGR1	(EPD_BASE + 0x54)
#define EPD_PMGR2	(EPD_BASE + 0x58)
#define EPD_PMGR3	(EPD_BASE + 0x5c)
#define EPD_PMGR4	(EPD_BASE + 0x60)
#define EPD_LUTBF	(EPD_BASE + 0x70)
#define EPD_LUTSIZE	(EPD_BASE + 0x74)
#define EPD_CURBF	(EPD_BASE + 0x80)
#define EPD_CURSIZE	(EPD_BASE + 0x84)
#define EPD_WRK0BF	(EPD_BASE + 0x90)
#define EPD_WRK0SIZE	(EPD_BASE + 0x94)
#define EPD_WRK1BF	(EPD_BASE + 0x98)
#define EPD_WRK1SIZE	(EPD_BASE + 0x9c)
#define EPD_VCOMBD0	(EPD_BASE + 0x100)
#define EPD_VCOMBD1	(EPD_BASE + 0x104)
#define EPD_VCOMBD2	(EPD_BASE + 0x108)
#define EPD_VCOMBD3	(EPD_BASE + 0x10c)
#define EPD_VCOMBD4	(EPD_BASE + 0x110)
#define EPD_VCOMBD5	(EPD_BASE + 0x114)
#define EPD_VCOMBD6	(EPD_BASE + 0x118)
#define EPD_VCOMBD7	(EPD_BASE + 0x11c)
#define EPD_VCOMBD8	(EPD_BASE + 0x120)
#define EPD_VCOMBD9	(EPD_BASE + 0x124)
#define EPD_VCOMBD10	(EPD_BASE + 0x128)
#define EPD_VCOMBD11	(EPD_BASE + 0x12c)
#define EPD_VCOMBD12	(EPD_BASE + 0x130)
#define EPD_VCOMBD13	(EPD_BASE + 0x134)
#define EPD_VCOMBD14	(EPD_BASE + 0x138)
#define EPD_VCOMBD15	(EPD_BASE + 0x13c)
#define EPD_PRI		(EPD_BASE + 0x410)
#define DDR_PRI		0xb3010128
#define DDR_CONFLICT	0xb3010104
#define DDR_SNP_HIGH	0xb30103c0
#define DDR_SNP_LOW	0xb30103c4


/* EPD Register */
#define REG_EPD_CTRL		REG32(EPD_CTRL)
#define REG_EPD_CFG		REG32(EPD_CFG)
#define REG_EPD_STA		REG32(EPD_STA)
#define REG_EPD_ISRC		REG32(EPD_ISRC)
#define REG_EPD_DIS0		REG32(EPD_DIS0)
#define REG_EPD_DIS1		REG32(EPD_DIS1)
#define REG_EPD_SIZE		REG32(EPD_SIZE)
#define REG_EPD_VAT		REG32(EPD_VAT)
#define REG_EPD_DAV		REG32(EPD_DAV)
#define REG_EPD_DAH		REG32(EPD_DAH)
#define REG_EPD_VSYNC		REG32(EPD_VSYNC)
#define REG_EPD_HSYNC		REG32(EPD_HSYNC)
#define REG_EPD_GDCLK		REG32(EPD_GDCLK)
#define REG_EPD_GDOE		REG32(EPD_GDOE)
#define REG_EPD_GDSP		REG32(EPD_GDSP)
#define REG_EPD_SDOE		REG32(EPD_SDOE)
#define REG_EPD_SDSP		REG32(EPD_SDSP)
#define REG_EPD_PMGR0		REG32(EPD_PMGR0)
#define REG_EPD_PMGR1		REG32(EPD_PMGR1)
#define REG_EPD_PMGR2		REG32(EPD_PMGR2)
#define REG_EPD_PMGR3		REG32(EPD_PMGR3)
#define REG_EPD_PMGR4		REG32(EPD_PMGR4)
#define REG_EPD_LUTBF		REG32(EPD_LUTBF)
#define REG_EPD_LUTSIZE		REG32(EPD_LUTSIZE)
#define REG_EPD_CURBF		REG32(EPD_CURBF)
#define REG_EPD_CURSIZE		REG32(EPD_CURSIZE)
#define REG_EPD_WRK0BF		REG32(EPD_WRK0BF)
#define REG_EPD_WRK0SIZE	REG32(EPD_WRK0SIZE)
#define REG_EPD_WRK1BF		REG32(EPD_WRK1BF)
#define REG_EPD_WRK1SIZE	REG32(EPD_WRK1SIZE)
#define REG_EPD_VCOMBD0		REG32(EPD_VCOMBD0)
#define REG_EPD_VCOMBD1		REG32(EPD_VCOMBD1)
#define REG_EPD_VCOMBD2		REG32(EPD_VCOMBD2)
#define REG_EPD_VCOMBD3		REG32(EPD_VCOMBD3)
#define REG_EPD_VCOMBD4		REG32(EPD_VCOMBD4)
#define REG_EPD_VCOMBD5		REG32(EPD_VCOMBD5)
#define REG_EPD_VCOMBD6		REG32(EPD_VCOMBD6)
#define REG_EPD_VCOMBD7		REG32(EPD_VCOMBD7)
#define REG_EPD_VCOMBD8		REG32(EPD_VCOMBD8)
#define REG_EPD_VCOMBD9		REG32(EPD_VCOMBD9)
#define REG_EPD_VCOMBD10	REG32(EPD_VCOMBD10)
#define REG_EPD_VCOMBD11	REG32(EPD_VCOMBD11)
#define REG_EPD_VCOMBD12	REG32(EPD_VCOMBD12)
#define REG_EPD_VCOMBD13	REG32(EPD_VCOMBD13)
#define REG_EPD_VCOMBD14	REG32(EPD_VCOMBD14)
#define REG_EPD_VCOMBD15	REG32(EPD_VCOMBD15)
#define REG_EPD_PRI		REG32(EPD_PRI)
#define REG_DDR_PRI		REG32(DDR_PRI)
#define REG_DDR_CONFLICT	REG32(DDR_CONFLICT)
#define REG_DDR_SNP_HIGH	REG32(DDR_SNP_HIGH)
#define REG_DDR_SNP_LOW		REG32(DDR_SNP_LOW)


/* EPD Register Bits Description*/
#define EPD_CTRL_BDR_STRT	(1 << 7)
#define EPD_CTRL_PWROFF		(1 << 5)
#define EPD_CTRL_PWRON		(1 << 4)
#define EPD_CTRL_REF_STOP	(1 << 3)
#define EPD_CTRL_REF_STRT	(1 << 2)
#define EPD_CTRL_LUT_STRT	(1 << 1)
#define EPD_CTRL_EPD_ENA	(1 << 0)

#define EPD_CFG_STEP2		21
#define EPD_CFG_STEP1		16
#define EPD_CFG_STEP		8
#define EPD_CFG_LUT_1MODE	(0 << 5)
#define EPD_CFG_LUT_2MODE	(1 << 5)
#define EPD_CFG_LUT_3MODE	(2 << 5)
#define EPD_CFG_AUTO_GATED	(1 << 4)
#define EPD_CFG_AUTO_STOP_ENA	(1 << 3)
#define EPD_CFG_IBPP_4BITS	(0 << 1)
#define EPD_CFG_IBPP_3BITS	(1 << 1)
#define EPD_CFG_IBPP_2BITS	(2 << 1)
#define EPD_CFG_PARTIAL		(1 << 0)
#define EPD_CFG_PARALLEL	(0 << 0)

#define EPD_STA_EPD_IDLE	(1 << 31)
#define EPD_STA_PANEL_IDLE	(1 << 30)
#define EPD_STA_BDR_IDLE	(1 << 29)
#define EPD_STA_REF_IDLE	(1 << 28)
#define EPD_STA_LUT_IDLE	(1 << 27)
#define EPD_STA_IFF2U		(1 << 13)
#define EPD_STA_IFF1U		(1 << 12)
#define EPD_STA_IFF0U		(1 << 11)
#define EPD_STA_WFF1O		(1 << 10)
#define EPD_STA_WFF0O		(1 << 9)
#define EPD_STA_OFFU		(1 << 8)
#define EPD_STA_BDR_DONE	(1 << 7)
#define EPD_STA_PWROFF		(1 << 5)
#define EPD_STA_PWRON		(1 << 4)
#define EPD_STA_REF_STOP	(1 << 3)
#define EPD_STA_REF_STRT	(1 << 2)
#define EPD_STA_LUT_DONE	(1 << 1)
#define EPD_STA_FRM_END		(1 << 0)

#define EPD_ISRC_IFF2U_INT_OPEN		(1 << 13)
#define EPD_ISRC_IFF2U_INT_CLOSE	(0 << 13)
#define EPD_ISRC_IFF1U_INT_OPEN		(1 << 12)
#define EPD_ISRC_IFF1U_INT_CLOSE	(0 << 12)
#define EPD_ISRC_IFF0U_INT_OPEN		(1 << 11)
#define EPD_ISRC_IFF0U_INT_CLOSE	(0 << 11)
#define EPD_ISRC_WFF1O_INT_OPEN		(1 << 10)
#define EPD_ISRC_WFF1O_INT_CLOSE	(0 << 10)
#define EPD_ISRC_WFF0O_INT_OPEN		(1 << 9)
#define EPD_ISRC_WFF0O_INT_CLOSE	(0 << 9)
#define EPD_ISRC_OFFU_INT_OPEN		(1 << 8)
#define EPD_ISRC_OFFU_INT_CLOSE		(0 << 8)
#define EPD_ISRC_BDR_DONE_INT_OPEN	(1 << 7)
#define EPD_ISRC_BDR_DONE_INT_CLOSE	(0 << 7)
#define EPD_ISRC_PWROFF_INT_OPEN	(1 << 5)
#define EPD_ISRC_PWROFF_INT_CLOSE	(0 << 5)
#define EPD_ISRC_PWRON_INT_OPEN		(1 << 4)
#define EPD_ISRC_PWRON_INT_CLOSE	(0 << 4)
#define EPD_ISRC_REF_STOP_INT_OPEN	(1 << 3)
#define EPD_ISRC_REF_STOP_INT_CLOSE	(0 << 3)
#define EPD_ISRC_REF_STRT_INT_OPEN	(1 << 2)
#define EPD_ISRC_REF_STRT_INT_CLOSE	(0 << 2)
#define EPD_ISRC_LUT_DONE_INT_OPEN	(1 << 1)
#define EPD_ISRC_LUT_DONE_INT_CLOSE	(0 << 1)
#define EPD_ISRC_FRM_END_INT_OPEN	(1 << 0)
#define EPD_ISRC_FRM_END_INT_CLOSE	(0 << 0)

#define EPD_DIS0_SDSP_CAS	(1 << 26)
#define EPD_DIS0_SDSP_MODE	(1 << 25)
#define EPD_DIS0_GDCLK_MODE	(1 << 24)
#define EPD_DIS0_GDUD		(1 << 21)
#define EPD_DIS0_SDRL		(1 << 20)
#define EPD_DIS0_GDCLK_POL	(1 << 19)
#define EPD_DIS0_GDOE_POL	(1 << 18)
#define EPD_DIS0_GDSP_POL	(1 << 17)
#define EPD_DIS0_SDCLK_POL	(1 << 16)
#define EPD_DIS0_SDOE_POL	(1 << 15)
#define EPD_DIS0_SDSP_POL	(1 << 14)
#define EPD_DIS0_SDCE_POL	(1 << 13)
#define EPD_DIS0_SDLE_POL	(1 << 12)
#define EPD_DIS0_GDSP_CAS	(1 << 9)
#define EPD_DIS0_EPD_OMODE	(1 << 0)

#define EPD_DIS1_SDDO_REV	(1 << 30)
#define EPD_DIS1_PDAT_SWAP	(1 << 29)
#define EPD_DIS1_SDCE_REV	(1 << 28)
#define EPD_DIS1_SDOS		16
#define EPD_DIS1_PDAT		8
#define EPD_DIS1_SDCE_STN	4
#define EPD_DIS1_SDCE_NUM	0

#define EPD_SIZE_VSIZE		16
#define EPD_SIZE_HSIZE		0

#define EPD_VAT_VT		16
#define EPD_VAT_HT		0

#define EPD_DAV_VDE		16
#define EPD_DAV_VDS		0

#define EPD_DAH_HDE		16
#define EPD_DAH_HDS		0

#define EPD_VSYNC_VPE		16
#define EPD_VSYNC_VPS		0

#define EPD_HSYNC_HPE		16
#define EPD_HSYNC_HPS		0

#define EPD_GDCLK_DIS	16
#define EPD_GDCLK_ENA	0

#define EPD_GDOE_DIS	16
#define EPD_GDOE_ENA	0

#define EPD_GDSP_DIS	16
#define EPD_GDSP_ENA	0

#define EPD_SDOE_DIS	16
#define EPD_SDOE_ENA	0

#define EPD_SDSP_DIS	16
#define EPD_SDSP_ENA	0

#define EPD_PMGR0_PWR_DLY12	16
#define EPD_PMGR0_PWR_DLY01	0

#define EPD_PMGR1_PWR_DLY34	16
#define EPD_PMGR1_PWR_DLY23	0

#define EPD_PMGR2_PWR_DLY56	16
#define EPD_PMGR2_PWR_DLY45	0

#define EPD_PMGR3_VCOM_IDLE	30
#define EPD_PMGR3_PWRCOM_POL	29
#define EPD_PMGR3_UNIPOL	28
#define EPD_PMGR3_BDR_ENA	27
#define EPD_PMGR3_BDR_IDLE	24
#define EPD_PMGR3_PWR_POL	16
#define EPD_PMGR3_PWR_DLY67	0

#define EPD_PMGR4_PWR_ENA	16
#define EPD_PMGR4_VCOMBD_STEP	8
#define EPD_PMGR4_PWR_VAL	0

#define EPD_LUT_POS0		(0 << 30)
#define EPD_LUT_POS800		(1 << 30)
#define EPD_LUT_POS1800		(2 << 30)
#define	EPD_LUT_SIZE		0

#define EPD_PRI_TH2		20
#define EPD_PRI_TH1		10
#define EPD_PRI_TH0		0

#endif /* __CHIP_EPD_H__ */

