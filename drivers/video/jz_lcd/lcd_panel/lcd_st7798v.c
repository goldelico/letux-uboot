#include <config.h>
#include <serial.h>
#include <common.h>
#include <lcd.h>
#include <linux/list.h>
#include <linux/fb.h>
#include <asm/types.h>
#include <asm/arch/tcu.h>
#include <asm/arch/gpio.h>
#include <regulator.h>


#include <jz_lcd/jz_lcd_v14.h>

vidinfo_t panel_info = { 240 , 320, LCD_BPP, };
extern struct jzfb_config_info lcd_config_info;

static struct smart_lcd_data_table st7789v_data_table[] = {
#if 1
	{SMART_CONFIG_CMD,	0x11},
	{SMART_CONFIG_CMD,	0x35},
	{SMART_CONFIG_PRM,  0x00},
	{SMART_CONFIG_CMD,	0x36},//Display Setting
	//{SMART_CONFIG_PRM,  (1<<6)/*(1<<5)|(1<<6)*/},
	{SMART_CONFIG_PRM,  0x00},
	{SMART_CONFIG_CMD,	 0x3A},
	{SMART_CONFIG_PRM,   0x05},
	{SMART_CONFIG_CMD,	 0xB2},
	{SMART_CONFIG_PRM,   0x0C},
	{SMART_CONFIG_PRM,   0x0C},
	{SMART_CONFIG_PRM,   0x00},
	{SMART_CONFIG_PRM,   0x33},
	{SMART_CONFIG_PRM,   0x33},
	{SMART_CONFIG_CMD,   0xB7},
	{SMART_CONFIG_PRM,   0x75},
	{SMART_CONFIG_CMD,   0xBB},
	{SMART_CONFIG_PRM,   0x19},
	{SMART_CONFIG_CMD,   0xC0},
	{SMART_CONFIG_PRM,   0x2C},
	{SMART_CONFIG_CMD,   0xC2},
	{SMART_CONFIG_PRM,   0x01},
	{SMART_CONFIG_CMD,   0xC3},
	{SMART_CONFIG_PRM,   0x0C},
	{SMART_CONFIG_CMD,   0xC4},
	{SMART_CONFIG_PRM,   0x20},
	{SMART_CONFIG_CMD,   0xC6},
	{SMART_CONFIG_PRM,   0x0F},
	{SMART_CONFIG_CMD,   0xD0},
	{SMART_CONFIG_PRM,   0xA4},
	{SMART_CONFIG_PRM,   0xA1},
	{SMART_CONFIG_CMD,   0xE0},//Gamma setting
	{SMART_CONFIG_PRM,   0xD0},
	{SMART_CONFIG_PRM,   0x04},
	{SMART_CONFIG_PRM,   0x0C},
	{SMART_CONFIG_PRM,   0x0E},
	{SMART_CONFIG_PRM,   0x0E},
	{SMART_CONFIG_PRM,   0x29},
	{SMART_CONFIG_PRM,   0x37},
	{SMART_CONFIG_PRM,   0x44},
	{SMART_CONFIG_PRM,   0x47},
	{SMART_CONFIG_PRM,   0x0B},
	{SMART_CONFIG_PRM,   0x17},
	{SMART_CONFIG_PRM,   0x16},
	{SMART_CONFIG_PRM,   0x1B},
	{SMART_CONFIG_PRM,   0x1F},
	{SMART_CONFIG_CMD,   0xE1},
	{SMART_CONFIG_PRM,   0xD0},
	{SMART_CONFIG_PRM,   0x04},
	{SMART_CONFIG_PRM,   0x0C},
	{SMART_CONFIG_PRM,   0x0E},
	{SMART_CONFIG_PRM,   0x0F},
	{SMART_CONFIG_PRM,   0x29},
	{SMART_CONFIG_PRM,   0x37},
	{SMART_CONFIG_PRM,   0x44},
	{SMART_CONFIG_PRM,   0x4A},
	{SMART_CONFIG_PRM,   0x0C},
	{SMART_CONFIG_PRM,   0x17},
	{SMART_CONFIG_PRM,   0x16},
	{SMART_CONFIG_PRM,   0x1B},
	{SMART_CONFIG_PRM,   0x1F},
	{SMART_CONFIG_CMD,   0x29},

	{SMART_CONFIG_CMD,   0x2A},
	{SMART_CONFIG_PRM,   0x0 },//Xstart
	{SMART_CONFIG_PRM,   0x0 },
	{SMART_CONFIG_PRM,   0x0 },//Xend
	{SMART_CONFIG_PRM,   0xEF},
	{SMART_CONFIG_CMD,   0x2B},
	{SMART_CONFIG_PRM,   0x0 },//Ystart
	{SMART_CONFIG_PRM,   0x0 },
	{SMART_CONFIG_PRM,   0x01},//Yend
	{SMART_CONFIG_PRM,   0x3F},
	{SMART_CONFIG_CMD,   0x2C},
#endif
};
struct fb_videomode jzfb1_videomode = {
		.name = "240x320",
		.refresh = 60,
		.xres = 240,
		.yres = 320,
		.pixclock = KHZ2PICOS(30000),
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

struct jzfb_smart_config st7789v_cfg = {
	//.te_anti_jit = 1,
	//.te_md = 0,
	.te_switch = 0,
	//.te_dp = 1,
	.dc_md = 0,
	.wr_md = 1,
	.smart_type = SMART_LCD_TYPE_8080,
	.pix_fmt = SMART_LCD_FORMAT_565,
	.dwidth = SMART_LCD_DWIDTH_8_BIT,
	.cwidth = SMART_LCD_CWIDTH_8_BIT,
	.bus_width = 8,

	.write_gram_cmd = 0x2C,
	.data_table = st7789v_data_table,
	.length_data_table = ARRAY_SIZE(st7789v_data_table),
};

struct jzfb_config_info jzfb1_init_data = {
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_SLCD,
	.bpp = 24,
	.smart_config = &st7789v_cfg,
	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,
	.dither_enable          = 0,
	.dither.dither_red      = 0,
	.dither.dither_green    = 0,
	.dither.dither_blue     = 0,
};


#define GPIO_PORTD_SET_FUNC2(pin)			\
do {					\
		*((volatile unsigned int *)0xB0013018) = (1 << pin); \
		*((volatile unsigned int *)0xB0013028) = (1 << pin); \
		*((volatile unsigned int *)0xB0013034) = (1 << pin); \
		*((volatile unsigned int *)0xB0013048) = (1 << pin); \
}while(0)

void panel_pin_init(void)
{
	/** set gpiod as slcd func */
	 GPIO_PORTD_SET_FUNC2(0);
	 GPIO_PORTD_SET_FUNC2(1);
	 GPIO_PORTD_SET_FUNC2(2);
	 GPIO_PORTD_SET_FUNC2(3);
	 GPIO_PORTD_SET_FUNC2(4);
	 GPIO_PORTD_SET_FUNC2(5);
	 GPIO_PORTD_SET_FUNC2(6);
	 GPIO_PORTD_SET_FUNC2(7);
	 GPIO_PORTD_SET_FUNC2(8);
	 GPIO_PORTD_SET_FUNC2(11);

	 /* dump_gpio_func(GPIO_PD(0)); */
	 /* dump_gpio_func(GPIO_PD(1)); */
	 /* dump_gpio_func(GPIO_PD(2)); */
	 /* dump_gpio_func(GPIO_PD(3)); */
	 /* dump_gpio_func(GPIO_PD(4)); */
	 /* dump_gpio_func(GPIO_PD(5)); */
	 /* dump_gpio_func(GPIO_PD(6)); */
	 /* dump_gpio_func(GPIO_PD(7)); */
	 /* dump_gpio_func(GPIO_PD(8)); */
	 /* dump_gpio_func(GPIO_PD(11)); */
}

static void panel_power_reset(void)
{
	gpio_port_direction_output(CONFIG_SLCD_RST_PIN / 32, CONFIG_SLCD_RST_PIN % 32,0);
	mdelay(20);
	gpio_port_direction_output(CONFIG_SLCD_RST_PIN / 32, CONFIG_SLCD_RST_PIN % 32,1);
	mdelay(20);
}

void panel_power_on(void)
{
	gpio_port_direction_output(CONFIG_SLCD_BL_PIN / 32,CONFIG_SLCD_BL_PIN % 32,1);
	gpio_port_direction_output(CONFIG_SLCD_VDDEN_PIN / 32,CONFIG_SLCD_VDDEN_PIN % 32,1);
	gpio_port_direction_output(CONFIG_SLCD_RD_PIN / 32,CONFIG_SLCD_RD_PIN % 32,1);
	gpio_port_direction_output(CONFIG_SLCD_RST_PIN / 32,CONFIG_SLCD_RST_PIN % 32,1);
	gpio_port_direction_output(CONFIG_SLCD_CS_PIN / 32,CONFIG_SLCD_CS_PIN % 32,1);
	mdelay(10);
	panel_power_reset();
	gpio_port_direction_output(CONFIG_SLCD_CS_PIN / 32,CONFIG_SLCD_CS_PIN % 32,0);

	 /* dump_gpio_func(CONFIG_SLCD_BL_PIN); */
	 /* dump_gpio_func(CONFIG_SLCD_VDDEN_PIN); */
	 /* dump_gpio_func(CONFIG_SLCD_RD_PIN); */
	 /* dump_gpio_func(CONFIG_SLCD_CS_PIN); */
	 /* dump_gpio_func(CONFIG_SLCD_RST_PIN); */
}

void panel_power_off(void)
{
	gpio_port_direction_output(CONFIG_SLCD_CS_PIN / 32,CONFIG_SLCD_CS_PIN % 32,0);
	gpio_port_direction_output(CONFIG_SLCD_RST_PIN / 32,CONFIG_SLCD_RST_PIN % 32,0);
	gpio_port_direction_output(CONFIG_SLCD_RD_PIN / 32,CONFIG_SLCD_RD_PIN % 32,0);
	gpio_port_direction_output(CONFIG_SLCD_VDDEN_PIN / 32,CONFIG_SLCD_VDDEN_PIN % 32,0);
	gpio_port_direction_output(CONFIG_SLCD_BL_PIN / 32,CONFIG_SLCD_BL_PIN % 32,0);
}

void panel_init_sequence(struct dsi_device *dsi)
{
	printf("%s %s ###########\r\n",__FILE__,__func__);
}

