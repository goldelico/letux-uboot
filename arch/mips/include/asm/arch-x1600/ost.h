/*
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

#ifndef __OST_H__
#define __OST_H__

#include <asm/arch/base.h>

#define OST_TCCR	(0x00)   /* OS Timer Clock Control Register*/
#define OST_TER     	(0x04)   /* OS Timer Counter Enable Register */
#define OST_TCR     	(0x08)   /* OS Timer clear Register */
#define OST_TFR     	(0x0C)   /* OS Timer Flag Register */
#define OST_TMR     	(0x10)   /* OS Timer Mask Register */
#define OST_T1DFR   	(0x14)   /* OS Timer1 Data FULL Register */
#define OST_T1CNT   	(0x18)   /* OS Timer1 Counter */
#define OST_T2CNTH  	(0x1c)   /* OS Timer2 Counter High 32 Bits */
#define OST_T2CNTL  	(0x20)   /* OS Timer2 Counter Lower 32 Bits */
#define OST_T2CNTB	(0x24)   /* OS Timer2 Counter Higher 32 Bits Buffer */
#define OST_TESR	(0x34)   /* OS Timer Counter Enable Set Register */
#define OST_TECR	(0x38)   /* OS Timer Counter Enable Clear Register */

#define TESR_OSTEN1   (1 << 0 )   /* enable the counter1 in ost */
#define TESR_OSTEN2   (1 << 1 )   /* enable the counter2 in ost */
#define TMR_OSTM    (1 << 0 )   /* ost comparison match interrupt mask */
#define TMR_OSTN    (0 << 0 )   /* ost comparison match interrupt no mask */
#define TFR_OSTM    (1 << 0)   /* Comparison match */
#define TFR_OSTN    (0 << 0)   /* Comparison not match */
#define TSR_OSTS    (1 << 15)   /*the clock supplies to osts is stopped */

#define OST2_EN				(1 << 1)
#define OST2CSR_PRESCALE_1		(0 << 2)
#define OST2CSR_PRESCALE_4		(1 << 2)
#define OST2CSR_PRESCALE_16		(2 << 2)

#define OST_DIV				4
#if (OST_DIV == 1)
#define OSTCSR_PRESCALE			OST2CSR_PRESCALE_1
#elif (OST_DIV == 4)
#define OSTCSR_PRESCALE			OST2CSR_PRESCALE_4
#elif (OST_DIV == 16)
#define OSTCSR_PRESCALE			OST2CSR_PRESCALE_16
#endif


#endif /* __OST_H__ */
