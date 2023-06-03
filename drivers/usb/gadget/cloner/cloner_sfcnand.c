#ifdef CONFIG_MTD_SFCNAND
#include <common.h>
#include <nand.h>
#include <linux/mtd/mtd.h>
#include <ingenic_nand_mgr/nand_param.h>
#include <asm/arch/spinand.h>

extern struct jz_sfcnand_partition *get_partion_index(u32 startaddr,u32 length,int *pt_index);
/*******************************************************************************
 * in burner init,we find spinand information from stage2_arg
 * and change it to struct nand_param_from_burner which uboot can use
 *for chip probe,but after chip probe the struct nand_param_from_burner
 *is changed,and para_num is changed to 1,and jz_spi_support_from_burner
 *pointer addr changed to the address which param we probe.
 * ******************************************************************************/

struct jz_sfcnand_burner_param bp;
void get_burner_nandinfo(char *flash_info)
{
	int i;
	struct jz_sfcnand_burner_param *tmpbp = (struct jz_sfcnand_burner_param*)flash_info;

	bp.magic_num = tmpbp->magic_num;
	bp.partition_num= tmpbp->partition_num;

	bp.partition = malloc(sizeof(struct jz_sfcnand_partition) * bp.partition_num);

	memcpy(bp.partition, &tmpbp->partition, sizeof(struct jz_sfcnand_partition) * bp.partition_num);

#ifdef DEBUG
	struct jz_sfcnand_partition *partition = bp.partition;
	printf("**** magic num = %x\n",bp.magic_num);
	printf("**** partition_num = %x\n",bp.partition_num);

	for(i = 0; i < bp.partition_num; i++){
		printf("name = %s\n",partition[i].name);
		printf("size = %x\n",partition[i].size);
		printf("offset= %x\n",partition[i].offset);
	}
#endif
}

extern nand_info_t nand_info[CONFIG_SYS_MAX_NAND_DEVICE];
static unsigned int bad_len = 0;

static int sfc_nand_skip_bad(unsigned int addr)
{
	nand_info_t *nand;
	nand = &nand_info[0];
	unsigned int offset;
	unsigned int block_size = nand->erasesize;

	offset = addr + bad_len;
	while (nand_block_isbad(nand, offset)) {
		printf("Skip bad block 0x%lx\n", offset);
		bad_len += block_size;
		offset += block_size;
	}
	return offset;
}

int spinand_read(struct cloner *cloner)
{
	int ret = 0;
	u32 addr = cloner->cmd->read.offset;
	u32 len = cloner->cmd->read.length;
	void *buf = (void *)cloner->read_req->buf;
	nand_info_t *nand;
	nand = &nand_info[0];

	ret = nand_read(nand, addr, &len, buf);
	if(ret < 0)
		printf("%s error\n",__func__);

	return ret;
}

int spinand_program(struct cloner *cloner)
{
	u32 length = cloner->cmd->write.length;
	u32 full_size = cloner->full_size;
	void *databuf = (void *)cloner->write_req->buf;
	u32 startaddr = cloner->cmd->write.partition + (cloner->cmd->write.offset);
	char command[128];
	volatile int pt_index = -1;
	struct jz_sfcnand_partition *partition;
	int ret;

	static int pt_index_bak = -1;
	static char *part_name = NULL;
	nand_info_t *nand;
	nand = &nand_info[0];
	unsigned int block_size = nand->erasesize;

	partition = get_partion_index(startaddr,length,&pt_index);
	if (pt_index < 0)
		return -EIO;
	if (startaddr==0 && spi_args->download_params != 0) {
		sfcnand_add_info_to_flash(databuf);
	}

	if ((partition->manager_mode == MTD_MODE) || (partition->manager_mode == MTD_D_MODE)) {
		if (pt_index != pt_index_bak) {
			bad_len = 0;
		}
		startaddr = sfc_nand_skip_bad(startaddr);
		if (!spi_args->spi_erase) {
			if (pt_index != pt_index_bak || (partition->manager_mode == MTD_D_MODE && !(startaddr % block_size))) {
				memset(command, 0 , 128);
				if (partition->manager_mode == MTD_D_MODE)
					sprintf(command, "nand erase 0x%x 0x%x", startaddr, ALIGN(length, block_size));
				else
					sprintf(command, "nand erase 0x%x 0x%x", partition->offset, partition->size);
				BURNNER_PRI("%s\n", command);
				ret = run_command(command, 0);
				if (ret)
					goto out;
			}
		}

		if (pt_index != pt_index_bak) {
			pt_index_bak = pt_index;
		}
		if ((startaddr + length) <= (partition->size + partition->offset)) {
			ret = nand_write(nand, startaddr, &length, databuf);
			BURNNER_PRI("nand write to offset 0x%lx, length = 0x%lx : ", startaddr, length);
			if (ret || (length == 0)) {
				BURNNER_PRI("ERROR\n");
				return -EIO;
			} else {
				BURNNER_PRI("OK\n");
			}
		} else {
			BURNNER_PRI("ERROR : out of partition !!!\n");
		}

	} else if (partition->manager_mode == UBI_MANAGER) {
		if (startaddr == partition->offset) {
			if (!spi_args->spi_erase) {
				if (pt_index != pt_index_bak) {
					pt_index_bak = pt_index;
					memset(command, 0 , 128);
					sprintf(command, "nand erase 0x%x 0x%x", partition->offset, partition->size);
					BURNNER_PRI("%s\n", command);
					ret = run_command(command, 0);
					if (ret)
						goto out;
				}
			}

			memset(command, 0, 128);
			sprintf(command, "ubi part %s", partition->name);
			BURNNER_PRI("%s\n", command);
			ret = run_command(command, 0);
			if (ret) {
				BURNNER_PRI("ubi part error...\n");
				return ret;
			}

			memset(command, 0, X_COMMAND_LENGTH);
			sprintf(command, "ubi create %s",partition->name);
			BURNNER_PRI("%s\n", command);
			ret = run_command(command, 0);
			if (ret) {
				BURNNER_PRI("ubi create error...\n");
				return ret;
			}
		}

		memset(command, 0, 128);
		static wlen = 0;
		wlen += length;
		if (full_size && (full_size <= length)) {
			length = full_size;
			sprintf(command, "ubi write 0x%x %s 0x%x", (unsigned)databuf, partition->name, length);
		} else if (full_size) {
			sprintf(command, "ubi write.part 0x%x %s 0x%x 0x%x",(unsigned)databuf, partition->name, length, full_size);
		} else {
			sprintf(command, "ubi write.part 0x%x %s 0x%x",(unsigned)databuf, partition->name, length);
		}


		ret = run_command(command, 0);
		if (ret) {
			BURNNER_PRI("...error\n");
			return ret;
		}
	}
	if (cloner->full_size)
		cloner->full_size = 0;
	return 0;
out:
	BURNNER_PRI("...error\n");
	return ret;

}
/****************************************************************************************
 * copy spinand information from burner to u-boot-with-spl.bin
 * char *buf:u-boot-with-spl.bin data pointer
 * in function :
 * param is global variable of struct nand_param_from_burner this struct is information in spinand
 * **************************************************************************************/
void sfcnand_add_info_to_flash(char *buf)
{
	memcpy(buf + CONFIG_SPIFLASH_PART_OFFSET, &bp, sizeof(struct jz_sfcnand_burner_param) - 4);
	memcpy(buf + CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct jz_sfcnand_burner_param) - 4, bp.partition, sizeof(struct jz_sfcnand_partition) * bp.partition_num);

	if(ddr_args != NULL && ddr_args->ddr_type > 0)
		*(volatile unsigned int *)(buf + 128) = ddr_args->ddr_type;

	if(*(volatile unsigned int *)(buf + 512) == 0 || *(volatile unsigned int *)(buf + 512) > 65535)
		*(volatile unsigned int *)(buf + 512) = 0x1111;
}
#endif
