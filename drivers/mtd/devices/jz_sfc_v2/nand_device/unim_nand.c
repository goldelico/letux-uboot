#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define THOLD	    5
#define TSETUP	    5
#define TSHSL_R	    30
#define TSHSL_W	    30

#define TRD	    95
#define TPP	    600
#define TBE	    10

struct jz_sfcnand_device *unim_nand;

static struct jz_sfcnand_base_param unim_param[] = {
	[0] = {
		/*UM19A9HISW*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 512,

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
	DEVICE_ID_STRUCT(0x0C, "UM19A9HISW", &unim_param[0]),
};

static cdt_params_t *unim_get_cdt_params(struct sfc_flash *flash, uint16_t device_id)
{
	CDT_PARAMS_INIT(unim_nand->cdt_params);
	switch(device_id) {
		case 0x0C:
			break;
		default:
			pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}
	return &unim_nand->cdt_params;
}

static inline int deal_ecc_status(struct sfc_flash *flash, uint16_t device_id, uint8_t ecc_status)
{
	switch(device_id) {
		case 0x0C:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
					return 2;
				case 0x2:
					return -EBADMSG;
				case 0x3:
					return 4;
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

static int unim_nand_init(void) {

	unim_nand = kzalloc(sizeof(*unim_nand), GFP_KERNEL);
	if(!unim_nand) {
		pr_err("alloc unim_nand struct fail\n");
		return -ENOMEM;
	}

	unim_nand->id_manufactory = 0xB0;
	unim_nand->id_device_list = device_id;
	unim_nand->id_device_count = ARRAY_SIZE(unim_param);

	unim_nand->ops.get_cdt_params = unim_get_cdt_params;
	unim_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	unim_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(unim_nand);
}

SPINAND_MOUDLE_INIT(unim_nand_init);
