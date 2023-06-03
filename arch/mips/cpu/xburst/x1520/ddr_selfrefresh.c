#include <config.h>
#include <common.h>
#include <ddr/ddr_common.h>
#include <generated/ddr_reg_values.h>
#include <asm/io.h>
#include <asm/jz_cache.h>
#include <asm/arch/clk.h>
#include <asm/mipsregs.h>


static unsigned int * test_data;
static unsigned int * test_data_uncache;
extern int ddr_training_hardware(int bypass);
extern void print_self(void);

struct sleep_save_register
{
	unsigned int lcr;
	unsigned int opcr;
	unsigned int spcr0;
	unsigned int gate;
	unsigned int ddr_ctrl;
	unsigned int ddr_autosr;
	unsigned int ddr_dlp;
	/* unsigned int sleep_voice_enable; */
	unsigned int ddr_training_space[20];
};

#define cache_prefetch(label,size)					\
do{									\
	register unsigned long addr,end;						\
	/* Prefetch codes from label */					\
	addr = (unsigned long)(&&label) & ~(32 - 1);			\
	end = (unsigned long)(&&label + size) & ~(32 - 1);		\
	end += 32;							\
	for (; addr < end; addr += 32) {				\
		__asm__ volatile (					\
				".set mips32\n\t"			\
				" cache %0, 0(%1)\n\t"			\
				".set mips32\n\t"			\
				:					\
				: "I" (0x1c), "r"(addr));	\
	}								\
}									\
while(0)

#define TCSM_DELAY(x)					\
	do{							\
		register unsigned int i = x;			\
	while(i--)					\
		__asm__ volatile(".set mips32\n\t"	\
				 "nop\n\t"		\
					 ".set mips32");	\
	}while(0)

void dump_reg()
{
	printf("DDRC_DLP: %x\n",ddr_readl(DDRC_DLP));
	printf("DDRP_DSGCR: %x\n",ddr_readl(DDRP_DSGCR));
	printf("DDRP_ACDLLCR: %x\n",ddr_readl(DDRP_ACDLLCR));
	printf("DDRP_DX0DLLCR: %x\n",ddr_readl(DDRP_DXDLLCR(0)));
	printf("DDRP_DX1DLLCR: %x\n",ddr_readl(DDRP_DXDLLCR(1)));
	printf("DDRP_DX2DLLCR: %x\n",ddr_readl(DDRP_DXDLLCR(2)));
	printf("DDRP_DX3DLLCR: %x\n",ddr_readl(DDRP_DXDLLCR(3)));
}

void set_gpio_irq(void)
{
	//gpio interrupt
	// int
	*(volatile unsigned int*)(0xb0010200 + 0x14) = 0x1 << 1;
	//mask
	*(volatile unsigned int*)(0xb0010200 + 0x28) = 0x1 << 1;
	//pad1
	*(volatile unsigned int*)(0xb0010200 + 0x34) = 0x1 << 1;
	//pad0
	*(volatile unsigned int*)(0xb0010200 + 0x44) = 0x1 << 1;
	//flag
	*(volatile unsigned int*)(0Xb0010200 + 0x58) = 0xffffffff;

	*(volatile unsigned int*)(0Xb000100c) = (1 << 15);

	while(0) {
		printf("#####INTPEND0:0x%x\n", *(volatile unsigned int*)(0xB0001010));
	}
}
void set_gpio_irq_clear(void)
{
	//gpio interrupt
	// int
	*(volatile unsigned int*)(0xb0010200 + 0x18) = 0x1 << 1;
	//mask
	*(volatile unsigned int*)(0xb0010200 + 0x24) = 0x1 << 1;
	//pad1
	*(volatile unsigned int*)(0xb0010200 + 0x38) = 0x1 << 1;
	//pad0
	*(volatile unsigned int*)(0xb0010200 + 0x44) = 0x1 << 1;
	//flag
	*(volatile unsigned int*)(0Xb0010200 + 0x58) = 0xffffffff;

	*(volatile unsigned int*)(0Xb000100c) = (1 << 15);
}

#define TEST_SIZE (4*1024*1024)
static struct sleep_save_register s_reg;

void ddr_selfresh_test()
{
	unsigned int val;
	unsigned int run = 1;
	unsigned int bypassmode;
	int ret;
	unsigned int opcr, lcr;
	test_data = 0x80000000 + 0x100;

	set_gpio_irq();
	test_data_uncache = (unsigned int *)((unsigned int)test_data | 0xa0000000);
	for(val = 0; val < TEST_SIZE / 4;val++)
		test_data_uncache[val] = (unsigned int)&test_data_uncache[val];

	s_reg.opcr  = *(volatile unsigned int *)(0xb0000024);
	s_reg.lcr  = *(volatile unsigned int *)(0xb0000004);
	opcr = s_reg.opcr;
	lcr = s_reg.lcr;
	opcr &= ~((1 << 24) | (0xfff << 8) | (1 << 4) | (1 << 2));
//	opcr |= (1 << 31) | (1 << 30) | (1 << 24) | (0x15 << 8) | (3 << 6) | (1 << 4);
	opcr |= (1 << 31) | (1 << 30) | (1 << 24) | (0x15 << 8) | (3 << 6) | (1 << 2);
	lcr &= ~3;
	lcr |= 0x1;
	*(volatile unsigned int *)(0xb0000004) = lcr;
	*(volatile unsigned int *)(0xb0000024) = opcr;

	ddr_writel(0, DDRP_DTAR);
	s_reg.ddr_dlp = ddr_readl(DDRC_DLP);
	s_reg.ddr_ctrl = ddr_readl(DDRC_CTRL);
	s_reg.ddr_autosr = ddr_readl(DDRC_AUTOSR_EN);
	ddr_writel(0,DDRC_AUTOSR_EN); // exit auto sel-refresh

	bypassmode = ddr_readl(DDRP_PIR) & DDRP_PIR_DLLBYP;
	if(!bypassmode)
	{
		ddr_writel(0xf003 , DDRC_DLP);
		/* val = ddr_readl(DDRP_DSGCR); */
		/* val |= (1 << 4); */
		/* ddr_writel(val,DDRP_DSGCR); */
	}
//	print_self();
//	flush_dcache_all();
//	flush_icache_all();
//	flush_scache_all();
//	cache_prefetch(test_pre, 256);

	__sync();
	__fast_iob();
test_pre:
	while(run--){
		val = ddr_readl(DDRC_CTRL);
		val &= ~(0x1f << 11);
		val |= (1 << 17) | (1 << 5);
		ddr_writel(val, DDRC_CTRL); //enter selrefresh.
		while(!(ddr_readl(DDRC_STATUS) & (1 << 2))); // wait finish.
		asm volatile (
			"nop \n\t"
			"wait \n\t"
			"nop \n\t"
			);
		bypassmode = ddr_readl(DDRP_PIR) & DDRP_PIR_DLLBYP;
		if(!bypassmode) {
			/**
			 * reset dll of ddr.
			 * WARNING: 2015-01-08
			 * 	DDR CLK GATE(CPM_DRCG 0xB00000D0), BIT6 must set to 1 (or 0x40).
			 * 	If clear BIT6, chip memory will not stable, gpu hang occur.
			 */
			/* { */
			/* 	val = ddr_readl(DDRP_DSGCR); */
			/* 	val &= ~(1 << 4); */
			/* 	ddr_writel(val,DDRP_DSGCR); */
			/* } */
#define CPM_DRCG (0xB00000D0)

			*(volatile unsigned int *)CPM_DRCG |= (1<<1);
			TCSM_DELAY(0x1ff);
			*(volatile unsigned int *)CPM_DRCG &= ~(1<<1);
			TCSM_DELAY(0x1ff);
			/*
			 * for disabled ddr enter power down.
			 */
			*(volatile unsigned int *)0xb301102c &= ~(1 << 4);
			TCSM_DELAY(0xf);

			/*
			 * reset dll of ddr too.
			 */
			*(volatile unsigned int *)CPM_DRCG |= (1<<1);
			TCSM_DELAY(0x1ff);
			*(volatile unsigned int *)CPM_DRCG &= ~(1<<1);
			TCSM_DELAY(0x1ff);

			val = DDRP_PIR_INIT | DDRP_PIR_ITMSRST  | DDRP_PIR_DLLSRST | DDRP_PIR_DLLLOCK;// | DDRP_PIR_ZCAL  ;
			ddr_writel(val, DDRP_PIR);
			val = DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_DIDONE;// | DDRP_PGSR_ZCDONE;
			while ((ddr_readl(DDRP_PGSR) & val) != val) {
				if(ddr_readl(DDRP_PGSR) & (DDRP_PGSR_DTERR | DDRP_PGSR_DTIERR)) {
					break;
				}
			}
		}

		val = ddr_readl(DDRC_CTRL);
		val &= ~((1 << 5) | (1 << 17));
		ddr_writel(val,DDRC_CTRL); //exit selrefresh.
		while(ddr_readl(DDRC_STATUS) & (1 << 2)); // wait finish.

		if(!bypassmode){
			val = DDRP_PIR_INIT | DDRP_PIR_QSTRN;
		}else
			val = DDRP_PIR_INIT | DDRP_PIR_QSTRN | DDRP_PIR_DLLBYP;
		ddr_writel(val, DDRP_PIR);
		val = (DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_DIDONE | DDRP_PGSR_DTDONE);
		while ((ddr_readl(DDRP_PGSR) & val) != val) {
			if(ddr_readl(DDRP_PGSR) & (DDRP_PGSR_DTERR | DDRP_PGSR_DTIERR)) {
				break;
			}
		}
		if(!bypassmode)
		{
			*(volatile unsigned int *)0xb301102c |= (1 << 4);
			TCSM_DELAY(0xf);
		}
		if(!s_reg.ddr_dlp && !bypassmode)
		{
			ddr_writel(0x0 , DDRC_DLP);
		}
		if(s_reg.ddr_autosr) {
			ddr_writel(1,DDRC_AUTOSR_EN);   // enter auto sel-refresh
		}
		ddr_writel(s_reg.ddr_ctrl, DDRC_CTRL);
		*(volatile unsigned int *)(0xb0000004) = s_reg.lcr;
		*(volatile unsigned int *)(0xb0000024) = s_reg.opcr;
		*(volatile unsigned int*)(0Xb0010200 + 0x58) = 0xffffffff;

		for(val = 0; val < TEST_SIZE/ 4;val++) {
			if(test_data_uncache[val] != (unsigned int)&test_data_uncache[val])
			{
				printf("%d: d=%x  e=%x\n",val,test_data_uncache[val],(unsigned int)&test_data_uncache[val]);
				run = 1;
			}
		}
		set_gpio_irq_clear();
		printf("ddr test finish!\n");
	}
}
void print_self(void)
{
	printf("#####clk_gate0:	0x%x\n", *(volatile unsigned int*)(0xB0000020));
	printf("#####clk_gate1:	0x%x\n", *(volatile unsigned int*)(0xB0000028));
	printf("#####cpm_cpcr:	0x%x\n", *(volatile unsigned int*)(0xB0000024));
	printf("#####cpm_LCR:	0x%x\n", *(volatile unsigned int*)(0xB0000004));
	printf("#####sram_pw0:	0x%x\n", *(volatile unsigned int*)(0xB00000b8));
	printf("#####sram_pw1:	0x%x\n", *(volatile unsigned int*)(0xB00000c0));
	printf("#####bus_ctl:	0x%x\n", *(volatile unsigned int*)(0xB00000c4));
	printf("#####ddr_st:	0x%x\n", *(volatile unsigned int*)(0xB34f0000));
	printf("#####ddr_ctl:	0x%x\n", *(volatile unsigned int*)(0xB34f0008));
	printf("#####cp0_cfg0:	0x%x\n",read_c0_config());
	printf("#####cp0_cfg1:	0x%x\n",read_c0_config1());
	printf("#####cp0_cfg2:	0x%x\n",read_c0_config2());
	printf("#####cp0_cfg3:	0x%x\n",read_c0_config3());
}
