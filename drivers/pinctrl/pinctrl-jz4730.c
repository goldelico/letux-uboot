// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4730 Pin control and GPIO driver.
 *
 * Copyright (C) 2020 Lubomir Rintel <lkundrak@v3.sk>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/pinctrl.h>
#include <asm/gpio.h>
#include <asm/io.h>

#define PINCTRL_DR(n)	(0x00 + (n) * 0x30)
#define PINCTRL_DIR(n)	(0x04 + (n) * 0x30)
#define PINCTRL_ODR(n)	(0x08 + (n) * 0x30)
#define PINCTRL_PUR(n)	(0x0c + (n) * 0x30)
#define PINCTRL_ALR(n)	(0x10 + (n) * 0x30)
#define PINCTRL_AUR(n)	(0x14 + (n) * 0x30)
#define PINCTRL_IDLR(n)	(0x18 + (n) * 0x30)
#define PINCTRL_IDUR(n)	(0x1c + (n) * 0x30)
#define PINCTRL_IER(n)	(0x20 + (n) * 0x30)
#define PINCTRL_IMR(n)	(0x24 + (n) * 0x30)
#define PINCTRL_FR(n)	(0x28 + (n) * 0x30)

struct jz4730_pinctrl_priv {
	void __iomem *base;
};

struct jz4730_gpio_priv {
	unsigned int bank;
};

static int jz4730_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct jz4730_gpio_priv *priv = dev_get_priv(dev);
	struct jz4730_pinctrl_priv *pc_priv = dev_get_priv(dev->parent);

	return !!(readl(pc_priv->base + PINCTRL_DR(priv->bank)) & BIT(offset));
}

static int jz4730_gpio_set_value(struct udevice *dev, unsigned int offset, int value)
{
	struct jz4730_gpio_priv *priv = dev_get_priv(dev);
	struct jz4730_pinctrl_priv *pc_priv = dev_get_priv(dev->parent);
	u32 gpdr = readl(pc_priv->base + PINCTRL_DR(priv->bank));

	if (value)
		gpdr |= BIT(offset);
	else
		gpdr &= ~BIT(offset);
	writel(gpdr, pc_priv->base + PINCTRL_DR(priv->bank));

	return 0;
}

static int jz4730_gpio_get_direction(struct udevice *dev, unsigned int offset)
{
	struct jz4730_gpio_priv *priv = dev_get_priv(dev);
	struct jz4730_pinctrl_priv *pc_priv = dev_get_priv(dev->parent);

	if (offset < 16) {
		if (readl(pc_priv->base + PINCTRL_ALR(priv->bank)) & (3 << (offset * 2)))
			return GPIOF_FUNC;
	} else {
		if (readl(pc_priv->base + PINCTRL_AUR(priv->bank)) & (3 << ((offset - 16) * 2)))
			return GPIOF_FUNC;
	}
	if (readl(pc_priv->base + PINCTRL_DIR(priv->bank)) & BIT(offset))
		return GPIOF_OUTPUT;

	return GPIOF_INPUT;
}

static int jz4730_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct jz4730_gpio_priv *priv = dev_get_priv(dev);
	struct jz4730_pinctrl_priv *pc_priv = dev_get_priv(dev->parent);

	clrbits_32(pc_priv->base + PINCTRL_IER(priv->bank), BIT(offset));
	clrbits_32(pc_priv->base + PINCTRL_DIR(priv->bank), BIT(offset));
	if (offset < 16)
		clrbits_32(pc_priv->base + PINCTRL_ALR(priv->bank), (3 << (offset << 1)));
	else
		clrbits_32(pc_priv->base + PINCTRL_AUR(priv->bank), (3 << ((offset - 16) << 1)));

	return 0;
}

static int jz4730_gpio_direction_output(struct udevice *dev, unsigned int offset, int value)
{
	struct jz4730_gpio_priv *priv = dev_get_priv(dev);
	struct jz4730_pinctrl_priv *pc_priv = dev_get_priv(dev->parent);

	jz4730_gpio_set_value(dev, offset, value);

	clrbits_32(pc_priv->base + PINCTRL_IER(priv->bank), BIT(offset));
	setbits_32(pc_priv->base + PINCTRL_DIR(priv->bank), BIT(offset));
	if (offset < 16)
		clrbits_32(pc_priv->base + PINCTRL_ALR(priv->bank), (3 << (offset << 1)));
	else
		clrbits_32(pc_priv->base + PINCTRL_AUR(priv->bank), (3 << ((offset - 16) << 1)));

	return 0;
}

static int jz4730_gpio_probe(struct udevice *dev)
{
	struct jz4730_gpio_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret;

	ret = ofnode_read_u32(dev_ofnode(dev), "reg", &priv->bank);
	if (ret)
		return ret;

	uc_priv->bank_name  = strdup(dev->name);
	uc_priv->gpio_count = 32;
	return 0;
}

static const struct dm_gpio_ops jz4730_gpio_ops = {
	.set_value = jz4730_gpio_set_value,
	.get_value = jz4730_gpio_get_value,
	.get_function = jz4730_gpio_get_direction,
	.direction_input = jz4730_gpio_direction_input,
	.direction_output = jz4730_gpio_direction_output,
};

static struct driver jz4730_gpio_driver = {
	.name = "jz4730-gpio",
	.id = UCLASS_GPIO,
	.probe = jz4730_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct jz4730_gpio_priv),
	.ops = &jz4730_gpio_ops,
};

static const char * const pin_names[] = {
	"PA0", "PA1", "PA2", "PA3", "PA4", "PA5", "PA6", "PA7",
	"PA8", "PA9", "PA10", "PA11", "PA12", "PA13", "PA14", "PA15",
	"PA16", "PA17", "PA18", "PA19", "PA20", "PA21", "PA22", "PA23",
	"PA24", "PA25", "PA26", "PA27", "PA28", "PA29", "PA30", "PA31",
	"PB0", "PB1", "PB2", "PB3", "PB4", "PB5", "PB6", "PB7",
	"PB8", "PB9", "PB10", "PB11", "PB12", "PB13", "PB14", "PB15",
	"PB16", "PB17", "PB18", "PB19", "PB20", "PB21", "PB22", "PB23",
	"PB24", "PB25", "PB26", "PB27", "PB28", "PB29", "PB30", "PB31",
	"PC0", "PC1", "PC2", "PC3", "PC4", "PC5", "PC6", "PC7",
	"PC8", "PC9", "PC10", "PC11", "PC12", "PC13", "PC14", "PC15",
	"PC16", "PC17", "PC18", "PC19", "PC20", "PC21", "PC22", "PC23",
	"PC24", "PC25", "PC26", "PC27", "PC28", "PC29", "PC30", "PC31",
	"PD0", "PD1", "PD2", "PD3", "PD4", "PD5", "PD6", "PD7",
	"PD8", "PD9", "PD10", "PD11", "PD12", "PD13", "PD14", "PD15",
	"PD16", "PD17", "PD18", "PD19", "PD20", "PD21", "PD22", "PD23",
	"PD24", "PD25", "PD26", "PD27", "PD28", "PD29", "PD30", "PD31",
};

static int jz4730_pinctrl_get_pins_count(struct udevice *dev)
{
	return ARRAY_SIZE(pin_names);
}

static const char *jz4730_pinctrl_get_pin_name(struct udevice *dev,
					       unsigned int selector)
{
	return pin_names[selector];
}

static const struct {
	const char *name;
	struct { u32 gpalr, gpaur; } mask[4];
} groups[] = {
	{ "lcd-16bit",		{ { 0, }, { 0x00000000, 0x00c0ffff }, { 0, }, { 0, 0, } } },
	{ "lcd-16bit-tft",	{ { 0, }, { 0x00000000, 0xff000000 }, { 0, }, { 0, 0, } } },
	{ "lcd-8bit",		{ { 0, }, { 0xffff0000, 0x003f0000 }, { 0, }, { 0, 0, } } },
	{ "lcd-no-pins",	{ { 0, }, { 0, }, { 0, }, { 0, 0, } } },
	{ "mac",		{ { 0, }, { 0, }, { 0, }, { 0, 0x03ffffff, } } },
	{ "mmc-1bit",		{ { 0, }, { 0x0000f030, 0 }, { 0, }, { 0, 0, } } },
	{ "mmc-4bit",		{ { 0, }, { 0x00000fc0, 0 }, { 0, }, { 0, 0, } } },
	{ "nand-cs1",		{ { 0, }, { 0, }, { 0, 0x000000c0 }, { 0, 0, } } },
	{ "nand-cs2",		{ { 0, }, { 0, }, { 0, 0x00000300 }, { 0, 0, } } },
	{ "nand-cs3",		{ { 0, }, { 0, }, { 0, 0x00000c00 }, { 0, 0, } } },
	{ "nand-cs4",		{ { 0, }, { 0, }, { 0, 0x00003000 }, { 0, 0, } } },
	{ "nand-cs5",		{ { 0, }, { 0, }, { 0, 0x0000c000 }, { 0, 0, } } },
	{ "pwm0",		{ { 0, }, { 0, }, { 0, 0x30000000 }, { 0, 0, } } },
	{ "pwm1",		{ { 0, }, { 0, }, { 0, 0xc0000000 }, { 0, 0, } } },
	{ "uart0-data",		{ { 0, }, { 0, }, { 0, }, { 0, 0xf0000000 } } },
};

static int jz4730_pinctrl_get_groups_count(struct udevice *dev)
{
	return ARRAY_SIZE(groups);
}

static const char *jz4730_pinctrl_get_group_name(struct udevice *dev,
						 unsigned int group_selector)
{
	return groups[group_selector].name;
}

static const struct {
	const char *name;
	struct { u32 gpalr, gpaur; } val[4];
} funcs[] = {
	{ "lcd",	{ { 0, }, { 0x55550000, 0x556a5555 }, { 0, }, { 0, 0, } } },
	{ "mac",	{ { 0, }, { 0, }, { 0, }, { 0, 0x01555555, } } },
	{ "mmc",	{ { 0, }, { 0x00005550, 0 }, { 0, }, { 0, 0, } } },
	{ "nand",	{ { 0, }, { 0, }, { 0, 0x000055c0 }, { 0, 0, } } },
	{ "pwm0",	{ { 0, }, { 0, }, { 0, 0x10000000 }, { 0, 0, } } },
	{ "pwm1",	{ { 0, }, { 0, }, { 0, 0x40000000 }, { 0, 0, } } },
	{ "sleep",	{ { 0, }, { 0, }, { 0, 0 }, { 0, 0, } } },
	{ "uart0",	{ { 0, }, { 0, }, { 0, }, { 0, 0x50000000 } } },
};

static int jz4730_pinctrl_get_funcs_count(struct udevice *dev)
{
	return ARRAY_SIZE(funcs);
}

static const char *jz4730_pinctrl_get_func_name(struct udevice *dev,
						unsigned int func_selector)
{
	return funcs[func_selector].name;
}

static inline void update_bits(void __iomem *reg, u32 mask, u32 val)
{
	if (mask)
		writel((readl(reg) & ~mask) | val, reg);
}

static int jz4730_pinctrl_set(struct udevice *dev,
			      unsigned int selector,
			      unsigned int func_selector)
{
	struct jz4730_pinctrl_priv *priv = dev_get_priv(dev);
	int bank = selector / 32;
	int pin = selector % 32;

	if (pin < 16) {
		update_bits(priv->base + PINCTRL_ALR(bank),
			    funcs[func_selector].val[bank].gpalr,
			    3 << (pin * 2));
	} else {
		update_bits(priv->base + PINCTRL_AUR(bank),
			    funcs[func_selector].val[bank].gpaur,
			    3 << ((pin - 16) * 2));
	}

	return 0;
}

static int jz4730_pinctrl_group_set(struct udevice *dev,
				    unsigned int group_selector,
				    unsigned int func_selector)
{
	struct jz4730_pinctrl_priv *priv = dev_get_priv(dev);
	int bank;

	for (bank = 0; bank < ARRAY_SIZE(groups[0].mask); bank++) {
		update_bits(priv->base + PINCTRL_ALR(bank),
			    groups[group_selector].mask[bank].gpalr,
			    funcs[func_selector].val[bank].gpalr);
		update_bits(priv->base + PINCTRL_AUR(bank),
			    groups[group_selector].mask[bank].gpaur,
			    funcs[func_selector].val[bank].gpaur);
	}

	return 0;
}

const int jz4730_pinctrl_get_pin_muxing(struct udevice *dev,
					unsigned int selector,
					char *buf, int size)
{
	struct jz4730_pinctrl_priv *priv = dev_get_priv(dev);
	int bank = selector / 32;
	int pin = selector % 32;

	snprintf(buf, size, "%3d D%d DI%d OD%d PU%d A%d ID%d IE%d IM%d F%d",
		 selector,
		 (readl(priv->base + PINCTRL_DR(bank)) >> pin) & 1,
		 (readl(priv->base + PINCTRL_DIR(bank)) >> pin) & 1,
		 (readl(priv->base + PINCTRL_ODR(bank)) >> pin) & 1,
		 (readl(priv->base + PINCTRL_PUR(bank)) >> pin) & 1,
		 pin < 16
			? (readl(priv->base + PINCTRL_ALR(bank)) >> pin * 2) & 3
			: (readl(priv->base + PINCTRL_AUR(bank)) >> (pin - 16) * 2) & 3,
		 pin < 16
			? (readl(priv->base + PINCTRL_IDLR(bank)) >> pin * 2) & 3
			: (readl(priv->base + PINCTRL_IDUR(bank)) >> (pin - 16) * 2) & 3,
		 (readl(priv->base + PINCTRL_IER(bank)) >> pin) & 1,
		 (readl(priv->base + PINCTRL_IMR(bank)) >> pin) & 1,
		 (readl(priv->base + PINCTRL_FR(bank)) >> pin) & 1);

	return 0;
}

const struct pinctrl_ops jz4730_pinctrl_ops  = {
	.get_pins_count = jz4730_pinctrl_get_pins_count,
	.get_pin_name = jz4730_pinctrl_get_pin_name,
	.get_groups_count = jz4730_pinctrl_get_groups_count,
	.get_group_name = jz4730_pinctrl_get_group_name,
	.get_functions_count = jz4730_pinctrl_get_funcs_count,
	.get_function_name = jz4730_pinctrl_get_func_name,
	.pinmux_set = jz4730_pinctrl_set,
	.pinmux_group_set = jz4730_pinctrl_group_set,
	.set_state = pinctrl_generic_set_state,
	.get_pin_muxing = jz4730_pinctrl_get_pin_muxing,
};

int jz4730_pinctrl_probe(struct udevice *dev)
{
	struct jz4730_pinctrl_priv *priv = dev_get_priv(dev);
	ofnode node;

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	dev_for_each_subnode(node, dev) {
		struct udevice *cdev;

		if (!ofnode_read_bool(node, "gpio-controller"))
			continue;

		device_bind_ofnode(dev, &jz4730_gpio_driver, ofnode_get_name(node),
				   priv, node, &cdev);
	}

	return 0;
}

static const struct udevice_id jz4730_pinctrl_of_match[] = {
	{ .compatible = "ingenic,jz4730-pinctrl", },
	{ }
};

U_BOOT_DRIVER(jz4730_pinctrl) = {
	.name = "jz4730-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(jz4730_pinctrl_of_match),
	.probe = jz4730_pinctrl_probe,
	.priv_auto_alloc_size = sizeof(struct jz4730_pinctrl_priv),
	.ops = &jz4730_pinctrl_ops,
};
