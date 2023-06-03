/*
 * Ingenic dorado lcd code
 */

#include <regulator.h>
#include <asm/gpio.h>
#include <jz_lcd/jz_lcd_v12.h>
#include <jz_lcd/spilcd_gc9203.h>

#define GPIO_LCD_VDDEN  GPIO_PC(23)
#define GPIO_LCD_VCCEN  GPIO_PC(9)
#define GPIO_LCD_LEDPWR GPIO_PE(1)
#define GPIO_LCD_RST    GPIO_PA(13)

#define GPIO_LCD_CS     GPIO_PC(8)
#define GPIO_LCD_RS     GPIO_PC(26)
#define GPIO_SPI_SCL    GPIO_PC(25)
#define GPIO_SPI_SDA    GPIO_PC(19)

void board_set_lcd_power_on(void)
{
	printf("%s() regulator lcd enable.\n", __func__);
#if 0	
	char *id = "lcd_1.8v";
	struct regulator *lcd_regulator = regulator_get(id);
	regulator_set_voltage(lcd_regulator, 3300000, 1800000);
	regulator_enable(lcd_regulator);
#endif
	//regulator_enable(dev->lcd_vcc_reg);
}

struct spilcd_gc9203_data spilcd_gc9203_pdata = {
	.gpio_lcd_vddpwr = GPIO_LCD_VDDEN,
	.gpio_lcd_vccpwr = GPIO_LCD_VCCEN,
	.gpio_lcd_ledpwr = GPIO_LCD_LEDPWR,
	.gpio_lcd_rst    = GPIO_LCD_RST,
	.gpio_spi_cs     = GPIO_LCD_CS,
	.gpio_spi_rs     = GPIO_LCD_RS,
	.gpio_spi_scl    = GPIO_SPI_SCL,
	.gpio_spi_sda    = GPIO_SPI_SDA,
};
