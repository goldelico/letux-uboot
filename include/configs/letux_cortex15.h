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

#ifndef CONFIG_SPL_BUILD

/* Letux Cortex 15 has no tca6424 */
#undef CONFIG_TCA642X
#undef CONFIG_CMD_TCA642X
#undef CONFIG_SYS_I2C_TCA642X_BUS_NUM
#undef CONFIG_SYS_I2C_TCA642X_ADDR

#if 0	/* for the moment, disable to store environment in eMMC */
#undef CONFIG_ENV_IS_IN_MMC
#undef CONFIG_SYS_MMC_ENV_DEV
#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_OFFSET
#endif

#endif /* __CONFIG_LETUX_CORTEX15_H */
