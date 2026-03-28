#include <stdio.h>
#include "nand_common.h"


static unsigned char tc_errstat_2[] = {0x2, 0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0xC2, 2048, 2, 4, 2, 1, tc_errstat_2, 0),
	DEVICE_STRUCT(0xED, 4096, 2, 4, 2, 1, tc_errstat_2, 0),
};

static struct nand_desc tc_nand = {

	.id_manufactory = 0x98,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int tc_nand_register_func(void) {
	return nand_register(&tc_nand);
}
