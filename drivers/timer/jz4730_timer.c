// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4730 Timer driver.
 *
 * Copyright (c) 2020 Lubomir Rintel <lkundrak@v3.sk>
 */

#include <common.h>
#include <config.h>
#include <clk.h>
#include <dm.h>
#include <timer.h>
#include <time.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>

#define CHANNEL_ID  0

#define OST_TER		(0x00)
#define OST_TRDR(n)	(0x10 + ((n) * 0x20))
#define OST_TCNT(n)	(0x14 + ((n) * 0x20))
#define OST_TCSR(n)	(0x18 + ((n) * 0x20))
#define OST_TCRB(n)	(0x1c + ((n) * 0x20))

#define OST_TCSR_CKS_PCLK_256	0x0003

struct jz4730_timer_priv {
	void __iomem *base;
};

static u64 jz4730_timer_get_count(struct udevice *dev)
{
	struct jz4730_timer_priv *priv = dev_get_priv(dev);
	u32 val = readl(priv->base + OST_TCNT(CHANNEL_ID));

	return timer_conv_64(U32_MAX - val);
}

static int jz4730_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct jz4730_timer_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_name(dev, "pclk", &clk);
	if (ret)
		return ret;

	uc_priv->clock_rate = clk_get_rate(&clk);
	uc_priv->clock_rate /= 256;

	writew(OST_TCSR_CKS_PCLK_256, priv->base + OST_TCSR(CHANNEL_ID));
	writel(U32_MAX, priv->base + OST_TRDR(CHANNEL_ID));
	writel(U32_MAX, priv->base + OST_TCNT(CHANNEL_ID));
	writeb(BIT(CHANNEL_ID), priv->base + OST_TER);

	return 0;
}

static const struct timer_ops jz4730_timer_ops = {
	.get_count = jz4730_timer_get_count,
};

static const struct udevice_id jz4730_timer_ids[] = {
	{ .compatible = "ingenic,jz4730-tcu" },
	{ }
};

U_BOOT_DRIVER(jz4730_timer) = {
	.name = "jz4730_timer",
	.id = UCLASS_TIMER,
	.of_match = jz4730_timer_ids,
	.priv_auto_alloc_size = sizeof(struct jz4730_timer_priv),
	.probe = jz4730_timer_probe,
	.ops = &jz4730_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
