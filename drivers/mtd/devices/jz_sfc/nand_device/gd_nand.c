#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define GD_DEVICES_NUM          11
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		20
#define	TSHSL_W		20

#define TRD_4G	        120

#define TRD		80
#define TPP		700
#define TBE		5

#define TRD_Q5	        50
#define TPP_Q5	        600

static struct jz_sfcnand_base_param gd_param[GD_DEVICES_NUM] = {

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

	[6] = {
		/*GD5F1GQ4RF9IG*/
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

		.tRD = TRD_Q5,
		.tPP = TPP_Q5,
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

		.tRD = TRD_Q5,
		.tPP = TPP_Q5,
		.tBE = TBE,

		.ecc_max = 0x4,

		.need_quad = 1,
	},

};

static struct device_id_struct device_id[GD_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0xD1, "GD5F1GQ4UB",&gd_param[0]),
	DEVICE_ID_STRUCT(0xD2, "GD5F2GQ4UB",&gd_param[1]),
	DEVICE_ID_STRUCT(0xD4, "GD5F4GQ4UB",&gd_param[2]),
	DEVICE_ID_STRUCT(0xB1, "GD5F1GQ4UC",&gd_param[3]),
	DEVICE_ID_STRUCT(0xB2, "GD5F2GQ4UC",&gd_param[4]),
	DEVICE_ID_STRUCT(0xB4, "GD5F4GQ4UC",&gd_param[5]),
	DEVICE_ID_STRUCT(0xA1, "GD5F1GQ4RF",&gd_param[6]),
	DEVICE_ID_STRUCT(0x51, "GD5F1GQ5UE",&gd_param[7]),
	DEVICE_ID_STRUCT(0x52, "GD5F2GQ5UE",&gd_param[8]),
	DEVICE_ID_STRUCT(0x55, "GD5F4GQ6UE",&gd_param[9]),
	DEVICE_ID_STRUCT(0x61, "GD5F2GQ5UF",&gd_param[10]),
};

static void gd_single_read(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	uint8_t device_id = nand_info->id_device;
	uint8_t addr_len = 0;
	switch(device_id) {
		case 0xB1 ... 0xB4:
		case 0xA1:
		case 0x61:
			addr_len = 3;
			break;
		case 0xD1 ... 0xD4:
		case 0x51 ... 0x55:
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
	uint8_t device_id = nand_info->id_device;
	uint8_t addr_len = 0;
	switch(device_id) {
		case 0xB1 ... 0xB4:
		case 0xA1:
		case 0x61:
			addr_len = 3;
			break;
		case 0xD1 ... 0xD4:
		case 0x51 ... 0x55:
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

static int32_t gd_get_f0_register_value(struct sfc_flash *flash) {

	struct sfc_transfer transfer;
	uint32_t buf = 0;

	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	transfer.cmd_info.cmd = SPINAND_CMD_GET_FEATURE;
	transfer.sfc_mode = TM_STD_SPI;

	transfer.addr = 0xf0;
	transfer.addr_len = 1;

	transfer.cmd_info.dataen = ENABLE;
	transfer.len = 1;
	transfer.data = (uint8_t *)&buf;
	transfer.direction = GLB_TRAN_DIR_READ;

	transfer.data_dummy_bits = 0;
	transfer.ops_mode = CPU_OPS;

	if(sfc_sync(flash->sfc, &transfer)) {
	        printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}
	return buf;
}

static int32_t gd_get_read_feature(struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct sfc_transfer transfer;
	uint8_t device_id = nand_info->id_device;
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
		case 0xB1 ... 0xB4:
		case 0xA1:
			switch((ecc_status >> 4) & 0x7) {
				case 0x7:
					ret = -EBADMSG;
					break;
				case 0x6:
					ret = 0x8;
					break;
				case 0x5:
					ret = 0x7;
					break;
				default:
					ret = 0;
					break;
			}
			break;
		case 0xD1 ... 0xD4:
			switch((ecc_status >> 4) & 0x3) {
				case 0x3:
					ret = 0x8;
					break;
				case 0x2:
					ret = -EBADMSG;
					break;
				case 0x1:
					if((ret = gd_get_f0_register_value(flash)) < 0)
						return ret;
					if(((ret >> 4) & 0x3) == 0x3)
						return 0x7;
				default:
					ret = 0;
					break;
			}
			break;
		case 0x51:
		case 0x55:
			switch((ecc_status >> 4) & 0x3) {
				case 0x3:
				case 0x0:
					ret = 0x0;
					break;
				case 0x1:
					ret = 0x4;
					break;
				default:
					ret = -EBADMSG;
			}
			break;
		case 0x52:
			switch((ecc_status >> 4) & 0x3) {
				case 0x1:
					if((ret = gd_get_f0_register_value(flash)) < 0)
						return ret;
					switch((ecc_status >> 4) & 0x3) {
						case 0x0:
							ret = 0x1;
							break;
						case 0x1:
							ret = 0x2;
							break;
						case 0x2:
							ret = 0x3;
							break;
						case 0x3:
							ret = 0x4;
							break;
						default:
							break;
					}
					break;
				case 0x2:
					ret = -EBADMSG;
					break;
				default:
					ret = 0x0;
					break;
			}
			break;
		case 0x61:
			switch((ecc_status >> 4) & 0x7) {
				case 0x7:
					ret = -EBADMSG;
					break;
				default:
					ret = 0;
					break;
			}
			break;
		default:
			printf("device_id err,it maybe don`t support this device, please check your device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;   //notice!!!
	}

	return ret;
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
	gd_nand->id_device_count = GD_DEVICES_NUM;

	gd_nand->ops.nand_read_ops.get_feature = gd_get_read_feature;
	gd_nand->ops.nand_read_ops.single_read = gd_single_read;
	gd_nand->ops.nand_read_ops.quad_read = gd_quad_read;

	return jz_sfcnand_register(gd_nand);
}
SPINAND_MOUDLE_INIT(gd_nand_init);
