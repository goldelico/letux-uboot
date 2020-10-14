// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4730 Clock Generation Unit driver.
 *
 * Copyright (C) 2020 Lubomir Rintel <lkundrak@v3.sk>
 */

#include <dt-bindings/clock/jz4730-cgu.h>
#include <common.h>
#include <dm.h>
#include <clk-uclass.h>
#include <div64.h>
#include <linux/bitops.h>
#include <asm/io.h>

#define CPM_CFCR	0x00
#define CPM_PLCR1	0x10
#define CPM_OCR		0x1c

#define CPM_OCR_EXT_RTC_CLK	BIT(8)

#define CPM_PLCR1_PLL1EN	BIT(8)

#define CPM_CFCR_PFR_SHIFT	8

#define CPM_PLCR1_PLL1FD_SHIFT	23
#define CPM_PLCR1_PLL1FD_MASK	(0x1ff << CPM_PLCR1_PLL1FD_SHIFT)
#define CPM_PLCR1_PLL1RD_SHIFT	18
#define CPM_PLCR1_PLL1RD_MASK	(0x1f << CPM_PLCR1_PLL1RD_SHIFT)
#define CPM_PLCR1_PLL1OD_SHIFT	16
#define CPM_PLCR1_PLL1OD_MASK	(0x03 << CPM_PLCR1_PLL1OD_SHIFT)

struct jz4730_cgu_priv {
	void __iomem *base;
	unsigned long ext_rate;
};

static inline unsigned int pdiv(u32 cfcr, u32 shift)
{
	static unsigned int pdiv_table[] = { 1, 2, 3, 4, 6, 8, 12, 16, 24, 32 };

	return pdiv_table[cfcr >> shift & 0xf];
}

static inline unsigned long pll_rate(u32 plcr1, unsigned long ext_rate)
{
	unsigned long long rate;

	rate = ext_rate;
	rate *= ((plcr1 & CPM_PLCR1_PLL1FD_MASK) >> CPM_PLCR1_PLL1FD_SHIFT) + 2;
	do_div(rate, ((plcr1 & CPM_PLCR1_PLL1RD_MASK) >> CPM_PLCR1_PLL1RD_SHIFT) + 2);

	return rate;
}

static ulong jz4730_cgu_clk_get_rate(struct clk *clk)
{
	struct jz4730_cgu_priv *priv = dev_get_priv(clk->dev);
	u32 cfcr, plcr1, ocr;

	switch (clk->id) {
	case JZ4730_CLK_PCLK:
		plcr1 = readl(priv->base + CPM_PLCR1);
		if (plcr1 & CPM_PLCR1_PLL1EN) {
			cfcr = readl(priv->base + CPM_CFCR);
			return pll_rate(plcr1, priv->ext_rate) /
				pdiv(cfcr, CPM_CFCR_PFR_SHIFT);
		}
		return priv->ext_rate;
	case JZ4730_CLK_WDT:
		ocr = readl(priv->base + CPM_OCR);
		if (ocr & CPM_OCR_EXT_RTC_CLK)
			return -EINVAL;
		return priv->ext_rate / 128;
	case JZ4730_CLK_UART0:
	case JZ4730_CLK_UART1:
	case JZ4730_CLK_UART2:
	case JZ4730_CLK_UART3:
	case JZ4730_CLK_I2C:
	case JZ4730_CLK_TCU:
		return priv->ext_rate;
	default:
		return -EINVAL;
	}
}

static const struct clk_ops jz4730_cgu_clk_ops = {
	.get_rate = jz4730_cgu_clk_get_rate,
};

static int jz4730_cgu_clk_probe(struct udevice *dev)
{
	struct jz4730_cgu_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_name(dev, "ext", &clk);
	if (ret)
		return ret;
	priv->ext_rate = clk_get_rate(&clk);

	return 0;
}

static const struct udevice_id jz4730_cgu_clk_of_match[] = {
	{ .compatible = "ingenic,jz4730-cgu" },
	{ },
};

U_BOOT_DRIVER(jz4730_cgu_clk) = {
	.name = "jz4730-cgu",
	.id = UCLASS_CLK,
	.of_match = jz4730_cgu_clk_of_match,
	.probe = jz4730_cgu_clk_probe,
	.priv_auto_alloc_size = sizeof(struct jz4730_cgu_priv),
	.ops = &jz4730_cgu_clk_ops,
};
