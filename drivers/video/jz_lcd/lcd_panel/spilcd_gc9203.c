/*
 * JZ LCD PANEL DATA: spilcd-r108103_gc9203.c
 */

#include <config.h>
#include <serial.h>
#include <common.h>
#include <lcd.h>
#include <linux/list.h>
//#include <linux/fb.h>
#include <asm/types.h>
#include <asm/arch-m200/tcu.h>
#include <asm/arch-m200/lcdc.h>
#include <asm/arch-m200/gpio.h>
#include <regulator.h>
#include <jz_lcd/jz_lcd_v12.h>
#include <jz_lcd/spilcd_gc9203.h>

#define LCD_XRES (220)
#define LCD_YRES (128+1)

vidinfo_t panel_info = {LCD_XRES, LCD_YRES, LCD_BPP, };

extern struct spilcd_gc9203_data spilcd_gc9203_pdata;

#define GPIO_PORTC_SET_FUNC0(pin)		\
do {					\
		*((volatile unsigned int *)0xB0010218) = pin; \
		*((volatile unsigned int *)0xB0010228) = pin; \
		*((volatile unsigned int *)0xB0010238) = pin; \
		*((volatile unsigned int *)0xB0010248) = pin; \
}while(0)
#define GPIO_PORTC_SET_FUNC2(pin)			\
do {					\
		*((volatile unsigned int *)0xB0010218) = pin; \
		*((volatile unsigned int *)0xB0010228) = pin; \
		*((volatile unsigned int *)0xB0010234) = pin; \
		*((volatile unsigned int *)0xB0010248) = pin; \
}while(0)


#include "spilcd_gc9203_init.c"



#if 0
/**
 * lcd_open_backlight() - Overwrite the weak function defined at common/lcd.c
 */
void lcd_open_backlight(void)
{
	gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_ledpwr, 1);
	return;
}

/**
 * lcd_close_backlight() - Overwrite the weak function defined at common/lcd.c
 */
void lcd_close_backlight(void)
{
	gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_ledpwr, 0);
	return;
}
#endif

void panel_pin_init(void)
{
	int ret = 0;
	serial_puts("spilcd_gc9203 panel display pin init\n");

#define REQUEST_GPIO(GPIO)						\
	ret = gpio_request(spilcd_gc9203_pdata.GPIO, #GPIO);		\
	if(ret){							\
		printf("canot request gpio" #GPIO ",ret=0x%x\n",ret);	\
	}

	REQUEST_GPIO(gpio_lcd_vddpwr);
	REQUEST_GPIO(gpio_lcd_vccpwr);
	REQUEST_GPIO(gpio_lcd_ledpwr);
	REQUEST_GPIO(gpio_lcd_rst);
	REQUEST_GPIO(gpio_spi_cs);
	REQUEST_GPIO(gpio_spi_rs);
	REQUEST_GPIO(gpio_spi_scl);
	REQUEST_GPIO(gpio_spi_sda);

#undef REQUEST_GPIO

}

static void panel_reset(void)
{
	/*power reset*/
        gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_rst, 1);
        mdelay(20);
	gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_rst, 0);
	mdelay(20);
	gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_rst, 1);
	mdelay(120);
}

void panel_power_on(void)
{
	serial_puts("spilcd_gc9203 panel display on\n");
	//regulator_enable(dev->lcd_vcc_reg);


	gpio_direction_output(spilcd_gc9203_pdata.gpio_spi_cs, 1);
	gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_vddpwr, 0);
	gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_vccpwr, 1);
	gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_ledpwr, 1);


	lcd_open_backlight();

	if (1) {
		/* setup oled by gpio simulator */
		Init_Oled();
	}

	if (1) {
		unsigned int pins;
		printk("=========================begin to set gpio func!!!!!!!!\n");
		pins = 1<<19 | 1<<25 | 1<<26;
		//pins = 1<< 8 | 1<<19 | 1<<25 | 1<<26;
		//pins = 1<<26;
		printk("PORTC func2: 0x%08x\n", pins);
		GPIO_PORTC_SET_FUNC2(pins);
		//pins = 1<<8;
		//pins = 1<< 8 | 1<<19 | 1<<25;
		//pins = 1<< 8 | 1<<25;
		//pins = 1<< 8 | 1<<19 | 1<<25 | 1<<26;
		//pins = 1<< 8 | 1<<19 | 1<<26;
		//printk("PORTC func0: 0x08x\n", pins);
		//GPIO_PORTC_SET_FUNC0(pins);
	}

	//RS_DC(1);
	CS(0);
}


void panel_power_off(void)
{
	gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_ledpwr, 0);
	gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_vddpwr, 1);
	gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_vccpwr, 0);
	serial_puts("spilcd_gc9203 panel display off\n");
}

static struct smart_lcd_data_table gc9203_init_cmd_table[] = {
};

/* cmd buffer at least 4 words. */
static unsigned long gc9203_cmd_buf[]= {
    0x22222222,
    0x22222222,
    0x22222222,
    0x22222222,
};


struct fb_videomode jzfb1_videomode = {
	.name = "spilcd_gc9203-220x128",
	.refresh = 60,
	.xres = LCD_XRES,
	.yres = LCD_YRES,
	/* VDDIO 1.8V, tSCYC min 80ns; VDDIO 3.3V, tSCYC min 25ns; */
	.pixclock = KHZ2PICOS(20000),
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

struct jzfb_config_info jzfb1_init_data = {
	.num_modes = 1,
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_SLCD,
	.bpp    = NBITS(LCD_BPP),//16,
	.pinmd  = 1,

	.smart_config.smart_type           = 1,
	.smart_config.newcfg_datatx_type   = 1,
	.smart_config.newcfg_cmdtx_type    = 1,
	//.smart_config.bus_width = 8,
	.smart_config.bus_width = 16,

	.smart_config.clkply_active_rising = 0,
	.smart_config.rsply_cmd_high       = 0,
	.smart_config.csply_active_high    = 0,
	.smart_config.newcfg_fmt_conv      = 1,

	/* write graphic ram command, in word, for example 8-bit bus, write_gram_cmd=C3C2C1C0. */
	.smart_config.write_gram_cmd = gc9203_cmd_buf,
	.smart_config.length_cmd = ARRAY_SIZE(gc9203_cmd_buf),
	.smart_config.data_table = gc9203_init_cmd_table,
	.smart_config.length_data_table =  ARRAY_SIZE(gc9203_init_cmd_table),
	.dither_enable = 0,
};
