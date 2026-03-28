#include <stdio.h>
#include "nand_common.h"


static unsigned char unim_errstat_2[] = {0x2, 0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0x0C, 2048, 2, 4, 2, 1, unim_errstat_2, 0),
};

static struct nand_desc unim_nand = {

	.id_manufactory = 0xB0,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int unim_nand_register_func(void) {
	return nand_register(&unim_nand);
}
