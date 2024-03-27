#include <stdio.h>
#include "nand_common.h"

#define MXIC_MID		    0xC2
#define MXIC_NAND_DEVICE_COUNT	    4

static unsigned char mxic_xge4ab[] = {0x2};


static struct device_struct device[] = {
	DEVICE_STRUCT(0x12, 2048, 2, 4, 2, 1,  mxic_xge4ab),
	DEVICE_STRUCT(0x22, 2048, 2, 4, 2, 1,  mxic_xge4ab),
	DEVICE_STRUCT(0x26, 2048, 2, 4, 2, 1,  mxic_xge4ab),
	DEVICE_STRUCT(0x37, 4096, 2, 4, 2, 1,  mxic_xge4ab),
};

static struct nand_desc mxic_nand = {

	.id_manufactory = MXIC_MID,
	.device_counts = MXIC_NAND_DEVICE_COUNT,
	.device = device,
};

int mxic_nand_register_func(void) {
	return nand_register(&mxic_nand);
}
