#include <config.h>
#include <stdio.h>
#include "ddr_params_creator.h"


struct ddr_chip_info supported_ddr_chips[] = {
#ifdef CONFIG_LPDDR2_M54D5121632A
	LPDDR2_M54D5121632A,
#endif
#ifdef CONFIG_LPDDR2_SCKX4BL512160AAC
	LPDDR2_SCKX4BL512160AAC,
#endif
#ifdef CONFIG_LPDDR2_SCB4BL256160AFL19GI
	LPDDR2_SCB4BL256160AFL19GI,
#endif
#ifdef CONFIG_DDR2_M14D2561616A
	DDR2_M14D2561616A,
#endif
#ifdef CONFIG_DDR2_M14D1G1664A
	DDR2_M14D1G1664A,
#endif
#ifdef CONFIG_DDR2_W975116NG18I
	DDR2_W975116NG18I,
#endif
#ifdef CONFIG_DDR2_W971GV6NG
	DDR2_W971GV6NG,
#endif
#ifdef CONFIG_LVDDR_W9464L6KH
	LVDDR_W9464L6KH,
#endif
};


void dump_ddr_info(struct ddr_chip_info *c)
{
	printf("name 		= %s\n", c->name);
	printf("id 		= %x\n", c->id);
	printf("type 		= %x\n", c->type);
	printf("size 		= %d\n", c->size);
	printf("DDR_ROW 	= %d\n", c->DDR_ROW);
	printf("DDR_ROW1 	= %d\n", c->DDR_ROW1);
	printf("DDR_COL 	= %d\n", c->DDR_COL);
	printf("DDR_COL1 	= %d\n", c->DDR_COL1);
	printf("DDR_BANK8 	= %d\n", c->DDR_BANK8);
	printf("DDR_BL 		= %d\n", c->DDR_BL);
	printf("DDR_RL 		= %d\n", c->DDR_RL);
	printf("DDR_WL 		= %d\n", c->DDR_WL);

	printf("DDR_tMRW 	= %d\n", c->DDR_tMRW);
	printf("DDR_tDQSCK 	= %d\n", c->DDR_tDQSCK);
	printf("DDR_tDQSCKMAX 	= %d\n", c->DDR_tDQSCKMAX);

	printf("DDR_tRAS 	= %d\n", c->DDR_tRAS);
	printf("DDR_tRTP 	= %d\n", c->DDR_tRTP);
	printf("DDR_tRP 	= %d\n", c->DDR_tRP);
	printf("DDR_tRCD 	= %d\n", c->DDR_tRCD);
	printf("DDR_tRC 	= %d\n", c->DDR_tRC);
	printf("DDR_tRRD 	= %d\n", c->DDR_tRRD);
	printf("DDR_tWR 	= %d\n", c->DDR_tWR);
	printf("DDR_tWTR 	= %d\n", c->DDR_tWTR);
	printf("DDR_tCCD 	= %d\n", c->DDR_tCCD);
	printf("DDR_tFAW 	= %d\n", c->DDR_tFAW);

	printf("DDR_tRFC 	= %d\n", c->DDR_tRFC);
	printf("DDR_tREFI 	= %d\n", c->DDR_tREFI);
	printf("DDR_tCKE 	= %d\n", c->DDR_tCKE);
	printf("DDR_tCKESR 	= %d\n", c->DDR_tCKESR);
	printf("DDR_tXSR 	= %d\n", c->DDR_tXSR);
	printf("DDR_tXP 	= %d\n", c->DDR_tXP);


}


int init_supported_ddr(void)
{

	return sizeof(supported_ddr_chips)/ sizeof(struct ddr_chip_info);
}

void dump_supported_ddr(void)
{
	struct ddr_chip_info *c = NULL;
	int i;

	for(i = 0; i < sizeof(supported_ddr_chips) / sizeof(struct ddr_chip_info); i++) {
		c = &supported_ddr_chips[i];
		dump_ddr_info(c);
	}
}

void create_supported_ddr_params(struct ddr_reg_value *reg_values)
{
	struct ddr_chip_info *chip;
	struct ddr_reg_value *reg;

	int i;

	for(i = 0; i < sizeof(supported_ddr_chips) / sizeof(struct ddr_chip_info); i++) {
		chip = &supported_ddr_chips[i];
		reg = &reg_values[i];

		__ps_per_tck = (1000000000 / (chip->freq / 1000));
		chip->init(chip);
		create_one_ddr_params(chip, reg);
	}
}
