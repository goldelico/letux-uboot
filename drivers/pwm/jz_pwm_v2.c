/*
 * Copyright (C) 2013 Ingenic Semiconductor Inc.
 * Author: Huddy <hyli@ingenic.cn>
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

#include <asm/arch/pwm.h>
#include <config.h>
#include <asm/io.h>
#include <common.h>
#include <asm/arch/clk.h>

char pwm_flag = 0;

/* Request a channel:
 * return 1:this channel is using
 * return 0:this channel is free*/
int pwm_readl(unsigned int offset){
	return readl(PWM_IOBASE + offset);
}

void pwm_writel(unsigned int value, unsigned int offset){
	writel(value, PWM_IOBASE + offset);
}

int pwm_request(int num){
	if (pwm_flag & (1 << num))
		return 1;
	else
		return 0;
}

void pwm_enable(int num)
{
	if (!pwm_request(num)){
		pwm_writel((1 << num), PWM_ENS);
		printf("pwm_en =0x%x\n",pwm_readl(PWM_EN));
		pwm_flag |= 1 << num;
	} else {
		printf("the channel is using!\n");
		return ;
	}
}

void pwm_disable(int num)
{
	if (pwm_flag & (1 << num)){
			pwm_writel((1 << num), PWM_ENC);
	}
	pwm_flag &= (~(1 << num));
}

void pwm_set_init_level(int num, int value)
{
#ifdef CONFIG_X2000_V12
	pwm_writel(value, PWM_INITR);
#else
	pwm_writel(value, PWM_INL);
#endif
}

void pwm_set_finish_level(int num, int value)
{
#ifdef CONFIG_X2000_V12
	pwm_writel(value,PWM_INITR <<16);
#else
	pwm_writel(value, PWM_IDL);
#endif
}

void pwm_config(int num, int div, int full_data, int half_data)
{
	unsigned int val;

	val = (half_data << 16) | (full_data - half_data);
	//PRESCALE
#ifdef CONFIG_X2000_V12
	{
		unsigned int n = num % 8;
		unsigned int div_ch = div << (4 * n);
		unsigned int reg = (num / 8 *4) +PWM_CCFG0;
		unsigned int divreg = pwm_readl(reg);
		divreg &= ~(0xf << (4 * n));
		divreg |= div << 4 * n;
			pwm_writel(divreg,reg);
	}
#else
	pwm_writel(div, PWM_CCFG(num));
#endif
	//cpu mode
	pwm_writel(CPU_MODE << num, PWM_MS);
	//duty period
	pwm_writel(val, PWM_WCFG(num));
}

void pwm_init(struct pwm *pwm_data)
{
	unsigned int rate;

	rate = clk_get_rate(PWM);
	clk_set_rate(PWM, rate);

	if (!pwm_request(pwm_data->channels)){
		pwm_disable(pwm_data->channels);
		pwm_set_init_level(pwm_data->channels, 0);
		pwm_set_finish_level(pwm_data->channels, 0);
		pwm_config(pwm_data->channels,
			pwm_data->div,
			pwm_data->full_data,
			pwm_data->half_data);
		pwm_enable(pwm_data->channels);
	//printf("channel = %d  div =%d full_data =%d  half_data =%d\n",pwm_data->channels,pwm_data->div,pwm_data->full_data,pwm_data->half_data);
	} else {
		printf("the channel is using!\n");
	}
}
