#ifdef CONFIG_MTD_SFCNOR
#include <asm/arch/spinor.h>

extern struct debug_param *debug_args;
extern struct nor_partition *get_partition_index(u32 offset,u32 length,int *pt_index);

extern struct burner_params params;
extern struct mini_spi_nor_info mini_params;
extern struct legacy_params *params_compatibility();

int sfc_reset()
{
	return sfc_nor_reset();
}

int sfc_erase()
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	int err = 0;
	int ret = jz_sfc_chip_erase();
	if (ret < 0)
		printf("sfc chip erese failed!\n");
	else
		printf("sfc chip erase ok\n");
	return ret;
}

static void sfcnor_add_info_to_flash(unsigned char *buf)
{
	struct legacy_params *l_params;
	int spl_version;
	/* spl_version is in 16byte of spl header,
	 * spl_version = 0x01, spl is new code, NOR_VERSION is 2,
	 * spl_version = 0x00, spl is old code, NOR_VERSION is 1.
	 * */
	spl_version = buf[CONFIG_SPL_VERSION_OFFSET];
	switch (spl_version) {
		case 0:
			l_params = params_compatibility();
			memcpy(buf + CONFIG_SPIFLASH_PART_OFFSET, l_params, sizeof(struct legacy_params));
			break;
		case 1:
			params.version = NOR_VERSION;
			memcpy(buf + CONFIG_SPIFLASH_PART_OFFSET, &params, sizeof(struct burner_params));
			memcpy(buf + CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct burner_params), &mini_params, sizeof(struct mini_spi_nor_info));
			break;
		default:
			printf("spl uboot version error !\n");
			break;
	}

	if(ddr_args != NULL && ddr_args->ddr_type > 0)
		*(volatile unsigned int *)(buf + 128) = ddr_args->ddr_type;

	if(*(volatile unsigned int *)(buf + 512) == 0 || *(volatile unsigned int *)(buf + 512) > 65535)
		*(volatile unsigned int *)(buf + 512) = 0x1111;
}

int sfcnor_read(struct cloner *cloner)
{
	int ret = 0;
	u32 addr = cloner->cmd->read.offset;
	u32 len = cloner->read_req->length;
	void *buf = (void *)cloner->read_req->buf;

	ret = sfc_nor_read(addr, len, buf);
	if(ret < 0)
		printf("%s error\n",__func__);

	return ret;
}

int sfc_program(struct cloner *cloner)
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	u32 offset = cloner->cmd->write.partition + cloner->cmd->write.offset;
	u32 length = cloner->cmd->write.length;
	int blk_size = spi_args->spi_erase_block_size;
	void *addr = (void *)cloner->write_req->buf;
	unsigned int ret;
	int len = 0,err = 0;
	struct spi_flash *flash;
	struct nor_partition *partition;

	volatile int pt_offset;
	volatile int pt_size;
	volatile int pt_index;
	static pt_index_bak = -1;


	BURNNER_PRI("the offset = %x\n",offset);

	if (length < blk_size || length%blk_size == 0){
		len = length;
		BURNNER_PRI("the length = %x\n",length);
	}
	else{
		len = (length/blk_size)*blk_size + blk_size;
		BURNNER_PRI("the length = %x, is no enough %x\n",len,blk_size);
	}

	partition = get_partition_index(offset,len, &pt_index);

	if(pt_index < 0 || partition == NULL){
		printf("out of partition\n");
		return -EIO;
	}

	if (spi_args->spi_erase == SPI_NO_ERASE) {
		if (partition->manager_mode == MTD_D_MODE)
			pt_index = offset / blk_size;
		if(pt_index != pt_index_bak){
			pt_index_bak = pt_index;

			if (partition->manager_mode == MTD_D_MODE) {
				ret = sfc_nor_erase(offset, length);
				BURNNER_PRI("SF: %zu bytes @ %#x Erased: %s\n",
						(size_t)length, (u32)offset,
						ret ? "ERROR" : "OK");
			} else {
				ret = sfc_nor_erase(partition->offset, partition->size);
				BURNNER_PRI("SF: %zu bytes @ %#x Erased: %s\n",
						(size_t)partition->size, (u32)partition->offset,
						ret ? "ERROR" : "OK");
			}
		}
	}

	if (offset == 0 && spi_args->download_params != 0) {
		sfcnor_add_info_to_flash(addr);
	}
	ret = sfc_nor_write(offset, len, addr);
	BURNNER_PRI("SF: %zu bytes @ %#x write: %s\n", (size_t)len, (u32)offset,
			ret ? "ERROR" : "OK");

	if(debug_args->write_back_chk){
		if(!readbuf){
			readbuf = malloc(len);
			if (!readbuf) {
				printf("malloc read buffer spaces error!\n");
				return -1;
			}
		}
		memset(readbuf,0,len);
		ret = sfc_nor_read(offset,len,readbuf);
		if(ret){
			BURNNER_PRI("SF: write back check read  ops error,please check flash info !\n");
			return -1;
		}
		ret = buf_compare(cloner->write_req->buf,readbuf,len,offset);
		BURNNER_PRI("SF: %zu bytes @ %#x check: %s\n", (size_t)len, (u32)offset, ret ? "ERROR" : "OK");
	}
	return ret;
}
#endif
