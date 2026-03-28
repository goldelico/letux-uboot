#include <stdio.h>
#include "nand_common.h"


static unsigned char yhy_c9_errstat_2[] = {0x02,0x03};

static struct device_struct device[] = {
        DEVICE_STRUCT(0x21, 2048, 2, 4, 2, 1, yhy_c9_errstat_2, 0),
        DEVICE_STRUCT(0x52, 2048, 2, 4, 2, 1, yhy_c9_errstat_2, 0),
        DEVICE_STRUCT(0xD4, 4096, 2, 4, 2, 1, yhy_c9_errstat_2, 0),
 };

static struct nand_desc yhy_c9_nand = {
	.id_manufactory = 0xC9,
	.device_counts  = ARRAY_SIZE(device),
	.device = device,
};

int yhy_nand_register_func(void) {
	return nand_register(&yhy_c9_nand);
}
