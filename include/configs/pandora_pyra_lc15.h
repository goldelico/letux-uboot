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

#ifndef __CONFIG_PANDORA_PYRA_LC15_H
#define __CONFIG_PANDORA_PYRA_LC15_H

#include <configs/letux_cortex15.h>

/* Re-enable support for the TCA642X GPIO we have on the Pyra mainbord (like uEVM) */
#define CONFIG_TCA642X
#define CONFIG_CMD_TCA642X
#define CONFIG_SYS_I2C_TCA642X_BUS_NUM 4
#define CONFIG_SYS_I2C_TCA642X_ADDR 0x22

#endif /* __CONFIG_PANDORA_PYRA_LC15_H */
