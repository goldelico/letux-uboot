#include <stdio.h>
#include "nand_common.h"


static unsigned char fs_errstat_2[] = {0x2,0x3};
static unsigned char fs_errstat_1[] = {0x7};

static struct device_struct device[] = {
	DEVICE_STRUCT(0xA1, 2048, 2, 4, 3, 1, fs_errstat_1, 0),
	DEVICE_STRUCT(0xB1, 2048, 2, 4, 3, 1, fs_errstat_1, 0),
	DEVICE_STRUCT(0xEB, 2048, 2, 4, 2, 1, fs_errstat_2, 0),
	DEVICE_STRUCT(0xEA, 2048, 2, 4, 2, 1, fs_errstat_2, 0),
	DEVICE_STRUCT(0x71, 2048, 2, 4, 2, 1, fs_errstat_2, 0),
	DEVICE_STRUCT(0x70, 2048, 2, 4, 2, 1, fs_errstat_2, 0),
	DEVICE_STRUCT(0x72, 2048, 2, 4, 2, 1, fs_errstat_2, 0),
	DEVICE_STRUCT(0x53, 4096, 2, 4, 3, 1, fs_errstat_1, 0),
	DEVICE_STRUCT(0x52, 2048, 2, 4, 2, 1, fs_errstat_1, 0),
};

static struct nand_desc fs_nand = {

	.id_manufactory = 0xCD,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int foresee_nand_register_func(void) {
	return nand_register(&fs_nand);
}
