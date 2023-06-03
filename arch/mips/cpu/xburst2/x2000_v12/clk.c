/*
 * x2000 clock common interface
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 * Based on: newxboot/modules/clk/jz4775_clk.c
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
/*#define DEBUG*/
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>
#include <generated/clk_reg_values.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef DUMP_CGU_SELECT
static char clk_name[][10] = {
	[DDR] = {"ddr"},
	[MACPHY] = {"macphy"},
	[MACTX0] = {"mactx0"},
	[MACTX1] = {"mactx1"},
	[MACPTP] = {"macptp"},
	[I2S0] = {"I2S0"},
	[I2S1] = {"I2S1"},
	[I2S2] = {"I2S2"},
	[I2S3] = {"I2S3"},
	[LCD] = {"lcd"},
	[MSC0] = {"msc0"},
	[MSC1] = {"msc1"},
	[MSC2] = {"msc2"},
	[SFC] = {"sfc"},
	[SSI] = {"ssi"},
	[CIM] = {"cim"},
	[PWM] = {"pwm"},
	[ISP] = {"isp"},
	[RSA] = {"rsa"},

};

static char * cgu_name(int clk) {
	return clk_name[clk];
}
#endif

struct clk_cgu_setting cgusetting[19] = CGU_REG_VALUE;

void clk_prepare(void)
{
	/*stop clk and set div max*/
	int i;
	unsigned regval = 0, reg = 0;
	unsigned int size = ARRAY_SIZE(cgusetting);
	unsigned int timeout = 0xfff;

	/*设置时钟到最大分频，防止PLL升频后，各外设时钟过高，工作不正常.*/

	for (i = 0; i < size; i++) {

		/* MSC的时钟使能需要在MSC控制器中设置相关bit，此处跳过.*/
		if((i == MSC0) || (i == MSC1) || (i == MSC2))
			continue;

		if((i == CIM)) {
			cpm_outl(cpm_inl(CPM_OPCR) & ~(1 << 5), CPM_OPCR);
		}
		reg = cgusetting[i].addr;
		regval = readl(reg);
		if(cgusetting[i].busy) {
			/*set div max*/
			regval |= cgusetting[i].val;
			writel(regval, reg);

			timeout = 0xfff;
			while (readl(reg) & (1 << cgusetting[i].busy) && --timeout);
			if(!timeout) {
				printf("wait clk %d timeout\n", i);
				continue;
			}
		} else {
			regval &= ~(1 << cgusetting[i].ce);
			writel(regval, reg);
		}
#ifdef DUMP_CGU_SELECT
		printf("%s(0x%x) :0x%x\n",clk_name[i] ,reg,  readl(reg));
#endif
	}
}

static inline void cgu_clks_set(void)
{
	int i;
	unsigned regval = 0, reg = 0;
	unsigned int size = ARRAY_SIZE(cgusetting);

	for (i = 0; i < size; i++) {
		reg = cgusetting[i].addr;
		regval = readl(reg);
		regval &= ~(3 << 30);
		regval |= cgusetting[i].sel_val;
		writel(regval, reg);
	}
}

static unsigned int pll_get_rate(int pll)
{
	unsigned int cpxpcr = 0;
	unsigned int m, n, od;
	unsigned int rate;

	switch (pll) {
	case APLL:
		cpxpcr = cpm_inl(CPM_CPAPCR);
		break;
	case MPLL:
		cpxpcr = cpm_inl(CPM_CPMPCR);
		break;
	case EPLL:
		cpxpcr = cpm_inl(CPM_CPEPCR);
		break;
	default:
		return 0;
	}
	m = ((cpxpcr >> 20) & 0xfff) + 1;
	n = ((cpxpcr >> 14) & 0x3f) + 1;
	od = (cpxpcr >> 11) & 0x7;
	od = 1 << od;
	rate = CONFIG_SYS_EXTAL / 1000 / 1000;
	rate = rate * m  * 2/ n / od;
	return (rate * 1000 * 1000);
}

static unsigned int get_cclk_rate(void)
{
	unsigned int cpccr  = cpm_inl(CPM_CPCCR);

	switch ((cpccr >> 28) & 3) {
	case 1:
		return pll_get_rate(APLL) / ((cpccr & 0xf) + 1);
	case 2:
		return pll_get_rate(MPLL) / ((cpccr & 0xf) + 1);
	}
	return 0;
}

static unsigned int get_cgu_rate(unsigned int clk_id)
{
	struct clk_cgu_setting *cgu = NULL;
	unsigned int pll_rate = 0;
	unsigned regval = 0, reg = 0;
	unsigned int ret = 0;

	cgu = &cgusetting[clk_id];
	reg = cgu->addr;
	regval = readl(reg);

	if (clk_id == DDR) {
		switch (regval >> 30) {
		case 1:
			pll_rate = pll_get_rate(APLL);
			break;
		case 2:
			pll_rate = pll_get_rate(MPLL);
			break;
		default:
			printf("DDR clk src err\n");
			break;
		}
	}

	switch (regval >> 30) {
	case 0:
		pll_rate = pll_get_rate(APLL);
		break;
	case 1:
		pll_rate = pll_get_rate(MPLL);
		break;
	case 2:
		if((clk_id == MSC0) || (clk_id == MSC1) || (clk_id == MSC2))
			pll_rate = CONFIG_SYS_EXTAL;
		else
			pll_rate = pll_get_rate(EPLL);
		break;
	default:
		printf("clk src err\n");
		break;
	}

	if((clk_id == MSC0) || (clk_id == MSC1) || (clk_id == MSC2))
		ret = pll_rate / (((regval & 0xff) + 1) * 4);
	else
		ret = pll_rate / ((regval & 0xff) + 1);
	return ret;
}

unsigned int cpm_get_h2clk(void)
{
	int h2clk_div;
	unsigned int cpccr  = cpm_inl(CPM_CPCCR);

	h2clk_div = (cpccr >> 12) & 0xf;

	switch ((cpccr >> 24) & 3) {
		case 1:
			return pll_get_rate(APLL) / (h2clk_div + 1);
		case 2:
			return pll_get_rate(MPLL) / (h2clk_div + 1);
	}
	return 0;
}

unsigned int clk_get_rate(int clk)
{
	switch (clk) {
	case APLL:
		return pll_get_rate(APLL);
	case MPLL:
		return pll_get_rate(MPLL);
	case CPU:
		return get_cclk_rate();
	case H2CLK:
		return cpm_get_h2clk();
	case 0 ... (CGU_CNT-1):
		return get_cgu_rate(clk);
	}

	return 0;
}

void clk_set_rate(int clk_id, unsigned long rate)
{
#ifndef CONFIG_X2000_FPGA
	unsigned int cdr;
	unsigned int pll_rate;
	struct clk_cgu_setting *cgu = NULL;
	unsigned regval = 0, reg = 0;
	unsigned int ratio;

	if(clk_id >= CGU_CNT) {
		/* printf("set clk id error\n"); */
		return;
	}

	cgu = &cgusetting[clk_id];
	reg = cgu->addr;
	pll_rate = pll_get_rate(cgu->sel_src);

	if(!pll_rate) {
		debug("clk id %d: get pll error\n", clk_id);
		return;
	}

	regval = readl(reg);

	if((clk_id == MSC0) || (clk_id == MSC1) || (clk_id == MSC2)) {
		if(rate > (CONFIG_SYS_EXTAL/4))
			regval = (regval & (~MSCCDR_MPCS_MASK)) | cgusetting[clk_id].sel_val;
		else {
			unsigned int tmp;
			tmp = cpm_inl(CPM_MSC0CDR) | MSCCDR_EXCK_E;
			cpm_outl(tmp, CPM_MSC0CDR);

			regval = readl(reg);
			regval = (regval & (~MSCCDR_MPCS_MASK)) | MSCCDR_MPCS_EXCLK;
			pll_rate = CONFIG_SYS_EXTAL;
		}

		ratio = (pll_rate + rate - 1)/rate;
		cdr = ((ratio % 4) ? ratio/4 : (ratio/4 - 1)) & 0xff;
	} else
		cdr = ((pll_rate + rate - 1)/rate - 1 ) & 0xff;
	/* debug("pll_rate = %d, rate = %d, cdr = %d\n",pll_rate,rate,cdr); */

	regval &= ~(3 << cgu->stop | 0xff);
	regval |= ((1 << cgu->ce) | cdr);
	writel(regval, reg);
	while (readl(reg) & (1 << cgu->busy))
		;
#ifdef DUMP_CGU_SELECT
	printf("%s(0x%x) :0x%x\n",clk_name[clk_id] ,reg,  readl(reg));
#endif
#endif
	return;
}
void clk_init(void)
{
	unsigned int reg_clkgr = cpm_inl(CPM_CLKGR0);
	unsigned int gate = CPM_CLKGR_DDR
#ifdef CONFIG_JZ_MMC_MSC0
		| CPM_CLKGR_MSC0
#endif
#ifdef CONFIG_JZ_MMC_MSC1
		| CPM_CLKGR_MSC1
#endif
#ifdef CONFIG_JZ_LCD_V14
		| CPM_CLKGR_LCD
#endif
#ifdef CONFIG_JZ_SFC
		| CPM_CLKGR_SFC
#endif
#ifdef CONFIG_JZ_SPI0
		| CPM_CLKGR_SSI0
#endif
#ifdef CONFIG_JZ_SPI1
		| CPM_CLKGR_SSI1
#endif
#ifdef CONFIG_JZ_EFUSE
		| CPM_CLKGR_EFUSE
#endif
#ifdef CONFIG_JZ_SCBOOT
		| CPM_CLKGR_RSA
		| CPM_CLKGR_AES
		| CPM_CLKGR_PDMA
		| CPM_CLKGR_EFUSE
		| CPM_CLKGR_DTRNG
#endif
		;
	reg_clkgr &=  ~gate;
	cpm_outl(reg_clkgr,CPM_CLKGR0);

	reg_clkgr = cpm_inl(CPM_CLKGR1);
	gate = 0
#ifdef CONFIG_GMAC0
		| CPM_CLKGR_GMAC0
#endif
#ifdef CONFIG_GMAC1
		| CPM_CLKGR_GMAC1
#endif
#ifdef CONFIG_JZ_MMC_MSC2
		| CPM_CLKGR_MSC2
#endif
#ifdef CONFIG_JZ_SCBOOT
		| CPM_CLKGR_HASH
#endif
#ifdef CONFIG_JZ_MIPI_DSI
		| CPM_CLKGR_MIPI_DSI
#endif
		;
	reg_clkgr &=  ~gate;
	cpm_outl(reg_clkgr,CPM_CLKGR1);

	cgu_clks_set();
}

void enable_uart_clk(void)
{
	unsigned int clkgr0 = cpm_inl(CPM_CLKGR0);
	unsigned int clkgr1 = cpm_inl(CPM_CLKGR1);

	switch (gd->arch.gi->uart_idx) {
#define _CASE0(U, N) case U: clkgr0 &= ~N; break
#define _CASE1(U, N) case U: clkgr1 &= ~N; break
		_CASE0(0, CPM_CLKGR_UART0);
		_CASE0(1, CPM_CLKGR_UART1);
		_CASE0(2, CPM_CLKGR_UART2);
		_CASE1(3, CPM_CLKGR_UART3);
		_CASE1(4, CPM_CLKGR_UART4);
		_CASE1(5, CPM_CLKGR_UART5);
		_CASE1(6, CPM_CLKGR_UART6);
		_CASE1(7, CPM_CLKGR_UART7);
		_CASE1(8, CPM_CLKGR_UART8);
		_CASE1(9, CPM_CLKGR_UART9);
	default:
		break;
	}
	cpm_outl(clkgr0, CPM_CLKGR0);
	cpm_outl(clkgr1, CPM_CLKGR1);
}
