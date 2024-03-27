#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"
#include <ubi_uboot.h>

#define	YHY_MIDC9_DEVICES_NUM         2
#define TSETUP		2
#define THOLD		4
#define	TSHSL_R		20
#define	TSHSL_W		50

#define TRD		200
#define TPP		800
#define TBE		10

static struct jz_sfcnand_device *yhy_midc9_nand;
static struct jz_sfcnand_base_param yhy_midc9_param[YHY_MIDC9_DEVICES_NUM] = {

	[0] = {
		/*HYF1GQ4U */
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

		.ecc_max = 0x1,
		.need_quad = 1,
	},

	[1] = {
		/*HYF2GQ4U */
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

		.ecc_max = 0x1,
		.need_quad = 1,
	},

};

static struct device_id_struct device_id[YHY_MIDC9_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0x21, "HYF1GQ4U", &yhy_midc9_param[0]),
	DEVICE_ID_STRUCT(0x52, "HYF2GQ4U", &yhy_midc9_param[1]),
};

static cdt_params_t *yhy_midc9_get_cdt_params(struct sfc_flash *flash, uint8_t device_id) {
	CDT_PARAMS_INIT(yhy_midc9_nand->cdt_params);
	switch(device_id) {
		case 0x21:
		case 0x52:
			break;
		default:
			pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}
	return &yhy_midc9_nand->cdt_params;
}

static inline int deal_ecc_status(struct sfc_flash *flash, uint8_t device_id, uint8_t ecc_status) {
	int ret = 0;
	switch(device_id) {
		case 0x21:
		case 0x52:
			switch((ecc_status >> 4) & 0x3) {
				case 0x00:
					ret = 0;
					break;
				case 0x01:
					ret = 0x1;
					break;
				default:
					ret = -EBADMSG;
			}
			break;
		default:
			printf("device_id err, it maybe don`t support this device, check your         device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;
	}
	return ret;
}

static int yhy_midc9_nand_init(void) {
	yhy_midc9_nand = kzalloc(sizeof(*yhy_midc9_nand), GFP_KERNEL);
	if(!yhy_midc9_nand) {
		pr_err("alloc yhy_midc9_nand struct fail\n");
		return -ENOMEM;
	}

	yhy_midc9_nand->id_manufactory = 0xC9;
	yhy_midc9_nand->id_device_list = device_id;
	yhy_midc9_nand->id_device_count = YHY_MIDC9_DEVICES_NUM;
	yhy_midc9_nand->ops.get_cdt_params = yhy_midc9_get_cdt_params;
	yhy_midc9_nand->ops.deal_ecc_status = deal_ecc_status;
	yhy_midc9_nand->ops.get_feature = NULL;
	return jz_sfcnand_register(yhy_midc9_nand);
}
SPINAND_MOUDLE_INIT(yhy_midc9_nand_init);
