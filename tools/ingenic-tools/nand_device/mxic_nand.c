#include <stdio.h>
#include "nand_common.h"


static unsigned char mxic_errstat_2[] = {0x2, 0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0x12, 2048, 2, 4, 2, 1, mxic_errstat_2, 0),
	DEVICE_STRUCT(0x22, 2048, 2, 4, 2, 1, mxic_errstat_2, 1),
	DEVICE_STRUCT(0x26, 2048, 2, 4, 2, 1, mxic_errstat_2, 0),
	DEVICE_STRUCT(0x37, 4096, 2, 4, 2, 1, mxic_errstat_2, 0),
	DEVICE_STRUCT(0xA6, 2048, 2, 4, 2, 1, mxic_errstat_2, 0),
};

static struct nand_desc mxic_nand = {

	.id_manufactory = 0xc2,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int mxic_nand_register_func(void) {
	return nand_register(&mxic_nand);
}
