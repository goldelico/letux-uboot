#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include "secall.h"
#include "pdma.h"
#include "aes.h"

int do_aes_dma(void *in_paaddr, void *out_paaddr, int len, int aeskey, int aescrypt)
{
	unsigned int ret = 0;
	volatile struct sc_args *args = (volatile struct sc_args *)(MCU_TCSM_SECALL_MSG);
	args->arg[0] |= aeskey | AES_CRYPT | AES_MODE;


	args->arg[1] = in_paaddr;
	args->arg[2] = out_paaddr;
	args->arg[3] = len;

	flush_cache_all();
	secall(args, SC_FUNC_SPLCKENC, 0, 1);
	flush_cache_all();
	ret = *(volatile unsigned int *)MCU_TCSM_RETVAL;

	ret &= 0xFFFF;

	return ret;
}

