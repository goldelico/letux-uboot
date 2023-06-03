#include <stdio.h>
#include "nand_common.h"

#define XCSP_MID			    0x9c

#define XCSP_NAND_DEVICD_COUNT	    2

static unsigned char xcsp_xaw[] = {0x03};

static struct device_struct device[XCSP_NAND_DEVICD_COUNT] = {
	DEVICE_STRUCT(0x01, 2048, 2, 4, 2, 1, xcsp_xaw),
	DEVICE_STRUCT(0xb1, 2048, 2, 4, 2, 1, xcsp_xaw),
};

static struct nand_desc xcsp_nand = {

	.id_manufactory = XCSP_MID,
	.device_counts  = XCSP_NAND_DEVICD_COUNT,
	.device = device,
};

int xcsp_nand_register_func(void) {
	return nand_register(&xcsp_nand);
}
