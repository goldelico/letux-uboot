#ifndef __DDR_CHIP_INFO_H__
#define __DDR_CHIP_INFO_H__

struct ddr_mr_config {
        unsigned char kgd_mr1_ds;
};

struct ddr2_mr_config {
        unsigned char kgd_mr0_dll_rst;
        unsigned char kgd_mr0_pd;
        unsigned char kgd_mr1_dll_en;
        unsigned char kgd_mr1_dic;
        unsigned char kgd_mr1_rtt_nom;
        unsigned char kgd_mr1_ocd;
        unsigned char kgd_mr2_dcc_en;
};

struct ddr3_mr_config {
        unsigned char kgd_mr0_dll_rst;
        unsigned char kgd_mr0_pd;
        unsigned char kgd_mr1_dll_en;
        unsigned char kgd_mr1_dic;
        unsigned char kgd_mr1_rtt_nom;
        unsigned char kgd_mr2_rtt_wr;
};

struct lpddr_mr_config {
        unsigned char kgd_mr2_ds;
};

struct lpddr2_mr_config {
        unsigned char kgd_mr3_ds;
};

struct lpddr3_mr_config {
        unsigned char kgd_mr3_ds;
        unsigned char kgd_mr11_odt;
};

/* KGD DLL & ODT & Drive Strength Tuning */
union kgd_mr_config {
        struct ddr_mr_config    ddr_config;
        struct ddr2_mr_config   ddr2_config;
        struct ddr3_mr_config   ddr3_config;
        struct lpddr_mr_config  lpddr_config;
        struct lpddr2_mr_config lpddr2_config;
        struct lpddr3_mr_config lpddr3_config;
};

struct kgd_config {
        unsigned char use_kgd_config;
        union kgd_mr_config mr_config;
};

/* PHY ODT & Drive Strength & SKEW Tuning */
struct phy_drvodt_config {

        unsigned char use_drvodt_config;

        unsigned char phy_pu_drv_cmd;
        unsigned char phy_pd_drv_cmd;
        unsigned char phy_pu_drv_ck;
        unsigned char phy_pd_drv_ck;
        unsigned char phy_pu_drv_dq7_0;
        unsigned char phy_pd_drv_dq7_0;
        unsigned char phy_pu_drv_dq15_8;
        unsigned char phy_pd_drv_dq15_8;

        unsigned char phy_pu_odt_dq7_0;
        unsigned char phy_pd_odt_dq7_0;
        unsigned char phy_pu_odt_dq15_8;
        unsigned char phy_pd_odt_dq15_8;
};

struct phy_deskew_config {

        unsigned char use_deskew_config;
	/* Per-bit de-skew of command signal are not defined in detail. */
        unsigned char phy_deskew_cmd;

        unsigned char phy_deskew_rx_dm0;
        unsigned char phy_deskew_tx_dm0;
        unsigned char phy_deskew_rx_dq7_0;
        unsigned char phy_deskew_tx_dq7_0;
        unsigned char phy_deskew_rx_dqs0;
        unsigned char phy_deskew_tx_dqs0;

        unsigned char phy_deskew_rx_dm1;
        unsigned char phy_deskew_tx_dm1;
        unsigned char phy_deskew_rx_dq15_8;
        unsigned char phy_deskew_tx_dq15_8;
        unsigned char phy_deskew_rx_dqs1;
        unsigned char phy_deskew_tx_dqs1;
};

struct ddr_chip_info {
	char name[32];
	unsigned int id;
	unsigned int type;	/*lpddr2/lpddr3...*/
	unsigned int size;	/*MBytes*/
	unsigned int freq;	/*ddr frequency.*/
	void (*init)(void *data);


	unsigned int DDR_ROW;		/* ROW : 12 to 14 row address for chip0 	*/
	unsigned int DDR_ROW1;		/* ROW1 : 12 to 14 row address for chip1 	*/
	unsigned int DDR_COL;		/* COL :  8 to 10 column address for chip0 	*/
	unsigned int DDR_COL1;		/* COL1 :  8 to 10 column address for chip1 	*/
	unsigned int DDR_BANK8;		/* DDR_BANK8: banks each chip: 0-4bank, 1-8bank */
	unsigned int DDR_BL;		/* LPDDR3 Burst length : 3 - 8 burst, 2 - 4 burst , 4 - 16 burst */
	unsigned int DDR_RL;
	unsigned int DDR_WL;


	unsigned int DDR_tMRW;		/* tMRD: Mode Register Delay . max(14nS, 10nCK ) */

	unsigned int DDR_tDQSCK;	/* tDQSCK: QS output access from ck_t/ck_c.*/
	unsigned int DDR_tDQSCKMAX;	/* tDQSCKMAX: MAX DQS output access from ck_t/ck_c.*/

	unsigned int DDR_tRAS;
	unsigned int DDR_tRTP;
	unsigned int DDR_tRP;
	unsigned int DDR_tRCD;
	unsigned int DDR_tRC;
	unsigned int DDR_tRRD;
	unsigned int DDR_tWR;
	unsigned int DDR_tWTR;
	unsigned int DDR_tCCD;
	unsigned int DDR_tFAW;

	unsigned int DDR_tRFC;		/* tRFC: AUTO-REFRESH command period.*/
	unsigned int DDR_tREFI;		/* tREFI: Refresh period: 4096 refresh cycles/64ms , line / 64ms*/
	unsigned int DDR_tCKE;
	unsigned int DDR_tCKESR;
	unsigned int DDR_tXSR;
	unsigned int DDR_tXP;

	/*DDR3 need.*/
	unsigned int DDR_tXSDLL;
	unsigned int DDR_tMOD;
	unsigned int DDR_tXPDLL;
	unsigned int DDR_tCKSRE;
	unsigned int DDR_CL;
	unsigned int DDR_CWL;

	/*DDR2 need.*/
	unsigned int DDR_tXSNR;
	unsigned int DDR_tXSRD;
	unsigned int DDR_tRDLAT;
	unsigned int DDR_tWDLAT;
	unsigned int DDR_tRTW;
	unsigned int DDR_tMRD;
	unsigned int DDR_tMINSR;
	unsigned int DDR_CLK_DIV;
	unsigned int DDR_tXARD;
	unsigned int DDR_tXARDS;
	unsigned int DDR_AL;

        struct kgd_config        kgd_config;
        struct phy_drvodt_config phy_drvodt;
        struct phy_deskew_config phy_deskew;
};

enum {
	VENDOR_WINBOND = 0x0,
	VENDOR_ESMT    = 0x1,
	VENDOR_NANYA   = 0x2,
	VENDOR_UNILC   = 0x3,
	VENDOR_ZENTEL  = 0x4,
	VENDOR_PMD     = 0x5,
};
#ifdef CONFIG_X1600
enum {
	TYPE_LPDDR2 = 0x0,
	TYPE_DDR2 = 0x1,
};
#else
enum {
	TYPE_LPDDR2 = 0x0,
	TYPE_LPDDR3 = 0x1,
	TYPE_DDR3 = 0x2,
	TYPE_DDR2 = 0x3,
};
#endif
enum {
	MEM_256M	= 0x0,
	MEM_128M	= 0x1,
	MEM_64M		= 0x2,
	MEM_32M		= 0x3,
	MEM_512M	= 0x4,
	MEM_16M		= 0x5,
	MEM_8M		= 0x6,
	MEM_1G		= 0x7,
};

#define DDR_CHIP_ID(vendor, type, capacity)	(type << 6 | vendor << 3 | capacity)

#include <asm/ddr_innophy.h>

#endif
