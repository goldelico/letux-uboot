/*
 * jz_sfc_ops.c is for sfc nor flash to set special function,
 * such as set_quad_mode, set_4byte_mode, and so on.
 *
 */

#include <errno.h>
#include <malloc.h>
#include <linux/string.h>

#include <asm/io.h>
#include <asm/arch/sfc.h>
#include <asm/arch/spinor.h>
#include "jz_sfc_common.h"

#define STATUS_MAX_LEN  4      //4 * byte = 32 bit

int get_status(struct sfc_flash *flash, unsigned short cmd_index, int len)
{
	struct sfc_cdt_xfer xfer;
	unsigned char buf[STATUS_MAX_LEN];
	unsigned int val = 0, i = 0;

	memset(&xfer, 0, sizeof(xfer));
	memset(buf, 0, sizeof(buf));
	len = (len > STATUS_MAX_LEN ? STATUS_MAX_LEN : len);

	/* set index */
	xfer.cmd_index = cmd_index;

	/* set addr */
	xfer.rowaddr = 0;
	xfer.columnaddr = 0;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = len;
	xfer.config.data_dir = GLB_TRAN_DIR_READ;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = buf;


	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	for(i = 0; i < len; i++) {
		val |= buf[i] << (i * 8);
	}

	return val;
}

/* do nothing to set quad mode, use cmd directly */
static int set_quad_mode_cmd(struct sfc_flash *flash)
{
	flash->quad_succeed = 1;
	return 0;
}

/* write nor flash status register QE bit to set quad mode */
static int set_quad_mode_reg(struct sfc_flash *flash)
{
	struct spi_nor_info *spi_nor_info;
	struct spi_nor_st_info *quad_set;

	struct sfc_cdt_xfer xfer;
	unsigned int data;
	int ret = 0;

	spi_nor_info = flash->g_nor_info;
	quad_set = &spi_nor_info->quad_set;
	data = (quad_set->val & quad_set->mask) << quad_set->bit_shift;

	/* 1. set nor quad */
	memset(&xfer, 0, sizeof(xfer));
	/* set index */
	xfer.cmd_index = NOR_QUAD_SET_ENABLE;

	/* set addr */
	xfer.columnaddr = 0;
	xfer.rowaddr = 0;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = quad_set->len;
	xfer.config.data_dir = GLB_TRAN_DIR_WRITE;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = (uint8_t *)&data;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret = -EIO;
	}
	flash->quad_succeed = 1;

	return ret;
}

static int write_enable(struct sfc_flash *flash)
{
	struct sfc_cdt_xfer xfer;
	int32_t ret = 0;

	memset(&xfer, 0, sizeof(xfer));

	/* set index */
	xfer.cmd_index = NOR_WRITE_ENABLE;

	/* set addr */
	xfer.columnaddr = 0;

	/* set transfer config */
	xfer.dataen = DISABLE;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret = -EIO;
	}

	return ret;
}

static int enter_4byte(struct sfc_flash *flash)
{
	struct sfc_cdt_xfer xfer;
	int32_t ret = 0;

	memset(&xfer, 0, sizeof(xfer));

	/* set index */
	xfer.cmd_index = NOR_EN_4BYTE;

	/* set addr */
	xfer.columnaddr = 0;

	/* set transfer config */
	xfer.dataen = DISABLE;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret = -EIO;
	}

	return ret;
}


/* send 4byte command to enter 4byte mode */
static int set_4byte_mode_normal(struct sfc_flash *flash)
{
	int ret;
	ret = enter_4byte(flash);
	if (ret) {
		printf( "enter 4byte mode failed\n");
	}
	return ret;
}

/**
 * 1.send write enable command
 * 2.send 4byte command to enter 4byte mode
 **/
static int set_4byte_mode_wren(struct sfc_flash *flash)
{
	int ret;
	ret = write_enable(flash);
	if (ret) {
		printf( "enter 4byte mode failed\n");
		return ret;
	}

	ret = enter_4byte(flash);
	if (ret) {
		printf( "enter 4byte mode failed\n");
	}
	return ret;
}


struct spi_nor_flash_ops nor_flash_ops;

static int noop(struct sfc_flash *flash)
{
	return 0;
}

int sfc_nor_get_special_ops(struct sfc_flash *flash)
{
	struct spi_nor_info *spi_nor_info = flash->g_nor_info;

	switch (spi_nor_info->quad_ops_mode) {
	case 0:
		nor_flash_ops.set_quad_mode = set_quad_mode_cmd;
		break;
	case 1:
		nor_flash_ops.set_quad_mode = set_quad_mode_reg;
		break;
	default:
		nor_flash_ops.set_quad_mode = noop;
		break;
	}

	switch (spi_nor_info->addr_ops_mode) {
	case 0:
		nor_flash_ops.set_4byte_mode = set_4byte_mode_normal;
		break;
	case 1:
		nor_flash_ops.set_4byte_mode = set_4byte_mode_wren;
		break;
	default:
		nor_flash_ops.set_4byte_mode = noop;
		break;
	}

	flash->nor_flash_ops = &nor_flash_ops;

	return 0;
}

