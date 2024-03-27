#include <common.h>
#include <command.h>
#include <config.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/jz_uart.h>

#include <asm/arch-x2000_v12/cpm.h>
#include "ssi.h"

#define GPIO_BIT_SPI_DATA           GPIO_PB(30)
#define GPIO_BIT_SPI_CLK           GPIO_PB(31)

void ssi_init(void)
{
	unsigned int tmp,cr0;
	printf("%s()\n", __func__);
	/*
	 *  SD12/SSI0_CE0_N/UART8_RXD/PB28
	 *  SD13/SSI0_DR/UART8_TXD/PB29
	 *  SD14/SSI0_DT/UART9_RXD/PB30
	 *  SD15/SSI0_CLK/UART9_TXD/PB31
	 */
	gpio_set_func((GPIO_BIT_SPI_DATA / 32), GPIO_FUNC_1, 1 << (GPIO_BIT_SPI_DATA % 32));
	gpio_set_func((GPIO_BIT_SPI_CLK / 32), GPIO_FUNC_1, 1 << (GPIO_BIT_SPI_CLK % 32));

	// spi clock gate
	tmp = readl(CPM_BASE + CPM_CLKGR0);
	//tmp &= ~(1 << CPM_CLKGR_SSI0);
	//tmp &= ~(CPM_CLKGR_SSI0);
	tmp &= ~(1<<19);
	writel(tmp,CPM_BASE + CPM_CLKGR0);
	// ssi clock divider
	tmp = 1200/60;	tmp--;	/* APLL=1200MHz, SPI=30MHz(60/2) */
	if (tmp>0xff) tmp=0xff;
	tmp = (0<<30)|(1 << 29)|(tmp&0xff);
	writel(tmp,CPM_BASE + CPM_SSICDR);


	//SSI disable.
	cr0 = readl(SSI0_BASE + SSI_CR0);
	cr0 &= ~SSI_CR0_SSIE;
	writel(cr0,SSI0_BASE + SSI_CR0);
	//设置时钟频率2分频
	writel(0,SSI0_BASE + SSI_GR);
	//清除FIFO
	cr0 = SSI_CR0_EACLRUN | SSI_CR0_RFLUSH | SSI_CR0_TFLUSH;
	writel(cr0,SSI0_BASE+SSI_CR0);
	//使用连续模式
	writel(0,SSI0_BASE + SSI_ITR);

	tmp = SSI_FRMHL_CE0_LOW_CE1_LOW | SSI_GPCMD | SSI_GPCHL_HIGH |  SSI_CR1_FLEN_8BIT | SSI_CR1_TFVCK_1;
	writel(tmp,SSI0_BASE+SSI_CR1);
}

void spi_enable()
{
	unsigned int cr0;
	cr0 = readl(SSI0_BASE + SSI_CR0);
	cr0 |= SSI_CR0_SSIE;
	writel(cr0,SSI0_BASE + SSI_CR0);
}

int spi_send(unsigned char *d,unsigned int count)
{
	while(--count)
		writel(*d++,SSI0_BASE + SSI_DR);
	return count;
}

#if 0
static void shift_spi_data(unsigned char data)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		// 设置数据
		gpio_direction_output_dir(GPIO_BIT_NO_STB, (data >> (7 - i)) & 1);
		// 翻转时钟
		gpio_direction_output_dir(GPIO_BIT_NO_CLK, 1);
		gpio_direction_output_dir(GPIO_BIT_NO_CLK, 0);

	}
}
#endif

static int spi_is_finish()
{
	return readl(SSI0_BASE + SSI_SR) & SSI_SR_END;
}

/* size in byte */
int spi_trans_data(unsigned char *data, int size)
{
	int i;
	unsigned char *c;

	//printf("%s(): %x\n", __func__, *data);
	//while(!spi_is_finish());

	c = (unsigned char*)data;
	for (i = 0; i < size; i++)
	{
		writel(c[i],SSI0_BASE + SSI_DR);
	}
	spi_enable();
//	while(!spi_is_finish());
	return size;
}

