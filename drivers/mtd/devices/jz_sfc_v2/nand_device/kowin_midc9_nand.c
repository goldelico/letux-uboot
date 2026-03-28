#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define TSETUP		2
#define THOLD		4
#define	TSHSL_R		30
#define	TSHSL_W		30

#define TRD		150
#define TPP		600
#define TBE		3

static struct jz_sfcnand_device *kowin_midc9_nand;

static struct jz_sfcnand_base_param kowin_midc9_param[] = {

	[0] = {
		/*KANY3D4S4WD*/
		.pagesize = 4 * 1024,
		.blocksize = 4 * 1024 * 64,
		.oobsize = 256,
		.flashsize = 4 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x6,
		.need_quad = 1,
	},

};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0xD4, "KANY3D4S4WD", &kowin_midc9_param[0]),
};


static cdt_params_t *kowin_midc9_get_cdt_params(struct sfc_flash *flash, uint16_t device_id)
{
	CDT_PARAMS_INIT(kowin_midc9_nand->cdt_params);

	switch(device_id) {
		case 0xD4:
			break;
		default:
			pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}

	return &kowin_midc9_nand->cdt_params;
}


static inline int deal_ecc_status(struct sfc_flash *flash, uint16_t device_id, uint8_t ecc_status)
{

	switch(device_id) {
		case 0xD4:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
					return 4;
				case 0x2:
					return 6;
				case 0x3:
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


static int kowin_midc9_nand_init(void) {

	kowin_midc9_nand = kzalloc(sizeof(*kowin_midc9_nand), GFP_KERNEL);
	if(!kowin_midc9_nand) {
		pr_err("alloc kowin_nand struct fail\n");
		return -ENOMEM;
	}

	kowin_midc9_nand->id_manufactory = 0xC9;
	kowin_midc9_nand->id_device_list = device_id;
	kowin_midc9_nand->id_device_count = ARRAY_SIZE(kowin_midc9_param); 

	kowin_midc9_nand->ops.get_cdt_params = kowin_midc9_get_cdt_params;
	kowin_midc9_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	kowin_midc9_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(kowin_midc9_nand);
}

SPINAND_MOUDLE_INIT(kowin_midc9_nand_init);
