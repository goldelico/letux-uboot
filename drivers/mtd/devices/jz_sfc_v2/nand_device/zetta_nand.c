#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define ZETTA_DEVICES_NUM         2
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		100
#define	TSHSL_W		100

#define TRD		70
#define TPP		320
#define TBE		2

static struct jz_sfcnand_device *zetta_nand;

static struct jz_sfcnand_base_param zetta_param[ZETTA_DEVICES_NUM] = {

	[0] = {
		/*ZD35Q1GA*/
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
		/*ZD35Q2GA*/
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
		.ecc_max = 0x4,
		.need_quad = 1,
	},

};

static struct device_id_struct device_id[ZETTA_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0x71, "ZD35Q1GA", &zetta_param[0]),
	DEVICE_ID_STRUCT(0x72, "ZD35Q2GA", &zetta_param[1]),
};


static cdt_params_t *zetta_get_cdt_params(struct sfc_flash *flash, uint8_t device_id)
{
	CDT_PARAMS_INIT(zetta_nand->cdt_params);

	switch(device_id) {
	    case 0x71:
	    case 0x72:
		    break;
	    default:
		    pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
		    return NULL;
	}

	return &zetta_nand->cdt_params;
}


static inline int deal_ecc_status(struct sfc_flash *flash, uint8_t device_id, uint8_t ecc_status)
{
	int ret = 0;

	switch(device_id) {
		case 0x71:
		case 0x72:
			switch((ecc_status >> 4) & 0x3) {
			    case 0x01:
				    ret = 0x4;
				    break;
			    case 0x02:
				    ret = -EBADMSG;
				    break;
			    default:
				    ret = 0;
			}
			break;

		default:
			printf("device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;   //notice!!!
	}
	return ret;
}


static int zetta_nand_init(void)
{
	zetta_nand = kzalloc(sizeof(*zetta_nand), GFP_KERNEL);
	if(!zetta_nand) {
		pr_err("alloc zetta_nand struct fail\n");
		return -ENOMEM;
	}

	zetta_nand->id_manufactory = 0xBA;
	zetta_nand->id_device_list = device_id;
	zetta_nand->id_device_count = ZETTA_DEVICES_NUM;

	zetta_nand->ops.get_cdt_params = zetta_get_cdt_params;
	zetta_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	zetta_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(zetta_nand);
}

SPINAND_MOUDLE_INIT(zetta_nand_init);
