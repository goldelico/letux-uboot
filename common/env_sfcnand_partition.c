/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2008 Atmel Corporation
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <environment.h>
#include <malloc.h>
#include <spi_flash.h>
#include <search.h>
#include <errno.h>

#ifdef CONFIG_ENV_OFFSET_REDUND
static ulong env_offset		= CONFIG_ENV_OFFSET;
static ulong env_new_offset	= CONFIG_ENV_OFFSET_REDUND;

#define ACTIVE_FLAG	1
#define OBSOLETE_FLAG	0

#define READ_OPS	0
#define WRITE_OPS	1
#define ERASE_OPS	2
#define X_COMMAND_LENGTH 128
#endif /* CONFIG_ENV_OFFSET_REDUND */

DECLARE_GLOBAL_DATA_PTR;

char *env_name_spec = "SFC NAND Flash";

static int sfc_nand_ops(int ops,u32 offset,size_t len,void *buf);

/* code from spl/spl_sfc_nand_v2.c */

#define SPINAND_PARAM_SIZE			2048
struct jz_sfcnand_partition {
	char name[32];		/* identifier string */
	uint32_t size;          /* partition size */
	uint32_t offset;        /* offset within the master MTD space */
	uint32_t mask_flags;    /* master MTD flags to mask out for this partition */
	uint32_t manager_mode;  /* manager_mode mtd or ubi */
};

struct jz_sfcnand_burner_param {
	uint32_t magic_num;
	int32_t partition_num;
	struct jz_sfcnand_partition *partition;
};

struct jz_sfcnand_partition_param {
	uint8_t num_partition;
/*	struct mtd_partition *partition;*/
	struct jz_sfcnand_partition *partition;
};

struct jz_sfcnand_partition_param *get_partitions(void)
{
	struct jz_sfcnand_burner_param *burn_param;
	static struct jz_sfcnand_partition_param partitions;

	/*read param*/
    sfc_nand_ops(READ_OPS,CONFIG_SPIFLASH_PART_OFFSET,SPINAND_PARAM_SIZE,CONFIG_SYS_TEXT_BASE);
	burn_param = (void *)(CONFIG_SYS_TEXT_BASE);
	partitions.num_partition = burn_param->partition_num;
	partitions.partition = (struct jz_sfcnand_partition *)&burn_param->partition;

	return &partitions;
}

unsigned int get_part_offset_by_name(struct jz_sfcnand_partition_param *partitions, char *name)
{
	int i = 0;

	for(i = 0; i < partitions->num_partition; i++) {
		if (!strcmp(partitions->partition[i].name, name)) {
			return partitions->partition[i].offset;
		}
	}

	return -1;
}

struct jz_sfcnand_partition *get_part_by_name(struct jz_sfcnand_partition_param *partitions, char *name)
{
	int i = 0;

	for(i = 0; i < partitions->num_partition; i++) {
		if (!strcmp(partitions->partition[i].name, name)) {
			return &partitions->partition[i];
		}
	}

	return NULL;
}

/* code from spl/spl_sfc_nand_v2.c */


/*static struct spi_flash *env_flash;*/

#if defined(CONFIG_ENV_OFFSET_REDUND)
static int sfc_nand_ops(int ops,u32 offset,size_t len,void *buf)
{
	char command[X_COMMAND_LENGTH];
	int ret;
	char *cmd;
	if(ops == READ_OPS)
		cmd = "read";
	else if(ops == WRITE_OPS)
		cmd = "write";
	else
		cmd = "erase";

	memset(command,0,X_COMMAND_LENGTH);
	if(ops == ERASE_OPS)
		sprintf(command,"nand %s 0x%x 0x%x",cmd,offset,len);
	else
		sprintf(command,"nand %s.jffs2 0x%x 0x%x 0x%x",cmd,(unsigned int)buf,offset,len);

	ret = run_command(command,0);
	if(ret)
		printf("do spinand read error ! please check your param !!\n");

	return 0;
}
int saveenv(void)
{
	env_t	env_new;
	ssize_t	len;
	env_t *tmp_tmp = NULL;
	char	*res, *saved_buffer = NULL, flag = OBSOLETE_FLAG;
	u32	saved_size, saved_offset, sector = 1;
	int	ret;

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new.crc	= crc32(0, env_new.data, ENV_SIZE);
	env_new.flags	= ACTIVE_FLAG;

	if (gd->env_valid == 1) {
		env_new_offset = CONFIG_ENV_OFFSET_REDUND;
		env_offset = CONFIG_ENV_OFFSET;
	} else {
		env_new_offset = CONFIG_ENV_OFFSET;
		env_offset = CONFIG_ENV_OFFSET_REDUND;
	}
	/* Is the sector larger than the env (i.e. embedded) */
	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		saved_size = CONFIG_ENV_SECT_SIZE - CONFIG_ENV_SIZE;
		saved_offset = env_new_offset + CONFIG_ENV_SIZE;
		saved_buffer = malloc(saved_size);
		if (!saved_buffer) {
			ret = 1;
			goto done;
		}
		ret = sfc_nand_ops(READ_OPS,saved_offset,saved_size,saved_buffer);
		if (ret)
			goto done;
	}

	if (CONFIG_ENV_SIZE > CONFIG_ENV_SECT_SIZE) {
		sector = CONFIG_ENV_SIZE / CONFIG_ENV_SECT_SIZE;
		if (CONFIG_ENV_SIZE % CONFIG_ENV_SECT_SIZE)
			sector++;
	}

	puts("Erasing SPI NAND flash...");
	ret = sfc_nand_ops(ERASE_OPS,env_new_offset,sector * CONFIG_ENV_SECT_SIZE,NULL);
	if (ret)
		goto done;

	puts("Writing to SPI NAND flash...");

	ret = sfc_nand_ops(WRITE_OPS,env_new_offset,CONFIG_ENV_SIZE,&env_new);
	if (ret)
		goto done;

	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		ret = sfc_nand_ops(WRITE_OPS,saved_offset,saved_size,saved_buffer);
		if (ret)
			goto done;
	}

	tmp_tmp = (env_t *)malloc(CONFIG_ENV_SIZE);
	sfc_nand_ops(READ_OPS,env_offset,sizeof(env_t),tmp_tmp);
	tmp_tmp->flags = flag;
	ret = sfc_nand_ops(ERASE_OPS,env_offset,sector * CONFIG_ENV_SIZE,NULL);
	if (ret)
		goto done;

	ret = sfc_nand_ops(WRITE_OPS,env_offset,sizeof(env_t),tmp_tmp);
	if (ret)
		goto done;

	puts("done\n");

	gd->env_valid = gd->env_valid == 2 ? 1 : 2;

	printf("Valid environment: %d\n", (int)gd->env_valid);

 done:
	if (saved_buffer)
		free(saved_buffer);

	return ret;
}

// 遍历partition寻找分区信息
void env_relocate_spec(void)
{
    struct jz_sfcnand_partition_param *partitions = get_partitions();
    struct jz_sfcnand_partition *env_info_partition = get_part_by_name(partitions, ENV_INFO_PARTITION);
    env_t *tmp_env = (char *)malloc(CONFIG_ENV_SIZE);
    if (!tmp_env) {
        error("malloc ENV_INFO_SIZE failed\n");
        goto out;
    }
    if (!env_info_partition) {
        error("get env info partition failed\n");
        goto out;
    } else {
        int ret;
        env_t *ep = NULL;
        int i = 0;
        do {
            ret = sfc_nand_ops(READ_OPS,env_info_partition->offset+CONFIG_ENV_SIZE*i,CONFIG_ENV_SIZE,tmp_env);
            if (ret) {
                printf("read env block%d error\n", i);
                continue;
            }
            if ((tmp_env->flags == ACTIVE_FLAG) && (crc32(0, tmp_env->data, ENV_SIZE) == tmp_env->crc)) {
                gd->env_valid = i;
                ep = tmp_env;
                break;
            }
            i++;
        } while (CONFIG_ENV_SIZE*i<env_info_partition->size);
        if (ep == NULL) {
            set_default_env("!bad CRC");
            goto err_read;
        }

        ret = env_import((char *)ep, 0);
        if (!ret) {
            error("Cannot import environment: errno = %d\n", errno);
            set_default_env("env_import failed");
        }

    }
err_write:
err_read:
out:
    free(tmp_env);
}
#endif
int env_init(void)
{
	/* SPI flash isn't usable before relocation */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}
