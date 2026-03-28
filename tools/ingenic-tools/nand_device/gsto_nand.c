#include <stdio.h>
#include "nand_common.h"


static unsigned char gsto_errstat_2[] = {0x2, 0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0xca13, 2048, 2, 4, 2, 1, gsto_errstat_2, 0),
	DEVICE_STRUCT(0xca23, 2048, 2, 4, 2, 1, gsto_errstat_2, 0),
};

static struct nand_desc gsto_nand = {

	.id_manufactory = 0x52,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int gsto_nand_register_func(void) {
	return nand_register(&gsto_nand);
}
