#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		10
#define	TSHSL_W		50

#define TRD		60
#define TPP		700
#define TBE		10

static struct jz_sfcnand_device *winbond_nand;

static struct jz_sfcnand_base_param winbond_param[] = {
	[0] = {
		/*W25N01GV*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP = TSETUP,
		.tHOLD  = THOLD,
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
		/*W25N02KVxxIR/U*/
		.pagesize = 2 * 1024,
		.oobsize = 128,
		.blocksize = 2 * 1024 * 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP = TSETUP,
		.tHOLD  = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 1,
		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[2] = {
		/*W25N01KVxxIR/U test failed*/
		/*Power outage test file system damage, resulting in inability to start normally*/
		.pagesize = 2 * 1024,
		.oobsize = 128,
		.blocksize = 2 * 1024 * 64,
		.flashsize = 1 * 1024 * 64 * 2048,

		.tSETUP = TSETUP,
		.tHOLD  = THOLD,
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

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0xAA21, "W25N01GVZEIG",   &winbond_param[0]),
	DEVICE_ID_STRUCT(0xAA22, "W25N02KVxxIR/U", &winbond_param[1]),
	DEVICE_ID_STRUCT(0xAE21, "W25N01KVxxIR/U", &winbond_param[2]),
};


static cdt_params_t *winbond_get_cdt_params(struct sfc_flash *flash, uint16_t device_id)
{
	CDT_PARAMS_INIT(winbond_nand->cdt_params);

	switch(device_id) {
		case 0xAA21:
		case 0xAA22:
		case 0xAE21:
		    break;
		default:
		    pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
		    return NULL;
	}

	return &winbond_nand->cdt_params;
}

static inline int deal_ecc_status(struct sfc_flash *flash, uint16_t device_id, uint8_t ecc_status)
{
	int ret = 0;
	switch(device_id) {
		case 0xAA21:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
				case 0x3:
					return 4;
				case 0x2:
					return -EBADMSG;
				default:
					break;
			}
			break;
		case 0xAA22:
		case 0xAE21:
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
				case 0x3:
					return 4;
				default:
					break;
			}
			break;
		default:
			pr_err("device_id err,it maybe don`t support this device, please check your device id: device_id = 0x%02x\n", device_id);
			break;   //notice!!!
	}
	return -EINVAL;
}


static int winbond_nand_init(void) {

	winbond_nand = kzalloc(sizeof(*winbond_nand), GFP_KERNEL);
	if(!winbond_nand) {
		pr_err("alloc winbond_nand struct fail\n");
		return -ENOMEM;
	}

	winbond_nand->id_manufactory = 0xEF;
	winbond_nand->id_device_list = device_id;
	winbond_nand->id_device_count = ARRAY_SIZE(winbond_param);

	winbond_nand->ops.get_cdt_params = winbond_get_cdt_params;
	winbond_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	winbond_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(winbond_nand);
}

SPINAND_MOUDLE_INIT(winbond_nand_init);
