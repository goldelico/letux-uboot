#include <stdio.h>
#include "nand_common.h"

#define XTX_0B_MID			    0x0B

#define XTX_0B_NAND_DEVICD_COUNT	    3

static unsigned char xtx_0b_xaw[] = {0x04,0x07};
static unsigned char xtx_0b_xaw_1[] = {0x0f};

static struct device_struct device[XTX_0B_NAND_DEVICD_COUNT] = {
	DEVICE_STRUCT(0xF2, 2048, 2, 4, 3, 1, xtx_0b_xaw),
	DEVICE_STRUCT(0x11, 2048, 2, 4, 4, 1, xtx_0b_xaw_1),
	DEVICE_STRUCT(0x12, 2048, 2, 4, 4, 1, xtx_0b_xaw_1),
};

static struct nand_desc xtx_0b_nand = {

	.id_manufactory = XTX_0B_MID,
	.device_counts  = XTX_0B_NAND_DEVICD_COUNT,
	.device = device,
};

int xtx_mid0b_nand_register_func(void) {
	return nand_register(&xtx_0b_nand);
}
