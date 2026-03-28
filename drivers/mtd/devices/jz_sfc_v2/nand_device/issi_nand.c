#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define THOLD	    5
#define TSETUP	    5
#define TSHSL_R	    100
#define TSHSL_W	    100

#define TRD	    100
#define TPP	    400
#define TBE	    10

struct jz_sfcnand_device *issi_nand;

static struct jz_sfcnand_base_param issi_param[] = {
	[0] = {
		/*IS37SML01G1*/
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
		.ecc_max = 0x1,
		.need_quad = 1,
	},
};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0x21, "IS37SML01G1", &issi_param[0]),
};

static cdt_params_t *issi_get_cdt_params(struct sfc_flash *flash, uint16_t device_id)
{
	CDT_PARAMS_INIT(issi_nand->cdt_params);
	switch(device_id) {
		case 0x21:
			break;
		default:
			pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}
	return &issi_nand->cdt_params;
}

static inline int deal_ecc_status(struct sfc_flash *flash, uint16_t device_id, uint8_t ecc_status)
{
	switch(device_id) {
		case 0x21:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
					return 1;
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


static int issi_nand_init(void) {

	issi_nand = kzalloc(sizeof(*issi_nand), GFP_KERNEL);
	if(!issi_nand) {
		pr_err("alloc issi_nand struct fail\n");
		return -ENOMEM;
	}

	issi_nand->id_manufactory = 0xC8;
	issi_nand->id_device_list = device_id;
	issi_nand->id_device_count = ARRAY_SIZE(issi_param);

	issi_nand->ops.get_cdt_params = issi_get_cdt_params;
	issi_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	issi_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(issi_nand);
}

SPINAND_MOUDLE_INIT(issi_nand_init);
