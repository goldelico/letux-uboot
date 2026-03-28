#include <asm/arch/clk.h>
#include <asm/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/arch/base.h>
#include <asm/io.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/err.h>
#include <common.h>
#include <malloc.h>

#include <edid.h>
#include <asm/arch/sfc.h>
#include <asm/arch/spinand.h>
#include <asm/arch/spinor.h>
#include "jz_sfc_common.h"
#include "reserved_part.h"

static LIST_HEAD(reserved_part_manager_list_head);

static struct reserved_part_manager *get_reserved_part_mgr(struct sfc_flash *flash)
{
	struct reserved_part_manager *reserved_part_mgr;
	list_for_each_entry(reserved_part_mgr, &reserved_part_manager_list_head, list)
		if(reserved_part_mgr->flash_owner == flash)
			return reserved_part_mgr;
	return NULL;
}

void modify_reserved_part_rw_mode(struct sfc_flash *flash, int rw_mode)
{
	struct reserved_part_manager *reserved_part_mgr = get_reserved_part_mgr(flash);
	struct reserved_partition *reserved_part;
	list_for_each_entry(reserved_part, &reserved_part_mgr->reserved_part_list_head, list){
		reserved_part->rw_mode = rw_mode;
	}
}

int scan_reserved_part(struct sfc_flash *flash, void *partitions, int nr_parts)
{
	u8 i, cnt = 1;
	int flash_type = (flash->flash_info != NULL && flash->g_nor_info == NULL)?
		SFC_NAND_FLASH:SFC_NOR_FLASH;
	struct sfc_flash_partition *parts = (flash_type == SFC_NAND_FLASH)? \
					    (struct jz_sfcnand_partition *)partitions:(struct nor_partition *)partitions;

	for(i = 0; i < nr_parts; i++)
	{
		if(parts[i].mask_flags & RESERVED_PART_SYMBOL_RO)
		{
			printf("NOTICE: Found reserved partition: %s \n", parts[i].name);
			if(add_reserved_part(flash, cnt, &parts[i], flash_type) != 0){
				return -1;
			}
			cnt++;
		}
	}
	return 0;
}
int add_reserved_part(struct sfc_flash *flash, int index, void *partitions, int flash_type)
{
	struct reserved_partition *reserved_part = kzalloc(sizeof(struct reserved_partition), GFP_KERNEL);
	if(!reserved_part){
		printf("ERROR: %s malloc failed!\n",__func__);
		return -ENOMEM;
	}
	struct reserved_part_manager *reserved_part_mgr = get_reserved_part_mgr(flash);
	if(!reserved_part_mgr){
		printf("ERROR: can`t find struct reserved_part_manager");
		return -1;
	}
	struct sfc_flash_partition *part = (struct sfc_flash_partition *)partitions;

	reserved_part->index = index;
	reserved_part->start = part->offset;
	if(flash_type == SFC_NAND_FLASH)
		reserved_part->end = (part->size == MTDPART_SIZ_FULL || part->size == MTDPART_OFS_APPEND)? \
				     flash->mtd->size:(part->offset + part->size);
	else
		reserved_part->end = (part->size == MTDPART_SIZ_FULL || part->size == MTDPART_OFS_APPEND)? \
				     flash->g_nor_info->chip_size:(part->offset + part->size);
	reserved_part->rw_mode = RESERVED_PART_R_ONLY;
	reserved_part->name = part->name;
	printf("add reserved partition %s %x - %x ",reserved_part->name, reserved_part->start, reserved_part->end);

	list_add_tail(&reserved_part->list, &reserved_part_mgr->reserved_part_list_head);
	return 0;
}

int reserved_part_init(struct sfc_flash *flash)
{
	int i, part_num;

	struct reserved_part_manager *reserved_part_mgr = kzalloc(sizeof(struct reserved_part_manager), GFP_KERNEL);
	if(!reserved_part_mgr){
		printk("ERROR: %s malloc failed!\n",__func__);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&reserved_part_mgr->reserved_part_list_head);
	reserved_part_mgr->flash_owner = flash;
	list_add_tail(&reserved_part_mgr->list, &reserved_part_manager_list_head);

	return 0;
}

static struct reserved_partition *offset_2_reserved_part(struct reserved_part_manager *reserved_part_mgr, uint32_t addr, uint32_t len)
{
	struct reserved_partition *reserved_part;
	list_for_each_entry(reserved_part, &reserved_part_mgr->reserved_part_list_head, list)
	{
		/*
		 * Flash operate has addr and len
		 * Reserved partition include offset and size
		 * The following situations are considered as reserved partitions for operation
		 *
		 * ---|---------|---------|----------|---
		 *   addr     offset offset+size addr+len
		 *
		 * ---|---------|---------|----------|---
		 *  offset     addr  offset+size addr+len
		 *
		 * ---|---------|---------|----------|---
		 *   addr     offset   addr+len offset+size
		 *
		 * ---|---------|---------|----------|---
		 *  offset     addr    addr+len offset+size
		 * */
		if((addr <= reserved_part->start && addr+len >= reserved_part->end) \
				||(addr >= reserved_part->start && addr < reserved_part->end) \
				||(addr+len > reserved_part->start && addr+len <= reserved_part->end))
			return reserved_part;
	}
	return NULL;
}

bool check_partition_is_writable(struct sfc_flash *flash, uint32_t to, uint32_t len)
{
	struct reserved_part_manager *reserved_part_mgr = get_reserved_part_mgr(flash);
	if(!reserved_part_mgr){
		printf("ERROR: can`t find struct reserved_part_manager");
		return -1;
	}
	struct reserved_partition *reserved_part = offset_2_reserved_part(reserved_part_mgr, to, len);
	if(reserved_part && reserved_part->rw_mode == RESERVED_PART_R_ONLY)
	{
		printk("ERROR: Unable to operate reserved partition: %s\n",reserved_part->name);
		return true;
	}
	return false;
}

int flash_erase_skip_reserved_part(struct sfc_flash *flash, void *partitions, int nr_parts)
{
	char command[128];
	u32 i, offset, len, ret, no_need_erase = 0;
	struct sfc_flash_partition *parts;
	struct reserved_partition *reserved_part;

	struct reserved_part_manager *reserved_part_mgr = get_reserved_part_mgr(flash);
	if(!reserved_part_mgr){
		printf("ERROR: can`t find struct reserved_part_manager");
		return -1;
	}

	printf("chip erase skip reserved partitions !\n");
	int flash_type = (flash->flash_info != NULL && flash->g_nor_info == NULL)?
		SFC_NAND_FLASH:SFC_NOR_FLASH;

	if(list_empty(&reserved_part_mgr->reserved_part_list_head))
		return (flash_type == SFC_NAND_FLASH)? \
				    run_command("nand erase.chip -y", 0):jz_sfc_chip_erase();

	parts = (flash_type == SFC_NAND_FLASH)? \
		(struct jz_sfcnand_partition *)partitions:(struct nor_partition *)partitions;

	for(i = 0; i < nr_parts; i++)
	{
		no_need_erase = 0;
		list_for_each_entry(reserved_part, &reserved_part_mgr->reserved_part_list_head, list){
			if(parts[i].name == reserved_part->name){
				printf(" flash erase skip partition:%s \n", reserved_part->name);
				no_need_erase = 1;
				break;
			}
		}
		if(no_need_erase){
			continue;
		}
		if(flash_type == SFC_NAND_FLASH){
			memset(command, 0 , 128);
			sprintf(command, "nand erase 0x%x 0x%x", parts[i].offset, parts[i].size);
			printf("nand erase 0x%x 0x%x\n", parts[i].offset, parts[i].size);
			ret = run_command(command, 0);
		}
		else{
			printf("nor erase 0x%x 0x%x\n", parts[i].offset, parts[i].size);
			ret = sfc_nor_erase(parts[i].offset, parts[i].size);
		}
		if(ret){
			printf("ERROR: %s erase error! \n", __func__);
			return -EIO;
		}
	}
	return 0;
}

