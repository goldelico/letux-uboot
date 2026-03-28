#include <asm/gpio.h>
#include <serial.h>

#define SPI_GPIO_CS CONFIG_GPIO_SPI_CS
#define SPI_GPIO_DT CONFIG_GPIO_SPI_DT
#define SPI_GPIO_DR CONFIG_GPIO_SPI_DR
#define SPI_GPIO_CLK CONFIG_GPIO_SPI_CLK

enum {
    CMD_WRITE_FIFO,
    CMD_READ_FIFO_CNT,
    CMD_READ_FIFO,
};

static void send_byte(unsigned int v)
{
    int i;

    int spi_dt = SPI_GPIO_DT;
    int spi_clk = SPI_GPIO_CLK;

    for (i = 0; i < 8; i++) {
        int value = !!(v & (1 << (7 - i)));
        gpio_set_value(spi_dt, value);
        udelay(1);
        gpio_set_value(spi_clk, 1);
        udelay(1);
        gpio_set_value(spi_clk, 0);
    }
    udelay(1);
}

static int receive_byte(unsigned char *b)
{
    int i;
    int v = 0;

    int spi_dr = SPI_GPIO_DR;
    int spi_clk = SPI_GPIO_CLK;

    for (i = 0; i < 8; i++) {
        gpio_set_value(spi_clk, 1);
        udelay(1);
        gpio_set_value(spi_clk, 0);
        udelay(1);
        if (gpio_get_value(spi_dr))
            v |= 1 << (7 - i);
    }

    *b = v;

    return 0;
}

static void set_cs_low(void)
{
    int spi_cs = SPI_GPIO_CS;

    gpio_set_value(spi_cs, 0);
    udelay(1);
}

static void set_cs_high(void)
{
    int spi_cs = SPI_GPIO_CS;

    udelay(1);
    gpio_set_value(spi_cs, 1);
    udelay(1);
}

static void send_char(unsigned char c)
{
    send_byte(CMD_WRITE_FIFO);
    send_byte(c);
}

static void receive_char(unsigned char cmd,  unsigned char *c)
{
    send_byte(cmd);
    receive_byte(c);
}

static void spi_uart_transfer(unsigned char c)
{
    set_cs_low();
    send_char(c);
    set_cs_high();
}

static void gpio_spi_to_uart_putc(char c)
{
    unsigned char r = '\r';
    if (c == '\n')
        spi_uart_transfer(r);

    spi_uart_transfer(c);
}

void gpio_spi_to_uart_puts(char *s)
{
    while (*s)
        gpio_spi_to_uart_putc(*s++);
}

int gpio_spi_to_uart_init(void)
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
