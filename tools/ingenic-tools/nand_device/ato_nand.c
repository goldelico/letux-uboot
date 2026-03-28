#include <stdio.h>
#include "nand_common.h"


static struct device_struct device[] = {
	DEVICE_STRUCT(0x12, 2048, 2, 0, 0, 0, NULL, 0),
};

static struct nand_desc ato_nand = {

	.id_manufactory = 0x9b,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int ato_nand_register_func(void) {
	return nand_register(&ato_nand);
}
