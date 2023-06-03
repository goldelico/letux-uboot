#ifndef __LVDDR_W9464L6KH_H
#define	__LVDDR_W9464L6KH_H


/* MDDR paramters */
static inline void LVDDR_W9464L6KH_init(void *data)
{
	struct ddr_chip_info *c = (struct ddr_chip_info *)data;

	c->DDR_ROW = 12; /* ROW : 12 to 14 row address */
	c->DDR_ROW1 = 12; /* ROW : 12 to 14 row address */
	c->DDR_COL = 8;/* COL :  8 to 10 column address */
	c->DDR_COL1 = 8;/* COL :  8 to 10 column address */
	c->DDR_BANK8 = 0; /* Banks each chip: 0-4bank, 1-8bank 0 for falcon fpga, 1 for develop board */
	c->DDR_CL = 3; /* CAS latency: 1 to 7 */


/*
 *  * MDDR controller timing1 register
 *   */
	c->DDR_tRAS = DDR__ns(40); /*t2 tRAS: ACTIVE to PRECHARGE command period to the same bank. */
	c->DDR_tRP = DDR__ns(15); /*t3 tRP: PRECHARGE command period to the same bank */
	c->DDR_tRCD = DDR__ns(15); /*t2 ACTIVE to READ or WRITE command period to the same bank. */
	c->DDR_tRC = (c->DDR_tRAS + c->DDR_tRP); /*t3 ACTIVE to ACTIVE command period to the same bank.*/
	c->DDR_tRRD = DDR__ns(10);   /*t3 ACTIVE bank A to ACTIVE bank B command period. */
	c->DDR_tWR = DDR__ns(15); /*t1 WRITE Recovery Time defined by register MR of DDR2 memory */
	c->DDR_tWTR = DDR__tck(2); /*t1 WRITE to READ command delay. */

/*
 *  * MDDR controller timing2 register
 *   */
	c->DDR_tRFC = DDR__ns(70); /*t4 ns,  AUTO-REFRESH command period. */

/*	c->DDR_tXP  DDR__tck(2) t4 EXIT-POWER-DOWN to next valid command period; note:get from JEDEC LPDDR*/
	c->DDR_tXP = DDR__ns(25); /*t4 EXIT-POWER-DOWN to next valid command period; note:get from JEDEC LPDDR*/
/*	c->DDR_tMRD DDR__ns(10) t4 unit: tCK Load-Mode-Register to next valid command period: 1 to 4 tCK */
	c->DDR_tMRD = DDR__tck(2); /*t4 unit: tCK Load-Mode-Register to next valid command period: 1 to 4 tCK */

/* new add */
	c->DDR_BL = 4;   /* MDDR Burst length: 3 - 8 burst, 2 - 4 burst , 1 - 2 burst*/
	c->DDR_RL = DDR__tck(c->DDR_CL);    /* MDDR: Read Latency = tAL + tCL */
	c->DDR_WL = DDR__tck(1);     /* MDDR: must 1 */

	c->DDR_tCKE = DDR__tck(2);        /*t4 CKE minimum pulse width, tCK */
	c->DDR_tXSR = DDR__tck(200);   /*t6?*/
/*	c->DDR_tDQSSMAX = DDR__tck(1);*/

/*
 *  * MDDR controller refcnt register
 *   */
	c->DDR_tREFI = DDR__ns(3900);   /* Refresh period: 4096 refresh cycles/64ms */

#ifdef CONFIG_LVDDR_INNOPHY
	c->DDR_tRTP = DDR__ns(8);   /* 7.5ns READ to PRECHARGE command period. */
	c->DDR_tCCD = DDR__tck(2);      /* CAS# to CAS# command delay , tCK*/
	c->DDR_tRTW = (((c->DDR_BL > 4) ? 6 : 4) + 1);/* 4 in case of BL=4, 6 in case of BL=8 */
	c->DDR_tFAW = DDR__ns(45);     /* Four bank activate period, ns */

	c->DDR_tXARD = DDR__tck(2);     /* DDR2 only: Exit active power down to read command , tCK*/
	c->DDR_tXARDS = DDR__tck(7);	/* DDR2 only: Exit active power down to read command (slow exit/low power mode), tCK */
	c->DDR_tXSNR = (c->DDR_tRFC + DDR__ns(10));   /*DDR2 only: Exit self-refresh to a non-read command , ns */
	c->DDR_tXSRD = DDR__tck(200);	/* DDR2 only : Exit self-refresh to a read command , tCK */

	c->DDR_tCKESR = DDR__tck(3);      /* CKE minimum pulse width, tCK */
	c->DDR_tCKSRE = DDR__ns(10000);   /* DDR2 no:Valid Clock Requirement after Self Refresh Entry or Power-Down Entry */

/*	c->DDR_tREFI       DDR__ns(15600)	* Refresh period: 4096 refresh cycles/64ms */

	c->DDR_CLK_DIV = 1;    /* Clock Divider. auto refresh*/
#endif
}

#ifndef CONFIG_LVDDR_W9464L6KH_MEM_FREQ
#define CONFIG_LVDDR_W9464L6KH_MEM_FREQ CONFIG_SYS_MEM_FREQ
#endif

#define LVDDR_W9464L6KH {					\
	.name 	= "W9464L6KH",					\
	.id	= DDR_CHIP_ID(VENDOR_WINBOND, TYPE_DDR2, MEM_8M),	\
	.type	= DDR2,						\
	.freq	= CONFIG_LVDDR_W9464L6KH_MEM_FREQ,			\
	.size	= 8,						\
	.init	= LVDDR_W9464L6KH_init,				\
}

#endif /* __MDDR_CONFIG_H */


