/*
 * Copyright (C) 2007-2008 Texas Instruments, Inc.
 *
 * (C) Copyright 2009
 * Windriver, <www.windriver.com>
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * this patch was found here: https://gforge.ti.com/gf/project/omapandroid/mailman/?action=ListThreads&mailman_id=22&_forum_action=ForumMessageBrowse&thread_id=1252
 *
 */
#include <twl4030.h>
#include "twl4030-additions.h"

/*
 * Battery
 */

#define mdelay(n) ({ unsigned long msec = (n); while (msec--) udelay(1000); })

static inline int clear_n_set(u8 chip_no, u8 clear, u8 set, u8 reg)
{
	int ret;
	u8 val = 0;
	
	/* Gets the initial register value */
	ret = twl4030_i2c_read_u8(chip_no, &val, reg);
	if (ret) {
		printf("a\n");
		return ret;
	}
	
	/* Clearing all those bits to clear */
	val &= ~(clear);
	
	/* Setting all those bits to set */
	val |= set;
	
	/* Update the register */
	ret = twl4030_i2c_write_u8(chip_no, val, reg);
	if (ret) {
		printf("b\n");
		return ret;
	}
	return 0;
}

/*
 * Disable/Enable AC Charge funtionality.
 */
static int twl4030_ac_charger_enable(int enable)
{
	int ret;
	
	if (enable) {
		/* forcing the field BCIAUTOAC (BOOT_BCI[0]) to 1 */
		ret = clear_n_set(TWL4030_CHIP_PM_MASTER, 0,
						  +	 (CONFIG_DONE | BCIAUTOWEN | BCIAUTOAC),
						  +	 REG_BOOT_BCI);
		if (ret)
			return ret;
	} else {
		/* forcing the field BCIAUTOAC (BOOT_BCI[0]) to 0 */
		ret = clear_n_set(TWL4030_CHIP_PM_MASTER, BCIAUTOAC,
						  +	 (CONFIG_DONE | BCIAUTOWEN),
						  +	 REG_BOOT_BCI);
		if (ret)
			return ret;
	}
	return 0;
}

/*
 * Disable/Enable USB Charge funtionality.
 */
static int twl4030_usb_charger_enable(int enable)
{
	u8 value;
	int ret;
	
	if (enable) {
		/* enable access to BCIIREF1 */
		ret = twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE, 0xE7,
								   +	 REG_BCIMFKEY);
		if (ret)
			return ret;
		
		/* set charging current = 852mA */
		ret = twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE, 0xFF,
								   +	 REG_BCIIREF1);
		if (ret)
			return ret;
		
		/* forcing the field BCIAUTOUSB (BOOT_BCI[1]) to 1 */
		ret = clear_n_set(TWL4030_CHIP_PM_MASTER, 0,
						  +	 (CONFIG_DONE | BCIAUTOWEN | BCIAUTOUSB),
						  +	 REG_BOOT_BCI);
		if (ret)
			return ret;
		
		/* Enabling interfacing with usb thru OCP */
		ret = clear_n_set(TWL4030_CHIP_USB, 0, PHY_DPLL_CLK,
						  +	 REG_PHY_CLK_CTRL);
		if (ret)
			return ret;
		
		value = 0;
		
		while (!(value & PHY_DPLL_CLK)) {
			udelay(10);
			ret = twl4030_i2c_read_u8(TWL4030_CHIP_USB, &value,
									  +	 REG_PHY_CLK_CTRL_STS);
			if (ret)
				return ret;
		}
		
		/* OTG_EN (POWER_CTRL[5]) to 1 */
		ret = clear_n_set(TWL4030_CHIP_USB, 0, OTG_EN,
						  +	 REG_POWER_CTRL);
		if (ret)
			return ret;
		
		mdelay(50);
		
		/* forcing USBFASTMCHG(BCIMFSTS4[2]) to 1 */
		ret = clear_n_set(TWL4030_CHIP_MAIN_CHARGE, 0,
						  +	 USBFASTMCHG, REG_BCIMFSTS4);
		if (ret)
			return ret;
	} else {
		ret = clear_n_set(TWL4030_CHIP_PM_MASTER, BCIAUTOUSB,
						  +	 (CONFIG_DONE | BCIAUTOWEN), REG_BOOT_BCI);
		if (ret)
			return ret;
	}
	
	return 0;
}

/*
 * Setup the twl4030 MADC module to measure the backup
 * battery voltage.
 */
static int twl4030_madc_setup(void)
{
	int ret = 0;
	
	/* turning MADC clocks on */
	ret = clear_n_set(TWL4030_CHIP_INTBR, 0,
					  +	 (MADC_HFCLK_EN | DEFAULT_MADC_CLK_EN), REG_GPBR1);
	if (ret)
		return ret;
	
	/* turning adc_on */
	ret = twl4030_i2c_write_u8(TWL4030_CHIP_MADC, MADC_ON,
							   +	 REG_CTRL1);
	if (ret)
		return ret;
	
	/* setting MDC channel 9 to trigger by SW1 */
	ret = clear_n_set(TWL4030_CHIP_MADC, 0, SW1_CH9_SEL,
					  +	 REG_SW1SELECT_MSB);
	
	return ret;
}

/*
 * Charge backup battery through main battery
 */
static int twl4030_charge_backup_battery(void)
{
	int ret;
	
	ret = clear_n_set(TWL4030_CHIP_PM_RECIEVER, 0xff,
					  +	 (BBCHEN | BBSEL_3200mV | BBISEL_500uA), REG_BB_CFG);
	if (ret)
		return ret;
	
	return 0;
}

/*
 * Helper function to read a 2-byte register on BCI module
 */
static int read_bci_val(u8 reg)
{
	int ret = 0, temp = 0;
	u8 val;
	
	/* reading MSB */
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_MAIN_CHARGE, &val, reg + 1);
	if (ret)
		return ret;
	
	temp = ((int)(val & 0x03)) << 8;
	
	/* reading LSB */
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_MAIN_CHARGE, &val, reg);
	if (ret)
		return ret;
	
	return temp + val;
}

/*
 * Triggers the sw1 request for the twl4030 module to measure the sw1 selected
 * channels
 */
static int twl4030_madc_sw1_trigger(void)
{
	u8 val;
	int ret;
	
	/* Triggering SW1 MADC convertion */
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val, REG_CTRL_SW1);
	if (ret)
		return ret;
	
	val |= SW1_TRIGGER;
	
	ret = twl4030_i2c_write_u8(TWL4030_CHIP_MADC, val, REG_CTRL_SW1);
	if (ret)
		return ret;
	
	/* Waiting until the SW1 conversion ends*/
	val = BUSY;
	
	while (!((val & EOC_SW1) && (!(val & BUSY)))) {
		ret = twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val,
								  +	 REG_CTRL_SW1);
		if (ret)
			return ret;
		mdelay(10);
	}
	
	return 0;
}

/*
 * Return battery voltage
 */
static int twl4030_get_battery_voltage(void)
{
	int volt;
	
	volt = read_bci_val(T2_BATTERY_VOLT);
	return (volt * 588) / 100;
}

/*
 * Return battery temperature (uncalibrated)
 * current = 10uA, full scale 1023 = 1.5V; prescaler=1
 * 10k @ 25C -> 0.1V -> 68
 * 26.6k @ 0C -> 0.26V -> 182
 */

#define R0C	26600
#define R25C 10000

#define VSTEP ((15*10000)/1023))
#define VAL0C (R0C/VSTEP)
#define VAL25C (R25C/VSTEP)

static int twl4030_get_battery_temperature(void)
{
	int temperature;
	
	temperature = read_bci_val(T2_BATTERY_TEMP);
	return ((R0C - temperature) * 25) / (R0C - R25C);	// CHECKME
//	return ((182 - temperature) * 25) / (182 - 68);	// CHECKME
}

/*
 * Return battery current
 */
static int twl4030_get_battery_current(void)
{
	int current;
	
	/* from linux driver
	 int curr;
	 int ret;
	 u8 bcictl1;
	 
	 curr = twl4030bci_read_adc_val(TWL4030_BCIICHG);
	 if (curr < 0)
	 return curr;
	 
	 ret = twl4030_bci_read(TWL4030_BCICTL1, &bcictl1);
	 if (ret)
	 return ret;
	 
	 ret = (curr * 16618 - 850 * 10000) / 10;
	 if (bcictl1 & TWL4030_CGAIN)
	 ret *= 2;
	 
	 return ret;
*/	 
	
	current = read_bci_val(T2_BATTERY_CUR);
	return ((current - 512) * 1) / 1;	// FIXME
}

/*
 * Return the battery backup voltage
 */
static int twl4030_get_backup_battery_voltage(void)
{
	int ret, temp;
	u8 volt;
	
	/* trigger MADC convertion */
	twl4030_madc_sw1_trigger();
	
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &volt, REG_GPCH9 + 1);
	if (ret)
		return ret;
	
	temp = ((int) volt) << 2;
	
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &volt, REG_GPCH9);
	if (ret)
		return ret;
	
	temp = temp + ((int) ((volt & MADC_LSB_MASK) >> 6));
	
	return (temp * 441) / 100;
}

/*
 * Return the AC power supply voltage
 */
static int twl4030_get_ac_charger_voltage(void)
{
	int volt = read_bci_val(T2_BATTERY_ACVOLT);
	return (volt * 735) / 100;
}

/*
 * Return the USB power supply voltage
 */
static int twl4030_get_usb_charger_voltage(void)
{
	int volt = read_bci_val(T2_BATTERY_USBVOLT);
	return (volt * 2058) / 300;
}

/*
 * Battery charging main function called from board-specific file
 */

int twl4030_init_battery_charging(void)
{
	u8 batstsmchg, batstspchg, hwsts;
	int battery_volt = 0, charger_present = 0;
	int ret = 0;
	
#ifdef CONFIG_3430ZOOM2
	/* For Zoom2 enable Main charge Automatic mode:
	 * by enabling MADC clocks
	 */
	
	/* Enable AC charging */
	ret = clear_n_set(TWL4030_CHIP_INTBR, 0,
					  +	 (MADC_HFCLK_EN | DEFAULT_MADC_CLK_EN), REG_GPBR1);
	
	udelay(100);
	
	
	ret = twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE, 0xE7,
							   +	 REG_BCIMFKEY);
	/* set MAX charging current */
	ret = twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE, 0xFF,
							   +	 REG_BCIIREF1);
		
	/* Done for Zoom2 */
	return 0;
#endif
	
	/* check for battery presence */
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_MAIN_CHARGE, &batstsmchg,
							  +	 REG_BCIMFSTS3);
	if (ret)
		return ret;
	
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_PRECHARGE, &batstspchg,
							  +	 REG_BCIMFSTS1);
	if (ret)
		return ret;
	
	if (!((batstspchg & BATSTSPCHG) || (batstsmchg & BATSTSMCHG))) {
		printf("no battery\n");
		return ret;	/* no battery */		
	}
	
	ret = twl4030_madc_setup();
	if (ret) {
		printf("twl4030 madc setup error %d\n", ret);
		return ret;
	}
	/* backup battery charges through main battery */
	ret = twl4030_charge_backup_battery();
	if (ret) {
		printf("backup battery charging error\n");
		return ret;
	}
	
	// SPLITME here - up to here it is init, all below can be a systest user command
	
	/* check for charger presence */
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER, &hwsts,
							  +	 REG_STS_HW_CONDITIONS);
	if (ret)
		return ret;
	
	if (hwsts & STS_CHG) {
		printf("AC charger detected\n");
		ret = twl4030_ac_charger_enable(1);
		if (ret)
			return ret;
		charger_present = 1;
	} else {
		if (hwsts & STS_VBUS) {
			printf("USB charger detected\n");
			charger_present = 1;
		}
		/* usb charging is enabled regardless of the whether the
		 * charger is attached, otherwise the main battery voltage
		 * cannot be read
		 */
		ret = twl4030_usb_charger_enable(1);
		if (ret)
			return ret;
	}
	battery_volt = twl4030_get_battery_voltage();
	printf("Main battery charge:    %d mV\n", battery_volt);
	printf("Battery temperature:    %d C\n", twl4030_get_battery_temperature());
	printf("Backup battery voltage: %d mV\n", twl4030_get_backup_battery_voltage());
	printf("AC charger voltage:     %d mV\n", twl4030_get_ac_charger_voltage());
	printf("USB charger voltage:    %d mV\n", twl4030_get_usb_charger_voltage());
	printf("Charging current:       %d mA\n", twl4030_get_battery_current());
	return ret;
}

#if (defined(CONFIG_TWL4030_KEYPAD) && (CONFIG_TWL4030_KEYPAD))
/*
 * Keypad
 */
int twl4030_keypad_init(void)
{
	int ret = 0;
	u8 ctrl;
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_KEYPAD, &ctrl, KEYPAD_KEYP_CTRL_REG);
	if (!ret) {
		ctrl |= CTRL_KBD_ON | CTRL_SOFT_NRST;
		ctrl &= ~CTRL_SOFTMODEN;
		ret = twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD, ctrl, KEYPAD_KEYP_CTRL_REG);
	}
	return ret;
}

int twl4030_keypad_reset(void)
{
	int ret = 0;
	u8 ctrl;
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_KEYPAD, &ctrl, KEYPAD_KEYP_CTRL_REG);
	if (!ret) {
		ctrl &= ~CTRL_SOFT_NRST;
		ret = twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD, ctrl, KEYPAD_KEYP_CTRL_REG);
	}
	return ret;
}

int twl4030_keypad_keys_pressed(unsigned char *key1, unsigned char *key2)
{
	int ret = 0;
	u8 cb, c, rb, r;
	for (cb = 0; cb < 8; cb++) {
		c = 0xff & ~(1 << cb);
		twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD, c, KEYPAD_KBC_REG);
		twl4030_i2c_read_u8(TWL4030_CHIP_KEYPAD, &r, KEYPAD_KBR_REG);
		for (rb = 0; rb < 8; rb++) {
			if (!(r & (1 << rb))) {
				if (!ret)
					*key1 = cb << 3 | rb;
				else if (1 == ret)
					*key2 = cb << 3 | rb;
				ret++;
			}
		}
	}
	return ret;
}

#endif
