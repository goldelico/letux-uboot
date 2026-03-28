#include <cloner/cloner.h>
#include "cloner_moudle.h"
#include "cloner_log.h"
#include <part.h>
#include <mmc.h>

#define MMC_BYTE_PER_BLOCK 512
#define SPL_CODE_START_ADDRESS 0x4400


static void mmc_add_info_to_flash(char *buf)
{
	if(ddr_args != NULL && ddr_args->ddr_type > 0)
		*(volatile unsigned int *)(buf + SPL_CODE_START_ADDRESS + 128) = ddr_args->ddr_type;
}

static int clmd_mmc_init(struct cloner *cloner, void *args, void *mdata)
{
	mmc_args = (struct mmc_param *)args;
	if(!mmc_args)
	{
		printf("Not found mmc parameters\n");
		return -EINVAL;
	}

	uint32_t blk, blk_end, blk_cnt;
	uint32_t erase_cnt = 0;
	int timeout = 30000;
	int i;
	int ret;
	int dev = 0;

	if (policy_args->use_mmc0)
		dev = 0;
	else if (policy_args->use_mmc1)
		dev = 1;
	else if (policy_args->use_mmc2)
		dev = 2;
	else
		return 0;

	struct mmc *mmc = find_mmc_device(dev);
	if (!mmc) {
		printf("no mmc device at slot %x\n", dev);
		return -ENODEV;
	}

	ret = mmc_init(mmc);
	if (ret) {
		printf("ERROR: MMC Init error\n");
		return -EPERM;
	}

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return -EPERM;
	}

#ifdef CONFIG_MMC_CREATE_GPP_AND_ENH
	if (mmc_args->mmc_gpp_area) {
		for(i=0; i<mmc_args->mmc_gpp_area_count; i++) {
			ret = create_gp_partition(mmc, i + 1,
					mmc_args->gpp_area[i].length_kib,
					mmc_args->gpp_area[i].enh_attr,
					mmc_args->gpp_area[i].ext_attr);
			if (ret) {
				printf("Error: mmc create gpp area error!\n");
				return -EPERM;
			}
		}
	}

	if (mmc_args->mmc_uda_enh_area) {
		ret = set_enh_area(mmc, mmc_args->uda_enh_area.start_kib, mmc_args->uda_enh_area.length_kib);
		if (ret) {
			printf("Error: mmc set uda enhanced area error!\n");
			return -EPERM;
		}
	}
#endif

	if (mmc_args->mmc_erase == MMC_ERASE_ALL) {
		blk = 0;
		blk_cnt = mmc->capacity / MMC_BYTE_PER_BLOCK;

		BURNNER_PRI("MMC erase: dev # %d, start block # %d, count %u ... \n",
				dev, blk, blk_cnt);

		ret = mmc->block_dev.block_erase(dev, blk, blk_cnt);
		if (!ret) {
			BURNNER_PRI("Error: mmc erase error\n");
			return -EIO;
		}

		BURNNER_PRI("mmc all erase ok, blocks %d\n", blk_cnt);
		return 0;
	} else if (mmc_args->mmc_erase != MMC_ERASE_PART) {
		return 0;
	}

	/*mmc part erase */
	erase_cnt = (mmc_args->mmc_erase_range_count > MMC_ERASE_CNT_MAX) ?
		MMC_ERASE_CNT_MAX : mmc_args->mmc_erase_range_count;

	for (i = 0; erase_cnt > 0; i++, erase_cnt--) {
		blk = mmc_args->mmc_erase_range[i].start / MMC_BYTE_PER_BLOCK;
		if(mmc_args->mmc_erase_range[i].end == -1){
			blk_cnt = mmc->capacity / MMC_BYTE_PER_BLOCK - blk ;
		}else{
			blk_end = mmc_args->mmc_erase_range[i].end / MMC_BYTE_PER_BLOCK;
			blk_cnt = blk_end - blk ;
		}

		BURNNER_PRI("MMC erase: dev # %d, start block # 0x%x, count 0x%x ... \n",
				dev, blk, blk_cnt);

		ret = mmc->block_dev.block_erase(dev, blk, blk_cnt);
		if (!ret) {
			printf("Error: mmc erase error\n");
			return -EIO;
		}

		BURNNER_PRI("mmc part erase, part %d ok\n", i);

	}
	BURNNER_PRI("mmc erase ok\n");
	return 0;
}

int clmd_mmc_write(struct cloner *cloner, int sub_type, void *ops_data)
{
	u32 n;
	u32 blk = (cloner->cmd->write.partition + cloner->cmd->write.offset)/MMC_BYTE_PER_BLOCK;
	u32 cnt = (cloner->cmd->write.length + MMC_BYTE_PER_BLOCK - 1)/MMC_BYTE_PER_BLOCK;
	int dev = sub_type;
	void *addr = (void *)cloner->write_req->buf;
	uint32_t write_crc = cloner->cmd->write.crc;
	uint32_t read_crc = 0;

	struct mmc *mmc = find_mmc_device(dev);
	if (!mmc) {
		printf("no mmc device at slot %x\n", dev);
		return -ENODEV;
	}

	mmc_init(mmc);

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return -EPERM;
	}

	BURNNER_PRI("MMC write: dev # %d, block # %d, count %d ... ", dev, blk, cnt);

	if(blk == 0){
		mmc_add_info_to_flash(addr);
		write_crc = local_crc32(0xffffffff, addr, cloner->cmd->write.length);
	}

	n = mmc->block_dev.block_write(dev, blk, cnt, addr);
	BURNNER_PRI("%d blocks write: %s\n",n, (n == cnt) ? "OK" : "ERROR");

	if (n != cnt)
		return -EIO;

	if (debug_args->write_back_chk) {
		memset(addr, 0, cloner->cmd->write.length);
		n = mmc->block_dev.block_read(dev, blk, cnt, addr);
		BURNNER_PRI("%d blocks read: %s\n",n, (n == cnt) ? "OK" : "ERROR");
		if (n != cnt)
			return -EIO;

		read_crc = local_crc32(0xffffffff, addr, cloner->cmd->write.length);
		BURNNER_PRI("%d blocks check: %s\n", n, (write_crc == read_crc) ? "OK" : "ERROR");
		if (write_crc != read_crc) {
			printf("src_crc32 = %08x , dst_crc32 = %08x\n", write_crc, read_crc);
			return -EIO;
		}
	}
	return 0;
}

int clmd_mmc_read(struct cloner *cloner, int sub_type, void *ops_data)
{
	u32 n;
	u32 blk = (cloner->cmd->read.partition + cloner->cmd->read.offset)/MMC_BYTE_PER_BLOCK;
	u32 cnt = (cloner->cmd->read.length + MMC_BYTE_PER_BLOCK - 1)/MMC_BYTE_PER_BLOCK;
	void *buf = cloner->read_req->buf;
	int dev = sub_type;

	struct mmc *mmc = find_mmc_device(dev);
	if (!mmc) {
		printf("no mmc device at slot %x\n", dev);
		return -ENODEV;
	}
	mmc_init(mmc);

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return -EPERM;
	}

	n = mmc->block_dev.block_read(dev, blk, cnt, buf);
	BURNNER_PRI("%d blocks read: %s\n",n, (n == cnt) ? "OK" : "ERROR");
	if (n != cnt)
		return -EIO;
	return 0;
}


int cloner_mmc_init(void)
{
	struct cloner_moudle *clmd = malloc(sizeof(struct cloner_moudle));
	int ret;

	if (!clmd)
		return -ENOMEM;
	clmd->medium = MAGIC_MMC;
	clmd->ops = MMC;
	clmd->read = clmd_mmc_read;
	clmd->write = clmd_mmc_write;
	clmd->init = clmd_mmc_init;
	clmd->info = NULL;
	clmd->check = NULL;
	clmd->reset = NULL;
	clmd->data = NULL;
//	printf("cloner mmc register\n");
	return register_cloner_moudle(clmd);
}
CLONER_MOUDLE_INIT(cloner_mmc_init);
