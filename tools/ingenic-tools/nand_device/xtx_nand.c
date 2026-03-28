#include <stdio.h>
#include "nand_common.h"


static unsigned char xtx_errstat_2[] = {0x2, 0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0xE1, 2048, 2, 4, 2, 1, xtx_errstat_2, 0),
	DEVICE_STRUCT(0xE2, 2048, 2, 4, 2, 1, xtx_errstat_2, 0),
	DEVICE_STRUCT(0xC1, 2048, 2, 4, 2, 1, xtx_errstat_2, 0),
};

static struct nand_desc xtx_nand = {

	.id_manufactory = 0xA1,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int xtx_nand_register_func(void) {
	return nand_register(&xtx_nand);
}
