/*
 * X2580 GPIO definitions
 * Copyright (c) 2019 Ingenic Semiconductor Co.,Ltd
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

#ifndef __GPIO_H__
#define __GPIO_H__

#include <asm/arch/base.h>

#define GPIO_PA(n) 	(0*32 + n)
#define GPIO_PB(n) 	(1*32 + n)
#define GPIO_PC(n) 	(2*32 + n)
#define GPIO_PD(n) 	(3*32 + n)
#define GPIO_PE(n) 	(4*32 + n)
#define GPIO_PF(n) 	(5*32 + n)
#define GPIO_PG(n) 	(6*32 + n)

enum gpio_function {
	GPIO_FUNC_0     = 0x00,  //0000, GPIO as function 0 / device 0
	GPIO_FUNC_1     = 0x01,  //0001, GPIO as function 1 / device 1
	GPIO_FUNC_2     = 0x02,  //0010, GPIO as function 2 / device 2
	GPIO_FUNC_3     = 0x03,  //0011, GPIO as function 3 / device 3
	GPIO_OUTPUT0    = 0x04,  //0100, GPIO output low  level
	GPIO_OUTPUT1    = 0x05,  //0101, GPIO output high level
	GPIO_INPUT      = 0x06,	 //0110, GPIO as input
	GPIO_PULL	    = 0x10,
	GPIO_PULL_DOWN  = 0X20,
	GPIO_PULL_HIZ   = 0x40,
};

enum gpio_port {
	GPIO_PORT_A,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
	/* this must be last */
	GPIO_NR_PORTS,
};

/* TYPEC and TYPED are not supported in setting driver strength */
enum gpio_driver_strength {
	GPIO_DS_LEVEL_INVALID = -1,

	GPIO_DS_LEVEL_0 = 0x0,
	GPIO_DS_LEVEL_1 = 0x1,
	GPIO_DS_LEVEL_2 = 0x2,
	GPIO_DS_LEVEL_3 = 0x3,

	/* GPIO_A 6~21 set valid */
	GPIO_DS_LEVEL_4 = 0x4,
	GPIO_DS_LEVEL_5 = 0x5,
	GPIO_DS_LEVEL_6 = 0x6,
	GPIO_DS_LEVEL_7 = 0x7,
	GPIO_DS_LEVEL_8 = 0x8,
	GPIO_DS_LEVEL_9 = 0x9,
	GPIO_DS_LEVEL_10 = 0xa,
	GPIO_DS_LEVEL_11 = 0xb,
	GPIO_DS_LEVEL_12 = 0xc,
	GPIO_DS_LEVEL_13 = 0xd,
	GPIO_DS_LEVEL_14 = 0xe,
	GPIO_DS_LEVEL_15 = 0xf,
};

struct jz_gpio_func_def {
	int port;
	int func;
	unsigned long pins;
};

/*************************************************************************
 * GPIO (General-Purpose I/O Ports)
 *************************************************************************/
#define MAX_GPIO_NUM	192

#define PXPIN		0x00   /* PIN Level Register */
#define PXINT		0x10   /* Port Interrupt Register */
#define PXINTS		0x14   /* Port Interrupt Set Register */
#define PXINTC		0x18   /* Port Interrupt Clear Register */
#define PXMSK		0x20   /* Port Interrupt Mask Reg */
#define PXMSKS		0x24   /* Port Interrupt Mask Set Reg */
#define PXMSKC		0x28   /* Port Interrupt Mask Clear Reg */
#define PXPAT1		0x30   /* Port Pattern 1 Set Reg. */
#define PXPAT1S		0x34   /* Port Pattern 1 Set Reg. */
#define PXPAT1C		0x38   /* Port Pattern 1 Clear Reg. */
#define PXPAT0		0x40   /* Port Pattern 0 Register */
#define PXPAT0S		0x44   /* Port Pattern 0 Set Register */
#define PXPAT0C		0x48   /* Port Pattern 0 Clear Register */
#define PXFLG		0x50   /* Port Flag Register */
#define PXFLGC		0x58   /* Port Flag clear Register */


#define PXGFCFG0	0x70   /* Port Glitch Filter Configure 0 Register */
#define PXGFCFG0S	0x74   /* Port Glitch Filter Configure 0 Set Register */
#define PXGFCFG0C	0x78   /* Port Glitch Filter Configure 0 Clear Register */
#define PXGFCFG1	0x80   /* Port Glitch Filter Configure 1 Register */
#define PXGFCFG1S	0x84   /* Port Glitch Filter Configure 1 Set Register */
#define PXGFCFG1C	0x88   /* Port Glitch Filter Configure 1 Clear Register */
#define PXGFCFG2	0x90   /* Port Glitch Filter Configure 2 Register */
#define PXGFCFG2S	0x94   /* Port Glitch Filter Configure 2 Set Register */
#define PXGFCFG2C	0x98   /* Port Glitch Filter Configure 2 Clear Register */
#define PXGFCFG3	0xa0   /* Port Glitch Filter Configure 3 Register */
#define PXGFCFG3S	0xa4   /* Port Glitch Filter Configure 3 Set Register */
#define PXGFCFG3C	0xa8   /* Port Glitch Filter Configure 3 Clear Register */


#define PXGPDCR		0x100   /* Port IO Power Domain control Register */
#define PXGPDCRS	0x104   /* Port IO Power Domain control Set Register */
#define PXGPDCRC	0x108   /* Port IO Power Domain control Clear Register */
#define PXPU		0x110   /* Port Pull-up status Register */
#define PXPUS		0x114   /* Port Pull-up status Set Register */
#define PXPUC		0x118   /* Port Pull-up status Clear Register */
#define PXPD		0x120   /* Port Pull-down status Register */
#define PXPDS		0x124   /* Port Pull-down status Set Register */
#define PXPDC		0x128   /* Port Pull-down status Clear Register */
#define PXPDS0		0x130   /* Port Driver-strength 0  Register */
#define PXPDS0S		0x134   /* Port Driver-strength 0 Set Register */
#define PXPDS0C		0x138   /* Port Driver-strength 0 Clear Register */
#define PXPDS1		0x140   /* Port Driver-strength 1  Register */
#define PXPDS1S		0x144   /* Port Driver-strength 1 Set Register */
#define PXPDS1C		0x148   /* Port Driver-strength 1 Clear Register */
#define PXPDS2		0x150   /* Port Driver-strength 2  Register */
#define PXPDS2S		0x154   /* Port Driver-strength 2 Set Register */
#define PXPDS2C		0x158   /* Port Driver-strength 2 Clear Register */
#define PXPDS3	   	0x1A0   /* PORT Drive Strength State Register3*/
#define PXPDS3S	   	0x1A4   /* PORT Drive Strength State set Register3*/
#define PXPDS3C	   	0x1A8   /* PORT Drive Strength State clear Register3*/
#define PXPSLW		0x160   /* Port Slew Rate Register */
#define PXPSLWS		0x164   /* Port Slew Rate Set Register */
#define PXPSLWC		0x168   /* Port Slew Rate Clear Register */
#define PXPSMT		0x170   /* Port Schmitt Trigger Register */
#define PXPSMTS		0x174   /* Port Schmitt Trigger Set Register */
#define PXPSMTC		0x178   /* Port Schmitt Trigger Clear Register */

/* Only PC use */
#define PXPSMT1		0x180   /* Port Schmitt 1 Trigger Register */
#define PXPSMT1S	0x184   /* Port Schmitt 1 Trigger Set Register */
#define PXPSMT1C	0x188   /* Port Schmitt 1 Trigger Clear Register */
#define PXPHE		0x190   /* Port Hold Enable Register */
#define PXPHES		0x194   /* Port Hold Enable Set Register */
#define PXPHEC		0x198   /* Port Hold Enable Clear Register */

#define PZGID2LD       0xF0    /* GPIOZ Group ID to load */

#define GPIO_PXPIN(n)	(GPIO_BASE + (PXPIN + (n)*0x1000))     /* PIN Level Register */
#define GPIO_PXINT(n)	(GPIO_BASE + (PXINT + (n)*0x1000))     /* Port Interrupt Register */
#define GPIO_PXINTS(n)	(GPIO_BASE + (PXINTS + (n)*0x1000))    /* Port Interrupt Set Register */
#define GPIO_PXINTC(n)	(GPIO_BASE + (PXINTC + (n)*0x1000))    /* Port Interrupt Clear Register */
#define GPIO_PXMSK(n)	(GPIO_BASE + (PXMSK + (n)*0x1000))     /* Port Interrupt Mask Register */
#define GPIO_PXMSKS(n)	(GPIO_BASE + (PXMSKS + (n)*0x1000))    /* Port Interrupt Mask Set Reg */
#define GPIO_PXMSKC(n)	(GPIO_BASE + (PXMSKC + (n)*0x1000))    /* Port Interrupt Mask Clear Reg */
#define GPIO_PXPAT1(n)	(GPIO_BASE + (PXPAT1 + (n)*0x1000))    /* Port Pattern 1 Register */
#define GPIO_PXPAT1S(n)	(GPIO_BASE + (PXPAT1S + (n)*0x1000))   /* Port Pattern 1 Set Reg. */
#define GPIO_PXPAT1C(n)	(GPIO_BASE + (PXPAT1C + (n)*0x1000))   /* Port Pattern 1 Clear Reg. */
#define GPIO_PXPAT0(n)	(GPIO_BASE + (PXPAT0 + (n)*0x1000))    /* Port Pattern 0 Register */
#define GPIO_PXPAT0S(n)	(GPIO_BASE + (PXPAT0S + (n)*0x1000))   /* Port Pattern 0 Set Register */
#define GPIO_PXPAT0C(n)	(GPIO_BASE + (PXPAT0C + (n)*0x1000))   /* Port Pattern 0 Clear Register */
#define GPIO_PXFLG(n)	(GPIO_BASE + (PXFLG + (n)*0x1000))     /* Port Flag Register */
#define GPIO_PXFLGC(n)	(GPIO_BASE + (PXFLGC + (n)*0x1000))    /* Port Flag clear Register */
#define GPIO_PXPUEN(n)	(GPIO_BASE + (PXPUEN + (n)*0x1000))    /* Port Pull-up status Register */
#define GPIO_PXPUENS(n)	(GPIO_BASE + (PXPUENS + (n)*0x1000))   /* Port Pull-up status Set Register */
#define GPIO_PXPUENC(n)	(GPIO_BASE + (PXPUENC + (n)*0x1000))   /* Port Pull-up status Clear Register */
#define GPIO_PXPDEN(n) 	(GPIO_BASE + (PXPDEN + (n)*0x1000))    /* Port Pull-down status Register */
#define GPIO_PXPDENS(n)	(GPIO_BASE + (PXPDENS + (n)*0x1000))   /* Port Pull-down status Set Register */
#define GPIO_PXPDENC(n)	(GPIO_BASE + (PXPDENC + (n)*0x1000))   /* Port Pull-down status Clear Register */


#define GPIO_PXPDS0(n)	(GPIO_BASE + (PXPDS0 + (n)*0x1000))  /* PORT Drive Strength State Register0*/
#define GPIO_PXPDS0S(n)	(GPIO_BASE + (PXPDS0S + (n)*0x1000)) /* PORT Drive Strength State set Register0*/
#define GPIO_PXPDS0C(n)	(GPIO_BASE + (PXPDS0C + (n)*0x1000)) /* PORT Drive Strength State clear Register0*/
#define GPIO_PXPDS1(n)	(GPIO_BASE + (PXPDS1 + (n)*0x1000))  /* PORT Drive Strength State Register1*/
#define GPIO_PXPDS1S(n)	(GPIO_BASE + (PXPDS1S + (n)*0x1000)) /* PORT Drive Strength State set Register1*/
#define GPIO_PXPDS1C(n)	(GPIO_BASE + (PXPDS1C + (n)*0x1000)) /* PORT Drive Strength State clear Register1*/
#define GPIO_PXPDS2(n)	(GPIO_BASE + (PXPDS2 + (n)*0x1000))  /* PORT Drive Strength State Register2*/
#define GPIO_PXPDS2S(n)	(GPIO_BASE + (PXPDS2S + (n)*0x1000)) /* PORT Drive Strength State set Register2*/
#define GPIO_PXPDS2C(n)	(GPIO_BASE + (PXPDS2C + (n)*0x1000)) /* PORT Drive Strength State clear Register2*/
#define GPIO_PXPDS3(n)	(GPIO_BASE + (PXPDS3 + (n)*0x1000))  /* PORT Drive Strength State Register3*/
#define GPIO_PXPDS3S(n)	(GPIO_BASE + (PXPDS3S + (n)*0x1000)) /* PORT Drive Strength State set Register3*/
#define GPIO_PXPDS3C(n)	(GPIO_BASE + (PXPDS3C + (n)*0x1000)) /* PORT Drive Strength State clear Register3*/


void gpio_set_func(enum gpio_port n, enum gpio_function func, unsigned int pins);
void gpio_port_set_value(int port, int pin, int value);
void gpio_port_direction_input(int port, int pin);
void gpio_port_direction_output(int port, int pin, int value);
void gpio_init(void);
void gpio_enable_pull_up(unsigned gpio);
void gpio_disable_pull_up(unsigned gpio);
void gpio_enable_pull_down(unsigned gpio);
void gpio_disable_pull_down(unsigned gpio);
void gpio_as_irq_high_level(unsigned gpio);
void gpio_as_irq_low_level(unsigned gpio);
void gpio_as_irq_rise_edge(unsigned gpio);
void gpio_as_irq_fall_edge(unsigned gpio);
void gpio_ack_irq(unsigned gpio);
void gpio_set_driver_strength(enum gpio_port gpio, int value, unsigned int pins);

#endif /* __GPIO_H__ */
