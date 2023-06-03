#include <stdio.h>
#include "nand_common.h"

#define DOSILICON_MID		    0xE5
#define DOSILICON_NAND_DEVICE_COUNT	    5

static unsigned char dosilicon_xge4ab[] = {0x2};


static struct device_struct device[] = {
	DEVICE_STRUCT(0x71, 2048, 2, 4, 2, 1,  dosilicon_xge4ab),
	DEVICE_STRUCT(0x72, 2048, 2, 4, 2, 1,  dosilicon_xge4ab),
	DEVICE_STRUCT(0xF2, 2048, 2, 4, 3, 1,  dosilicon_xge4ab),
	DEVICE_STRUCT(0x21, 2048, 2, 4, 2, 1,  dosilicon_xge4ab),
	DEVICE_STRUCT(0x22, 2048, 2, 4, 2, 1,  dosilicon_xge4ab),
};

static struct nand_desc dosilicon_nand = {
	.id_manufactory = DOSILICON_MID,
	.device_counts = DOSILICON_NAND_DEVICE_COUNT,
	.device = device,
};

int dosilicon_nand_register_func(void) {
	return nand_register(&dosilicon_nand);
}
