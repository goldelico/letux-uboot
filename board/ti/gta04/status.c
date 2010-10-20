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
#include "status.h"

static int isGTA04 = 0;

// we can't include "beagle.h"
/* BeagleBoard revisions */
#define REVISION_AXBX	0x7
#define REVISION_CX	0x6
#define REVISION_C4	0x5
#define REVISION_XM	0x0

static int isXM = 0;

// Note: on GTA04 the LEDs will be connected to TCA8418 and controlled through I2C
// Note: BB-XM has scrambled DSS assignment

#define GPIO_LED_AUX_RED		(isXM?88:70)		// AUX
#define GPIO_LED_AUX_GREEN		(isXM?89:71)		// AUX
#define GPIO_LED_POWER_RED		78					// Power
#define GPIO_LED_POWER_GREEN	79					// Power
#define GPIO_LED_VIBRA			(isXM?2:88)			// Vibracall motor
#define GPIO_LED_UNUSED			(isXM?3:89)			// unused

// Note: on GTA04 the GPIOs will be assigned differently and the state of the POWER button is only available through the TPS65950

#define GPIO_AUX		(isGTA04?136:136)
#define GPIO_POWER		137
#define GPIO_GPSEXT		(isGTA04?138:138)		// external GPS antenna is plugged in
#define GPIO_PENIRQ		(isGTA04?157:157)		// TSC must be set up to provide PENIRQ

void led_set_led(int value)
{
	if(!isGTA04) {
		omap_set_gpio_dataout(GPIO_LED_AUX_RED, (value&(1 << 0)));
		omap_set_gpio_dataout(GPIO_LED_AUX_GREEN, (value&(1 << 1)));
		omap_set_gpio_dataout(GPIO_LED_POWER_RED, (value&(1 << 3)));
		omap_set_gpio_dataout(GPIO_LED_POWER_GREEN, (value&(1 << 4)));
		omap_set_gpio_dataout(GPIO_LED_VIBRA, (value&(1 << 6)));
		omap_set_gpio_dataout(GPIO_LED_UNUSED, (value&(1 << 7)));
		}
	else {
		// initialize i2c controller
	}
}

int led_get_buttons(void)
{ // convert button state into led state
	if(isGTA04) {
		// read GPIOs and TPS65950
	}
	return
		((!omap_get_gpio_datain(GPIO_AUX)) << 0) |
		((omap_get_gpio_datain(GPIO_GPSEXT)) << 1) |
		((!omap_get_gpio_datain(GPIO_POWER)) << 3) |
		((omap_get_gpio_datain(GPIO_PENIRQ)) << 4);
}

int led_init(void)
{
	isXM = (beagle_get_revision() == REVISION_XM);
	
	if(!isGTA04) {
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
	}
	
	omap_request_gpio(GPIO_AUX);
	omap_request_gpio(GPIO_POWER);
	omap_request_gpio(GPIO_GPSEXT);
	omap_request_gpio(GPIO_PENIRQ);
	
	if(!isGTA04) {
		omap_set_gpio_direction(GPIO_LED_AUX_GREEN, 0);		// output
		omap_set_gpio_direction(GPIO_LED_AUX_RED, 0);		// output
		omap_set_gpio_direction(GPIO_LED_POWER_GREEN, 0);		// output
		omap_set_gpio_direction(GPIO_LED_POWER_RED, 0);		// output
		omap_set_gpio_direction(GPIO_LED_VIBRA, 0);		// output
		omap_set_gpio_direction(GPIO_LED_UNUSED, 0);		// output
		}
	
	omap_set_gpio_direction(GPIO_AUX, 1);		// input
	omap_set_gpio_direction(GPIO_POWER, 1);		// input
	omap_set_gpio_direction(GPIO_AUX, 1);		// input
	omap_set_gpio_direction(GPIO_GPSEXT, 1);		// input

	// when sould we do omap_free_gpio(GPIO_LED_AUX_GREEN);
	printf("did led_init()\n");

	return 0;
}

