#ifndef __EFUSE_DDR_H__
#define __EFUSE_DDR_H__

enum
{
    /* par */
    ODT_PD, // 0
    ODT_PU,
    CMD_RC_PD,
    CMD_RC_PU,
    CLK_RC_PD,
    CLK_RC_PU,
    DQX_RC_PD,
    DQX_RC_PU,
    VREF,
    KGD_ODT,
    KGD_DS,
    KGD_RTT_DIC,
    SKEW_DQS0R,
    SKEW_DQS1R,
    SKEW_DQRX,
    SKEW_DQS0T,
    SKEW_DQS1T,
    SKEW_DQTX,
    SKEW_TRX,
    /* flag */
    INDEX_EN,
    INDEX_ODT_PU,
    INDEX_ODT_PD,
    INDEX_CMD_PU,
    INDEX_CMD_PD,
    INDEX_CLK_PU,
    INDEX_CLK_PD,
    INDEX_DQ_PU,
    INDEX_DQ_PD,
    INDEX_VREF,
    INDEX_KGD,
    INDEX_DQSR,
    INDEX_DQST,
    INDEX_DQRX,
    INDEX_DQTX,
    DDR_PAR_NUM,
};

typedef struct
{
    int odt[5];
    int cmd_rc[5];
    int ck_rc[5];
    int dq_rc[5];
    int kgd_odt[3];
    int kgd_ds[2];
    int vref[8];
} hanming_ddr_data;

#endif
