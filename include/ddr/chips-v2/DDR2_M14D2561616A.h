#ifndef __DDR2_M14D2561616A_CONFIG_H__
#define __DDR2_M14D2561616A_CONFIG_H__

/*
 * This file contains the memory configuration parameters for the cygnus board.
 */
/*--------------------------------------------------------------------------------
 * DDR2 info
 */
/* DDR2 paramters */
static inline void DDR2_M14D2561616A_init(void *data)
{
	struct ddr_chip_info *c = (struct ddr_chip_info *)data;

	c->DDR_ROW = 13;  /* ROW : 12 to 14 row address */
	c->DDR_COL = 9;  /* COL :  8 to 10 column address */
	c->DDR_ROW1 = 13;  /* ROW : 12 to 14 row address */
	c->DDR_COL1 = 9;  /* COL :  8 to 10 column address */

	c->DDR_BANK8 = 0; /* Banks each chip: 0-4bank, 1-8bank */
	c->DDR_CL = 7;   /* CAS latency: 1 to 7 */

	/*
	 * DDR2 controller timing1 register
	 */
	c->DDR_tRAS = DDR__ns(45);  /* tRAS: ACTIVE to PRECHARGE command period to the same bank. */
	c->DDR_tRTP = DDR__ns(8);   /* 7.5ns READ to PRECHARGE command period. */
	c->DDR_tRP = DDR__ps(13125);  /* tRP: PRECHARGE command period to the same bank */
	c->DDR_tRCD = DDR__ps(13125);  /* ACTIVE to READ or WRITE command period to the same bank. */
	// =  c->DDR_tRC =  (DDR_tRAS + DDR_tRP)  /* ACTIVE to ACTIVE command period to the same bank.*/
	c->DDR_tRC = DDR__ps(58125);  /* ACTIVE to ACTIVE command period to the same bank.*/
	c->DDR_tRRD = DDR__ns(8);  /* ACTIVE bank A to ACTIVE bank B command period. */
	c->DDR_tWR = DDR__ns(15);  /* WRITE Recovery Time defined by register MR of DDR2 memory */
	c->DDR_tWTR = DDR__ps(7500);  /* WRITE to READ command delay. */
	/*
	 * DDR2 controller timing2 register
	 */
	c->DDR_tRFC = DDR__ns(75); /* ns,  AUTO-REFRESH command period. */
	c->DDR_tMINSR = DDR__ns(60); /* Minimum Self-Refresh / Deep-Power-Down */
	c->DDR_tXP = DDR__tck(3);    /* tCK EXIT-POWER-DOWN to next valid command period: 1 to 8 tCK. */
	c->DDR_tMRD = DDR__tck(2);   /* unit: tCK. Load-Mode-Register to next valid command period: 1 to 4 tCK */

	/* new add */
	c->DDR_BL = 8;   /* Burst length: 3 - 8 burst, 2 - 4 burst , 1 - 2 burst */
	/* =  c->DDR_RL =  DDR__tck(DDR_CL)  [> DDR2: Read Latency = tAL + tCL <]*/
	/* =  c->DDR_WL =  (DDR_CL - 1)[>(DDR_tRL - 1)        DDR2: Write Latency = tAL + tCL - 1<]*/
	c->DDR_RL = DDR__tck(7);
	c->DDR_WL = DDR__tck(6);
	c->DDR_tCCD = DDR__tck(2);      /* CAS# to CAS# command delay , tCK*/
	c->DDR_tRTW = (((c->DDR_BL > 4) ? 6 : 4) + 1); /* 4 in case of BL=4, 6 in case of BL=8 */
	c->DDR_tFAW = DDR__ns(45);     /* Four bank activate period, ns */
	c->DDR_tCKE = DDR__tck(3);      /* CKE minimum pulse width, tCK */
	c->DDR_tCKESR = DDR__tck(3);      /* CKE minimum pulse width, tCK */
	c->DDR_tRDLAT = DDR__tck(7); //(DDR_tRL - DDR__tck(2));
	c->DDR_tWDLAT = DDR__tck(3);/*(DDR_tWL - 1)*/
	c->DDR_tXARD = DDR__tck(3);    /* DDR2 only: Exit active power down to read command , tCK*/
	c->DDR_tXARDS = DDR__tck(10);/* DDR2 only: Exit active power down to read command (slow exit/low power mode), tCK */

	c->DDR_tXSNR = (c->DDR_tRFC + DDR__ns(10));   /*DDR2 only: Exit self-refresh to a non-read command , ns */
	c->DDR_tXSRD = DDR__tck(200); /* DDR2 only : Exit self-refresh to a read command , tCK */
	c->DDR_tCKSRE = DDR__ns(10000);   /* DDR2 no:Valid Clock Requirement after Self Refresh Entry or Power-Down Entry */
	/*
	 * DDR2 controller refcnt register
	 */
	c->DDR_tREFI = DDR__ns(7800); /* Refresh period: ns */

	c->DDR_CLK_DIV = 1;    /* Clock Divider. auto refresh
         *  cnt_clk = memclk/(16*(2^DDR_CLK_DIV))
         */
}

#ifndef CONFIG_DDR2_M14D2561616A_MEM_FREQ
#define CONFIG_DDR2_M14D2561616A_MEM_FREQ CONFIG_SYS_MEM_FREQ
#endif

#define DDR2_M14D2561616A {					\
	.name = "M14D2561616A",					\
	.id = DDR_CHIP_ID(VENDOR_ESMT, TYPE_DDR2, MEM_32M),	\
	.type = DDR2,					\
	.freq = CONFIG_DDR2_M14D2561616A_MEM_FREQ,	\
	.size = 64,					\
	.init = DDR2_M14D2561616A_init,			\
}
#endif /* __DDR2_CONFIG_H */
