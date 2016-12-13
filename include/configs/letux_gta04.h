/*
 * (C) Copyright 2010
 * Nikolaus Schaller <hns@goldelico.com>
 *
 * Configuration settings for the Goldelico GTA04
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H

#include "letux_beagle.h"	/* share basic configs */

/* Status LED */
/* undef? - we have no such status leds in u-boot */
#define CONFIG_STATUS_LED		1
#define CONFIG_BOARD_SPECIFIC_LED	1
#define STATUS_LED_BIT			0x01
#define STATUS_LED_STATE		STATUS_LED_ON
#define STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BIT1			0x02
#define STATUS_LED_STATE1		STATUS_LED_ON
#define STATUS_LED_PERIOD1		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BOOT			STATUS_LED_BIT
#define STATUS_LED_GREEN		STATUS_LED_BIT1

/* USB EHCI */
/* undef? - we can't boot through modem... */
#define CONFIG_USB_EHCI

/* check me */
#define CONFIG_USB_EHCI_OMAP
#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO	147

#if 0	/* not very useful for GTA04 */
#undef CONFIG_USB_HOST_ETHER
#undef CONFIG_USB_ETHER_ASIX
#undef CONFIG_USB_ETHER_MCS7830
#undef CONFIG_USB_ETHER_SMSC95XX
#endif

/* undef? we have a vibra motor connected */
#define CONFIG_TWL4030_LED		1

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		"GTA04 # "

/* spurious defines - still needed? */
#define CONFIG_DISPLAY_CPUINFO		1
#define CONFIG_DISPLAY_BOARDINFO	1
#define CONFIG_G_DNL_VENDOR_NUM		0x0451
#define CONFIG_G_DNL_PRODUCT_NUM	0xd022
#define CONFIG_SYS_CACHELINE_SIZE	64

#endif /* __CONFIG_H */
