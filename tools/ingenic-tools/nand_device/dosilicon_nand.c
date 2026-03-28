#include <stdio.h>
#include "nand_common.h"

static unsigned char ds_errstat_2[] = {0x2,0x3};
static unsigned char ds_errstat_1[] = {0x7};

static struct device_struct device[] = {
	DEVICE_STRUCT(0x71, 2048, 2, 4, 2, 1, ds_errstat_2, 0),
	DEVICE_STRUCT(0x72, 2048, 2, 4, 2, 1, ds_errstat_2, 1),
	DEVICE_STRUCT(0xF2, 2048, 2, 4, 3, 1, ds_errstat_1, 1),
	DEVICE_STRUCT(0x21, 2048, 2, 4, 2, 1, ds_errstat_2, 0),
	DEVICE_STRUCT(0x22, 2048, 2, 4, 2, 1, ds_errstat_2, 1),
	DEVICE_STRUCT(0xF1, 2048, 2, 4, 3, 1, ds_errstat_1, 1),
	DEVICE_STRUCT(0xB4, 2048, 2, 4, 3, 1, ds_errstat_1, 1),
        DEVICE_STRUCT(0x75, 2048, 2, 4, 3, 1, ds_errstat_1, 1),
        DEVICE_STRUCT(0xF4, 2048, 2, 4, 3, 1, ds_errstat_1, 1),
        DEVICE_STRUCT(0xB2, 2048, 2, 4, 3, 1, ds_errstat_2, 0),
};

static struct nand_desc dosilicon_nand = {
	.id_manufactory = 0xE5,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int dosilicon_nand_register_func(void) {
	return nand_register(&dosilicon_nand);
}
