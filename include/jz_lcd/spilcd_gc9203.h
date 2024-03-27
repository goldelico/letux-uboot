#ifndef __SPILCD_GC9203_H
#define __SPILCD_GC9203_H

struct spilcd_gc9203_data
{
	unsigned int gpio_lcd_vddpwr;
	unsigned int gpio_lcd_vccpwr;
	unsigned int gpio_lcd_ledpwr; /* backlight led power */
	unsigned int gpio_lcd_rst; /* This signal will reset the device and it must be applied to properly initialize the chip. Signal is active low. */
	unsigned int gpio_spi_cs;  /* Chip selection pin; Low enable, High disable. */
	unsigned int gpio_spi_rs;  /* Display data/command selection pin in 4-line serial interface. */
	unsigned int gpio_spi_scl; /* This pin is used to be serial interface clock. */
	unsigned int gpio_spi_sda; /* SPI interface input/output pin. The data is latched on the rising edge of the SCL signal. */
};


#endif //__SPILCD_GC9203_H
