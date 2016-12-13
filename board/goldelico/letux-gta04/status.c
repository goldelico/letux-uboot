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
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <i2c.h>
#include <twl4030.h>
#include "status.h"

#if defined(CONFIG_TARGET_LETUX_GTA04) || defined(CONFIG_TARGET_LETUX_GTA04_B2) || defined(CONFIG_TARGET_LETUX_GTA04_B3) || defined(CONFIG_TARGET_LETUX_GTA04_B4) || defined(CONFIG_TARGET_LETUX_GTA04_B7)
#define IS_GTA04
#elif defined(CONFIG_TARGET_LETUX_BEAGLE) || defined(CONFIG_TARGET_LETUX_BEAGLE_B1) || defined(CONFIG_TARGET_LETUX_BEAGLE_B2) || defined(CONFIG_TARGET_LETUX_BEAGLE_B4) || defined(CONFIG_TARGET_LETUX_BEAGLE_B7)
#define IS_BEAGLE
#else
#error "unknown CONFIG_TARGET"
#endif

// FIXME: move these configs to include/configs/letux_*.h

#if defined(IS_GTA04)

#define CHECK_TCA6507	0	// no need to probe for LED controller (compiler should optimize unnecessary code)
#define hasTCA6507 (1==1)

#define GPIO_AUX		7		// AUX/User button
#define GPIO_AUX_ACTIVE	1
#define GPIO_POWER		-1		// N/A on GTA04 (access through TPS65950)
#define GPIO_GPSEXT		144		// external GPS antenna is plugged in
#define GPIO_PENIRQ		160		// TSC must be set up to provide PENIRQ
#define GPIO_KEYIRQ		176		// FIXME: was 63, 10 on GTA04A2 and A3

// FIXME: no mainboard variantions?

#else	/* IS_GTA04 */

static int hasTCA6507=0;
#define GPIO_AUX_ACTIVE	1

#if defined(CONFIG_TARGET_LETUX_BEAGLE)
// no expander

#define GPIO_AUX		7		// AUX/User button
#define GPIO_POWER		-1		// N/A on BB (access through TPS65950)
#define GPIO_GPSEXT		-1		// external GPS antenna is plugged in
#define GPIO_PENIRQ		-1		// TSC must be set up to provide PENIRQ
#define GPIO_KEYIRQ		-1		// FIXME: was 63, 10 on GTA04A2 and A3

#elif defined(CONFIG_TARGET_LETUX_BEAGLE_B1)
// openmoko beagle hybrid

#define CHECK_TCA6507	1

#define GPIO_AUX		136		// AUX/User button on expansion board
#define GPIO_POWER		137		// POWER button
#define GPIO_GPSEXT		144		// external GPS antenna is plugged in
#define GPIO_PENIRQ		157		// TSC must be set up to provide PENIRQ
#define GPIO_KEYIRQ		-1

#elif defined(CONFIG_TARGET_LETUX_BEAGLE_B2) || defined(CONFIG_TARGET_LETUX_GTA04_B2)

#define CHECK_TCA6507	1

#define GPIO_AUX		136		// AUX/User button on expansion board
#define GPIO_POWER		137		// POWER button
#define GPIO_GPSEXT		144		// external GPS antenna is plugged in
#define GPIO_PENIRQ		157		// TSC must be set up to provide PENIRQ
#define GPIO_KEYIRQ		138		// TRF79x0

#elif defined(CONFIG_TARGET_LETUX_BEAGLE_B4) || defined(CONFIG_TARGET_LETUX_GTA04_B2)

#elif defined(CONFIG_TARGET_LETUX_BEAGLE_B2) || defined(CONFIG_TARGET_LETUX_GTA04_B2)

#define CHECK_TCA6507	1

#define GPIO_AUX		136		// AUX/User button on expansion board
#define GPIO_POWER		137		// POWER button
#define GPIO_GPSEXT		144		// external GPS antenna is plugged in
#define GPIO_PENIRQ		157		// TSC must be set up to provide PENIRQ
#define GPIO_KEYIRQ		138		// TRF79x0

#elif defined(CONFIG_TARGET_LETUX_BEAGLE_B3) || defined(CONFIG_TARGET_LETUX_GTA04_B3)

#define CHECK_TCA6507	1

#define GPIO_AUX		7		// AUX/User button sits on main board
#define GPIO_POWER		-1		// POWER button
#define GPIO_GPSEXT		144		// external GPS antenna is plugged in
#define GPIO_PENIRQ		157		// TSC must be set up to provide PENIRQ
#define GPIO_KEYIRQ		138		// PPS interrupt

#elif defined(CONFIG_TARGET_LETUX_BEAGLE_B7) || defined(CONFIG_TARGET_LETUX_GTA04_B2)
// Neo900 demo

#define GPIO_AUX		7		// AUX/User button sits on main board
#define GPIO_POWER		-1		// POWER button
#define GPIO_GPSEXT		144		// external GPS antenna is plugged in
#define GPIO_PENIRQ		157		// TSC must be set up to provide PENIRQ
#define GPIO_KEYIRQ		138		// PPS interrupt

#endif

#endif	/* CONFIG_TARGET_LETUX_GTA04 */

#define TWL4030_I2C_BUS		(1-1)	// I2C1
#define TCA6507_BUS			(2-1)	// I2C2
#define TCA6507_ADDRESS		0x45

/* register numbers */
#define TCA6507_SELECT0			0
#define TCA6507_SELECT1			1
#define TCA6507_SELECT2			2
#define TCA6507_FADE_ON_TIME			3
#define TCA6507_FULLY_ON_TIME			4
#define TCA6507_FADR_OFF_TIME			5
#define TCA6507_FIRST_FULLY_OFF_TIME		6
#define TCA6507_SECOND_FULLY_OFF_TIME		7
#define TCA6507_MAXIMUM_INTENSITY		8
#define TCA6507_ONE_SHOT_MASTER_INTENSITY	9
#define TCA6507_INITIALIZATION			10

#define TCA6507_AUTO_INCREMENT			16

static int thisIsXM = 0;

// FIXME some BB Expanders have neither TCA6507 nor LEDs
// we could use the GPIO 149 and 150 LEDs

#define GPIO_LED_AUX_RED		(thisIsXM?88:70)		// AUX
#define GPIO_LED_AUX_GREEN		(thisIsXM?89:71)		// AUX
#define GPIO_LED_POWER_RED		78				// Power
#define GPIO_LED_POWER_GREEN		79				// Power
#define GPIO_LED_VIBRA			(thisIsXM?2:88)			// Vibracall motor
#define GPIO_LED_UNUSED		(thisIsXM?3:89)			// unused

static int status;

void status_set_status(int value)
{
	status=value;
	if(!hasTCA6507) {
		gpio_direction_output(GPIO_LED_AUX_RED, (value&(1 << 0)));
		gpio_direction_output(GPIO_LED_AUX_GREEN, (value&(1 << 1)));
		gpio_direction_output(GPIO_LED_POWER_RED, (value&(1 << 3)));
		gpio_direction_output(GPIO_LED_POWER_GREEN, (value&(1 << 4)));
		gpio_direction_output(GPIO_LED_VIBRA, (value&(1 << 6)));
		gpio_direction_output(GPIO_LED_UNUSED, (value&(1 << 7)));
		}
	else {
		value &= 0x3f;	// 6 LEDs only - 7th is reserved to reset the WLAN/BT chip
		i2c_set_bus_num(TCA6507_BUS);	// write I2C2
		// we could write a autoincrement address and all 3 bytes in a single message
		// we could set the TCA to do smooth transitions
#if defined(CONFIG_TARGET_LETUX_BEAGLE_B2) || defined(CONFIG_TARGET_LETUX_GTA04_B2)
		value |= (value >> 3) & 0x03;	// map power LEDs to AUX LEDs (we only have 2)
#endif
		i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT0, 0);
		i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT1, 0);
		i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT2, value);	// 1 = on
	}
}

int status_get_buttons(void)
{ // convert button state into led state (for mirror)
	int status=0;
	if(GPIO_AUX >= 0)
		status |= ((gpio_get_value(GPIO_AUX) == GPIO_AUX_ACTIVE) << 0);
	if(GPIO_GPSEXT >= 0)
		status |= ((gpio_get_value(GPIO_GPSEXT)) << 1);
	if(GPIO_POWER >= 0)
		status |= ((!gpio_get_value(GPIO_POWER)) << 3);
	else
		{
		u8 val;
		i2c_set_bus_num(TWL4030_I2C_BUS);	// read I2C1
		twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER, TWL4030_PM_MASTER_STS_HW_CONDITIONS, &val);	// read state of power button (bit 0) from TPS65950
		status |= (((val&0x01) != 0) << 3);
		}
	if(GPIO_PENIRQ >= 0)
		status |= ((!gpio_get_value(GPIO_PENIRQ)) << 4);
	if(GPIO_KEYIRQ >= 0)
		status |= ((gpio_get_value(GPIO_KEYIRQ)) << 5);
	return status;
}

int status_init(void)
{
	extern int isXM(void);
	i2c_set_bus_num(TWL4030_I2C_BUS);
	thisIsXM = isXM();
	if(thisIsXM) {
		/* Set VAUX1 to 3.3V for GTA04E display board */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX1_DEDICATED,
								/*TWL4030_PM_RECEIVER_VAUX1_VSEL_33*/ 0x07,
								TWL4030_PM_RECEIVER_VAUX1_DEV_GRP,
								TWL4030_PM_RECEIVER_DEV_GRP_P1);
		udelay(5000);
	}

#if CHECK_TCA6507
	if(i2c_set_bus_num(TCA6507_BUS))
		{ // check if we have a tca
		printf ("could not select I2C2 to probe for TCA6507\n");
		return 1;
		}
	hasTCA6507 = !i2c_probe(TCA6507_ADDRESS);
#endif
	
	if(!hasTCA6507) { // reuse DSS pins
		if(thisIsXM) { // XM has scrambled dss assignment with respect to default ball names
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
		
		gpio_request(GPIO_LED_AUX_GREEN, "green-aux");
		gpio_request(GPIO_LED_AUX_RED, "red-aux");
		gpio_request(GPIO_LED_POWER_GREEN, "green-power");
		gpio_request(GPIO_LED_POWER_RED, "red-power");
		gpio_request(GPIO_LED_VIBRA, "vibra");
		gpio_request(GPIO_LED_UNUSED, "unused");
		if(GPIO_POWER >= 0)
			gpio_request(GPIO_POWER, "power");
	}
	else {
		// initialize I2C controller
	}
	
	if(GPIO_AUX >= 0)
		gpio_request(GPIO_AUX, "aus");
	if(GPIO_POWER >= 0)
		gpio_request(GPIO_POWER, "power");
	if(GPIO_GPSEXT >= 0)
		gpio_request(GPIO_GPSEXT, "ext-gps");
	if(GPIO_PENIRQ >= 0)
		gpio_request(GPIO_PENIRQ, "penirq");
	if(GPIO_KEYIRQ >= 0)
		gpio_request(GPIO_KEYIRQ, "keyirq");
	
	if(!hasTCA6507) {
		gpio_direction_output(GPIO_LED_AUX_GREEN, 0);		// output
		gpio_direction_output(GPIO_LED_AUX_RED, 0);		// output
		gpio_direction_output(GPIO_LED_POWER_GREEN, 0);		// output
		gpio_direction_output(GPIO_LED_POWER_RED, 0);		// output
		gpio_direction_output(GPIO_LED_VIBRA, 0);		// output
		gpio_direction_output(GPIO_LED_UNUSED, 0);		// output
		}
	
	if(GPIO_AUX >= 0)
		gpio_direction_input(GPIO_AUX);		// input
	if(GPIO_POWER >= 0)
		gpio_direction_input(GPIO_POWER);		// input
	if(GPIO_GPSEXT >= 0)
		gpio_direction_input(GPIO_GPSEXT);	// input
	if(GPIO_PENIRQ >= 0)
		gpio_direction_input(GPIO_PENIRQ);	// input
	if(GPIO_KEYIRQ >= 0)
		gpio_direction_input(GPIO_KEYIRQ);	// input

	// when sould we do omap_free_gpio(GPIO_LED_AUX_GREEN); ?
	printf("did init LED driver for %s\n", hasTCA6507?"TCA6507":"GPIOs");

	return 0;
}

int status_set_flash (int mode)
{ // 0: off, 1: torch, 2: flash
	if(i2c_set_bus_num(TCA6507_BUS))
		{
		printf ("could not select I2C2\n");
		return 1;
		}
	// initialize if needed
	// set flash controller mode
	return 0;
}

int status_set_vibra (int value)
{ // 0: off otherwise msb controls left/right (2's complement)
	unsigned char byte;
	if(i2c_set_bus_num(TWL4030_I2C_BUS))
		{
		printf ("could not select I2C1\n");
		return 1;
		}
	
	// program Audio controller (see document SWCU050D)
	
	byte = 0x00;						// LEDAON=LEDBON=0
	i2c_write(0x4A, 0xEE, 1, &byte, 1);	// LEDEN
	byte = value != 0 ? 0x03 : 0x00;	// 8 kHz, Codec on (if value != 0), Option 1:RX and TX stereo audio path
	i2c_write(0x49, 0x01, 1, &byte, 1);	// CODEC_MODE
	byte = 0x16;						// APLL_EN enabled, 26 MHz
	i2c_write(0x49, 0x3a, 1, &byte, 1);	// APLL_CTL
	byte = 0x04;						// use PWM
	i2c_write(0x4B, 0x60, 1, &byte, 1);	// VIBRATOR_CFG
	byte = value > 0?0x01:0x03;			// use VIBRADIR, local driver, enable
	i2c_write(0x49, 0x45, 1, &byte, 1);	// VIBRATOR_CFG
	byte = 256-(value>=0?value:-value);	// PWM turnon value
	if(byte == 0) byte = 0x01;			// 0x00 is forbidden!
	i2c_write(0x49, 0x46, 1, &byte, 1);	// VIBRA_SEL

	// do we have to set some Audio PLL frequency (number 6 & 7?)

	/*
	 To use the vibrator H-bridge:
	 1. Disable LEDA: Set the LEDEN[0]LEDAON bit to logic 0.
	 2. Disable LEDB: Set the LEDEN[1]LEDBON bit to logic 0.
	 3. Turn on the codec:
	 Set the CODEC_MODE[1] CODECPDZ bit to 1.
	 The H-bridge vibrator can get its operation from the following sources:
	 •	An audio channel can provide the stimulus.
	 •	A PWM in the audio subchip can generate the signal.
	 If an audio channel provides the motivating force for the vibrator (for example: the audio right 1 channel):
	 1. Set the VIBR_CTL[4]VIBRA_SEL bit to 1.
	 2. Set the VIBR_CTL[5]VIBRA_DIR_SEL bit to 1.
	 3. Set the VIBR_CTL[3:2]VIBRA_AUDIO_SEL bit field to 0x1 (audio right 1 channel).
	 4. Select the use of the SIGN bit to determine the output phase to VIBRA_P and VIBRA_M. 
	 5. Set the VIBRA_CTL[0]VIBRA_EN bit to 1 (power to the H-bridge is driven by audiodata).
	 Notes:
	 •	If audio data drives the vibrator H-bridge, set the VIBRA_SET register to 0xFF.
	 •	The direction of the vibrator H-bridge controlled by the VIBRA_DIR bit can be changed
	 on the fly.
	 6. Set the audio PLL input frequency: The APLL_CTL APLL_INFREQ bitfield=0x6.
	 7. Enable the audio PLL: The APLL_CTL APLL_ENbit=1.
	 Note:	Do not enable LEDA/B and the H-vibrator simultaneously.
	 */
	return 0;	
}

/* compatibility to u-boot led command like beagleboard */

#if defined(IS_GTA04)

static int get_led_gpio(led_id_t mask)
{
	if(mask & (1<<0)) return GPIO_LED_AUX_RED;
	if(mask & (1<<1)) return GPIO_LED_AUX_GREEN;
	if(mask & (1<<2)) return GPIO_LED_POWER_RED;
	if(mask & (1<<3)) return GPIO_LED_POWER_GREEN;
	if(mask & (1<<4)) return GPIO_LED_VIBRA;
	if(mask & (1<<5)) return GPIO_LED_UNUSED;
	return 0;
}

void __led_init (led_id_t mask, int state)
{
	int toggle_gpio;

	toggle_gpio = get_led_gpio(mask);

	if (toggle_gpio && !gpio_request(toggle_gpio, "led"))
		__led_set(mask, state);
}

void __led_toggle (led_id_t mask)
{
	int state, toggle_gpio;

	toggle_gpio = get_led_gpio(mask);
	if (toggle_gpio) {
		state = gpio_get_value(toggle_gpio);
		gpio_direction_output(toggle_gpio, !state);
	}
}

void __led_set (led_id_t mask, int state)
{
	int toggle_gpio;

	toggle_gpio = get_led_gpio(mask);
	if (toggle_gpio)
		gpio_direction_output(toggle_gpio, state);
}

#endif
