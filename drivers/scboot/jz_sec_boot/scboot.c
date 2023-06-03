#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>
#include <asm/reboot.h>
#include <asm/spl.h>

#include "secall.h"
#include "pdma.h"

/*
 * security boot.
 * 1. prepare a bin encrypted.
 *		|---------------|-------------------|					|
 *		| SC KEY(2048)	|  CODE encrypted   |
 *		|---------------|-------------------|					|
 * 2.
 *
 * */

#undef TCSM_CODE_ADDR
#undef TCSM_SC_KEY_ADDR
#undef MCU_TCSM_RETVAL
#undef MCU_TCSM_SECALL_MSG

#define TCSM_CODE_ADDR			(TCSM_BANK(1) + 0)
#define TCSM_SC_KEY_ADDR		(TCSM_BANK(1) + 2048)
#define MCU_TCSM_RETVAL			(TCSM_BANK(0) + 2048 + 1084) /* cal from sc_interface. */
#define MCU_TCSM_SECALL_MSG		(TCSM_BANK(0) + 2048 + 128) /* MCU_TCSM_SECALL_MSG */

#define SC_MAX_SIZE_PERTIME		(2048)
#define SC_MAGIC_SIZE			(512)
#define SC_KEY_SIZE				(1536)

#define SECURE_SCBOOT_MAGIC		0x54424353

#define USERKEY_ENCRYPT  0x2
#define CHIPKEY_ENCRYPT  0x1

#define NKU_NKEY_WORD_OFF 2
#define NKU_KUKEY_WORD_OFF (2+32)

#define PARAM_N_WORD_OFF 64
#define PARAM_KU_WORD_OFF 128
#define PARAM_RSASHA_WORD_OFF 192

#define MCU_TCSM_SPLSHA1ENCBUF_SIZE 264

#define SCKEY_INFO 64

#define RSASHA_SIZE 32

extern void flush_cache_all(void);

static int setup_sckeys(void *addr)
{
	volatile unsigned int *tcsmptr = (volatile unsigned int *)(TCSM_SC_KEY_ADDR);
	int iLoop = 0;
	int *ddrptr = (int *)addr;

	/* 384 * 4 = 1536, sc_key */
	for (iLoop = 0; iLoop < SC_KEY_SIZE / 4; iLoop++){
		tcsmptr[iLoop] = ddrptr[iLoop];
	}
	return tcsmptr[0]; //firmware length
}
struct sckey {
	uint32_t code_len;		/* the len of program code */
	uint32_t code_encrypt;	/* the way of program code crypt ---- 0:don't crypt 1: chipkey 2: userkey*/
	uint32_t kn_bit;				/* the bits of key n */
	uint32_t ku_bit;				/* the bits of key u ---- public key*/
	uint32_t key_encrypt;		/* the way of key crypt*/
	uint32_t reserved_0[59];// [64 - 5] /* pad to 2048bit / 256 Bytes / 64 words */

	uint32_t key_n[32];			/* key n*/
	uint32_t reserved_1[32]; //[64 - 32] /* pad to 2048bit / 256 Bytes / 64 words */

	uint32_t key_u[32];			/* key u*/
	uint32_t reserved_2[32];//[64 - 32] /* pad to 2048bit / 256 Bytes / 64 words */

	uint32_t code_md5_sig[32];	/* signature the md5 value of program code*/
	uint32_t reserved_3[32];//[64 - 32] /* pad to 2048bit / 256 Bytes / 64 words */
};

static int start_scboot(void *input, void *output, unsigned int binlen)
{
	volatile struct sc_args *args = (volatile struct sc_args *)(MCU_TCSM_SECALL_MSG);
	volatile unsigned int *tcsmptr = (volatile unsigned int *)(TCSM_CODE_ADDR);
	unsigned int ret;
	int iLoop = 0;
	int *srcptr = (int *)(input + sizeof(struct sckey));
	int *dstptr = (int *)(output);

	{
		int newround = 1;
		int endround = 0;
		int pos = 0;

		boot_up_mcu();

		do {
			int lens = binlen > SC_MAX_SIZE_PERTIME ? SC_MAX_SIZE_PERTIME : binlen;

			if (binlen <= SC_MAX_SIZE_PERTIME)
				endround = 1;

			args->arg[0] = endround << 1 | newround;
			args->arg[1] = lens;

			for (iLoop = 0; iLoop < lens / 4; iLoop++)
				tcsmptr[iLoop] = srcptr[pos++];

			newround = 0;

			ret = secall(args, SC_FUNC_SCBOOT, 0, 1);

			for (iLoop = 0; iLoop < lens / 4; iLoop++)
				dstptr[pos - lens / 4 + iLoop] = tcsmptr[iLoop];
			binlen -= SC_MAX_SIZE_PERTIME;
		} while (!endround);

		ret = *(volatile unsigned int *)MCU_TCSM_RETVAL;
		ret &= 0xFFFF;

	}
	return ret;
}

static void pdma_wait(void)
{
	__asm__ volatile (
		"	.set	push		\n\t"
		"	.set	noreorder	\n\t"
		"	.set	mips32		\n\t"
		"	li	$26, 0		\n\t"
		"	mtc0	$26, $12	\n\t"
		"	nop			\n\t"
		"1:				\n\t"
		"	wait			\n\t"
		"	b	1b		\n\t"
		"	nop			\n\t"
		"	.set	reorder		\n\t"
		"	.set	pop		\n\t"
		);
}

int scboot_only(void *input,void *output)
{
	u32 ret = 0;
	u32 len;
	u32 *pdma_ins = (u32 *)pdma_wait;
	unsigned int *pdma_bank0_off = (unsigned int *)TCSM_BANK0;
	u32 tmp;
	int i;
	/* start aes, pdma clk */
	tmp = cpm_readl(CPM_CLKGR);
	tmp &= ~(CPM_CLKGR_AES | CPM_CLKGR_PDMA);
	cpm_writel(tmp, CPM_CLKGR);

	for (i = 0; i < 6; i++)
		pdma_bank0_off[i] = pdma_ins[i];

	printf("Security boot...\n");

    len = setup_sckeys(input);

	if(len == 0) {
		printf("ERROR: please check header information!!\n");
		hang();
	}
	ret = start_scboot(input, output, len);
	if(ret) {
		printf("ERROR: please check your image !!\n");
		hang();
	}
// 现场清理，防止key泄露，不建议去掉
	memset((void*)TCSM_SC_KEY_ADDR, 0, SC_KEY_SIZE);
	memset((void*)MCU_TCSM_SPLSHA1ENCBUF, 0, RSASHA_SIZE);
	memset((void*)MCU_TCSM_NKU, 0, MCU_TCSM_SPLSHA1ENCBUF_SIZE);
	return ret;
}

int is_security_boot()
{
#define EFUSE_REG_STAT 0xb3540008
#define EFUSTATE_SECBOOT_EN_SFT (0x1 << 8)
	return *(volatile unsigned int *)(EFUSE_REG_STAT) & EFUSTATE_SECBOOT_EN_SFT;
}
