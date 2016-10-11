/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Simple command line monitor
 * run in Kermit mode by starting with "M"
 */
#include <common.h>

#include <asm/arch/mux.h>

#define putc serial_putc
#define tstc serial_tstc


/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}
static inline void udelay(unsigned long us)
{
	delay(us * 200); /* approximate */
}

#ifndef CONFIG_OMAP44XX

typedef struct gpio {
	unsigned char res1[0x34];
	unsigned int oe;		/* 0x34 */
	unsigned int datain;            /* 0x38 */
	unsigned char res2[0x54];
	unsigned int cleardataout;	/* 0x90 */
	unsigned int setdataout;	/* 0x94 */
} gpio_t;

// unclear if the functional clocks are enabled!

static gpio_t *blocks[]={ // GPIO1 .. 6
	OMAP34XX_GPIO1_BASE,
	OMAP34XX_GPIO2_BASE,
	OMAP34XX_GPIO3_BASE,
	OMAP34XX_GPIO4_BASE,
	OMAP34XX_GPIO5_BASE,
	OMAP34XX_GPIO6_BASE
};

static inline int gpio_get(int n)
{
	int bit=n % 32;
	gpio_t *base=blocks[n/32];
	return (base->datain >> bit)&1;
}

static inline int gpio_is_input(int n)
{ // geht nicht richtig
	int bit=n % 32;
	gpio_t *base=blocks[n/32];
	return (base->oe >> bit)&1;
}

#endif

static int strcmp(char *s1, char *s2)
{
	while(*s1 == *s2) {
		if(*s1 == 0)
			return 0;	// same
		s1++;
		s2++;		
	}
	return *s1 > *s2?1:-1;
}

static long xtol(char *s)
{
	long val=0;
	while(1)
		{
		if(*s >= '0' && *s <= '9')
			val=16*val+(*s++-'0');
		else if(*s >= 'A' && *s <= 'F')
			val=16*val+(*s++-'A'+10);
		else if(*s >= 'a' && *s <= 'f')
			val=16*val+(*s++-'a'+10);
		else
			break;
		}
	return val;
}

static char *addr=(char *) CFG_LOADADDR;
static char line[100];
static char *argv[10];

#define __raw_readl(a)    (*(volatile unsigned int *)(a))
#define __raw_writel(v,a) (*(volatile unsigned int *)(a) = (v))
#define __raw_readw(a)    (*(volatile unsigned short *)(a))
#define __raw_writew(v,a) (*(volatile unsigned short *)(a) = (v))

#ifndef CONFIG_OMAP44XX

#define 	MUX_VAL(OFFSET,VALUE)\
__raw_writew((VALUE), OMAP34XX_CTRL_BASE + (OFFSET));

#define		CP(x)	(CONTROL_PADCONF_##x)

#endif

void testfn(void)
{ // code can be copied to DRAM (assuming that it is position-independent!)
	int i;
#ifndef CONFIG_OMAP44XX
	for(i=0; i<10; i++) {
		if(i & 1)
			MUX_VAL(CP(GPMC_nCS6),      (IEN | PTD | EN  | M4)) /*GPT_PWM11/GPIO57*/
		else
			MUX_VAL(CP(GPMC_nCS6),      (IEN | PTU | EN  | M4)) /*GPT_PWM11/GPIO57*/
		udelay(500*1000);
	}
#endif
}

int lowlevel_monitor (void)
{ // add testing code here
	int i=0;
	printf("GTA04: Hello World!\n");
	while(1) {
		char *c;
		int argc=0;
		int pos=0;
		printf ("$ ");
		while(1) {
			line[pos]=getc();
			switch(line[pos]) {
				case '\b':
				case 0x7f:
					if(pos > 0) {
						putc('\b');
						pos--;
					}
					continue;
				case 0:
				case '\n':
					continue;	// ignore
				case '\r':
					break;	// command
				default:
					if(pos < sizeof(line)/sizeof(line[0])) {
						putc(line[pos]);
						pos++;
					}
					continue;
			}
			line[pos]=0;
			printf("\n");
			break;
		}
		c=line;
		while(argc < sizeof(argv)/sizeof(argv[0])) {
			while (*c == ' ' || *c == '\t')
				c++;
			if(*c == 0)
				break;
			argv[argc]=c;
			while(*c != 0 && *c != '\t' && *c != ' ')
				c++;
			argc++;
			if(*c == 0)
				break;
			*c=0;	// substitute
			c++;
		}
		if(argc == 0)
			continue;	// empty line
//		printf("argc=%d argv[0]=%s\n", argc, argv[0]);
		if(strcmp(argv[0], "c") == 0) { // c - copy test function to DRAM and execute
			extern void * memcpy(void * dest,const void *src,size_t count);
			void (*fn)(void) = &testfn;
			size_t cnt = 0x1000;
			addr=(char *) CFG_LOADADDR;
			printf("%08x (%d) --> %08x\n", fn, cnt, addr);
			memcpy(addr, fn, cnt);
			(*fn)();
		}
		else if(strcmp(argv[0], "x") == 0) { // x [addr] - execute code
			void (*fn)(void) = addr;
			if(argc > 1)
				fn = (void *) xtol(argv[1]);
			(*fn)();
		}
		else if(strcmp(argv[0], "l") == 0) { // l - loop
			int d=1;
			while(1)
				printf ("Welcome to uart-monitor (%d)\n", d++), udelay(500*1000);
		}
		else if(strcmp(argv[0], "a") == 0) { // a [addr]
			if(argc > 1)
				addr=(char *) xtol(argv[1]);
			printf("%08x\n", addr);
		}
		else if(strcmp(argv[0], "r") == 0) { // r [size]
			int i;
			int n=1;
			if(argc > 1)
				n=xtol(argv[1]);
			printf("%08x:", addr);
			for(i=0; i<n; i++) {
				printf(" %02x", *addr++);
				if(i % 16 == 15 && i != n-1)
					printf("\n%08x:", addr);
			}
			printf("\n");
		}
		else if(strcmp(argv[0], "rl") == 0) { // rl [size]
			int i;
			int n=1;
			if(argc > 1)
				n=xtol(argv[1]);
			printf("%08x:", addr);
			for(i=0; i<n; i++) {
				printf(" %08x", *(volatile unsigned int *) addr);
				addr+=4;
				if(i % 4 == 3 && i != n-1)
					printf("\n%08x:", addr);
			}
			printf("\n");
		}
		else if(strcmp(argv[0], "f") == 0) { // f [value [size]]
			int i;
			int n=1;
			int val=0x55;
			if(argc > 1)
				val=xtol(argv[1]);
			if(argc > 2)
				n=xtol(argv[2]);
			for(i=0; i<n; i++)
				addr[i]=val;
		}
		else if(strcmp(argv[0], "ram") == 0) {
			int val=0x55;
			if(argc == 2)
				val=xtol(argv[1]);
			for(addr=(char *) CFG_LOADADDR; addr < 256+(char *) CFG_LOADADDR; addr++) {
				*addr=val++;
			}
			addr=(char *) CFG_LOADADDR;
		}
		else if(strcmp(argv[0], "rt") == 0) { // rt
			unsigned char val;
			printf("filling\n");
			for(addr=(char *) CFG_LOADADDR; addr < 256*1024*1024+(char *) 0x80000000; addr++) {
				val=(unsigned int) addr + (((unsigned int) addr) >> 8) + (((unsigned int) addr) >> 16) + (((unsigned int) addr) >> 24);	// build from address
				if((((unsigned int)addr) & 0xfffff) == 0)	// approx. 1 MByte per second
					printf("%08x: %02x\n", addr, val);
				*addr=val;	// fill ram with pattern
			}
			printf("\nchecking\n");
			for(addr=(char *) CFG_LOADADDR; addr < 256*1024*1024+(char *) 0x80000000; addr++) {
				unsigned char r=*addr;		// read back
				val=(unsigned int) addr + (((unsigned int) addr) >> 8) + (((unsigned int) addr) >> 16) + (((unsigned int) addr) >> 24);
				if((((unsigned int)addr) & 0xfffff) == 0)
					printf("%08x\n", addr);
				if(r != val)	// check RAM
					printf("read error %8x: %02x (%02x)\n", addr, r, val);
			}
			printf("\ndone\n");
			addr=(char *) CFG_LOADADDR;
		}
#if 0
		else if(strcmp(argv[0], "clk") == 0) { // clk
			extern u32 osc_clk;
			printf("clk=%d\n", osc_clk);
		}
#endif
#ifndef CONFIG_OMAP44XX
		else if(strcmp(argv[0], "bl") == 0) { // blink backlight
			int i;
			for(i=0; i<10; i++) {
				printf("blink %d\n", i+1);
				// works only on GTA04A3 board since R209 is high compared to PTU
				if(i & 1)
					MUX_VAL(CP(GPMC_nCS6),      (IEN | PTD | EN  | M4)) /*GPT_PWM11/GPIO57*/
				else
					MUX_VAL(CP(GPMC_nCS6),      (IEN | PTU | EN  | M4)) /*GPT_PWM11/GPIO57*/
				udelay(500*1000);
			}
		}
#endif
#ifndef CONFIG_OMAP44XX
		else if(strcmp(argv[0], "g") == 0) { // g - read GPIO
			unsigned int i;
			int n=32*sizeof(blocks)/sizeof(blocks[0]);
			printf("GPIO1 data: %08x\n", ((gpio_t *) OMAP34XX_GPIO1_BASE) -> datain);
			printf("%03d:", 0);
			for(i=0; i<n; i++) {
				printf(" %c%d", (gpio_is_input(i)?' ':'o'), gpio_get(i));
				if(i % 10 == 9 && i != n-1)
					printf("\n%03d:", i);
			}
			printf("\n");
			
		}
#endif
#ifndef CONFIG_OMAP44XX
		else if(strcmp(argv[0], "m") == 0) { // m - read pinmux
			int cols=0;
			MUX_VAL(CP(SYS_BOOT5),      (IEN  | PTD | DIS | M4)) /*GPIO_7 */
			addr=0x48002030;
			printf("%08x", addr);					
			while(addr <= 0x480025F8) {
				unsigned mux=*(unsigned *) addr;
				int i;
				for(i=1; i <= 2; i++) {
					printf(" %c%d%c", (mux&8)?((mux&0x10?'U':'D')):' ', (mux&7), (mux&0x100)?'I':'O');
					mux >>= 16;
				}
				if(addr == 0x48002264) {
					addr=0x480025DC;
					printf("\n%08x", addr);
					cols=0;
				}
				else {
					addr+=4;
					if(++cols == 8)
						printf("\n%08x", addr), cols=0;
				}
			}
			printf("\n");
		}
#endif
		else if(strcmp(argv[0], "k") == 0) { // k - back to kermit mode
			printf("Going back to Kermit mode\n");
			break;
		}
		else
			{
			printf("uart-monitor unknown command: %s\n", argv[0]);
			printf("a [addr]: set address\n");
#ifndef CONFIG_OMAP44XX
			printf("bl: blink backlight\n");
#endif
			printf("c: copy and run test from DRAM\n");
			printf("f: fill RAM with pattern\n");
#ifndef CONFIG_OMAP44XX
			printf("g: read GPIOs\n");
#endif
			printf("k: back to Kermit mode\n");
			printf("l: loop hello message\n");
#ifndef CONFIG_OMAP44XX
			printf("m: read Pinmux\n");
#endif
			printf("r: \n");
			printf("ram: \n");
			printf("rl [size]: RAM listing\n");
			printf("rt: RAM test\n");
			printf("x: \n");
			}
	}
	return 0;
}
