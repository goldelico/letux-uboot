#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define FS_DEVICES_NUM         6
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		20
#define	TSHSL_W		20

#define TRD		180
#define TPP		400
#define TBE		3

static struct jz_sfcnand_base_param fs_param[FS_DEVICES_NUM] = {

	[0] = {
		/*FS35ND01G-V1*/
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

		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[1] = {
		/*FS35ND01G-V2*/
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

		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[2] = {
		/*FS35ND02G*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 450,
		.tPP = 800,
		.tBE = 10,

		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[3] = {
		/*FS35ND01G-S1*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 450,
		.tPP = 800,
		.tBE = 10,

		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[4] = {
		/*FS35SQA001G*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 60,
		.tPP = 700,
		.tBE = 10,

		.ecc_max = 0x1,
		.need_quad = 1,
	},
	[5] = {
		/*FS35SQA512M*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 512,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 60,
		.tPP = 700,
		.tBE = 10,

		.ecc_max = 0x1,
		.need_quad = 1,
	},
};

static struct device_id_struct device_id[FS_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0xA1, "FS35ND01G-V1", &fs_param[0]),
	DEVICE_ID_STRUCT(0xB1, "FS35ND01G-V2", &fs_param[1]),
	DEVICE_ID_STRUCT(0xEB, "FS35ND02G",    &fs_param[2]),
	DEVICE_ID_STRUCT(0xEA, "FS35ND01G-S1", &fs_param[3]),
	DEVICE_ID_STRUCT(0x71, "F35SQA001G",   &fs_param[4]),
	DEVICE_ID_STRUCT(0x70, "F35SQA512M",   &fs_param[5]),
};

static int32_t fs_get_read_feature(struct flash_operation_message *op_info) {

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
		case 0xA1:
		case 0xB1:
			switch((ret = ((ecc_status >> 4) & 0x7))) {
				case 0x0 ... 0x4:
					break;
				default:
					ret = -EBADMSG;
			}
			break;

		case 0xEB:
		case 0xEA:
			switch((ret = ((ecc_status >> 4) & 0x3))) {
				case 0x2:
					ret = -EBADMSG;
					break;
				default:
					ret = 0;
			}
			break;
		case 0x70:
		case 0x71:
			switch((ret = ((ecc_status >> 4) & 0x3))) {
				case 0x0 ... 0x1:
					break;
				default:
					ret = -EBADMSG;
			}
			break;
		default:
			printf("device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;   //notice!!!

	}
	return ret;
}

static int fs_nand_init(void) {
	struct jz_sfcnand_device *fs_nand;
	fs_nand = kzalloc(sizeof(*fs_nand), GFP_KERNEL);
	if(!fs_nand) {
		pr_err("alloc fs_nand struct fail\n");
		return -ENOMEM;
	}

	fs_nand->id_manufactory = 0xCD;
	fs_nand->id_device_list = device_id;
	fs_nand->id_device_count = FS_DEVICES_NUM;

	fs_nand->ops.nand_read_ops.get_feature = fs_get_read_feature;
	return jz_sfcnand_register(fs_nand);
}
SPINAND_MOUDLE_INIT(fs_nand_init);
