#include <stdio.h>
#include "nand_common.h"


static unsigned char xtx_0b_errstat_2[] = {0x2, 0x3};
static unsigned char xtx_0b_errstat_1[] = {0xf};

static struct device_struct device[] = {
	DEVICE_STRUCT(0xE1, 2048, 2, 4, 3, 1, xtx_0b_errstat_2, 0),
	DEVICE_STRUCT(0xF2, 2048, 2, 4, 3, 1, xtx_0b_errstat_2, 0),
	DEVICE_STRUCT(0x11, 2048, 2, 4, 4, 1, xtx_0b_errstat_1, 0),
	DEVICE_STRUCT(0x12, 2048, 2, 4, 4, 1, xtx_0b_errstat_1, 0),
	DEVICE_STRUCT(0x32, 2048, 2, 4, 4, 1, xtx_0b_errstat_2, 0),
};

static struct nand_desc xtx_0b_nand = {

	.id_manufactory = 0x0B,
	.device_counts  = ARRAY_SIZE(device),
	.device = device,
};

int xtx_mid0b_nand_register_func(void) {
	return nand_register(&xtx_0b_nand);
}
