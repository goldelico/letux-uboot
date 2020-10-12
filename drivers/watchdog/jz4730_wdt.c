// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4730 Watchdog Timer driver.
 *
 * Copyright (c) 2020 Lubomir Rintel <lkundrak@v3.sk>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <wdt.h>
#include <div64.h>
#include <linux/bitops.h>
#include <asm/io.h>

#define WDT_WTCSR	0x00
#define WDT_WTCNT	0x04

#define WDT_WTCSR_START BIT(4)

struct jz4730_wdt_priv {
	void __iomem *base;
	unsigned long clk_rate;
	u32 timeout;
};

static int jz4730_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct jz4730_wdt_priv *priv = dev_get_priv(dev);
	u64 timeout = timeout_ms * priv->clk_rate;

	do_div(timeout, 1000);
	priv->timeout = U32_MAX - max_t(u32, timeout, 32);
	wdt_reset(dev);
	if (timeout)
		writeb(WDT_WTCSR_START, priv->base + WDT_WTCSR);

	return 0;
}

static int jz4730_wdt_reset(struct udevice *dev)
{
	struct jz4730_wdt_priv *priv = dev_get_priv(dev);

	writel(priv->timeout, priv->base + WDT_WTCNT);

	return 0;
}

static const struct wdt_ops jz4730_wdt_ops = {
	.start = jz4730_wdt_start,
	.reset = jz4730_wdt_reset,
};

static int jz4730_wdt_probe(struct udevice *dev)
{
	struct jz4730_wdt_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	priv->clk_rate = clk_get_rate(&clk);

	return 0;
}

static const struct udevice_id jz4730_wdt_ids[] = {
	{ .compatible = "ingenic,jz4730-watchdog" },
	{ }
};

U_BOOT_DRIVER(wdt_jz4730) = {
	.name = "wdt_jz4730",
	.id = UCLASS_WDT,
	.of_match = jz4730_wdt_ids,
	.ops = &jz4730_wdt_ops,
	.priv_auto_alloc_size = sizeof(struct jz4730_wdt_priv),
	.probe = jz4730_wdt_probe,
};
