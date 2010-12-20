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
#include <i2c.h>
#include <twl4030.h>
#include "systest.h"

int systest(void)
{
	// do mixture of gps_echo, tsc_loop, status mirror status blink
	i2c_set_bus_num(0);	// I2C1
	printf("TPS65950:      %s\n", i2c_probe(TWL4030_CHIP_USB)?"-":"found");	// responds on 4 addresses 0x48..0x4b
	i2c_set_bus_num(1);	// I2C2
	printf("TSC2007:       %s\n", i2c_probe(0x48)?"-":"found");
	printf("TCA6507:       %s\n", i2c_probe(0x45)?"-":"found");
	printf("LIS302 TOP:    %s\n", i2c_probe(0x1c)?"-":"found");
	printf("LIS302 BOTTOM: %s\n", i2c_probe(0x1d)?"-":"found");
	printf("LSM303:        %s\n", i2c_probe(0x19)?"-":"found");
	printf("HMC5843:       %s\n", i2c_probe(0x1e)?"-":"found");
	printf("BMP085:        %s\n", i2c_probe(0x77)?"-":"found");
	printf("ITG3200:       %s\n", i2c_probe(0x68)?"-":"found");
	printf("Si4721:        %s\n", i2c_probe(0x21)?"-":"found");
	printf("TCA8418:       %s\n", i2c_probe(0x64)?"-":"found");
	i2c_set_bus_num(2);	// I2C3
	// LEDs
	// GPS UART
	// RAM-Test
	// NAND-Test
	// Buttons
	// Power
	// Display communication
	return (0);
}

