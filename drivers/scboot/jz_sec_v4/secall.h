#ifndef _SECALL_H_
#define _SECALL_H_

#include "pdma.h"
#include "sc.h"

#define send_secall(func)	(REG32(DMCS) = (REG32(DMCS) & 0xf) | ((func)<<8) | 0x8)

#define polling_done(args) ({ \
			while (args->retval & 0x80000000);	\
    })

static int secall(volatile struct sc_args *argsx,unsigned int func,unsigned state,unsigned int wait){
	argsx->func = (func);
	argsx->state = (state);
	argsx->retval = 0x80000000;
/*	serial_debug("1..mcu control:%08x, func:%x\n", *(volatile unsigned int *)0xb3421030, func);*/
	send_secall(1);
/*	serial_debug("2..mcu control:%08x\n", *(volatile unsigned int *)0xb3421030);*/
	if (wait)
	{
		while (argsx->retval & 0x80000000) {
			/*serial_debug("dbg :%x\n", *(volatile unsigned int *)MCU_TCSM_DBG);*/
			/*serial_debug("tcsm bank 1:%08x\n", *(volatile unsigned int *)0xb3423000);*/
			/*serial_debug("3..mcu control:%08x\n", *(volatile unsigned int *)0xb3421030);*/
			/*serial_debug("wait secall excuted: retval:%x\n", argsx->retval);*/
		}
	}
/*	serial_debug("wait secall excuted: retval:%x\n", argsx->retval);*/
	return argsx->retval;
}
#define get_secall_off(x) ((unsigned int)x - TCSM_BANK(0))
#endif /* _SECALL_H_ */
