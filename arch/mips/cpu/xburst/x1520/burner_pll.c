/*
 * X1520 burner pll configuration
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: cli <cli@ingenic.cn>
 *
 * Note: Burner's firmware is too big , for reduce code.
 *	 Burner fix cpu freq and ddr freq code.
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

/* #define DEBUG */
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

#define CPCCR_CFG(a,b,c,d,e,f,g,h,i)	\
	(((a & 3) << 30)                    \
	 | ((b & 3) << 28)                  \
	 | ((c & 3) << 26)                  \
	 | ((d & 3) << 24)                  \
	 | (((e - 1) & 0xf) << 16)          \
	 | (((f - 1) & 0xf) << 12)          \
	 | (((g - 1) & 0xf) << 8)           \
	 | (((h - 1) & 0xf) << 4)           \
	 | (((i - 1) & 0xf) << 0))


static unsigned int nfro(unsigned int fin, unsigned int fout)
{
	unsigned mnod = 0;
	unsigned nr = 0,nf = 0,od1 =7 ,od0;
	fout = fout / 1000000;
	fin = fin / 1000000;

	/*Align to extal clk*/
	if (fout%fin  >= fin/2) {
		fout += (fin - fout%fin);
	} else {
	    fout -= fout%fin;
	}

	/*caculate nf*/
	do {
		nr++;
		nf = (fout * nr) / fin;
	} while ((nf * fin != nr * fout || nf >= 4096) && nr < 63);

	/*caculate od1*/
	while ((nr % od1) && od1 > 1) {
		od1--;
	}
	nr = nr / od1;

	/*caculate od0*/
	od0 = od1;
	while((nr % od0) && od0 > 1) {
		od0--;
	}
	nr = nr / od0;

	if(fout < 800000000) {
		nf *= 2;
		od1 *= 2;
	}

	printf("nf=%d,nr=%d,od1=%d,od0=%d,mpll=%d\n",nf,nr,od1,od0,fout);
	mnod = (nf << 20) | (nr << 14) | (od1 << 11) | (od0 << 8);
	return mnod;
}

void pll_init(void)
{
	unsigned int pll_value;
	unsigned int tmp;
	unsigned int cpccr = 0;
	unsigned int sel = 0x1; /* cpu sel apll default */
	unsigned int sel_a = 0x1; /* sclk_a use exclk default */

	tmp = gd->arch.gi->cpufreq/gd->arch.gi->ddrfreq;
	if (tmp < 1)
		gd->arch.gi->ddr_div = tmp = 1;
	gd->arch.gi->cpufreq = gd->arch.gi->ddrfreq * tmp;
	gd->arch.gi->ddr_div = tmp;


	printf("extal=%d, cpu=%d\n",gd->arch.gi->extal,gd->arch.gi->cpufreq);
	pll_value = nfro(gd->arch.gi->extal, gd->arch.gi->cpufreq);

	if (cpm_inl(CPM_CPAPCR) & 0x1) {
		sel_a = 0x2;
		sel = 0x2;
	}

	if (sel == 0x1) {
		sel_a = 0x2;
		cpm_outl(pll_value | (0x1 << 0), CPM_CPAPCR);
		while(!(cpm_inl(CPM_CPAPCR) & (0x1 << 3)));
	} else {
		cpm_outl(pll_value | (0x1 << 0), CPM_CPMPCR);
		while(!(cpm_inl(CPM_CPMPCR) & (0x1 << 3)));
	}

	if(gd->arch.gi->ddrfreq >= 450000000) {
		cpccr =	CPCCR_CFG(sel_a,sel,sel,sel, 10,5,5,2,1) | (7 << 20);
	} else {
		cpccr = CPCCR_CFG(sel_a,sel,sel,sel, 12,6,6,2,1) | (7 << 20);
	}

	tmp = (cpm_inl(CPM_CPCCR) & (0xff << 24)) | (cpccr & ~(0xff << 24)) | (7 << 20);
	cpm_outl(tmp, CPM_CPCCR);
	while(cpm_inl(CPM_CPCSR) & 0x7);

	tmp = (cpm_inl(CPM_CPCCR) & ~(0xff << 24)) | (cpccr & (0xff << 24));
	cpm_outl(tmp ,CPM_CPCCR);
	while(!(cpm_inl(CPM_CPCSR) & (0x1f << 27)));

	printf("cpapcr %x\n", cpm_inl(CPM_CPAPCR));
	printf("cpmpcr %x\n", cpm_inl(CPM_CPMPCR));
	printf("cpccr:%x\n", cpm_inl(CPM_CPCCR));
}
