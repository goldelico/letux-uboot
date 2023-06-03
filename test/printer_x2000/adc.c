
#include <common.h>
#include <command.h>
#include <config.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/jz_uart.h>

#include <asm/arch-x2000_v12/cpm.h>
#include "adc.h"

static inline int read_channel_data(int ch)
{
	unsigned int val;
	val = read_reg(SADC_ADATA0 + (ch/2)*4);
	val = (val >> ((ch%2)*16))&0xffff; /* 10bit data */
	return val;
}

int read_adc_channel(int ch)
{
	unsigned int val;

	write_reg((1<<ch), SADC_ADSTATE); /* clear ready */
	/* Set ADENA.AUXn(0-2) to 1 to enable the channel. */
	val = read_reg(SADC_ADENA);
	val |= (1<<ch);
	write_reg(val, SADC_ADENA);

	/* When ADSTATE.VRDYn(0-2) = 1, you can read the sample data from corresponding
	 * ADVDATn(0-2). And the AUXEN will be set to 0 auto.
	 */
	do {
		val = read_reg(SADC_ADSTATE);
		if (val & (1<<ch)) { /* data ready */
			val = read_channel_data(ch);
			return val;
		}
	} while(1);
	//return val;
}

/* adc voltage in mV */
int get_adc_channel_voltage(int ch)
{
	unsigned int val;
	val = read_adc_channel(ch);
	val = (val*ADC_REF_VOL)/1024;
	return val;
}

/* void adc_init(void) called by printer_init.c
 */
void adc_init(void)
{
	unsigned int val, tmp;
	/* adc clock gate */
	val = readl(CPM_BASE + CPM_CLKGR0);
	serial_puts("CPM_CLKGR0: "); serial_put_hex(val);
	val &= ~((1 << 13));
	serial_puts("CPM_CLKGR0: "); serial_put_hex(val);
	*(volatile unsigned int *) (CPM_BASE + CPM_CLKGR0) = val;
	//writel(val,CPM_BASE + CPM_CLKGR0);
	val = readl(CPM_BASE + CPM_CLKGR0);
	serial_puts("CPM_CLKGR0: "); serial_put_hex(val);
	serial_puts("CPM_BASE: "); serial_put_hex(CPM_BASE);

	/* power on adc */
	val = read_reg(SADC_ADENA);
	val &= ~(1<<15);
	write_reg(val, SADC_ADENA);
	//mdelay(2);		// at least 2ms

	/* set adc clock */
#if 0
#define ADC_CLK_REQUIRE (100*1024) /* 20KHz~200KHz */
#define DEV_CLK         (CONFIG_SYS_EXTAL) /* 24MHz */
	val = DEV_CLK/ADC_CLK_REQUIRE;
	val --;
	if (val>0xff) val=0xff;
#endif
	val = read_reg(SADC_ADCLK);
	serial_puts("SADC_ADCLK: "); serial_put_hex(val);
	val = 0x100;
	write_reg(val, SADC_ADSTB);

	serial_puts("\n");
	return;	
}
