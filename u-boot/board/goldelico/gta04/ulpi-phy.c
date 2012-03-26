/* u-boot driver to access ULPI PHY registers through EHCI controller
 *
 * Copyright (C) 2010 by Golden Delicious Computers GmbH&Co. KG
 * Author: H. Nikolaus Schaller <hns@goldelico.com>
 * All rights reserved.
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
 *
 */

/* FIXME 
 * needs to enable clock USBHOST_FCLK etc.
 */

#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include "ulpi-phy.h"

#define EHCI_BASE			(0x480648A4)

#define	EHCI_INSNREG05_ULPI				(0xA4)
#define	EHCI_INSNREG05_ULPI_CONTROL_SHIFT		31
#define	EHCI_INSNREG05_ULPI_PORTSEL_SHIFT		24
#define	EHCI_INSNREG05_ULPI_OPSEL_SHIFT			22
#define	EHCI_INSNREG05_ULPI_REGADD_SHIFT		16
#define	EHCI_INSNREG05_ULPI_EXTREGADD_SHIFT		8
#define	EHCI_INSNREG05_ULPI_WRDATA_SHIFT		0

/* read/write Registers in ULPI-PHY */

u8 ulpi_direct_access(u8 port, u8 reg, int write, u8 value)
{
#if 0	// FIXME: we need to have the clocks and the EHCI controller enabled!
	u32 val;
	u32 cmd = (((u32)value) << EHCI_INSNREG05_ULPI_WRDATA_SHIFT); /* value to write */
	if(reg > 0x3f || reg == 0x2f)
		cmd |= (((u32)reg & 0xff) << EHCI_INSNREG05_ULPI_EXTREGADD_SHIFT) | (0x2f << EHCI_INSNREG05_ULPI_REGADD_SHIFT);	/* extended address */
	else
		cmd |= (((u32)reg & 0x3f) << EHCI_INSNREG05_ULPI_REGADD_SHIFT);
	cmd |= ((write? 0x02 : 0x03) << EHCI_INSNREG05_ULPI_OPSEL_SHIFT);
	cmd |= (((u32)(port+1) & 3) << EHCI_INSNREG05_ULPI_PORTSEL_SHIFT);	/* port counts 0,1,2 */
	cmd |= (1 << EHCI_INSNREG05_ULPI_CONTROL_SHIFT);	/* Start */
	
	writel(cmd, EHCI_BASE + EHCI_INSNREG05_ULPI);	/* write EHCI_INSNREG05_ULPI */
	
	// do {
	udelay(100);	/* wait a little */
	val = readl(EHCI_BASE + EHCI_INSNREG05_ULPI);	/* read EHCI_INSNREG05_ULPI */
	/* do we need a timepout if the PHY chip is not present? */
	// } while(!(val & (1<<EHCI_INSNREG05_ULPI_CONTROL_SHIFT)));
	return val;
#else
	return 0;
#endif
}
