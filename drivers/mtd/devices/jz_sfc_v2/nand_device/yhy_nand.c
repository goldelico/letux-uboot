#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

/*
HYF1GQ4UDACAE:HYF2GQ4UAACAE:HYF2GQ4UADCAE:
include/configs/halley5.h
        #define CONFIG_SFC_NAND_RATE    200000000
arch/mips/boot/dts/ingenic/halley5_v20.dts
	ingenic,sfc-init-frequency = <100000000>;
	ingenic,sfc-max-frequency = <200000000>;
cloner sfc rate:100MHZ；

HYF4GQ4UAACBE:
include/configs/halley5.h
        #define CONFIG_SFC_NAND_RATE    200000000
        #define CONFIG_SPI_NAND_BPP    (4096 +256)
arch/mips/boot/dts/ingenic/halley5_v20.dts
	ingenic,sfc-init-frequency = <100000000>;
	ingenic,sfc-max-frequency = <200000000>;
buildroot-menuconfig
	BR2_TARGET_ROOTFS_UBIFS_LEBSIZE [=0x3E000]
	BR2_TARGET_ROOTFS_UBIFS_MINIOSIZE [=0x1000]
	BR2_TARGET_ROOTFS_UBIFS_MAXLEBCNT [=4096]
cloner sfc rate:100MHZ；
*/

#define TSETUP		20
#define THOLD		20
#define	TSHSL_R		50
#define	TSHSL_W		50

#define TRD		200
#define TPP		800
#define TBE		10

static struct jz_sfcnand_device *yhy_midc9_nand;
static struct jz_sfcnand_base_param yhy_midc9_param[] = {

	[0] = {
		/*HYF1GQ4U */
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
		/*HYF2GQ4U */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 2 * 1024 * 64 * 2048,

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
		/*HYF4GQ4U */
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
		.ecc_max = 0x4,
		.need_quad = 1,
	},

};

static struct device_id_struct device_id[] = {
	DEVICE_ID_STRUCT(0x21, "HYF1GQ4U", &yhy_midc9_param[0]),
	DEVICE_ID_STRUCT(0x52, "HYF2GQ4U", &yhy_midc9_param[1]),
	DEVICE_ID_STRUCT(0xD4, "HYF4GQ4U", &yhy_midc9_param[2]),
};

static cdt_params_t *yhy_midc9_get_cdt_params(struct sfc_flash *flash, uint16_t device_id) {
	CDT_PARAMS_INIT(yhy_midc9_nand->cdt_params);
	switch(device_id) {
		case 0x21:
		case 0x52:
		case 0xD4:
			break;
		default:
			pr_err("device_id err, please check your  device id: device_id = 0x%02x\n", device_id);
			return NULL;
	}
	return &yhy_midc9_nand->cdt_params;
}

static inline int deal_ecc_status(struct sfc_flash *flash, uint16_t device_id, uint8_t ecc_status) {

	switch(device_id) {
		case 0x21:
		case 0x52:
		case 0xD4:
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
		default:
			printf("device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			break;
	}
	return -EINVAL;
}

static int yhy_midc9_nand_init(void) {
	yhy_midc9_nand = kzalloc(sizeof(*yhy_midc9_nand), GFP_KERNEL);
	if(!yhy_midc9_nand) {
		pr_err("alloc yhy_nand struct fail\n");
		return -ENOMEM;
	}

	yhy_midc9_nand->id_manufactory = 0xC9;
	yhy_midc9_nand->id_device_list = device_id;
	yhy_midc9_nand->id_device_count = ARRAY_SIZE(yhy_midc9_param);
	yhy_midc9_nand->ops.get_cdt_params = yhy_midc9_get_cdt_params;
	yhy_midc9_nand->ops.deal_ecc_status = deal_ecc_status;
	yhy_midc9_nand->ops.get_feature = NULL;
	return jz_sfcnand_register(yhy_midc9_nand);
}
SPINAND_MOUDLE_INIT(yhy_midc9_nand_init);
