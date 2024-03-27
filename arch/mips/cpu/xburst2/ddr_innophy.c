/*
 * DDR driver for Synopsys DWC DDR PHY.
 * Used by Jz4775, JZ4780...
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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
/* #define DEBUG_READ_WRITE */
#include <config.h>
#include <common.h>
#include <ddr/ddr_common.h>
#ifndef CONFIG_BURNER
#include <generated/ddr_reg_values.h>
#endif

#include <asm/io.h>
#include <asm/arch/clk.h>

/*#define CONFIG_DWC_DEBUG 0*/
#define ddr_hang() do{						\
		debug("%s %d\n",__FUNCTION__,__LINE__);	\
		hang();						\
	}while(0)

DECLARE_GLOBAL_DATA_PTR;
struct ddr_reg_value *global_reg_value __attribute__ ((section(".data")));


extern void ddrp_auto_calibration(void);
extern void ddrp_cfg(struct ddr_reg_value *global_reg_value);
extern void ddrp_pll_init(void);
#ifdef CONFIG_DDRP_SOFTWARE_TRAINING
extern void ddrp_software_calibration(void);
#endif


#ifdef  CONFIG_DWC_DEBUG
#define FUNC_ENTER() debug("%s enter.\n",__FUNCTION__);
#define FUNC_EXIT() debug("%s exit.\n",__FUNCTION__);

static void dump_ddrc_register(void)
{
	debug("DDRC_STATUS         0x%x\n", ddr_readl(DDRC_STATUS));
	debug("DDRC_CFG            0x%x\n", ddr_readl(DDRC_CFG));
	debug("DDRC_CTRL           0x%x\n", ddr_readl(DDRC_CTRL));
	debug("DDRC_LMR            0x%x\n", ddr_readl(DDRC_LMR));
	debug("DDRC_DLP            0x%x\n", ddr_readl(DDRC_DLP));
	debug("DDRC_TIMING1        0x%x\n", ddr_readl(DDRC_TIMING(1)));
	debug("DDRC_TIMING2        0x%x\n", ddr_readl(DDRC_TIMING(2)));
	debug("DDRC_TIMING3        0x%x\n", ddr_readl(DDRC_TIMING(3)));
	debug("DDRC_TIMING4        0x%x\n", ddr_readl(DDRC_TIMING(4)));
	debug("DDRC_TIMING5        0x%x\n", ddr_readl(DDRC_TIMING(5)));
	debug("DDRC_REFCNT         0x%x\n", ddr_readl(DDRC_REFCNT));
	debug("DDRC_AUTOSR_CNT     0x%x\n", ddr_readl(DDRC_AUTOSR_CNT));
	debug("DDRC_AUTOSR_EN      0x%x\n", ddr_readl(DDRC_AUTOSR_EN));
	debug("DDRC_MMAP0          0x%x\n", ddr_readl(DDRC_MMAP0));
	debug("DDRC_MMAP1          0x%x\n", ddr_readl(DDRC_MMAP1));
	debug("DDRC_REMAP1         0x%x\n", ddr_readl(DDRC_REMAP(1)));
	debug("DDRC_REMAP2         0x%x\n", ddr_readl(DDRC_REMAP(2)));
	debug("DDRC_REMAP3         0x%x\n", ddr_readl(DDRC_REMAP(3)));
	debug("DDRC_REMAP4         0x%x\n", ddr_readl(DDRC_REMAP(4)));
	debug("DDRC_REMAP5         0x%x\n", ddr_readl(DDRC_REMAP(5)));
	debug("DDRC_DWCFG          0x%x\n", ddr_readl(DDRC_DWCFG));
	debug("DDRC_HREGPRO        0x%x\n", ddr_readl(DDRC_HREGPRO));
	debug("DDRC_PREGPRO        0x%x\n", ddr_readl(DDRC_PREGPRO));
	debug("DDRC_CGUC0          0x%x\n", ddr_readl(DDRC_CGUC0));
	debug("DDRC_CGUC1          0x%x\n", ddr_readl(DDRC_CGUC1));

}


#else
#define FUNC_ENTER()
#define FUNC_EXIT()

#define dump_ddrc_register()
#endif

#define DDR_CHOOSE_PARAMS	0
#ifdef DDR_CHOOSE_PARAMS
int atoi(char *pstr)
{
	int value = 0;
	int sign = 1;
	int radix;

	if(*pstr == '-'){
		sign = -1;
		pstr++;
	}
	if(*pstr == '0' && (*(pstr+1) == 'x' || *(pstr+1) == 'X')){
		radix = 16;
		pstr += 2;
	}
	else
		radix = 10;
	while(*pstr){
		if(radix == 16){
			if(*pstr >= '0' && *pstr <= '9')
				value = value * radix + *pstr - '0';
			else if(*pstr >= 'A' && *pstr <= 'F')
				value = value * radix + *pstr - 'A' + 10;
			else if(*pstr >= 'a' && *pstr <= 'f')
				value = value * radix + *pstr - 'a' + 10;
		}
		else
			value = value * radix + *pstr - '0';
		pstr++;
	}
	return sign*value;
}


int choose_params(int m)
{
	char buf[16];
	char ch;
	char *p = buf;
	int select_m;

	debug("Please select from [0 to %d]\n", m);

	debug(">>  ");

	while((ch = getc()) != '\r') {
		putc(ch);
		*p++ = ch;

		if((p - buf) > 16)
			break;
	}
	*p = '\0';

	debug("\n");

	select_m = atoi(buf);
	debug("slected: %d\n", select_m);

	return select_m;
}
#endif


static void mem_remap(void)
{
	int i;
	unsigned int *remap;

	remap = global_reg_value->REMMAP_ARRAY;


	for(i = 0;i < ARRAY_SIZE(global_reg_value->REMMAP_ARRAY);i++)
	{
		ddr_writel(remap[i], DDRC_REMAP(i+1));
	}
}

static enum ddr_type get_ddr_type(void)
{
	int type;

	type = global_reg_value->h.type;
	switch(global_reg_value->h.type){

		case DDR3:
			printf("DDR: %s type is : DDR3\n", global_reg_value->h.name);
			break;
		case LPDDR:
			printf("DDR: %s type is : LPDDR\n", global_reg_value->h.name);
			break;
		case LPDDR2:
			printf("DDR: %s type is : LPDDR2\n", global_reg_value->h.name);
			break;
		case LPDDR3:
			printf("DDR: %s type is : LPDDR3\n", global_reg_value->h.name);
			break;
		case DDR2:
			printf("DDR: %s type is : DDR2\n", global_reg_value->h.name);
			break;
		default:
			type = UNKOWN;
			printf("unsupport ddr type!\n");
			ddr_hang();
	}

	return type;
}
#if 0
static void ddrc_reset_phy(void)
{
	FUNC_ENTER();
	ddr_writel(0xf << 20, DDRC_CTRL);
	mdelay(1);
	ddr_writel(0x8 << 20, DDRC_CTRL);  //dfi_reset_n low for innophy
	mdelay(1);
	FUNC_EXIT();
}
#endif

static struct jzsoc_ddr_hook *ddr_hook = NULL;
void register_ddr_hook(struct jzsoc_ddr_hook * hook)
{
	ddr_hook = hook;
}


void ddrc_dfi_init(enum ddr_type type)
{
	unsigned int reg_val;
	FUNC_ENTER();

	reg_val = ddr_readl(DDRC_DWCFG);
	reg_val &= ~(1 << 3);
	ddr_writel(reg_val, DDRC_DWCFG); // set dfi_init_start low, and buswidth 16bit
	while(!(ddr_readl(DDRC_DWSTATUS) & DDRC_DWSTATUS_DFI_INIT_COMP)); //polling dfi_init_complete

	reg_val = ddr_readl(DDRC_CTRL);
	reg_val |= (1 << 23);
	ddr_writel(reg_val, DDRC_CTRL); //set dfi_reset_n high

	ddr_writel(global_reg_value->DDRC_CFG_VALUE, DDRC_CFG);
	ddr_writel(DDRC_CTRL_CKE, DDRC_CTRL); // set CKE to high

//	type = LPDDR3;

	ddr_writel(7 << 9 | 1 << 0, DDRC_LMR); //Send All bank precharge.
	mdelay(1);

	switch(type) {
	case LPDDR2:
#define DDRC_LMR_MR(n)                                                          \
                global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |		\
		((global_reg_value->DDR_MR##n##_VALUE & 0xff) << 24)  |                           \
		(((global_reg_value->DDR_MR##n##_VALUE >> 8) & 0xff) << (16))
		ddr_writel(DDRC_LMR_MR(63), DDRC_LMR); //set MRS reset
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(10), DDRC_LMR); //set IO calibration
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //set MR1
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //set MR2
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //set MR3
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(11), DDRC_LMR); //set MR11
		mdelay(1);

#ifdef CONFIG_LPDDR2_W97BV6MK

#define DDRC_LMR_MRW(v)                                                          \
                global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |		 \
		((v & 0xff) << 24)  |                           		 \
		(((v >> 8) & 0xff) << (16))

		/* TODO: Force disable autosr. */
		global_reg_value->DDRC_AUTOSR_EN_VALUE = 0;
		if(global_reg_value->DDRC_AUTOSR_EN_VALUE) {
			ddr_writel(DDRC_LMR_MRW(0x092a), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MRW(0x0915), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MRW(0x0900), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MRW(0x093c), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MRW(0x0900), DDRC_LMR);
			mdelay(1);

			ddr_writel(DDRC_LMR_MRW(0x092a), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MRW(0x0915), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MRW(0x0951), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MRW(0x093c), DDRC_LMR);
			mdelay(1);

			ddr_writel(DDRC_LMR_MRW(0x092a), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MRW(0x0915), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MRW(0x0917), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MRW(0x093c), DDRC_LMR);
			mdelay(1);

			ddr_writel(DDRC_LMR_MRW(0x092a), DDRC_LMR);
			mdelay(1);
			ddr_writel(DDRC_LMR_MRW(0x0915), DDRC_LMR);
			mdelay(1);

			// test mode pattern1: ok? on board1, on board x2
			ddr_writel(DDRC_LMR_MRW(0x0907), DDRC_LMR);
			mdelay(1);
			//printf("test pattern1 0x0907\n");

			// test mode pattern2
			//ddr_writel(DDRC_LMR_MRW(0x0945), DDRC_LMR);
			//mdelay(1);
			//printf("test pattern2 0x0945\n");
			// test mode pattern3
			//ddr_writel(DDRC_LMR_MRW(0x09F7), DDRC_LMR);
			//mdelay(1);
			//printf("test pattern3 0x09F7\n");

			ddr_writel(DDRC_LMR_MRW(0x093c), DDRC_LMR);
			mdelay(1);
		}
#endif
#undef DDRC_LMR_MR

		break;

	case LPDDR3:
#define DDRC_LMR_MR(n)                                                          \
                global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |		\
		((global_reg_value->DDR_MR##n##_VALUE & 0xff) << 24)  |                           \
		(((global_reg_value->DDR_MR##n##_VALUE >> 8) & 0xff) << (16))
		ddr_writel(DDRC_LMR_MR(63), DDRC_LMR); //set MRS reset
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(10), DDRC_LMR); //set IO calibration
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //set MR1
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //set MR2
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //set MR3
		mdelay(1);
		ddr_writel(DDRC_LMR_MR(11), DDRC_LMR); //set MR11
		mdelay(1);

		break;

	case DDR3:
#define DDRC_LMR_MR(n)								\
		global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |           \
			((global_reg_value->DDR_MR##n##_VALUE & 0xffff) << DDRC_LMR_DDR_ADDR_BIT) |       \
			(((global_reg_value->DDR_MR##n##_VALUE >> 16) & 0x7) << DDRC_LMR_BA_BIT)

		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //MR2
		udelay(5);
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //MR3
		udelay(5);
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //MR1
		udelay(5);
		ddr_writel(DDRC_LMR_MR(0), DDRC_LMR); //MR0
		udelay(5);
//		ddr_writel(global_reg_value->DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_ZQCL_CS0, DDRC_LMR); //ZQCL
#undef DDRC_LMR_MR
		break;

	case DDR2:
#define DDRC_LMR_MR(n)											\
		global_reg_value->DDRC_DLMR_VALUE | 1 << 1 | DDRC_LMR_START | DDRC_LMR_CMD_LMR |	\
			((global_reg_value->DDR_MR##n##_VALUE & 0x1fff) << DDRC_LMR_DDR_ADDR_BIT) |		\
			(((global_reg_value->DDR_MR##n##_VALUE >> 13) & 0x3) << DDRC_LMR_BA_BIT)

		while (ddr_readl(DDRC_LMR) & (1 << 0));
		ddr_writel(0x400003, DDRC_LMR);
		udelay(100);
		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //MR2
		udelay(5);
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //MR3
		udelay(5);
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //MR1
		udelay(5);
		ddr_writel(DDRC_LMR_MR(0), DDRC_LMR); //MR0
		udelay(5 * 1000);
		ddr_writel(0x400003, DDRC_LMR);
		udelay(100);
		ddr_writel(0x43, DDRC_LMR);
		udelay(5);
		ddr_writel(0x43, DDRC_LMR);
		udelay(5 * 1000);
#undef DDRC_LMR_MR
		break;

	default:
		ddr_hang();
	}
	FUNC_EXIT();
}

static void ddrc_prev_init(void)
{
	FUNC_ENTER();
	/* DDRC CFG init*/
	/* /\* DDRC CFG init*\/ */
	/* ddr_writel(DDRC_CFG_VALUE, DDRC_CFG); */
	/* DDRC timing init*/
	ddr_writel(global_reg_value->DDRC_TIMING1_VALUE, DDRC_TIMING(1));
	ddr_writel(global_reg_value->DDRC_TIMING2_VALUE, DDRC_TIMING(2));
	ddr_writel(global_reg_value->DDRC_TIMING3_VALUE, DDRC_TIMING(3));
	ddr_writel(global_reg_value->DDRC_TIMING4_VALUE, DDRC_TIMING(4));
	ddr_writel(global_reg_value->DDRC_TIMING5_VALUE, DDRC_TIMING(5));

	/* DDRC memory map configure*/
	ddr_writel(global_reg_value->DDRC_MMAP0_VALUE, DDRC_MMAP0);
	ddr_writel(global_reg_value->DDRC_MMAP1_VALUE, DDRC_MMAP1);

	/* ddr_writel(DDRC_CTRL_CKE, DDRC_CTRL); */
	ddr_writel(global_reg_value->DDRC_CTRL_VALUE & ~(7 << 12), DDRC_CTRL);

	FUNC_EXIT();
}

static void ddrc_post_init(void)
{
	FUNC_ENTER();

	ddr_writel(global_reg_value->DDRC_REFCNT_VALUE, DDRC_REFCNT);
	mem_remap();
	debug("DDRC_STATUS: %x\n",ddr_readl(DDRC_STATUS));
	ddr_writel(global_reg_value->DDRC_CTRL_VALUE, DDRC_CTRL);

	ddr_writel(global_reg_value->DDRC_CGUC0_VALUE, DDRC_CGUC0);
	ddr_writel(global_reg_value->DDRC_CGUC1_VALUE, DDRC_CGUC1);

	FUNC_EXIT();
}

void dump_generated_reg(struct ddr_reg_value *reg)
{
	int i;
	printf("name		      = %s\n", reg->h.name);
	printf("id		      = %x\n", reg->h.id);
	printf("type		      = %x\n", reg->h.type);
	printf("freq		      = %x\n", reg->h.freq);
	printf("DDRC_CFG_VALUE        = %x\n", reg->DDRC_CFG_VALUE);
	printf("DDRC_CTRL_VALUE       = %x\n", reg->DDRC_CTRL_VALUE);
	printf("DDRC_DLMR_VALUE       = %x\n", reg->DDRC_DLMR_VALUE);
	printf("DDRC_DDLP_VALUE       = %x\n", reg->DDRC_DDLP_VALUE);
	printf("DDRC_MMAP0_VALUE      = %x\n", reg->DDRC_MMAP0_VALUE);
	printf("DDRC_MMAP1_VALUE      = %x\n", reg->DDRC_MMAP1_VALUE);
	printf("DDRC_REFCNT_VALUE     = %x\n", reg->DDRC_REFCNT_VALUE);
	printf("DDRC_TIMING1_VALUE    = %x\n", reg->DDRC_TIMING1_VALUE);
	printf("DDRC_TIMING2_VALUE    = %x\n", reg->DDRC_TIMING2_VALUE);
	printf("DDRC_TIMING3_VALUE    = %x\n", reg->DDRC_TIMING3_VALUE);
	printf("DDRC_TIMING4_VALUE    = %x\n", reg->DDRC_TIMING4_VALUE);
	printf("DDRC_TIMING5_VALUE    = %x\n", reg->DDRC_TIMING5_VALUE);
	printf("DDRC_AUTOSR_CNT_VALUE = %x\n", reg->DDRC_AUTOSR_CNT_VALUE);
	printf("DDRC_AUTOSR_EN_VALUE  = %x\n", reg->DDRC_AUTOSR_EN_VALUE);
	printf("DDRC_HREGPRO_VALUE    = %x\n", reg->DDRC_HREGPRO_VALUE);
	printf("DDRC_PREGPRO_VALUE    = %x\n", reg->DDRC_PREGPRO_VALUE);
	printf("DDRC_CGUC0_VALUE      = %x\n", reg->DDRC_CGUC0_VALUE);
	printf("DDRC_CGUC1_VALUE      = %x\n", reg->DDRC_CGUC1_VALUE);
	printf("DDRP_MEMCFG_VALUE     = %x\n", reg->DDRP_MEMCFG_VALUE);
	printf("DDRP_CL_VALUE         = %x\n", reg->DDRP_CL_VALUE);
	printf("DDRP_CWL_VALUE        = %x\n", reg->DDRP_CWL_VALUE);
	printf("DDR_MR0_VALUE         = %x\n", reg->DDR_MR0_VALUE);
	printf("DDR_MR1_VALUE         = %x\n", reg->DDR_MR1_VALUE);
	printf("DDR_MR2_VALUE         = %x\n", reg->DDR_MR2_VALUE);
	printf("DDR_MR3_VALUE         = %x\n", reg->DDR_MR3_VALUE);
	printf("DDR_MR10_VALUE        = %x\n", reg->DDR_MR10_VALUE);
	printf("DDR_MR11_VALUE        = %x\n", reg->DDR_MR11_VALUE);
	printf("DDR_MR63_VALUE        = %x\n", reg->DDR_MR63_VALUE);
	printf("DDR_CHIP_0_SIZE       = %x\n", reg->DDR_CHIP_0_SIZE);
	printf("DDR_CHIP_1_SIZE       = %x\n", reg->DDR_CHIP_1_SIZE);
	for(i = 0; i < 5; i++) {
		printf("REMMAP_ARRAY[%d] = %x\n", i, reg->REMMAP_ARRAY[i]);
	}

}

#ifndef CONFIG_BURNER
void get_ddr_params_normal(void)
{
	int found = 0;
	int size = 0;
	int i;
	unsigned int burned_ddr_id = *(volatile unsigned int *)(CONFIG_SPL_TEXT_BASE + 128);

	if((burned_ddr_id & 0xffff) != (burned_ddr_id >> 16)) {
		printf("invalid burned ddr id\n");
	}

	burned_ddr_id &= 0xffff;

	size = ARRAY_SIZE(supported_ddr_reg_values);

	if(size == 1) {
		found = 1;
		global_reg_value = &supported_ddr_reg_values[0];
	} else {
		for(i = 0; i < ARRAY_SIZE(supported_ddr_reg_values); i++) {
			global_reg_value = &supported_ddr_reg_values[i];
			if(burned_ddr_id == global_reg_value->h.id) {
				found = 1;
				break;
			}
		}
	}
}
#else
void get_ddr_params_burner(void)
{
	/* keep ddr_reg_value inc ddr_innophy.h
	 * with ddr_registers the same
	 * */
	global_reg_value = g_ddr_param;
}
#endif

void get_ddr_params(void)
{
#ifndef CONFIG_BURNER
	get_ddr_params_normal();
#else
	get_ddr_params_burner();
#endif
	//dump_generated_reg(global_reg_value);

}

unsigned int get_ddr_size(void)
{
	return (global_reg_value->DDR_CHIP_0_SIZE) >> 20;
}

void sdram_init(void)
{
	enum ddr_type type;
	unsigned int rate;
	unsigned int reg_val;

	debug("sdram init start\n");
	soc_ddr_init();
	get_ddr_params();
	type = get_ddr_type();
	clk_set_rate(DDR, global_reg_value->h.freq);

	if(ddr_hook && ddr_hook->prev_ddr_init)
		ddr_hook->prev_ddr_init(type);
	rate = clk_get_rate(DDR);
	debug("DDR clk rate %d\n", rate);


	ddr_writel(1 << 20, DDRC_CTRL);  /* ddrc_reset_phy */

	ddrp_cfg(global_reg_value);

	reg_val = ddr_readl(DDRC_CTRL);
	reg_val &= ~ (1 << 20);
	ddr_writel(reg_val, DDRC_CTRL); /*ddrc_reset_phy clear*/

	ddrp_pll_init();

	ddrc_dfi_init(type);

	/* DDR Controller init*/
	ddrc_prev_init();

	ddr_writel(global_reg_value->DDRC_AUTOSR_CNT_VALUE, DDRC_AUTOSR_CNT);

	ddrc_post_init();
#ifdef CONFIG_DDRP_SOFTWARE_TRAINING
	ddrp_software_calibration();
#else
	ddrp_auto_calibration();
#endif

	if(ddr_hook && ddr_hook->post_ddr_init)
		ddr_hook->post_ddr_init(type);

//	get_dynamic_calib_value(rate);/*reserved*/

	if(global_reg_value->DDRC_AUTOSR_EN_VALUE) {
		/* ddr_writel(DDRC_AUTOSR_CNT_VALUE, DDRC_AUTOSR_CNT); */
		ddr_writel(1, DDRC_AUTOSR_EN);
	} else {
		ddr_writel(0, DDRC_AUTOSR_EN);
	}

	dump_ddrc_register();

	debug("DDR size is : %d MByte\n", (global_reg_value->DDR_CHIP_0_SIZE + global_reg_value->DDR_CHIP_1_SIZE) / 1024 /1024);
	/* DDRC address remap configure*/

	debug("sdram init finished\n");
}

phys_size_t initdram(int board_type)
{
	/* SDRAM size was calculated when compiling. */
#ifndef EMC_LOW_SDRAM_SPACE_SIZE
#define EMC_LOW_SDRAM_SPACE_SIZE 0x10000000 /* 256M */
#endif /* EMC_LOW_SDRAM_SPACE_SIZE */

	unsigned int ram_size;

	/*init ddr params in uboot env. */
	get_ddr_params();
	ram_size = (unsigned int)(global_reg_value->DDR_CHIP_0_SIZE) + (unsigned int)(global_reg_value->DDR_CHIP_1_SIZE);
	debug("ram_size=%x\n", ram_size);


	return ram_size;
}
