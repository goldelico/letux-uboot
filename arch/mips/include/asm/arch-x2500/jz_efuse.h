#ifndef __JZ_EFUSE_H__
#define __JZ_EFUSE_H__

struct jz_efuse_platform_data {
	int gpio_vddq_en_n;	/* supply 1.8V to VDDQ */
	int gpio_vddq_en_n_loop;
};

enum segment_id {
	CHIP_ID,
	USER_ID,
	SARADC_CAL_DAT,
	TRIM_DATA,
	PROGRAM_PROTECT,
	CPU_ID,
        SPECIAL_USE,
	CUSTOMER_RESV,
};

void jz_efuse_id_read(int is_chip_id, uint32_t *buf);

#endif
