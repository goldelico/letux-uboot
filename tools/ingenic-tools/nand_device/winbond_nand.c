#include <stdio.h>
#include "nand_common.h"


static unsigned char winbond_errstat_2[] = {0x2, 0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0xAA21, 2048, 2, 4, 2, 1, winbond_errstat_2, 0),
	DEVICE_STRUCT(0xAA22, 2048, 2, 4, 2, 1, winbond_errstat_2, 1),
	DEVICE_STRUCT(0xAB21, 2048, 2, 4, 2, 1, winbond_errstat_2, 0),
	DEVICE_STRUCT(0xAE21, 2048, 2, 4, 2, 1, winbond_errstat_2, 1),
};

static struct nand_desc winbond_nand = {

	.id_manufactory = 0xEF,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int winbond_nand_register_func(void) {
	return nand_register(&winbond_nand);
}
