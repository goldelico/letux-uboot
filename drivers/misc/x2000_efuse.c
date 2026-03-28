#include <common.h>
#include <exports.h>
#include <malloc.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/err.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/errno.h>
#include <asm/arch/base.h>
#include <asm/arch/clk.h>
#include <asm/arch/efuse.h>
#include <efuse.h>
#include "hamming.h"

static int efuse_debug = 0;
static int efuse_gpio = -1;
static int efuse_en_active = 0;

static struct seg_info info;

static uint32_t efuse_readl(uint32_t reg_off)
{
	return readl(EFUSE_BASE + reg_off);
}

static void efuse_writel(uint32_t val, uint32_t reg_off)
{
	writel(val, EFUSE_BASE + reg_off);
}

static void boost_vddq(int gpio)
{
	int val;
	printf("boost vddq\n");
	gpio_direction_output(gpio, efuse_en_active);
	do {
		val = gpio_get_value(gpio);
		printf("gpio %d level %d\n",gpio,val);
	} while (val != efuse_en_active);
	mdelay(10);		/*  mdelay(10) wait for EFUSE VDDQ setup. */
}

static void reduce_vddq(int gpio)
{
	int val;
	printf("reduce vddq\n");
	gpio_direction_output(gpio, !efuse_en_active);
	do {
		val = gpio_get_value(gpio);
		printf("gpio %d level %d\n",gpio,val);
	} while (val == efuse_en_active);
	mdelay(10);		/*  mdelay(10) wait for EFUSE VDDQ fall down. */
}


static void otp_r_efuse(uint32_t addr, uint32_t wlen)
{
	unsigned int val;
	int n;

	efuse_writel(0, EFUSE_CTRL);

	for(n = 0; n < 8; n++)
		efuse_writel(0, EFUSE_DATA(n));

	/* set read address and data length */
	val =  addr << EFUSE_CTRL_ADDR | (wlen - 1) << EFUSE_CTRL_LEN;
	efuse_writel(val, EFUSE_CTRL);

	val = efuse_readl(EFUSE_CTRL);
	val &= ~EFUSE_CTRL_PD;
	efuse_writel(val, EFUSE_CTRL);

	/* enable read */
	val = efuse_readl(EFUSE_CTRL);
	val |= EFUSE_CTRL_RDEN;
	efuse_writel(val, EFUSE_CTRL);

	//printf("efuse ctrl regval=0x%x\n",val);
	/* wait read done status */
	while(!(efuse_readl(EFUSE_STATE) & EFUSE_STA_RD_DONE));

	efuse_writel(0, EFUSE_CTRL);
	efuse_writel(EFUSE_CTRL_PD, EFUSE_CTRL);
}

static void rir_w(uint32_t addr, uint32_t value)
{
	unsigned int val;

	efuse_writel(value, EFUSE_DATA(0));
	efuse_writel(0, EFUSE_CTRL);

	val =  addr << EFUSE_CTRL_ADDR | 0 << EFUSE_CTRL_LEN;
	efuse_writel(val, EFUSE_CTRL);

	val = efuse_readl(EFUSE_CTRL);
	val &= ~EFUSE_CTRL_PD;
	efuse_writel(val, EFUSE_CTRL);

	val = efuse_readl(EFUSE_CTRL);
	val |= EFUSE_CTRL_PS | EFUSE_CTRL_RWL;
	efuse_writel(val, EFUSE_CTRL);

	val = efuse_readl(EFUSE_CTRL);
	val |= EFUSE_CTRL_PGEN;
	efuse_writel(val, EFUSE_CTRL);

	boost_vddq(efuse_gpio);

	val = efuse_readl(EFUSE_CTRL);
	val |= EFUSE_CTRL_WREN;
	efuse_writel(val, EFUSE_CTRL);

	/* wait write done status */
	while(!(efuse_readl(EFUSE_STATE) & EFUSE_STA_WR_DONE));

	reduce_vddq(efuse_gpio);

	efuse_writel(0, EFUSE_CTRL);
	efuse_writel(EFUSE_CTRL_PD, EFUSE_CTRL);
}

static void rir_r(void)
{
	unsigned int val;

	efuse_writel(0, EFUSE_CTRL);
	efuse_writel(0, EFUSE_DATA(0));
	efuse_writel(0, EFUSE_DATA(1));

	/* set rir read address and data length */
	val =  0x1f << EFUSE_CTRL_ADDR | 0x1 << EFUSE_CTRL_LEN;
	efuse_writel(val, EFUSE_CTRL);

	val = efuse_readl(EFUSE_CTRL);
	val &= ~EFUSE_CTRL_PD;
	efuse_writel(val, EFUSE_CTRL);

	val = efuse_readl(EFUSE_CTRL);
	val &= ~EFUSE_CTRL_PS;
	efuse_writel(val, EFUSE_CTRL);

	val = efuse_readl(EFUSE_CTRL);
	val |= EFUSE_CTRL_RWL;
	efuse_writel(val, EFUSE_CTRL);

	val = efuse_readl(EFUSE_CTRL);
	val |= EFUSE_CTRL_RDEN;
	efuse_writel(val, EFUSE_CTRL);

	/* wait read done status */
	while(!(efuse_readl(EFUSE_STATE) & EFUSE_STA_RD_DONE));

	efuse_writel(0, EFUSE_CTRL);
	efuse_writel(EFUSE_CTRL_PD, EFUSE_CTRL);

	printf("RIR0=0x%08x\n", efuse_readl(EFUSE_DATA(0)));
	printf("RIR1=0x%08x\n", efuse_readl(EFUSE_DATA(1)));
}

static int rir_op(uint32_t value, uint32_t flag)
{
	unsigned int addr = 0, rf_addr = 0;
	unsigned int fb_disable = 0;
	unsigned int ret1, ret2;
	int rir_num = 0;

	if(value == 0)
		return -1;

	rir_r();
	ret1 = efuse_readl(EFUSE_DATA(0));
	ret2 = efuse_readl(EFUSE_DATA(1));

	if((ret1 & 0xFFFF) && (ret1 & (0xFFFF << 16)) &&
			(ret2 & 0xFFFF) && (ret2 & (0xFFFF << 16))) {
		if(flag == 1) {
			fb_disable = 0x1 << 31;
			rf_addr = 0x20;
			rir_w(rf_addr, fb_disable);
		}
		printf("not redundancy bits!\n");
		return -1;
	}

	if(((ret1 & (0xFFFF)) && (ret1 & (0xFFFF << 16)))) {
		addr = 0x20;
		if(ret2 & 0xFFFF) {
			value = value << 16;
			rir_num = 4;
		} else {
			rir_num = 3;
		}
	} else {
		addr = 0;
		if(ret1 & 0xFFFF) {
			value = value << 16;
			rir_num = 2;
		} else {
			rir_num = 1;
		}
	}

	if(flag == 1) {
		switch(rir_num) {
			case 2:
				fb_disable = 0x1 << 15;
				rf_addr = 0x0;
				break;
			case 3:
				fb_disable = 0x1 << 31;
				rf_addr = 0x0;
				break;
			case 4:
				fb_disable = 0x1 << 15;
				rf_addr = 0x20;
				break;
			default:
				printf("not rir %d!\n", rir_num);
				return -1;
		}

		rir_w(rf_addr, fb_disable);
	}

	rir_w(addr, value);

	return 0;
}

static int rir_check(struct seg_info *info, uint32_t woffs, uint32_t val)
{
	unsigned int rval, errbits;

	rir_r();
	otp_r_efuse(info->word_address + woffs, 1);

	rval = efuse_readl(EFUSE_DATA(0));
	if(woffs == 0)
		rval &= 0xffffffff << info->begin_align * 8;
	else if(woffs == info->word_num - 1)
		rval &= 0xffffffff >> info->end_align* 8;

	printf("%08x ^ %08x\n", rval, val);
	errbits = rval ^ val;

	return errbits;
}

static int rir_repair(struct seg_info *info, uint32_t *buf)
{
	unsigned int errbits, rir_data, repair_result, repair_fail;
	int ret, n, ebit;

	for(n = 0; n < info->word_num; n++) {
		errbits = rir_check(info, n, buf[n]);
		printf("addr=%x, errbits=0x%08x\n", info->word_address + n, errbits);

		while((ebit = ffs(errbits)) > 0) {
			rir_data = 0x1 << EFUSE_RIR_RF;
			rir_data |= (buf[n] & ebit) << EFUSE_RIR_DATA;
			rir_data |= (info->word_address + n + ((ebit + info->begin_align * 8) << 6)) << EFUSE_RIR_ADDR;
			//rir_data &= 0 << EFUSE_RIR_DISABLE;

			ret = rir_op(rir_data, 0);
			if(ret) {
				printf("rir repair failed!\n");
				return -1;
			}

			do {
				repair_result = rir_check(info, n, buf[n]);
				repair_fail = repair_result & (0x1 << ebit);
				if(repair_fail) {
					ret = rir_op(rir_data, 1);
					if(ret) {
						printf("rir repair failed!\n");
						return -1;
					}
				}
			} while(repair_fail);
			errbits &= 0 << ebit;
		}
	}

	return 0;
}

static void rir_disable_all(void)
{
	rir_r();
	rir_w(0x0, (1 << 15));
	rir_w(0x0, (1 << 31));
	rir_w(0x20, (1 << 15));
	rir_w(0x20, (1 << 31));
}

static int jz_efuse_read(struct seg_info *info, uint32_t *buf)
{
	uint32_t val;
	uint32_t rbuf[8] = {0};
	uint32_t hamming_tmp_buf[8] = {0};
	uint32_t double_tmp_buf[8] = {0};
	uint32_t double_tmp1_buf[8] = {0};
	uint32_t hamming_buf[8] = {0};
	uint32_t byte_num = 0;
	uint32_t half_bit_num = 0;
	uint32_t half_byte_num = 0;
	uint32_t half_byte_tmp_num = 0;
	uint32_t half_bit_align = 0;
	uint32_t hamming_byte_num = 0;
	uint32_t hamming_bit_num = 0;
	int n, ret;

	printf("segment name: %s\nsegment addr: 0x%02x\nbegin align: %d\nend align: %d\n"
			"word num: %d\nbit num: %d\nverify mode: %d\n",
			info->seg_name, info->word_address, info->begin_align, info->end_align,
			info->word_num, info->bit_num, info->verify_mode);

	rir_r();
	otp_r_efuse(info->word_address, info->word_num);

	for(n = 0; n < info->word_num; n++) {
		val = efuse_readl(EFUSE_DATA(n));
		printf("%08x\n", val);
		if(n == 0)
			rbuf[n] = val & (0xffffffff << info->begin_align * 8);
		if(n == info->word_num - 1)
			rbuf[n] = val & (0xffffffff >> info->end_align * 8);
		else
			rbuf[n] = val;
	}

	byte_num = info->bit_num / 8;
	byte_num += info->bit_num % 8 ? 1 : 0;

	switch(info->verify_mode) {
		case HAMMING:
			hamming_bit_num = info->bit_num + cal_k(info->bit_num);
			hamming_byte_num = hamming_bit_num / 8;
			hamming_byte_num += hamming_bit_num % 8 ? 1 : 0;
			memcpy((char *)hamming_tmp_buf,((char *)rbuf + info->begin_align),hamming_byte_num);
			decode(hamming_tmp_buf,hamming_bit_num, hamming_buf);
			memcpy((char *)buf, (char *)hamming_buf, byte_num);
			break;
		case DOUBLE:
			half_bit_num = info->bit_num / 2;
			half_byte_num = half_bit_num / 8;
			half_bit_align = half_bit_num % 8;
			half_byte_tmp_num = half_bit_num / 8;
			half_byte_tmp_num += half_bit_num % 8 ? 1: 0;

			memcpy((char *)double_tmp_buf,((char *)rbuf + info->begin_align),half_byte_tmp_num);
			memcpy((char *)double_tmp1_buf,((char *)rbuf + info->begin_align + half_byte_num),half_byte_tmp_num);
			ret = checkbit(double_tmp_buf,double_tmp1_buf,0, half_bit_align, half_bit_num);
			if(ret){
				printf("double verify failed!\n");
				return -1;
			}
			memcpy((char *)buf, ((char *)rbuf + info->begin_align), byte_num);
			break;
		case NONE:
		default:
			memcpy((char *)buf, ((char *)rbuf + info->begin_align), byte_num);
			break;
	}
	/*
	   printf("efuse read data after decode :\n");
	   for(n = 0; n < info->word_num; n++) {
	   printf("%08x\n", buf[n]);

	   }
	   */
	/* clear read done status */
	efuse_writel(0, EFUSE_STATE);

	return 0;
}


static void otp_w(uint32_t addr, uint32_t wlen)
{
	unsigned int val;

	efuse_writel(0, EFUSE_CTRL);

	/* set write Programming address and data length */
	val =  addr << EFUSE_CTRL_ADDR | (wlen - 1) << EFUSE_CTRL_LEN;
	efuse_writel(val, EFUSE_CTRL);

	val = efuse_readl(EFUSE_CTRL);
	val &= ~EFUSE_CTRL_PD;
	efuse_writel(val, EFUSE_CTRL);

	val = efuse_readl(EFUSE_CTRL);
	val |= EFUSE_CTRL_PS;
	efuse_writel(val, EFUSE_CTRL);

	/* Programming EFUSE enable */
	val = efuse_readl(EFUSE_CTRL);
	val |= EFUSE_CTRL_PGEN;
	efuse_writel(val, EFUSE_CTRL);

	/* Connect VDDQ pin from 1.8V */
	boost_vddq(efuse_gpio);

	/* enable write */
	val = efuse_readl(EFUSE_CTRL);
	val |= EFUSE_CTRL_WREN;
	efuse_writel(val, EFUSE_CTRL);

	/* wait write done status */
	while(!(efuse_readl(EFUSE_STATE) & EFUSE_STA_WR_DONE));

	/* Disconnect VDDQ pin from 1.8V. */
	reduce_vddq(efuse_gpio);

	efuse_writel(0, EFUSE_CTRL);
	efuse_writel(EFUSE_CTRL_PD, EFUSE_CTRL);
}

static int jz_efuse_write(struct seg_info *info, uint32_t *buf)
{
	unsigned int val[8] = {0};
	unsigned int pbuf[8] = {0};
	unsigned int sbuf[8] = {0};
	uint32_t regval = 0;
	uint32_t byte_num = 0;
	uint32_t half_bit_num = 0;
	uint32_t half_byte_num = 0;
	uint32_t half_bit_align = 0;
	uint32_t hamming_bit_num = 0;
	uint32_t hamming_byte_num = 0;
	int ret = 0;
	int n = 0;


	printf("segment name: %s\nsegment addr: 0x%02x\nbegin align: %d\nend align: %d\n"
			"word num: %d\nbit num: %d\nverify mode: %d\n",
			info->seg_name, info->word_address, info->begin_align, info->end_align,
			info->word_num, info->bit_num, info->verify_mode);

	if(info->seg_id != PRT) {
		regval = efuse_readl(EFUSE_STATE);
		if(info->prt_bit & regval) {
			printf("segment[%s] has been protected!\n", info->seg_name);
			return -1;
		}
	}

	byte_num = info->bit_num / 8;
	byte_num += info->bit_num % 8 ? 1 : 0;
	switch(info->verify_mode) {
		case HAMMING:
			hamming_bit_num = info->bit_num + cal_k(info->bit_num);
			encode(buf, info->bit_num, pbuf);
			hamming_byte_num = hamming_bit_num / 8;
			hamming_byte_num += hamming_bit_num % 8 ? 1 : 0;
			memcpy(((char *)val+info->begin_align),(char *)pbuf,hamming_byte_num);
			break;
		case DOUBLE:
			half_bit_num = info->bit_num / 2;
			half_byte_num = half_bit_num / 8;
			half_bit_align = half_bit_num % 8;
			dump(buf, 0, half_bit_num);
			memcpy((char *)sbuf, ((char *)buf + half_byte_num), half_byte_num);
			dump(sbuf, 0, half_bit_num);
			ret = checkbit(buf, sbuf, 0, half_bit_align, half_bit_num);
			if(ret){
				printf("double verify failed!\n");
				return -1;
			}
			memcpy(((char *)val + info->begin_align), (char *)buf, byte_num);
			break;
		case NONE:
		default:
			memcpy(((char *)val + info->begin_align), (char *)buf, byte_num);
			break;
	}
	printf("efuse write data:\n");
	for(n = 0; n < info->word_num; n++) {
		printf("%08x\n", val[n]);
		efuse_writel(val[n], EFUSE_DATA(n));
	}
	otp_w(info->word_address, info->word_num);

	if(info->verify_mode == HAMMING) {
		ret = rir_repair(info, val);
		if(ret < 0){
			printf("hamming verify failed!\n");
			return -1;
		}
	}
	return 0;
}


static int adjust_efuse()
{

	uint32_t val, ns;
	int i, rd_strobe, wr_strobe;
	uint32_t rd_adj, wr_adj;
	int flag;
	int h2clk = clk_get_rate(H2CLK);

	ns = 1000000000 / h2clk;
	printf("rate = %d, ns = %d\n", h2clk, ns);

	for(i = 0; i < 0x4; i++)
		if((( i + 1) * ns ) > 4)
			break;
	if(i == 0x4) {
		printf("get efuse cfg rd_adj fail!\n");
		return -1;
	}
	rd_adj = wr_adj = i;

	for(i = 0; i < 0x8; i++)
		if(((rd_adj + i + 30) * ns ) > 100)
			break;
	if(i == 0x8) {
		printf("get efuse cfg rd_strobe fail!\n");
		return -1;
	}
	rd_strobe = i;

	for(i = 0; i < 0x3ff; i++) {
		val = (wr_adj + i + 3000) * ns;
		if(val >= 13 * 1000) {
			val = (wr_adj - i + 3000) * ns;
			flag = 1;
		}
		if(val > 11 * 1000 && val < 13 * 1000)
			break;
	}
	if(i >= 0x3ff) {
		printf("get efuse cfg wd_strobe fail!\n");
		return -1;
	}

	if(flag)
		i |= 1 << 10;

	wr_strobe = i;

	printf("rd_adj = %d | rd_strobe = %d | "
			"wr_adj = %d | wr_strobe = %d\n", rd_adj, rd_strobe,
			wr_adj, wr_strobe);

	/*set configer register*/
	val = rd_adj << EFUSE_CFG_RD_ADJ | rd_strobe << EFUSE_CFG_RD_STROBE;
	val |= wr_adj << EFUSE_CFG_WR_ADJ | wr_strobe;
	efuse_writel(val, EFUSE_CFG);
	printf("h2clk is %d, efuse_reg 0x%x\n",h2clk,efuse_readl(EFUSE_CFG));
	return 0;
}

int efuse_read(void *buf, int length, off_t offset)
{
        int i = 0;
	int ret = -EPERM;
	uint32_t seg_id = 0;
	uint32_t seg_length = 0;
	char *last = NULL;
	uint32_t val[8] = {0};

	seg_length = sizeof(seg_info_array) / sizeof(seg_info_array[0]);

	if(offset < seg_info_array[0].word_address || offset >= seg_info_array[seg_length-1].word_address+seg_info_array[seg_length-1].word_num) {
	    printf("ERROR:offset is error\n");
	    return -EPERM;
	}

	for(i=0; i < seg_length; i++) {
	    if (offset >= seg_info_array[i].word_address && offset < seg_info_array[i].word_address + seg_info_array[i].word_num) {
		seg_id = i;
		break;
	    }
	}

	info = seg_info_array[seg_id];
	last = (char *)val + info.bit_num / 8 - 1;
	ret = jz_efuse_read(&info,val);
	if(ret < 0) {
	        printf("efuse_read_id: read id error\n");
		return ret;
	}

        for(i = 0; i < info.bit_num / 8; i++)
		snprintf((char *)buf + (i * 2), 3, "%02x", *((uint8_t *)last - i));
	strcat(buf, "\n");
	printf("read efuse data: %s\n",buf);
	return 0;
}

int efuse_read_id(void *buf, int length, int seg_id)
{
	int i = 0;
	int ret = -EPERM;
	char *last = NULL;
	uint32_t val[8] = {0};
	info = seg_info_array[seg_id];
	last = (char *)val + info.bit_num / 8 - 1;
	ret = jz_efuse_read(&info,val);
	if(ret < 0) {
		printf("efuse_read_id: read id error\n");
		return ret;
	}

	for(i = 0; i < info.bit_num / 8; i++)
		snprintf((char *)buf + (i * 2), 3, "%02x", *((uint8_t *)last - i));
	strcat(buf, "\n");
	printf("read efuse data: %s\n",buf);
	return info.bit_num / 4;
}
int efuse_write(void *buf, int length, off_t seg_id)
{
	int ret = -EPERM;
	int byte_num = 0;
	int word_num = 0;
	int left_num = 0;
	struct seg_info info;
	unsigned int prtbit = 0;
	unsigned int val[8] = {0};
	char tmp[9] = {'\0'};
	char *last = (char *)buf + length;
	int i = 0;

	if (IS_ERR(buf)) {
		printf("%s %d: buffer error!\n",__func__,__LINE__);
		return ret;
	}

	if (seg_id < 0 || seg_id > NKU) {
		printf("%s %d: segment id error!\n",__func__,__LINE__);
		return ret;
	}

	info = seg_info_array[seg_id];
	byte_num = length / 2;
	word_num = byte_num / 4;
	left_num = byte_num % 4;

	if (byte_num > info.bit_num / 8) {
		printf("%s %d: %s segment size error! %d %d\n",
				__func__,__LINE__,info.seg_name,info.bit_num,byte_num);
		return ret;
	}


	printf("%s %d: input %s\n",__func__,__LINE__,buf);
	for (i = 0; i < word_num; i++) {
		memcpy(tmp, last - ((i + 1) * 8), 8);
		val[i] = (unsigned int)simple_strtoul(tmp, NULL, 16);
	}

	if (left_num > 0)  {
		memcpy(tmp, (char *)buf, left_num * 2);
		val[i] = (unsigned int)simple_strtoull(tmp, NULL, 16);
	}

	ret = jz_efuse_write(&info, val);
	if (ret != 0) {
		printf("%s %d: write error!\n",__func__,__LINE__);
		return ret;
	}

	return ret;
}

int efuse_init(int gpio_pin, int active)
{
	if(gpio_pin >= 0){
		if(efuse_gpio >= 0) gpio_free(efuse_gpio);
		efuse_gpio = gpio_request(gpio_pin, "VDDQ");
		if(efuse_gpio < 0) return efuse_gpio;
		efuse_en_active = active;
	}else{
		efuse_gpio = -1;
	}

	if(adjust_efuse() < 0)
		return -1;
	return 0;
}
void efuse_debug_enable(int enable)
{
	efuse_debug = !!enable;
	return;
}
