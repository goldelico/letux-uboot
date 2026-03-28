#include <stdio.h>
#include "nand_common.h"


static unsigned char fm_errstat_2[] = {0x2, 0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0xE4, 2048, 2, 4, 2, 1, fm_errstat_2, 0),
	DEVICE_STRUCT(0xE5, 2048, 2, 4, 2, 2, fm_errstat_2, 0),
	DEVICE_STRUCT(0xD4, 2048, 2, 4, 2, 1, fm_errstat_2, 0),
	DEVICE_STRUCT(0xD6, 2048, 2, 4, 2, 1, fm_errstat_2, 0),
};

static struct nand_desc fm_nand = {

	.id_manufactory = 0xA1,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int fm_nand_register_func(void) {
	return nand_register(&fm_nand);
}
