
#include <config.h>
#include <common.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>

#include "traps.h"

extern void handle_exception(void);

void inline set_handler(unsigned long offset, void *addr, unsigned long size)
{
	memcpy((void *)(KSEG0 + offset), addr, size);
}

char *get_exception(int num)
{
	switch(num) {
		case 0  : return "Interrupt";
		case 1  : return "TLB modification exception (4KEc core)";
		case 2  : return "TLB exception (load or instruction fetch) (4KEc core)";
		case 3  : return "TLB exception (store) (4KEc core)";
		case 4  : return "Address error exception (load or instruction fetch)";
		case 5  : return "Address error exception (store)";
		case 6  : return "Bus error exception (instruction fetch)";
		case 7  : return "Bus error exception (data reference: load or store)";
		case 8  : return "Syscall exception";
		case 9  : return "Breakpoint exception";
		case 10 : return "Reserved instruction exception";
		case 11 : return "Coprocessor Unusable exception";
		case 12 : return "Arithmetic Overflow exception";
		case 13 : return "Trap exception";
		case 16 : return "Implementation-Specific Exception 1 (COP2)";
		case 17 : return "Implementation-Specific Exception 2(COP2)";
		case 18 : return "Coprocessor 2 exceptions";
		case 23 : return "Reference to WatchHi/WatchLo address";
		case 24 : return "Machine check";
	}
	return "UNKNOW";
}

void dump_task()
{
	int i;
	unsigned int *p;
	struct task_info_regs *regs = *((unsigned int *)0x80000004);
	p = (int *)(*((volatile unsigned int *)(0x80000004)) + PT_LEN);

	serial_debug("epc:%x\n",read_c0_epc());
	serial_debug("status:%x\n",regs->status);
	serial_debug("cause:%x\n",read_c0_cause());

	serial_debug("\ndump regs:\n");
	serial_debug("ra[%08x] fp[%08x] t9[%08x] t8[%08x]\n", regs->ra,regs->fp,regs->t9,regs->t8);
	serial_debug("s7[%08x] s6[%08x] s5[%08x] s4[%08x]\n", regs->s7,regs->s6,regs->s5,regs->s4);
	serial_debug("s3[%08x] s2[%08x] s1[%08x] s0[%08x]\n", regs->s3,regs->s2,regs->s1,regs->s0);
	serial_debug("t7[%08x] t6[%08x] t5[%08x] t4[%08x]\n", regs->t7,regs->t6,regs->t5,regs->t4);
	serial_debug("t3[%08x] t2[%08x] t1[%08x] t0[%08x]\n", regs->t3,regs->t2,regs->t1,regs->t0);
	serial_debug("a3[%08x] a2[%08x] a1[%08x] a0[%08x]\n", regs->a3,regs->a2,regs->a1,regs->a0);
	serial_debug("v0[%08x] v1[%08x] lo[%08x] hi[%08x]\n", regs->v0,regs->v1,regs->lo,regs->hi);

	serial_debug("\nsp:%08x \n",p);
	serial_debug("================dump stack==============\n");
	for(i=0; i < 32; i++) {
		if(i%4 == 0) serial_debug("\naddr 0x%08x:  ",&p[i]);
		serial_debug("%08x ",p[i]);
	}
	serial_debug("\n\n=====================================================\n\n");

	p = read_c0_epc();
#define DUMP_CODE_SIZE	128
	unsigned int *up = (unsigned int) p | 0xa0000000;
	if(p < 0x80000000 || p >= 0x90000000) {
		serial_debug("invalid epc value!\n");
	} else {
		serial_debug("================dump code(cache)===============\n");
		for(i = 0; i < DUMP_CODE_SIZE; i++) {
			if(i%4 == 0) serial_debug("\naddr 0x%08x:  ",&p[i]);
			serial_debug("%08x ",p[i]);
		}

		serial_debug("\n================dump code(uncache)===============\n");
		for(i = 0; i < DUMP_CODE_SIZE; i++) {
			if(i%4 == 0) serial_debug("\naddr 0x%08x:  ",&up[i]);
			serial_debug("%08x ", up[i]);
		}
		serial_debug("\n");
	}
	serial_debug("==============done=============\n");
}

void __attribute__ ((noreturn)) print_exp(void)
{
	serial_debug("\nEXCEPTION:%s\n",get_exception((read_c0_cause() & 0x7c)>>2));
	dump_task();
	while(1)
		__asm__ __volatile__("wait\n\t");
}


int traps_init(void)
{
	*((volatile unsigned int *)(0x80000000)) = print_exp;
	set_handler(0x180, &handle_exception, 180);
	flush_cache_all();
	write_c0_cause(0x0 | 0x1<<23);
	write_c0_status(0x1000ff01);
	return 0;
}

