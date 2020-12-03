/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Lubomir Rintel <lkundrak@v3.sk>
 *
 * Configuration settings for Skytone Alpha 400 board.
 */

#ifndef __CONFIG_ALPHA400_H__
#define __CONFIG_ALPHA400_H__

#include <linux/sizes.h>

#ifndef CONFIG_SPL_BUILD
#define BOOTENV_DEV_NAND_LEGACY(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
	"nboot ${kernel_addr_r} " #instance " 0x100000; bootm\0"

#define BOOTENV_DEV_NAME_NAND_LEGACY(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(NAND_LEGACY, nand, 0)

#include <config_distro_bootcmd.h>
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootargs=mem=128M console=ttyS0,115200n8 root=/dev/ssfdca1 rw noatime\0" \
	"kernel_addr_r=0x80600000\0" \
	"scriptaddr=0x81000000\0" \
	"fdt_addr_r=0x81100000\0" \
	"ramdisk_addr_r=0x81200000\0" \
	BOOTENV

#define CONFIG_SYS_BOOTM_LEN		SZ_64M
#define CONFIG_SYS_MALLOC_LEN		SZ_16M
#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_INIT_SP_OFFSET	0x00400000
#define CONFIG_SYS_INIT_SP_ADDR		0x80004000
#define CONFIG_SYS_LOAD_ADDR		0x81000000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_DST	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_SIZE	SZ_512K

#define CONFIG_SPL_MAX_SIZE		SZ_4K
#define CONFIG_SPL_BSS_MAX_SIZE		SZ_4K
#define CONFIG_SPL_PAD_TO		0x00020000
#define CONFIG_SPL_BSS_START_ADDR	0x80001000
#define CONFIG_SPL_STACK		0x80004000

#define CONFIG_SPL_NAND_LOAD
#define CONFIG_SPL_NAND_RAW_ONLY

/*
 * NAND flash.
 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_MAX_CHIPS	2
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_BASE		0xb4000000
#define CONFIG_SYS_NAND_PAGE_SIZE	SZ_2K
#define CONFIG_SYS_NAND_BLOCK_SIZE	SZ_128K
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS

/*
 * The MAC address is in a NAND flash partition.
 */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_RX_ETH_BUFFER	16

/*
 * The "recovery" partition.
 */
#define CONFIG_JFFS2_NAND
#define CONFIG_JFFS2_DEV		"nand0"
#define CONFIG_JFFS2_PART_OFFSET	0x00500000
#define CONFIG_JFFS2_PART_SIZE		0x00500000

/*
 * Useful for SPL.
 */
#define CONFIG_CPU_FREQ_HZ		336000000
#define CONFIG_SYS_CLK			3686400

/*
 * These are only useful for CONFIG_DEBUG_UART.
 * The regular serial driver gets its params from DT.
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_CLK
#define CONFIG_SYS_NS16550_COM1		0xb0030000

#endif /* __CONFIG_ALPHA400_H__ */
