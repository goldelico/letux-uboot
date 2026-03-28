#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		20
#define	TSHSL_W		20

#define TRD		80
#define TRD_4G		120
#define TPP		700
#define TBE		5

#define TRD_Q5	        50
#define TPP_Q5	        600
#define TRD_QH          60

static struct jz_sfcnand_base_param gd_param[] = {

	[0] = {
		/*GD5F1GQ4UB*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[1] = {
		 /*GD5F2GQ4UB*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[2] = {
		/*GD5F4GQ4UB*/
		.pagesize = 4 * 1024,
		.blocksize = 4 * 1024 * 64,
		.oobsize = 256,
		.flashsize = 4 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD_4G,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[3] = {
		/*GD5F1GQ4UC*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[4] = {
		/*GD5F2GQ4UC*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
#if 1
	[5] = {
		/*GD5F4GQ4UC*/
		.pagesize = 4 * 1024,
		.blocksize = 4 * 1024 * 64,
		.oobsize = 256,
		.flashsize = 4 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD_4G,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
#else
      [5] = {
		/*GD5F4GM5UF*/
		.pagesize = 4 * 1024,
		.blocksize = 4 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 4 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD_4G,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
#endif
      [6] = {
		/*GD5F1GQ4RF*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x8,
		.need_quad = 1,
	},

	[7] = {
		/*GD5F1GQ5UE*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD_Q5,
		.tPP = TPP_Q5,
		.tBE = TBE,

		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[8] = {
		/*GD5F2GQ5UE*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD_Q5,
		.tPP = TPP_Q5,
		.tBE = TBE,

		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[9] = {
		/*GD5F4GQ6UE*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 4096,

		.tSETUP  = 20,
		.tHOLD   = 20,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[10] = {
		/*GD5F2GQ5UF*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD_QH,
		.tPP = TPP_Q5,
		.tBE = TBE,

		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[11] = {
		/*GD5F2GM7UE*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 120,
		.tPP = TPP_Q5,
		.tBE = 10,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[12] = {
		/*GD5F1GM7UE*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 120,
		.tPP = TPP_Q5,
		.tBE = 10,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[13] = {
		/*GD5F2GQ5UExxH*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD_QH,
		.tPP = TPP_Q5,
		.tBE = TBE,

		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[14] = {
		/*GD5F4GM8UE*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 4096,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 120,
		.tPP = TPP_Q5,
		.tBE = 10,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[15] = {
		/*GD5F1GQ4UExxH*/
		/* Not aging test */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 80,
		.tPP = 700,
		.tBE = 5,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0xD1,   "GD5F1GQ4UB",    &gd_param[0]),
	DEVICE_ID_STRUCT(0xD2,   "GD5F2GQ4UB",    &gd_param[1]),
	DEVICE_ID_STRUCT(0xD4,   "GD5F4GQ4UB",    &gd_param[2]),
	DEVICE_ID_STRUCT(0xB1,   "GD5F1GQ4UC",    &gd_param[3]),
	DEVICE_ID_STRUCT(0xB2,   "GD5F2GQ4UC",    &gd_param[4]),
	DEVICE_ID_STRUCT(0xB468, "GD5F4GQ4UC",    &gd_param[5]),
	DEVICE_ID_STRUCT(0xA1,   "GD5F1GQ4RF",    &gd_param[6]),
	DEVICE_ID_STRUCT(0x51,   "GD5F1GQ5UE",    &gd_param[7]),
	DEVICE_ID_STRUCT(0x52,   "GD5F2GQ5UE",    &gd_param[8]),
	DEVICE_ID_STRUCT(0x55,   "GD5F4GQ6UE",    &gd_param[9]),
	DEVICE_ID_STRUCT(0x61,   "GD5F2GQ5UF",    &gd_param[10]),
	DEVICE_ID_STRUCT(0x92,   "GD5F2GM7UE",    &gd_param[11]),
	DEVICE_ID_STRUCT(0x91,   "GD5F1GM7UE",    &gd_param[12]),
	DEVICE_ID_STRUCT(0x32,   "GD5F2GQ5UExxH", &gd_param[13]),
	DEVICE_ID_STRUCT(0x95,   "GD5F4GM8UE",    &gd_param[14]),
	DEVICE_ID_STRUCT(0xD9,   "GD5F1GQ4UExxH", &gd_param[15]),
};

static void gd_single_read(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	uint16_t device_id = nand_info->id_device;
	uint8_t addr_len = 0;
	switch(device_id) {
	    case 0xB1 ... 0xB4:
	    case 0xA1:
		case 0x61:
	    case 0xB468:
			addr_len = 3;
			break;
	    case 0xD1 ... 0xD4:
	    case 0x51 ... 0x55:
	    case 0x32:
	    case 0x92:
	    case 0x91:
		case 0x95:
		case 0xd9:
			addr_len = 2;
			break;
		default:
			printf("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			addr_len = 2;
			break;
	}

	transfer->cmd_info.cmd = SPINAND_CMD_FRCH;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = op_info->columnaddr;
	transfer->addr_len = addr_len;

	transfer->cmd_info.dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_READ;

	transfer->data_dummy_bits = 8;
	transfer->ops_mode = DMA_OPS;

	return;
}

static void gd_quad_read(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	uint16_t device_id = nand_info->id_device;
	uint8_t addr_len = 0;
	switch(device_id) {
		case 0xB1 ... 0xB4:
		case 0xA1:
		case 0x61:
		case 0xB468:
			addr_len = 3;
			break;
		case 0xD1 ... 0xD4:
		case 0x51 ... 0x55:
		case 0x91:
		case 0x92:
		case 0x95:
		case 0xd9:
			addr_len = 2;
			break;
		default:
			printf("device_id err, please check your device id: device_id = 0x%02x\n", device_id);
			addr_len = 2;
		    break;
	}
	transfer->cmd_info.cmd = SPINAND_CMD_RDCH_X4;
	transfer->sfc_mode = TM_QI_QO_SPI;

	transfer->addr = op_info->columnaddr;
	transfer->addr_len = addr_len;

	transfer->cmd_info.dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_READ;

	transfer->data_dummy_bits = 8;
	transfer->ops_mode = DMA_OPS;

	return;
}

static int32_t gd_get_read_feature(struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct sfc_transfer transfer;
	uint16_t device_id = nand_info->id_device;
	uint8_t ecc_status = 0;
	int32_t ret = 0;

retry:
	ecc_status = 0;
	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	transfer.cmd_info.cmd = SPINAND_CMD_GET_FEATURE;
	transfer.sfc_mode = TM_STD_SPI;

	transfer.addr = SPINAND_ADDR_STATUS;
	transfer.addr_len = 1;

	transfer.cmd_info.dataen = ENABLE;
	transfer.data = &ecc_status;
	transfer.len = 1;
	transfer.direction = GLB_TRAN_DIR_READ;

	transfer.data_dummy_bits = 0;
	transfer.ops_mode = CPU_OPS;

	if(sfc_sync(flash->sfc, &transfer)) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	if(ecc_status & SPINAND_IS_BUSY)
		goto retry;

	switch(device_id) {
		case 0x61:
			switch((ecc_status >> 4) & 0x7) {
				case 0x0:
					return 0;
				case 0x1 ... 0x4:
					return ((ecc_status >> 4) & 0x7);
				case 0x7:
					return -EBADMSG;
				default:
					break;
			}
			break;
		case 0xA1:
		case 0xB1 ... 0xB4:
		case 0xB468:
			switch((ecc_status >> 4) & 0x7) {
				case 0x0:
					return 0;
				case 0x1 ... 0x6:
					return ((ecc_status >> 4) & 0x7) + 2;
				case 0x7:
					return -EBADMSG;
				default:
					break;
			}
			break;
		case 0xD1 ... 0xD4:
		case 0x92:
		case 0x91:
		case 0x95:
		case 0xD9:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
					ret = nand_get_ecc_conf(flash, 0xf0);
					if (ret < 0)
						return ret;
					return ((ret >> 4) & 0x3) + 4;
				case 0x2:
					return -EBADMSG;
				case 0x3:
					return 8;
				default:
					break;
			}
			break;
		case 0x51:
		case 0x55:
		case 0x52:
		case 0x32:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
					ret = nand_get_ecc_conf(flash, 0xf0);
					if (ret < 0)
						return ret;
					return ((ret >> 4) & 0x3) + 1;
				case 0x2:
					return -EBADMSG;
					break;
				default:
					break;
			}
			break;
		default:
			printf("device_id err,it maybe don`t support this device, please check your device id: device_id = 0x%02x\n", device_id);
			break;
	}

	return -EINVAL;
}

static int gd_nand_init(void) {
	struct jz_sfcnand_device *gd_nand;
	gd_nand = kzalloc(sizeof(*gd_nand), GFP_KERNEL);
	if(!gd_nand) {
		pr_err("alloc gd_nand struct fail\n");
		return -ENOMEM;
	}

	gd_nand->id_manufactory = 0xC8;
	gd_nand->id_device_list = device_id;
	gd_nand->id_device_count = ARRAY_SIZE(gd_param);

	gd_nand->ops.nand_read_ops.get_feature = gd_get_read_feature;
	gd_nand->ops.nand_read_ops.single_read = gd_single_read;
	gd_nand->ops.nand_read_ops.quad_read = gd_quad_read;

	return jz_sfcnand_register(gd_nand);
}
SPINAND_MOUDLE_INIT(gd_nand_init);
