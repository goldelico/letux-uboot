#ifndef __BURNER_COMMON_H__
#define __BURNER_COMMON_H__


#define CONFIG_SYS_EXTAL		24000000	/* EXTAL freq: 24 MHz */
#define CONFIG_SYS_HZ			1000		/* incrementer freq */

#define CONFIG_BAUDRATE                 115200

/**
 * DDR
 */
#define CONFIG_DDR_TYPE_VARIABLE


/**
 * Boot command definitions.
 */
#define CONFIG_BOOTDELAY                0


/**
 * Drivers configuration.
 */
#define CONFIG_NOR_MAJOR_VERSION_NUMBER     1
#define CONFIG_NOR_MINOR_VERSION_NUMBER     0
#define CONFIG_NOR_REVERSION_NUMBER         0
#define CONFIG_NOR_VERSION                  (CONFIG_NOR_MAJOR_VERSION_NUMBER |       \
                                            (CONFIG_NOR_MINOR_VERSION_NUMBER << 8) | \
                                            (CONFIG_NOR_REVERSION_NUMBER << 16))


/* GPIO */
#define CONFIG_JZ_GPIO
#define CONFIG_INGENIC_SOFT_I2C



/**
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_BASE	        0 /* init flash_base as 0 */

#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_MAXARGS              16
#define CONFIG_SYS_PROMPT               "burner# "

#define CONFIG_SYS_MONITOR_LEN		(1024 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000 /* cached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0x90000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0x88000000
#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_MONITOR_BASE         CONFIG_SYS_TEXT_BASE


/**
 * SPL configuration
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK

#define CONFIG_SPL_START_S_PATH         "$(CPUDIR)/$(SOC)"
#define CONFIG_SPL_LDSCRIPT		"$(TOPDIR)/board/$(BOARDDIR)/u-boot-spl.lds"
#define CONFIG_SPL_PAD_TO		24576 /* equal to spl max size */

#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT


/* RTC */
#define CONFIG_CMD_DATE
#define CONFIG_RTC_JZ47XX

/**
 * Environment
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE                 32768


/**
 * Command configuration.
 */
#define CONFIG_CMD_CONSOLE

#define CONFIG_SYS_CBSIZE               1024 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE               (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)


/**
 * Burner
 */
#define CONFIG_BURNER
#define CONFIG_CMD_BURN
#define CONFIG_SOFT_BURNER
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_JZ_BURNER_GADGET
#define CONFIG_USB_JZ_DWC2_UDC_V1_1
#define CONFIG_USB_SELF_POLLING
#define CONFIG_USB_GADGET_VBUS_DRAW         500
#define CONFIG_BURNER_PRIDUCT_INFO          "Ingenic USB BOOT DEVICE"
#define CONFIG_BOOTCOMMAND                  "burn"
#define CONFIG_JZ_VERDOR_BURN_FUNCTION
/*
#define CONFIG_JZ_VERDOR_BURN_EXTPOL
#define CONFIG_JZ_VERDOR_BURN_EP_TEST
*/

#endif /* __BURNER_COMMON_H__ */

