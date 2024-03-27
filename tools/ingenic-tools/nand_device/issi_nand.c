#include <stdio.h>
#include "nand_common.h"

#define ISSI_MID			    0xC8
#define ISSI_NAND_DEVICD_COUNT	    1

static unsigned char issi_xaw[] = {0x2};

static struct device_struct device[] = {
	DEVICE_STRUCT(0x21, 2048, 2, 4, 2, 1, issi_xaw),
};

static struct nand_desc issi_nand = {

	.id_manufactory = ISSI_MID,
	.device_counts = ISSI_NAND_DEVICD_COUNT,
	.device = device,
};

int issi_nand_register_func(void) {
	return nand_register(&issi_nand);
}
