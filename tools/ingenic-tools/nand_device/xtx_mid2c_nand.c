#include <stdio.h>
#include "nand_common.h"

#define XTX_2C_MID			    0x2C

#define XTX_2C_NAND_DEVICD_COUNT	    1

static unsigned char xtx_2c_xaw[] = {0x02};

static struct device_struct device[XTX_2C_NAND_DEVICD_COUNT] = {
	DEVICE_STRUCT(0x24, 2048, 2, 4, 3, 1, xtx_2c_xaw),
};

static struct nand_desc xtx_2c_nand = {

	.id_manufactory = XTX_2C_MID,
	.device_counts  = XTX_2C_NAND_DEVICD_COUNT,
	.device = device,
};

int xtx_mid2c_nand_register_func(void) {
	return nand_register(&xtx_2c_nand);
}
