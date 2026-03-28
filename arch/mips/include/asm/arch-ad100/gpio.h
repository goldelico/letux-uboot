/*
 * ad100 GPIO definitions
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

#define GPIO_PA(n)     (0*32 + n)
#define GPIO_PB(n)     (1*32 + n)
#define GPIO_PC(n)     (2*32 + n)
#define GPIO_PD(n)     (3*32 + n)
#define GPIO_PE(n)     (4*32 + n)
#define GPIO_PF(n)     (4*32 + n)

enum gpio_function {
    GPIO_FUNC_0     = 0x00,  //0000, GPIO as function 0 / device 0
    GPIO_FUNC_1     = 0x01,  //0001, GPIO as function 1 / device 1
    GPIO_FUNC_2     = 0x02,  //0010, GPIO as function 2 / device 2
    GPIO_FUNC_3     = 0x03,  //0011, GPIO as function 3 / device 3
    GPIO_OUTPUT0    = 0x04,  //0100, GPIO output low  level
    GPIO_OUTPUT1    = 0x05,  //0101, GPIO output high level
    GPIO_INPUT      = 0x06,  //0110, GPIO as input
    GPIO_RISE_EDGE  = 0x0b,  //1011, GPIO as rise edge interrupt
    GPIO_PULLUP     = 0x10,  //    enable gpio pull up
    GPIO_PULLDOWN   = 0x20,  // enable gpio pull down
    GPIO_PULL_HIZ   = 0x40,  // enable gpio no pull
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

enum gpio_driver_strength {
    GPIO_DS_LEVEL_INVALID = -1,

    GPIO_DS_LEVEL_0 = 0x0,
    GPIO_DS_LEVEL_1 = 0x1,
    GPIO_DS_LEVEL_2 = 0x2,
    GPIO_DS_LEVEL_3 = 0x3,
};

struct jz_gpio_func_def {
    int port;
    int func;
    unsigned long pins;
};

/*************************************************************************
 * GPIO (General-Purpose I/O Ports)
 *************************************************************************/
#define MAX_GPIO_NUM    192

#define PXPIN        0x00   /* PIN Level Register */
#define PXINT        0x10   /* Port Interrupt Register */
#define PXINTS       0x14   /* Port Interrupt Set Register */
#define PXINTC       0x18   /* Port Interrupt Clear Register */
#define PXMSK        0x20   /* Port Interrupt Mask Reg */
#define PXMSKS       0x24   /* Port Interrupt Mask Set Reg */
#define PXMSKC       0x28   /* Port Interrupt Mask Clear Reg */
#define PXPAT1       0x30   /* Port Pattern 1 Set Reg. */
#define PXPAT1S      0x34   /* Port Pattern 1 Set Reg. */
#define PXPAT1C      0x38   /* Port Pattern 1 Clear Reg. */
#define PXPAT0       0x40   /* Port Pattern 0 Register */
#define PXPAT0S      0x44   /* Port Pattern 0 Set Register */
#define PXPAT0C      0x48   /* Port Pattern 0 Clear Register */
#define PXFLG        0x50   /* Port Flag Register */
#define PXFLGC       0x58   /* Port Flag clear Register */

#define PXPU         0x80   /* Port PULL-UP enabled State Register */
#define PXPUS        0x84   /* Port PULL-UP enabled State Set Register */
#define PXPUC        0x88   /* Port PULL-UP enabled State Clear Register */
#define PXPD         0x90   /* Port PULL-DOWN enabled State Register */
#define PXPDS        0x94   /* Port PULL-DOWN enabled State Set Register */
#define PXPDC        0x98   /* Port PULL-DOWN enabled State Clear Register */

#define PEPU         0x80   /* Port PULL State Register */
#define PEPUS        0x84   /* Port PULL State Set Register */
#define PEPUC        0x88   /* Port PULL State Clear Register */
#define PEPS         0x90   /* Port PULL State Select Register */
#define PEPSS        0x94   /* Port PULL State Select Set Register */
#define PEPSC        0x98   /* Port PULL State SelectClear Register */

#define PXDS0        0xA0   /* PORT Drive Strength State Register0*/
#define PXDS0S       0xA4   /* PORT Drive Strength State set Register0*/
#define PXDS0C       0xA8   /* PORT Drive Strength State clear Register0*/
#define PXDS1        0xB0   /* PORT Drive Strength State Register1*/
#define PXDS1S       0xB4   /* PORT Drive Strength State set Register1*/
#define PXDS1C       0xB8   /* PORT Drive Strength State clear Register1*/

#define GPIO_PXPIN(n)     (GPIO_BASE + (PXPIN + (n)*0x1000)) /* PIN Level Register */
#define GPIO_PXINT(n)     (GPIO_BASE + (PXINT + (n)*0x1000)) /* Port Interrupt Register */
#define GPIO_PXINTS(n)    (GPIO_BASE + (PXINTS + (n)*0x1000)) /* Port Interrupt Set Register */
#define GPIO_PXINTC(n)    (GPIO_BASE + (PXINTC + (n)*0x1000)) /* Port Interrupt Clear Register */
#define GPIO_PXMSK(n)     (GPIO_BASE + (PXMSK + (n)*0x1000)) /* Port Interrupt Mask Register */
#define GPIO_PXMSKS(n)    (GPIO_BASE + (PXMSKS + (n)*0x1000)) /* Port Interrupt Mask Set Reg */
#define GPIO_PXMSKC(n)    (GPIO_BASE + (PXMSKC + (n)*0x1000)) /* Port Interrupt Mask Clear Reg */
#define GPIO_PXPAT1(n)    (GPIO_BASE + (PXPAT1 + (n)*0x1000)) /* Port Pattern 1 Register */
#define GPIO_PXPAT1S(n)   (GPIO_BASE + (PXPAT1S + (n)*0x1000)) /* Port Pattern 1 Set Reg. */
#define GPIO_PXPAT1C(n)   (GPIO_BASE + (PXPAT1C + (n)*0x1000)) /* Port Pattern 1 Clear Reg. */
#define GPIO_PXPAT0(n)    (GPIO_BASE + (PXPAT0 + (n)*0x1000)) /* Port Pattern 0 Register */
#define GPIO_PXPAT0S(n)   (GPIO_BASE + (PXPAT0S + (n)*0x1000)) /* Port Pattern 0 Set Register */
#define GPIO_PXPAT0C(n)   (GPIO_BASE + (PXPAT0C + (n)*0x1000)) /* Port Pattern 0 Clear Register */
#define GPIO_PXFLG(n)     (GPIO_BASE + (PXFLG + (n)*0x1000)) /* Port Flag Register */
#define GPIO_PXFLGC(n)    (GPIO_BASE + (PXFLGC + (n)*0x1000)) /* Port Flag clear Register */

#define GPIO_PXDS0(n)     (GPIO_BASE + (PXDS0 + (n)*0x1000))  /* PORT Drive Strength State Register0*/
#define GPIO_PXDS0S(n)    (GPIO_BASE + (PXDS0S + (n)*0x1000)) /* PORT Drive Strength State set Register0*/
#define GPIO_PXDS0C(n)    (GPIO_BASE + (PXDS0C + (n)*0x1000)) /* PORT Drive Strength State clear Register0*/
#define GPIO_PXDS1(n)     (GPIO_BASE + (PXDS1 + (n)*0x1000))  /* PORT Drive Strength State Register1*/
#define GPIO_PXDS1S(n)    (GPIO_BASE + (PXDS1S + (n)*0x1000)) /* PORT Drive Strength State set Register1*/
#define GPIO_PXDS1C(n)    (GPIO_BASE + (PXDS1C + (n)*0x1000)) /* PORT Drive Strength State clear Register1*/

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
void gpio_set_driver_strength(enum gpio_port gpio, int value, unsigned int pins);

#endif /* __GPIO_H__ */
