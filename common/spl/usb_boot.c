/*
	USB 启动第二阶段处理代码.
	1. 必须与bootrom 的usbboot相结合。
	2. 初始化部分在bootrom处理。
*/
#include <common.h>
#include <asm/io.h>
#include <usb_boot/otg_dwc2.h>
#include <usb_boot/usb_boot.h>

#define DEBUG(n) 	do {} while(0)
#define DEBUG_HEX(n)	do {} while(0)

__weak void jump_to_entry_point(unsigned long entry_point)
{
        flush_cache_all();
        __asm__ volatile (
                        ".set push              \n\t"
                        ".set noreorder         \n\t"
                        ".set mips32r2          \n\t"
                        "jr.hb %0              \n\t"
                        "nop    \n\t"
                        :
                        :"r"(entry_point));
}

__weak void read_socid(unsigned int *buf)
{
	/* Nothing to do! */
}

static u32 status_map;

enum {
	HIGH_SPEED = 0,
	FULL_SPEED,
	LOW_SPEED
};

enum {
	DWC_DEFAULT = 0,
	DWC_ADDRESS,
	DWC_CONFIG,
};

static int usb_loop = 1;
static int enum_done_speed = HIGH_SPEED;

static  int DEP_EP_MAXPKT_SIZE(int epnum)
{
	if (epnum) {
		if (enum_done_speed == HIGH_SPEED)
			return DEP_HS_PKTSIZE;
		else if (enum_done_speed == FULL_SPEED)
			return DEP_FS_PKTSIZE;
		else
			return 0;
	}
	return 64;
}

static void read_out_packet(USB_STATUS *status, u16 count,u32 epnum)
{
	int dwords = (count+3) / 4;
	int i;

	DEBUG("epnum:");
	DEBUG_HEX(epnum);
	DEBUG("count:");
	DEBUG_HEX(count);
	DEBUG("dst addr:");
	DEBUG_HEX((u32)status->addr_out);
	if (epnum == 1 && status->addr_out != 0 && status->length >= count) {
		for (i = 0; i < dwords; i++)
			*((unsigned int *)(status->addr_out) + i) = usb_readl(EP_FIFO_OFF(1));
		status->addr_out += (dwords*4);
		status->length -= (dwords*4);
	} else {
		/*discard out*/
		DEBUG("discard\n");
		for (i = 0; i < dwords; i++)
			usb_readl(EP_FIFO_OFF(epnum));
	}
}

static void dwc_otg_flush_rx_fifo(void)
{
	int timeout = 0xffff;
	usb_writel(RSTCTL_RXFIFO_FLUSH, GRST_CTL_OFF);

	DEBUG("dwc_otg_flush_rx_fifo :\n");
	while ((usb_readl(GRST_CTL_OFF) & RSTCTL_RXFIFO_FLUSH) && timeout--);
	if(timeout < 0)
		DEBUG("ERROR: flush rx fifo timeout!\n");
}

static void dwc_otg_flush_tx_fifo(unsigned char txf_num)
{
	u32 gintsts;
	u32 grstctl = 0;
	int timeout = 0xffff;
	gintsts = usb_readl(GINT_STS_OFF);

	/*
	 * Step1: Check that GINTSTS.GinNakEff=0 if this
	 *		  bit is cleared then set Dctl.SGNPInNak = 1.
	 *        Nak effective interrupt = H indicating the core
	 *        is not reading from fifo
	 */

	DEBUG("dwc_otg_flush_tx_fifo :");
	DEBUG_HEX(txf_num);
	if ((gintsts & GINTSTS_GINNAK_EFF))
	{
		DEBUG("DCTL_SET_GNPINNAK \n");
		usb_writel(usb_readl(OTG_DCTL_OFF) | DCTL_SET_GNPINNAK, OTG_DCTL_OFF);

		/* Step2: wait for GINTSTS.GINNakEff=1,which indicates
		 *        the NAK setting has taken effect to all IN endpoints */
		while(!(usb_readl(GINT_STS_OFF) & GINTSTS_GINNAK_EFF))
			    udelay(1);
	}

	/* Step3: wait for ahb master idle state */
	while (!(usb_readl(GRST_CTL_OFF) & RSTCTL_AHB_IDLE));

	/* Step4: Check that GrstCtl.TxFFlsh=0, if it is 0, then write
	 *        the TxFIFO number you want to flush to GrstCTL.TxFNum*/
	//while (usb_readl(GRST_CTL_OFF) & RSTCTL_TXFIFO_FLUSH);
	grstctl = usb_readl(GRST_CTL_OFF) & ~(RSTCTL_TXFNUM_MASK);
	grstctl |= (txf_num << 6) | RSTCTL_TXFIFO_FLUSH;

	/* Step5: Set GRSTCTL.TxFFlsh=1 and wait for it to clear */
	usb_writel(grstctl, GRST_CTL_OFF);
	while ((usb_readl(GRST_CTL_OFF) & RSTCTL_TXFIFO_FLUSH) && timeout--);
	if(timeout < 0)
		DEBUG("error: flush fifo timeout!");
	/* Step6: Set the DCTL.GCNPinNak bit */
	usb_writel(usb_readl(OTG_DCTL_OFF) | DCTL_CLR_GNPINNAK, OTG_DCTL_OFF);
}

static void dwc_set_in_nak(int epnum)
{
	int  timeout = 5000;
	usb_writel(usb_readl(DIEP_CTL_OFF(epnum)) | DEP_SET_NAK, DIEP_CTL_OFF(epnum));
	do {
		udelay(1);
	} while((!(usb_readl(DIEP_INT_OFF(epnum)) & DEP_INEP_NAKEFF)) && timeout--);
	if(timeout < 0) {
		DEBUG("dwc set in nak timeout epnum:");
		DEBUG_HEX(epnum);
	}
}

void dwc_disable_in_ep(int epnum)
{
	int  timeout = 100000;

	if (!(usb_readl(DIEP_CTL_OFF(epnum)) & DEP_ENA_BIT))
		return ;

	DEBUG("disable in endpoint epnum:\n");
	DEBUG_HEX(epnum);

	/*step 1 : set nak*/
	dwc_set_in_nak(epnum);

	/*step 2: disable endpoint*/
	usb_writel(usb_readl(DIEP_CTL_OFF(epnum)) | DEP_DISENA_BIT, DIEP_CTL_OFF(epnum));
    	do {
		udelay(1);
	}while((!(usb_readl(DIEP_INT_OFF(epnum)) & DEP_EPDIS_INT)) && timeout--);
	if(timeout < 0) {
		DEBUG("dwc disable in ep timeout epnum:");
		DEBUG_HEX(epnum);
	}

	usb_writel(DEP_EPDIS_INT, DIEP_INT_OFF(epnum));

	/*step 3: flush tx fifo*/
	dwc_otg_flush_tx_fifo(epnum);

	usb_writel(0x0, DIEP_SIZE_OFF(epnum));

	/*step 4: clear nak*/
	if (epnum == 1)
		usb_writel(usb_readl(DIEP_CTL_OFF(1)) | DEP_CLEAR_NAK, DIEP_CTL_OFF(1));
}

static void dwc_otg_core_reset(void)
{
	int timeout = 10000;
	u32 greset = usb_readl(GRST_CTL_OFF);

	usb_writel(greset | RSTCTL_CORE_RST, GRST_CTL_OFF);
	do {
		udelay(10);
	} while ((usb_readl(GRST_CTL_OFF) & RSTCTL_CORE_RST) && timeout--);
	if (timeout < 0) {
		DEBUG("GRESET wait reset timeout.\n");
	}
	/* wait for 3 phy clocks */
	udelay(100);
}

static void dwc_otg_core_init(void)
{
	u32 reset = 0;
	u32 gusbcfg = 0;
	DEBUG("dwc_otg_core_init :\n");

	/* AHB config Slave mode ,Unmask globle inter*/
/*	usb_writel(AHBCFG_GLOBLE_INTRMASK, GAHB_CFG_OFF);*/

	/*Mask RxfvlMsk Intr*/
	usb_writel(usb_readl(GINT_MASK_OFF) & ~GINTSTS_RXFIFO_NEMPTY, GINT_MASK_OFF);

	/*HNP SRP not support , usb2.0 , utmi+, 16bit phy*/
	DEBUG("GUSB_CFG :");
	DEBUG_HEX(usb_readl(GUSB_CFG_OFF));

	gusbcfg = usb_readl(GUSB_CFG_OFF);
	if (!(gusbcfg & USBCFG_16BIT_PHY) ||
			(gusbcfg & USBCFG_PHY_INF_UPLI)) {
		/*When we change phy config, we must reset core*/
		reset = 1;
	}

	usb_writel(USBCFG_16BIT_PHY | USBCFG_TRDTIME(5), GUSB_CFG_OFF);

	if (reset) {
		dwc_otg_core_reset();
		usb_writel(AHBCFG_GLOBLE_INTRMASK, GAHB_CFG_OFF);
	}

	usb_writel(GINTSTS_MODE_MISMATCH | GINTSTS_OTG_INTR, GINT_MASK_OFF);
	usb_writel(GINTSTS_START_FRAM, GINT_STS_OFF);
}

static void dwc_fifo_allocate(void)
{
	u32 start_addr = 0;
	u32 gdfifocfg;

	/*rx fifo size*/
	usb_writel(DEP_RXFIFO_SIZE, GRXFIFO_SIZE_OFF);
	DEBUG("rx fifo size: \n");
	DEBUG_HEX(DEP_RXFIFO_SIZE);

	/* txfifo0 size */
	start_addr += DEP_RXFIFO_SIZE;
	usb_writel((DEP_NPTXFIFO_SIZE << 16) | start_addr, GNPTXFIFO_SIZE_OFF);
	DEBUG("tx fifo 0  size:\n");
	DEBUG_HEX((DEP_NPTXFIFO_SIZE << 16) | start_addr);

	/* txfifo1 size */
	start_addr += DEP_NPTXFIFO_SIZE;
	usb_writel((DEP_DTXFIFO1_SIZE << 16) | start_addr, DIEPTXF_OFF(1));
	DEBUG("tx fifo 1 size:");
	DEBUG_HEX((DEP_DTXFIFO1_SIZE << 16) | start_addr);

	start_addr += DEP_DTXFIFO1_SIZE;
	gdfifocfg = (start_addr << 16) | usb_readl(GHW_CFG3_OFF) >> 16;
	usb_writel(gdfifocfg, GDFIFO_CFG_OFF);

	dwc_otg_flush_tx_fifo(0x10);
	dwc_otg_flush_rx_fifo();
}

static void dwc_otg_device_init(void)
{

	/* dma disable ,High speed , stall no zero handshack*/
	usb_writel(DCFG_HANDSHAKE_STALL_ERR_STATUS, OTG_DCFG_OFF);

	/* Soft  connect*/
	usb_writel((usb_readl(OTG_DCTL_OFF) & (~DCTL_SOFT_DISCONN)) | DCTL_NAK_ON_BBLE, OTG_DCTL_OFF);

	/* Unmask suspend earlysuspend reset enumdone sof intr*/
	usb_writel(usb_readl(GINT_MASK_OFF) | GINTSTS_USB_SUSPEND
			| GINTSTS_USB_RESET | GINTSTS_ENUM_DONE
			| GINTSTS_USB_EARLYSUSPEND, GINT_MASK_OFF);

}

void handle_in_transfer(USB_STATUS *status, int epnum)
{
	int mps = DEP_EP_MAXPKT_SIZE(epnum);
	int xfer_size = status->length;

	/*only handle one packet one time*/
	DEBUG("handle_in_transfer epnum :");
	DEBUG_HEX(epnum);

	if (usb_readl(DIEP_CTL_OFF(epnum)) & DEP_ENA_BIT) {
		DEBUG("WARNING: ep already enable");
		return;
	}
	if (status->addr_in == NULL || xfer_size < 0)
		xfer_size = 0;

	if (mps < xfer_size)
		xfer_size = mps;

	DEBUG("handle in length left: ");
	DEBUG_HEX(status->length);
	DEBUG("transfer size: ");
	DEBUG_HEX(xfer_size);

	usb_writel((1 << DXEPSIZE_PKTCNT) | xfer_size,DIEP_SIZE_OFF(epnum));
	DEBUG("epnum :");
	DEBUG_HEX(epnum);
	DEBUG("DIEP_SIZE_OFF :");
	DEBUG_HEX(DIEP_SIZE_OFF(epnum));
	usb_writel(usb_readl(DIEP_CTL_OFF(epnum)) | DEP_ENA_BIT | DEP_CLEAR_NAK, DIEP_CTL_OFF(epnum));


	if (xfer_size)
		usb_writel(usb_readl(DIEP_EMPMSK_OFF) | (1 << epnum), DIEP_EMPMSK_OFF);
}

static void write_transfer_fifo(USB_STATUS *status, int epnum)
{
	u32 dwords = 0;
	u32 xfer_size = 0;
	u32 txstatus = 0;
	int i;
	int mps = DEP_EP_MAXPKT_SIZE(epnum);

	if (status->length > mps)
		xfer_size = mps;
	else
		xfer_size = status->length;

	dwords = (xfer_size + 3) / 4;
	if (dwords) {
		txstatus = (usb_readl(DIEP_TXFSTS_OFF(epnum)) & 0xffff);
		if (txstatus >= dwords) {
			DEBUG("xfer size one time = :");
			DEBUG_HEX(xfer_size);
			for (i = 0; i < dwords; i++) {
				usb_writel(*((unsigned int *)(status->addr_in)+i),  EP_FIFO_OFF(epnum));
			}
			status->addr_in += xfer_size;
			status->length -= xfer_size;
			usb_writel(usb_readl(DIEP_EMPMSK_OFF) & (~(1 << epnum)), DIEP_EMPMSK_OFF);
			usb_writel(DEP_INTOKEN_RECV_TXFIFO_EMPTY | DEP_TXFIFO_EMPTY, DIEP_INT_OFF(epnum));
		} else {
			DEBUG("ERROR: fifo no empty");
		}
	}
}

void dwc_udc_init(void)
{
	/*dwc core reset*/
	dwc_otg_core_reset();

	/*dwc core init*/
	dwc_otg_core_init();

	/*dwc fifo allocate*/
	dwc_fifo_allocate();

	/*dwc device init*/
	dwc_otg_device_init();

	DEBUG("dwc_udc_init ok\n");
}


static void handle_early_suspend_intr(USB_STATUS *status)
{
	u32 dsts = usb_readl(OTG_DSTS_OFF);
	DEBUG("Handle early suspend intr.\n");
	usb_writel(GINTSTS_USB_EARLYSUSPEND, GINT_STS_OFF);
	if (dsts & DSTS_ERRATIC_ERROR) {
		/*handle babble conditional,software reset*/
		//memset((int *)status, 0,2*sizeof(USB_STATUS));
		status[0].addr_in  = NULL;
		status[0].addr_out = NULL;
		status[0].setup_packet[0] = 0;
		status[0].setup_packet[1] = 0;
		status[0].length = 0;
		status[0].fw_len = 0;

		status[1].addr_in  = NULL;
		status[1].addr_out = NULL;
		status[1].setup_packet[0] = 0;
		status[1].setup_packet[1] = 0;
		status[1].length = 0;
		status[1].fw_len = 0;
		usb_writel(usb_readl(OTG_DCTL_OFF) | DCTL_SOFT_DISCONN, OTG_DCTL_OFF);
		mdelay(100);
		dwc_udc_init();
	}
}

void handle_reset_intr(int *config)
{
	DEBUG("Handle reset intr called \n");

	/* step1 :NAK OUT ep */
	usb_writel(DEP_SET_NAK, DOEP_CTL_OFF(0));
	usb_writel(DEP_SET_NAK, DOEP_CTL_OFF(1));

	/* step2: unmask inter */
	usb_writel(DAINT_IEP_MASK(0) | DAINT_OEP_MASK(0), DAINT_MASK_OFF);
	usb_writel(DEP_SETUP_PHASE_DONE | DEP_XFER_COMP, DOEP_MASK_OFF);
	usb_writel(DEP_XFER_COMP, DIEP_MASK_OFF);

	/* step3: device init nothing to do */

	/* step4: dfifo dynamic allocated */
	//dwc_fifo_allocate();

	/* step5: Reset Device Address */
	usb_writel(usb_readl(OTG_DCFG_OFF) & (~DCFG_DEV_ADDR_MASK), OTG_DCFG_OFF);

	/* step6: setup EP0 to receive SETUP packets */
	usb_writel(DOEPSIZE0_SUPCNT_3, DOEP_SIZE_OFF(0));
	usb_writel(usb_readl(DOEP_CTL_OFF(0)) | DEP_ENA_BIT | DEP_CLEAR_NAK, DOEP_CTL_OFF(0));

	usb_writel(GINTSTS_USB_RESET, GINT_STS_OFF);

	/* step7: software*/
	*config = DWC_DEFAULT;
	usb_writel(0, DIEP_EMPMSK_OFF);

	dwc_disable_in_ep(0);
	dwc_disable_in_ep(1);
}	/* handle_reset_intr */

void handle_enum_done_intr(int *flag)
{
	u32 dsts = usb_readl(OTG_DSTS_OFF);
	*flag = 1;

	DEBUG("Handle enum done intr.\n");
	switch(dsts & DSTS_ENUM_SPEED_MASK) {
	case DSTS_ENUM_SPEED_HIGH:
		DEBUG("High speed.\n");
		enum_done_speed = HIGH_SPEED;
		break;
	case DSTS_ENUM_SPEED_FULL_30OR60:
	case DSTS_ENUM_SPEED_FULL_48:
		DEBUG("Full speed.\n");
		enum_done_speed = FULL_SPEED;
		break;
	case DSTS_ENUM_SPEED_LOW:
		enum_done_speed = LOW_SPEED;
	default:
		DEBUG("Fault speed enumration\n");
		return;
	}

	usb_writel(usb_readl(OTG_DCTL_OFF) | DCTL_CLR_GNPINNAK, OTG_DCTL_OFF);
	usb_writel(usb_readl(DIEP_CTL_OFF(0)) | DEP_EP0_MPS_64 | DEP_IN_FIFO_SEL(0), DIEP_CTL_OFF(0));
	usb_writel(usb_readl(DOEP_CTL_OFF(0)) | DEP_EP0_MPS_64 | DEP_CLEAR_NAK, DOEP_CTL_OFF(0));
	usb_writel(usb_readl(GINT_MASK_OFF) | GINTSTS_RXFIFO_NEMPTY
			| GINTSTS_IEP_INTR | GINTSTS_OEP_INTR
			| GINTSTS_START_FRAM, GINT_MASK_OFF);
	usb_writel(usb_readl(DOEP_MASK_OFF) | DEP_XFER_COMP
		| DEP_EPDIS_INT | DEP_SETUP_PHASE_DONE
		| DEP_OUTTOKEN_RECV_EPDIS | DEP_NAK_INT
		| DEP_NYET_INT, DOEP_MASK_OFF);
	usb_writel(usb_readl(DIEP_MASK_OFF) | DEP_XFER_COMP
		| DEP_EPDIS_INT | DEP_INTOKEN_RECV_TXFIFO_EMPTY
		| DEP_INEP_NAKEFF | DEP_NAK_INT,  DIEP_MASK_OFF);
	usb_writel(GINTSTS_ENUM_DONE, GINT_STS_OFF);
}

static void handle_rxfifo_nempty(USB_STATUS *status)
{
	u32 rxsts_pop = usb_readl(GRXSTS_POP_OFF);
	u16 count = (rxsts_pop & GRXSTSP_BYTE_CNT_MASK) >> GRXSTSP_BYTE_CNT_BIT;
	u32 epnum = (rxsts_pop & 0xf);

	switch(rxsts_pop & GRXSTSP_PKSTS_MASK) {
	case GRXSTSP_PKSTS_GOUT_NAK:
		DEBUG("GRXSTSP_PKSTS_GOUT_NAK.\n");
		break;
	case GRXSTSP_PKSTS_TX_COMP:
		DEBUG("GRXSTSP_PKSTS_RX_COMP.\n");
		break;
	case GRXSTSP_PKSTS_GOUT_RECV:
		DEBUG("GRXSTSP_PKSTS_GOUT_RECV.\n");
		read_out_packet(&status[epnum], count, epnum);
		break;
	case GRXSTSP_PKSTS_SETUP_COMP:
		DEBUG("GRXSTSP_PKSTS_SETUP_COMP.\n");
		break;
	case GRXSTSP_PKSTS_SETUP_RECV:
		DEBUG("GRXSTSP_PKSTS_SETUP_RECV.\n");
		status[epnum].setup_packet[0] = usb_readl(EP_FIFO_OFF(epnum));
		status[epnum].setup_packet[1] = usb_readl(EP_FIFO_OFF(epnum));
		break;
	default:
		DEBUG("rxsts_pop :");
		DEBUG_HEX(rxsts_pop);
		break;
	}

	usb_writel(GINTSTS_RXFIFO_NEMPTY, GINT_STS_OFF);
}

static void ep0_in_stall(void)
{
	DEBUG("ep0 stall\n");
	dwc_otg_flush_tx_fifo(0);
	usb_writel(usb_readl(DIEP_EMPMSK_OFF) & (~1), DIEP_EMPMSK_OFF);
	/*stall ep0*/
	usb_writel(usb_readl(DIEP_CTL_OFF(0)) | DEP_SET_STALL, DIEP_CTL_OFF(0));

	/*restart ep0 out*/
	usb_writel(DOEPSIZE0_SUPCNT_3, DOEP_SIZE_OFF(0));
	usb_writel(usb_readl(DOEP_CTL_OFF(0)) | DEP_ENA_BIT | DEP_CLEAR_NAK, DOEP_CTL_OFF(0));
}


static int handle_setup_packet(USB_STATUS *status,int *config)
{
	u32 i;
	u32 addr;
	u32 word1 = status[0].setup_packet[0];
	u32 word2 = status[0].setup_packet[1];
	u32 usb_stall = 0;
	int desc_size = 0;
	status[0].length = 0;
	unsigned int buffer[2] = {0,0};

	if ((word1 & 0x60) == 0x40 && (*config >= DWC_CONFIG)) {
		u32 addr_start = 0, addr_end = 0;
		/* vendor_request handle */
		DEBUG("Vendor_request case :");
		switch((word1 >> 8) & 0xff) {
		case EP0_GET_CPU_INFO:
			DEBUG("EP0_GET_CPU_INFO \n");
			dwc_disable_in_ep(1);
#if defined(CONFIG_CHECK_SOCID) && defined(CONFIG_SPL_USB_BOOT)
			read_socid(buffer);
			status[0].addr_in = (u8* )buffer;
			status[0].length = sizeof(buffer);
#else
			status[0].addr_in = (u8* )cpu_info_data;
			status[0].length = sizeof(cpu_info_data);
#endif
			break;

		case EP0_SET_DATA_ADDRESS:
			status[1].fw_len = status[1].length = 0; // keepping setting address and setting length is atomic.
			addr_start = (word1 & 0xffff0000) | (word2 & 0xffff);
			if(addr_start & 0x3)
				status[1].addr_in = status[1].addr_out = NULL;
			else
				status[1].addr_in = status[1].addr_out = addr_start;
			DEBUG("EP0_SET_DATA_ADDRESS. Data Addr = 0x");
			DEBUG_HEX((u32)status[1].addr_out);
			break;

		case EP0_SET_DATA_LENGTH:
			status[1].addr_in = status[1].addr_out; // keepping setting address and setting length is atomic.
			status[1].length = (word1 & 0xffff0000) | (word2 & 0xffff);
			if(status[1].length < 0)
				status[1].length = 0;
			addr_end = (u32)status[1].addr_in + status[1].length;
			status[1].fw_len = status[1].length;
			DEBUG("EP0_SET_DATA_LENGTH. Data length = ");
			DEBUG_HEX(status[1].length);
			break;
		case EP0_FLUSH_CACHES:
			DEBUG("flush cache\n");
			flush_cache_all();
			break;

		case EP0_PROG_START2:
			addr = ((word1 & 0xffff0000)|(word2 & 0xffff));
			DEBUG("EP0_PROG_START2. Start addr = 0x");
			DEBUG_HEX(addr);
			handle_in_transfer(&status[0],0);
			{
				int timeout = 0xffff;
				while ((!(usb_readl(DIEP_INT_OFF(0)) & DEP_XFER_COMP)) && (timeout--));
				if(timeout < 0)
					DEBUG("ERROR: status stage timeout!");
				usb_writel(DEP_XFER_COMP, DIEP_INT_OFF(0));

				DEBUG("bootrom start stage 2\n");
				jump_to_entry_point(addr);
			}
			break;
		case EP0_EXIT_LOOP:
			usb_loop = 0;
			break;
		default:
			usb_stall = 1;
			break;
		}
	} else if (((word1 & 0x60) == 0x0)) {
		DEBUG("do not support standard request.\n");
		usb_stall = 1;
	} else {
		DEBUG("class request,reserve request will be stall\n");
		usb_stall = 1;
	}

	if (!usb_stall) {
		handle_in_transfer(&status[0],0);
	} else {
		DEBUG_HEX(word1);
		DEBUG_HEX(word2);
		ep0_in_stall();
	}
	return 0;
}

static void handle_outep_intr(USB_STATUS *status,int *config)
{
	u32 ep_intr, intr;
	int epnum = 0;

	ep_intr = (usb_readl(OTG_DAINT_OFF) >> 16) & 0x3;

	while (ep_intr) {
		if (ep_intr & 0x1) {
			intr = usb_readl(DOEP_INT_OFF(epnum));
			usb_writel(intr, DOEP_INT_OFF(epnum));

			if (intr & DEP_XFER_COMP) {
				if (epnum) {
					DEBUG("out xfer complex:");
					usb_writel((1 << 19) | DEP_EP_MAXPKT_SIZE(1), DOEP_SIZE_OFF(epnum));
				} else {
					DEBUG("Handle epnum 0 xfer complex:");
					usb_writel(DOEPSIZE0_SUPCNT_3, DOEP_SIZE_OFF(0));
				}
				DEBUG_HEX(epnum);
				usb_writel(usb_readl(DOEP_CTL_OFF(epnum)) | DEP_ENA_BIT | DEP_CLEAR_NAK, DOEP_CTL_OFF(epnum));
			}

			if (intr & DEP_SETUP_PHASE_DONE) {
				DEBUG("Handle setup done inter:");
				DEBUG_HEX(epnum);
				if (!epnum) {
					if (!(usb_readl(DOEP_CTL_OFF(0)) & DEP_ENA_BIT)) {
						usb_writel(DOEPSIZE0_SUPCNT_3, DOEP_SIZE_OFF(0));
						usb_writel(usb_readl(DOEP_CTL_OFF(0)) | DEP_ENA_BIT | DEP_CLEAR_NAK, DOEP_CTL_OFF(0));
					}
					handle_setup_packet(status,config);
				}
			}

			if (intr & DEP_EPDIS_INT) {
				DEBUG("out disabled.\n");
			}

			if (intr & DEP_BABBLE_ERR_INT) {
				DEBUG("out DEP_BABBLE_ERR_INT.\n");
			}

			if (intr & DEP_NAK_INT) {
				DEBUG("out DEP_NAK_INT.\n");
			}

			if (intr & DEP_NYET_INT) {
				DEBUG("out DEP_NYET_INT.\n");
			}
		}
		epnum++;
		ep_intr >>= 1;
	}
}

static void handle_inep_intr(USB_STATUS *status)
{
	u32 ep_intr;
	int epnum = 0;

	ep_intr = (usb_readl(OTG_DAINT_OFF) & 0x3);
	while (ep_intr) {
		if (ep_intr & 0x1) {
			u32 intr = usb_readl(DIEP_INT_OFF(epnum));
			if (intr & DEP_XFER_COMP) {
				DEBUG("in xfer complex:");
				DEBUG_HEX(epnum);
				usb_writel(DEP_XFER_COMP, DIEP_INT_OFF(epnum));
			}

			if (intr & DEP_EPDIS_INT)
				usb_writel(DEP_EPDIS_INT, DIEP_INT_OFF(epnum));

			if (intr & DEP_NAK_INT)
				usb_writel(DEP_NAK_INT, DIEP_INT_OFF(epnum));

			if (intr & DEP_NYET_INT)
				usb_writel(DEP_NYET_INT, DIEP_INT_OFF(epnum));

			if (intr & DEP_INTOKEN_RECV_TXFIFO_EMPTY) {
				DEBUG("DEP_INTOKEN_RECV_TXFIFO_EMPTY: ");
				DEBUG_HEX(epnum);
				usb_writel(DEP_INTOKEN_RECV_TXFIFO_EMPTY, DIEP_INT_OFF(epnum));
				if (epnum == 1 && !(usb_readl(DIEP_EMPMSK_OFF) & (1 << epnum)))
					handle_in_transfer(&status[1],1);
			}

			if ((intr & DEP_TXFIFO_EMPTY) && (usb_readl(DIEP_EMPMSK_OFF) & (1 << epnum))) {
				DEBUG("in fifo empty:");
				DEBUG_HEX(epnum);
				write_transfer_fifo(&status[epnum], epnum);
			}
		}
		epnum++;
		ep_intr >>= 1;
	}
}

int usb_boot_loop(void)
{
	int usb_reset_flag = 0;
	int config = DWC_CONFIG;
	u32 intpending;
	USB_STATUS status[2];

	status_map = 0;
	enum_done_speed = HIGH_SPEED;

//	printf("==== enter %s, %d\n", __func__, __LINE__);
	/* Main loop of polling the usb commands */
	while (usb_loop) {

		intpending = usb_readl(GINT_STS_OFF) & usb_readl(GINT_MASK_OFF);

		if (intpending & GINTSTS_USB_EARLYSUSPEND)
			handle_early_suspend_intr(status);

		if (intpending & GINTSTS_USB_RESET)
			handle_reset_intr(&config);

		if (intpending & GINTSTS_ENUM_DONE)
			handle_enum_done_intr(&usb_reset_flag);

		if (intpending & GINTSTS_IEP_INTR)
			handle_inep_intr(status);

		if (intpending & GINTSTS_OEP_INTR)
			handle_outep_intr(status,&config);

		if (intpending & GINTSTS_RXFIFO_NEMPTY)
			handle_rxfifo_nempty(status);

	}
//	printf("==== exit %s, %d\n", __func__, __LINE__);
}

