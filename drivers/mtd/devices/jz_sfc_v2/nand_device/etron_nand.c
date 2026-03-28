#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define TSETUP                  5
#define THOLD                   5
#define TSHSL_R                 30
#define TSHSL_W                 30

#define TRD                     45
#define TPP                     350
#define TBE                     4

static struct jz_sfcnand_device *etron_nand;

static struct jz_sfcnand_base_param etron_param[] = {

	[0] = {
		/*EM73C044VCG-H*/
		.pagesize  = 2048,
		.oobsize   = 64,
		.blocksize = 2048 * 64,
		.flashsize = 2048 * 64 * 1024,

		.tSETUP = TSETUP,
		.tHOLD  = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 4,
		.need_quad = 1,
	},
	[1] = {
		/*EM73C044VCS-H*/
		.pagesize  = 2048,
		.oobsize   = 128,
		.blocksize = 2048 * 64,
		.flashsize = 2048 * 64 * 2048,

		.tSETUP = TSETUP,
		.tHOLD  = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 4,
		.need_quad = 1,
	},
};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0x15, "EM73C044VCG-H", &etron_param[0]),
	DEVICE_ID_STRUCT(0x25, "EM73C044VCS-H", &etron_param[1]),
};


static cdt_params_t *etron_get_cdt_params(struct sfc_flash *flash, uint16_t device_id)
{
	CDT_PARAMS_INIT(etron_nand->cdt_params);

	switch(device_id) {
		case 0x15:
		case 0x25:
			break;
		default:
			printf("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}

	return &etron_nand->cdt_params;
}


static inline int deal_ecc_status(struct sfc_flash *flash, uint16_t device_id, uint8_t ecc_status)
{
	switch(device_id) {
		case 0x15:
		case 0x25:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
				case 0x1:
					return 0;
				case 0x2:
					return 4;
				case 0x3:
					return -EBADMSG;
				default:
					break;
			}
		default:
			printf("device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			break;
	}
	return -EINVAL;
}


static int etron_nand_init(void) {

	etron_nand = kzalloc(sizeof(*etron_nand), GFP_KERNEL);
	if(!etron_nand) {
		pr_err("alloc etron_nand struct fail\n");
		return -ENOMEM;
	}

	etron_nand->id_manufactory = 0x01;
	etron_nand->id_device_list = device_id;
	etron_nand->id_device_count = ARRAY_SIZE(etron_param);

	etron_nand->ops.get_cdt_params = etron_get_cdt_params;
	etron_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	etron_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(etron_nand);
}

SPINAND_MOUDLE_INIT(etron_nand_init);
