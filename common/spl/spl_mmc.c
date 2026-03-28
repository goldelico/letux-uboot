/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <mmc.h>
#include <fat.h>
#include "spl_rtos.h"
#include <version.h>

#include <asm/arch/clk.h>
#include <asm/arch/mmc.h>
#include <asm/arch/cpm.h>
#include <asm/io.h>
#include "spl_gpt_partition.h"
#ifdef CONFIG_JZSD_OTA_VERSION20
#include "spl_ota_jzsd.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

static ulong mmc_block_read(lbaint_t start, lbaint_t blkcnt, void *dst)
{
	int err;
	struct mmc *mmc = find_mmc_device(0);
	if (!mmc)
		return 0;
	err = mmc->block_dev.block_read(0, start, blkcnt, dst);
}

static int mmc_load_image_raw(struct mmc *mmc, unsigned long sector)
{
	unsigned long err;
	u32 image_size_sectors;
	struct image_header *header;

#ifdef CONFIG_SUPPORT_EMMC_BOOT
	mmc_boot_part_access(mmc, 0x1, 0x1, 0x1);
#endif

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
						sizeof(struct image_header));

	/* read image header to find the image size & load address */
	err = mmc->block_dev.block_read(0, sector, 1, header);
	if (err == 0)
		goto end;

	spl_parse_image_header(header);

	/* convert size to sectors - round up */
	image_size_sectors = (spl_image.size + mmc->read_bl_len - 1) /
				mmc->read_bl_len;

	/* Read the header too to avoid extra memcpy */
	err = mmc->block_dev.block_read(0, sector, image_size_sectors,
		(void *)spl_image.load_addr);

end:
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	if (err == 0)
		printf("spl: mmc blk read err - %lu\n", err);
#endif

#ifdef CONFIG_SUPPORT_EMMC_BOOT
	mmc_boot_part_access(mmc, 0x1, 0x1, 0x0);
#endif

	return (err == 0);
}

#ifdef CONFIG_SPL_OS_BOOT

#ifndef CONFIG_GPT_CREATOR
#error "must define CONFIG_GPT_CREATOR"
#endif

static int mmc_load_img_from_partition(const char *name)
{
	unsigned int start_sector;
	int ret;
	struct mmc *mmc;

	mmc = find_mmc_device(0);
	if (!mmc) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("spl: mmc device not found!!\n");
#endif
		hang();
	}

	ret = spl_get_built_in_gpt_partition(name, &start_sector, NULL);
	if (ret) {
		printf("mmc:failed get part %s\n", name);
		return ret;
	}

	return mmc_load_image_raw(mmc, start_sector);
}

static int mmc_load_image_raw_os(struct mmc *mmc)
{
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR
	if (!mmc->block_dev.block_read(0,
			CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
			CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
			(void *)CONFIG_SYS_SPL_ARGS_ADDR)) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("mmc args blk read error\n");
#endif
		return -1;
	}
#endif
	return mmc_load_img_from_partition(CONFIG_SPL_OS_NAME);
}
#endif

#ifdef CONFIG_SPL_FAT_SUPPORT
static int mmc_load_image_fat(struct mmc *mmc, const char *filename)
{
	int err;
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
						sizeof(struct image_header));

	err = file_fat_read(filename, header, sizeof(struct image_header));
	if (err <= 0)
		goto end;

	spl_parse_image_header(header);

	err = file_fat_read(filename, (u8 *)spl_image.load_addr, 0);

end:
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	if (err <= 0)
		printf("spl: error reading image %s, err - %d\n",
		       filename, err);
#endif

	return (err <= 0);
}

#ifdef CONFIG_SPL_OS_BOOT
static int mmc_load_image_fat_os(struct mmc *mmc)
{
	int err;

	err = file_fat_read(CONFIG_SPL_FAT_LOAD_ARGS_NAME,
			    (void *)CONFIG_SYS_SPL_ARGS_ADDR, 0);
	if (err <= 0) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: error reading image %s, err - %d\n",
		       CONFIG_SPL_FAT_LOAD_ARGS_NAME, err);
#endif
		return -1;
	}

	return mmc_load_image_fat(mmc, CONFIG_SPL_FAT_LOAD_KERNEL_NAME);
}
#endif

#endif

#if CONFIG_SPL_RTOS_BOOT

struct rtos_header *rtos_header;

static int mmc_rtos_load(struct mmc *mmc, unsigned long sector)
{
	unsigned long err;
	u32 rtos_size_sectors;
	struct rtos_header *header;
	struct rtos_header mheader;

	header = (struct rtos_header *)(CONFIG_SYS_TEXT_BASE -
						sizeof(struct rtos_header));

	rtos_header = header;
	/* read image header to find the image size & load address */
	err = mmc->block_dev.block_read(0, sector, 1, header);
	if (err == 0)
		goto end;

	memcpy(&mheader, header, sizeof(struct rtos_header));
	if (rtos_check_header(header))
		return -1;

	/* convert size to sectors - round up */
	rtos_size_sectors = (header->img_end - header->img_start + 512 - 1) / 512;

	/* Read the header too to avoid extra memcpy */
	err = mmc->block_dev.block_read(0, sector, rtos_size_sectors, header->img_start);
	if (err == 0)
		goto end;

	flush_cache_all();
	rtos_raw_start(&mheader, NULL);
	return 0;
end:
	printf("spl: [rtos] mmc blk read err, %d\n", err);
	return -1;
}

static void mmc_load_rtos_boot(struct mmc *mmc)
{
	int ret;
	unsigned int rtos_offset = CONFIG_RTOS_OFFSET_SECTOR;

	ret = spl_get_built_in_gpt_partition(CONFIG_SPL_RTOS_NAME, &rtos_offset, NULL);
	if (ret) {
		printf("rtos not found: "CONFIG_SPL_RTOS_NAME"\n");
		printf("use rtos default offset_addr:%d\n", CONFIG_RTOS_OFFSET_SECTOR);
		rtos_offset = CONFIG_RTOS_OFFSET_SECTOR;
	}

	if (mmc_rtos_load(mmc, rtos_offset))
		hang();
}

#endif /* CONFIG_SPL_RTOS_BOOT */

#ifdef CONFIG_JZSD_OTA_VERSION20
static struct jzsd_ota_ops jzsd_ota_ops = {
	.jzsd_read = mmc_block_read,
	.jzsd_load_img_from_partition = mmc_load_img_from_partition,
};
#endif

char *spl_mmc_load_image(void)
{
	struct mmc *mmc;
	int err;
	u32 boot_mode;

	mmc_initialize(gd->bd);
	/* We register only one device. So, the dev id is always 0 */
	mmc = find_mmc_device(0);
	if (!mmc) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("spl: mmc device not found!!\n");
#endif
		hang();
	}

	err = mmc_init(mmc);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: mmc init failed: err - %d\n", err);
#endif
		hang();
	}

#ifdef CONFIG_SPL_RTOS_BOOT
	mmc_load_rtos_boot(mmc);
#endif

	boot_mode = spl_boot_mode();
	if (boot_mode == MMCSD_MODE_RAW) {
		debug("boot mode - RAW\n");
#ifdef CONFIG_BOOT_VMLINUX
		/**
		 * warning!!!
		 * kernel must set load-addr=0x80010000 when default CONFIG_BOOT_VMLINUX
		 */
		spl_image.os = IH_OS_LINUX;
		spl_image.entry_point = CONFIG_LOAD_VMLINUX_ADDR;
		spl_image.load_addr = CONFIG_LOAD_VMLINUX_ADDR;
		err = mmc->block_dev.block_read(0, 0x1800, 0x6000,(void *)spl_image.load_addr);
		return NULL;
#endif /* CONFIG_BOOT_VMLINUX */
#ifdef CONFIG_SPL_OS_BOOT
#ifdef CONFIG_JZSD_OTA_VERSION20
		register_jzsd_ota_ops(&jzsd_ota_ops);
		return spl_jzsd_ota_load_image();
#endif
		if (spl_start_uboot() || mmc_load_image_raw_os(mmc))
#endif /* CONFIG_SPL_OS_BOOT */
			err = mmc_load_image_raw(mmc,
				CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#ifdef CONFIG_SPL_FAT_SUPPORT
	} else if (boot_mode == MMCSD_MODE_FAT) {
		debug("boot mode - FAT\n");

		err = fat_register_device(&mmc->block_dev,
			CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION);
		if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
			printf("spl: fat register err - %d\n", err);
#endif
			hang();
		}

#ifdef CONFIG_SPL_OS_BOOT
		if (spl_start_uboot() || mmc_load_image_fat_os(mmc))
#endif
		err = mmc_load_image_fat(mmc, CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME);
#endif /* CONFIG_SPL_FAT_SUPPORT */
	} else {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("spl: wrong MMC boot mode\n");
#endif
		hang();
	}

	if (err)
		hang();

	return NULL;
}
