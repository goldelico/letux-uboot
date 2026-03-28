/*
 * Ingenic burner configuration
 *
 * Copyright (c) 2024  Ingenic Semiconductor Co.,Ltd
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

#ifndef __BURNER_H__
#define __BURNER_H__

/*
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32		        /* MIPS32 CPU core */
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_X1000
#define CONFIG_XBURST_TRAPS


#define CONFIG_SYS_MPLL_FREQ		0
#define CONFIG_SYS_APLL_FREQ		0
#define CONFIG_SYS_MEM_FREQ		0
#define CONFIG_SYS_CPU_FREQ		0
#define CONFIG_CPU_SEL_PLL		APLL
#define CONFIG_DDR_SEL_PLL		MPLL

/*
 *  Cache
 */
#define CONFIG_SYS_DCACHE_SIZE		16384
#define CONFIG_SYS_DCACHELINE_SIZE	(32)
#define CONFIG_SYS_DCACHE_WAYS		(4)
#define CONFIG_SYS_ICACHE_SIZE		16384
#define CONFIG_SYS_ICACHELINE_SIZE	(32)
#define CONFIG_SYS_ICACHE_WAYS		(4)
#define CONFIG_SYS_CACHELINE_SIZE	32


/* CLK CGU */
#define  CGU_CLK_SRC {				\
		{OTG, EXCLK},			\
		{MSC, MPLL},			\
		{SFC, MPLL},			\
		{SRC_EOF,SRC_EOF}		\
	}

/*
 * Uart
 */
#define CONFIG_SYS_UART2_PC
#define CONFIG_SYS_UART_INDEX           2

/*
 * Burner
 */

#define CONFIG_SPIFLASH_PART_OFFSET	    0x3c00

#define CONFIG_USB_PRODUCT_ID               0x1000
#define CONFIG_USB_VENDOR_ID                0xa108
#define CONFIG_BURNER_CPU_INFO              "X1000"


/*
 * Drivers configuration.
 */

/* MMC */
#define CONFIG_MMC
#define CONFIG_JZ_MMC
#define CONFIG_JZ_MMC_MSC0
#define CONFIG_JZ_MMC_MSC1
#define CONFIG_GENERIC_MMC

/* Nor */
#define CONFIG_MTD_SFCNOR
#define CONFIG_SFC_NOR_RATE                 100000000
#define CONFIG_CMD_SFC_NOR
#define CONFIG_JZ_SFC
#define CONFIG_JZ_SFC_NOR
#define CONFIG_SPL_VERSION_OFFSET           16
#define CONFIG_SPI_NORFLASH_PART_OFFSET     0x3c74


/* Nand */
#define CONFIG_MTD_SFCNAND
#define CONFIG_SFC_NAND_RATE	            100000000

#define CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE          1
#define CONFIG_SYS_NAND_BASE                0
#define CONFIG_SYS_NAND_SELF_INIT

#define CONFIG_CMD_UBI
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define MTDIDS_DEFAULT                      "nand0=nand"
#define CONFIG_MTD_UBI_BEB_LIMIT            20
#define CONFIG_LZO
#define CONFIG_RBTREE


/* EFUSE */
#define CONFIG_CMD_EFUSE
#define CONFIG_JZ_EFUSE
#define CONFIG_JZ_SCBOOT


/**
 * SPL configuration
 */
#define CONFIG_SPL_GINFO_BASE		0xf4001000
#define CONFIG_SPL_GINFO_SIZE		0x800

#define CONFIG_SPL_TEXT_BASE		0xf4001800
#define CONFIG_SPL_MAX_SIZE		CONFIG_SPIFLASH_PART_OFFSET

#include "burner_common.h"

#endif /* __BURNER_H__ */
