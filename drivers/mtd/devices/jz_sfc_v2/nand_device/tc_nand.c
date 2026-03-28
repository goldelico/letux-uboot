#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		100
#define	TSHSL_W		100

#define TRD		30
#define TPP		360
#define TBE		2

static struct jz_sfcnand_device *tc_nand;

static struct jz_sfcnand_base_param tc_param[] = {

	[0] = {
		/*TC58CVG0S3HRAIG*/
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
		.need_quad = 0, // unsupport quad
	},
	[1] = {
		/*TC58CVG2S0HRAIJ */
		.pagesize = 4 * 1024,
		.blocksize = 4 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 4 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 300,
		.tPP = 600,
		.tBE = 7,

		.plane_select = 0,
		.ecc_max = 0x4,
		.need_quad = 1,
	},
};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0xC2, "TC58CVG0S3HRAIG", &tc_param[0]),
	DEVICE_ID_STRUCT(0xed, "TC58CVG2S0HRAIJ", &tc_param[1]),
};


static cdt_params_t *tc_get_cdt_params(struct sfc_flash *flash, uint16_t device_id)
{
	CDT_PARAMS_INIT(tc_nand->cdt_params);

	switch(device_id) {
		case 0xC2:
		case 0xed:
			break;
		default:
			pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}

	return &tc_nand->cdt_params;
}

static inline int deal_ecc_status(struct sfc_flash *flash, uint16_t device_id, uint8_t ecc_status)
{
	int ret = 0;
	switch(device_id) {
		case 0xC2:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
					return 8;
				case 0x2:
					return -EBADMSG;
				default:
					break;
			}
			break;
		case 0xED:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
					ret = nand_get_ecc_conf(flash, 0x30);
					if (ret < 0)
						return ret;
					ret >>= 4;
					return ret;
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


static int tc_nand_init(void)
{
	tc_nand = kzalloc(sizeof(*tc_nand), GFP_KERNEL);
	if(!tc_nand) {
		pr_err("alloc tc_nand struct fail\n");
		return -ENOMEM;
	}

	tc_nand->id_manufactory = 0x98;
	tc_nand->id_device_list = device_id;
	tc_nand->id_device_count = ARRAY_SIZE(tc_param);

	tc_nand->ops.get_cdt_params = tc_get_cdt_params;
	tc_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	tc_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(tc_nand);
}

SPINAND_MOUDLE_INIT(tc_nand_init);
