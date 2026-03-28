#include <stdio.h>
#include "nand_common.h"


static unsigned char zb_errstat_2[]= {0x2, 0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0x41,   2048, 2, 4, 2, 1, zb_errstat_2, 0),
	DEVICE_STRUCT(0xA1A1, 2048, 2, 4, 2, 1, zb_errstat_2, 0),
	DEVICE_STRUCT(0xA2A1, 2048, 2, 4, 2, 1, zb_errstat_2, 0),
	DEVICE_STRUCT(0xA3,   2048, 2, 4, 2, 1, zb_errstat_2, 0),
};

static struct nand_desc zb_nand = {

	.id_manufactory = 0x5E,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int zb_nand_register_func(void) {
	return nand_register(&zb_nand);
}
