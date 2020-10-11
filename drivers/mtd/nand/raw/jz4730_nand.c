// SPDX-License-Identifier: GPL-2.0+
/*
 * JZ4730 NAND flash driver.
 *
 * Copyright (c) 2007 Ingenic Semiconductor Inc.
 * Author: <jlwei@ingenic.cn>
 *
 * Copyright (C) 2020 Lubomir Rintel <lkundrak@v3.sk>
 */

#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <nand.h>

#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/rawnand.h>

#define EMC_SMCR3	0x1c
#define EMC_NFCSR	0x50
#define EMC_NFECC	0x54

#define EMC_NFCSR_NFE	BIT(0)
#define EMC_NFCSR_FCE	BIT(1)
#define EMC_NFCSR_ECCE	BIT(2)
#define EMC_NFCSR_ERST	BIT(3)
#define EMC_NFCSR_RB	BIT(7)

struct jz4730_nand_priv {
	void __iomem *base;
	struct nand_chip nand;
};

static inline void __iomem *mtd_to_base(struct mtd_info *mtd);

static void jz4730_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, uint ctrl)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	void __iomem *base = mtd_to_base(mtd);
	unsigned long IO_ADDR_W;

	if (ctrl & NAND_CTRL_CHANGE) {
		IO_ADDR_W = (unsigned long)nand->IO_ADDR_W;

		if (ctrl & NAND_NCE)
			setbits_32(base + EMC_NFCSR, EMC_NFCSR_FCE);
		else
			clrbits_32(base + EMC_NFCSR, EMC_NFCSR_FCE);

		IO_ADDR_W &= ~(BIT(18) | BIT(19));
		if (ctrl & NAND_CLE)
			IO_ADDR_W |= BIT(18);
		if (ctrl & NAND_ALE)
			IO_ADDR_W |= BIT(19);

		nand->IO_ADDR_W = (void *)IO_ADDR_W;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, nand->IO_ADDR_W);
}

static int jz4730_nand_dev_ready(struct mtd_info *mtd)
{
	void __iomem *base = mtd_to_base(mtd);

	return (readl(base + EMC_NFCSR) & EMC_NFCSR_RB) ? 1 : 0;
}

static void jz4730_nand_select_chip(struct mtd_info *mtd, int chip)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	unsigned long IO_ADDR_R = (unsigned long)nand->IO_ADDR_R;
	unsigned long IO_ADDR_W = (unsigned long)nand->IO_ADDR_W;

	if (chip == 0) {
		IO_ADDR_R &= ~(BIT(16) | BIT(17));
		IO_ADDR_W &= ~(BIT(16) | BIT(17));
	} else if (chip == 1) {
		IO_ADDR_R |= (BIT(16) | BIT(17));
		IO_ADDR_W |= (BIT(16) | BIT(17));
	}

	nand->IO_ADDR_R = (void *)IO_ADDR_R;
	nand->IO_ADDR_W = (void *)IO_ADDR_W;
}

static void jz4730_nand_ecc_hwctl(struct mtd_info *mtd, int mode)
{
	void __iomem *base = mtd_to_base(mtd);

	setbits_32(base + EMC_NFCSR, EMC_NFCSR_ECCE | EMC_NFCSR_ERST);
}

static int jz4730_nand_ecc_calculate(struct mtd_info *mtd,
				     const u_char *dat, u_char *ecc)
{
	void __iomem *base = mtd_to_base(mtd);
	u32 val = readl(base + EMC_NFECC);

	clrbits_32(base + EMC_NFCSR, EMC_NFCSR_ECCE);
	val = readl(base + EMC_NFECC);
	val = ~val | 0x00030000;

	ecc[0] = val >> 8;
	ecc[1] = val;
	ecc[2] = val >> 16;

	return 0;
}

void jz4730_nand_chip_init(struct nand_chip *nand)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	void __iomem *base = mtd_to_base(mtd);

	if (!IS_ENABLED(SPL_BUILD) || IS_ENABLED(SPL_NAND_ECC)) {
		nand->ecc.mode = NAND_ECC_HW;
		nand->ecc.hwctl = jz4730_nand_ecc_hwctl;
		nand->ecc.calculate = jz4730_nand_ecc_calculate;
		nand->ecc.correct = nand_correct_data;
		nand->ecc.bytes = 3;
	}

	nand->cmd_ctrl = jz4730_nand_cmd_ctrl;
	nand->dev_ready = jz4730_nand_dev_ready;
	nand->select_chip = jz4730_nand_select_chip;
	nand->IO_ADDR_R = (void __iomem *)CONFIG_SYS_NAND_BASE;
	nand->IO_ADDR_W = (void __iomem *)CONFIG_SYS_NAND_BASE;
	nand->chip_delay = 20;

	/* Enable the NAND functionality. */
	setbits_32(base + EMC_NFCSR, EMC_NFCSR_NFE);

	/* Nobody knows what does this do. */
	writel(0x06644400, base + EMC_SMCR3);
}

#if !defined(CONFIG_SPL_BUILD)

static inline void __iomem *mtd_to_base(struct mtd_info *mtd)
{
	struct nand_chip *nand = mtd_to_nand(mtd);

	return container_of(nand, struct jz4730_nand_priv, nand)->base;
}

static int jz4730_nand_probe(struct udevice *dev)
{
	struct jz4730_nand_priv *priv = dev_get_priv(dev);
	struct nand_chip *nand = &priv->nand;
	struct mtd_info *mtd = nand_to_mtd(nand);
	ofnode node;
	int ret;

	priv->base = dev_remap_addr(dev->parent);
	if (!priv->base)
		return -EINVAL;

	jz4730_nand_chip_init(nand);

	ofnode_for_each_subnode(node, dev->node)
		nand_set_flash_node(nand, node);

	ret = nand_scan(mtd, CONFIG_SYS_NAND_MAX_CHIPS);
	if (ret)
		return ret;

	return nand_register(0, mtd);
}

static const struct udevice_id jz4730_nand_dt_ids[] = {
	{ .compatible = "ingenic,jz4730-nand", },
	{ }
};

U_BOOT_DRIVER(jz4730_nand_dt) = {
	.name = "jz4730-nand",
	.id = UCLASS_MTD,
	.of_match = jz4730_nand_dt_ids,
	.probe = jz4730_nand_probe,
	.priv_auto_alloc_size = sizeof(struct jz4730_nand_priv),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_GET_DRIVER(jz4730_nand_dt),
					  &dev);
	if (ret && ret != -ENODEV)
		pr_err("Failed to initialize %s. (error %d)\n", dev->name, ret);
}

#else /* CONFIG_SPL_BUILD */

static inline void __iomem *mtd_to_base(struct mtd_info *mtd)
{
	return (void __iomem *)KSEG1 + 0x13010000;
}

int board_nand_init(struct nand_chip *nand)
{
	jz4730_nand_chip_init(nand);
	nand->read_buf = nand_read_buf;

	return 0;
}

#endif /* CONFIG_SPL_BUILD */
