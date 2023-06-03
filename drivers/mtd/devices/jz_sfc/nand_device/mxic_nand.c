#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define MXIC_DEVICES_NUM         4
#define MXIC_CMD_GET_ECC	0x7c
#define THOLD	    4
#define TSETUP	    4
#define TSHSL_R	    100
#define TSHSL_W	    100

#define TRD	    70
#define TPP	    600
#define TBE	    4

static struct jz_sfcnand_base_param mxic_param[MXIC_DEVICES_NUM] = {
	[0] = {
	/*MX35LF1GE4AB*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 1024,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[1] = {
	/*MX35LF2GE4AB*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x4,
		.need_quad = 1,
	},
	[2] = {
	/*MX35LF2GE4AD*/
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 70,
		.tPP = 760,
		.tBE = 6,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
	[3] = {
	/*MX35LF4GE4AD*/
		.pagesize = 4 * 1024,
		.blocksize = 4 * 1024 * 64,
		.oobsize = 128,
		.flashsize = 4 * 1024 * 64 * 2048,

		.tHOLD  = THOLD,
		.tSETUP = TSETUP,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = 110,
		.tPP = 800,
		.tBE = 6,

		.ecc_max = 0x8,
		.need_quad = 1,
	},
};

static struct device_id_struct device_id[MXIC_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0x12, "MX35LF1GE4AB", &mxic_param[0]),
	DEVICE_ID_STRUCT(0x22, "MX35LF2GE4AB", &mxic_param[1]),
	DEVICE_ID_STRUCT(0x26, "MX35LF2GE4AD", &mxic_param[2]),
	DEVICE_ID_STRUCT(0x37, "MX35LF4GE4AD", &mxic_param[3]),
};

static void mxic_pageread_to_cache(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

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

static int32_t get_ecc_value(struct sfc_flash *flash) {
	struct sfc_transfer transfer;
	uint32_t buf = 0;
	int8_t count = 5;

try_read_again:
	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);
	transfer.cmd_info.cmd = MXIC_CMD_GET_ECC;
	transfer.sfc_mode = TM_STD_SPI;

	transfer.addr = 0;
	transfer.addr_len = 0;

	transfer.cmd_info.dataen = ENABLE;
	transfer.len = 1;
	transfer.data = (uint8_t *)&buf;
	transfer.direction = GLB_TRAN_DIR_READ;

	transfer.data_dummy_bits = 8;
	transfer.ops_mode = CPU_OPS;

	if(sfc_sync(flash->sfc, &transfer) && count--)
		goto try_read_again;
	if(count < 0)
		return -EIO;
	return buf & 0xf;
}

static int32_t mxic_get_read_feature(struct flash_operation_message *op_info) {

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
		case 0x12:
		case 0x22:
			switch((ecc_status >> 0x4) & 0x3) {
			    case 0x0:
				    ret = 0;
				    break;
			    case 0x1:
				    if((ret = get_ecc_value(flash)) > 0x4)
					    ret = -EBADMSG;
				    break;
			    case 0x2:
				    ret = -EBADMSG;
				    break;
			    default:
				    printf("it is flash Unknown state, device_id: 0x%02x\n", device_id);
				    ret = -EIO;
			}
			break;
		case 0x26:
		case 0x37:
			switch((ecc_status >> 0x4) & 0x3) {
			    case 0x0:
				    ret = 0;
				    break;
			    case 0x1:
				    if((ret = get_ecc_value(flash)) > 0x8)
					    ret = -EBADMSG;
				    break;
			    case 0x2:
				    ret = -EBADMSG;
				    break;
			    default:
				    printf("it is flash Unknown state, device_id: 0x%02x\n", device_id);
				    ret = -EIO;
			}
			break;
		default:
			printf("device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;
	}
	return ret;
}

static void mxic_single_read(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	uint8_t device_id = nand_info->id_device;
	uint32_t columnaddr = op_info->columnaddr;
	int plane_flag = 0;

	switch(device_id) {
		case 0x22:
			plane_flag = (op_info->pageaddr >> 6) & 1;
			columnaddr |= (plane_flag << 12);
			break;
		case 0x12:
		case 0x26:
		case 0x37:
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

static void mxic_quad_read(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	uint8_t device_id = nand_info->id_device;
	uint32_t columnaddr = op_info->columnaddr;
	int plane_flag = 0;

	switch(device_id) {
		case 0x22:
			plane_flag = (op_info->pageaddr >> 6) & 1;
			columnaddr |= (plane_flag << 12);
			break;
		case 0x12:
		case 0x26:
		case 0x37:
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

static void mxic_single_load(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	uint8_t device_id = nand_info->id_device;
	uint32_t columnaddr = op_info->columnaddr;
	int plane_flag = 0;

	switch(device_id) {
		case 0x22:
			plane_flag = (op_info->pageaddr >> 6) & 1;
			columnaddr |= (plane_flag << 12);
			break;
		case 0x12:
		case 0x26:
		case 0x37:
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

static void mxic_quad_load(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	uint8_t device_id = nand_info->id_device;
	uint32_t columnaddr = op_info->columnaddr;
	int plane_flag = 0;

	switch(device_id) {
		case 0x22:
			plane_flag = (op_info->pageaddr >> 6) & 1;
			columnaddr |= (plane_flag << 12);
			break;
		case 0x12:
		case 0x26:
		case 0x37:
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

static void mxic_program_exec(struct sfc_transfer *transfer, struct flash_operation_message *op_info) {

	transfer->cmd_info.cmd = SPINAND_CMD_PRO_EN;
	transfer->sfc_mode = TM_STD_SPI;

	transfer->addr = op_info->pageaddr;
	transfer->addr_len = 3;

	transfer->cmd_info.dataen = DISABLE;
	transfer->len = 0;

	transfer->data_dummy_bits = 0;
	transfer->ops_mode = CPU_OPS;
}

static int mxic_nand_init(void) {
	struct jz_sfcnand_device *mxic_nand;
	mxic_nand = kzalloc(sizeof(*mxic_nand), GFP_KERNEL);
	if(!mxic_nand) {
		pr_err("alloc mxic_nand struct fail\n");
		return -ENOMEM;
	}

	mxic_nand->id_manufactory = 0xC2;
	mxic_nand->id_device_list = device_id;
	mxic_nand->id_device_count = MXIC_DEVICES_NUM;

	mxic_nand->ops.nand_read_ops.pageread_to_cache = mxic_pageread_to_cache;
	mxic_nand->ops.nand_read_ops.get_feature = mxic_get_read_feature;
	mxic_nand->ops.nand_read_ops.single_read = mxic_single_read;
	mxic_nand->ops.nand_read_ops.quad_read = mxic_quad_read;

	mxic_nand->ops.nand_write_ops.single_load = mxic_single_load;
	mxic_nand->ops.nand_write_ops.quad_load = mxic_quad_load;
	mxic_nand->ops.nand_write_ops.program_exec = mxic_program_exec;

	return jz_sfcnand_register(mxic_nand);
}
SPINAND_MOUDLE_INIT(mxic_nand_init);
