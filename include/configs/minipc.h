/*
 * (C) Copyright 2005
 * Ingenic Semiconductor, <jlwei@ingenic.cn>
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

/*
 * This file contains the configuration parameters for the pmp board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MIPS32		1  /* MIPS32 CPU core */
#define CONFIG_JzRISC		1  /* JzRISC core */
#define CONFIG_JZSOC		1  /* Jz SoC */
#define CONFIG_JZ4730		1  /* Jz4730 SoC */
#define CONFIG_MINIPC           1

////#define CONFIG_LCD                 /* LCD support */
////#define CONFIG_JZLCD_SAMSUNG_LTP400WQF01 /* LCD panel */
////#define LCD_BPP			4  /* 4: 16bits per pixel */
////#define CFG_WHITE_ON_BLACK
////#define CONFIG_LCD_LOGO
//#define CFG_LCD_LOGOONLY_NOINFO

#define CFG_CPU_SPEED		336000000	/* CPU speed */

#define CFG_EXTAL		3686400		/* EXTAL freq: 12 MHz */
#define	CFG_HZ			(CFG_CPU_SPEED/(3*256)) /* incrementer freq */

#define CFG_UART_BASE  		UART0_BASE	/* Base of the UART channel */
#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_MMC      1		//wjx
#define CONFIG_FAT      1    
#define CONFIG_SUPPORT_VFAT 1

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL | \
				 CFG_CMD_ASKENV | \
         CFG_CMD_NAND   | \
				 CFG_CMD_DHCP	| \
				 CFG_CMD_PING	|	\
				 CFG_CMD_MMC	|	\
				 CFG_CMD_FAT	)
		 

#define CONFIG_BOOTP_MASK	( CONFIG_BOOTP_DEFAUL )

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	1
#define CONFIG_BOOTFILE	        "uImage"	/* file to load */
//#define CONFIG_BOOTARGS		"mem=128M console=ttyS0,115200n8 ip=dhcp nfsroot=192.168.1.15:/nfsroot/pmp-root rw noatime"
#define CONFIG_BOOTARGS		"mem=128M console=ttyS0,115200n8 root=/dev/ssfdca1 rw noatime"	//wjx
//#define CONFIG_BOOTCOMMAND	"bootp;setenv serverip 192.168.1.20;tftp;bootm"
#define CONFIG_BOOTCOMMAND	"nboot 0x80600000 0 0x100000;bootm"	//wjx
#define CONFIG_BOOTCOMMAND1	"nboot 0x80600000 0 0x280000;bootm"	//wjx

#define CONFIG_BOOTCOMMAND_SD1	"mmcinit;fatload mmc 0 0x80600000 uimage;bootm"	//wjx load the kernel from sd and boot

#define CONFIG_BOOTCOMMAND_SD2	"mmcinit;fatload mmc 0 0x80600000 uimage;nand erase 0x100000 0x180000 ;nand write 0x80600000 0x100000 0x180000 ;bootm"	//wjx load the kernel from sd and nand write to nand ,boot

#define CONFIG_BOOTCOMMAND_SD_MINIFS	"mmcinit;fatload mmc 0 0x80600000 minifs.img;nand erase 0x500000 0x500000 ;nand write 0x80600000 0x500000 0x500000 ;boot"	//wjx load the kernel from sd and nand write to nand ,boot

#define CONFIG_BOOTCOMMAND_SD_ALL	"mmcinit;fatload mmc 0 0x80600000 minipc.img;nand erase 0 0xa00000 ;nand write 0x80600000 0 0xa00000 ;boot"	//wjx load the bootloader,two kernel, mini_rootfs from sd and nand write to nand ,boot

#define CONFIG_TFTP_MINIPC	"set bootfile minipc.img ;tftp ;nand read 0x82000000 0x400000 0x100000 ;nand erase 0 0xa00000 ;nand write 0x80600000 0 0xa00000 ;nand erase 0x400000 0x100000 ;nand write 0x82000000 0x400000 0x100000 ;bootm"	//hjf load the bootloader,two kernel,minifs from tftp server and nand write to nand ,boot

#define CONFIG_TFTP_BOOT	"set bootfile u-boot ;tftp ;nand erase 0 0x100000 ;nand write 0x80600000 0 0x100000 ;bootm"	//hjf load the bootloader from tftp server nand write to nand ,boot

#define CONFIG_TFTP_KERNEL	"set bootfile uImage-2 ;tftp ;nand erase 0x100000 0x300000 ;nand write 0x80600000 0x100000 0x300000 ;bootm"	//hjf load the two kernel from tftp server and nand write to nand ,boot

#define CONFIG_TFTP_MINIFS	"set bootfile minifs.img ;tftp ;nand erase 0x500000 0x500000 ;nand write 0x80600000 0x500000 0x500000 ;bootm"	//hjf load the minifs from tftp server and nand write to nand ,boot

#define CFG_AUTOLOAD		"n"		/* No autoload */

#define CONFIG_NET_MULTI
#define CONFIG_ETHADDR		00:52:c2:2a:bd:f5    /* Ethernet address */
#define CONFIG_IPADDR		192.168.1.10		//wjx set ipaddress
#define CONFIG_SERVERIP		192.168.1.128		//wjx set the tftp server ip
/*
 * Serial download configuration
 *
 */
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory      */
#define	CFG_PROMPT		"PMP # "	/* Monitor Command Prompt    */
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size   */
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)  /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args*/

#define CFG_MALLOC_LEN		128*1024
#define CFG_BOOTPARAMS_LEN	128*1024

#define CFG_SDRAM_BASE		0x80000000     /* Cached addr */

#define CFG_INIT_SP_OFFSET	0x400000

#define	CFG_LOAD_ADDR		0x80600000     /* default load address	*/

#define CFG_MEMTEST_START	0x80100000
#define CFG_MEMTEST_END		0x80800000

#define CFG_RX_ETH_BUFFER	16	/* use 16 rx buffers on jz4730 eth */

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/
#else
#define CFG_ENV_IS_IN_NAND	1	/* use NAND for environment vars	*/
#endif

/*-----------------------------------------------------------------------
 * NAND FLASH configuration
 */
#define CFG_MAX_NAND_DEVICE     1
#define NAND_MAX_CHIPS          1
#define CFG_NAND_BASE           0xB4000000
#define CFG_NAND_SELECT_DEVICE  1       /* nand driver supports mutipl. chips   */

/*
 * IPL (Initial Program Loader, integrated inside CPU)
 * Will load first 4k from NAND (SPL) into cache and execute it from there.
 *
 * SPL (Secondary Program Loader)
 * Will load special U-Boot version (NUB) from NAND and execute it. This SPL
 * has to fit into 4kByte. It sets up the CPU and configures the SDRAM
 * controller and the NAND controller so that the special U-Boot image can be
 * loaded from NAND to SDRAM.
 *
 * NUB (NAND U-Boot)
 * This NAND U-Boot (NUB) is a special U-Boot version which can be started
 * from RAM. Therefore it mustn't (re-)configure the SDRAM controller.
 *
 */
#define CFG_NAND_U_BOOT_DST	0x80100000	/* Load NUB to this addr	*/
#define CFG_NAND_U_BOOT_START	CFG_NAND_U_BOOT_DST /* Start NUB from this addr	*/

/*
 * Define the partitioning of the NAND chip (only RAM U-Boot is needed here)
 */
#define CFG_NAND_U_BOOT_OFFS	(128 << 10)	/* Offset to RAM U-Boot image	*/
#define CFG_NAND_U_BOOT_SIZE	(512 << 10)	/* Size of RAM U-Boot image	*/

#define CFG_NAND_BLOCK_SIZE	(128 << 10)	/* NAND chip block size		*/

#ifdef CFG_ENV_IS_IN_NAND
#define CFG_ENV_SIZE		CFG_NAND_BLOCK_SIZE
#define CFG_ENV_OFFSET		(CFG_NAND_U_BOOT_OFFS + CFG_NAND_U_BOOT_SIZE + CFG_NAND_BLOCK_SIZE)	/* environment starts here  */
#define CFG_ENV_OFFSET_REDUND	(CFG_ENV_OFFSET + CFG_ENV_SIZE)
#endif

/*-----------------------------------------------------------------------
 * Flash configuration
 * (SST 39VF6401 16-bit Flash): 2KWord(4KB) * 2048 sectors = 8MB
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	2048	/* max number of sectors on one chip */

#define PHYS_FLASH_1		0xbf800000   /* Flash Bank #1 */
#define CFG_FLASH_BASE		PHYS_FLASH_1 /* Flash at 0xbf800000 - 0xbfffffff */

#define	CFG_MONITOR_BASE	0xbfc00000
#define	CFG_MONITOR_LEN		(256*1024)  /* Reserve 256 kB for Monitor */

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x10000 /* Total Size of Environment Sector */
#define CFG_ENV_SIZE		CFG_ENV_SECT_SIZE
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN) /* Environment after Monitor */

#define CFG_DIRECT_FLASH_TFTP	1	/* allow direct tftp to flash */
#endif

#define CONFIG_ENV_OVERWRITE	1	/* allow overwrite MAC address */

/*
 *  SDRAM Info.
 */
#define CONFIG_NR_DRAM_BANKS	1

/* zxl */
#define CONFIG_CHIP_HY57V561620 1	//wjx:
//#define CONFIG_CHIP_K4S511632C  1


#if defined(CONFIG_CHIP_K4S511632C)
	#define CONFIG_MB_PER_BANK 128
	#define SDRAM_ROW          13
	#define SDRAM_COL          10
	#define SDRAM_BANK4        1
#elif defined(CONFIG_CHIP_HY57V561620)
	#define CONFIG_MB_PER_BANK      64
	#define SDRAM_ROW               13
	#define SDRAM_COL               9
	#define SDRAM_BANK4             1
#else
#error
#endif

#if CONFIG_MB_PER_BANK == 64
  #define CONFIG_DMAR1_VAL        0x20F4
  #define CONFIG_DMAR2_VAL        0x24F4
#elif CONFIG_MB_PER_BANK == 32
  #define CONFIG_DMAR1_VAL        0x20F2
  #define CONFIG_DMAR2_VAL        0x22F2
#elif CONFIG_MB_PER_BANK == 128
  #define CONFIG_DMAR1_VAL        0x20F8
  #define CONFIG_DMAR2_VAL        0x28F8
#else
#error
#endif

// SDRAM paramters
#define SDRAM_BW16		0	/* Data bus width: 0-32bit, 1-16bit */
//#define SDRAM_BANK4		1	/* Banks each chip: 0-2bank, 1-4bank */
//#define SDRAM_ROW		13	/* Row address: 11 to 13 */
//#define SDRAM_COL		9	/* Column address: 8 to 12 */
#define SDRAM_CASL		2	/* CAS latency: 2 or 3 */

// SDRAM Timings, unit: ns
#define SDRAM_TRAS		45	/* RAS# Active Time */
#define SDRAM_RCD		20	/* RAS# to CAS# Delay */
#define SDRAM_TPC		20	/* RAS# Precharge Time */
#define SDRAM_TRWL		7	/* Write Latency Time */
#define SDRAM_TREF	        7812	/* Refresh period: 8192 refresh cycles/64ms */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384
#define CFG_ICACHE_SIZE		16384
#define CFG_CACHELINE_SIZE	32

/*-----------------------------------------------------------------------
 * GPIO definition
 */
#define GPIO_PW_I		97
#define GPIO_PW_O		66
#define GPIO_LED_EN		92
#define GPIO_DISP_OFF_N		93
#define GPIO_RTC_IRQ		96
#define GPIO_USB_CLK_EN		29
#define GPIO_CHARG_STAT		125
#define GPIO_SD_VCC_EN_N	21
#define GPIO_SD_CD_N		64
#define GPIO_SD_WP		82

#define GPIO_NAND_FLASH_WP	87   // nand flash write protect
#define GPIO_CAPSLOCK_LED	27   // 
#define GPIO_NUMLOCK_LED	86   // 
#define GPIO_NETWORK_LED	9   // 
#endif	/* __CONFIG_H */
