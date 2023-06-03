/*
 * SFC controller for SPI protocol, use FIFO and DMA;
 *
 * Copyright (c) 2015 Ingenic
 * Author: <xiaoyang.fu@ingenic.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <asm/arch/clk.h>
#include <asm/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/arch/base.h>
#include <asm/io.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/err.h>
#include <common.h>
#include <malloc.h>

#include <asm/arch/sfc.h>


//#define	SFC_REG_DEBUG

#define clamp(x, low, high) (min(max(low, x), high))

static void sfc_writel(struct sfc *sfc, unsigned short offset, u32 value)
{
	writel(value, SFC_BASE + offset);
}

static unsigned int sfc_readl(struct sfc *sfc, unsigned short offset)
{
	return readl(SFC_BASE + offset);
}

#ifdef SFC_REG_DEBUG
void dump_sfc_reg(struct sfc *sfc)
{
	int i = 0;

	printf("SFC_GLB0	 = %08x\n", sfc_readl(sfc, SFC_GLB));
	printf("SFC_DEV_CONF	 = %08x\n", sfc_readl(sfc, SFC_DEV_CONF));
	printf("SFC_DEV_STA_EXP	 = %08x\n", sfc_readl(sfc, SFC_DEV_STA_EXP));
	printf("SFC_DEV_STA_RT	 = %08x\n", sfc_readl(sfc, SFC_DEV_STA_RT));
	printf("SFC_DEV_STA_MASK = %08x\n", sfc_readl(sfc, SFC_DEV_STA_MSK));
	for(i = 0; i < 6; i++){
		printf("SFC_TRAN_CONF0(%d) = %08x\n", i, sfc_readl(sfc, SFC_TRAN_CONF0(i)));
	}
	printf("SFC_TRAN_LEN	 = %08x\n", sfc_readl(sfc, SFC_TRAN_LEN));
	for(i = 0; i < 6; i++){
		printf("SFC_DEV_ADDR%d		= %08x\n", i, sfc_readl(sfc, SFC_DEV_ADDR(i)));
		printf("SFC_DEV_ADDR_PLUS%d	= %08x\n", i, sfc_readl(sfc, SFC_DEV_ADDR_PLUS(i)));
	}
	printf("SFC_MEM_ADDR	= %08x\n", sfc_readl(sfc, SFC_MEM_ADDR));
	printf("SFC_TRIG	= %08x\n", sfc_readl(sfc, SFC_TRIG));
	printf("SFC_SR		= %08x\n", sfc_readl(sfc, SFC_SR));
	printf("SFC_SCR		= %08x\n", sfc_readl(sfc, SFC_SCR));
	printf("SFC_INTC	= %08x\n", sfc_readl(sfc, SFC_INTC));
	printf("SFC_FSM		= %08x\n", sfc_readl(sfc, SFC_FSM));
	printf("SFC_CGE		= %08x\n", sfc_readl(sfc, SFC_CGE));
	printf("SFC_CMD_IDX	= %08x\n", sfc_readl(sfc, SFC_CMD_IDX));
	printf("SFC_COL_ADDR	= %08x\n", sfc_readl(sfc, SFC_COL_ADDR));
	printf("SFC_ROW_ADDR	= %08x\n", sfc_readl(sfc, SFC_ROW_ADDR));
	printf("SFC_STA_ADDR0	= %08x\n", sfc_readl(sfc, SFC_STA_ADDR0));
	printf("SFC_STA_ADDR1	= %08x\n", sfc_readl(sfc, SFC_STA_ADDR1));
	printf("SFC_DES_ADDR	= %08x\n", sfc_readl(sfc, SFC_DES_ADDR));
	printf("SFC_GLB1	= %08x\n", sfc_readl(sfc, SFC_GLB1));
	printf("SFC_DEV1_STA_RT = %08x\n", sfc_readl(sfc, SFC_DEV1_STA_RT));
	for(i = 0; i < 6; i++) {
		printf("SFC_TRAN_CONF1(%d)	= %08x\n", i, sfc_readl(sfc, SFC_TRAN_CONF1(i)));
	}
	//printf("SFC_CDT	= %08x\n", sfc_readl(sfc, SFC_CDT));
	//printf("SFC_DR	= %08x\n", sfc_readl(sfc, SFC_RM_DR));
}

void dump_cdt(struct sfc *sfc)
{
	struct sfc_cdt *cdt;
	int i;

	if(sfc->cdt_addr == NULL){
		printf("%s error: sfc res not init !\n", __func__);
		return;
	}

	cdt = sfc->cdt_addr;

	for(i = 0; i < 32; i++){
		printf("\nnum------->%d\n", i);
		printf("link:%02x, ENDIAN:%02x, WORD_UINT:%02x, TRAN_MODE:%02x, ADDR_KIND:%02x\n",
				(cdt[i].link >> 31) & 0x1, (cdt[i].link >> 18) & 0x1,
				(cdt[i].link >> 16) & 0x3, (cdt[i].link >> 4) & 0xf,
				(cdt[i].link >> 0) & 0x3
				);
		printf("CLK_MODE:%02x, ADDR_WIDTH:%02x, POLL_EN:%02x, CMD_EN:%02x,PHASE_FORMAT:%02x, DMY_BITS:%02x, DATA_EN:%02x, TRAN_CMD:%04x\n",
				(cdt[i].xfer >> 29) & 0x7, (cdt[i].xfer >> 26) & 0x7,
				(cdt[i].xfer >> 25) & 0x1, (cdt[i].xfer >> 24) & 0x1,
				(cdt[i].xfer >> 23) & 0x1, (cdt[i].xfer >> 17) & 0x3f,
				(cdt[i].xfer >> 16) & 0x1, (cdt[i].xfer >> 0) & 0xffff
				);
		printf("DEV_STA_EXP:%08x\n", cdt[i].staExp);
		printf("DEV_STA_MSK:%08x\n", cdt[i].staMsk);
	}
}

static void dump_data(unsigned char *buf,size_t len)
{
	int i;
	for(i = 0;i<len;i++){
		if(!(i % 16)){
			printf("\n");
			printf("%08x:",i);
		}
		printf("%02x ",buf[i]);
	}
}
#endif

void sfc_init(struct sfc *sfc)
{
	int n;
	for(n = 0; n < N_MAX; n++) {
		sfc_writel(sfc, SFC_TRAN_CONF0(n), 0);
		sfc_writel(sfc, SFC_TRAN_CONF1(n), 0);
		sfc_writel(sfc, SFC_DEV_ADDR(n), 0);
		sfc_writel(sfc, SFC_DEV_ADDR_PLUS(n), 0);
	}

	sfc_writel(sfc, SFC_DEV_CONF, 0);
	sfc_writel(sfc, SFC_DEV_STA_EXP, 0);
	sfc_writel(sfc, SFC_DEV_STA_MSK, 0);
	sfc_writel(sfc, SFC_TRAN_LEN, 0);
	sfc_writel(sfc, SFC_MEM_ADDR, 0);
	sfc_writel(sfc, SFC_TRIG, 0);
	sfc_writel(sfc, SFC_SCR, 0);
	sfc_writel(sfc, SFC_INTC, 0);
	sfc_writel(sfc, SFC_CGE, 0);
	sfc_writel(sfc, SFC_RM_DR, 0);
}

void sfc_stop(struct sfc*sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRIG);
	tmp |= TRIG_STOP;
	sfc_writel(sfc, SFC_TRIG, tmp);
}

void sfc_start(struct sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRIG);
	tmp |= TRIG_START;
	sfc_writel(sfc, SFC_TRIG, tmp);
}

void sfc_flush_fifo(struct sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_TRIG);
	tmp |= TRIG_FLUSH;
	sfc_writel(sfc, SFC_TRIG, tmp);
}

void sfc_ce_invalid_value(struct sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp &= ~DEV_CONF_CEDL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp |= DEV_CONF_CEDL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	}
}

void sfc_hold_invalid_value(struct sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp &= ~DEV_CONF_HOLDDL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp |= DEV_CONF_HOLDDL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	}
}

void sfc_wp_invalid_value(struct sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp &= ~DEV_CONF_WPDL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp |= DEV_CONF_WPDL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	}
}

void sfc_clear_end_intc(struct sfc *sfc)
{
	int tmp = 0;
	tmp = CLR_END;
	sfc_writel(sfc, SFC_SCR, tmp);
}

void sfc_clear_treq_intc(struct sfc *sfc)
{
	int tmp = 0;
	tmp = CLR_TREQ;
	sfc_writel(sfc, SFC_SCR, tmp);
}

void sfc_clear_rreq_intc(struct sfc *sfc)
{
	int tmp = 0;
	tmp = CLR_RREQ;
	sfc_writel(sfc, SFC_SCR, tmp);
}

void sfc_clear_over_intc(struct sfc *sfc)
{
	int tmp = 0;
	tmp = CLR_OVER;
	sfc_writel(sfc, SFC_SCR, tmp);
}

void sfc_clear_under_intc(struct sfc *sfc)
{
	int tmp = 0;
	tmp = CLR_UNDER;
	sfc_writel(sfc, SFC_SCR, tmp);
}

void sfc_clear_all_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_SCR, 0x1f);
}

void sfc_mask_all_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_INTC, 0x1f);
}

void sfc_clock_phase(struct sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp &= ~DEV_CONF_CPHA;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp |= DEV_CONF_CPHA;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	}
}

void sfc_clock_polarity(struct sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp &= ~DEV_CONF_CPOL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_DEV_CONF);
		tmp |= DEV_CONF_CPOL;
		sfc_writel(sfc, SFC_DEV_CONF, tmp);
	}
}

void sfc_threshold(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_GLB);
	tmp &= ~GLB_THRESHOLD_MSK;
	tmp |= value << GLB_THRESHOLD_OFFSET;
	sfc_writel(sfc, SFC_GLB, tmp);
}

void sfc_smp_delay(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~DEV_CONF_SMP_DELAY_MSK;
	tmp |= value << DEV_CONF_SMP_DELAY_OFFSET;
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

void sfc_hold_delay(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~DEV_CONF_THOLD_MSK;
	tmp |= value << DEV_CONF_THOLD_OFFSET;
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

void sfc_setup_delay(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~DEV_CONF_TSETUP_MSK;
	tmp |= value << DEV_CONF_TSETUP_OFFSET;
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

void sfc_interval_delay(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~DEV_CONF_TSH_MSK;
	tmp |= value << DEV_CONF_TSH_OFFSET;
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

static int timing_is_valid(uint32_t *p_thold, uint32_t *p_tsetup, uint32_t *p_tsh)
{
	int ret = 0;

#define THOLD_LO_VAL 0x0
#define THOLD_HI_VAL 0x3
#define TSETUP_LO_VAL 0x0
#define TSETUP_HI_VAL 0x3
#define TSH_LO_VAL 0x0
#define TSH_HI_VAL 0xf

	if ((*p_thold > THOLD_HI_VAL) || (*p_thold < THOLD_LO_VAL)) {
		pr_err("ERROR: Check that the SFC timing parameter is invalid, thold:%d !\n", *p_thold);
		*p_thold = clamp((uint32_t)*p_thold, (uint32_t)THOLD_LO_VAL, (uint32_t)THOLD_HI_VAL);
		ret = -EINVAL;
	}

	if ((*p_tsetup > TSETUP_HI_VAL) || (p_thold < TSETUP_LO_VAL)) {
		pr_err("ERROR: Check that the SFC timing parameter is invalid, tsetup:%d !\n", *p_tsetup);
		*p_tsetup = clamp((uint32_t)*p_tsetup, (uint32_t)TSETUP_LO_VAL, (uint32_t)TSETUP_HI_VAL);
		ret = -EINVAL;
	}

	if ((*p_tsh > TSH_HI_VAL) || (*p_tsh < TSH_LO_VAL)){
		pr_err("ERROR: Check that the SFC timing parameter is invalid, tsh:%d !\n", *p_tsh);
		*p_tsh = clamp((uint32_t)*p_tsh, (uint32_t)TSH_LO_VAL, (uint32_t)TSH_HI_VAL);
		ret = -EINVAL;
	}

	return ret;
}

int set_flash_timing(struct sfc *sfc, unsigned int t_hold, unsigned int t_setup, unsigned int t_shslrd, unsigned int t_shslwr)
{
	uint32_t c_hold, c_setup, t_in, c_in;
	uint32_t c_half_hold, c_half_setup, c_half_in;
	uint32_t thold, tsetup, tsh;
	uint32_t tmp;
	unsigned long cycle;
	unsigned long half_cycle;
	unsigned long long ns;

	/* NOTE: 4 frequency division. */
	sfc->src_clk /= 4;

	ns = 1000000000ULL;
	do_div(ns, sfc->src_clk);
	cycle = ns;
	half_cycle = cycle / 2;

	/* Calculate the number of cycle */
	c_hold = t_hold / cycle;
	c_half_hold = t_hold % cycle;
	if(c_half_hold > half_cycle) {
		c_half_hold = 0;
		c_hold += 1;
	}

	c_setup = t_setup / cycle;
	c_half_setup = t_setup % cycle;
	if(c_half_setup > half_cycle) {
		c_half_setup = 0;
		c_setup += 1;
	}

	t_in = max(t_shslrd, t_shslwr);
	c_in = t_in / cycle;
	c_half_in = t_in % cycle;
	if(c_half_in > half_cycle) {
		c_half_in = 0;
		c_in += 1;
	}

	/* Calculate timing parameters */
	if(!c_hold && !c_half_hold)
		thold = 0;
	else
		thold = (2 * c_hold) - 1 + (!!c_half_hold);

	if(!c_setup && !c_half_setup)
		tsetup = 0;
	else
		tsetup = (2 * c_setup) - 1 + (!!c_half_setup);

	if(!c_in && !c_half_in)
		tsh = 0;
	else
		tsh = (2 * c_in) - 1 + (!!c_half_in);

	/* Verify parameters validity */
	timing_is_valid(&thold, &tsetup, &tsh);

	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~(DEV_CONF_THOLD_MSK | DEV_CONF_TSETUP_MSK | DEV_CONF_TSH_MSK);
	tmp |= (thold << DEV_CONF_THOLD_OFFSET) | \
		  (tsetup << DEV_CONF_TSETUP_OFFSET) | \
		  (tsh << DEV_CONF_TSH_OFFSET);

	sfc_writel(sfc, SFC_DEV_CONF, tmp);
	return 0;
}

void sfc_set_length(struct sfc *sfc, int value)
{
	sfc_writel(sfc, SFC_TRAN_LEN, value);
}

void sfc_transfer_mode(struct sfc *sfc, int value)
{
	if(value == 0) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_GLB);
		tmp &= ~GLB_OP_MODE;
		sfc_writel(sfc, SFC_GLB, tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_GLB);
		tmp |= GLB_OP_MODE;
		sfc_writel(sfc, SFC_GLB, tmp);
	}
}

void sfc_read_data(struct sfc *sfc, unsigned int *value)
{
	*value = sfc_readl(sfc, SFC_RM_DR);
}

void sfc_write_data(struct sfc *sfc, const unsigned int value)
{
	sfc_writel(sfc, SFC_RM_DR, value);
}

unsigned int sfc_fifo_num(struct sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(sfc, SFC_SR);
	tmp &= (0x7f << 16);
	tmp = tmp >> 16;
	return tmp;
}

static unsigned int cpu_read_rxfifo(struct sfc *sfc)
{
	int i;
	unsigned long align_len = 0;
	unsigned int fifo_num = 0;
	unsigned int data[1] = {0};
	unsigned int last_word = 0;
	struct sfc_cdt_xfer *xfer;

	xfer = sfc->xfer;
	align_len = ALIGN(xfer->config.datalen, 4);

	if(((align_len - xfer->config.cur_len) / 4) > THRESHOLD) {
		fifo_num = THRESHOLD;
		last_word = 0;
	} else {
		/* last aligned THRESHOLD data */
		if(xfer->config.datalen % 4) {
			fifo_num = (align_len - xfer->config.cur_len) / 4 - 1;
			last_word = 1;
		} else {
			fifo_num = (align_len - xfer->config.cur_len) / 4;
			last_word = 0;
		}
	}

	for(i = 0; i < fifo_num; i++) {
		sfc_read_data(sfc, (unsigned int *)xfer->config.buf);
		xfer->config.buf += 4;
		xfer->config.cur_len += 4;
	}

	/* last word */
	if(last_word == 1) {
		sfc_read_data(sfc, data);
		memcpy((void *)xfer->config.buf, data, xfer->config.datalen % 4);

		xfer->config.buf += xfer->config.datalen % 4;
		xfer->config.cur_len += 4;
	}
	return 0;
}

static unsigned int cpu_write_txfifo(struct sfc *sfc)
{
	int i;
	unsigned long align_len = 0;
	unsigned int fifo_num = 0;
	struct sfc_cdt_xfer *xfer;

	xfer = sfc->xfer;
	align_len = ALIGN(xfer->config.datalen, 4);

	if (((align_len - xfer->config.cur_len) / 4) > THRESHOLD){
		fifo_num = THRESHOLD;
	} else {
		fifo_num = (align_len - xfer->config.cur_len) / 4;
	}

	for(i = 0; i < fifo_num; i++) {
		sfc_write_data(sfc, *(unsigned int *)xfer->config.buf);
		xfer->config.buf += 4;
		xfer->config.cur_len += 4;
	}

	return 0;
}

unsigned int sfc_get_sta_rt(struct sfc *sfc)
{
	return sfc_readl(sfc,SFC_DEV_STA_RT);
}

unsigned int sfc_get_fsm(struct sfc *sfc)
{
	return sfc_readl(sfc,SFC_FSM);
}

void sfc_data_en(struct sfc *sfc, int channel, unsigned int value)
{
	if(value == 1) {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_TRAN_CONF0(channel));
		tmp |= TRAN_CONF0_DATEEN;
		sfc_writel(sfc, SFC_TRAN_CONF0(channel), tmp);
	} else {
		unsigned int tmp;
		tmp = sfc_readl(sfc, SFC_TRAN_CONF0(channel));
		tmp &= ~TRAN_CONF0_DATEEN;
		sfc_writel(sfc, SFC_TRAN_CONF0(channel), tmp);
	}
}

void sfc_dev_sta_exp(struct sfc *sfc, unsigned int value)
{
	sfc_writel(sfc, SFC_DEV_STA_EXP, value);
}

void sfc_dev_sta_msk(struct sfc *sfc, unsigned int value)
{
	sfc_writel(sfc, SFC_DEV_STA_MSK, value);
}

void sfc_enable_all_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_INTC, 0);
}

void sfc_set_mem_addr(struct sfc *sfc,unsigned int addr)
{
	sfc_writel(sfc, SFC_MEM_ADDR, addr);
}

static int sfc_sr_handle(struct sfc *sfc)
{
	uint32_t reg_sr = 0;
	uint32_t tmp = 0;
	int ret = 0;

	while (1) {
		reg_sr = sfc_readl(sfc, SFC_SR);

		if (reg_sr & CLR_RREQ) {
			sfc_writel(sfc, SFC_SCR, CLR_RREQ);
			cpu_read_rxfifo(sfc);
		}

		if (reg_sr & CLR_TREQ) {
			sfc_writel(sfc, SFC_SCR, CLR_TREQ);
			cpu_write_txfifo(sfc);
		}

		if(reg_sr & CLR_END){
			tmp = CLR_END;
			break;
		}

		if(reg_sr & CLR_UNDER){
			tmp = CLR_UNDER;
			printf("underun");
			ret = -1;
			break;
		}

		if(reg_sr & CLR_OVER){
			tmp = CLR_OVER;
			printf("overrun");
			ret = -1;
			break;
		}
	}
	if (tmp)
		sfc_writel(sfc, SFC_SCR, tmp);

	return ret;
}

static int sfc_start_transfer(struct sfc *sfc)
{
	int ret;
	sfc_clear_all_intc(sfc);
	sfc_mask_all_intc(sfc);
	sfc_start(sfc);

	ret = sfc_sr_handle(sfc);

	return ret;
}

static void sfc_use_cdt_mode(struct sfc *sfc)
{
	uint32_t tmp = sfc_readl(sfc, SFC_GLB);
	tmp |= GLB_CDT_EN;
	sfc_writel(sfc, SFC_GLB, tmp);
}

void write_cdt(struct sfc *sfc, struct sfc_cdt *cdt, uint16_t start_index, uint16_t end_index)
{
	uint32_t cdt_num, cdt_size;

	cdt_num = end_index - start_index + 1;
	cdt_size = sizeof(struct sfc_cdt);

	memcpy((void *)sfc->cdt_addr + (start_index * cdt_size), (void *)cdt + (start_index * cdt_size), cdt_num * cdt_size);
	printf("create CDT index: %d ~ %d,  index number:%d.\n", start_index, end_index, cdt_num);
}

static void sfc_set_index(struct sfc *sfc, unsigned short index)
{

	uint32_t tmp = sfc_readl(sfc, SFC_CMD_IDX);
	tmp &= ~CMD_IDX_MSK;
	tmp |= index;
	sfc_writel(sfc, SFC_CMD_IDX, tmp);
}

static void sfc_set_dataen(struct sfc *sfc, uint8_t dataen)
{

	uint32_t tmp = sfc_readl(sfc, SFC_CMD_IDX);
	tmp &= ~CDT_DATAEN_MSK;
	tmp |= (dataen << CDT_DATAEN_OFF);
	sfc_writel(sfc, SFC_CMD_IDX, tmp);
}

static void sfc_set_datadir(struct sfc *sfc, uint8_t datadir)
{

	uint32_t tmp = sfc_readl(sfc, SFC_CMD_IDX);
	tmp &= ~CDT_DIR_MSK;
	tmp |= (datadir << CDT_DIR_OFF);
	sfc_writel(sfc, SFC_CMD_IDX, tmp);
}

static void sfc_set_addr(struct sfc *sfc, struct sfc_cdt_xfer *xfer)
{
	sfc_writel(sfc, SFC_COL_ADDR, xfer->columnaddr);
	sfc_writel(sfc, SFC_ROW_ADDR, xfer->rowaddr);
	sfc_writel(sfc, SFC_STA_ADDR0, xfer->staaddr0);
	sfc_writel(sfc, SFC_STA_ADDR1, xfer->staaddr1);
}

extern void flush_cache_all(void);
static void sfc_set_data_config(struct sfc *sfc, struct sfc_cdt_xfer *xfer)
{
	sfc_set_dataen(sfc, xfer->dataen);

	if(xfer->dataen){
		sfc_set_datadir(sfc, xfer->config.data_dir);
		sfc_transfer_mode(sfc, xfer->config.ops_mode);
		sfc_set_length(sfc, xfer->config.datalen);

		if((xfer->config.ops_mode == DMA_OPS)){
			flush_cache_all();
			sfc_set_mem_addr(sfc, virt_to_phys(xfer->config.buf));
		}else{
			sfc_set_mem_addr(sfc, 0);
			sfc->xfer = xfer;
		}
	}
}

int sfc_sync_cdt(struct sfc *sfc, struct sfc_cdt_xfer *xfer)
{
	/* 1. set index */
	sfc_set_index(sfc, xfer->cmd_index);

	/* 2. set addr */
	sfc_set_addr(sfc, xfer);

	/* 3. config data config */
	sfc_set_data_config(sfc, xfer);

	return sfc_start_transfer(sfc);
}

static int sfc_ctl_init(struct sfc *sfc)
{
	sfc_init(sfc);
	sfc_stop(sfc);

	/*set hold high*/
	sfc_hold_invalid_value(sfc, 1);
	/*set wp high*/
	sfc_wp_invalid_value(sfc, 1);

	sfc_clear_all_intc(sfc);
	sfc_mask_all_intc(sfc);

	sfc_threshold(sfc, sfc->threshold);
	/*config the sfc pin init state*/
	sfc_clock_phase(sfc, 0);
	sfc_clock_polarity(sfc, 0);
	sfc_ce_invalid_value(sfc, 1);

	/* use CDT mode*/
	sfc_use_cdt_mode(sfc);
	printf("Enter 'CDT' mode.\n");

	sfc_transfer_mode(sfc, SLAVE_MODE);
	return 0;
}

void sfc_clk_set(struct sfc *sfc, uint32_t sfc_rate)
{
	sfc->src_clk = (unsigned long)sfc_rate;
	clk_set_rate(SFC, sfc->src_clk);

	if(sfc->src_clk >= 200000000){
		/* set sample delay */
		sfc_smp_delay(sfc,DEV_CONF_SMP_DELAY_180);
	}
}

struct sfc *sfc_res_init(uint32_t def_sfc_rate)
{
	struct sfc *sfc = NULL;
	sfc = malloc(sizeof(struct sfc));
	if (!sfc) {
		printf("ERROR: %s %d kzalloc() error !\n",__func__,__LINE__);
		return ERR_PTR(-ENOMEM);
	}
	memset(sfc, 0, sizeof(struct sfc));

	/* sfc CDT init*/
	sfc->cdt_addr = (volatile void *)(SFC_BASE + SFC_CDT);

	sfc_clk_set(sfc, def_sfc_rate);

	sfc->threshold = THRESHOLD;

	sfc_ctl_init(sfc);

	return sfc;

}
