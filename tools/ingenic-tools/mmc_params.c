/*
 * MMC SPL check tool.
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
#include <config.h>

#if defined(CONFIG_X2000) || defined(CONFIG_X2000_V12) || defined(CONFIG_M300) || defined(CONFIG_X2100) || defined(CONFIG_X2500) || defined(CONFIG_X2600) ||defined(CONFIG_AD100) || defined(CONFIG_X2580)
#define SPL_SIZE (24 * 1024)
#endif
#if defined(CONFIG_X1600)
#define SPL_SIZE (26 * 1024)
#endif

#define BLOCK_SIZE 512
#define SPL_MAX_BLOCK (SPL_SIZE / BLOCK_SIZE)

#define BUFFER_SIZE 4
#define SPL_LENGTH_POSITION	4	/* 4th */

int main(int argc, char *argv[])
{
	int count;
	FILE *fd;
	int bytes_read;
	char buffer[BUFFER_SIZE];
	unsigned int check = 0;
	volatile int t = 0;

	if (argc != 2) {
		printf("Usage: %s fromfile tofile\n\a",argv[0]);
		return 1;
	}

	fd = fopen(argv[1], "rb+");
	if (fd == NULL) {
		printf("Open %s Error\n", argv[1]);
		return 1;
	}

	count = 0;

	while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fd)) > 0) {
			count += bytes_read;
	}
	printf("mmc spl count = %08x Bytes\n", count);

	count = (count + BLOCK_SIZE - 1) / BLOCK_SIZE;
	if(count > SPL_MAX_BLOCK)
		count = SPL_MAX_BLOCK;
	printf("mmc spl count = %08x Blocks\n", count);

	/*set spl len*/
	fseek( fd, SPL_LENGTH_POSITION, SEEK_SET);
	if ((t = fwrite(&count, 4, 1, fd)) != 1) {
		printf("Check: Write %s Error\n",argv[1]);
		return 1;
	}

	fclose(fd);

	return 0;
}
