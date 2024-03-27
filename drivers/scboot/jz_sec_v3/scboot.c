#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>

#include "rsa1.h"
#include "secall.h"
#include "pdma.h"
#include "otp.h"

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

#define TCSM_CODE_ADDR		(TCSM_BANK(7) + 0)
#define TCSM_SC_KEY_ADDR	(TCSM_BANK(7) + 2048)
#define MCU_TCSM_RETVAL		(TCSM_BANK(6) + 1108) /* cal from sc_interface. */
#define MCU_TCSM_SECALL_MSG	(TCSM_BANK(6) + 128)  /* MCU_TCSM_SECALL_MSG */

#define SC_MAX_SIZE_PERTIME		(2048)
#define SC_MAGIC_SIZE			(512)
#define SC_KEY_SIZE				(1536)

#define SECURE_SCBOOT_MAGIC		0x54424353

/* rsa key */
#define NKU_NKEY_WORD_OFF 2
#define NKU_KUKEY_WORD_OFF (2+64)
#define NKU_KEY_LEN (64)

/* sc key */
#define SC_KEY_WORD_SIZE	(64 * 4)

#define SC_KEY_INFO_WORD_SIZE	(64)
#define SC_KEY_N_WORD_SIZE	(64)
#define SC_KEY_KU_WORD_SIZE	(64)
#define SC_KEY_CODESIG_WORD_SIZE	(64)

#define SC_KEY_INFO_WORD_OFF	(0)
#define SC_KEY_N_WORD_OFF	(SC_KEY_INFO_WORD_OFF + SC_KEY_INFO_WORD_SIZE)
#define SC_KEY_KU_WORD_OFF	(SC_KEY_N_WORD_OFF + SC_KEY_N_WORD_SIZE)
#define SC_KEY_CODESIG_WORD_OFF	(SC_KEY_KU_WORD_OFF + SC_KEY_KU_WORD_SIZE)
/*
      ____________  0
      | head info  |
      |___________ |
      | env        |
      |___________ | 512b
      |  code_len  |
      |___________ | 516b
      |code_encrypt|
      |___________ | 520b
      | kn_bit     |
      |___________ | 524b
      | ku_bit     |
      |___________ | 528b
      |key_encrypt |
      |___________ | 532b
      |reserved    |
      |___________ | 768b
      |key_n       |
      |___________ | 1024b
      |key_u       |
      |___________ | 1280b
      |code_sig    |
      |___________ | 1536b
*/

static void secure_check(void *addr, int *issig)
{
	int *ddrptr = (int *)(addr);
	int p = (unsigned long)SECURE_SCBOOT_MAGIC;

	if(*ddrptr++ == p) {
		*issig = 1;
	}
}

static int setup_sckeys(void *addr, unsigned int *len)
{
	volatile struct sc_args *args = (volatile struct sc_args *)(MCU_TCSM_SECALL_MSG);
	volatile unsigned int *tcsmptr = (volatile unsigned int *)(TCSM_SC_KEY_ADDR);
	unsigned int *rsa_key = (unsigned int *)(TCSM_SC_KEY_ADDR + 1024);
	int *ddrptr = (int *)(addr + SC_MAGIC_SIZE);
	int iLoop = 0;
	unsigned int ret;

#ifdef CONFIG_X1600
	/* parsing sc_key: info */
	for (iLoop = 0; iLoop < SC_KEY_INFO_WORD_SIZE; iLoop++)
		tcsmptr[SC_KEY_INFO_WORD_OFF + iLoop]
			= ddrptr[SC_KEY_INFO_WORD_OFF + iLoop];

	/* parsing sc_key: codesig; (Note:Soft RSA needs to switch word between big and small end !) */
	for (iLoop = 0; iLoop < SC_KEY_CODESIG_WORD_SIZE; iLoop++)
		tcsmptr[SC_KEY_CODESIG_WORD_OFF + iLoop]
			= ddrptr[SC_KEY_CODESIG_WORD_OFF + (SC_KEY_CODESIG_WORD_SIZE - 1) - iLoop];

	/* parsing sc_key: n + ku */
	rsa_key[0] = tcsmptr[2];
	rsa_key[1] = tcsmptr[3];

	for (iLoop = 0; iLoop < SC_KEY_N_WORD_SIZE; iLoop++)
		rsa_key[NKU_NKEY_WORD_OFF + iLoop] = ddrptr[SC_KEY_N_WORD_OFF + iLoop];
	for (iLoop = 0; iLoop < SC_KEY_KU_WORD_SIZE; iLoop++)
		rsa_key[NKU_KUKEY_WORD_OFF + iLoop] = ddrptr[SC_KEY_KU_WORD_OFF + iLoop];

	*len = tcsmptr[0]; /* len in spl structure */

	/* len must 4 wrod align */
	if((*len) == 0 || (*len) % 16)
		return -1;

	/*
	 * security init:
	 * 1.clear scram KEYDONE segment
	 * 2.nku verify
	 */
	args->arg[0] = tcsmptr[4];
	args->arg[1] = MCU_TCSM_PADDR(rsa_key);

	ret = secall(args, SC_FUNC_INIT_SCRAM, 0, 1);
	/* rsa public decrtpt codesig: */
	{
		/* for rsa n; (Note:Soft RSA needs to switch word between big and small end !) */
		for (iLoop = 0; iLoop < SC_KEY_N_WORD_SIZE; iLoop++) {
			tcsmptr[SC_KEY_N_WORD_OFF + iLoop]
				= rsa_key[NKU_NKEY_WORD_OFF + (NKU_KEY_LEN - 1) - iLoop];
		}

		/* for rsa ku; (Note:Soft RSA needs to switch word between big and small end !) */
		tcsmptr[SC_KEY_KU_WORD_OFF]
			= rsa_key[NKU_KUKEY_WORD_OFF + (NKU_KEY_LEN - 1)];

		f_rsa_public_decrypt(tcsmptr + SC_KEY_CODESIG_WORD_OFF,
				tcsmptr + SC_KEY_CODESIG_WORD_OFF,
				SC_KEY_CODESIG_WORD_SIZE,
				tcsmptr + SC_KEY_N_WORD_OFF,
				tcsmptr + SC_KEY_KU_WORD_OFF,
				SC_KEY_N_WORD_SIZE);
	}
#else
	/* 384 * 4 = 1536, sc_key */
	for (iLoop = 0; iLoop < SC_KEY_SIZE/4; iLoop++)
		tcsmptr[iLoop] = ddrptr[iLoop];

	*len = tcsmptr[0]; /* image length */

	/* len must 4 wrod align */
	if((*len) == 0 || (*len) % 16)
		return -1;

#endif
	return 0;
}

extern void flush_cache_all(void);
static int start_scboot(void *input, void *output, unsigned int binlen)
{
	struct sc_args *args = (struct sc_args *)(MCU_TCSM_SECALL_MSG);
	unsigned int *tcsmptr = (unsigned int *)(TCSM_CODE_ADDR);
	unsigned int ret;
	int iLoop = 0;
	int *srcptr = (int *)(input + SC_MAGIC_SIZE + SC_KEY_SIZE);
	int *dstptr = (int *)(output);

#ifdef CONFIG_X1600
	int newround = 1;
	int endround = 0;
	int pos = 0;

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

//			if(ret)
//				return ret;
#else

	args->arg[0] = 1 | (1 << 1) | (1 << 2); //bit 0:newround bit 1:endround bit 2:dmamode
	args->arg[2] = virt_to_phys(srcptr);

	flush_cache_all();
	ret = secall(args, SC_FUNC_SCBOOT, 0, 1);
	flush_cache_all();
#endif

	ret = *(volatile unsigned int *)MCU_TCSM_RETVAL;
	ret &= 0xFFFF;

	return ret;
}

void pdma_wait(void)
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

int secure_scboot(void *input, void *output)
{
	unsigned int ret = 0;
	unsigned int len;
	unsigned int *pdma_ins = (unsigned int *)pdma_wait;
	volatile unsigned int *pdma_bank0_off = (unsigned int *)TCSM_BANK0;
	int issig = 0;
	int tmp, i;

	/* start aes, pdma clk */
//	tmp = cpm_readl(CPM_CLKGR);
//	tmp &= ~(CPM_CLKGR_AES | CPM_CLKGR_PDMA);
//	cpm_writel(tmp, CPM_CLKGR);

	for (i = 0; i < 6; i++)
		pdma_bank0_off[i] = pdma_ins[i];

	boot_up_mcu();

	secure_check(input, &issig);

	if(EFUSTATE_SECBOOT_EN == 0) {
		if (issig == 0) {
			printf("Normal boot...\n");
			return 0;
		} else {
			printf("ERROR: please check image size !!\n");
			return -1;
		}
	} else if (EFUSTATE_SECBOOT_EN) {
		if(issig == 1) {
			printf("Security boot...\n");
			ret = setup_sckeys(input, &len);
			if(ret) {
				printf("ERROR: please check image size, ret = %x !!\n", ret);
				return -1;
			}

			ret = start_scboot(input, output, len);
			if(ret) {
				printf("ERROR: please check your image, ret = %x!!\n", ret);
				return -1;
			}
		} else {
			printf("ERROR: please sign your image !!\n");
			return -1;
		}
	}

	return ret;
}

int is_security_boot()
{
#define EFUSE_REG_STAT 0xb3540008
#define EFUSTATE_SECBOOT_EN_SFT (0x1 << 8)
	return *(volatile unsigned int *)(EFUSE_REG_STAT) & EFUSTATE_SECBOOT_EN_SFT;
}
