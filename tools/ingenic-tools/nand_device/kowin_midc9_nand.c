#include <stdio.h>
#include "nand_common.h"


static unsigned char kowin_c9_errstat_1[]= {0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0xD4, 4096, 2, 4, 2, 1, kowin_c9_errstat_1, 0),
};

static struct nand_desc kowin_c9_nand = {

	.id_manufactory = 0xC9,
	.device_counts = ARRAY_SIZE(device),
	.device = device,
};

int kowin_midc9_nand_register_func(void) {
	return nand_register(&kowin_c9_nand);
}
