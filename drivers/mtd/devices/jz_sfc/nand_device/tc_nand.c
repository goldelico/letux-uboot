#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define TC_DEVICES_NUM         1
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		100
#define	TSHSL_W		100

#define TRD		30
#define TPP		360
#define TBE		2

static struct jz_sfcnand_base_param tc_param[TC_DEVICES_NUM] = {

	[0] = {
		/*TC58CVG0S3HRAIG*/
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
		.need_quad = 0, // unsupport quad
	},

};

static struct device_id_struct device_id[TC_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0xC2, "TC58CVG0S3HRAIG", &tc_param[0]),
};

static int32_t tc_get_read_feature(struct flash_operation_message *op_info) {

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
		case 0xC2:
			switch((ret = ((ecc_status >> 4) & 0x3))) {
				case 0x0:
				case 0x1:
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

static int tc_nand_init(void) {
	struct jz_sfcnand_device *tc_nand;
	tc_nand = kzalloc(sizeof(*tc_nand), GFP_KERNEL);
	if(!tc_nand) {
		pr_err("alloc tc_nand struct fail\n");
		return -ENOMEM;
	}

	tc_nand->id_manufactory = 0x98;
	tc_nand->id_device_list = device_id;
	tc_nand->id_device_count = TC_DEVICES_NUM;

	tc_nand->ops.nand_read_ops.get_feature = tc_get_read_feature;
	return jz_sfcnand_register(tc_nand);
}
SPINAND_MOUDLE_INIT(tc_nand_init);
