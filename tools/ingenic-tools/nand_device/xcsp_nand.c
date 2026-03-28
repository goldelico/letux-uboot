#include <stdio.h>
#include "nand_common.h"


static unsigned char xcsp_errstat_1[] = {0x03};

static struct device_struct device[] = {
	DEVICE_STRUCT(0x01, 2048, 2, 4, 2, 1, xcsp_errstat_1, 0),
	DEVICE_STRUCT(0xa1, 2048, 2, 4, 2, 1, xcsp_errstat_1, 0),
	DEVICE_STRUCT(0xb1, 2048, 2, 4, 2, 1, xcsp_errstat_1, 0),
};

static struct nand_desc xcsp_nand = {

	.id_manufactory = 0x9C,
	.device_counts  = ARRAY_SIZE(device),
	.device = device,
};

int xcsp_nand_register_func(void) {
	return nand_register(&xcsp_nand);
}
