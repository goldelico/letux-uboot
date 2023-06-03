#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define ATO_DEVICES_NUM         1
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		30
#define	TSHSL_W		30

#define TRD		25
#define TPP		500
#define TBE		3

static struct jz_sfcnand_device *ato_nand;

static struct jz_sfcnand_base_param ato25d1ga_param = {

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

	.plane_select = 0,
	.ecc_max = 0,//0x3,
	.need_quad = 1,

};

static struct device_id_struct device_id[ATO_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0x12, "ATO25D1GA", &ato25d1ga_param),
};


static cdt_params_t *ato_get_cdt_params(struct sfc_flash *flash, uint8_t device_id)
{
	CDT_PARAMS_INIT(ato_nand->cdt_params);

	switch(device_id) {
		case 0x12:
		    break;
		default:
		    pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
		    return NULL;
	}

	return &ato_nand->cdt_params;
}


static inline int deal_ecc_status(struct sfc_flash *flash, uint8_t device_id, uint8_t ecc_status)
{
	int ret = 0;
	switch(device_id) {
		case 0x12:
			return 0;
		default:
			pr_err("device_id err,it maybe don`t support this device, please check your device id: device_id = 0x%02x\n", device_id);
			return -EIO;   //notice!!!
	}
	return ret;
}


static int ato_nand_init(void) {

	ato_nand = kzalloc(sizeof(*ato_nand), GFP_KERNEL);
	if(!ato_nand) {
		pr_err("alloc ato_nand struct fail\n");
		return -ENOMEM;
	}

	ato_nand->id_manufactory = 0x9B;
	ato_nand->id_device_list = device_id;
	ato_nand->id_device_count = ATO_DEVICES_NUM;

	ato_nand->ops.get_cdt_params = ato_get_cdt_params;
	ato_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	ato_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(ato_nand);
}

SPINAND_MOUDLE_INIT(ato_nand_init);
