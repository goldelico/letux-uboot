#include <asm/arch/sfc.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include <asm-generic/errno.h>
#include "nand_common.h"

/*
 * Note: the deal_ecc_status() function needs to be defined in xxx_nand.c
 */
int32_t nand_common_get_feature(struct sfc_flash *flash, uint8_t flag)
{
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct jz_sfcnand_ops *ops = nand_info->ops;
	uint16_t device_id = nand_info->id_device;
	struct sfc_cdt_xfer xfer;
	uint8_t ecc_status = 0;
	int32_t ret = 0;

retry:
	ecc_status = 0;

	memset(&xfer, 0, sizeof(xfer));

	/*set index*/
	xfer.cmd_index = NAND_GET_FEATURE;

	/* set addr */
	xfer.staaddr0 = SPINAND_ADDR_STATUS;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = 1;
	xfer.config.data_dir = GLB_TRAN_DIR_READ;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = &ecc_status;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		pr_err("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	if(ecc_status & SPINAND_IS_BUSY)
		goto retry;

	switch(flag) {
		case GET_WRITE_STATUS:
			if(ecc_status & (0x1 << 3))
				ret = -EIO;
			break;

		case GET_ERASE_STATUS:
			if(ecc_status & (0x1 << 2))
				ret = -EIO;
			break;

		case GET_ECC_STATUS:
			ret = ops->deal_ecc_status(flash, device_id, ecc_status);
			break;
		default:
			pr_err("flag value is error ! %s %s %d\n",__FILE__,__func__,__LINE__);
	}
	return ret;
}
EXPORT_SYMBOL_GPL(nand_common_get_feature);


int32_t nand_get_ecc_conf(struct sfc_flash *flash, uint8_t addr)
{
	struct sfc_cdt_xfer xfer;
	uint32_t buf = 0;

	memset(&xfer, 0, sizeof(xfer));

	/*set index*/
	xfer.cmd_index = NAND_GET_FEATURE;

	/* set addr */
	xfer.staaddr0 = addr;

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
EXPORT_SYMBOL_GPL(nand_get_ecc_conf);
