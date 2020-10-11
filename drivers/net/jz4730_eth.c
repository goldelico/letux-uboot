// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4730 Ethernet MAC driver.
 *
 * Copyright (C) 2005 Ingenic Semiconductor <jlwei@ingenic.cn>
 * Copyright (C) 2020 Lubomir Rintel <lkundrak@v3.sk>
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <net.h>
#include <phy.h>
#include <eth_phy.h>
#include <miiphy.h>
#include <command.h>
#include <cpu_func.h>
#include <linux/delay.h>
#include <asm/io.h>

#define ETH_BMR		0x1000
#define ETH_TPDR	0x1004
#define ETH_RAR		0x100C
#define ETH_TAR		0x1010
#define ETH_SR		0x1014
#define ETH_OMR		0x1018
#define ETH_IER		0x101C
#define ETH_MFCR	0x1020
#define ETH_MCR		0x0000
#define ETH_MAHR	0x0004
#define ETH_MALR	0x0008
#define ETH_HTHR	0x000C
#define ETH_HTLR	0x0010
#define ETH_MIAR	0x0014
#define ETH_MIDR	0x0018

/* Bus Mode Register (DMA_BMR) */
#define BMR_SWR		0x00000001	/* Software Reset */

#define DMA_BURST	4

/* Operation Mode Register (DMA_OMR) */
#define OMR_TTM		0x00400000	/* Transmit Threshold Mode */
#define OMR_SF		0x00200000	/* Store and Forward */
#define OMR_ST		0x00002000	/* Start/Stop Transmission Command */
#define OMR_SR		0x00000002	/* Start/Stop Receive */

/* Mac control	Register (MAC_MCR) */
#define MCR_FDX		0x00100000	/* Full Duplex Mode */
#define MCR_LCC		0x00001000	/* Late Collision control */
#define MCR_DBF		0x00000800	/* Broadcast frame Disable */
#define MCR_TE		0x00000008	/* Transmitter enable */
#define MCR_RE		0x00000004	/* Receiver enable */

/* Constants for the intr mask and intr status registers. (DMA_SIS and DMA_IER) */
#define DMA_INT_FB	0x00002000	/* Fatal bus error */
#define DMA_INT_RW	0x00000200	/* Receive watchdog timeout */
#define DMA_INT_RS	0x00000100	/* Receive stop */
#define DMA_INT_RU	0x00000080	/* Receive buffer unavailble */
#define DMA_INT_RI	0x00000040	/* Receive interrupt */
#define DMA_INT_TJ	0x00000008	/* Transmit jabber timeout */
#define DMA_INT_TU	0x00000004	/* Transmit buffer unavailble */
#define DMA_INT_TS	0x00000002	/* Transmit stop */
#define DMA_INT_TI	0x00000001	/* Transmit interrupt */

/* Receive Descriptor Bit Summary */
#define R_OWN		0x80000000	/* Own Bit */
#define RD_FL		0x3fff0000	/* Frame Length */
#define RD_ES		0x00008000	/* Error Summary */

#define RD_RER		0x02000000	/* Receive End Of Ring */
#define RD_RCH		0x01000000	/* Second Address Chained */

/* Transmit Descriptor Bit Summary */
#define T_OWN		0x80000000	/* Own Bit */

#define TD_LS		0x40000000	/* Last Segment */
#define TD_FS		0x20000000	/* First Segment */
#define TD_TER		0x02000000	/* Transmit End Of Ring */
#define TD_TCH		0x01000000	/* Second Address Chained */

#define MAX_WAIT 1000

/* Tx and Rx Descriptor */
struct jz4730_eth_desc {
	u32 status;
	u32 ctrl;
	u32 addr;
	u32 next;
};

#define NUM_TX_DESCS 4

struct jz4730_eth_priv {
	void __iomem *base;
	struct mii_dev *bus;
	struct phy_device *phy;

	int next_rx;
	int next_tx;

	struct jz4730_eth_desc rx_desc[PKTBUFSRX];
	struct jz4730_eth_desc tx_desc[NUM_TX_DESCS];
};

#define MII_CMD_ADDR(id, offset) (((id) << 11) | ((offset) << 6))
#define MII_CMD_READ(id, offset) (MII_CMD_ADDR(id, offset))
#define MII_CMD_WRITE(id, offset) (MII_CMD_ADDR(id, offset) | 0x2)

static int jz4730_eth_mdio_read(struct mii_dev *bus, int addr,
				int devad, int reg)
{
	struct jz4730_eth_priv *priv = bus->priv;
	u32 mii_cmd = MII_CMD_READ(addr, reg);
	int i;

	writel(mii_cmd, priv->base + ETH_MIAR);

	/* wait for completion */
	for (i = 0; i < MAX_WAIT; i++) {
		if (!(readl(priv->base + ETH_MIAR) & 0x1))
			break;
		udelay(1);
	}

	if (i == MAX_WAIT) {
		printf("MII wait timeout\n");
		return -EIO;
	}

	return readl(priv->base + ETH_MIDR) & 0x0000ffff;
}

static int jz4730_eth_mdio_write(struct mii_dev *bus, int addr,
				 int devad, int reg, u16 value)
{
	struct jz4730_eth_priv *priv = bus->priv;
	u32 mii_cmd = MII_CMD_WRITE(addr, reg);
	int i;

	writel(value, priv->base + ETH_MIDR);
	writel(mii_cmd, priv->base + ETH_MIAR);

	/* wait for completion */
	for (i = 0; i < MAX_WAIT; i++) {
		if (!(readl(priv->base + ETH_MIAR) & 0x1))
			break;
		udelay(1);
	}

	if (i == MAX_WAIT) {
		printf("MII wait timeout\n");
		return -EIO;
	}

	return 0;
}

static int jz4730_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct jz4730_eth_priv *priv = dev_get_priv(dev);
	int addr = eth_phy_get_addr(dev);
	int ret;

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;
	pdata->iobase = (phys_addr_t)priv->base;

	priv->bus = mdio_alloc();
	if (!priv->bus)
		return -ENOMEM;

	priv->bus->read = jz4730_eth_mdio_read;
	priv->bus->write = jz4730_eth_mdio_write;
	priv->bus->priv = priv;
	strcpy(priv->bus->name, "jz4730_eth");

	ret = mdio_register(priv->bus);
	if (ret)
		goto free_mdio;

	priv->phy = phy_connect(priv->bus, addr, dev, PHY_INTERFACE_MODE_MII);
	if (!priv->phy) {
		ret = -EIO;
		goto unregister_mdio;
	}

	phy_config(priv->phy);

	return 0;

unregister_mdio:
	mdio_unregister(priv->bus);
free_mdio:
	mdio_free(priv->bus);
	return ret;
}

static int jz4730_eth_remove(struct udevice *dev)
{
	struct jz4730_eth_priv *priv = dev_get_priv(dev);

	phy_shutdown(priv->phy);
	free(priv->phy);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

	return 0;
}

static void config_mac(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct jz4730_eth_priv *priv = dev_get_priv(dev);
	u32 omr, mcr;

	/* Set MAC address */
#define ea pdata->enetaddr
	writel((ea[3] << 24) | (ea[2] << 16) | (ea[1] << 8) | ea[0], priv->base + ETH_MALR);
	writel((ea[5] << 8) | ea[4], priv->base + ETH_MAHR);

	writel(0, priv->base + ETH_HTLR);
	writel(0, priv->base + ETH_HTHR);

	/* Assert the MCR_PS bit in CSR */
	if (priv->phy->speed == SPEED_100)
		omr = OMR_SF;
	else
		omr = OMR_TTM | OMR_SF;

	mcr = MCR_TE | MCR_RE | MCR_DBF | MCR_LCC;

	if (priv->phy->duplex == DUPLEX_FULL)
		mcr |= MCR_FDX;

	/* Set the Operation Mode (OMR) and Mac Control (MCR) registers */
	writel(omr, priv->base + ETH_OMR);
	writel(mcr, priv->base + ETH_MCR);

	/* Set the Programmable Burst Length (BMR.PBL, value 1 or 4 is validate) */
	writel(DMA_BURST << 8, priv->base + ETH_BMR);

	/* Reset csr8 */
	readl(priv->base + ETH_MFCR); /* missed frams counter */
}

static int jz4730_eth_start(struct udevice *dev)
{
	struct jz4730_eth_priv *priv = dev_get_priv(dev);
	int i;

	/* Reset ethernet unit */
	writel(readl(priv->base + ETH_BMR) | BMR_SWR, priv->base + ETH_BMR);

	for (i = 0; i < MAX_WAIT; i++) {
		if (!(readl(priv->base + ETH_BMR) & BMR_SWR))
			break;
		udelay(1);
	}

	if (i == MAX_WAIT) {
		printf("Reset eth timeout\n");
		return -EIO;
	}

	/* Disable interrupts: we don't use ethernet interrupts */
	writel(0, priv->base + ETH_IER);

	/* Configure PHY */
	phy_startup(priv->phy);

	/* Configure MAC */
	config_mac(dev);

	/* Setup the Rx&Tx descriptors */
	for (i = 0; i < PKTBUFSRX; i++) {
		priv->rx_desc[i].status = R_OWN;
		priv->rx_desc[i].ctrl = PKTSIZE_ALIGN | RD_RCH;
		priv->rx_desc[i].addr = virt_to_phys(net_rx_packets[i]);
		priv->rx_desc[i].next = virt_to_phys(priv->rx_desc + i + 1);

		flush_dcache_range((ulong)net_rx_packets[i],
				   (ulong)net_rx_packets[i] + PKTSIZE_ALIGN);
	}
	priv->rx_desc[PKTBUFSRX - 1].next = virt_to_phys(priv->rx_desc);
	priv->rx_desc[PKTBUFSRX - 1].ctrl |= RD_RER;

	flush_dcache_range((ulong)priv->rx_desc,
			   (ulong)priv->rx_desc + sizeof(priv->rx_desc));

	for (i = 0; i < NUM_TX_DESCS; i++) {
		priv->tx_desc[i].status = 0;
		priv->tx_desc[i].ctrl  = TD_TCH;
		priv->tx_desc[i].addr = 0;
		priv->tx_desc[i].next = virt_to_phys(priv->tx_desc + i + 1);
	}
	priv->tx_desc[NUM_TX_DESCS - 1].next = virt_to_phys(priv->tx_desc);
	priv->tx_desc[NUM_TX_DESCS - 1].ctrl |= TD_TER;

	flush_dcache_range((ulong)priv->tx_desc,
			   (ulong)priv->tx_desc + sizeof(priv->tx_desc));

	writel(virt_to_phys(priv->rx_desc), priv->base + ETH_RAR);
	writel(virt_to_phys(priv->tx_desc), priv->base + ETH_TAR);

	priv->next_rx = 0;
	priv->next_tx = 0;

	/* Enable ETH */
	writel(readl(priv->base + ETH_OMR) | OMR_ST | OMR_SR, priv->base + ETH_OMR);

	return 0;
}

static int jz4730_eth_send(struct udevice *dev, void *packet, int length)
{
	struct jz4730_eth_priv *priv = dev_get_priv(dev);
	struct jz4730_eth_desc *desc = &priv->tx_desc[priv->next_tx];
	int i;

	/* tx fifo should always be idle */
	desc->addr = virt_to_phys(packet);
	desc->ctrl |= TD_LS | TD_FS | length;
	desc->status = T_OWN;

	flush_dcache_range((ulong)packet, (ulong)packet + length);
	flush_dcache_range((ulong)desc, (ulong)desc + sizeof(*desc));

	/* Start the tx */
	writel(1, priv->base + ETH_TPDR);

	i = 0;
	while (desc->status & T_OWN) {
		if (i > MAX_WAIT) {
			printf("ETH TX timeout\n");
			break;
		}
		udelay(1);
		i++;

		flush_dcache_range((ulong)desc, (ulong)desc + sizeof(*desc));
	}

	writel(DMA_INT_TI | DMA_INT_TS | DMA_INT_TU |
	       DMA_INT_TJ | DMA_INT_FB, priv->base + ETH_SR);

	desc->status = 0;
	desc->addr = 0;
	desc->ctrl &= ~(TD_LS | TD_FS);

	flush_dcache_range((ulong)desc, (ulong)desc + sizeof(*desc));

	priv->next_tx++;
	if (priv->next_tx >= NUM_TX_DESCS)
		priv->next_tx = 0;

	return 0;
}

static int jz4730_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct jz4730_eth_priv *priv = dev_get_priv(dev);
	struct jz4730_eth_desc *desc = &priv->rx_desc[priv->next_rx];
	int length;
	u32 status;

	flush_dcache_range((ulong)desc, (ulong)desc + sizeof(*desc));

	status = desc->status;

	if (status & R_OWN) {
		/* Nothing has been received */
		return -EINVAL;
	}

	length = ((status & RD_FL) >> 16); /* with 4-byte CRC value */

	if (status & RD_ES) {
		printf("ETH RX error 0x%x\n", status);
		return 0;
	}

	if (length < 4) {
		printf("ETH RX buffer too short\n");
		return 0;
	}

	flush_dcache_range((ulong)net_rx_packets[priv->next_rx],
			   (ulong)net_rx_packets[priv->next_rx] + PKTSIZE_ALIGN);
	*packetp = net_rx_packets[priv->next_rx];

	return length - 4;
}

static int jz4730_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct jz4730_eth_priv *priv = dev_get_priv(dev);
	struct jz4730_eth_desc *desc = &priv->rx_desc[priv->next_rx];

	/* Clear done bits */
	writel(DMA_INT_RI | DMA_INT_RS | DMA_INT_RU |
	       DMA_INT_RW | DMA_INT_FB, priv->base + ETH_SR);

	desc->status = R_OWN;

	flush_dcache_range((ulong)desc, (ulong)desc + sizeof(*desc));

	priv->next_rx++;
	if (priv->next_rx >= PKTBUFSRX)
		priv->next_rx = 0;

	return 0;
}

static void jz4730_eth_stop(struct udevice *dev)
{
	struct jz4730_eth_priv *priv = dev_get_priv(dev);

	phy_shutdown(priv->phy);
	writel(readl(priv->base + ETH_OMR) & ~(OMR_ST | OMR_SR), priv->base + ETH_OMR);
}

static const struct eth_ops jz4730_eth_ops = {
	.start		= jz4730_eth_start,
	.send		= jz4730_eth_send,
	.recv		= jz4730_eth_recv,
	.free_pkt	= jz4730_eth_free_pkt,
	.stop		= jz4730_eth_stop,
};

static const struct udevice_id jz4730_eth_ids[] = {
	{ .compatible = "ingenic,jz4730-ethernet" },
	{ }
};

U_BOOT_DRIVER(jz4730_eth) = {
	.name		= "jz4730_eth",
	.id		= UCLASS_ETH,
	.of_match	= jz4730_eth_ids,
	.probe		= jz4730_eth_probe,
	.remove		= jz4730_eth_remove,
	.ops		= &jz4730_eth_ops,
	.priv_auto_alloc_size = sizeof(struct jz4730_eth_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
