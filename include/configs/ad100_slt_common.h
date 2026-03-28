#ifndef __AD100_SLT_COMMON_H__
#define __AD100_SLT_COMMON_H__

#define CONFIG_SYS_APLL_FREQ        1248000000    /*If APLL not use mast be set 0*/
#define CONFIG_SYS_CPU_FREQ         1248000000
#define CONFIG_SYS_EPLL_FREQ        300000000    /*If MPLL not use mast be set 0*/
#define CONFIG_CPU_SEL_PLL          APLL
#define CONFIG_DDR_SEL_PLL          MPLL

#if defined(CONFIG_AD101P_DDR)
  #define CONFIG_SYS_MPLL_FREQ        1900000000    /*If MPLL not use mast be set 0*/
  #define CONFIG_SYS_MEM_FREQ         950000000
  #define CONFIG_SYS_AHB0_FREQ        380000000
  #define CONFIG_SYS_AHB2_FREQ        317000000    /*APB = AHB2/2*/
#elif defined(CONFIG_AD100N_DDR)
  #define CONFIG_SYS_MPLL_FREQ        1400000000    /*If MPLL not use mast be set 0*/
  #define CONFIG_SYS_MEM_FREQ         700000000
  #define CONFIG_SYS_AHB0_FREQ        350000000
  #define CONFIG_SYS_AHB2_FREQ        280000000    /*APB = AHB2/2*/
#else
  #error "please add more define here"
#endif

#define CONFIG_SYS_UART_INDEX         2
#define CONFIG_BAUDRATE               115200
#define CONFIG_AUTO_COMPLETE

/**
 * Boot command definitions.
 */
#define CONFIG_BOOTDELAY 1

/**
 * Boot arguments definitions.
 */

#if defined(CONFIG_AD101P_DDR)
#define BOOTARGS_COMMON "console=ttyS2,115200 mem=128M@0x0 quiet"
#elif defined(CONFIG_AD100N_DDR)
#define BOOTARGS_COMMON "console=ttyS2,115200 mem=64M@0x0 quiet"
#else
#error "please add boards config"
#endif

#define CONFIG_BOOTARGS BOOTARGS_COMMON " ip=off init=/linuxrc rootfstype=cramfs root=/dev/mtdblock2 rw flashtype=nor"
#define CONFIG_BOOTCOMMAND "sfcnor read 0x40000 0x600000 0x80a00000 ;bootm 0x80a00000"

#endif/*END OF __AD100_SLT_COMMON_H__ */
