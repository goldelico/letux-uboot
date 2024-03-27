#ifndef __ADC_H__
#define __ADC_H__

#define SADC_IOBASE	0xb0070000
#define SADC_ADENA	(0x00)    /* ADC Enable register */
#define SADC_ADCFG	(0x04)    /* Sadc's PHY testmode register */
#define SADC_ADCTRL	(0x08)    /* ADC Control register */
#define SADC_ADSTATE	(0x0c)    /* ADC Status register */
#define SADC_ADATA0	(0x10)    /* ADC AUX Data register */
#define SADC_ADATA1	(0x14)    /* ADC AUX Data register */
#define SADC_ADATA2	(0x18)    /* ADC AUX Data register */
#define SADC_ADCLK	(0x20)    /* ADC Clock Divide register */
#define SADC_ADSTB	(0x24)    /* ADC Stable Register */
#define SADC_ADRETM	(0x28)    /* ADC REPEAT TIME register */

#define ADC_REF_VOL   (1800) // 1.8V

static inline unsigned int read_reg(unsigned int reg)
{
	return readl(SADC_IOBASE+reg);
}

static inline unsigned int write_reg(unsigned int v, unsigned int reg)
{
	return writel(v, SADC_IOBASE+reg);
}

void adc_init(void);
int read_adc_channel(int ch);
int get_adc_channel_voltage(int ch);

#endif
