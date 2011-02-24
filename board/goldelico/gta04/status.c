/* u-boot driver for the GTA04 LEDs and Buttons
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
#include "status.h"

#if defined(CONFIG_OMAP3_GTA04)

// no need to probe for LED controller (compiler should optimize unnecessary code)
#define hasTCA6507 (1==1)

#else

static int hasTCA6507=0;

#endif

#define TWL4030_I2C_BUS		1	// I2C1
#define TCA6507_BUS			2	// I2C2
#define TCA6507_ADDRESS		0x45

/* register numbers */
#define TCA6507_SELECT0						0
#define TCA6507_SELECT1						1
#define TCA6507_SELECT2						2
#define TCA6507_FADE_ON_TIME				3
#define TCA6507_FULLY_ON_TIME				4
#define TCA6507_FADR_OFF_TIME				5
#define TCA6507_FIRST_FULLY_OFF_TIME		6
#define TCA6507_SECOND_FULLY_OFF_TIME		7
#define TCA6507_MAXIMUM_INTENSITY			8
#define TCA6507_ONE_SHOT_MASTER_INTENSITY	9
#define TCA6507_INITIALIZATION				10

#define TCA6507_AUTO_INCREMENT				16

// we can't include "beagle.h"
/* BeagleBoard revisions */
extern int get_board_revision(void); 
#define REVISION_AXBX	0x7
#define REVISION_CX	0x6
#define REVISION_C4	0x5
#define REVISION_XM	0x0

static int isXM = 0;

#if defined(CONFIG_OMAP3_GTA04)

#define GPIO_AUX		7		// AUX/User button
#define GPIO_POWER		-1		// N/A on GTA04 (access through TPS65950)
#define GPIO_GPSEXT		144		// external GPS antenna is plugged in
#define GPIO_PENIRQ		160		// TSC must be set up to provide PENIRQ

#elif defined(CONFIG_OMAP3_BEAGLE_EXPANDER)

#define GPIO_AUX		136		// AUX/User button
#define GPIO_POWER		137		// POWER button
#define GPIO_GPSEXT		138		// external GPS antenna is plugged in
#define GPIO_PENIRQ		157		// TSC must be set up to provide PENIRQ

#else

#define GPIO_AUX		136		// AUX/User button
#define GPIO_POWER		137		// N/A on GTA04
#define GPIO_GPSEXT		138		// external GPS antenna is plugged in
#define GPIO_PENIRQ		157		// TSC must be set up to provide PENIRQ

#endif

#define GPIO_LED_AUX_RED		(isXM?88:70)		// AUX
#define GPIO_LED_AUX_GREEN		(isXM?89:71)		// AUX
#define GPIO_LED_POWER_RED		78					// Power
#define GPIO_LED_POWER_GREEN	79					// Power
#define GPIO_LED_VIBRA			(isXM?2:88)			// Vibracall motor
#define GPIO_LED_UNUSED			(isXM?3:89)			// unused

void status_set_status(int value)
{
	if(!hasTCA6507) {
		omap_set_gpio_dataout(GPIO_LED_AUX_RED, (value&(1 << 0)));
		omap_set_gpio_dataout(GPIO_LED_AUX_GREEN, (value&(1 << 1)));
		omap_set_gpio_dataout(GPIO_LED_POWER_RED, (value&(1 << 3)));
		omap_set_gpio_dataout(GPIO_LED_POWER_GREEN, (value&(1 << 4)));
		omap_set_gpio_dataout(GPIO_LED_VIBRA, (value&(1 << 6)));
		omap_set_gpio_dataout(GPIO_LED_UNUSED, (value&(1 << 7)));
		}
	else {
		value &= 0x3f;	// 6 LEDs only - 7th is reserved to reset the WLAN/BT chip
		i2c_set_bus_num(TCA6507_BUS-1);	// write I2C2
		// we could write a autoincrement address and all 3 bytes in a single message
		// we could set the TCA to do smooth transitions
		i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT0, 0);
		i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT1, 0);
		i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT2, value);	// 1 = on
	}
}

int status_get_buttons(void)
{ // convert button state into led state
#if defined(CONFIG_OMAP3_GTA04)
	u8 val;
	i2c_set_bus_num(TWL4030_I2C_BUS-1);	// read I2C1
	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER, &val, TWL4030_PM_MASTER_STS_HW_CONDITIONS);	// read state of power button (bit 0) from TPS65950
	return ((omap_get_gpio_datain(GPIO_AUX)) << 0) |
		((omap_get_gpio_datain(GPIO_GPSEXT)) << 1) |
		(((val&0x01) != 0) << 3) |
		((omap_get_gpio_datain(GPIO_PENIRQ)) << 4);
#elif defined(CONFIG_OMAP3_BEAGLE_EXPANDER)
	return
		((omap_get_gpio_datain(GPIO_AUX)) << 0) |
		((0) << 1) |
		((!omap_get_gpio_datain(GPIO_POWER)) << 3) |
		((omap_get_gpio_datain(GPIO_PENIRQ)) << 4);
#else
	return
		((!omap_get_gpio_datain(GPIO_AUX)) << 0) |
		((omap_get_gpio_datain(GPIO_GPSEXT)) << 1) |
		((!omap_get_gpio_datain(GPIO_POWER)) << 3) |
		((omap_get_gpio_datain(GPIO_PENIRQ)) << 4);
#endif
}

int status_init(void)
{
	isXM = (get_board_revision() == REVISION_XM);
#if !defined(CONFIG_OMAP3_GTA04)
	if(i2c_set_bus_num(TCA6507_BUS-1))
		{
		printf ("could not select I2C2\n");
		return 1;
		}
	hasTCA6507 = !i2c_probe(TCA6507_ADDRESS);
#endif
	
	if(!hasTCA6507) {
		if(isXM) { // XM has scrambled dss assignment with respect to default ball name
			MUX_VAL(CP(DSS_DATA18),		(IEN | PTD | EN | M4)); /*GPIO */
			MUX_VAL(CP(DSS_DATA19),		(IEN | PTD | EN | M4)); /*GPIO */
			MUX_VAL(CP(DSS_DATA8),		(IEN | PTD | EN | M4)); /*GPIO */
			MUX_VAL(CP(DSS_DATA9),		(IEN | PTD | EN | M4)); /*GPIO */
			MUX_VAL(CP(SYS_BOOT0),		(IEN | PTD | EN | M4)); /*GPIO */
			MUX_VAL(CP(SYS_BOOT1),		(IEN | PTD | EN | M4)); /*GPIO */
		}
		else {
			MUX_VAL(CP(DSS_DATA0),		(IEN | PTD | EN | M4)); /*GPIO */
			MUX_VAL(CP(DSS_DATA1),		(IEN | PTD | EN | M4)); /*GPIO */
			MUX_VAL(CP(DSS_DATA8),		(IEN | PTD | EN | M4)); /*GPIO */
			MUX_VAL(CP(DSS_DATA9),		(IEN | PTD | EN | M4)); /*GPIO */
			MUX_VAL(CP(DSS_DATA16),		(IEN | PTD | EN | M4)); /*GPIO */
			MUX_VAL(CP(DSS_DATA17),		(IEN | PTD | EN | M4)); /*GPIO */
		}
		
		omap_request_gpio(GPIO_LED_AUX_GREEN);
		omap_request_gpio(GPIO_LED_AUX_RED);
		omap_request_gpio(GPIO_LED_POWER_GREEN);
		omap_request_gpio(GPIO_LED_POWER_RED);
		omap_request_gpio(GPIO_LED_VIBRA);
		omap_request_gpio(GPIO_LED_UNUSED);
		omap_request_gpio(GPIO_POWER);
	}
	else {
		// initialize I2C controller
	}
	
	omap_request_gpio(GPIO_AUX);
	omap_request_gpio(GPIO_GPSEXT);
	omap_request_gpio(GPIO_PENIRQ);
	
	if(!hasTCA6507) {
		omap_set_gpio_direction(GPIO_LED_AUX_GREEN, 0);		// output
		omap_set_gpio_direction(GPIO_LED_AUX_RED, 0);		// output
		omap_set_gpio_direction(GPIO_LED_POWER_GREEN, 0);		// output
		omap_set_gpio_direction(GPIO_LED_POWER_RED, 0);		// output
		omap_set_gpio_direction(GPIO_LED_VIBRA, 0);		// output
		omap_set_gpio_direction(GPIO_LED_UNUSED, 0);		// output
		omap_set_gpio_direction(GPIO_POWER, 1);		// input
		}
	
	omap_set_gpio_direction(GPIO_AUX, 1);		// input
#ifndef CONFIG_OMAP3_GTA04
	omap_set_gpio_direction(GPIO_POWER, 1);		// input
#endif
	omap_set_gpio_direction(GPIO_GPSEXT, 1);	// input

	// when sould we do omap_free_gpio(GPIO_LED_AUX_GREEN); ?
	printf("did init LED driver for %s\n", hasTCA6507?"TCA6507":"GPIOs");

	return 0;
}

int status_set_flash (int mode)
{ // 0: off, 1: torch, 2: flash
	if(i2c_set_bus_num(TCA6507_BUS-1))
		{
		printf ("could not select I2C2\n");
		return 1;
		}
	// initialize if needed
	// set flash controller mode
	return 0;
}

int status_set_vibra (int mode)
{ // 0: off, 1: left, 2: right
	if(i2c_set_bus_num(TWL4030_I2C_BUS-1))
		{
		printf ("could not select I2C1\n");
		return 1;
		}
	// initialize if needed
	// set vibra controller mode
	return 0;	
}
