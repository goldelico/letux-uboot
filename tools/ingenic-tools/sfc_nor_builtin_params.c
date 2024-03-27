/*
 * SFC NOR params builtin.
 * It is used to build in the norflash parameter.
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <config.h>
#include <asm/arch/spinor.h>
#include "sfc_builtin_params/nor_device.h"

/* global params */
extern struct spi_nor_info builtin_spi_nor_info;
extern struct norflash_partitions builtin_norflash_partitions;
extern private_params_t builtin_private_params;

static struct builtin_params builtin_params;

void dump_cloner_params(struct burner_params *params);
void dump_mini_cloner_params(struct mini_spi_nor_info *mini_params);

static void dump_params(void)
{
	struct burner_params *burner_params = &builtin_params.burner_params;
	struct mini_spi_nor_info *mini_params = &builtin_params.mini_spi_nor_info;

	dump_cloner_params(burner_params);
	dump_mini_cloner_params(mini_params);

	printf("fs_erase_size=%d\n", burner_params->fs_erase_size);
	printf("uk_quad=%d\n", burner_params->uk_quad);
}

void mini_spi_nor_info_init(struct burner_params *params, struct mini_spi_nor_info *mini)
{
	struct spi_nor_info *info = &params->spi_nor_info;

	memcpy((void *)mini->name, (void *)info->name, sizeof(mini->name));
	mini->id = info->id;
	mini->read_standard = info->read_standard;
	mini->read_quad = info->read_quad;
	mini->wr_en = info->wr_en;
	mini->en4byte = info->en4byte;
	mini->quad_set = info->quad_set;
	mini->quad_get = info->quad_get;
	mini->busy = info->busy;
	mini->quad_ops_mode = info->quad_ops_mode;
	mini->chip_size = info->chip_size;
	mini->page_size = info->page_size;
	mini->erase_size = info->erase_size;
}

static int nor_builtin_params_init(void)
{
	struct burner_params *burner_params = &builtin_params.burner_params;
	struct mini_spi_nor_info *mini_params = &builtin_params.mini_spi_nor_info;

	/* 1.other params */
	burner_params->magic = NOR_MAGIC;
	burner_params->version = NOR_VERSION;
	burner_params->fs_erase_size = builtin_private_params.fs_erase_size;
	burner_params->uk_quad = builtin_private_params.uk_quad;

	/* 2.spi nor info params */
	memcpy((void *)&burner_params->spi_nor_info, &builtin_spi_nor_info,
			sizeof(struct spi_nor_info));

	/* 3.nor flash partitions params */
	memcpy((void *)&burner_params->norflash_partitions, &builtin_norflash_partitions,
			sizeof(struct norflash_partitions));


	/* 4.mini params */
	mini_spi_nor_info_init(burner_params, mini_params);

	if(!burner_params->spi_nor_info.id && !mini_params->id) {
		printf("nor builtin params init fail!\n");
		return -EINVAL;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int fd, i, ret, offset, params_length;
	char *spl_path, *fix_file;

	if (argc != 4) {
		printf("Usage: %s fix_file spl_path offset\n",argv[0]);
		return 1;
	}

	fix_file = argv[1];
	spl_path = argv[2];
	offset = strtol(argv[3], NULL, 16);
	params_length = sizeof(struct builtin_params);

	/* builtin params init */
	ret = nor_builtin_params_init();
	if (ret < 0)
		return ret;

	/* dump builtin params */
	//dump_params();

	printf("fix_file:%s spl_path:%s offset:%d\n", fix_file, spl_path, offset);

	fd = open(fix_file, O_RDWR);
	if (fd < 0) {
		printf("open %s Error\n", fix_file);
		return -1;
	}

	i = lseek(fd, offset, SEEK_SET);
	if (i != offset) {
		printf("lseek to %d Error\n", offset);
		return -1;
	}

	if (write(fd, (void *)&builtin_params, params_length) != params_length) {
		printf("write %s Error\n", spl_path);
		return -1;
	}

	close(fd);

	return 0;
}

void dump_cloner_params(struct burner_params *params)
{
	struct spi_nor_info *spi_nor_info;

	spi_nor_info = &params->spi_nor_info;

	printf("name=%s\n", spi_nor_info->name);
	printf("id=0x%x\n", spi_nor_info->id);

	printf("read_standard->cmd=0x%x\n",		spi_nor_info->read_standard.cmd);
	printf("read_standard->dummy=0x%x\n",		spi_nor_info->read_standard.dummy_byte);
	printf("read_standard->addr_nbyte=0x%x\n",	spi_nor_info->read_standard.addr_nbyte);
	printf("read_standard->transfer_mode=0x%x\n",	spi_nor_info->read_standard.transfer_mode);

	printf("read_quad->cmd=0x%x\n",			spi_nor_info->read_quad.cmd);
	printf("read_quad->dummy=0x%x\n",		spi_nor_info->read_quad.dummy_byte);
	printf("read_quad->addr_nbyte=0x%x\n",		spi_nor_info->read_quad.addr_nbyte);
	printf("read_quad->transfer_mode=0x%x\n",	spi_nor_info->read_quad.transfer_mode);

	printf("write_standard->cmd=0x%x\n",		spi_nor_info->write_standard.cmd);
	printf("write_standard->dummy=0x%x\n",		spi_nor_info->write_standard.dummy_byte);
	printf("write_standard->addr_nbyte=0x%x\n",	spi_nor_info->write_standard.addr_nbyte);
	printf("write_standard->transfer_mode=0x%x\n",	spi_nor_info->write_standard.transfer_mode);

	printf("write_quad->cmd=0x%x\n",		spi_nor_info->write_quad.cmd);
	printf("write_quad->dummy=0x%x\n",		spi_nor_info->write_quad.dummy_byte);
	printf("write_quad->addr_nbyte=0x%x\n",		spi_nor_info->write_quad.addr_nbyte);
	printf("write_quad->transfer_mode=0x%x\n",	spi_nor_info->write_quad.transfer_mode);

	printf("sector_erase->cmd=0x%x\n",		spi_nor_info->sector_erase.cmd);
	printf("sector_erase->dummy=0x%x\n",		spi_nor_info->sector_erase.dummy_byte);
	printf("sector_erase->addr_nbyte=0x%x\n",	spi_nor_info->sector_erase.addr_nbyte);
	printf("sector_erase->transfer_mode=0x%x\n",	spi_nor_info->sector_erase.transfer_mode);

	printf("wr_en->cmd=0x%x\n",		spi_nor_info->wr_en.cmd);
	printf("wr_en->dummy=0x%x\n",		spi_nor_info->wr_en.dummy_byte);
	printf("wr_en->addr_nbyte=0x%x\n",	spi_nor_info->wr_en.addr_nbyte);
	printf("wr_en->transfer_mode=0x%x\n",	spi_nor_info->wr_en.transfer_mode);

	printf("en4byte->cmd=0x%x\n",		spi_nor_info->en4byte.cmd);
	printf("en4byte->dummy=0x%x\n",		spi_nor_info->en4byte.dummy_byte);
	printf("en4byte->addr_nbyte=0x%x\n",	spi_nor_info->en4byte.addr_nbyte);
	printf("en4byte->transfer_mode=0x%x\n",	spi_nor_info->en4byte.transfer_mode);

	printf("quad_set->cmd=0x%x\n",		spi_nor_info->quad_set.cmd);
	printf("quad_set->bit_shift=0x%x\n",		spi_nor_info->quad_set.bit_shift);
	printf("quad_set->mask=0x%x\n",		spi_nor_info->quad_set.mask);
	printf("quad_set->val=0x%x\n",		spi_nor_info->quad_set.val);
	printf("quad_set->len=0x%x\n",		spi_nor_info->quad_set.len);
	printf("quad_set->dummy=0x%x\n",	spi_nor_info->quad_set.dummy);

	printf("quad_get->cmd=0x%x\n",		spi_nor_info->quad_get.cmd);
	printf("quad_get->bit_shift=0x%x\n",		spi_nor_info->quad_get.bit_shift);
	printf("quad_get->mask=0x%x\n",		spi_nor_info->quad_get.mask);
	printf("quad_get->val=0x%x\n",		spi_nor_info->quad_get.val);
	printf("quad_get->len=0x%x\n",		spi_nor_info->quad_get.len);
	printf("quad_get->dummy=0x%x\n",	spi_nor_info->quad_get.dummy);

	printf("busy->cmd=0x%x\n",		spi_nor_info->busy.cmd);
	printf("busy->bit_shift=0x%x\n",		spi_nor_info->busy.bit_shift);
	printf("busy->mask=0x%x\n",		spi_nor_info->busy.mask);
	printf("busy->val=0x%x\n",		spi_nor_info->busy.val);
	printf("busy->len=0x%x\n",		spi_nor_info->busy.len);
	printf("busy->dummy=0x%x\n",		spi_nor_info->busy.dummy);

	printf("quad_ops_mode=%d\n",	spi_nor_info->quad_ops_mode);
	printf("addr_ops_mode=%d\n",	spi_nor_info->addr_ops_mode);

	printf("tCHSH=%d\n",	spi_nor_info->tCHSH);
	printf("tSLCH=%d\n",	spi_nor_info->tSLCH);
	printf("tSHSL_RD=%d\n", spi_nor_info->tSHSL_RD);
	printf("tSHSL_WR=%d\n", spi_nor_info->tSHSL_WR);

	printf("chip_size=%d\n",	spi_nor_info->chip_size);
	printf("page_size=%d\n",	spi_nor_info->page_size);
	printf("erase_size=%d\n",	spi_nor_info->erase_size);

	printf("chip_erase_cmd=0x%x\n",	spi_nor_info->chip_erase_cmd);
}

void dump_mini_cloner_params(struct mini_spi_nor_info *mini_params)
{
	struct mini_spi_nor_info *spi_nor_info;

	spi_nor_info = mini_params;

	printf("mini_name=%s\n", spi_nor_info->name);
	printf("mini_id=0x%x\n", spi_nor_info->id);

	printf("mini_read_standard->cmd=0x%x\n",		spi_nor_info->read_standard.cmd);
	printf("mini_read_standard->dummy=0x%x\n",		spi_nor_info->read_standard.dummy_byte);
	printf("mini_read_standard->addr_nbyte=0x%x\n",	spi_nor_info->read_standard.addr_nbyte);
	printf("mini_read_standard->transfer_mode=0x%x\n",	spi_nor_info->read_standard.transfer_mode);

	printf("mini_read_quad->cmd=0x%x\n",			spi_nor_info->read_quad.cmd);
	printf("mini_read_quad->dummy=0x%x\n",		spi_nor_info->read_quad.dummy_byte);
	printf("mini_read_quad->addr_nbyte=0x%x\n",		spi_nor_info->read_quad.addr_nbyte);
	printf("mini_read_quad->transfer_mode=0x%x\n",	spi_nor_info->read_quad.transfer_mode);

	printf("mini_wr_en->cmd=0x%x\n",		spi_nor_info->wr_en.cmd);
	printf("mini_wr_en->dummy=0x%x\n",		spi_nor_info->wr_en.dummy_byte);
	printf("mini_wr_en->addr_nbyte=0x%x\n",	spi_nor_info->wr_en.addr_nbyte);
	printf("mini_wr_en->transfer_mode=0x%x\n",	spi_nor_info->wr_en.transfer_mode);

	printf("mini_en4byte->cmd=0x%x\n",		spi_nor_info->en4byte.cmd);
	printf("mini_en4byte->dummy=0x%x\n",		spi_nor_info->en4byte.dummy_byte);
	printf("mini_en4byte->addr_nbyte=0x%x\n",	spi_nor_info->en4byte.addr_nbyte);
	printf("mini_en4byte->transfer_mode=0x%x\n",	spi_nor_info->en4byte.transfer_mode);

	printf("mini_quad_set->cmd=0x%x\n",		spi_nor_info->quad_set.cmd);
	printf("mini_quad_set->bit_shift=0x%x\n",		spi_nor_info->quad_set.bit_shift);
	printf("mini_quad_set->mask=0x%x\n",		spi_nor_info->quad_set.mask);
	printf("mini_quad_set->val=0x%x\n",		spi_nor_info->quad_set.val);
	printf("mini_quad_set->len=0x%x\n",		spi_nor_info->quad_set.len);
	printf("mini_quad_set->dummy=0x%x\n",	spi_nor_info->quad_set.dummy);

	printf("mini_quad_get->cmd=0x%x\n",		spi_nor_info->quad_get.cmd);
	printf("mini_quad_get->bit_shift=0x%x\n",		spi_nor_info->quad_get.bit_shift);
	printf("mini_quad_get->mask=0x%x\n",		spi_nor_info->quad_get.mask);
	printf("mini_quad_get->val=0x%x\n",		spi_nor_info->quad_get.val);
	printf("mini_quad_get->len=0x%x\n",		spi_nor_info->quad_get.len);
	printf("mini_quad_get->dummy=0x%x\n",	spi_nor_info->quad_get.dummy);

	printf("mini_busy->cmd=0x%x\n",		spi_nor_info->busy.cmd);
	printf("mini_busy->bit_shift=0x%x\n",		spi_nor_info->busy.bit_shift);
	printf("mini_busy->mask=0x%x\n",		spi_nor_info->busy.mask);
	printf("mini_busy->val=0x%x\n",		spi_nor_info->busy.val);
	printf("mini_busy->len=0x%x\n",		spi_nor_info->busy.len);
	printf("mini_busy->dummy=0x%x\n",		spi_nor_info->busy.dummy);

	printf("mini_quad_ops_mode=%d\n",	spi_nor_info->quad_ops_mode);
	printf("addr_ops_mode=%d\n",	spi_nor_info->addr_ops_mode);

	printf("mini_chip_size=%d\n",	spi_nor_info->chip_size);
	printf("mini_page_size=%d\n",	spi_nor_info->page_size);
	printf("mini_erase_size=%d\n",	spi_nor_info->erase_size);
}

