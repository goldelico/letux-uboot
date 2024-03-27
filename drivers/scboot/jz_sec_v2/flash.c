#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>
#include <asm/reboot.h>
#include <asm/spl.h>
#include <mmc.h>
#include <nand.h>
#include <linux/mtd/mtd.h>


#ifdef CONFIG_MTD_SFCNAND
extern nand_info_t nand_info[CONFIG_SYS_MAX_NAND_DEVICE];
static nand_info_t *nand = NULL;
#endif

static int jz_mmc_read(const int dev, const unsigned int addr, const unsigned int len, const unsigned char *from)
{
	struct mmc *mmc_t = find_mmc_device(dev);
	if (mmc_t == NULL) {
		return -1;
	}
#define MMC_BYTE_PER_BLOCK 512
	mmc_init(mmc_t);
	int s_beg = addr / MMC_BYTE_PER_BLOCK;
	int s_offset = addr % MMC_BYTE_PER_BLOCK;

	int e_b = 0;
	int e_offset = 0;
	int ret = 0;
	if (s_offset == 0) {
		e_b = len / MMC_BYTE_PER_BLOCK;
		e_offset = len % MMC_BYTE_PER_BLOCK;
	} else {
		int len_t = len - MMC_BYTE_PER_BLOCK + s_offset;
		e_b = len_t / MMC_BYTE_PER_BLOCK;
		e_offset = len_t % MMC_BYTE_PER_BLOCK;
	}

	if (s_offset == 0 && e_offset == 0) {
		ret = (mmc_t->block_dev.block_read(dev, s_beg, e_b, from) == e_b) ? len : -1;
	} else if (s_offset == 0 && e_offset != 0) {
		unsigned char *buff = malloc((e_b + 1) * MMC_BYTE_PER_BLOCK);
		if (buff == NULL) {
			return -1;
		}
		memset(buff, 0xff, (e_b + 1) * MMC_BYTE_PER_BLOCK);
		ret = (mmc_t->block_dev.block_read(dev, s_beg, e_b + 1, buff) == (e_b + 1)) ? len : -1;
		memcpy(from, buff, len);
		free(buff);
	} else if (s_offset != 0) {
		unsigned char *buff = malloc((e_b + 2) * MMC_BYTE_PER_BLOCK);
		if (buff == NULL) {
			return -1;
		}
		memset(buff, 0xff, (e_b + 2) * MMC_BYTE_PER_BLOCK);
		ret = (mmc_t->block_dev.block_read(dev, s_beg, e_b + 2, buff) == (e_b + 2)) ? len : -1;
		memcpy(from, buff + s_offset, len);
		free(buff);
	}

	return ret;
}

static int jz_mmc_write(const int dev, const unsigned int addr, const unsigned int len, unsigned char *from)
{

	struct mmc *mmc = find_mmc_device(dev);
	if (mmc == NULL) {
		printf("%s %s %d init_mmc err\n", __FILE__, __func__, __LINE__);
		return -1;
	}

	mmc_init(mmc);
	int s_beg = addr / MMC_BYTE_PER_BLOCK;
	int s_offset = addr % MMC_BYTE_PER_BLOCK;
	int e_b = 0;
	int e_offset = 0;
	int ret = 0;
	if (s_offset == 0) {
		e_b = len / MMC_BYTE_PER_BLOCK;
		e_offset = len % MMC_BYTE_PER_BLOCK;
	} else {
		e_b = (len - MMC_BYTE_PER_BLOCK + s_offset) / MMC_BYTE_PER_BLOCK;
		e_offset = (len - MMC_BYTE_PER_BLOCK + s_offset) % MMC_BYTE_PER_BLOCK;
	}

	if (s_offset == 0 && e_offset == 0) {
		ret = (mmc->block_dev.block_write(dev, s_beg, e_b, from) == e_b) ? len : -1;
	} else if (s_offset == 0 && e_offset != 0) {
		if (mmc->block_dev.block_write(dev, s_beg, e_b, from) != e_b)
			return -1;
		unsigned char *buf = malloc(MMC_BYTE_PER_BLOCK);
		if (buf == NULL)
			return -1;
		memset(buf, 0, MMC_BYTE_PER_BLOCK);
		if (mmc->block_dev.block_read(dev, s_beg + e_b, 1, buf) != 1)
			return -1;

		memcpy(buf, from + e_b * MMC_BYTE_PER_BLOCK, len - e_b * MMC_BYTE_PER_BLOCK);
		ret = (mmc->block_dev.block_write(dev, s_beg + e_b, 1, buf) == 1) ? len : -1;
		free(buf);
	} else if (s_offset != 0 && e_offset == 0) {
		unsigned char *buff = malloc(MMC_BYTE_PER_BLOCK);
		if (buff == NULL)
			return -1;

		memset(buff, 0x0, MMC_BYTE_PER_BLOCK);
		if (mmc->block_dev.block_read(dev, s_beg, 1, buff) != 1)
			return -1;

		memcpy(buff + s_offset, from, MMC_BYTE_PER_BLOCK - s_offset);
		if (mmc->block_dev.block_write(dev, s_beg, 1, buff) != 1)
			return -1;

		free(buff);

		ret = (mmc->block_dev.block_write(dev, s_beg + 1, e_b, from) == e_b) ? len : -1;
	} else if (s_offset != 0 && e_offset != 0) {
		unsigned char *buff = malloc(MMC_BYTE_PER_BLOCK);
		if (buff == NULL)
			return -1;
		memset(buff, 0x0, MMC_BYTE_PER_BLOCK);
		if (mmc->block_dev.block_read(dev, s_beg, 1, buff) != 1)
			return -1;

		memcpy(buff + s_offset, buff, MMC_BYTE_PER_BLOCK - s_offset);
		if (mmc->block_dev.block_write(dev, s_beg, 1, buff) != 1)
			return -1;

		if (mmc->block_dev.block_write(dev, s_beg + 1, e_b, from) != e_b)
			return -1;

		memset(buff, 0xff, MMC_BYTE_PER_BLOCK);
		if (mmc->block_dev.block_read(dev, s_beg + e_b + 1, 1, buff) != 1)
			return -1;

		memcpy(buff, from + (len - MMC_BYTE_PER_BLOCK + s_offset - e_offset), e_offset);
		ret = (mmc->block_dev.block_write(dev, s_beg + 1 + e_b, 1, buff) == 1) ? len : -1;
		free(buff);
	}

	return ret;
}

int read_flash(unsigned int from, unsigned int len, unsigned char *buf)
{
	u32 boot_device;

	boot_device = spl_boot_device();

	int ret = -1;

	switch(boot_device) {
#ifdef CONFIG_MTD_SFCNOR
	case BOOT_DEVICE_SFC_NOR:
		sfc_nor_read(from, len, buf);
		ret = len;
		break;
#endif
#ifdef CONFIG_MTD_SFCNAND
	case BOOT_DEVICE_SFC_NAND:
		nand = &nand_info[0];
		ret = len;
		nand_read(nand, from, &ret, buf);
		ret = len;
		break;
#endif
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		ret = jz_mmc_read(boot_device, from, len, buf);
		break;
	default:

		printf("## ERROR ## Ckey aes only support sfc_nor ##\n");
		hang();
	}

	return ret;
}


int write_flash(unsigned int from, unsigned int len, unsigned char *buf)
{
	u32 boot_device;
	int ret = -1;

	boot_device = spl_boot_device();

	switch (boot_device) {

#ifdef CONFIG_MTD_SFCNOR
	case BOOT_DEVICE_SFC_NOR:
		if (sfc_nor_erase(from, len)) {
			printf("sfcnor erase err!\n");
			_machine_restart();
		}

		sfc_nor_write(from, len, buf);
		ret = len;
		break;
#endif
#ifdef CONFIG_MTD_SFCNAND
	case BOOT_DEVICE_SFC_NAND:
		nand = &nand_info[0];
		nand_erase(nand, from, len);
		ret = len;
		nand_write(nand, from, &ret, buf);
		ret = len;
		break;
#endif
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		ret = jz_mmc_write(boot_device, from, len, buf);
		break;

	default:
		printf("## ERROR ## Ckey aes only support sfc_nor ##\n");
		hang();
	}

	return ret;
}
