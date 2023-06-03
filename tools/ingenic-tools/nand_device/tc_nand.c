#include <stdio.h>
#include "nand_common.h"

#define TC_MID			    0x98
#define TC_NAND_DEVICD_COUNT	    1

static unsigned char tc_xaw[] = {0x2};

static struct device_struct device[1] = {
	DEVICE_STRUCT(0xC2, 2048, 2, 4, 2, 1, tc_xaw),
};

static struct nand_desc tc_nand = {

	.id_manufactory = TC_MID,
	.device_counts = TC_NAND_DEVICD_COUNT,
	.device = device,
};

int tc_nand_register_func(void) {
	return nand_register(&tc_nand);
}
