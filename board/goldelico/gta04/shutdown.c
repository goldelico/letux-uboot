/* u-boot driver for the GTA04 shutdown
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

#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include <twl4030.h>
#include "shutdown.h"

void shutdown(void)
{
	u8 val = 0;
	if(i2c_set_bus_num(0))
		{
		printf ("could not select I2C1\n");
		return;
		}
	printf("shutting down by writing DEVOFF register of TPS65950\n");
	if (twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER, &val,
							TWL4030_PM_MASTER_P1_SW_EVENTS)) {
		printf("Error:TWL4030: failed to read the power register\n");
		printf("Could not initialize hardware reset\n");
	} else {
		val |= TWL4030_PM_MASTER_SW_EVENTS_DEVOFF;
		if (twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, val,
								 TWL4030_PM_MASTER_P1_SW_EVENTS)) {
			printf("Error:TWL4030: failed to write the power register\n");
			printf("Could not initialize hardware reset\n");
		}
	}
	// CHECKME: do we have to write PM_MASTER_P2 and _P3?
}

void suspend(void)
{
	// enable the AUX button interrupt for Wakeup
	// suspend (sleep) the CPU
	// the TPS should remain powered on
}
