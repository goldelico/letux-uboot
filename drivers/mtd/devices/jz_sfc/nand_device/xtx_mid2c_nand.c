#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define XTX_MID2C_DEVICES_NUM         1
#define TSETUP		4
#define THOLD		4
#define	TSHSL_R		30
#define	TSHSL_W		30

#define TRD		70
#define TPP		600
#define TBE		10

static struct jz_sfcnand_device *xtx_mid2c_nand;

static struct jz_sfcnand_base_param xtx_mid2c_param[XTX_MID2C_DEVICES_NUM] = {

	[0] = {
		/*XT26G02E */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tHOLD   = THOLD,
		.tSETUP  = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x8,
		.need_quad = 1,
	},

};

static struct device_id_struct device_id[XTX_MID2C_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0x24, "XT26G02E ", &xtx_mid2c_param[0]),
};


static void xtx_mid2c_pageread_to_cache(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	transfer->cmd_info.cmd = SPINAND_CMD_PARD;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = op_info->pageaddr;
	transfer->addr_len = 3;

	transfer->cmd_info.dataen = DISABLE;
	transfer->len = 0;

	transfer->data_dummy_bits = 0;
	transfer->ops_mode = CPU_OPS;
	return;
}

static int32_t xtx_mid2c_get_read_feature(struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct sfc_transfer transfer;
	uint8_t device_id = nand_info->id_device;
	uint8_t ecc_status = 0;
	int32_t ret = 0;

retry:
	ecc_status = 0;
	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	transfer.cmd_info.cmd = SPINAND_CMD_GET_FEATURE;
	transfer.sfc_mode = TM_STD_SPI;

	transfer.addr = SPINAND_ADDR_STATUS;
	transfer.addr_len = 1;

	transfer.cmd_info.dataen = ENABLE;
	transfer.data = &ecc_status;
	transfer.len = 1;
	transfer.direction = GLB_TRAN_DIR_READ;

	transfer.data_dummy_bits = 0;
	transfer.ops_mode = CPU_OPS;

	if(sfc_sync(flash->sfc, &transfer)) {
	        printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	if(ecc_status & SPINAND_IS_BUSY)
		goto retry;

	switch(device_id) {
		case 0x24:
			switch((ecc_status >> 0x4) & 0x7) {
			    case 0x5:
				    ret = 8;
				    break;
			    case 0x3:
				    ret = 6;
				    break;
			    case 0x2:
				    ret = -EBADMSG;
				    break;
			    case 0x1:
				    ret = 3;
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

static void xtx_mid2c_single_read(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	uint8_t device_id = nand_info->id_device;
	uint32_t columnaddr = op_info->columnaddr;
	int plane_flag = 0;

	switch(device_id) {
	    case 0x24:
			plane_flag = (op_info->pageaddr >> 6) & 1;
			columnaddr |= (plane_flag << 12);
			break;
	    default:
		    pr_err("device_id err,it maybe don`t support this device, please check your device id: device_id = 0x%02x\n", device_id);
		    break;
	}

	transfer->cmd_info.cmd = SPINAND_CMD_FRCH;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = columnaddr;
	transfer->addr_len = 2;

	transfer->cmd_info.dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_READ;

	transfer->data_dummy_bits = 8;
	transfer->ops_mode = DMA_OPS;
	return;
}

static void xtx_mid2c_quad_read(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	uint8_t device_id = nand_info->id_device;
	uint32_t columnaddr = op_info->columnaddr;
	int plane_flag = 0;

	switch(device_id) {
	    case 0x24:
			plane_flag = (op_info->pageaddr >> 6) & 1;
			columnaddr |= (plane_flag << 12);
			break;
	    default:
		    pr_err("device_id err,it maybe don`t support this device, please check your device id: device_id = 0x%02x\n", device_id);
		    break;
	}

	transfer->cmd_info.cmd = SPINAND_CMD_RDCH_X4;
	transfer->sfc_mode = TM_QI_QO_SPI;

	transfer->addr = columnaddr;
	transfer->addr_len = 2;

	transfer->cmd_info.dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_READ;

	transfer->data_dummy_bits = 8;
	transfer->ops_mode = DMA_OPS;
	return;
}

static void xtx_mid2c_single_load(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	uint8_t device_id = nand_info->id_device;
	uint32_t columnaddr = op_info->columnaddr;
	int plane_flag = 0;

	switch(device_id) {
		case 0x24:
			plane_flag = (op_info->pageaddr >> 6) & 1;
			columnaddr |= (plane_flag << 12);
			break;
		default:
		    pr_err("device_id err,it maybe don`t support this device, please check your device id: device_id = 0x%02x\n", device_id);
		    break;
	}

	transfer->cmd_info.cmd = SPINAND_CMD_PRO_LOAD;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = columnaddr;
	transfer->addr_len = 2;

	transfer->cmd_info.dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_WRITE;

	transfer->data_dummy_bits = 0;
	transfer->ops_mode = DMA_OPS;
}

static void xtx_mid2c_quad_load(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	uint8_t device_id = nand_info->id_device;
	uint32_t columnaddr = op_info->columnaddr;
	int plane_flag = 0;

	switch(device_id) {
		case 0x24:
			plane_flag = (op_info->pageaddr >> 6) & 1;
			columnaddr |= (plane_flag << 12);
			break;
	    default:
		    pr_err("device_id err,it maybe don`t support this device, please check your device id: device_id = 0x%02x\n", device_id);
		    break;
	}

	transfer->cmd_info.cmd = SPINAND_CMD_PRO_LOAD_X4;
	transfer->sfc_mode = TM_QI_QO_SPI;

	transfer->addr = columnaddr;
	transfer->addr_len = 2;

	transfer->cmd_info.dataen = ENABLE;
	transfer->data = op_info->buffer;
	transfer->len = op_info->len;
	transfer->direction = GLB_TRAN_DIR_WRITE;

	transfer->data_dummy_bits = 0;
	transfer->ops_mode = DMA_OPS;

}

static void xtx_mid2c_program_exec(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	transfer->cmd_info.cmd = SPINAND_CMD_PRO_EN;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = op_info->pageaddr;
	transfer->addr_len = 3;

	transfer->cmd_info.dataen = DISABLE;
	transfer->len = 0;

	transfer->data_dummy_bits = 0;
	transfer->ops_mode = CPU_OPS;
}

static int xtx_mid2c_nand_init(void) {
	struct jz_sfcnand_device *xtx_mid2c_nand;
	xtx_mid2c_nand = kzalloc(sizeof(*xtx_mid2c_nand), GFP_KERNEL);
	if(!xtx_mid2c_nand) {
		pr_err("alloc xtx_mid2c_nand struct fail\n");
		return -ENOMEM;
	}

	xtx_mid2c_nand->id_manufactory = 0x2C;
	xtx_mid2c_nand->id_device_list = device_id;
	xtx_mid2c_nand->id_device_count = XTX_MID2C_DEVICES_NUM;

	xtx_mid2c_nand->ops.nand_read_ops.pageread_to_cache = xtx_mid2c_pageread_to_cache;
	xtx_mid2c_nand->ops.nand_read_ops.get_feature = xtx_mid2c_get_read_feature;
	xtx_mid2c_nand->ops.nand_read_ops.single_read = xtx_mid2c_single_read;
	xtx_mid2c_nand->ops.nand_read_ops.quad_read = xtx_mid2c_quad_read;

	xtx_mid2c_nand->ops.nand_write_ops.single_load = xtx_mid2c_single_load;
	xtx_mid2c_nand->ops.nand_write_ops.quad_load = xtx_mid2c_quad_load;
	xtx_mid2c_nand->ops.nand_write_ops.program_exec = xtx_mid2c_program_exec;

	return jz_sfcnand_register(xtx_mid2c_nand);
}
SPINAND_MOUDLE_INIT(xtx_mid2c_nand_init);
