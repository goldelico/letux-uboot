#ifndef __EFUSE_H__
#define __EFUSE_H__

#include <linux/types.h>

#define EFUSE_R_CHIP_ID		(0x1)
#define EFUSE_R_USER_ID		(0x2)
#define EFUSE_R_RN		(0X3)


#if defined(CONFIG_CMD_EFUSE)
int efuse_write(void *buf, int length, off_t offset);
int efuse_read(void *buf, int length, off_t offset);
int efuse_read_chipid(void *buf, int length, off_t offset);
int efuse_read_id(void *buf, int length, int id);
int efuse_init(int gpio_pin, int active);
void efuse_deinit(void);
void efuse_debug_enable(int enable);
#else
static int inline efuse_write(void *buf, int length, off_t offset) {return 0;}
static int inline efuse_read(void *buf, int length, off_t offset) {return 0;}
static int inline efuse_read_chipid(void *buf, int length, off_t offset) {return 0;}
static int efuse_read_id(void *buf, int length, int id) {return 0;}
static int inline efuse_init(int gpio_pin, int active) {return 0;}
static void inline efuse_deinit(void)	{;}
static void inline efuse_debug_enable(int enable) {;}
#endif
#endif	/*EFUSE_H*/
