/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated.
 * Sricharan R	  <r.sricharan@ti.com>
 *
 * Configuration settings for the TI EVM5430 board.
 * See ti_omap5_common.h for omap5 common settings.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_LETUX_CORTEX15_H
#define __CONFIG_LETUX_CORTEX15_H

#include <configs/omap5_uevm.h>

#undef CONFIG_OMAP_EHCI_PHY3_RESET_GPIO	/* gpio 79 used differently */
#undef CONFIG_USB_HOST_ETHER
#undef CONFIG_USB_ETHER_SMSC95XX

/* Letux Cortex 15 has no tca6424 */
#undef CONFIG_TCA642X
#undef CONFIG_SYS_I2C_TCA642X_BUS_NUM
#undef CONFIG_SYS_I2C_TCA642X_ADDR

#if 0	/* for the moment, disable to store environment in eMMC */
#undef CONFIG_ENV_IS_IN_MMC
#undef CONFIG_SYS_MMC_ENV_DEV
#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_OFFSET
#endif

/* should this better go to omap5_uevm? */

#ifdef CONFIG_USB_GADGET
#define CONFIG_USB_FUNCTION_MASS_STORAGE
#endif

#define CONFIG_SCSI
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)

#endif /* __CONFIG_LETUX_CORTEX15_H */
