#include <stdio.h>
#include "nand_common.h"


static unsigned char issi_errstat_2[] = {0x2, 0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0x21, 2048, 2, 4, 2, 1, issi_errstat_2, 0),
};

static struct nand_desc issi_nand = {

	.id_manufactory = 0xC8,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int issi_nand_register_func(void) {
	return nand_register(&issi_nand);
}
