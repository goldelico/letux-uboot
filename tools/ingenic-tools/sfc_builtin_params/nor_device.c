#include <config.h>
#include "nor_device.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/*
 * Only one is supported at a time
 */
//#define CONFIG_INGENIC_GD25Q127C
//#define CONFIG_INGENIC_XXXXX

#ifdef CONFIG_NOR_COMMON_PARAMS

struct nor_id nor_id_list_1[] = {
	/* cmd_type: NOR_CMD_TYPE_1
         *      write status register command 0x01
         *      data length 1 byte. */
        {"GD25Q256DYIG", 0xc84019}, // id repeated
        {"MX25L12845G",  0xc22018},
        {"MX25L25645G",  0xc22019},
        {"MX25L6433F",   0xc22017},
        {"XM25QH128B",   0x206018},
        {"XM25QU128B",   0x205018},
        {"XM25QH256B",   0x206019}, // id repeated
        {"XM25QH128D",   0x204018},
        {"IS25LP128",    0x9d6018},
        {"IS25WP128",    0x9d7018},
        {"IS25WP256E",   0x9d6019},
        {"SM25QU01GMP",  0x20bb21},
        {"SM25QH256M",   0x206019}, // id repeated
};

struct nor_id nor_id_list_2[] = {
	/* cmd_type: NOR_CMD_TYPE_2
         *      write status register command 0x01
         *      data length 2 bytes. */
        {"GD25LQ128C",   0xc86018},
        {"GD25LQ256C",   0xc86019},
        {"GD25Q16CSIG",  0xc84015},
        {"W25Q64FV",     0xef4017}, // id repeated
        {"XTX25F16B",    0x0b4015},
        {"XT25F128B",    0x0b4018}, // id repeated
        {"XT25F64F",     0x0b4017},
        {"FM25Q16A",     0xa14015},
};


struct nor_id nor_id_list_3[] = {
	/* cmd_type: NOR_CMD_TYPE_3
         *      write status register command 0x31
         *      data length 1 byte */
        {"GD25Q64CSIG",  0xc84017},
        {"GD25Q32CSIG",  0xc84016},
        {"GD25Q127CSIG", 0xc84018},
        {"GD25S512MD",   0xC84019}, // id repeated
        {"GD25Q256DYIG", 0xc84019}, // id repeated
        {"W25Q128JVSQ",  0xef4018},
        {"W25Q256JVEQ",  0xef4019},
        {"W25Q16JVSSIQ", 0xef4015},
        {"W25Q16FWSSIQ", 0xef6015},
        {"W25Q128FWSIQ", 0xef6018},
        {"W25Q128JVSM",  0xef7018},
        {"W25Q32JV",     0xef4016},
        {"W25Q64JV",     0xef4017}, // id repeated
        {"BY25Q256FS",   0x684919},
        {"BY25Q128AS",   0x684018},
        {"BY25Q64AS",    0x684017},
        {"EN25QH128A",   0x1c7018},
        {"DS25M4BA",     0xe54219},
        {"FM25Q128X",    0xa14018},
        {"FM25M4AA",     0xf84218},
        {"XM25QH256C",   0x204019},
        {"XM25QH32C",    0x204016},
        {"W25Q512JV",    0xef4020},
        {"PY25Q64HA",    0x852017},
        {"PY25Q64H",     0x856017},
        {"FM25Q64",      0xa14017},
        {"PY25Q64H",     0x856015},
        {"PY25Q128H",    0x852018},
        {"PY25Q256HB",   0x852019},
        {"XT25F128FW",   0x0b4018}, // id repeated
        {"XT25F256BWSIG",0x0b4019},
        {"ZB25VQ64A",    0x5e4017},
        {"ZB25VQ128D",   0x5e4018},
};


struct nor_id_info nor_id_info_list[] = {
        {
                .cmd_type = NOR_CMD_TYPE_1,
                .id_count = ARRAY_SIZE(nor_id_list_1),
                .id_list = nor_id_list_1,
        },
        {
                .cmd_type = NOR_CMD_TYPE_2,
                .id_count = ARRAY_SIZE(nor_id_list_2),
                .id_list = nor_id_list_2,
        },
        {
                .cmd_type = NOR_CMD_TYPE_3,
                .id_count = ARRAY_SIZE(nor_id_list_3),
                .id_list = nor_id_list_3,
        },
};


int nor_id_info_count = ARRAY_SIZE(nor_id_info_list);

#endif

/*
 * params: spi nor info
 * useage:
 *	set cmmand info:
 *		CMD_INFO(Command, Dummy bits, Address len, Transfer mode)
 *	set status info:
 *		ST_INFO(Command, Bit shift, Mask, Val, Len, Dummy bits)
 */
struct spi_nor_info builtin_spi_nor_info[] = {
#ifdef CONFIG_NOR_COMMON_PARAMS
        {
                .name = "nor common params type 1",
                .id = NOR_CMD_TYPE_1,

                .read_standard	  = CMD_INFO(0x03, 0, 3, 0),
                .read_quad	  = CMD_INFO(0x6b, 8, 3, 5),
                .write_standard	  = CMD_INFO(0x02, 0, 3, 0),
                .write_quad	  = CMD_INFO(0x32, 0, 3, 5),
                .sector_erase	  = CMD_INFO(0x52, 0, 3, 0),
                .wr_en		  = CMD_INFO(0x06, 0, 0, 0),
                .en4byte	  = CMD_INFO(0, 0, 0, 0),
                .quad_set	  = ST_INFO(0x01, 6, 1, 1, 1, 0),
                .quad_get	  = ST_INFO(0x05, 6, 1, 1, 1, 0),
                .busy		  = ST_INFO(0x05, 0, 1, 0, 1, 0),

                .tCHSH = 10,
                .tSLCH = 10,
                .tSHSL_RD = 50,
                .tSHSL_WR = 50,

                .chip_size = 16777216,
                .page_size = 256,
                .erase_size = 32768,

                .quad_ops_mode = 1,
                .addr_ops_mode = 0,
        },
        {
                .name = "nor common params type 2",
                .id = NOR_CMD_TYPE_2,

                .read_standard	  = CMD_INFO(0x03, 0, 3, 0),
                .read_quad	  = CMD_INFO(0x6b, 8, 3, 5),
                .write_standard	  = CMD_INFO(0x02, 0, 3, 0),
                .write_quad	  = CMD_INFO(0x32, 0, 3, 5),
                .sector_erase	  = CMD_INFO(0x52, 0, 3, 0),
                .wr_en		  = CMD_INFO(0x06, 0, 0, 0),
                .en4byte	  = CMD_INFO(0, 0, 0, 0),
                .quad_set	  = ST_INFO(0x01, 9, 1, 1, 2, 0),
                .quad_get	  = ST_INFO(0x35, 1, 1, 1, 1, 0),
                .busy		  = ST_INFO(0x05, 0, 1, 0, 1, 0),

                .tCHSH = 10,
                .tSLCH = 10,
                .tSHSL_RD = 50,
                .tSHSL_WR = 50,

                .chip_size = 16777216,
                .page_size = 256,
                .erase_size = 32768,

                .quad_ops_mode = 1,
                .addr_ops_mode = 0,
        },
        {
                .name = "nor common params type 3",
                .id = NOR_CMD_TYPE_3,

                .read_standard	  = CMD_INFO(0x03, 0, 3, 0),
                .read_quad	  = CMD_INFO(0x6b, 8, 3, 5),
                .write_standard   = CMD_INFO(0x02, 0, 3, 0),
                .write_quad	  = CMD_INFO(0x32, 0, 3, 5),
                .sector_erase	  = CMD_INFO(0x52, 0, 3, 0),
                .wr_en		  = CMD_INFO(0x06, 0, 0, 0),
                .en4byte	  = CMD_INFO(0, 0, 0, 0),
                .quad_set	  = ST_INFO(0x31, 1, 1, 1, 1, 0),
                .quad_get	  = ST_INFO(0x35, 1, 1, 1, 1, 0),
                .busy		  = ST_INFO(0x05, 0, 1, 0, 1, 0),

                .tCHSH = 10,
                .tSLCH = 10,
                .tSHSL_RD = 50,
                .tSHSL_WR = 50,

                .chip_size = 16777216,
                .page_size = 256,
                .erase_size = 32768,

                .quad_ops_mode = 1,
                .addr_ops_mode = 0,
        },
#endif


#ifdef CONFIG_INGENIC_GD25Q127C
	/* GD25Q127C */
        {
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
        },
#endif

#ifdef CONFIG_INGENIC_XXXXX
	//add your flash
#endif
};

int builtin_params_count = ARRAY_SIZE(builtin_spi_nor_info);

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
		[0].mask_flags = PART_RW,

		[1].name = "kernel",
		[1].offset = 0x40000,
		[1].size =   0x300000,
		[1].mask_flags = PART_RW,

		[2].name = "rootfs",
		[2].offset = 0x360000,
		[2].size = 0xca0000,
		[2].mask_flags = PART_RW,
	},
};


/*
 * params: private params
 */
private_params_t builtin_private_params = {
	.fs_erase_size = 32768,
	.uk_quad = 1,
};

