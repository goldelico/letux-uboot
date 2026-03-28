#ifndef __OTP_H__
#define __OTP_H__

#define SC_OTP_SEL_CKEY 0x1
#define SC_OTP_SEL_UKEY 0x2
#define SC_OTP_SEL_UKEY1 0x4
#define SC_OTP_SEL_NKU 0x8
#define SC_OTP_RW  (0x1 << 5) //0:R  1:W
#define SC_OTP_RD  0
#define SC_OTP_RIR_RW  0x1 //0:R  1:W
#define SC_OTP_RIR_RD  0 //0:R  1:W
#define WT_OTP_UK   0x1
#define WT_OTP_UK1   0x2
#define WT_OTP_NKU  0x4
#define WT_OTP_CK   0x8

#define EFUSE_SECURE_RKEY_ADDR 0xb3480240
#define EFUSE_SECURE_CKEY_ADDR 0xb3480250
#define EFUSE_SECURE_UKEY_ADDR 0xb3480260
#define EFUSE_SECURE_MD5_ADDR 0xb3480270
#define EFUSE_ADDR_PROT  0x1C

#define EFUSE_REGOFF_CRTL_ADDR 21
#define EFUSE_REGOFF_CRTL_LENG 16

#define EFUSE_REG_CTRL_PGEN (0x1 << 15)
#define EFUSE_REG_CTRL_WTEN (0x1 << 1)
#define EFUSE_REG_CTRL_RDEN (0x1)

#define EFUSE_REG_STAT_RDDONE 0x1
#define EFUSE_REG_STAT_WTDONE (0x1 << 1)


#define EFUSE_REG_CTRL 0xb3480000
	#define EFUSE_REGOFF_CRTL_ADDR 21
	#define EFUSE_REGOFF_CRTL_LENG 16
	#define EFUSE_REG_CTRL_PGEN (0x1 << 15)
	#define EFUSE_REG_CTRL_PS (0x1 << 9)
	#define EFUSE_REG_CTRL_RWL (0x1 << 11)
	#define EFUSE_REG_CTRL_PD (0x1 << 8)
	#define EFUSE_REG_CTRL_WTEN (0x1 << 1)
	#define EFUSE_REG_CTRL_RDEN (0x1)
#define EFUSE_REG_CFG  0xb3480004
  #define EFUSE_REG_CFG_INT_EN  (0x1 << 31)
  #define EFUSE_REG_CFG_RD_ADJ  (0xf << 24)
  #define EFUSE_REG_CFG_RD_STROBE  (0x1f << 16)
  #define EFUSE_REG_CFG_WR_ADJ  (0xf << 12)
  #define EFUSE_REG_CFG_WR_STROBE  (0x7ff << 0)
#define EFUSE_REG_STAT 0xb3480008
	#define EFUSE_REG_STAT_RDDONE (0x1 << 0)
	#define EFUSE_REG_STAT_WTDONE (0x1 << 1)
	#define EFUSTATE_UK_PRT_SFT (0x1 << 23)
	#define EFUSTATE_MD5_PRT_SFT (0x1 << 22)
	#define EFUSTATE_NKU_PRT_SFT (0x1 << 22)

#define EFUSE_REG_DAT0 0xb348000C
#define EFUSE_REG_DAT1 0xb3480010
#define EFUSE_REG_DAT2 0xb3480014
#define EFUSE_REG_DAT3 0xb3480018
#define EFUSE_REG_DAT4 0xb348001C
#define EFUSE_REG_DAT5 0xb3480020
#define EFUSE_REG_DAT6 0xb3480024
#define EFUSE_REG_DAT7 0xb3480028

#define EFUSE_PTCOFF_NKU   15 //nku protect
#define EFUSE_PTCOFF_UKP1  14 //user key1 protect
#define EFUSE_PTCOFF_UKP   13 //user key protect
#define EFUSE_PTCOFF_CKP   12 //chip key protect
#define EFUSE_PTCOFF_HIDE  11 //HIDEBLK protect
#define EFUSE_PTCOFF_SOC   10 //SOCINFO protect
#define EFUSE_PTCOFF_TR2    9 //TRIM2 protect
#define EFUSE_PTCOFF_TR1    8 //TRIM1 protect
#define EFUSE_PTCOFF_TR0    7 //TRIM0 protect
#define EFUSE_PTCOFF_UID2    6 //CUSTID2 protect
#define EFUSE_PTCOFF_UID1    5 //CUSTID1 protect
#define EFUSE_PTCOFF_UID    4 //CUSTID0 protect
#define EFUSE_PTCOFF_CID    3 //CHIPID protect
#define EFUSE_PTCOFF_SCB    2 //secure boot protect
#define EFUSE_PTCOFF_DJG    1 //DISJTAG protect
#define EFUSE_PTCOFF_SEC    0 //SECBOOT_EN protect


#define EFUSE_REG32(a) *(volatile unsigned int *)(a)

#define EFUSTATE_CK_PRT_SFT (0x1 << 20)
#define EFUSTATE_UK_PRT_SFT (0x1 << 21)
#define EFUSTATE_UK1_PRT_SFT (0x1 << 22)
#define EFUSTATE_NKU_PRT_SFT (0x1 << 23)
#define EFUSTATE_SCB_PRT_SFT (0x1 << 10)
#define EFUSTATE_SECBOOT_EN_SFT (0x1 << 8)

#define EFUSTATE_UK_PRT (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_UK_PRT_SFT)
#define EFUSTATE_UK1_PRT (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_UK1_PRT_SFT)
#define EFUSTATE_MD5_PRT (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_MD5_PRT_SFT)
#define EFUSTATE_NKU_PRT (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_NKU_PRT_SFT)
#define EFUSTATE_CK_PRT (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_CK_PRT_SFT)
#define EFUSTATE_SCB_PRT (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_SCB_PRT_SFT)
#define EFUSTATE_SECBOOT_EN (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_SECBOOT_EN_SFT)


int cpu_burn_rckey(void);
int cpu_burn_nku(void *idata,unsigned int length);
int get_rsakeylen(void);
int cpu_get_enckey(unsigned int *odata);
int cpu_burn_ukey(void *idata);
int cpu_burn_secboot_enable(void);

#endif
