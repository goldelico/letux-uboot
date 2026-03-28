#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		20
#define	TSHSL_W		20

#define TRD		180
#define TPP		400
#define TBE		3

static struct jz_sfcnand_device *fs_nand;

static struct jz_sfcnand_base_param fs_param[] = {

	[0] = {
		/*FS35ND01G-V1*/
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
		/*FS35ND01G-V2*/
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
	[2] = {
		/*FS35ND02G*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 450,
		.tPP = 800,
		.tBE = 10,

		.plane_select = 0,
		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[3] = {
		/*FS35ND01G-S1*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 450,
		.tPP = 800,
		.tBE = 10,

		.plane_select = 0,
		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[4] = {
		/*FS35SQA001G*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 60,
		.tPP = 700,
		.tBE = 10,

		.plane_select = 0,
		.ecc_max = 0x1,
		.need_quad = 1,
	},
	[5] = {
		/*F35SQA512M*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 512,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 60,
		.tPP = 700,
		.tBE = 10,

		.plane_select = 0,
		.ecc_max = 0x1,
		.need_quad = 1,
	},
	[6] = {
		/*F35SQA002G*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 60,
		.tPP = 750,
		.tBE = 10,

		.plane_select = 0,
		.ecc_max = 0x1,
		.need_quad = 1,
	},
	[7] = {
		/*F35SQA004G*/
		.pagesize = 2 * 2 * 1024,
		.blocksize = 2 * 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 105,
		.tPP = 830,
		.tBE = 10,

		.plane_select = 0,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[8] = {
		/*F35SQB002G*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 135,
		.tPP = 750,
		.tBE = 10,

		.plane_select = 0,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0xA1, "FS35ND01G-V1", &fs_param[0]),
	DEVICE_ID_STRUCT(0xB1, "FS35ND01G-V2", &fs_param[1]),
	DEVICE_ID_STRUCT(0xEB, "FS35ND02G",    &fs_param[2]),
	DEVICE_ID_STRUCT(0xEA, "FS35ND01G-S1", &fs_param[3]),
	DEVICE_ID_STRUCT(0x71, "F35SQA001G",   &fs_param[4]),
	DEVICE_ID_STRUCT(0x70, "F35SQA512M",   &fs_param[5]),
	DEVICE_ID_STRUCT(0x72, "F35SQA002G",   &fs_param[6]),
	DEVICE_ID_STRUCT(0x53, "F35SQA004G",   &fs_param[7]),
	DEVICE_ID_STRUCT(0x52, "F35SQB002G",   &fs_param[8]),
};


static cdt_params_t *fs_get_cdt_params(struct sfc_flash *flash, uint16_t device_id)
{
	CDT_PARAMS_INIT(fs_nand->cdt_params);

	switch(device_id) {
		case 0xA1:
		case 0xB1:
		case 0xEB:
		case 0xEA:
		case 0x71:
		case 0x70:
		case 0x72:
		case 0x53:
		case 0x52:
			break;
		default:
			pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}

	return &fs_nand->cdt_params;
}


static inline int deal_ecc_status(struct sfc_flash *flash, uint16_t device_id, uint8_t ecc_status)
{

	switch(device_id) {
		case 0xA1:
		case 0xB1:
			switch((ecc_status >> 4) & 0x7) {
				case 0x0:
					return 0;
				case 0x1 ... 0x4:
					return ((ecc_status >> 4) & 0x7);
				case 0x7:
					return -EBADMSG;
				default:
					break;
			}
			break;
		case 0x70:
		case 0x71:
		case 0x72:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					return 0;
				case 0x1:
					return 1;
				case 0x2:
				case 0x3:
					return -EBADMSG;
				default:
					break;
			}
			break;
                case 0xEB:
		case 0xEA:
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
		case 0x53:
		case 0x52:
			switch((ecc_status >> 4) & 0x7) {
				case 0x0:
					return 0;
				case 0x1 ... 0x6:
					return ((ecc_status >> 4) & 0x7) + 2;
				case 0x7:
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

static int fs_nand_init(void) {

	fs_nand = kzalloc(sizeof(*fs_nand), GFP_KERNEL);
	if(!fs_nand) {
		pr_err("alloc fs_nand struct fail\n");
		return -ENOMEM;
	}

	fs_nand->id_manufactory = 0xCD;
	fs_nand->id_device_list = device_id;
	fs_nand->id_device_count = ARRAY_SIZE(fs_param);

	fs_nand->ops.get_cdt_params = fs_get_cdt_params;
	fs_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	fs_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(fs_nand);
}

SPINAND_MOUDLE_INIT(fs_nand_init);
