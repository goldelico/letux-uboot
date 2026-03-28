#ifndef __OTG_DWC2_H
#define __OTG_DWC2_H

#define BIT0	(1 << 0)
#define BIT1	(1 << 1)
#define BIT2	(1 << 2)
#define BIT3	(1 << 3)
#define BIT4	(1 << 4)
#define BIT5	(1 << 5)
#define BIT6	(1 << 6)
#define BIT7	(1 << 7)
#define BIT8	(1 << 8)
#define BIT9	(1 << 9)
#define BIT10	(1 << 10)
#define BIT11	(1 << 11)
#define BIT12	(1 << 12)
#define BIT13	(1 << 13)
#define BIT14	(1 << 14)
#define BIT15	(1 << 15)
#define BIT16	(1 << 16)
#define BIT17	(1 << 17)
#define BIT18	(1 << 18)
#define BIT19	(1 << 19)
#define BIT20	(1 << 20)
#define BIT21	(1 << 21)
#define BIT22	(1 << 22)
#define BIT23	(1 << 23)
#define BIT24	(1 << 24)
#define BIT25	(1 << 25)
#define BIT26	(1 << 26)
#define BIT27	(1 << 27)
#define BIT28	(1 << 28)
#define BIT29	(1 << 29)
#define BIT30	(1 << 30)
#define BIT31	(1 << 31)

#define OTG_BASE        0xb3500000
#define USB_PHY_BASE    0xb0078000

#define PHY_STP_ADJ_LOW3BIT	(0x4)
#define PHY_STP_ADJ_HIGH1BIT	(0x8)
#define PHY_ODT_STRENGTH_ADJ	(0x18)
#define PHY_EYE_ADJ		(0x30)

#define PHY_STP_ADJ_125MV_HIGH3BIT	(0x0)
#define PHY_STP_ADJ_125MV_LOW1BIT	(0x1)

/* Globle Regs define */
#define GOTG_CTL_OFF		(0x00)
#define GOTG_INTR_OFF		(0x04)
#define GAHB_CFG_OFF		(0x08)
#define GUSB_CFG_OFF		(0x0c)
#define GRST_CTL_OFF		(0x10)
#define GINT_STS_OFF		(0x14)
#define GINT_MASK_OFF		(0x18)
#define GRXSTS_READ_OFF		(0x1c)
#define GRXSTS_POP_OFF		(0x20)
#define GRXFIFO_SIZE_OFF	(0x24)
#define GNPTXFIFO_SIZE_OFF	(0x28)
#define GDTXFIFO_SIZE_OFF	(0x104)
#define GHW_CFG1_OFF		(0x44)
#define GHW_CFG2_OFF		(0x48)
#define GHW_CFG3_OFF		(0x4c)
#define GHW_CFG4_OFF		(0x50)
#define GDFIFO_CFG_OFF		(0x5c)
#define DIEPTXF_OFF(n)		(0x104 + ((n) - 1) * 0x04)

/* Device Regs define */
#define EP_FIFO_OFF(n)      ((n+1)*0x1000)

#define OTG_DCFG_OFF		(0x800)
#define OTG_DCTL_OFF		(0x804)
#define OTG_DSTS_OFF		(0x808)
#define DIEP_MASK_OFF		(0x810)
#define DOEP_MASK_OFF		(0x814)
#define OTG_DAINT_OFF		(0x818)
#define DAINT_MASK_OFF		(0x81c)
#define DIEP_EMPMSK_OFF		(0x834)

#define DIEP_CTL_OFF(n)		((0x900 + (n)*0x20))
#define DOEP_CTL_OFF(n)		((0xb00 + (n)*0x20))

#define DIEP_INT_OFF(n)		((0x908 + (n)*0x20))
#define DOEP_INT_OFF(n)		((0xb08 + (n)*0x20))

#define DIEP_SIZE_OFF(n)	((0x910 + (n)*0x20))
#define DOEP_SIZE_OFF(n)	((0xb10 + (n)*0x20))

#define DIEP_TXFSTS_OFF(n)	((0x918 + (n)*0x20))

#define usb_readl(o)		readl(OTG_BASE + (o))
#define usb_writel(b, o)	writel(b, OTG_BASE + (o))

#define usb_phy_readb(o)        readb(USB_PHY_BASE + (o))
#define usb_phy_writeb(b, o)    writeb(b, USB_PHY_BASE + (o))

/* Regs macro define */
/*************************************************/
/* GOTGINTR*/
#define GOTGINT_SESSION_END BIT2

/* GAHBCFG */
#define AHBCFG_TX_FIFO_EMT_LEVEL	BIT7
#define AHBCFG_DMA_ENA			BIT5
#define AHBCFG_GLOBLE_INTRMASK		BIT0

/* GUSBCFG */
#define USBCFG_FORCE_DEVICE	BIT31
#define USBCFG_TRDTIME_MASK	(0xf << 10)
#define USBCFG_TRDTIME(n)	(((n) << 10) & USBCFG_TRDTIME_MASK)
#define USBCFG_HNP_EN		BIT9
#define USBCFG_SRP_EN		BIT8
#define USBCFG_PHY_SEL_USB1	BIT6
#define USBCFG_PHY_INF_UPLI	BIT4
#define USBCFG_16BIT_PHY	BIT3
#define USBCFG_TOUT_CAL_MASK (0x7)

/* GRSTCTL */
#define RSTCTL_AHB_IDLE		BIT31
#define RSTCTL_TXFNUM_ALL	(0x10 << 6)
#define RSTCTL_TXFNUM_MASK	(0x1f << 6)
#define RSTCTL_TXFIFO_FLUSH	BIT5
#define RSTCTL_RXFIFO_FLUSH	BIT4
#define RSTCTL_INTK_FLUSH	BIT3
#define RSTCTL_FRMCNT_RST	BIT2
#define RSTCTL_CORE_RST		BIT0

/* GINTMSK */
#define GINTMSK_RXFVL		BIT4
#define GINTMSK_OTG		BIT2
#define GINTMSK_MIS_MODE	BIT1

/* GINTSTS*/
#define GINTSTS_RSUME_DETE	BIT31
#define GINTSTS_CONID_STSCHG	BIT28
#define GINTSTS_RESET_DETE	BIT23
#define GINTSTS_FETCH_SUSPEND	BIT22
#define GINTSTS_OEP_INTR	BIT19
#define GINTSTS_IEP_INTR	BIT18
#define GINTSTS_EP_MISMATCH	BIT17
#define GINTSTS_ENUM_DONE	BIT13
#define GINTSTS_USB_RESET	BIT12
#define GINTSTS_USB_SUSPEND	BIT11
#define GINTSTS_USB_EARLYSUSPEND	BIT10
#define GINTSTS_I2C_INT		BIT9
#define GINTSTS_ULPK_CKINT	BIT8
#define GINTSTS_GOUTNAK_EFF	BIT7
#define GINTSTS_GINNAK_EFF	BIT6
#define GINTSTS_NPTXFIFO_EMPTY	BIT5
#define GINTSTS_RXFIFO_NEMPTY	BIT4
#define GINTSTS_START_FRAM	BIT3
#define GINTSTS_OTG_INTR	BIT2
#define GINTSTS_MODE_MISMATCH	BIT1

/* DCTL */
#define DCTL_NAK_ON_BBLE	BIT16
#define DCTL_CLR_GOUTNAK    BIT10
#define DCTL_SET_GOUTNAK    BIT9
#define DCTL_CLR_GNPINNAK   BIT8
#define DCTL_SET_GNPINNAK   BIT7
#define DCTL_SOFT_DISCONN   BIT1

/* DCFG */
#define DCFG_DEV_ADDR_MASK	(0x7f << 4)
#define DCFG_DEV_ADDR_BIT	4
#define DCFG_HANDSHAKE_STALL_ERR_STATUS	BIT2

/* DAINT_MSK*/
#define DAINT_IEP_MASK(n)		(0x1 << ((n) & 0xffff))
#define DAINT_OEP_MASK(n)		(0x10000 << ((n) & 0xffff))

/* DSTS */
#define DSTS_ERRATIC_ERROR			BIT3
#define DSTS_ENUM_SPEED_MASK		(0x3 << 1)
#define DSTS_ENUM_SPEED_BIT			BIT1
#define DSTS_ENUM_SPEED_HIGH		(0x0 << 1)
#define DSTS_ENUM_SPEED_FULL_30OR60	(0x1 << 1)
#define DSTS_ENUM_SPEED_LOW			(0x2 << 1)
#define DSTS_ENUM_SPEED_FULL_48		(0x3 << 1)

/* GRXSTSR/GRXSTSP */
#define GRXSTSP_PKSTS_MASK		(0xf << 17)
#define GRXSTSP_PKSTS_GOUT_NAK		(0x1 << 17)
#define GRXSTSP_PKSTS_GOUT_RECV		(0x2 << 17)
#define GRXSTSP_PKSTS_TX_COMP		(0x3 << 17)
#define GRXSTSP_PKSTS_SETUP_COMP	(0x4 << 17)
#define GRXSTSP_PKSTS_SETUP_RECV	(0x6 << 17)
#define GRXSTSP_BYTE_CNT_MASK		(0x7ff << 4)
#define GRXSTSP_BYTE_CNT_BIT		4
#define GRXSTSP_EPNUM_MASK		(0xf)
#define GRXSTSP_EPNUM_BIT		BIT0


/* DIOEPCTL */
// ep0
#define DEP_EP0_MAXPKET_SIZE	64
#define DEP_EP0_MPS_64		(0x0)
#define DEP_EP0_MPS_32		(0x1)
#define DEP_EP0_MPS_16		(0x2)
#define DEP_EP0_MPS_8		(0x3)

#define DEP_ENA_BIT			BIT31
#define DEP_DISENA_BIT		BIT30
#define DEP_RESET_DATA0		BIT28
#define DEP_SET_NAK			BIT27
#define DEP_CLEAR_NAK		BIT26
#define DEP_IN_FIFO_SEL(n)	(((n) & 0xf) << 22)
#define DEP_SET_STALL		BIT21
#define DEP_TYPE_MASK		(0x3 << 18)
#define DEP_TYPE_CNTL		(0x0 << 18)
#define DEP_TYPE_ISO		(0x1 << 18
#define DEP_TYPE_BULK		(0x2 << 18)
#define DEP_TYPE_INTR		(0x3 << 18)
#define DEP_TYPE(x)		(((x)&0x3) << 18)
#define USB_ACTIVE_EP		BIT15
#define DEP_PKTSIZE_MASK	0x7ff
#define DEP_FS_PKTSIZE		64
#define DEP_HS_PKTSIZE		512

/* DIOEPINT */
#define DEP_SETUP_RECV		BIT15
#define DEP_NYET_INT		BIT14
#define DEP_NAK_INT		BIT13
#define DEP_BABBLE_ERR_INT	BIT12
#define DEP_PKT_DROP_STATUS	BIT11
#define DEP_BNA_INT		BIT9
#define DEP_TXFIFO_UNDRN	BIT8		// Only for INEP
#define DEP_OUTPKT_ERR		BIT8		// Only for OUTEP
#define DEP_TXFIFO_EMPTY	BIT7
#define DEP_INEP_NAKEFF		BIT6		// Only for INEP
#define DEP_B2B_SETUP_RECV	BIT6		// Only for OUTEP0
#define DEP_INTOKEN_EPMISATCH	BIT5		// Only for INEP
#define DEP_SETUP_OUT_DATA_DONE BIT5
#define DEP_INTOKEN_RECV_TXFIFO_EMPTY	BIT4	// Only for INEP
#define DEP_OUTTOKEN_RECV_EPDIS	BIT4		// Only for OUTEP
#define DEP_TIME_OUT		BIT3		// Only for INEP
#define DEP_SETUP_PHASE_DONE	BIT3		// Only for OUTEP0
#define DEP_AHB_ERR		BIT2
#define DEP_EPDIS_INT		BIT1
#define DEP_XFER_COMP		BIT0		// Used by INEP and OUTEP

/* DOEPSIZ0 */
#define DOEPSIZE0_SUPCNT_1	(0x1 << 29)
#define DOEPSIZE0_SUPCNT_2	(0x2 << 29)
#define DOEPSIZE0_SUPCNT_3	(0x3 << 29)
#define DOEPSIZE0_PKTCNT_BIT	BIT19

/* DIEPSIZ0 */
#define DIEPSIZE0_PKTCNT_BIT	BIT19

/* DI/OEPSIZn */
#define DXEPSIZE_PKTCNT		19


/*RX for all rx endpoint*/
#define DEP_RXFIFO_SIZE		1047	/* (4*1 +6) + (7 + 1)*(512/4 + 1) + 2*2 + 1 fix*/
/*tx 0 for control endpoint*/
#define DEP_NPTXFIFO_SIZE	128	/* (7 + 1)*(64/4) fix*/
/*tx 1 for bulk	endpoint */
#define DEP_DTXFIFO1_SIZE	1024	/* (7 + 1)*(512/4) fix*/

/*cpm about usb phy */

/* OTG parameter control register(USBPCR) */
#define USBPCR_USB_MODE         BIT31
#define USBPCR_AVLD_REG         BIT30
#define USBPCR_UTMIRST          BIT20
#define USBPCR_POR              BIT22
#define USBPCR_IDPULLUP_LSB     28   /* IDPULLUP_MASK bit */
#define USBPCR_IDPULLUP_MASK    BITS_H2L(29, USBPCR_USBPCR_IDPULLUP_LSB)

/*Soft Reset and Bus Control Register (SRBC)*/
#define SRBC_OTGSOFTRST		BIT12

/* OTG reset detect timer register(USBRDT) */
#define USBRDT_VBFIL_LD_EN      BIT25
#define USBRDT_IDDIG_EN         BIT24
#define USBRDT_IDDIG_REG        BIT23

#define USBRDT_USBRDT_LSB       0
#define USBRDT_USBRDT_MASK      BITS_H2L(22, USBRDT_USBRDT_LSB)

/* Oscillator and power control register(OPCR) */
#define OPCR_OTGPHY_ENABLE      BIT7    /* SPENDN bit */

#endif
