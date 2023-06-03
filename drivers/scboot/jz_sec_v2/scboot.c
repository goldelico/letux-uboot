#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>

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
	volatile unsigned int *tcsmptr = (volatile unsigned int *)(TCSM_SC_KEY_ADDR);
	int iLoop = 0;
	int *ddrptr = (int *)(addr + SC_MAGIC_SIZE);

	/* 384 * 4 = 1536, sc_key */
	for (iLoop = 0; iLoop < SC_KEY_SIZE/4; iLoop++)
		tcsmptr[iLoop] = ddrptr[iLoop];

	*len = tcsmptr[0]; /* image length */

	/* len must 4 wrod align */
	if((*len) == 0 || (*len) % 16)
		return -1;

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

#if 0
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

	args->arg[0] = 1 | 1 << 1 | 1 << 2;
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

