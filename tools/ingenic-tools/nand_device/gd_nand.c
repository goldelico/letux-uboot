#include <stdio.h>
#include "nand_common.h"

static unsigned char gd_errstat_2[]= {0x2, 0x3};
static unsigned char gd_errstat_1[]= {0x7};

static struct device_struct device[] = {
	DEVICE_STRUCT(0xD1,   2048, 2, 4, 2, 1, gd_errstat_2, 0),
	DEVICE_STRUCT(0xD2,   2048, 2, 4, 2, 1, gd_errstat_2, 0),
	DEVICE_STRUCT(0xD4,   4096, 2, 4, 2, 1, gd_errstat_2, 0),
	DEVICE_STRUCT(0x51,   2048, 2, 4, 2, 1, gd_errstat_2, 0),
	DEVICE_STRUCT(0x52,   2048, 2, 4, 2, 1, gd_errstat_2, 0),
	DEVICE_STRUCT(0x32,   2048, 2, 4, 2, 1, gd_errstat_2, 0),
	DEVICE_STRUCT(0x55,   2048, 2, 4, 2, 1, gd_errstat_2, 0),
	DEVICE_STRUCT(0x92,   2048, 2, 4, 2, 1, gd_errstat_2, 0),
	DEVICE_STRUCT(0x91,   2048, 2, 4, 2, 1, gd_errstat_2, 0),
	DEVICE_STRUCT(0x95,   2048, 2, 4, 2, 1, gd_errstat_2, 0),
	DEVICE_STRUCT(0xA1,   2048, 3, 4, 3, 1, gd_errstat_1, 0),
	DEVICE_STRUCT(0xB1,   2048, 3, 4, 3, 1, gd_errstat_1, 0),
	DEVICE_STRUCT(0xB2,   2048, 3, 4, 3, 1, gd_errstat_1, 0),
	DEVICE_STRUCT(0xB468, 4096, 3, 4, 3, 1, gd_errstat_1, 0),
	DEVICE_STRUCT(0x61,   2048, 3, 4, 3, 1, gd_errstat_1, 0),
	DEVICE_STRUCT(0xD9,   2048, 2, 4, 2, 1, gd_errstat_2, 0),
	DEVICE_STRUCT(0x41,   2048, 2, 4, 2, 1, gd_errstat_2, 0),
};

static struct nand_desc gd_nand = {

	.id_manufactory = 0xC8,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int gd_nand_register_func(void) {
	return nand_register(&gd_nand);
}
