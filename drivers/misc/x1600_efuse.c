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

static int efuse_debug = 1;
static int efuse_en_gpio = -1;
static int efuse_en_active = 0;


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


static void otp_r(uint32_t addr, uint32_t blen)
{
	unsigned int val;
	int n;

	efuse_writel(0, EFUSE_CTRL);

	for(n = 0; n < 8; n++)
		efuse_writel(0, EFUSE_DATA(n));

	/* set read address and data length */
	val =  addr << EFUSE_CTRL_ADDR | (blen - 1) << EFUSE_CTRL_LEN;
	efuse_writel(val, EFUSE_CTRL);

	/* enable read */
	val = efuse_readl(EFUSE_CTRL);
	val |= EFUSE_CTRL_RDEN;
	efuse_writel(val, EFUSE_CTRL);

	/* wait read done status */
	while(!(efuse_readl(EFUSE_STATE) & EFUSE_STA_RD_DONE));
}

static int jz_efuse_read(struct seg_info *info, uint8_t *buf)
{
	int i;
	unsigned int val = 0;
	int byte_num = info->bit_num / 8;
	int word_num = byte_num / 4;
	word_num += byte_num % 4 ? 1 : 0;


	printf("segment name: %s\nsegment addr: 0x%02x\nbyte num: %d\nbit num: %d\n",
			info->seg_name, info->offset_address, byte_num, info->bit_num);

	otp_r(info->offset_address, byte_num);

	debug_cond(efuse_debug, "efuse read data:\n");
	for (i = 0; i < word_num; i++) {
		val = efuse_readl(EFUSE_DATA(i));
		debug_cond(efuse_debug, "0x%08x\n", val);
		*((unsigned int *)buf + i) = val;
	}

	/* clear read done status */
	efuse_writel(0, EFUSE_STATE);

	return 0;
}

static void otp_w(uint32_t addr, uint32_t blen)
{
	unsigned int val;

	efuse_writel(0, EFUSE_CTRL);

	/* set  Programming address and data length */
	val =  addr << EFUSE_CTRL_ADDR | (blen - 1) << EFUSE_CTRL_LEN;
	efuse_writel(val, EFUSE_CTRL);

	/* Programming EFUSE enable */
	val = efuse_readl(EFUSE_CTRL);
	val |= EFUSE_CTRL_PGEN;
	efuse_writel(val, EFUSE_CTRL);

	/* Connect VDDQ pin from 2.5V */
	boost_vddq(efuse_en_gpio);
	mdelay(1);

	/* enable write */
	val = efuse_readl(EFUSE_CTRL);
	val |= EFUSE_CTRL_WREN;
	efuse_writel(val, EFUSE_CTRL);

	/* wait write done status */
	while(!(efuse_readl(EFUSE_STATE) & EFUSE_STA_WR_DONE));

	/* Disconnect VDDQ pin from 2.5V. */
	reduce_vddq(efuse_en_gpio);
	mdelay(1);

	val = efuse_readl(EFUSE_CTRL);
	val &= ~(EFUSE_CTRL_PGEN);
	efuse_writel(val, EFUSE_CTRL);
}

static int jz_efuse_write(struct seg_info *info, uint8_t *buf)
{
	int byte_num = info->bit_num / 8;
	int word_num = byte_num / 4;
	word_num += byte_num % 4 ? 1 : 0;
	unsigned int val = 0;
	int i;

	debug_cond(efuse_debug, "segment name: %s\nsegment addr: 0x%02x\nbyte num: %d\nbit num: %d\n",
			info->seg_name, info->offset_address, byte_num, info->bit_num);

	if (info->seg_id != PRT) {
		val = efuse_readl(EFUSE_STATE);
		if(info->prt_bit & val) {
			printf("segment[%s] has been protected!\n", info->seg_name);
			return -1;
		}
	}

	debug_cond(efuse_debug, "efuse write data:\n");
	for (i = 0; i < word_num; i++) {
		val = *((unsigned int *)buf + i);
		efuse_writel(val, EFUSE_DATA(i));
		debug_cond(efuse_debug, "0x%08x\n", val);
	}

	otp_w(info->offset_address, byte_num);

	return 0;
}

static int set_efuse_timing()
{
	unsigned long rate;
	uint32_t val, ns;
	int i, rd_strobe, wr_strobe;
	uint32_t rd_adj, wr_adj;
	int flag = 0;

	rate = clk_get_rate(H2CLK);
	ns = 1000000000 / rate;
	printf("rate = %lu, ns = %d\n", rate, ns);


	for(i = 0; i < 0x4; i++)
		if((( i + 1) * ns ) > 7)
			break;
	if(i == 0x4) {
		printf("get efuse cfg rd_adj fail!\n");
		return -1;
	}
	rd_adj = wr_adj = i;

	for(i = 0; i < 0x8; i++)
		if(((rd_adj + i + 5) * ns ) > 35)
			break;
	if(i == 0x8) {
		printf("get efuse cfg rd_strobe fail!\n");
		return -1;
	}
	rd_strobe = i;

	for(i = 0; i < 0x7ff; i++) {
		val = (wr_adj + i + 1666) * ns;
		if(val > 11 * 1000) {
			val = (wr_adj - i + 1666) * ns;
			flag = 1;
		}
		if(val > 9 * 1000 && val < 11 * 1000)
			break;
	}
	if(i >= 0x7ff) {
		printf("get efuse cfg wd_strobe fail!\n");
		return -1;
	}

	if(flag)
		i |= 1 << 11;

	wr_strobe = i;

	printf("rd_adj = %d | rd_strobe = %d | wr_adj = %d | wr_strobe = %d\n",
			rd_adj, rd_strobe, wr_adj, wr_strobe);

	/*set configer register*/
	val = rd_adj << EFUSE_CFG_RD_ADJ | rd_strobe << EFUSE_CFG_RD_STROBE;
	val |= wr_adj << EFUSE_CFG_WR_ADJ | wr_strobe;
	efuse_writel(val, EFUSE_CFG);

	return 0;
}

int efuse_read(void *buf, int length, off_t seg_id)
{
	int ret = -EPERM;
	int byte_num = 0;
	struct seg_info info;

	if(IS_ERR(buf)) {
		printf("%s %d: buffer error!\n",__func__,__LINE__);
		return ret;
	}

	info = seg_info_array[seg_id];
	byte_num = info.bit_num / 8;

	if(length > byte_num) {
		printf("%s %d: %s segment size error! %d\n",__func__,__LINE__,info.seg_name,byte_num);
		return ret;
	}

	ret = jz_efuse_read(&info, buf);
	if(ret < 0) {
		printf("%s %d: read error!\n",__func__,__LINE__);
		return ret;
	}

	return ret;
}

int efuse_read_id(void *buf, int length, int seg_id)
{
	int ret = -EPERM;
	unsigned int val[8] = {0};
	struct seg_info info;
	info = seg_info_array[seg_id];
	int byte_num = info.bit_num / 8;
	char *last = (char *)val + byte_num - 1;
	int i = 0;

	if(IS_ERR(buf)) {
		printf("%s %d: buffer error!\n",__func__,__LINE__);
		return ret;
	}

	ret = jz_efuse_read(&info, val);
	if(ret < 0) {
		printf("%s %d: read error!\n",__func__,__LINE__);
		return ret;
	}

	for(i = 0; i < byte_num; i++) {
		snprintf((char *)buf + (i * 2), 3, "%02x", *((uint8_t *)last - i));
	}

	return strlen(buf);
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
		if(efuse_en_gpio >= 0) gpio_free(efuse_en_gpio);
		efuse_en_gpio = gpio_request(gpio_pin, "VDDQ");
		if(efuse_en_gpio < 0) return efuse_en_gpio;
		efuse_en_active = active;
	}else{
		efuse_en_gpio = -1;
	}

	if(set_efuse_timing() < 0)
		return -1;

	printf("%s %d: successful!\n",__func__,__LINE__);
	return 0;
}

void efuse_deinit(void)
{
	if (efuse_en_gpio >= 0)
		gpio_free(efuse_en_gpio);
	efuse_en_gpio = -1;
	return;
}

void efuse_debug_enable(int enable)
{
	efuse_debug = !!enable;
	return;
}
