/*
 * X2000 GPIO definitions
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

#ifndef __GPIO_H__
#define __GPIO_H__

#include <asm/arch/base.h>

#define GPIO_PA(n) 	(0*32 + n)
#define GPIO_PB(n) 	(1*32 + n)
#define GPIO_PC(n) 	(2*32 + n)
#define GPIO_PD(n) 	(3*32 + n)
#define GPIO_PE(n) 	(4*32 + n)
#define GPIO_PF(n) 	(4*32 + n)

enum gpio_function {
	GPIO_FUNC_0     = 0x00,  //0000, GPIO as function 0 / device 0
	GPIO_FUNC_1     = 0x01,  //0001, GPIO as function 1 / device 1
	GPIO_FUNC_2     = 0x02,  //0010, GPIO as function 2 / device 2
	GPIO_FUNC_3     = 0x03,  //0011, GPIO as function 3 / device 3
        GPIO_OUTPUT0    = 0x04,  //0100, GPIO output low  level
        GPIO_OUTPUT1    = 0x05,  //0101, GPIO output high level
	GPIO_INPUT	= 0x06,	 //0110, GPIO as input
	GPIO_RISE_EDGE  = 0x0b,	 //1011, GPIO as rise edge interrupt
	GPIO_PULL	= 0x10,  //
};

enum gpio_port {
	GPIO_PORT_A,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	/* this must be last */
	GPIO_NR_PORTS,
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
#define PXDG 		0x70   /* PIN  Dual-Edge Interrupt Register */
#define PXDGS		0x74   /* Port Dual-Edge Interrupt Set Register */
#define PXDGC		0x78   /* Port Dual-Edge Interrupt Clear Register */
#define PXPU 		0x80   /* Port IPULL-UP enabled State Register */
#define PXPUS		0x84   /* Port IPULL-UP enabled State Set Register */
#define PXPUC		0x88   /* Port IPULL-UP enabled State Clear Register */
#define PXPD 		0x90   /* Port IPULL-Down enabled State Register */
#define PXPDS		0x94   /* Port IPULL-Down enabled State Set Register */
#define PXPDC		0x98   /* Port IPULL-Down enabled State Clear Register */
#define PXDS0		0xA0   /* Port Drive Strength Register0 */
#define PXDS0S		0xA4   /* Port Drive Strength Set Register0 */
#define PXDS0C		0xA8   /* Port Drive Strength Clear Register0 */
#define PXDS1		0xB0   /* Port Drive Strength Register1 */
#define PXDS1S		0xB4   /* Port Drive Strength Set Register1 */
#define PXDS1C		0xB8   /* Port Drive Strength Clear Register1 */
#define PXSR		0xD0   /* Port Slew Rate Register */
#define PXSRS		0xD4   /* Port Slew Rate Set Register */
#define PXSRC		0xD8   /* Port PORTA Slew Rate Clear Register */
#define PXSMT 		0xE0   /* Port Schmitt Trigger Register*/
#define PXSMTS		0xE4   /* Port Schmitt Trigger Set Register */
#define PXSMTC		0xE8   /* Port Schmitt Trigger Clear Register */

#define GPIO_PXPIN(n)	(GPIO_BASE + (PXPIN + (n)*0x100)) /* PIN Level Register */
#define GPIO_PXINT(n)	(GPIO_BASE + (PXINT + (n)*0x100)) /* Port Interrupt Register */
#define GPIO_PXINTS(n)	(GPIO_BASE + (PXINTS + (n)*0x100)) /* Port Interrupt Set Register */
#define GPIO_PXINTC(n)	(GPIO_BASE + (PXINTC + (n)*0x100)) /* Port Interrupt Clear Register */
#define GPIO_PXMSK(n)	(GPIO_BASE + (PXMSK + (n)*0x100)) /* Port Interrupt Mask Register */
#define GPIO_PXMSKS(n)	(GPIO_BASE + (PXMSKS + (n)*0x100)) /* Port Interrupt Mask Set Reg */
#define GPIO_PXMSKC(n)	(GPIO_BASE + (PXMSKC + (n)*0x100)) /* Port Interrupt Mask Clear Reg */
#define GPIO_PXPAT1(n)	(GPIO_BASE + (PXPAT1 + (n)*0x100)) /* Port Pattern 1 Register */
#define GPIO_PXPAT1S(n)	(GPIO_BASE + (PXPAT1S + (n)*0x100)) /* Port Pattern 1 Set Reg. */
#define GPIO_PXPAT1C(n)	(GPIO_BASE + (PXPAT1C + (n)*0x100)) /* Port Pattern 1 Clear Reg. */
#define GPIO_PXPAT0(n)	(GPIO_BASE + (PXPAT0 + (n)*0x100)) /* Port Pattern 0 Register */
#define GPIO_PXPAT0S(n)	(GPIO_BASE + (PXPAT0S + (n)*0x100)) /* Port Pattern 0 Set Register */
#define GPIO_PXPAT0C(n)	(GPIO_BASE + (PXPAT0C + (n)*0x100)) /* Port Pattern 0 Clear Register */
#define GPIO_PXFLG(n)	(GPIO_BASE + (PXFLG + (n)*0x100)) /* Port Flag Register */
#define GPIO_PXFLGC(n)	(GPIO_BASE + (PXFLGC + (n)*0x100)) /* Port Flag clear Register */
#define GPIO_PXDG(n)	(GPIO_BASE + (PXDG   + (n)*0x100))/* PIN  Dual-Edge Interrupt Register */
#define GPIO_PXDGS(n)	(GPIO_BASE + (PXDGS  + (n)*0x100))/* Port Dual-Edge Interrupt Set Register */
#define GPIO_PXDGC(n)	(GPIO_BASE + (PXDGC  + (n)*0x100))/* Port Dual-Edge Interrupt Clear Register */
#define GPIO_PXPU(n)	(GPIO_BASE + (PXPU   + (n)*0x100))/* Port IPULL-UP enabled State Register */
#define GPIO_PXPUS(n)	(GPIO_BASE + (PXPUS  + (n)*0x100))/* Port IPULL-UP enabled State Set Register */
#define GPIO_PXPUC(n)	(GPIO_BASE + (PXPUC  + (n)*0x100))/* Port IPULL-UP enabled State Clear Register */
#define GPIO_PXPD(n)	(GPIO_BASE + (PXPD   + (n)*0x100))/* Port IPULL-Down enabled State Register */
#define GPIO_PXPDS(n)	(GPIO_BASE + (PXPDS  + (n)*0x100))/* Port IPULL-Down enabled State Set Register */
#define GPIO_PXPDC(n)	(GPIO_BASE + (PXPDC  + (n)*0x100))/* Port IPULL-Down enabled State Clear Register */
#define GPIO_PXDS0(n)	(GPIO_BASE + (PXDS0  + (n)*0x100))/* Port Drive Strength Register0 */
#define GPIO_PXDS0S(n)	(GPIO_BASE + (PXDS0S + (n)*0x100))/* Port Drive Strength Set Register0 */
#define GPIO_PXDS0C(n)	(GPIO_BASE + (PXDS0C + (n)*0x100))/* Port Drive Strength Clear Register0 */
#define GPIO_PXDS1(n)	(GPIO_BASE + (PXDS1  + (n)*0x100))/* Port Drive Strength Register1 */
#define GPIO_PXDS1S(n)	(GPIO_BASE + (PXDS1S + (n)*0x100))/* Port Drive Strength Set Register1 */
#define GPIO_PXDS1C(n)	(GPIO_BASE + (PXDS1C + (n)*0x100))/* Port Drive Strength Clear Register1 */
#define GPIO_PXSR(n)	(GPIO_BASE + (PXSR   + (n)*0x100))/* Port Slew Rate Register */
#define GPIO_PXSRS(n)	(GPIO_BASE + (PXSRS  + (n)*0x100))/* Port Slew Rate Set Register */
#define GPIO_PXSRC(n)	(GPIO_BASE + (PXSRC  + (n)*0x100))/* Port PORTA Slew Rate Clear Register */
#define GPIO_PXSMT(n)	(GPIO_BASE + (PXSMT  + (n)*0x100))/* Port Schmitt Trigger Register*/
#define GPIO_PXSMTS(n)	(GPIO_BASE + (PXSMTS + (n)*0x100))/* Port Schmitt Trigger Set Register */
#define GPIO_PXSMTC(n)	(GPIO_BASE + (PXSMTC + (n)*0x100))/* Port Schmitt Trigger Clear Register */

void gpio_set_func(enum gpio_port n, enum gpio_function func, unsigned int pins);
void gpio_port_set_value(int port, int pin, int value);
void gpio_port_direction_input(int port, int pin);
void gpio_port_direction_output(int port, int pin, int value);
void gpio_init(void);
void gpio_enable_pull(unsigned gpio);
void gpio_disable_pull(unsigned gpio);
void gpio_as_irq_high_level(unsigned gpio);
void gpio_as_irq_low_level(unsigned gpio);
void gpio_as_irq_rise_edge(unsigned gpio);
void gpio_as_irq_fall_edge(unsigned gpio);
void gpio_ack_irq(unsigned gpio);
int gpio_clear_flag(unsigned gpio);
int gpio_get_flag(unsigned int gpio);

#endif /* __GPIO_H__ */
