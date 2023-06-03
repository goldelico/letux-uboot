#include <stdio.h>
#include "nand_common.h"

#define FM_MID			    0xA1
#define FM_NAND_DEVICD_COUNT	    2

static unsigned char fm_xaw[] = {0x2, 0x3};

static struct device_struct device[FM_NAND_DEVICD_COUNT] = {
	DEVICE_STRUCT(0xE4, 2048, 2, 4, 2, 2, fm_xaw),
	DEVICE_STRUCT(0xE5, 2048, 2, 4, 2, 2, fm_xaw),
};

static struct nand_desc fm_nand = {

	.id_manufactory = FM_MID,
	.device_counts = FM_NAND_DEVICD_COUNT,
	.device = device,
};

int fm_nand_register_func(void) {
	return nand_register(&fm_nand);
}
