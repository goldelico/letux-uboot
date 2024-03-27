/*
 * Ingenic X1600 configuration
 */

#ifndef __X1600_BASE_COMMON_H__
#define __X1600_BASE_COMMON_H__

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

/*
 * uart setting
 */
#ifndef CONFIG_SYS_UART_INDEX
#define CONFIG_SYS_UART_INDEX		2
#endif

#ifndef CONFIG_BAUDRATE
#define CONFIG_BAUDRATE			3000000
#endif

#define CONFIG_SYS_DCACHE_SIZE		(16 * 1024)
#define CONFIG_SYS_DCACHELINE_SIZE	(32)
#define CONFIG_SYS_DCACHE_WAYS		(8)
#define CONFIG_SYS_ICACHE_SIZE		(16 * 1024)
#define CONFIG_SYS_ICACHELINE_SIZE	(32)
#define CONFIG_SYS_ICACHE_WAYS		(8)
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

#define CONFIG_DDR_TYPE_LPDDR2
/*#define CONFIG_DDR_TYPE_LPDDR3*/

#ifdef CONFIG_DDR_TYPE_LPDDR2
#ifdef CONFIG_X1660_DDR
#define CONFIG_LPDDR2_SCB4BL256160AFL19GI  /* 1660 ddr */
#else
#define CONFIG_LPDDR2_M54D5121632A
/*#define CONFIG_MCP_H9TP32A8JDMC_PRKGM_LPDDR2*/
/*#define CONFIG_LPDDR2_FMT4D32UAB_25LI_FPGA*/
#endif
#endif

#ifdef CONFIG_DDR_TYPE_LPDDR
#define CONFIG_MDDR_H5MS5122DFR_J3M
#endif

#ifdef CONFIG_DDR_TYPE_DDR3
#define CONFIG_DDR3_TSD34096M1333C9_E
#endif

#ifdef CONFIG_DDR_TYPE_LPDDR3
	#define CONFIG_LPDDR3_MT52L256M32D1PF_FPGA
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


#define CONFIG_SPL_MAX_SIZE		26624	/* 18KB */
#define CONFIG_SPL_PAD_TO		26624  /* equal to spl max size in x1600 */
#define CONFIG_UBOOT_OFFSET             26624 /* equal to spl max size in x1600 */

/********** RTOS **********/
#ifdef CONFIG_SPL_RTOS_BOOT

#define CONFIG_RTOS_OFFSET CONFIG_SPL_PAD_TO
#define CONFIG_SPL_RTOS_NAME    "rtos"

#endif
/********** RTOS **********/

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
#define CONFIG_FLASH_TYPE "flashtype=nor"
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
#define CONFIG_FLASH_TYPE "flashtype=nand"
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
#define CONFIG_CMD_LOADB	/* loadb			*/
#define CONFIG_CMD_LOADS	/* loads			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#define CONFIG_CMD_MMC		/* MMC/SD support			*/
#define CONFIG_CMD_NET		/* networking support			*/
#define CONFIG_CMD_PING
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/
#define CONFIG_CMD_SOURCE	/* "source" command support	*/
#define CONFIG_CMD_GETTIME
#define CONFIG_CMD_GPIO
#define CONFIG_CMD_SAVEENV	/* saveenv			*/
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
#define CONFIG_SYS_SDRAM_MAX_TOP	0x90000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0x88000000
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x88000000

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



/*
 * boot args console tty
 */
#if CONFIG_SYS_UART_INDEX == 0
#define ARG_CONSOLE_TTY "console=ttyS0,"
#elif CONFIG_SYS_UART_INDEX == 1
#define ARG_CONSOLE_TTY "console=ttyS1,"
#elif CONFIG_SYS_UART_INDEX == 2
#define ARG_CONSOLE_TTY "console=ttyS2,"
#elif CONFIG_SYS_UART_INDEX == 3
#define ARG_CONSOLE_TTY "console=ttyS3,"
#else
#error "please add more define here"
#endif

/*
 * boot args uart rate
 */
#if CONFIG_BAUDRATE == 115200
#define ARG_CONSOLE_RATE "115200n8"
#elif CONFIG_BAUDRATE == 3000000
#define ARG_CONSOLE_RATE "3000000n8"
#else
#error "please add more define here"
#endif

/*
 * boot args console
 */
#define ARGS_CONSOLE ARG_CONSOLE_TTY ARG_CONSOLE_RATE

#ifndef CONFIG_SPL_SERIAL_SUPPORT
#define ARGS_CONSOLE "no_console"
#endif

#ifdef CONFIG_ARG_NO_CONSOLE
#undef ARGS_CONSOLE
#define ARGS_CONSOLE "no_console"
#endif

#ifdef CONFIG_ARG_QUIET
#define ARGS_QUIET "quiet"
#else
#define ARGS_QUIET ""
#endif


/*
 * boot args mem define
 */
/* #define CONFIG_SPL_AUTO_PROBE_ARGS_MEM */
#define CONFIG_SPL_AUTO_PROBE_ARGS_MEM
#ifdef CONFIG_SPL_AUTO_PROBE_ARGS_MEM
#define ARGS_MEM_RESERVED "[mem-start------------------------------------------------------------mem-end]"
#else
#ifdef CONFIG_X1660_DDR
#define ARGS_MEM_RESERVED "mem=32M@0x0 "
#else
#define ARGS_MEM_RESERVED "mem=64M@0x0 "
#endif
#endif

#ifndef CONFIG_RMEM_MB
#define CONFIG_RMEM_MB 0
#endif

#ifndef CONFIG_RTOS_SIZE_MB
#define CONFIG_RTOS_SIZE_MB 0
#endif

/*
 * boot args init program
 */
#ifndef CONFIG_ROOTFS_INITRC
#define CONFIG_ROOTFS_INITRC "init=/linuxrc"
#endif

#ifndef CONFIG_ROOTFS2_INITRC
#define CONFIG_ROOTFS2_INITRC CONFIG_ROOTFS_INITRC
#endif

/*
 * boot args rootfs
 */
#if defined(CONFIG_ROOTFS_UBI)
#define ARG_ROOTFS_TYPE "rootfstype=ubifs ro"
#elif defined(CONFIG_ROOTFS_SQUASHFS)
#define ARG_ROOTFS_TYPE "rootfstype=squashfs ro"
#elif defined(CONFIG_ROOTFS_RAMDISK)
#define ARG_ROOTFS_TYPE "rw"
#else
#error "please add more define here"
#endif

#ifndef CONFIG_ROOTFS_DEV
#ifdef CONFIG_SPL_JZMMC_SUPPORT
#define CONFIG_ROOTFS_DEV "root=/dev/mmcblk0p2 rootwait"
#else
#define CONFIG_ROOTFS_DEV CONFIG_FLASH_TYPE " " "root=/dev/mtdblock_bbt_ro2"
#endif
#endif

#define ARGS_ROOTFS CONFIG_ROOTFS_INITRC" "CONFIG_ROOTFS_DEV" "ARG_ROOTFS_TYPE

/* boot args rootfs2
 */
#ifndef CONFIG_ROOTFS2_DEV
#ifdef CONFIG_SPL_JZMMC_SUPPORT
#define CONFIG_ROOTFS2_DEV "root=/dev/mmcblk0p4 rootwait"
#else
#define CONFIG_ROOTFS2_DEV CONFIG_FLASH_TYPE " " "root=/dev/mtdblock_bbt_ro4"
#endif
#endif

#ifdef CONFIG_ROOTFS2_DEV
#if defined(CONFIG_ROOTFS2_UBI)
#define ARG_ROOTFS2_TYPE "rootfstype=ubifs ro"
#elif defined(CONFIG_ROOTFS2_SQUASHFS)
#define ARG_ROOTFS2_TYPE "rootfstype=squashfs ro"
#elif defined(CONFIG_ROOTFS2_RAMDISK)
#define ARG_ROOTFS2_TYPE "rw"
#else
#error "please add more define here"
#endif

#define ARGS_ROOTFS2 CONFIG_ROOTFS2_INITRC " " CONFIG_ROOTFS2_DEV " " ARG_ROOTFS2_TYPE
#endif

#ifndef CONFIG_ARGS_EXTRA
#define CONFIG_ARGS_EXTRA ""
#endif

#define BOOTARGS_COMMON ARGS_CONSOLE " " ARGS_QUIET " " ARGS_MEM_RESERVED " " CONFIG_ARGS_EXTRA

#ifdef CONFIG_SPL_RTOS_BOOT

#if defined(CONFIG_SPL_MMC_SUPPORT) || defined(CONFIG_SPL_JZMMC_SUPPORT)
#define CONFIG_RTOS_OFFSET              (17 * 1024 + CONFIG_SPL_PAD_TO)
#define CONFIG_RTOS_OFFSET_SECTOR       (CONFIG_RTOS_OFFSET / 512)
#else
#define CONFIG_RTOS_OFFSET              CONFIG_SPL_PAD_TO
#define CONFIG_SPL_RTOS_NAME            "rtos"
#endif

#endif

#ifdef CONFIG_SPL_OS_OTA_BOOT
#define CONFIG_SPL_OS_BOOT
#endif

#ifdef CONFIG_SPL_OS_OTA_BOOT
#ifndef CONFIG_SPL_RTOS_NAME2
    #define CONFIG_SPL_RTOS_NAME2       "rtos"
#endif
    #define CONFIG_SPL_OTA_NAME         "ota"
    #define CONFIG_SPL_OS_NAME2         "kernel2"
    #define CONFIG_SPL_BOOTARGS2        BOOTARGS_COMMON " " ARGS_ROOTFS2
    #define CONFIG_SYS_SPL_ARGS_ADDR2   CONFIG_SPL_BOOTARGS2
#endif

#ifdef CONFIG_SPL_OS_BOOT
    #define CONFIG_SPL_RTOS_NAME        "rtos"
    #define CONFIG_SPL_BOOTARGS         BOOTARGS_COMMON " " ARGS_ROOTFS
    #define CONFIG_SPL_OS_NAME          "kernel" /* spi offset of xImage being loaded */
    #define CONFIG_SYS_SPL_ARGS_ADDR    CONFIG_SPL_BOOTARGS
    #define CONFIG_BOOTX_BOOTARGS       ""
    #undef  CONFIG_BOOTCOMMAND
    #define CONFIG_BOOTCOMMAND          ""
    #define CONFIG_LOAD_ADDR            0x80001000
#endif  /* CONFIG_SPL_OS_BOOT */

#define CONFIG_BOOTARGS                 ""

#define PARTITION_NUM                   10


#endif /* __X1600_BASE_COMMON_H__ */
