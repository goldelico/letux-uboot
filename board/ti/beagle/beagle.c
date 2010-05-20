/*
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#include <common.h>
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include "beagle.h"

static struct {
	unsigned int device_vendor;
	unsigned char revision;
	unsigned char content;
	unsigned char fab_revision[8];
	unsigned char env_var[16];
	unsigned char env_setting[64];
} expansion_config;

#define TWL4030_I2C_BUS			0

#define EXPANSION_EEPROM_I2C_BUS	1
#define EXPANSION_EEPROM_I2C_ADDRESS	0x50

#define TINCANTOOLS_VENDORID	0x0100
#define GUMSTIX_VENDORID		0x0200
#define SPECIALCOMP_VENDORID	0x0300
#define HYR_VENDORID			0x0400
#define MENTOREL_VENDORID		0x0500
#define KBADC_VENDORID			0x0600

#define TINCANTOOLS_ZIPPY		0x01000100
#define TINCANTOOLS_ZIPPY2		0x02000100
#define TINCANTOOLS_TRAINER		0x04000100
#define TINCANTOOLS_SHOWDOG		0x03000100
#define KBADC_BEAGLEFPGA		0x01000600

#define BEAGLE_NO_EEPROM		0xffffffff

static int beagle_revision;

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3_BEAGLE;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

#if defined(CONFIG_STATUS_LED) && defined(STATUS_LED_BOOT)
	status_led_set (STATUS_LED_BOOT, STATUS_LED_ON);
#endif

	return 0;
}

/*
 * Routine: beagle_get_revision
 * Description: Return the revision of the BeagleBoard this code is running on.
 */
int beagle_get_revision(void)
{
	return beagle_revision;
}

/*
 * Routine: beagle_identify
 * Description: Detect if we are running on a Beagle revision Ax/Bx,
 *		C1/2/3, C4 or D. This can be done by reading
 *		the level of GPIO173, GPIO172 and GPIO171. This should
 *		result in
 *		GPIO173, GPIO172, GPIO171: 1 1 1 => Ax/Bx
 *		GPIO173, GPIO172, GPIO171: 1 1 0 => C1/2/3
 *		GPIO173, GPIO172, GPIO171: 1 0 1 => C4
 *		GPIO173, GPIO172, GPIO171: 0 0 0 => XM
 */
void beagle_identify(void)
{
	omap_request_gpio(171);
	omap_request_gpio(172);
	omap_request_gpio(173);
	omap_set_gpio_direction(171, 1);
	omap_set_gpio_direction(172, 1);
	omap_set_gpio_direction(173, 1);

	beagle_revision = omap_get_gpio_datain(173) << 2 |
			  omap_get_gpio_datain(172) << 1 |
			  omap_get_gpio_datain(171);
	omap_free_gpio(171);
	omap_free_gpio(172);
	omap_free_gpio(173);
}

/*
 * Routine: get_expansion_id
 * Description: This function checks for expansion board by checking I2C
 *		bus 2 for the availability of an AT24C01B serial EEPROM.
 *		returns the device_vendor field from the EEPROM
 */
unsigned int get_expansion_id(void)
{
	i2c_set_bus_num(EXPANSION_EEPROM_I2C_BUS);

	/* return BEAGLE_NO_EEPROM if eeprom doesn't respond */
	if (i2c_probe(EXPANSION_EEPROM_I2C_ADDRESS) == 1)
		return BEAGLE_NO_EEPROM;

	/* read configuration data */
	i2c_read(EXPANSION_EEPROM_I2C_ADDRESS, 0, 1, (u8 *)&expansion_config,
		 sizeof(expansion_config));

	return expansion_config.device_vendor;
}

/*
 * Configure DSS to display background color on DVID
 * Configure VENC to display color bar on S-Video
 */
void display_init(void)
{
	omap3_dss_venc_config(&venc_config_std_tv, VENC_HEIGHT, VENC_WIDTH);
	omap3_dss_panel_config(&dvid_cfg);
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	struct gpio *gpio5_base = (struct gpio *)OMAP34XX_GPIO5_BASE;
	struct gpio *gpio6_base = (struct gpio *)OMAP34XX_GPIO6_BASE;

	beagle_identify();

	printf("\nProbing for expansion boards, if none are connected you'll see a harmless I2C error.\n\n");

	switch (get_expansion_id()) {
		case TINCANTOOLS_ZIPPY:
			printf("Recognized Tincantools Zippy expansion board (rev %d %s)\n",
				expansion_config.revision, expansion_config.fab_revision);
			MUX_TINCANTOOLS_ZIPPY();
			setenv("buddy", "zippy");
			break;
		case TINCANTOOLS_ZIPPY2:
			printf("Recognized Tincantools Zippy2 expansion board (rev %d %s)\n",
				expansion_config.revision, expansion_config.fab_revision);
			MUX_TINCANTOOLS_ZIPPY();
			setenv("buddy", "zippy2");
			break;
		case TINCANTOOLS_TRAINER:
			printf("Recognized Tincantools Trainer expansion board (rev %d %s)\n",
				expansion_config.revision, expansion_config.fab_revision);
			MUX_TINCANTOOLS_ZIPPY();
			MUX_TINCANTOOLS_TRAINER();
			setenv("buddy", "trainer");
			break;
		case TINCANTOOLS_SHOWDOG:
			printf("Recognized Tincantools Showdow expansion board (rev %d %s)\n",
				expansion_config.revision, expansion_config.fab_revision);
			/* Place holder for DSS2 definition for showdog lcd */
			setenv("defaultdisplay", "showdoglcd");
			setenv("buddy", "showdog");
			break;
		case KBADC_BEAGLEFPGA:
			printf("Recognized KBADC Beagle FPGA board\n");
			MUX_KBADC_BEAGLEFPGA();
			setenv("buddy", "beaglefpga");
			break;
		case BEAGLE_NO_EEPROM:
			printf("No EEPROM on expansion board\n");
			setenv("buddy", "none");
			break;
		default:
			printf("Unrecognized expansion board: %x\n", expansion_config.device_vendor);
			setenv("buddy", "unknown");
	}

	if (expansion_config.content == 1)
		setenv(expansion_config.env_var, expansion_config.env_setting);

	i2c_set_bus_num(TWL4030_I2C_BUS);

	twl4030_power_init();
	twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);
	display_init();

	switch (beagle_revision) {
	case REVISION_AXBX:
		printf("Beagle Rev Ax/Bx\n");
		setenv("mpurate", "600");
		setenv("beaglerev", "AxBx");
		break;
	case REVISION_CX:
		printf("Beagle Rev C1/C2/C3\n");
		MUX_BEAGLE_C();
		setenv("mpurate", "600");
		setenv("beaglerev", "Cx");
		break;
	case REVISION_C4:
		printf("Beagle Rev C4\n");
		setenv("beaglerev", "Cx");
		MUX_BEAGLE_C();
		/* Set VAUX2 to 1.8V for EHCI PHY */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
					TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
					TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);
		setenv("mpurate", "720");
		break;
	case REVISION_XM:
		printf("Beagle xM Rev A\n");
		setenv("beaglerev", "xMA");
		MUX_BEAGLE_XM();
		/* Set VAUX2 to 1.8V for EHCI PHY */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
					TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
					TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);
		setenv("mpurate", "1000");
		break;
	default:
		printf("Beagle unknown 0x%02x\n", beagle_revision);
	}

	/* Configure GPIOs to output */
	writel(~(GPIO23 | GPIO10 | GPIO8 | GPIO2 | GPIO1), &gpio6_base->oe);
	writel(~(GPIO31 | GPIO30 | GPIO29 | GPIO28 | GPIO22 | GPIO21 |
		GPIO15 | GPIO14 | GPIO13 | GPIO12), &gpio5_base->oe);

	/* Set GPIOs */
	writel(GPIO23 | GPIO10 | GPIO8 | GPIO2 | GPIO1,
		&gpio6_base->setdataout);
	writel(GPIO31 | GPIO30 | GPIO29 | GPIO28 | GPIO22 | GPIO21 |
		GPIO15 | GPIO14 | GPIO13 | GPIO12, &gpio5_base->setdataout);

	dieid_num_r();
	omap3_dss_enable();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_BEAGLE();
}

