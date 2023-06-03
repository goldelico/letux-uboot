#include <stdio.h>
#include "nand_common.h"

#define FS_MID			    0xCD
#define FS_NAND_DEVICD_COUNT	    6

static unsigned char fs_xaw[] = {0x4, 0x7};
static unsigned char fs_xaw1[] = {0x2};
static unsigned char fs_xaw2[] = {0x2,0x3};

static struct device_struct device[] = {
	DEVICE_STRUCT(0xA1, 2048, 2, 4, 3, 2, fs_xaw),
	DEVICE_STRUCT(0xB1, 2048, 2, 4, 3, 2, fs_xaw),
	DEVICE_STRUCT(0xEB, 2048, 2, 4, 2, 1, fs_xaw1),
	DEVICE_STRUCT(0xEA, 2048, 2, 4, 2, 1, fs_xaw1),
	DEVICE_STRUCT(0x71, 2048, 2, 4, 2, 2, fs_xaw2),
	DEVICE_STRUCT(0x70, 2048, 2, 4, 2, 2, fs_xaw2),
};

static struct nand_desc fs_nand = {

	.id_manufactory = FS_MID,
	.device_counts = FS_NAND_DEVICD_COUNT,
	.device = device,
};

int foresee_nand_register_func(void) {
	return nand_register(&fs_nand);
}
