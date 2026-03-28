#include <stdio.h>
#include "nand_common.h"


static unsigned char zetta_errstat_2[] = {0x2, 0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0x71, 2048, 2, 4, 2, 1, zetta_errstat_2, 0),
	DEVICE_STRUCT(0x72, 2048, 2, 4, 2, 1, zetta_errstat_2, 1),
};

static struct nand_desc zetta_nand = {

	.id_manufactory = 0xBA,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int zetta_nand_register_func(void) {
	return nand_register(&zetta_nand);
}
