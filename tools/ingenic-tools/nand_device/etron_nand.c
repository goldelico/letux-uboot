#include <stdio.h>
#include "nand_common.h"

static unsigned char etron_errstat_1[]= {0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0x15, 2048, 2, 4, 2, 1, etron_errstat_1, 0),
	DEVICE_STRUCT(0x25, 2048, 2, 4, 2, 1, etron_errstat_1, 0),
};

static struct nand_desc etron_nand = {

	.id_manufactory = 0x01,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int etron_nand_register_func(void) {
	return nand_register(&etron_nand);
}
