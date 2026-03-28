#include <stdio.h>
#include "nand_common.h"


static unsigned char hik_errstat_2[] = {0x2, 0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0xD1D1, 2048, 2, 4, 2, 1, hik_errstat_2, 0),
	DEVICE_STRUCT(0xD2D1, 2048, 2, 4, 2, 1, hik_errstat_2, 0),
};

static struct nand_desc hik_nand = {

	.id_manufactory = 0x3C,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int hik_nand_register_func(void) {
	return nand_register(&hik_nand);
}
