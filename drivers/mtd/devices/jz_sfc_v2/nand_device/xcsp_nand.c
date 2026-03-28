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

static struct jz_sfcnand_device *xcsp_nand;
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

		.plane_select = 0,
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

		.plane_select = 0,
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

		.plane_select = 0,
		.ecc_max = 0x8,
		.need_quad = 1,
	},

};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0x01, "XCSP1AAWH ", &xcsp_param[0]),
	DEVICE_ID_STRUCT(0xA1, "XCSP2AAWH ", &xcsp_param[1]),
	DEVICE_ID_STRUCT(0xB1, "XCSP4AAWH ", &xcsp_param[2]),
};

static cdt_params_t *xcsp_nand_get_cdt_params(struct sfc_flash *flash, uint16_t device_id) {
	CDT_PARAMS_INIT(xcsp_nand->cdt_params);
	switch(device_id) {
		case 0x01:
		case 0xA1:
		case 0xB1:
			break;
		default:
			pr_err("device_id err, please check your device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}
	return &xcsp_nand->cdt_params;
}

static inline int deal_ecc_status(struct sfc_flash *flash, uint16_t device_id, uint8_t ecc_status) {

	int ret = 0;
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
	xcsp_nand = kzalloc(sizeof(*xcsp_nand), GFP_KERNEL);
	if(!xcsp_nand) {
		pr_err("alloc xcsp_nand struct fail\n");
		return -ENOMEM;
	}

	xcsp_nand->id_manufactory = 0x9C;
	xcsp_nand->id_device_list = device_id;
	xcsp_nand->id_device_count = ARRAY_SIZE(xcsp_param);
	xcsp_nand->ops.get_cdt_params = xcsp_nand_get_cdt_params;
	xcsp_nand->ops.deal_ecc_status = deal_ecc_status;
	xcsp_nand->ops.get_feature = NULL;
	return jz_sfcnand_register(xcsp_nand);
}
SPINAND_MOUDLE_INIT(xcsp_nand_init);
