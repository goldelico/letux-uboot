#ifndef __OTP_H__
#define __OTP_H__

#define SC_OTP_SEL_CKEY 0x1
#define SC_OTP_SEL_UKEY 0x2
#define SC_OTP_SEL_NKU 0x8
#define SC_OTP_RW  (0x1 << 5) //0:R  1:W
#define SC_OTP_RD  0
#define WT_OTP_UK   0x1
#define WT_OTP_NKU  0x4
#define WT_OTP_CK   0x8

/*register addr*/
#define EFUSE_REG_CTRL 0xb3540000
#define EFUSE_REG_CFG  0xb3540004
#define EFUSE_REG_STAT 0xb3540008
#define EFUSE_REG_DAT1 0xb354000C
#define EFUSE_REG_DAT2 0xb3540010
#define EFUSE_REG_DAT3 0xb3540014
#define EFUSE_REG_DAT4 0xb3540018
#define EFUSE_REG_DAT5 0xb354001C
#define EFUSE_REG_DAT6 0xb3540020
#define EFUSE_REG_DAT7 0xb3540024
#define EFUSE_REG_DAT8 0xb3540028

#define EFUSE_REGOFF_CRTL_ADDR 21
#define EFUSE_REGOFF_CRTL_LENG 16

#define EFUSE_REG_CTRL_PGEN (0x1 << 15)
#define EFUSE_REG_CTRL_WTEN (0x1 << 1)
#define EFUSE_REG_CTRL_RDEN (0x1)

#define EFUSTATE_NKU_PRT_SFT (0x1 << 21)
#define EFUSTATE_UK_PRT_SFT  (0x1 << 20)
#define EFUSTATE_CK_PRT_SFT  (0x1 << 19)
#define EFUSTATE_SCB_PRT_SFT  (0x1 << 10)
#define EFUSTATE_SECBOOT_EN_SFT (0x1 << 8)

#define EFUSE_REG_STAT_RDDONE (0x1 << 0)
#define EFUSE_REG_STAT_WTDONE (0x1 << 1)

#define EFUSE_ADDR_PROT    0x2e
#define EFUSE_PTCOFF_NKU   13 //nku protect
#define EFUSE_PTCOFF_UKP   12 //user key protect
#define EFUSE_PTCOFF_CKP   11 //chip key protect
#define EFUSE_PTCOFF_HIDE  10 //HIDEBLK protect
#define EFUSE_PTCOFF_SOC    9 //SOCINFO protect
#define EFUSE_PTCOFF_TR3    8 //TRIM3 protect
#define EFUSE_PTCOFF_TR2    7 //TRIM2 protect
#define EFUSE_PTCOFF_TR1    6 //TRIM1 protect
#define EFUSE_PTCOFF_TR0    5 //TRIM0 protect
#define EFUSE_PTCOFF_UID    4 //CUSTID0 protect
#define EFUSE_PTCOFF_CID    3 //CHIPID protect
#define EFUSE_PTCOFF_SCB    2 //secure boot protect
#define EFUSE_PTCOFF_DJG    1 //DISJTAG protect
#define EFUSE_PTCOFF_SEC    0 //SECBOOT_EN protect


#define EFUSE_REG32(a) *(volatile unsigned int *)(a)
#define EFUSTATE_CK_PRT  (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_CK_PRT_SFT)
#define EFUSTATE_UK_PRT  (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_UK_PRT_SFT)
#define EFUSTATE_NKU_PRT (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_NKU_PRT_SFT)
#define EFUSTATE_SCB_PRT (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_NKU_PRT_SFT)
#define EFUSTATE_SECBOOT_EN (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_SECBOOT_EN_SFT)


int cpu_burn_rckey(void);
int cpu_burn_nku(void *idata,unsigned int length);
int get_rsakeylen(void);
int cpu_get_enckey(unsigned int *odata);
int cpu_burn_ukey(void *idata);
int cpu_burn_secboot_enable(void);

#endif
