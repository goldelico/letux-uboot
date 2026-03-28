#ifndef __X2600_BASE_COMMON_H__
#define	__X2600_BASE_COMMON_H__

/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32		/* MIPS32 CPU core */
#define CONFIG_CPU_XBURST2
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_X2600		/* x2600 SoC */

#if defined(CONFIG_X2670_DDR) || defined(CONFIG_X2670M_DDR)
#include "x2670_ddr.h"
#else
#include "x2600_ddr.h"
#endif

#define CONFIG_SYS_APLL_FREQ		1152000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_FREQ		1800000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_EPLL_FREQ		300000000	/*If MPLL not use mast be set 0*/
#define CONFIG_CPU_SEL_PLL		APLL
#define CONFIG_DDR_SEL_PLL		MPLL
#define CONFIG_SYS_CPU_FREQ		1152000000
#define CONFIG_SYS_MEM_FREQ		900000000

#define CONFIG_SYS_AHB0_FREQ		360000000
#define CONFIG_SYS_AHB2_FREQ		300000000	/*APB = AHB2/2*/

#ifdef CONFIG_QUICK_START
#define CONFIG_SPL_RTOS_LOAD_KERNEL
#define CONFIG_RTOS_SIZE_MB 32
#endif

/* Device Tree Configuration*/
/*#define CONFIG_OF_LIBFDT 1*/
#ifdef CONFIG_OF_LIBFDT
#define IMAGE_ENABLE_OF_LIBFDT  1
#define CONFIG_LMB
#endif

#ifdef CONFIG_SPL_OF_LIBFDT

#ifndef CONFIG_DTB_ADRESS
#define CONFIG_DTB_ADRESS 0x80E80000
#endif

#ifndef CONFIG_DTB_SIZE
#define CONFIG_DTB_SIZE 0x20000
#endif

#ifndef CONFIG_DTB_NAME
#define CONFIG_DTB_NAME "dtb"
#endif

#endif /* CONFIG_SPL_OF_LIBFDT */

/* CLK CGU */
#define  CGU_CLK_SRC {				\
		{LCD, MPLL},			\
		{MSC0, MPLL},			\
		{SFC, MPLL},			\
		{SRC_EOF,SRC_EOF}		\
	}

#define CONFIG_SYS_EXTAL		24000000	/* EXTAL freq: 24 MHz */
#define CONFIG_SYS_OST_FREQ		12500000	/* on fpga, 12.5MHz */
#define CONFIG_SYS_HZ			1000		/* incrementer freq */

/**
 *  Cache Configs:
 *  	Must implement DCACHE/ICACHE SCACHE according to xburst spec.
 * */
#define CONFIG_SYS_DCACHE_SIZE		(32 * 1024)
#define CONFIG_SYS_DCACHELINE_SIZE	(32)
#define CONFIG_SYS_DCACHE_WAYS		(8)
#define CONFIG_SYS_ICACHE_SIZE		(32 * 1024)
#define CONFIG_SYS_ICACHELINE_SIZE	(32)
#define CONFIG_SYS_ICACHE_WAYS		(8)
#define CONFIG_SYS_CACHELINE_SIZE	(32)
/* A switch to configure whether cpu has a 2nd level cache */
#define CONFIG_BOARD_SCACHE
#define CONFIG_SYS_SCACHE_SIZE		(256 * 1024)
#define CONFIG_SYS_SCACHELINE_SIZE	(64)
#define CONFIG_SYS_SCACHE_WAYS		(8)

/*
 * uart setting
 */
#ifndef CONFIG_SYS_UART_INDEX
#define CONFIG_SYS_UART_INDEX		2
#endif

#ifndef CONFIG_BAUDRATE
#define CONFIG_BAUDRATE			3000000
#endif

#if defined(CONFIG_SPL_SFC_NOR) || defined(CONFIG_SPL_SFC_NAND)
#define CONFIG_SPL_SFC_SUPPORT
#define CONFIG_SPL_VERSION	1
#endif

#ifdef CONFIG_JZ_WATCHDOG
#define CONFIG_HW_WATCHDOG
#define CONFIG_WDT_TIMEOUT_BY_MS (20*000)
#endif

/**
 * Boot command definitions.
 */
#define CONFIG_BOOTDELAY 1

/**
 * Drivers configuration.
 */

/* MMC */
#ifdef CONFIG_JZ_MMC_MSC0

#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_MMC_SPL_PARAMS
#define CONFIG_SDHCI
#define CONFIG_JZ_SDHCI

#ifndef CONFIG_JZ_SCBOOT
#ifdef CONFIG_BOOT_FAST_FIXED
#define CONFIG_MMC_SDMA
#define MSC_INIT_CLK                      (400 * 1000)  /* 400k */
#define MSC_WORKING_CLK                   (100 * 1000000)    /* 100M */
#endif /* end of CONFIG_BOOT_FAST_FIXED */
#endif

/* MSC Command configuration */
#define CONFIG_CMD_MMC

#define CONFIG_JZ_MMC_MSC0_PD   //set gpio
/*#define CONFIG_MMC_TRACE		// only for DEBUG*/
/*#define CONFIG_SDHCI_TRACE	// only for DEBUG*/
#endif

#ifdef CONFIG_JZ_MMC_MSC1
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_SDHCI
#define CONFIG_MMC_SPL_PARAMS
#define CONFIG_JZ_SDHCI

#ifndef CONFIG_JZ_SCBOOT
#ifdef CONFIG_BOOT_FAST_FIXED
#define CONFIG_MMC_SDMA
#define MSC_INIT_CLK                      (400 * 1000)  /* 400k */
#define MSC_WORKING_CLK                   (50 * 1000000)    /* 100M */
#endif /* end of CONFIG_BOOT_FAST_FIXED */
#endif

/* MSC Command configuration */
#define CONFIG_CMD_MMC

/*#define CONFIG_JZ_MMC_MSC1_PC   //set gpio*/
#define CONFIG_JZ_MMC_MSC1_PD   //set gpio
/*#define CONFIG_MMC_TRACE		// only for DEBUG*/
/*#define CONFIG_SDHCI_TRACE	// only for DEBUG*/
#endif

/* SFC */
#define CONFIG_SFC_V20
#define CONFIG_JZ_SFC_PD

#if defined(CONFIG_X2660_DDR)
/* sfc ota config */
#ifdef CONFIG_OTA_VERSION30
#ifdef CONFIG_SPL_SFC_NAND
#define CONFIG_KUNPENG_OTA_VERSION20
#elif CONFIG_JZ_MMC_MSC2
#define CONFIG_JZSD_OTA_VERSION20
#endif
#endif /*end of ota*/
#endif

/* sfc nor config */
#ifdef CONFIG_SPL_SFC_NOR
#define CONFIG_JZ_SFC
#define CONFIG_CMD_SFC_NOR
#define CONFIG_JZ_SFC_NOR
#define CONFIG_SPI_SPL_CHECK
#ifdef  CONFIG_QUICK_START
#define CONFIG_SFC_NOR_INIT_RATE 300000000
#define CONFIG_SFC_NOR_RATE	300000000	/* value <= 400000000(sfc 100Mhz)*/
#else
#define CONFIG_SFC_NOR_RATE	200000000	/* value <= 400000000(sfc 100Mhz)*/
#endif
#define CONFIG_SFC_QUAD
#define CONFIG_SPIFLASH_PART_OFFSET		0x5800
#define CONFIG_SPI_NORFLASH_PART_OFFSET		0x5874
#define CONFIG_NOR_MAJOR_VERSION_NUMBER		1
#define CONFIG_NOR_MINOR_VERSION_NUMBER		0
#define CONFIG_NOR_REVERSION_NUMBER		0
#define CONFIG_NOR_VERSION     (CONFIG_NOR_MAJOR_VERSION_NUMBER | (CONFIG_NOR_MINOR_VERSION_NUMBER << 8) | (CONFIG_NOR_REVERSION_NUMBER <<16))
#define CONFIG_FLASH_TYPE "flashtype=nor"
/*#define CONFIG_NOR_BUILTIN_PARAMS*/
/*#define CONFIG_NOR_COMMON_PARAMS*/
/*#define CONFIG_NOR_COMMON_PARAMS_COUNT          3*/
#endif

/* sfc nand config */
#ifdef  CONFIG_SPL_SFC_NAND
#ifdef  CONFIG_QUICK_START
#define CONFIG_SFC_NAND_INIT_RATE 300000000
#define CONFIG_SFC_NAND_RATE    300000000	/* value <= 400000000(sfc 100Mhz)*/
#else
#define CONFIG_SFC_NAND_RATE    200000000	/* value <= 400000000(sfc 100Mhz)*/
#endif
#define CONFIG_SFC_QUAD
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_SPIFLASH_PART_OFFSET		0x5800
#ifdef CONFIG_SPI_NAND_BPP_4K
#define CONFIG_SPI_NAND_BPP                     (4096 + 128)      /*Bytes Per Page*/
#else
#define CONFIG_SPI_NAND_BPP                     (2048 +64)      /*Bytes Per Page*/
#endif
#define CONFIG_SPI_NAND_PPB                     (64)            /*Page Per Block*/
#define CONFIG_JZ_SFC
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
#define CONFIG_FLASH_TYPE "flashtype=nand"
#endif

#define CONFIG_SYS_NAND_SELF_INIT

/* end of sfc */

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
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
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
#define CONFIG_CMD_RISCV


#define CONFIG_CMD_DDR_TEST	/* DDR Test Command */

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

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(16 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000 /* cached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0x90000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0x88000000
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x88000000

#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_SC_TEXT_BASE     0x80100004
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_UBOOT_OFFSET             0x6000

/**
 * Environment
 */
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
#if defined(CONFIG_X2670_DDR) || defined(CONFIG_X2600E_DDR)
/* nand Environment variables */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE	CONFIG_SYS_NAND_BLOCK_SIZE /* 128K */
#define SPI_NAND_BLK            CONFIG_SYS_NAND_BLOCK_SIZE /* the spi nand block size */
#define CONFIG_ENV_SIZE         SPI_NAND_BLK /* uboot is 1M but the last block size is the env */
#define CONFIG_ENV_OFFSET       (CONFIG_SYS_NAND_BLOCK_SIZE * 6) /* offset is 768k */
#define CONFIG_ENV_OFFSET_REDUND (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_IS_IN_SFC_NAND
#else
/*
#define CONFIG_ENV_IS_IN_NAND
*/
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_NAND_BLOCK_SIZE * 5)
#endif
#endif

/* spl secure load kernel */
#ifdef CONFIG_JZ_SCBOOT
#define CONFIG_JZ_SECURE_SUPPORT
#define CONFIG_JZ_CKEYAES
#endif

/**
 * SPL configuration
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK

#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH		"$(CPUDIR)/$(SOC)"
#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-nor-spl.lds"
#else
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-spl.lds"
#endif
#define CONFIG_SPL_PAD_TO		24576 /* equal to spl max size */

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
/* #define CONFIG_SPL_I2C_SUPPORT */
/* #define CONFIG_SPL_REGULATOR_SUPPORT */
/* #define CONFIG_SPL_CORE_VOLTAGE		1300 */
#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_TEXT_BASE		0xba000000
#else
#define CONFIG_SPL_TEXT_BASE		0x80001000
#endif	/*CONFIG_SPL_NOR_SUPPORT*/
#define CONFIG_SPL_MAX_SIZE		(24 * 1024)


#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SYS_UBOOT_BASE		(CONFIG_SPL_TEXT_BASE + CONFIG_SPL_PAD_TO - 0x40)	//0x40 = sizeof (image_header)
#define CONFIG_SYS_OS_BASE		0
#define CONFIG_SYS_SPL_ARGS_ADDR	0
#define CONFIG_SYS_FDT_BASE		0
#endif

/* MMC  spl stage */
#if defined(CONFIG_SPL_MMC_SUPPORT) || defined(CONFIG_SPL_JZMMC_SUPPORT)
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	82 /* 17k + 24k (17KB GPT offset and spl size CONFIG_SPL_PAD_TO) */
#define CONFIG_CMD_SAVEENV  /* saveenv */
#define CONFIG_SPL_JZ_MSC_BUS_4BIT	//only for emmc
  #ifdef CONFIG_SPL_JZ_MSC_BUS_8BIT
  #define CONFIG_JZ_MMC_MSC0_PE
  #endif
  #ifdef CONFIG_SPL_JZMMC_SUPPORT
	#define CONFIG_SPL_JZSDHCI
  #endif
  #ifdef CONFIG_SPL_MMC_SUPPORT
	#define CONFIG_JZ_MMC_SPLMSC		//Configuration SPL stage msc controller use jz_sdhci driver
  #endif
#endif /* CONFIG_SPL_MMC_SUPPORT || CONFIG_SPL_JZMMC_SUPPORT */

/**
 * GPT configuration
 */
#ifdef CONFIG_GPT_CREATOR
#ifndef CONFIG_GPT_TABLE_PATH
#define CONFIG_GPT_TABLE_PATH	"$(TOPDIR)/board/$(BOARDDIR)"
#endif
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

/* boot args console tty
 */
#if CONFIG_SYS_UART_INDEX == 0
#define ARG_CONSOLE_TTY "console=ttyS0,"
#elif CONFIG_SYS_UART_INDEX == 1
#define ARG_CONSOLE_TTY "console=ttyS1,"
#elif CONFIG_SYS_UART_INDEX == 2
#define ARG_CONSOLE_TTY "console=ttyS2,"
#elif CONFIG_SYS_UART_INDEX == 3
#define ARG_CONSOLE_TTY "console=ttyS3,"
#elif CONFIG_SYS_UART_INDEX == 4
#define ARG_CONSOLE_TTY "console=ttyS4,"
#else
#error "please add more define here"
#endif

/* boot args uart rate
 */
#if CONFIG_BAUDRATE == 115200
#define ARG_CONSOLE_RATE "115200n8"
#elif CONFIG_BAUDRATE == 3000000
#define ARG_CONSOLE_RATE "3000000n8"
#else
#error "please add more define here"
#endif

/* boot args console
 */
#define ARGS_CONSOLE ARG_CONSOLE_TTY ARG_CONSOLE_RATE

#ifdef CONFIG_ARG_NO_CONSOLE
#undef ARGS_CONSOLE
#define ARGS_CONSOLE "no_console"
#endif

#ifdef CONFIG_ARG_QUIET
#define ARGS_QUIET "quiet"
#else
#define ARGS_QUIET ""
#endif

/* boot args mem define
 */

#define CONFIG_BOOTARGS_AUTO_MODIFY     1       /*auto detect memory size, and modify bootargs for kernel.*/

#define CONFIG_SPL_AUTO_PROBE_ARGS_MEM
#define ARGS_MEM_RESERVED "[mem-start------------------------------------------------------------mem-end]"
#ifndef CONFIG_RMEM_MB
#define CONFIG_RMEM_MB 0
#endif

#ifndef CONFIG_NMEM_MB
#define CONFIG_NMEM_MB 0
#endif

#ifndef CONFIG_RTOS_SIZE_MB
#define CONFIG_RTOS_SIZE_MB 0
#endif

#ifndef CONFIG_LCD_MEM_MB
#define CONFIG_LCD_MEM_MB 0
#endif

/* boot args init program
 */
#ifndef CONFIG_ROOTFS_INITRC
#define CONFIG_ROOTFS_INITRC "init=/linuxrc"
#endif

#ifndef CONFIG_ROOTFS2_INITRC
#define CONFIG_ROOTFS2_INITRC CONFIG_ROOTFS_INITRC
#endif

/* boot args rootfs
 */
#if defined(CONFIG_ROOTFS_EXT2)
#define ARG_ROOTFS_TYPE " ro" /* rootfstype=ext2 */
#elif defined(CONFIG_ROOTFS_UBI)
#define ARG_ROOTFS_TYPE "rootfstype=ubifs ro"
#elif defined(CONFIG_ROOTFS_SQUASHFS)
#define ARG_ROOTFS_TYPE "rootfstype=squashfs ro"
#elif defined(CONFIG_ROOTFS_RAMDISK)
#define ARG_ROOTFS_TYPE "rw"
#else
#error "please add more define here"
#endif

#if defined(CONFIG_RTOS_CONN_WITH_OS) || defined(CONFIG_SPL_MCU_RTOS_BOOT) || defined(CONFIG_SPL_RTOS_LOAD_KERNEL)
#define CONFIG_CLK_IGNORE_UNUSED " clk_ignore_unused "
#else
#define CONFIG_CLK_IGNORE_UNUSED " "
#endif

#ifdef CONFIG_LPJ
#define CONFIG_BOGOMIPS "lpj="CONFIG_LPJ
#else
#define CONFIG_BOGOMIPS ""
#endif

#ifndef CONFIG_ROOTFS_PARAM

#ifndef CONFIG_ROOTFS_DEV
#ifdef CONFIG_SPL_JZMMC_SUPPORT
#define CONFIG_ROOTFS_DEV "root=/dev/mmcblk0p2 rootwait"
#else
#define CONFIG_ROOTFS_DEV "root=/dev/mtdblock_bbt_ro2"
#endif /* CONFIG_SPL_JZMMC_SUPPORT */
#endif /* CONFIG_ROOTFS_DEV */

#ifdef CONFIG_SPL_JZMMC_SUPPORT
#define CONFIG_ROOTFS_PARAM CONFIG_CLK_IGNORE_UNUSED CONFIG_ROOTFS_DEV
#else
#define CONFIG_ROOTFS_PARAM CONFIG_FLASH_TYPE CONFIG_CLK_IGNORE_UNUSED " " CONFIG_ROOTFS_DEV
#endif

#endif

#define ARGS_ROOTFS CONFIG_ROOTFS_INITRC" "CONFIG_ROOTFS_PARAM" "ARG_ROOTFS_TYPE" "CONFIG_BOGOMIPS

/* boot args rootfs2
 */
#if defined(CONFIG_ROOTFS2_EXT2)
#define ARG_ROOTFS2_TYPE " ro" /* rootfstype=ext2 */
#elif defined(CONFIG_ROOTFS2_UBI)
#define ARG_ROOTFS2_TYPE "rootfstype=ubifs ro"
#elif defined(CONFIG_ROOTFS2_SQUASHFS)
#define ARG_ROOTFS2_TYPE "rootfstype=squashfs ro"
#elif defined(CONFIG_ROOTFS2_RAMDISK)
#define ARG_ROOTFS2_TYPE "rw"
#else
#error "please add more define here"
#endif

#ifndef CONFIG_ROOTFS2_PARAM

#ifndef CONFIG_ROOTFS2_DEV
#ifdef CONFIG_SPL_JZMMC_SUPPORT
#define CONFIG_ROOTFS2_DEV "root=/dev/mmcblk0p4 rootwait"
#else
#define CONFIG_ROOTFS2_DEV "root=/dev/mtdblock_bbt_ro4"
#endif /* CONFIG_SPL_JZMMC_SUPPORT */
#endif /* CONFIG_ROOTFS2_DEV */

#ifdef CONFIG_SPL_JZMMC_SUPPORT
#define CONFIG_ROOTFS2_PARAM CONFIG_CLK_IGNORE_UNUSED CONFIG_ROOTFS2_DEV
#else
#define CONFIG_ROOTFS2_PARAM CONFIG_FLASH_TYPE CONFIG_CLK_IGNORE_UNUSED " " CONFIG_ROOTFS2_DEV
#endif /* CONFIG_SPL_JZMMC_SUPPORT */
#endif /* CONFIG_ROOTFS2_PARAM */

#define ARGS_ROOTFS2 CONFIG_ROOTFS2_INITRC " " CONFIG_ROOTFS2_PARAM " " ARG_ROOTFS2_TYPE " " CONFIG_BOGOMIPS

#ifndef CONFIG_ARGS_EXTRA
#define CONFIG_ARGS_EXTRA ""
#endif

#define BOOTARGS_COMMON ARGS_CONSOLE " " ARGS_QUIET " " ARGS_MEM_RESERVED " " CONFIG_ARGS_EXTRA

#ifdef CONFIG_SPL_RTOS_LOAD_KERNEL
#ifndef CONFIG_SPL_RTOS_BOOT
#define CONFIG_SPL_RTOS_BOOT 1
#endif
#ifndef CONFIG_SPL_OS_BOOT
#define CONFIG_SPL_OS_BOOT 1
#endif
#ifndef CONFIG_RTOS_CAN_RETURN
#define CONFIG_RTOS_CAN_RETURN 1
#endif
#endif

#ifdef CONFIG_SPL_RTOS_BOOT

#if defined(CONFIG_SPL_MMC_SUPPORT) || defined(CONFIG_SPL_JZMMC_SUPPORT)
#define CONFIG_RTOS_OFFSET (17 * 1024 + CONFIG_SPL_PAD_TO)
#define CONFIG_RTOS_OFFSET_SECTOR (CONFIG_RTOS_OFFSET / 512)
#else
#define CONFIG_RTOS_OFFSET CONFIG_SPL_PAD_TO
#endif

#endif

#define CONFIG_CMD_EJTAG
#ifdef CONFIG_CMD_EJTAG
#define CONFIG_XBURST_TRAPS
#define CONFIG_INT_DEBUG
#endif

#define CONFIG_SFC_RATE			48000000

#ifdef CONFIG_SPL_OS_OTA_BOOT
#define CONFIG_SPL_OS_BOOT
#endif

#ifdef CONFIG_SPL_OS_OTA_BOOT
#ifndef CONFIG_SPL_RTOS_NAME2
    #define CONFIG_SPL_RTOS_NAME2    "rtos"
#endif
    #define CONFIG_SPL_OTA_NAME       "ota"
    #define CONFIG_SPL_OS_NAME2       "kernel2"
#ifdef CONFIG_JZ_SECURE_ROOTFS
    #define CONFIG_SPL_ROOTFS_NAME2   "rootfs2"
#endif /* end of CONFIG_JZ_SECURE_ROOTFS */
    #define CONFIG_SPL_BOOTARGS2      BOOTARGS_COMMON " " ARGS_ROOTFS2
#ifndef CONFIG_SPL_OF_LIBFDT
    #define CONFIG_SYS_SPL_ARGS_ADDR2    CONFIG_SPL_BOOTARGS2
#else
    #define CONFIG_SYS_SPL_ARGS_ADDR2 " "
    #define CONFIG_DTB_NAME2          "dtb2"
#endif
#endif

#ifdef CONFIG_SPL_OS_BOOT
    #define CONFIG_SPL_RTOS_NAME    "rtos"
    #define CONFIG_SPL_BOOTARGS	 BOOTARGS_COMMON " " ARGS_ROOTFS
    #define CONFIG_SPL_OS_NAME        "kernel" /* spi offset of xImage being loaded */
    #define CONFIG_SPL_RTOS_LINUX_MAPPED_FILESYSTEM_NAME    "rtosdata"
#ifndef CONFIG_SPL_OF_LIBFDT
    #define CONFIG_SYS_SPL_ARGS_ADDR    CONFIG_SPL_BOOTARGS
#else
    #define CONFIG_SYS_SPL_ARGS_ADDR " "
#endif
#ifdef CONFIG_JZ_SECURE_ROOTFS
    #define CONFIG_SPL_SIG_NAME         "signature"
    #define CONFIG_SPL_ROOTFS_NAME      "rootfs"
#endif
    #define CONFIG_BOOTX_BOOTARGS ""
    #undef  CONFIG_BOOTCOMMAND
    #define CONFIG_BOOTCOMMAND    ""
    #define CONFIG_LOAD_ADDR	0x80001000
#endif	/* CONFIG_SPL_OS_BOOT */

#ifdef CONFIG_BOOT_RTOS_OTA
    #define CONFIG_SPL_RTOS_OTA_NAME  "rtos_ota"
#ifndef CONFIG_SPL_OTA_NAME
    #define CONFIG_SPL_OTA_NAME       "ota"
#endif

#ifndef CONFIG_SPL_RTOS_OTA_INFO
    #define CONFIG_SPL_RTOS_OTA_INFO       "ota:rtos_ota"
#endif

#ifdef CONFIG_SPL_RTOS_BOOT
#ifndef CONFIG_SPL_RTOS_NAME
    #define CONFIG_SPL_RTOS_NAME    "rtos"
#endif
#endif /* CONFIG_SPL_RTOS_BOOT */

#endif /* CONFIG_BOOT_RTOS_OTA */

#define CONFIG_BOOTARGS ""

#define PARTITION_NUM 10

#endif/*END OF __X2600_BASE_COMMON_H__ */
