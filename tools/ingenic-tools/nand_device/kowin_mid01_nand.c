#include <stdio.h>
#include "nand_common.h"


static unsigned char kowin_01_errstat_1[]= {0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0x15, 2048, 2, 4, 2, 1, kowin_01_errstat_1, 0),
};

static struct nand_desc kowin_01_nand = {

	.id_manufactory = 0x01,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int kowin_mid01_nand_register_func(void) {
	return nand_register(&kowin_01_nand);
}
