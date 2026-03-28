#include <config.h>
#include <stdio.h>
#include "ddr_params_creator.h"


struct ddr_chip_info supported_ddr_chips[] = {
#ifdef CONFIG_LPDDR2_W97BV6MK
	LPDDR2_W97BV6MK,
#endif
#ifdef CONFIG_LPDDR3_W63AH6NKB_BI
	LPDDR3_W63AH6NKB_BI,
#endif
#ifdef CONFIG_LPDDR2_M54D5121632A
	LPDDR2_M54D5121632A,
#endif
#ifdef CONFIG_LPDDR2_SCB4BL256160AFL19GI
	LPDDR2_SCB4BL256160AFL19GI,
#endif
#ifdef CONFIG_DDR3_W631GU6NG
	DDR3_W631GU6NG,
#endif
#ifdef CONFIG_DDR3L_W631GU6RG
	DDR3L_W631GU6RG,
#endif
#ifdef CONFIG_DDR2_M14D5121632A
	DDR2_M14D5121632A,
#endif
#ifdef CONFIG_DDR2_M14F5121632A
	DDR2_M14F5121632A,
#endif
#ifdef CONFIG_DDR3_M15T1G1664A
	DDR3_M15T1G1664A,
#endif
#ifdef	CONFIG_DDR3_NK5CC128M8HKX
	DDR3_NK5CC128M8HKX,
#endif
#ifdef	CONFIG_DDR3_PMF512816FBR_MBDN
	DDR3_PMF512816FBR_MBDN,
#endif
#ifdef CONFIG_DDR3L_W632GU6QG_11
	DDR3L_W632GU6QG_11,
#endif
#ifdef CONFIG_DDR3L_W634GU6QB_11
	DDR3L_W634GU6QB_11,
#endif
#ifdef CONFIG_LPDDR3_NK6CL256M16DKX_H1
	LPDDR3_NK6CL256M16DKX_H1,
#endif
#ifdef CONFIG_DDR3L_GDP1BFLM_CB
	DDR3L_GDP1BFLM_CB,
#endif

#ifdef CONFIG_DDR2_W9751V6NG
	DDR2_W9751V6NG,
#endif

#ifdef CONFIG_DDR2_SCKL18T512XX0AAE1
	DDR2_SCKL18T512XX0AAE1,
#endif
#ifdef  CONFIG_DDR3_NT5CB128M16JR
        DDR3_NT5CB128M16JR,
#endif

#ifdef CONFIG_X2670M_DDR2
	X2670M_DDR2,
#endif

#ifdef CONFIG_X2600M_DDR2
	X2600M_DDR2,
#endif

#ifdef CONFIG_X2600_DDR3
	X2600_DDR3,
#endif

#ifdef CONFIG_X2600E_DDR3
	X2600E_DDR3,
#endif

#ifdef CONFIG_X2600H_DDR3L
	X2600H_DDR3L,
#endif

#ifdef CONFIG_X2600N_DDR3
	X2600N_DDR3,
#endif

#ifdef CONFIG_X2670N_DDR3
	X2670N_DDR3,
#endif

#ifdef CONFIG_DDR3L_W634GU6QG_11
	DDR3L_W634GU6QG_11
#endif

#ifdef CONFIG_X2100_LPDDR2
	X2100_LPDDR2
#endif
};


void dump_ddr_info(struct ddr_chip_info *c)
{
	printf("/** Only DDR test\n");
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
	printf("**/ \n");
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
