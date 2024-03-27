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
#include "sfc_builtin_params/nand_device.h"

/* global params */
extern nand_partition_builtin_params_t nand_builtin_params;

static int nand_builtin_params_init(void)
{
	if(!nand_builtin_params.partition_num) {
		printf("nand builtin params init fail!\n");
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
	params_length = sizeof(nand_partition_builtin_params_t) +
		(nand_builtin_params.partition_num * sizeof(struct jz_sfcnand_partition));

	/* builtin params init */
	ret = nand_builtin_params_init();
	if (ret < 0)
		return ret;

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

	if (write(fd, (void *)&nand_builtin_params, params_length) != params_length) {
		printf("write %s Error\n", spl_path);
		return -1;
	}

	close(fd);

	return 0;
}

