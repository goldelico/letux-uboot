#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>
#include <generated/pll_reg_values.h>

#define CONFIG_PLL_STABILIZE   0x20

static void pll_set(unsigned int reg)
{
	unsigned int val;
	unsigned int timeout = 0;

	val = cpm_inl(reg);
	val &= ~(1 << 0);
	cpm_outl(val,reg);

	switch(reg) {
	case CPM_CPAPCR:
		val = CONFIG_SYS_APLL_MNOD | (APLL_EN_VALUE << 0);
		cpm_outl(val,CPM_CPAPCR);
		break;
	case CPM_CPMPCR:
		val = CONFIG_SYS_MPLL_MNOD | (MPLL_EN_VALUE << 0);
		cpm_outl(val,CPM_CPMPCR);
		break;
	case CPM_CPVPCR:
		val = CONFIG_SYS_VPLL_MNOD | (VPLL_EN_VALUE << 0);
		cpm_outl(val,CPM_CPVPCR);
		break;
	default:
		serial_debug("pll reg[0x%x] not recognise!\n",reg, val);
	}
	timeout = 0x10000;
	while((!(cpm_inl(reg) & (1 << 3))) && --timeout);
	if(timeout == 0) {
		serial_debug("pll reg[0x%x] val[0x%x] setting timeout!\n",reg, val);
	}
}
static void pll_sets(void)
{
	if(APLL_EN_VALUE) {
		pll_set(CPM_CPAPCR);
		serial_debug("CPA_CPAPCR:%x\n",cpm_inl(CPM_CPAPCR));
	}
	if(MPLL_EN_VALUE){
		pll_set(CPM_CPMPCR);
		serial_debug("CPM_CPMPCR:%x\n",cpm_inl(CPM_CPMPCR));
	}
	if(VPLL_EN_VALUE){
		pll_set(CPM_CPVPCR);
		serial_debug("CPM_CPVPCR:%x\n",cpm_inl(CPM_CPVPCR));
	}
}
static void cpccr_default(void)
{
	cpm_outl(0x55700000,CPM_CPCCR);
	while((cpm_inl(CPM_CPCSR) & 0xf0000007) != 0xf0000000);
}
static void cpccr_sets(void)
{
	unsigned int val;
	val = 0x55700000 |
		(CDIV_REG_VALUE  <<  0) |
		(L2DIV_REG_VALUE <<  4) |
		(H0DIV_REG_VALUE <<  8) |
		(H2DIV_REG_VALUE << 12) |
		(PDIV_REG_VALUE <<  16);
	cpm_outl(val,CPM_CPCCR);
	while(cpm_inl(CPM_CPCSR) & 7);
	val = (SEL_SRC_REG_VALUE   << 30) |
		(SEL_CPLL_REG_VALUE  << 28) |
		(SEL_H0PLL_REG_VALUE << 26) |
		(SEL_H2PLL_REG_VALUE << 24) |
		(CDIV_REG_VALUE  <<  0) |
		(L2DIV_REG_VALUE <<  4) |
		(H0DIV_REG_VALUE <<  8) |
		(H2DIV_REG_VALUE << 12) |
		(PDIV_REG_VALUE <<  16);
	cpm_outl(val,CPM_CPCCR);
	while((cpm_inl(CPM_CPCSR) & 0xf0000000) != 0xf0000000);
	serial_debug("CPM_CPCCR:%x\n",cpm_inl(CPM_CPCCR));
}
int pll_init(void)
{
	cpccr_default();
	pll_sets();
	cpccr_sets();
}
