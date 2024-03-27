#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/cpm.h>
#include "secall.h"
#include "pdma.h"
#include "secure.h"


static void pdma_wait(void)
{
	__asm__ volatile (
			"       .set    push            \n\t"
			"       .set    noreorder       \n\t"
			"       .set    mips32          \n\t"
			"       li      $26, 0          \n\t"
			"       mtc0    $26, $12        \n\t"
			"       nop                     \n\t"
			"1:                             \n\t"
			"       wait                    \n\t"
			"       b       1b              \n\t"
			"       nop                     \n\t"
			"       .set    reorder         \n\t"
			"       .set    pop             \n\t"
			);
}

static void load_pdma_firmware(void)
{
	int i;
	unsigned int *pdma_ins = (unsigned int *)pdma_wait;
	unsigned int *dst_ptr = (unsigned int *)(TCSM_BANK0);//cacheable

	for(i=0; i < 6; i++)
		dst_ptr[i] = pdma_ins[i];
}

int init_seboot(void)
{
	int ret = 0;
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();

	printf("clkgate0: %x\n", REG32(CPM_BASE + CPM_CLKGR0));
	printf("clkgate1: %x\n", REG32(CPM_BASE + CPM_CLKGR1));
	REG32(CPM_BASE + CPM_CLKGR0) = 0;
	REG32(CPM_BASE + CPM_CLKGR1) = 0;
	printf("clkgate0: %x\n", REG32(CPM_BASE + CPM_CLKGR0));
	printf("clkgate1: %x\n", REG32(CPM_BASE + CPM_CLKGR1));

	reset_mcu();
	printf("%s %d: 0x%x\n",__func__,__LINE__,REG32(PDMA_BASE + DMCS_OFF));
	load_pdma_firmware();
	boot_up_mcu();
	udelay(50 * 1000);
	ret = otp_init();

	return ret;
}
