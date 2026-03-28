#include <config.h>
#include <serial.h>
#include <common.h>
#include <lcd.h>
#include <linux/list.h>
#include <linux/fb.h>
#include <asm/types.h>
#include <asm/arch/pwm.h>
#include <asm/arch/gpio.h>
#include <regulator.h>


#include <jz_lcd/jz_lcd_v14.h>

vidinfo_t panel_info = { 320 , 480, LCD_BPP, };
extern struct jzfb_config_info lcd_config_info;

static struct smart_lcd_data_table fw035_data_table[] = {
/* LCD init code */
	{SMART_CONFIG_CMD    , 0xE0},
	{SMART_CONFIG_PRM    , 0x00},
	{SMART_CONFIG_PRM    , 0x07},
	{SMART_CONFIG_PRM    , 0x10},
	{SMART_CONFIG_PRM    , 0x09},
	{SMART_CONFIG_PRM    , 0x17},
	{SMART_CONFIG_PRM    , 0x0B},
	{SMART_CONFIG_PRM    , 0x40},
	{SMART_CONFIG_PRM    , 0x8A},
	{SMART_CONFIG_PRM    , 0x4B},
	{SMART_CONFIG_PRM    , 0x0A},
	{SMART_CONFIG_PRM    , 0x0D},
	{SMART_CONFIG_PRM    , 0x0F},
	{SMART_CONFIG_PRM    , 0x15},
	{SMART_CONFIG_PRM    , 0x16},
	{SMART_CONFIG_PRM    , 0x0F},

	{SMART_CONFIG_CMD    , 0xE1},
	{SMART_CONFIG_PRM    , 0x00},
	{SMART_CONFIG_PRM    , 0x1A},
	{SMART_CONFIG_PRM    , 0x1B},
	{SMART_CONFIG_PRM    , 0x02},
	{SMART_CONFIG_PRM    , 0x0D},
	{SMART_CONFIG_PRM    , 0x05},
	{SMART_CONFIG_PRM    , 0x30},
	{SMART_CONFIG_PRM    , 0x35},
	{SMART_CONFIG_PRM    , 0x43},
	{SMART_CONFIG_PRM    , 0x02},
	{SMART_CONFIG_PRM    , 0x0A},
	{SMART_CONFIG_PRM    , 0x09},
	{SMART_CONFIG_PRM    , 0x32},
	{SMART_CONFIG_PRM    , 0x36},
	{SMART_CONFIG_PRM    , 0x0F},

	{SMART_CONFIG_CMD    , 0xB1},
	{SMART_CONFIG_PRM    , 0xA0},
	{SMART_CONFIG_PRM    , 0x11},

	{SMART_CONFIG_CMD    , 0xB4},
	{SMART_CONFIG_PRM    , 0x02},

	{SMART_CONFIG_CMD    , 0xC0},
	{SMART_CONFIG_PRM    , 0x17},
	{SMART_CONFIG_PRM    , 0x15},

	{SMART_CONFIG_CMD    , 0xC1},
	{SMART_CONFIG_PRM    , 0x41},

	{SMART_CONFIG_CMD    , 0xC5},
	{SMART_CONFIG_PRM    , 0x00},
	{SMART_CONFIG_PRM    , 0x0A},
	{SMART_CONFIG_PRM    , 0x80},

	{SMART_CONFIG_CMD    , 0xB6},
	{SMART_CONFIG_PRM    , 0x02},

	{SMART_CONFIG_CMD    , 0x36},
	{SMART_CONFIG_PRM    , 0x48},

	{SMART_CONFIG_CMD    , 0x3A},
	{SMART_CONFIG_PRM    , 0x56},

	{SMART_CONFIG_CMD    , 0xE9},
	{SMART_CONFIG_PRM    , 0x00},

	{SMART_CONFIG_CMD    , 0xF7},
	{SMART_CONFIG_PRM    , 0xA9},
	{SMART_CONFIG_PRM    , 0x51},
	{SMART_CONFIG_PRM    , 0x2C},
	{SMART_CONFIG_PRM    , 0x82},

	{SMART_CONFIG_CMD    , 0x11},
	{SMART_CONFIG_UDELAY, 120000},
	{SMART_CONFIG_CMD    , 0x29},
	{SMART_CONFIG_CMD    , 0x2c},

};

struct fb_videomode jzfb1_videomode = {
		.name = "320x480",
		.refresh = 60,
		.xres = 320,
		.yres = 480,
		.pixclock = KHZ2PICOS(18432),
		.left_margin = 0,
		.right_margin = 0,
		.upper_margin = 0,
		.lower_margin = 0,
		.hsync_len = 0,
		.vsync_len = 0,
		.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
		.vmode = FB_VMODE_NONINTERLACED,
		.flag = 0,
};

struct jzfb_smart_config fw035_cfg = {
	.te_anti_jit = 1,
	.te_md = 0,
	.te_switch = 0,
	.te_dp = 1,
	.dc_md = 0,
	.wr_md = 1,
	.smart_type = SMART_LCD_TYPE_8080,
	.pix_fmt = SMART_LCD_FORMAT_888,
	.dwidth = SMART_LCD_DWIDTH_8_BIT,
	.cwidth = SMART_LCD_CWIDTH_8_BIT,
	.bus_width = 8,

	.write_gram_cmd = 0x2C,
	.data_table = fw035_data_table,
	.length_data_table = ARRAY_SIZE(fw035_data_table),
};

struct jzfb_config_info jzfb1_init_data = {
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_SLCD,
	.bpp = 24,
	.smart_config = &fw035_cfg,
	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,
	.dither_enable          = 0,
	.dither.dither_red      = 0,
	.dither.dither_green    = 0,
	.dither.dither_blue     = 0,
};


#define GPIO_PORTA_SET_FUNC1(pin)			\
do {					\
		*((volatile unsigned int *)0xB0010018) = (1 << pin); \
		*((volatile unsigned int *)0xB0010028) = (1 << pin); \
		*((volatile unsigned int *)0xB0010038) = (1 << pin); \
		*((volatile unsigned int *)0xB0010044) = (1 << pin); \
}while(0)

void panel_pin_init(void)
{
	/** set gpiod as slcd func */
	 GPIO_PORTA_SET_FUNC1(0);
	 GPIO_PORTA_SET_FUNC1(1);
	 GPIO_PORTA_SET_FUNC1(2);
	 GPIO_PORTA_SET_FUNC1(3);
	 GPIO_PORTA_SET_FUNC1(4);
	 GPIO_PORTA_SET_FUNC1(5);
	 GPIO_PORTA_SET_FUNC1(6);
	 GPIO_PORTA_SET_FUNC1(7);
	 GPIO_PORTA_SET_FUNC1(25);
	 GPIO_PORTA_SET_FUNC1(26);
	 GPIO_PORTA_SET_FUNC1(27);
}

static void panel_power_reset(void)
{
	gpio_direction_output(CONFIG_SLCD_RST_PIN, 0);
	mdelay(120);
	gpio_direction_output(CONFIG_SLCD_RST_PIN, 1);
	mdelay(120);
}

void panel_power_on(void)
{
	gpio_direction_output(CONFIG_SLCD_RST_PIN, 1);
	gpio_direction_output(CONFIG_SLCD_CS_PIN, 1);
	gpio_direction_output(CONFIG_SLCD_RD_PIN, 1);
	gpio_direction_output(CONFIG_SLCD_VDD_PIN, 1);
	mdelay(10);
	panel_power_reset();
	gpio_direction_output(CONFIG_SLCD_CS_PIN, 0);
	 dump_gpio_func(CONFIG_SLCD_VDD_PIN);
	 dump_gpio_func(CONFIG_SLCD_RD_PIN);
	 dump_gpio_func(CONFIG_SLCD_CS_PIN);
	 dump_gpio_func(CONFIG_SLCD_RST_PIN);
}

void panel_power_off(void)
{
	gpio_direction_output(CONFIG_SLCD_CS_PIN, 1);
	gpio_direction_output(CONFIG_SLCD_RST_PIN, 1);
	gpio_direction_output(CONFIG_SLCD_VDD_PIN, 0);
	gpio_direction_output(CONFIG_SLCD_RD_PIN, 0);
}

void panel_init_sequence(struct dsi_device *dsi)
{
	printf("%s %s ###########\r\n",__FILE__,__func__);
}

