#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"
#include <ubi_uboot.h>

#define XCSP_DEVICES_NUM         2
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		20
#define	TSHSL_W		20

#define TRD		120
#define TPP		600
#define TBE		8

static struct jz_sfcnand_device *xcsp_nand;
static struct jz_sfcnand_base_param xcsp_param[XCSP_DEVICES_NUM] = {

	[0] = {
		/*XCSP1AAWH */
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
		.ecc_max = 0x8,
		.need_quad = 1,
	},

	[1] = {
		/*XCSP4AAWH */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 4096,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.plane_select = 0,
		.ecc_max = 0x8,
		.need_quad = 1,
	},

};

static struct device_id_struct device_id[XCSP_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0x01, "XCSP1AAWH ", &xcsp_param[0]),
	DEVICE_ID_STRUCT(0xb1, "XCSP4AAWH ", &xcsp_param[1]),
};

static cdt_params_t *xcsp_nand_get_cdt_params(struct sfc_flash *flash, uint8_t device_id) {
	CDT_PARAMS_INIT(xcsp_nand->cdt_params);
	switch(device_id) {
		case 0x01:
		case 0xb1:
			break;
		default:
			pr_err("device_id err, please check your device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}
	return &xcsp_nand->cdt_params;
}

static int32_t xcsp_get_f0_register_value(struct sfc_flash *flash)
{
	struct sfc_cdt_xfer xfer;
	uint32_t buf = 0;

	memset(&xfer, 0, sizeof(xfer));

	/*set index*/
	xfer.cmd_index = NAND_GET_FEATURE;

	/* set addr */
	xfer.staaddr0 = 0xf0;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = 1;
	xfer.config.data_dir = GLB_TRAN_DIR_READ;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = (uint8_t *)&buf;

	if(sfc_sync_cdt(flash->sfc, &xfer)){
		pr_err("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}
	return buf;
}

static inline int deal_ecc_status(struct sfc_flash *flash, uint8_t device_id, uint8_t ecc_status) {
	int ret = 0;
	switch(device_id) {
		case 0x01:
		case 0xb1:
			switch((ecc_status >> 4) & 0x3) {
				case 0x0:
					if((ret = xcsp_get_f0_register_value(flash)) < 0)
						return ret;
					ret = (ret >> 4) & 0x3;
					break;
				case 0x1:
					if((ret = xcsp_get_f0_register_value(flash)) < 0)
						return ret;
					ret = ((ret >> 4) & 0x3) + 0x4;
					break;
				case 0x2:
					if((ret = xcsp_get_f0_register_value(flash)) < 0)
						return ret;
					if(((ret >> 4) & 0x3) == 0)
						ret = 0x8;
					else
						ret = -EBADMSG;
					break;
				case 0x3:
					ret = -EBADMSG;
					break;
				default:
					ret = 0;
					break;
			}
			break;
		default:
			printf("device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;
	}
	return ret;
}

static int xcsp_nand_init(void) {
	xcsp_nand = kzalloc(sizeof(*xcsp_nand), GFP_KERNEL);
	if(!xcsp_nand) {
		pr_err("alloc xcsp_nand struct fail\n");
		return -ENOMEM;
	}

	xcsp_nand->id_manufactory = 0x9c;
	xcsp_nand->id_device_list = device_id;
	xcsp_nand->id_device_count = XCSP_DEVICES_NUM;
	xcsp_nand->ops.get_cdt_params = xcsp_nand_get_cdt_params;
	xcsp_nand->ops.deal_ecc_status = deal_ecc_status;
	xcsp_nand->ops.get_feature = NULL;
	return jz_sfcnand_register(xcsp_nand);
}
SPINAND_MOUDLE_INIT(xcsp_nand_init);
