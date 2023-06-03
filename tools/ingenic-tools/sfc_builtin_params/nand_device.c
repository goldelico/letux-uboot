#include "nand_device.h"

/*
 * params: nand flash partitions
 */
nand_partition_builtin_params_t nand_builtin_params = {

	.magic_num = SPINAND_MAGIC_NUM,

	/* max 10 partitions*/
	.partition_num = 3,
#if 0
	.partition_num = 4,
#endif

	.partition = {

		[0].name = "uboot",
		[0].offset = 0x0,
		[0].size =   0x100000,
		[0].mask_flags = NANDFLASH_PART_RW,
		[0].manager_mode = MTD_MODE,

		[1].name = "kernel",
		[1].offset = 0x100000,
		[1].size =   0x800000,
		[1].mask_flags = NANDFLASH_PART_RW,
		[1].manager_mode = MTD_MODE,

		[2].name = "rootfs",
		[2].offset = 0x900000,
		[2].size = 0xf700000,
		[2].mask_flags = NANDFLASH_PART_RW,
		[2].manager_mode = UBI_MANAGER,

#if 0
		[2].name = "rootfs",
		[2].offset = 0x900000,
		[2].size = 0x2800000,
		[2].mask_flags = NANDFLASH_PART_RW,
		[2].manager_mode = UBI_MANAGER,

		[3].name = "userdata",
		[3].offset = 0x3100000,
		[3].size = 0xcf00000,
		[3].mask_flags = NANDFLASH_PART_RW,
		[3].manager_mode = UBI_MANAGER,
#endif
	},
};

