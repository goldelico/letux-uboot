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

#define TRD		120
#define TPP		600
#define TBE		8

static struct jz_sfcnand_base_param xcsp_param[] = {

	[0] = {
		/*XCSP1AAWH */
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

	[1] = {
		/*XCSP2AAWH */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
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
		/*XCSP4AAWH */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 4096,

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

};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0x01, "XCSP1AAWH ", &xcsp_param[0]),
	DEVICE_ID_STRUCT(0xA1, "XCSP2AAWH ", &xcsp_param[1]),
	DEVICE_ID_STRUCT(0xB1, "XCSP4AAWH ", &xcsp_param[2]),
};

static int32_t xcsp_get_read_feature(struct flash_operation_message *op_info) {

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
		case 0x01:
		case 0xA1:
		case 0xB1:
			ret = nand_get_ecc_conf(flash, 0xf0);
			switch((ret >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
				case 0x2:
					return 8;
				case 0x3:
					return -EBADMSG;
			}
			break;
		default:
			printf("device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			break;
	}
	return -EINVAL;
}

static int xcsp_nand_init(void) {
	struct jz_sfcnand_device *xcsp_nand;
	xcsp_nand = kzalloc(sizeof(*xcsp_nand), GFP_KERNEL);
	if(!xcsp_nand) {
		pr_err("alloc xcsp_nand struct fail\n");
		return -ENOMEM;
	}

	xcsp_nand->id_manufactory = 0x9C;
	xcsp_nand->id_device_list = device_id;
	xcsp_nand->id_device_count = ARRAY_SIZE(xcsp_param);

	xcsp_nand->ops.nand_read_ops.get_feature = xcsp_get_read_feature;
	return jz_sfcnand_register(xcsp_nand);
}
SPINAND_MOUDLE_INIT(xcsp_nand_init);
