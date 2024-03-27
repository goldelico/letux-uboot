#include <stdio.h>
#include "nand_common.h"

#define YHY_C9_MID              0XC9

#define YHY_C9_NAND_DEVICD_COUNT	    2


static unsigned char yhy_c9_xaw[] = {0x02,0x03};

static struct device_struct device[YHY_C9_NAND_DEVICD_COUNT] = {
        DEVICE_STRUCT(0x21, 2048, 2, 4, 2, 2, yhy_c9_xaw),
        DEVICE_STRUCT(0x52, 2048, 2, 4, 2, 2, yhy_c9_xaw),
 };


static struct nand_desc yhy_c9_nand = {
	.id_manufactory = YHY_C9_MID,
	.device_counts  = YHY_C9_NAND_DEVICD_COUNT,
	.device = device,
};

int yhy_nand_register_func(void) {
	return nand_register(&yhy_c9_nand);
}
