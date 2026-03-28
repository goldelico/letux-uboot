#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define THOLD	    5
#define TSETUP	    5
#define TSHSL_R	    20
#define TSHSL_W	    20

#define TRD	    450
#define TPP	    800
#define TBE	    10

struct jz_sfcnand_device *hik_nand;

static struct jz_sfcnand_base_param hik_param[] = {
	[0] = {
		/*HSESYHDSW1G*/
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
		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[1] = {
		/*HSESYHDSW2G*/
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

		.plane_select = 0,
		.ecc_max = 0x4,
		.need_quad = 1,
	},
};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0xD1D1, "HSESYHDSW1G", &hik_param[0]),
	DEVICE_ID_STRUCT(0xD2D2, "HSESYHDSW2G", &hik_param[1]),
};

static cdt_params_t *hik_get_cdt_params(struct sfc_flash *flash, uint16_t device_id)
{
	CDT_PARAMS_INIT(hik_nand->cdt_params);
	switch(device_id) {
		case 0xD1D1:
		case 0xD2D2:
			break;
		default:
			pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}
	return &hik_nand->cdt_params;
}

static inline int deal_ecc_status(struct sfc_flash *flash, uint16_t device_id, uint8_t ecc_status)
{
	switch(device_id) {
		case 0xD1D1:
		case 0xD2D2:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
					return 4;
				case 0x2:
					return -EBADMSG;
				default:
					break;
			}
			break;
		default:
			printf("device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			break;
	}
	return -EINVAL;
}


static int hik_nand_init(void) {

	hik_nand = kzalloc(sizeof(*hik_nand), GFP_KERNEL);
	if(!hik_nand) {
		pr_err("alloc hik_nand struct fail\n");
		return -ENOMEM;
	}

	hik_nand->id_manufactory = 0x3C;
	hik_nand->id_device_list = device_id;
	hik_nand->id_device_count = ARRAY_SIZE(hik_param);

	hik_nand->ops.get_cdt_params = hik_get_cdt_params;
	hik_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	hik_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(hik_nand);
}

SPINAND_MOUDLE_INIT(hik_nand_init);
