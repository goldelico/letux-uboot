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

#define TRD		400
#define TPP		1000
#define TBE		5

static struct jz_sfcnand_base_param zb_param[] = {

	[0] = {
		/*ZB35Q01A*/
		.pagesize = 2 * 1024,
		.oobsize = 64,
		.blocksize = 2 * 1024 * 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP = TSETUP,
		.tHOLD  = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 8,
		.need_quad = 1,
	},
	[1] = {
		/*ZB35Q01BYIG*/
		.pagesize = 2 * 1024,
		.oobsize = 64,
		.blocksize = 2 * 1024 * 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP = TSETUP,
		.tHOLD  = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 180,
		.tPP = 520,
		.tBE = 3,

		.ecc_max = 8,
		.need_quad = 1,
	},
	[2] = {
		/*ZB35Q02BYIG*/
		.pagesize = 2 * 1024,
		.oobsize = 64,
		.blocksize = 2 * 1024 * 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP = TSETUP,
		.tHOLD  = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 180,
		.tPP = 520,
		.tBE = 3,

		.ecc_max = 8,
		.need_quad = 1,
	},
	[3] = {
		/*ZB35Q04BYIG*/
		.pagesize = 2 * 1024,
		.oobsize = 128,
		.blocksize = 2 * 1024 * 128,
		.flashsize = 2 * 1024 * 128 * 2048,

		.tSETUP = TSETUP,
		.tHOLD  = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 400,
		.tPP = 10000,
		.tBE = 5,

		.ecc_max = 8,
		.need_quad = 1,
	},
};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0x41,   "ZB35Q01A", &zb_param[0]),
	DEVICE_ID_STRUCT(0xA1A1, "ZB35Q01B", &zb_param[1]),
	DEVICE_ID_STRUCT(0xA2A1, "ZB35Q02B", &zb_param[2]),
	DEVICE_ID_STRUCT(0xA3,   "ZB35Q04B", &zb_param[3]),
};

static int32_t zb_get_read_feature(struct flash_operation_message *op_info)
{

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct sfc_transfer transfer;
	uint16_t device_id = nand_info->id_device;
	uint32_t ecc_status;

retry:
	ecc_status = 0;
	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	transfer.cmd_info.cmd = SPINAND_CMD_GET_FEATURE;
	transfer.sfc_mode = TM_STD_SPI;

	transfer.addr = SPINAND_ADDR_STATUS;
	transfer.addr_len = 1;

	transfer.cmd_info.dataen = ENABLE;
	transfer.data = (uint8_t *)&ecc_status;
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
		case 0x41:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
				case 0x1:
					return 0;
				case 0x3:
					return 8;
				case 0x2:
					return -EBADMSG;
				default:
					break;
			}
		case 0xA1A1:
		case 0xA2A1:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
					return 8;
				case 0x2:
					return -EBADMSG;
				default:
					break;
			}
		case 0xA3:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
					return 4;
				case 0x2:
					return -EBADMSG;
				case 0x3:
					return 8;
				default:
					break;
			}
		default:
			printf("device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			break;
	}
	return -EINVAL;
}

static int zb_nand_init(void) {
	struct jz_sfcnand_device *zb_nand;
	zb_nand = kzalloc(sizeof(*zb_nand), GFP_KERNEL);
	if(!zb_nand) {
		pr_err("alloc zb_nand struct fail\n");
		return -ENOMEM;
	}

	zb_nand->id_manufactory = 0x5E;
	zb_nand->id_device_list = device_id;
	zb_nand->id_device_count = ARRAY_SIZE(zb_param);

	zb_nand->ops.nand_read_ops.get_feature = zb_get_read_feature;
	return jz_sfcnand_register(zb_nand);
}

SPINAND_MOUDLE_INIT(zb_nand_init);
