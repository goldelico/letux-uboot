#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define TSETUP		4
#define THOLD		4
#define	TSHSL_R		30
#define	TSHSL_W		30

#define TRD		70
#define TPP		600
#define TBE		10

static struct jz_sfcnand_device *xtx_mid2c_nand;

static struct jz_sfcnand_base_param xtx_mid2c_param[] = {

	[0] = {
		/*XT26G02E */
		/*NM5A02G01A*/
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

		.plane_select = 1,
		.ecc_max = 0x8,
		.need_quad = 1,
	},

};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0x24, "XT26G02E ", &xtx_mid2c_param[0]),
	/* DEVICE_ID_STRUCT(0x24, "NM5A02G01A", &xtx_mid2c_param[0]), */
};


static cdt_params_t *xtx_mid2c_get_cdt_params(struct sfc_flash *flash, uint16_t device_id)
{
	CDT_PARAMS_INIT(xtx_mid2c_nand->cdt_params);

	switch(device_id) {
		case 0x24:            /* same as NM5A02G01A nand device */
			break;
		default:
			pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}

	return &xtx_mid2c_nand->cdt_params;
}


static inline int deal_ecc_status(struct sfc_flash *flash, uint16_t device_id, uint8_t ecc_status)
{

	switch(device_id) {
		case 0x24:
			switch((ecc_status >> 4) & 0x7) {
				case 0x0:
				case 0x1:
					return 0;
				case 0x2:
					return -EBADMSG;
				case 0x3:
				case 0x5:
					return 8;
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


static int xtx_mid2c_nand_init(void) {

	xtx_mid2c_nand = kzalloc(sizeof(*xtx_mid2c_nand), GFP_KERNEL);
	if(!xtx_mid2c_nand) {
		pr_err("alloc xtx_mid2c_nand struct fail\n");
		return -ENOMEM;
	}

	xtx_mid2c_nand->id_manufactory = 0x2C;
	xtx_mid2c_nand->id_device_list = device_id;
	xtx_mid2c_nand->id_device_count = ARRAY_SIZE(xtx_mid2c_param);

	xtx_mid2c_nand->ops.get_cdt_params = xtx_mid2c_get_cdt_params;
	xtx_mid2c_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	xtx_mid2c_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(xtx_mid2c_nand);
}

SPINAND_MOUDLE_INIT(xtx_mid2c_nand_init);
