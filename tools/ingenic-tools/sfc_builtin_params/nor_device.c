#include "nor_device.h"


/*
 * Only one is supported at a time
 */
#define CONFIG_INGENIC_GD25Q127C
//#define CONFIG_INGENIC_XXXXX


/*
 * params: spi nor info
 * useage:
 *	set cmmand info:
 *		CMD_INFO(Command, Dummy bits, Address len, Transfer mode)
 *	set status info:
 *		ST_INFO(Command, Bit shift, Mask, Val, Len, Dummy bits)
 */
struct spi_nor_info builtin_spi_nor_info = {
#ifdef CONFIG_INGENIC_GD25Q127C
	/* GD25Q127C */
	.name = "GD25Q127C",
	.id = 0xc84018,

	.read_standard	  = CMD_INFO(0x03, 0, 3, 0),
	.read_quad	  = CMD_INFO(0x6b, 8, 3, 5),
	.write_standard	  = CMD_INFO(0x02, 0, 3, 0),
	.write_quad	  = CMD_INFO(0x32, 0, 3, 5),
	.sector_erase	  = CMD_INFO(0x52, 0, 3, 0),
	.wr_en		  = CMD_INFO(0x06, 0, 0, 0),
	.en4byte	  = CMD_INFO(0, 0, 0, 0),
	.quad_set	  = ST_INFO(0x31, 1, 1, 1, 1, 0),
	.quad_get	  = ST_INFO(0x35, 1, 1, 1, 1, 0),
	.busy		  = ST_INFO(0x05, 0, 1, 0, 1, 0),

	.tCHSH = 5,
	.tSLCH = 5,
	.tSHSL_RD = 20,
	.tSHSL_WR = 50,

	.chip_size = 16777216,
	.page_size = 256,
	.erase_size = 32768,

	.quad_ops_mode = 1,
	.addr_ops_mode = 0,
#endif

#ifdef CONFIG_INGENIC_XXXXX
	//add your flash
#endif
};


/*
 * params: nor flash partitions
 */
struct norflash_partitions builtin_norflash_partitions = {

	/* max 10 partitions*/
	.num_partition_info = 3,

	.nor_partition = {

		[0].name = "uboot",
		[0].offset = 0x0,
		[0].size =   0x40000,
		[0].mask_flags = NORFLASH_PART_RW,

		[1].name = "kernel",
		[1].offset = 0x40000,
		[1].size =   0x300000,
		[1].mask_flags = NORFLASH_PART_RW,

		[2].name = "rootfs",
		[2].offset = 0x360000,
		[2].size = 0xca0000,
		[2].mask_flags = NORFLASH_PART_RW,
	},
};


/*
 * params: private params
 */
private_params_t builtin_private_params = {
	.fs_erase_size = 32768,
	.uk_quad = 1,
};

