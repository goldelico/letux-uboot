#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define TSETUP		5
#define THOLD		5
#define TSHSL_R	20
#define TSHSL_W	20

#define TRD		450
#define TPP		800
#define TBE		10


static struct jz_sfcnand_base_param gsto_param[] = {
	[0] = {
		/*GSS01GSAX1*/
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
};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0xCA13, "GSS01GSAX1", &gsto_param[0]),
};

static int32_t gsto_get_read_feature(struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct sfc_transfer transfer;
	uint16_t device_id = nand_info->id_device;
	uint8_t ecc_status = 0;

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
		case 0xCA13:
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
			break;
		default:
			pr_err("device_id err,it maybe don`t support this device, please check your device id: device_id = 0x%02x\n", device_id);
			break;
	}
	return -EINVAL;
}

static int gsto_nand_init(void) {
	struct jz_sfcnand_device *gsto_nand;
	gsto_nand = kzalloc(sizeof(*gsto_nand), GFP_KERNEL);
	if(!gsto_nand) {
		pr_err("alloc gsto_nand struct fail\n");
		return -ENOMEM;
	}

	gsto_nand->id_manufactory = 0x52;
	gsto_nand->id_device_list = device_id;
	gsto_nand->id_device_count = ARRAY_SIZE(gsto_param);

	gsto_nand->ops.nand_read_ops.get_feature = gsto_get_read_feature;
	return jz_sfcnand_register(gsto_nand);
}

SPINAND_MOUDLE_INIT(gsto_nand_init);
