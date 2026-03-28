#include <asm/gpio.h>
#include <serial.h>

#define SPI_GPIO_CS CONFIG_GPIO_SPI_CS
#define SPI_GPIO_DT CONFIG_GPIO_SPI_DT
#define SPI_GPIO_DR CONFIG_GPIO_SPI_DR
#define SPI_GPIO_CLK CONFIG_GPIO_SPI_CLK

enum {
    CMD_ZERO,
    CMD_WRITE_FIFO,
    CMD_READ_FIFO_CNT,
    CMD_READ_FIFO,
};

static void set_cs_low(void)
{
    gpio_set_value(SPI_GPIO_CS, 0);
    udelay(1);
}

static void set_cs_high(void)
{
    udelay(1);
    gpio_set_value(SPI_GPIO_CS, 1);
    udelay(1);
}

static void tx_rx_byte(unsigned char t, unsigned char *b)
{
    unsigned char i;
    unsigned char v = 0;

    unsigned char spi_dt = SPI_GPIO_DT;
    unsigned char spi_dr = SPI_GPIO_DR;
    unsigned char spi_clk = SPI_GPIO_CLK;

    set_cs_low();
    for (i = 0; i < 8; i++) {
        unsigned char value = !!(t & (1 << (7 - i)));
        gpio_set_value(spi_dt, value);
        gpio_set_value(spi_clk, 0);
        udelay(1);
        if (gpio_get_value(spi_dr))
            v |= 1 << (7 - i);
        gpio_set_value(spi_clk, 1);
        udelay(1);
    }
    set_cs_high();

    *b = v;
}

static void send_char(unsigned char c)
{
    unsigned char data = 0;
    tx_rx_byte(CMD_WRITE_FIFO, &data);
    tx_rx_byte(c, &data);
}

static void spi_uart_transfer(unsigned char c)
{
    send_char(c);
}

static void gpio_spi_to_uart_putc2(char c)
{
    unsigned char r = '\r';
    if (c == '\n')
        spi_uart_transfer(r);

    spi_uart_transfer(c);
}

void gpio_spi_to_uart_puts2(char *s)
{
    while (*s)
        gpio_spi_to_uart_putc2(*s++);
}

int gpio_spi_to_uart_init2(void)
{
    gpio_direction_output(SPI_GPIO_CS, 1);
    gpio_direction_output(SPI_GPIO_DT, 0);
    gpio_direction_input(SPI_GPIO_DR);
    gpio_direction_output(SPI_GPIO_CLK, 0);
}

__weak struct serial_device *default_serial_console(void)
{
    return NULL;
}
