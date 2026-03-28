/*
 * Ingenic mensa configuration
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

#ifndef __CONFIG_MYPAI_H__
#define __CONFIG_MYPAI_H__

/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32R2		/* MIPS32 CPU core */
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_X1000
#define CONFIG_CHECK_SOCID

#if 0
#define CONFIG_OLED_SSD1306_128X32  /* move to board.cfg for oled*/
#endif

#ifdef CONFIG_OLED_SSD1306_128X32
#if 0
#define DEBUG_USE_ISP_I2C
#endif

#ifdef DEBUG_USE_ISP_I2C
#define GPIO_OLED_I2C_SCK GPIO_PD(0) /*GPIO_PB(23)*/
#define GPIO_OLED_I2C_SDA GPIO_PD(1) /*GPIO_PB(24)*/
#define ISP_I2C_VCC         GPIO_PD(3)
#define ISP_PWDN_PIN        GPIO_PA(6)
#define ISP_RESET_PIN       GPIO_PA(7)
#else
#define GPIO_OLED_I2C_SCK GPIO_PB(23)
#define GPIO_OLED_I2C_SDA GPIO_PB(24)
#endif

#define GPIO_OLED_RES     GPIO_PD(2)
#define GPIO_OLED_D0      GPIO_OLED_I2C_SCK
#define GPIO_OLED_D1      GPIO_OLED_I2C_SDA
#define GPIO_OLED_VBAT    GPIO_PC(25)
#define GPIO_OLED_VDDIO   -1 //GPIO_PD(3)
#endif

#define CONFIG_SYS_APLL_FREQ		1008000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_FREQ		600000000	/*If MPLL not use mast be set 0*/
#define CONFIG_CPU_SEL_PLL		APLL
#define CONFIG_DDR_SEL_PLL		MPLL
#define CONFIG_SYS_CPU_FREQ		1008000000
#define CONFIG_SYS_MEM_FREQ		200000000

#define CONFIG_SYS_EXTAL		24000000	/* EXTAL freq: 24 MHz */
#define CONFIG_SYS_HZ			1000 /* incrementer freq */


#define CONFIG_SYS_DCACHE_SIZE		16384
#define CONFIG_SYS_DCACHELINE_SIZE	(32)
#define CONFIG_SYS_DCACHE_WAYS		(4)
#define CONFIG_SYS_ICACHE_SIZE		16384
#define CONFIG_SYS_ICACHELINE_SIZE	(32)
#define CONFIG_SYS_ICACHE_WAYS		(4)
#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_SYS_UART_INDEX		2
#define CONFIG_SYS_UART2_PC
#define CONFIG_BAUDRATE			115200

/*#define CONFIG_DDR_TEST*/
#define CONFIG_DDR_PARAMS_CREATOR
#define CONFIG_DDR_HOST_CC
#define CONFIG_DDR_TYPE_LPDDR
#define CONFIG_DDR_CS0			1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1			0	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32			0	/* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDR_tREFI	    DDR__ns(7800)
/*
  Output Drive Strength: Controls the output drive strength. Valid values are:
  000 = Full strength driver
  001 = Half strength driver
  110 = Quarter strength driver
  111 = Octant strength driver
  100 = Three-quarters strength driver
*/
#define CONFIG_DDR_DRIVER_STRENGTH             4

/* CONFIG_CMD_FASTBOOT */

#define  CONFIG_DDR_64M     64
#define  CONFIG_DDR_32M     32
#define CONFIG_MDDR_EMD56164PC_50I

/* CONFIG_CMD_FASTBOOT */
#ifdef CONFIG_ARDUINO
#define CONFIG_CMD_FASTBOOT
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_JZ_DWC2_UDC_V1_1
#define CONFIG_FASTBOOT_GADGET
#define CONFIG_FASTBOOT_FUNCTION
#define CONFIG_G_FASTBOOT_VENDOR_NUM	(0x18d1)
#define CONFIG_G_FASTBOOT_PRODUCT_NUM	(0xdddd)
#define CONFIG_USB_GADGET_VBUS_DRAW 500
#endif

/*pmu slp pin*/
#define CONFIG_REGULATOR
#ifdef  CONFIG_REGULATOR
#define CONFIG_PMU_RICOH6x
/*#define CONFIG_PMU_AXP216*/

/* I2C */
#define CONFIG_INGENIC_SOFT_I2C

#define CONFIG_RICOH61X_I2C_SCL	GPIO_PC(26)
#define CONFIG_RICOH61X_I2C_SDA	GPIO_PC(27)

#define CONFIG_SOFT_I2C_READ_REPEATED_START
#define CONFIG_PMU_RICOH6x
#define CONFIG_JZ_PMU_SLP_OUTPUT1

#define CONFIG_500MAH_CAPACITY_BATTERY
#define CONFIG_BATTERY_VOLTAGE_MAX       4200
#define CONFIG_BATTERY_VOLTAGE_MIN       3400
#define CONFIG_BATTERY_VOLTAGE_SCALE     150
#endif

/* #define CONFIG_DDR_DLL_OFF */
/*
 * #define CONFIG_DDR_CHIP_ODT
 * #define CONFIG_DDR_PHY_ODT
 * #define CONFIG_DDR_PHY_DQ_ODT
 * #define CONFIG_DDR_PHY_DQS_ODT
 * #define CONFIG_DDR_PHY_IMPED_PULLUP		0xe
 * #define CONFIG_DDR_PHY_IMPED_PULLDOWN	0xe
 */

#if defined(CONFIG_SPL_SFC_NOR) || defined(CONFIG_SPL_SFC_NAND)
#define CONFIG_SPL_SFC_SUPPORT
#endif

/**
 * Boot arguments definitions.
 */

#if 1
#define CONFIG_X1000E
#endif

#define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "# "

#ifdef CONFIG_X1000E
#define BOOTARGS_COMMON "console=ttyS2,115200n8 mem=63M@0x0 "
#else
#define BOOTARGS_COMMON "console=ttyS2,115200n8 mem=32M@0x0 "
#endif

#if defined(CONFIG_SPL_JZMMC_SUPPORT) && defined(CONFIG_JZ_MMC_MSC0)
    #ifdef CONFIG_BOOT_ANDROID
        #define CONFIG_BOOTARGS BOOTARGS_COMMON " ip=off root=/dev/ram0 rw rdinit=/init"
        #define CONFIG_BOOTCOMMAND "cls; boota mmc 0 0x80f00000 6144"
        #define CONFIG_NORMAL_BOOT CONFIG_BOOTCOMMAND
        #define CONFIG_RECOVERY_BOOT "boota nand 0 0x80f00000 24576"
    #else
        #define CONFIG_BOOTARGS BOOTARGS_COMMON "root=/dev/mmcblk0p2 init=/linuxrc rootwait rw"
      	#define CONFIG_UPDATEARGS BOOTARGS_COMMON "root=/dev/mmcblk0p3 init=/linuxrc rootwait rw"
        #define CONFIG_BOOTCOMMAND "mmc dev 0;mmc read 0x80600000 0x1800 0x3000; bootm 0x80600000"
    #endif
#elif defined(CONFIG_SPL_JZMMC_SUPPORT)
    #define CONFIG_BOOTARGS BOOTARGS_COMMON " root=/dev/mmcblk1p2 rootfstype=ext4 rootdelay=1 rw"
    #define CONFIG_BOOTCOMMAND "mmc dev 1;mmc read 0x80600000 0x1800 0x3000; bootm 0x80600000"
#endif

#define PARTITION_NUM 10

/**
 * Boot command definitions.
 */
#define CONFIG_BOOTDELAY 0 /*1 为了加速，取消delay*/

/* CLK CGU */
#define  CGU_CLK_SRC {				\
		{OTG, EXCLK},			\
		{LCD, MPLL},			\
		{MSC, MPLL},			\
		{SFC, MPLL},			\
		{CIM, MPLL},			\
		{PCM, MPLL},			\
		{I2S, EXCLK},			\
		{SRC_EOF,SRC_EOF}		\
	}
/* GPIO */
#define CONFIG_JZ_GPIO

/**
 * Command configuration.
 */
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_ECHO		/* echo arguments		*/
#define CONFIG_CMD_EXT4 	/* ext4 support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_SAVEENV	/* saveenv			*/
#define CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/
#define CONFIG_CMD_SOURCE	/* "source" command support	*/
#define CONFIG_CMD_GETTIME
#define CONFIG_CMD_UNZIP        /* unzip from memory to memory  */
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NET     /* networking support*/
#define CONFIG_CMD_PING

/*
#define CONFIG_CMD_EFUSE
#define	CONFIG_JZ_EFUSE
*/

#ifndef CONFIG_SPL_BUILD
#define CONFIG_USE_ARCH_MEMSET
#define CONFIG_USE_ARCH_MEMCPY
#endif

/* LCD */
#define CONFIG_GPIO_PWR_WAKE		GPIO_PB(31)
#define CONFIG_GPIO_PWR_WAKE_ENLEVEL	0

/* #define CONFIG_LCD */

#ifdef CONFIG_LCD
/*#define CONFIG_LCD_FORMAT_X8B8G8R8*/
#define LCD_BPP             4
#define CONFIG_GPIO_LCD_PWM     GPIO_PC(25)

#define CONFIG_LCD_LOGO
/* #define CONFIG_RLE_LCD_LOGO */
/*#define CONFIG_LCD_INFO_BELOW_LOGO      //display the console info on lcd panel for debugg*/
#define CONFIG_SYS_WHITE_ON_BLACK
#define CONFIG_SYS_PWM_PERIOD       10000 /* Pwm period in ns */
#define CONFIG_SYS_PWM_CHN      0  /* Pwm channel ok*/
#define CONFIG_SYS_PWM_FULL     256
#define CONFIG_SYS_BACKLIGHT_LEVEL  80 /* Backlight brightness is (80 / 256) */
#define CONFIG_JZ_LCD_V12
#define SOC_X1000
#define CONFIG_JZ_PWM
#define CONFIG_SYS_CONSOLE_INFO_QUIET
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
/*#define CONFIG_SLCDC_CONTINUA*/
/*#define CONFIG_LCD_GPIO_FUNC0_24BIT*/
#define CONFIG_LCD_GPIO_FUNC1_SLCD
/*#define CONFIG_VIDEO_BM347WV_F_8991FTGF*/
#define CONFIG_VIDEO_TRULY_TFT240240_2_E

#ifdef CONFIG_VIDEO_TRULY_TFT240240_2_E
#define CONFIG_GPIO_LCD_RD GPIO_PB(16)
#define CONFIG_GPIO_LCD_RST GPIO_PD(0)
#define CONFIG_GPIO_LCD_CS GPIO_PB(18)
#define CONFIG_GPIO_LCD_BL GPIO_PD(1)
#endif	/* CONFIG_VIDEO_TRULY_TFT240240_2_E */

#ifdef CONFIG_RLE_LCD_LOGO
#define CONFIG_CMD_BATTERYDET       /* detect battery and show charge logo */
#define CONFIG_CMD_LOGO_RLE /*display the logo using rle command*/
#endif

#endif /* CONFIG_LCD */

/* MMC */
#define CONFIG_CMD_MMC
/*#define CONFIG_MMC_TRACE*/


#ifdef CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_JZ_MMC			1

#ifdef CONFIG_JZ_MMC_MSC0
#define CONFIG_JZ_MMC_SPLMSC 0
#define CONFIG_JZ_MMC_MSC0_PA_4BIT 1
/* #define CONFIG_JZ_MMC_MSC0_PA_8BIT 1 */
/* #define CONFIG_MSC_DATA_WIDTH_8BIT */
#define CONFIG_MSC_DATA_WIDTH_4BIT
/* #define CONFIG_MSC_DATA_WIDTH_1BIT */
#endif

#ifdef CONFIG_JZ_MMC_MSC1
/*#define CONFIG_JZ_MMC_SPLMSC 1*/
#define CONFIG_JZ_MMC_MSC1_PC 1
/* #define CONFIG_MSC_DATA_WIDTH_8BIT */
#define CONFIG_MSC_DATA_WIDTH_4BIT
/* #define CONFIG_MSC_DATA_WIDTH_1BIT */
#endif
#endif

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

#if defined(CONFIG_SUPPORT_EMMC_BOOT)
#define CONFIG_SYS_MONITOR_LEN		(384 * 1024)
#else
#define CONFIG_SYS_MONITOR_LEN		(512 << 10)
#endif

#define CONFIG_SYS_MALLOC_LEN		(8 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000 /* cached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0x90000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0x88000000
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x88000000
#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

/**
 * Environment
 */
#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)
#elif defined(CONFIG_ENV_IS_IN_SFC)
#define CONFIG_ENV_SIZE			(4 << 10)
#define CONFIG_ENV_OFFSET		0x3f000 /*write nor flash 252k address*/
#define CONFIG_CMD_SAVEENV
#endif

/**
 * SPL configuration
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH		"$(CPUDIR)/$(SOC)"
#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_LDSCRIPT             "$(CPUDIR)/$(SOC)/u-boot-nor-spl.lds"
#else
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-spl.lds"
#endif
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x3A /* 12KB+17K offset */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#if defined(CONFIG_SPL_NOR_SUPPORT)
#define CONFIG_SPL_TEXT_BASE		0xba000000
#define CONFIG_SYS_UBOOT_BASE		(CONFIG_SPL_TEXT_BASE + CONFIG_SPL_PAD_TO - 0x40)
					/* 0x40 = sizeof (image_header)*/
#define CONFIG_SYS_OS_BASE		0
#define CONFIG_SYS_SPL_ARGS_ADDR	0
#define CONFIG_SYS_FDT_BASE		0
#define CONFIG_SPL_PAD_TO		32768
#define CONFIG_SPL_MAX_SIZE		(32 * 1024)
#elif defined(CONFIG_SPL_JZMMC_SUPPORT)
#define CONFIG_SPL_PAD_TO		12288			/* spl size */
#define CONFIG_SPL_TEXT_BASE		0xf4001000
#define CONFIG_SPL_MAX_SIZE		(12 * 1024)
#elif defined(CONFIG_SPL_SFC_SUPPORT)
#define CONFIG_UBOOT_OFFSET             (4<<12)
#define CONFIG_JZ_SFC_PA_6BIT
#ifdef	CONFIG_SPL_SFC_NAND
#define CONFIG_SFC_RATE    150000000
#define CONFIG_SPIFLASH_PART_OFFSET     0x3c00
#define CONFIG_SPI_NAND_BPP			(2048 +64)		/*Bytes Per Page*/
#define CONFIG_SPI_NAND_PPB			(64)		/*Page Per Block*/
#define CONFIG_SPL_TEXT_BASE		0xf4001000
#define CONFIG_SPL_MAX_SIZE		(12 * 1024)
#define CONFIG_SPL_PAD_TO		16384
#define CONFIG_SPL_SFC_NAND
#define CONFIG_MTD_SFCNAND
#define CONFIG_JZ_SFC
#define CONFIG_CMD_SFCNAND
#define CONFIG_CMD_NAND
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE    0xb3441000

/*SFCNAND env*/
/* spi nand environment */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE 0x20000 /* 128K*/
#define SPI_NAND_BLK            0x20000 /*the spi nand block size */
#define CONFIG_ENV_SIZE         SPI_NAND_BLK /* uboot is 1M but the last block size is the env*/
#define CONFIG_ENV_OFFSET       0xc0000 /* offset is 768k */
#define CONFIG_ENV_OFFSET_REDUND (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_IS_IN_SFC_NAND


#else
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_SPL_TEXT_BASE		0xf4001000
#define CONFIG_SPL_MAX_SIZE		(12 * 1024)
#define CONFIG_SPL_PAD_TO		16384
#define CONFIG_CMD_SFC_NOR
#endif
#endif

#ifdef CONFIG_SPL_SPI_NAND
#define CONFIG_SPIFLASH_PART_OFFSET     0x3c00
#define CONFIG_UBOOT_OFFSET             (4<<12)
#define CONFIG_SPL_TEXT_BASE		0xf4001000
#define CONFIG_SPL_MAX_SIZE		(12 * 1024)
#define CONFIG_SPL_PAD_TO		16384
#define CONFIG_SPI_NAND_BPP			(2048 +128)		/*Bytes Per Page*/
#define CONFIG_SPI_NAND_PPB			(64)		/*Page Per Block*/
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_JZ_SPI
#define CONFIG_JZ_SSI0_PA
#define CONFIG_MTD_SPINAND
#define CONFIG_CMD_SPINAND
#define CONFIG_SPI_FLASH
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_ENV_IS_IN_SPI_NAND

/* spi nand environment */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE 0x20000 /* 128K*/
#define SPI_NAND_BLK		0x20000 /*the spi nand block size */
#define CONFIG_ENV_SIZE		SPI_NAND_BLK /* uboot is 1M but the last block size is the env*/
#define CONFIG_ENV_OFFSET	0xc0000 /* offset is 768k */
#define CONFIG_ENV_OFFSET_REDUND (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)


#define CONFIG_CMD_NAND /*use the mtd and the function do_nand() */
#define CONFIG_SYS_MAX_NAND_DEVICE  1

#endif /*CONFIG_SPL_SPI_NAND*/

#ifdef CONFIG_CMD_SPI
#define CONFIG_SSI_BASE SSI0_BASE
#define CONFIG_SPI_BUILD
#ifdef CONFIG_SOFT_SPI
#undef SPI_INIT
#define SPI_DELAY
#define	SPI_SDA(val)    gpio_direction_output(GPIO_PA(21), val)
#define	SPI_SCL(val)	gpio_direction_output(GPIO_PA(18), val)
#define	SPI_READ	gpio_get_value(GPIO_PA(20))
#else
#define CONFIG_JZ_SPI
#endif
/*#define CONFIG_JZ_SPI_FLASH*/
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_INGENIC
#define CONFIG_SPI_FLASH
#define CONFIG_UBOOT_OFFSET             (4<<12)
#endif

#ifdef CONFIG_CMD_SFC_NOR
#define CONFIG_JZ_SFC
#define CONFIG_JZ_SFC_NOR
#define CONFIG_SFC_QUAD
#define CONFIG_SFC_RATE    150000000
#endif

#ifdef CONFIG_JZ_SFC_NOR
#define CONFIG_SPIFLASH_PART_OFFSET     0x3c00
#define CONFIG_SPI_NORFLASH_PART_OFFSET     0x3c74
#define CONFIG_NOR_MAJOR_VERSION_NUMBER     1
#define CONFIG_NOR_MINOR_VERSION_NUMBER     0
#define CONFIG_NOR_REVERSION_NUMBER     0
#define CONFIG_NOR_VERSION     (CONFIG_NOR_MAJOR_VERSION_NUMBER | (CONFIG_NOR_MINOR_VERSION_NUMBER << 8) | (CONFIG_NOR_REVERSION_NUMBER <<16))
#endif
/**
 * MBR configuration
 */
#ifdef CONFIG_MBR_CREATOR
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
#else
#define CONFIG_GPT_TABLE_PATH	"$(TOPDIR)/board/$(BOARDDIR)"
#endif

/* Wrong keys. */
/*clivia del #define CONFIG_KEYSECLECT_RECOVERY*/
#define CONFIG_GPIO_RECOVERY_KEY          GPIO_PB(30)     /* SW4 */
#define CONFIG_GPIO_BOOT_SELECT_ENLEVEL   0

/* Wrong keys. */
#define CONFIG_GPIO_FASTBOOT            GPIO_PB(31)     /* SW4 */
#define CONFIG_GPIO_FASTBOOT_ENLEVEL    0

/*
* MTD support
*/
#define CONFIG_SYS_NAND_SELF_INIT

#endif /* __CONFIG_PHOENIX_H__ */
