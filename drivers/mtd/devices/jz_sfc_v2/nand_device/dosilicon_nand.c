#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define THOLD	    5
#define TSETUP	    5
#define TSHSL_R	    100
#define TSHSL_W	    100

#define TRD	    90
#define TPP	    700
#define TBE	    10

struct jz_sfcnand_device *dosilicon_nand;

static struct jz_sfcnand_base_param dosilicon_param[] = {
	[0] = {
		/*DS35Q1GAXXX*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 70,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[1] = {
		/*DS35Q2GAXXX*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 90,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 1,
		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[2] = {
		/*DS35Q2GBXXX*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 120,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 1,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[3] = {
		/*DS35M1GAXXX-1.8V*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 80,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[4] = {
		/*DS35Q2GAXXX-1V8*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 90,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 1,
		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[5] = {
		/*DS35X1GBXXX*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 120,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 1,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[6] = {
		/*DS35Q4GBXXX*/
		/*Power outage test file system damage, resulting in inability to start normally
		 *During the power on and power off test, the file system was changed to read-only,
		 * and the data partition was used for reading and writing. However, during the test,
		 * the data partition was unable to write files properly*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 4096,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 120,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 1,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
        [7] = {
		/*DS35Q12C-1B*/
		/*this parameter has not been tested for high/low temperature and random power on/off.*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 512,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 120,
		.tPP = TPP,
		.tBE = TBE,


		.plane_select = 1,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[8] = {
		/*LC35Q4GMXXX*/
		/* Not aging test */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 4096,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 120,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 1,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[9] = {
		/*DS35Q2GBSXX*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 120,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x8,
		.need_quad = 1,
	},
};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0x71, "DS35Q1GAXXX",     &dosilicon_param[0]),
	DEVICE_ID_STRUCT(0x72, "DS35Q2GAXXX",     &dosilicon_param[1]),
	DEVICE_ID_STRUCT(0xF2, "DS35Q2GBXXX",     &dosilicon_param[2]),
	DEVICE_ID_STRUCT(0x21, "DS35M1GAXXX-1V8", &dosilicon_param[3]),
	DEVICE_ID_STRUCT(0x22, "DS35Q2GAXXX-1V8", &dosilicon_param[4]),
	DEVICE_ID_STRUCT(0xF1, "DS35X1GBXXX",     &dosilicon_param[5]),
	DEVICE_ID_STRUCT(0xB4, "DS35Q4GBXXX",     &dosilicon_param[6]),
	DEVICE_ID_STRUCT(0x75, "DS35Q12C-1B",     &dosilicon_param[7]),
	DEVICE_ID_STRUCT(0xF4, "LC35X4GMXXX",     &dosilicon_param[8]),
	DEVICE_ID_STRUCT(0xB2, "DS35Q2GBSXX",     &dosilicon_param[9]),
};


static cdt_params_t *dosilicon_get_cdt_params(struct sfc_flash *flash, uint16_t device_id)
{
	CDT_PARAMS_INIT(dosilicon_nand->cdt_params);

	switch(device_id) {
		case 0x71:
		case 0x72:
		case 0xF2:
		case 0x21:
		case 0x22:
		case 0xF1:
		case 0xB4:
		case 0x75:
		case 0xF4:
		case 0xB2:
			break;
		default:
			pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}

	return &dosilicon_nand->cdt_params;
}


static inline int deal_ecc_status(struct sfc_flash *flash, uint16_t device_id, uint8_t ecc_status)
{
	switch(device_id) {
		case 0x71:
		case 0x72:
		case 0x21:
		case 0x22:
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
		case 0xF2:
		case 0xF1:
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
		case 0xB2:
		case 0xB4:
		case 0x75:
		case 0xF4:
			switch((ecc_status >> 4) & 0x7) {
				case 0x0:
					return 0;
				case 0x1:
					return 3;
				case 0x2:
					return -EBADMSG;
					break;
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


static int dosilicon_nand_init(void) {

	dosilicon_nand = kzalloc(sizeof(*dosilicon_nand), GFP_KERNEL);
	if(!dosilicon_nand) {
		pr_err("alloc dosilicon_nand struct fail\n");
		return -ENOMEM;
	}

	dosilicon_nand->id_manufactory = 0xE5;
	dosilicon_nand->id_device_list = device_id;
	dosilicon_nand->id_device_count = ARRAY_SIZE(dosilicon_param);

	dosilicon_nand->ops.get_cdt_params = dosilicon_get_cdt_params;
	dosilicon_nand->ops.deal_ecc_status = deal_ecc_status;

	/* use private get feature interface, please define it in this document */
	dosilicon_nand->ops.get_feature = NULL;

	return jz_sfcnand_register(dosilicon_nand);
}

SPINAND_MOUDLE_INIT(dosilicon_nand_init);
