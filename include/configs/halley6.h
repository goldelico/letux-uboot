/*
 * Ingenic X1600 configuration
 *
 * Copyright (c) 2015 Ingenic Semiconductor Co.,Ltd
 * Author: Matthew <xu.guo@ingenic.com>
 * Based on: include/configs/urboard.h
 *           Written by Paul Burton <paul.burton@imgtec.com>
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

#ifndef __HALLEY6__
#define __HALLEY6__

/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32		/* MIPS32 CPU core */
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_X1600
#define CONFIG_SOC_NAME		x1600


#define CONFIG_SYS_APLL_FREQ		1200000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_EPLL_FREQ		200000000	/*If EPLL not use mast be set 0*/
#define CONFIG_CPU_SEL_PLL		APLL
#define CONFIG_DDR_SEL_PLL		MPLL
#define CONFIG_SYS_CPU_FREQ		1200000000
#define CONFIG_SYS_MEM_FREQ		400000000

#define CONFIG_SYS_EXTAL		24000000	/* EXTAL freq: 48 MHz */
#define CONFIG_SYS_HZ			1000		/* incrementer freq */

#define CONFIG_SYS_UART_INDEX		2
#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_DCACHE_SIZE		(16 * 1024)
#define CONFIG_SYS_DCACHELINE_SIZE	(32)
#define CONFIG_SYS_DCACHE_WAYS		(4)
#define CONFIG_SYS_ICACHE_SIZE		(16 * 1024)
#define CONFIG_SYS_ICACHELINE_SIZE	(32)
#define CONFIG_SYS_ICACHE_WAYS		(4)
#define CONFIG_SYS_CACHELINE_SIZE	(32)
/* A switch to configure whether cpu has a 2nd level cache */
#define CONFIG_BOARD_SCACHE
#define CONFIG_SYS_SCACHE_SIZE		(128 * 1024)
#define CONFIG_SYS_SCACHELINE_SIZE	(32)
#define CONFIG_SYS_SCACHE_WAYS		(8)


#define  CGU_CLK_SRC {				\
		{LCD, MPLL},			\
		{MSC0, MPLL},			\
		{SFC, MPLL},			\
		{CIM, MPLL},			\
		{SRC_EOF,SRC_EOF}		\
	}

/*
 *#define CONFIG_DDR_TEST_CPU
 *#define CONFIG_DDR_TEST
 */

#define CONFIG_DDR_DLL_OFF
#define CONFIG_DDR_PARAMS_CREATOR
#define CONFIG_DDR_HOST_CC
/*#define CONFIG_DDR_FORCE_SELECT_CS1*/

#define CONFIG_DDR_TYPE_DDR2
#define CONFIG_DDR_TYPE_LPDDR2

#ifdef CONFIG_DDR_TYPE_LPDDR2
#define CONFIG_LPDDR2_M54D5121632A
#define CONFIG_LPDDR2_SCB4BL256160AFL19GI
#define CONFIG_LPDDR2_SCKX4BL512160AAC
#endif

#ifdef CONFIG_DDR_TYPE_DDR2
#define CONFIG_LVDDR_INNOPHY
#define CONFIG_DDR2_W975116NG18I
#define CONFIG_DDR2_W971GV6NG
#define CONFIG_DDR2_M14D2561616A
#define CONFIG_DDR2_M14D1G1664A
#define CONFIG_LVDDR_W9464L6KH
#endif


#define CONFIG_DDR_INNOPHY
#define CONFIG_DDR_CS0          1   /* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1          0   /* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32         0   /* 1-32bit-width, 0-16bit-width */

#define CONFIG_DDR_PHY_IMPEDANCE 40
#define CONFIG_DDR_PHY_ODT_IMPEDANCE 120


#define CONFIG_DDR_AUTO_SELF_REFRESH
#define CONFIG_DDR_AUTO_SELF_REFRESH_CNT 257


/* #define CONFIG_DDR_DLL_OFF */
/*
 * #define CONFIG_DDR_CHIP_ODT
 * #define CONFIG_DDR_PHY_ODT
 * #define CONFIG_DDR_PHY_DQ_ODT
 * #define CONFIG_DDR_PHY_DQS_ODT
 * #define CONFIG_DDR_PHY_IMPED_PULLUP		0xe
 * #define CONFIG_DDR_PHY_IMPED_PULLDOWN	0xe
 */

/**
 * Boot arguments definitions.
 */

#define CONFIG_BOOTARGS_AUTO_MODIFY	1	/*auto detect memory size, and modify bootargs for kernel.*/
#define CONFIG_BOOTARGS_MEM_32M			"mem=32M@0x0"	/* customize bootargs for default env.*/
#define CONFIG_BOOTARGS_MEM_64M			"mem=64M@0x0"	/* customize bootargs for default env.*/
#define CONFIG_BOOTARGS_MEM_128M		"mem=128M@0x0"

#if (CONFIG_BOOTARGS_AUTO_MODIFY == 1)
	#define BOOTARGS_COMMON "console=ttyS2,115200n8 "
#else
	#define BOOTARGS_COMMON "console=ttyS2,115200n8 mem=64M@0x0 "
#endif
/**
 * Boot command definitions.
 */
#define CONFIG_BOOTDELAY 1

#if defined(CONFIG_SPL_JZMMC_SUPPORT) || defined(CONFIG_SPL_MMC_SUPPORT)
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " rootfstype=ext4 root=/dev/mmcblk0p2 rw"
#elif defined(CONFIG_SPL_NOR_SUPPORT)
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " ip=192.168.10.207:192.168.10.1:192.168.10.1:255.255.255.0 nfsroot=192.168.4.13:/home/nfsroot/fpga/user/pzqi/rootfs-tst rw"
#elif defined(CONFIG_SPL_SFC_NOR)
	#define CONFIG_BOOTARGS BOOTARGS_COMMON "ip=off init=/linuxrc rootfstype=jffs2 root=/dev/mtdblock2 rw flashtype=nor"
#elif defined(CONFIG_SPL_SFC_NAND)
	#define CONFIG_BOOTARGS BOOTARGS_COMMON "ip=off init=/linuxrc ubi.mtd=2 root=ubi0:rootfs ubi.mtd=3 rootfstype=ubifs rw flashtype=nand"
#else
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " ubi.mtd=1 root=ubi0:root rootfstype=ubifs rw"
#endif

#if defined(CONFIG_SPL_NOR_SUPPORT)
	#define CONFIG_BOOTCOMMAND "tftpboot 0x80600000 user/pzqi/uImage; bootm 0x80600000"
/*#define CONFIG_BOOTCOMMAND "loady 0x80600000; bootm 0x80600000"*/
#elif defined(CONFIG_SPL_JZMMC_SUPPORT) || defined(CONFIG_SPL_MMC_SUPPORT)
	#define CONFIG_BOOTCOMMAND "fatload mmc 0 0x81f00000 /ingenic/lx16.dtb; fatload mmc 0 0x80a00000 /uImage; bootm 0x80a00000 - 0x81f00000"
#elif defined(CONFIG_SPL_SFC_NOR)
	#define CONFIG_BOOTCOMMAND "sfcnor read 0x40000 0x600000 0x80a00000 ;bootm 0x80a00000"
#elif defined(CONFIG_SPL_SFC_NAND)
	#define CONFIG_BOOTCOMMAND "sfcnand read 0x100000 0x600000 0x80a00000 ;bootm 0x80a00000"
#else
	#define CONFIG_BOOTCOMMAND						\
	"mtdparts default; ubi part system; ubifsmount ubi:boot; "	\
	"ubifsload 0x80f00000 vmlinux.ub; bootm 0x80f00000"
#endif

#define CONFIG_SPL_MAX_SIZE		26624	/* 18KB */
#define CONFIG_SPL_PAD_TO		26624  /* equal to spl max size in x1600 */
#define CONFIG_UBOOT_OFFSET             26624 /* equal to spl max size in x1600 */

/* security */
/*#define CONFIG_JZ_SCBOOT_TEST*/

/* used for debug */
/* #define CONFIG_SPL_LIBCOMMON_SUPPORT */

#ifdef CONFIG_X1600_NEMC
#define CONFIG_CMD_NEMC
#endif

/* SFC */
#if defined(CONFIG_SPL_SFC_NOR) || defined(CONFIG_SPL_SFC_NAND)
/* serial support */
#define CONFIG_SPL_SERIAL_SUPPORT
/* sfc gpio */
#define CONFIG_SPL_SFC_SUPPORT
/* spl version */
#define CONFIG_SPL_VERSION	1
#endif

/* driver version */
#define CONFIG_SFC_V20

/* sfc nor config */
#ifdef CONFIG_SPL_SFC_NOR
#define CONFIG_JZ_SFC
#define CONFIG_CMD_SFC_NOR
#define CONFIG_JZ_SFC_NOR
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_SFC_NOR_INIT_RATE		100000000
#define CONFIG_SFC_NOR_RATE			100000000	/* value <= 400000000(sfc 100Mhz)*/
#define CONFIG_SFC_QUAD
#define CONFIG_SPIFLASH_PART_OFFSET		0x5800
#define CONFIG_SPI_NORFLASH_PART_OFFSET		0x5874
#define CONFIG_NOR_MAJOR_VERSION_NUMBER		1
#define CONFIG_NOR_MINOR_VERSION_NUMBER		0
#define CONFIG_NOR_REVERSION_NUMBER		0
#define CONFIG_NOR_VERSION     (CONFIG_NOR_MAJOR_VERSION_NUMBER | (CONFIG_NOR_MINOR_VERSION_NUMBER << 8) | (CONFIG_NOR_REVERSION_NUMBER <<16))
/*#define CONFIG_NOR_BUILTIN_PARAMS*/
#endif

/* sfc nand config */
#ifdef  CONFIG_SPL_SFC_NAND
#define CONFIG_JZ_SFC
#define CONFIG_SFC_NAND_INIT_RATE		100000000
#define CONFIG_SFC_NAND_RATE			100000000	/* value <= 400000000(sfc 100Mhz)*/
#define CONFIG_SFC_QUAD
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_SPIFLASH_PART_OFFSET		0x5800
#define CONFIG_SPI_NAND_BPP                     (2048 +64)      /*Bytes Per Page*/
#define CONFIG_SPI_NAND_PPB                     (64)            /*Page Per Block*/
#define CONFIG_CMD_SFCNAND
#define CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			0xb3441000
#define CONFIG_SYS_MAXARGS			16
/*#define CONFIG_NAND_BUILTIN_PARAMS*/

/* sfc nand env config */
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_SAVEENV		/* saveenv */
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS
#define MTDIDS_DEFAULT                  "nand0:nand"
#define MTDPARTS_DEFAULT                "mtdparts=nand:1M(boot),8M(kernel),40M(rootfs),-(data)"
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)
#endif

#ifdef CONFIG_SPL_OS_BOOT
    #if defined(CONFIG_SPL_SFC_NOR)
	     #define CONFIG_SPL_BOOTARGS	BOOTARGS_COMMON "ip=off init=/linuxrc rootfstype=jffs2 root=/dev/mtdblock2 rw"
    #elif defined(CONFIG_SPL_MMC_SUPPORT)
	     #define CONFIG_SPL_BOOTARGS        BOOTARGS_COMMON "ip=off init=/linuxrc rootfstype=ext4 root=/dev/mmcblk0p1 rootdelay=3 rw"
    #else
	     #define CONFIG_SPL_BOOTARGS	BOOTARGS_COMMON "ip=off init=/linuxrc ubi.mtd=2 root=ubi0:rootfs ubi.mtd=3 rootfstype=ubifs rw"
    #endif
    #ifdef CONFIG_OTA_VERSION30
		#define CONFIG_PAT_KERNEL_NAME	  "kernel"
		#define CONFIG_PAT_RECOVERY_NAME  "recovery"
		#define CONFIG_PAT_NV_NAME        "nv"
		#undef CONFIG_SPL_BOOTARGS
		#ifdef CONFIG_SPL_SFC_NOR
			#define CONFIG_PAT_USERFS_NAME   "userfs"
			#define CONFIG_PAT_UPDATEFS_NAME "updatefs"
			#define CONFIG_SPL_BOOTARGS    BOOTARGS_COMMON "ip=off init=/linuxrc rootfstype=cramfs root=/dev/mtdblock5 rw"
		#else
			#define CONFIG_SPL_BOOTARGS    BOOTARGS_COMMON "ip=off init=/linuxrc ubi.mtd=4 root=ubi0:system ubi.mtd=5 rootfstype=ubifs ro"
			#define CONFIG_SPL_OTA_BOOTARGS    BOOTARGS_COMMON "ip=off ubi.mtd=4 ubi.mtd=5 root=/dev/ram0 rw rdinit=/linuxrc"
		#endif
    #else
		#ifdef CONFIG_BOOT_VMLINUX
			#undef CONFIG_SPL_BOOTARGS
			#define CONFIG_SPL_BOOTARGS         BOOTARGS_COMMON "ip=off init=/linuxrc rootfstype=cramfs root=/dev/mtdblock2 rw"
		#endif /* CONFIG_BOOT_VMLINUX */

        #define CONFIG_SOFT_BURNER
    #endif /*CONFIG_OTA_VERSION30*/

     #ifdef CONFIG_BOOT_VMLINUX
             #undef CONFIG_SPL_BOOTARGS
             #define CONFIG_SPL_BOOTARGS         BOOTARGS_COMMON "ip=off init=/linuxrc rootfstype=cramfs root=/dev/mtdblock2 rw"
     #endif /* CONFIG_BOOT_VMLINUX */

     #define CONFIG_SPL_OS_NAME        "kernel" /* spi offset of xImage being loaded */
     #define CONFIG_SYS_SPL_ARGS_ADDR    CONFIG_SPL_BOOTARGS
     #define CONFIG_SYS_SPL_OTA_ARGS_ADDR    CONFIG_SPL_OTA_BOOTARGS
     #define CONFIG_BOOTX_BOOTARGS       BOOTARGS_COMMON "ip=off init=/linuxrc rootfstype=cramfs root=/dev/mtdblock6 rw"
     #undef  CONFIG_BOOTCOMMAND
     #define CONFIG_BOOTCOMMAND    "bootx sfc 0x80f00000"
     #ifdef CONFIG_BOOT_RTOS
             #define CONFIG_LOAD_ADDR	0x80004000
     #else
             #define CONFIG_LOAD_ADDR	0x80001000
     #endif
#endif	/* CONFIG_SPL_OS_BOOT */
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0x1800 /* address 0xa0000 */

#define CONFIG_SYS_NAND_SELF_INIT
/* end of sfc */

/* MMC */
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_JZ_MMC 1

#ifdef CONFIG_JZ_MMC_MSC0
#define CONFIG_JZ_MMC_SPLMSC 0
#define CONFIG_JZ_MMC_MSC0_PC_4BIT 1
#endif
#ifdef CONFIG_JZ_MMC_MSC1
#define CONFIG_JZ_MMC_SPLMSC 1
#define CONFIG_JZ_MMC_MSC1_PD 1
#endif

/* DEBUG ETHERNET */
#define CONFIG_SERVERIP		192.168.4.13
#define CONFIG_IPADDR		192.168.4.145
#define CONFIG_GATEWAYIP        192.168.4.1
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_ETHADDR          00:11:22:33:44:55

#define GMAC_PHY_RMII   2//4
#define CONFIG_SYS_RX_ETH_BUFFER 64

#define CONFIG_NET_X1600
#ifdef CONFIG_NET_X1600
#define CONFIG_MAC_AHB_BUS

/* Select GMAC Controller */
#define CONFIG_GMAC0
/* Select GMAC Interface mode */

#define CONFIG_NET_GMAC_PHY_MODE GMAC_PHY_RMII

#ifdef CONFIG_GMAC0
#define CONFIG_HALLEY6_MAC_POWER_EN
#define CONFIG_GAMAC_MODE_CTRL_ADDR	0xb00000e4
#define JZ_GMAC_BASE			0xb34b0000
#define CONFIG_GMAC_CRLT_PORT GPIO_PORT_B
#define CONFIG_GMAC_CRLT_PORT_PINS (0x3ff << 19)
#define CONFIG_GMAC_CRTL_PORT_INIT_FUNC GPIO_FUNC_1
#define CONFIG_GMAC_PHY_RESET	GPIO_PB(31)
#define CONFIG_GMAC_TX_CLK_DELAY 0x3f
#define CONFIG_GMAC_RX_CLK_DELAY 0
#endif

#define CONFIG_GMAC_CRTL_PORT_SET_FUNC GPIO_INPUT
#define CONFIG_GMAC_PHY_RESET_ENLEVEL	0
#endif /* CONFIG_NET_X1600 */


/* GPIO */
#define CONFIG_JZ_GPIO

/**
 * Command configuration.
 */
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_DHCP 	/* DHCP support			*/
#define CONFIG_CMD_ECHO		/* echo arguments		*/
#define CONFIG_CMD_EXT4 	/* ext4 support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_USE_XYZMODEM	/* xyzModem 			*/
#define CONFIG_CMD_LOAD		/* serial load support 		*/
#define CONFIG_CMD_LOADB	/* loadb			*/
#define CONFIG_CMD_LOADS	/* loads			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#if 1
#define CONFIG_LOOPW
#define CONFIG_CMD_MEMTEST
#endif
#if 0
#define CONFIG_SYS_ALT_MEMTEST
#endif
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#define CONFIG_CMD_MMC		/* MMC/SD support			*/
#if 1
#define CONFIG_OF_LIBFDT	/* fdt				*/
#define CONFIG_LMB		/* image_setup_linux() support	*/

#endif
#define CONFIG_CMD_NET		/* networking support			*/
#define CONFIG_CMD_PING
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/
#define CONFIG_CMD_SOURCE	/* "source" command support	*/
#define CONFIG_CMD_GETTIME
#define CONFIG_CMD_GPIO
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_FAT
#define CONFIG_CMD_SAVEENV	/* saveenv			*/
#define CONFIG_EFI_PARTITION
#define CONFIG_SOFT_BURNER

/* USB */
/*#define CONFIG_CMD_FASTBOOT*/
/*#define CONFIG_USB_GADGET*/
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_JZ_DWC2_UDC_V1_1
#define CONFIG_FASTBOOT_GADGET
#define CONFIG_FASTBOOT_FUNCTION
#define CONFIG_G_FASTBOOT_VENDOR_NUM	(0x18d1)
#define CONFIG_G_FASTBOOT_PRODUCT_NUM	(0xdddd)
#define CONFIG_USB_GADGET_VBUS_DRAW 500

/**
 * Serial download configuration
 */
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */

/**
 * Miscellaneous configurable options
 */
#define CONFIG_DOS_PARTITION

#define CONFIG_LZO
#define CONFIG_RBTREE

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_BASE	0 /* init flash_base as 0 */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_MISC_INIT_R 1

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAUL)

#define CONFIG_SYS_MAXARGS 16
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "# "
#define CONFIG_SYS_CBSIZE 1024 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_MONITOR_LEN		(768 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)

/*#define DBG_USE_UNCACHED_MEMORY*/
#ifdef DBG_USE_UNCACHED_MEMORY
#define CONFIG_SYS_SDRAM_BASE		0xA0000000 /* uncached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0xB0000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0xA8000000
#define CONFIG_SYS_MEMTEST_START	0xA0000000
#define CONFIG_SYS_MEMTEST_END		0xA8000000

#define CONFIG_SYS_TEXT_BASE		0xA0100000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#else /* DBG_USE_UNCACHED_MEMORY */
#define CONFIG_SYS_SDRAM_BASE		0x80000000 /* cached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0x84000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0x80010000
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x82000000

#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#endif /* DBG_USE_UNCACHED_MEMORY */

/**
 * Environment
 */
#ifdef  CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE         (32 << 10)
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#elif defined(CONFIG_ENV_IS_IN_SFC)
#define CONFIG_CMD_SFC_NOR
#define CONFIG_ENV_SIZE                 (4 << 10)
#define CONFIG_ENV_OFFSET               0x3f000 /*write nor flash 252k address*/
#define CONFIG_CMD_SAVEENV

#else
/* nand Environment variables */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE	CONFIG_SYS_NAND_BLOCK_SIZE /* 128K */
#define SPI_NAND_BLK            CONFIG_SYS_NAND_BLOCK_SIZE /* the spi nand block size */
#define CONFIG_ENV_SIZE         SPI_NAND_BLK /* uboot is 1M but the last block size is the env */
#define CONFIG_ENV_OFFSET       (CONFIG_SYS_NAND_BLOCK_SIZE * 6) /* offset is 768k */
#define CONFIG_ENV_OFFSET_REDUND (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_IS_IN_SFC_NAND
#endif

/**
 * SPL configuration
 */
#define CONFIG_SPL_FRAMEWORK

#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH		"$(CPUDIR)/$(SOC)"
#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-nor-spl.lds"
#else
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-spl.lds"
#endif

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	86//0x5A //wli changed 0x20 /* 16KB offset */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x400 /* 512 KB */
#define CONFIG_SYS_NAND_U_BOOT_OFFS	(CONFIG_SYS_NAND_BLOCK_SIZE * 4)
#define CONFIG_SYS_NAND_U_BOOT_DST	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_NAND_U_BOOT_DST
#define CONFIG_SYS_NAND_U_BOOT_SIZE	(512 * 1024)

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
/* #define CONFIG_SPL_I2C_SUPPORT */
/* #define CONFIG_SPL_REGULATOR_SUPPORT */
#define CONFIG_SPL_CORE_VOLTAGE		1100

#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_TEXT_BASE		0xba000000
#else
#define CONFIG_SPL_TEXT_BASE		0x80001000
#endif	/*CONFIG_SPL_NOR_SUPPORT*/

#ifdef CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_MMC_SPL_PARAMS
/*#define CONFIG_SPL_PARAMS_FIXER*/
#endif /* CONFIG_SPL_MMC_SUPPORT */

#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SYS_UBOOT_BASE		(CONFIG_SPL_TEXT_BASE + CONFIG_SPL_PAD_TO - 0x40)	//0x40 = sizeof (image_header)
#define CONFIG_SYS_OS_BASE		0
#define CONFIG_SYS_SPL_ARGS_ADDR	0
#define CONFIG_SYS_FDT_BASE		0
#endif

/**
 * GPT configuration
 */
#ifdef CONFIG_GPT_CREATOR
#define CONFIG_GPT_TABLE_PATH	"$(TOPDIR)/board/$(BOARDDIR)"
#else
/* USE MBR + zero-GPT-table instead if no gpt table defined*/
#define CONFIG_MBR_P0_OFF	64mb
#define CONFIG_MBR_P0_END	556mb
#define CONFIG_MBR_P0_TYPE 	linux

#define CONFIG_MBR_P1_OFF	580mb
#define CONFIG_MBR_P1_END 	1604mb
#define CONFIG_MBR_P1_TYPE 	linux

#define CONFIG_MBR_P2_OFF	28mb
#define CONFIG_MBR_P2_END	58mb
#define CONFIG_MBR_P2_TYPE 	linux

#define CONFIG_MBR_P3_OFF	1609mb
#define CONFIG_MBR_P3_END	7800mb
#define CONFIG_MBR_P3_TYPE 	fat
#endif

/**
 * Keys.
 */
#define CONFIG_GPIO_USB_DETECT		GPIO_PA(14)
#define CONFIG_GPIO_USB_DETECT_ENLEVEL	1

#define CONFIG_GPIO_PWR_WAKE		GPIO_PA(30)
#define CONFIG_GPIO_PWR_WAKE_ENLEVEL	0

#endif
