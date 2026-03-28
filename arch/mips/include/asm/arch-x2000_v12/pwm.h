/*
 * JZ4775 TCU definitions
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
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

#ifndef __PWM_H__
#define __PWM_H__

#include <asm/arch/base.h>

enum pwm_clk{
	EXTAL,
	RTCCLK,
	PCLK,
};

enum pwm_mode{
	CPU_MODE,
	DMA_MODE,
};

struct pwm {
	int channels;
	int div;
	enum pwm_clk pwm_clock;
	int full_data;
	int half_data;
};

/*struct pwm pwm_backlight;*/

/*************************************************************************
 * PWM (Timer Counter Unit)
 *************************************************************************/
#define GENMASK(hi, lo)   (((1ULL << ((hi) - (lo) + 1)) - 1) << (lo))
#define PWM_CCFG0       (0x00)
#define PWM_CCFG1       (0x04)
#define PWM_ENS         (0x10)
#define PWM_ENC         (0x14)
#define PWM_EN          (0x18)
#define PWM_UPT         (0x20)
#define PWM_BUSY        (0x24)
#define PWM_INITR       (0x30)
#define PWM_WCFG0       (0xb0)
#define PWM_DES         (0x100)
#define PWM_DEC         (0x104)
#define PWM_DE          (0x108)
#define PWM_MS          (0x110)
#define PWM_DCR1        (0x114)
#define PWM_DTRIG       (0x120)
#define PWM_DFER        (0x124)
#define PWM_DFSM        (0x128)
#define PWM_DSR         (0x130)
#define PWM_DSCR        (0x134)
#define PWM_DINTC       (0x138)
#define PWM_DMADDR      (0x140)
#define PWM_DTLR        (0x190)
#define PWM_OEN         (0x300)
#define PWM_WCFG(n)     ((PWM_WCFG0) + 4*n)

#define PWM_WCFG_HIGH		(16)
#define PWM_WCFG_LOW		(0)

#define PWM_STORE_SE	BIT(18)
#define PWM_STORE_SPE	BIT(17)
#define PWM_STORE_SNE	BIT(16)
#define PWM_STORE_SFN_LBIT	0
#define PWM_STORE_SFN_HBIT	9
#define PWM_STORE_SFN_MASK	\
	GENMASK(PWM_STORE_SFN_HBIT, PWM_STORE_SFN_LBIT)

int pwm_request(int num);
void pwm_enable(int num);
void pwm_disable(int num);
void pwm_set_init_level(int num,int value);
void pwm_set_finish_level(int num,int value);
void pwm_config(int num,int div,int full_data,int half_data);
void pwm_init(struct pwm *pwm_data);

#endif /* __PWM_H__ */
