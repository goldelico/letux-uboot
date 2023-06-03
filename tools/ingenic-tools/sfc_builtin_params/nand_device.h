#ifndef  __NAND_DEVICE_H
#define  __NAND_DEVICE_H

#include <asm/arch/spinand.h>
#include <mtd/mtd-abi.h>

#define NANDFLASH_PART_RW 0
#define NANDFLASH_PART_RO (MTD_WRITEABLE)

typedef struct nand_partition_builtin_params {
	uint32_t magic_num;
	int32_t partition_num;
	struct jz_sfcnand_partition partition[];
}nand_partition_builtin_params_t;


#endif
