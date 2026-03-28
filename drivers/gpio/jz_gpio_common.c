/*
 * JZ4775 common routines
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Sonil <ztyan@ingenic.cn>
 * Based on: newxboot/modules/gpio/jz4775_gpio.c|jz4780_gpio.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/cpm.h>

#include <ingenic_soft_i2c.h>
#include <jz_pca953x.h>

#define JZGPIO_GROUP_OFFSET     (0x100)
#if defined (CONFIG_JZ4775)
#include "jz_gpio/jz4775_gpio.c"
#elif defined (CONFIG_JZ4780)
#include "jz_gpio/jz4780_gpio.c"
#elif defined (CONFIG_M200)
#include "jz_gpio/m200_gpio.c"
#elif defined (CONFIG_M150)
#include "jz_gpio/m150_gpio.c"
#elif defined (CONFIG_X1000)
#include "jz_gpio/x1000_gpio.c"
#elif defined (CONFIG_X2000)
#include "jz_gpio/x2000_gpio.c"
#elif defined (CONFIG_X2000_V12)
#include "jz_gpio/x2000_v12_gpio.c"
#elif defined (CONFIG_X2100)
#include "jz_gpio/x2100_gpio.c"
#elif defined (CONFIG_X2500)
#define JZGPIO_GROUP_OFFSET     (0x1000)
#include "jz_gpio/x2500_gpio.c"
#elif defined (CONFIG_M300)
#include "jz_gpio/m300_gpio.c"
#elif defined (CONFIG_X1520)
#include "jz_gpio/x1520_gpio.c"
#elif defined (CONFIG_X1800)
#include "jz_gpio/x1800_gpio.c"
#elif defined (CONFIG_X1520)
#include "jz_gpio/x1520_gpio.c"
#elif defined (CONFIG_X1630)
#undef JZGPIO_GROUP_OFFSET
#define JZGPIO_GROUP_OFFSET     (0x1000)
#include "jz_gpio/x1630_gpio.c"
#elif defined (CONFIG_X1830)
#undef JZGPIO_GROUP_OFFSET
#define JZGPIO_GROUP_OFFSET     (0x1000)
#include "jz_gpio/x1830_gpio.c"
#elif defined (CONFIG_T31)
#undef JZGPIO_GROUP_OFFSET
#define JZGPIO_GROUP_OFFSET     (0x1000)
#include "jz_gpio/t31_gpio.c"
#elif defined (CONFIG_X1021)
#undef JZGPIO_GROUP_OFFSET
#define JZGPIO_GROUP_OFFSET     (0x1000)
#include "jz_gpio/x1021_gpio.c"
#elif defined (CONFIG_X1521)
#undef JZGPIO_GROUP_OFFSET
#define JZGPIO_GROUP_OFFSET     (0x1000)
#include "jz_gpio/x1521_gpio.c"
#elif defined (CONFIG_X1600)
#include "jz_gpio/x1600_gpio.c"
#elif defined (CONFIG_A1)
#undef JZGPIO_GROUP_OFFSET
#define JZGPIO_GROUP_OFFSET     (0x1000)
#include "jz_gpio/a1_gpio.c"
#elif defined (CONFIG_AD100)
#undef JZGPIO_GROUP_OFFSET
#define JZGPIO_GROUP_OFFSET     (0x1000)
#include "jz_gpio/ad100_gpio.c"
#elif defined (CONFIG_X2580)
#undef JZGPIO_GROUP_OFFSET
#define JZGPIO_GROUP_OFFSET     (0x1000)
#include "jz_gpio/x2580_gpio.c"
#elif defined (CONFIG_X2600)
#undef JZGPIO_GROUP_OFFSET
#define JZGPIO_GROUP_OFFSET     (0x1000)
#include "jz_gpio/x2600_gpio.c"
#endif
DECLARE_GLOBAL_DATA_PTR;

#define BIT(nr)         (1UL << (nr))

static inline int is_gpio_from_chip(int gpio_num)
{
	return gpio_num < (GPIO_NR_PORTS * 32) ? 1 : 0;
}

#if defined(PXPEL) && defined(PXPEH)
void gpio_set_driver_state(enum gpio_port n, unsigned int pins, unsigned int state)
{
	unsigned int base = GPIO_BASE + JZGPIO_GROUP_OFFSET * n;
	unsigned int tmp = pins;
	unsigned int val = 0;
	unsigned int pin = 0;

	if (tmp & 0xffff) {
		val = readl(base + PXPEL);
		while (!!(pin = fls((tmp & 0xffff)))) {
			pin = pin - 1;
			tmp &= ~(BIT(pin));
			val |= state << (pin << 1);
		}

		writel(val, base + PXPEL);
	}
	if (tmp & 0xffff0000) {
		val = readl(base + PXPEH);
		while (!!(pin = fls((tmp)))) {
			pin = pin - 1;
			tmp &= ~(BIT(pin));
			val |= state << ((pin - 16) << 1);
		}

		writel(val, base + PXPEH);
	}
}
#endif

void gpio_set_func(enum gpio_port n, enum gpio_function func, unsigned int pins)
{
	unsigned int base = GPIO_BASE + JZGPIO_GROUP_OFFSET * n;

	writel(func & 0x8? pins : 0, base + PXINTS);
	writel(func & 0x4? pins : 0, base + PXMSKS);
	writel(func & 0x2? pins : 0, base + PXPAT1S);
	writel(func & 0x1? pins : 0, base + PXPAT0S);

	writel(func & 0x8? 0 : pins, base + PXINTC);
	writel(func & 0x4? 0 : pins, base + PXMSKC);
	writel(func & 0x2? 0 : pins, base + PXPAT1C);
	writel(func & 0x1? 0 : pins, base + PXPAT0C);

#ifdef CPM_EXCLK_DS
	if(func & 0x80) {
		/*controlled SD voltage to 1.8V*/
		int val = cpm_inl(CPM_EXCLK_DS) | (1 << 31);
		cpm_outl(val, CPM_EXCLK_DS);
	}
#endif
#if defined(CONFIG_X2600) || defined(CONFIG_AD100)
	if(n == GPIO_PORT_E){	//PE GROUP
		// hiz
		writel(func & 0x40? pins : 0, base + PEPUC);
		// pull up
		writel(func & 0x10? pins : 0, base + PEPUS);
		writel(func & 0x10? pins : 0, base + PEPSS);
		// pull down
		writel(func & 0x20? pins : 0, base + PEPUS);
		writel(func & 0x20? pins : 0, base + PEPSC);

		return;
	}
#endif
/* pull up */
#if defined(PXPES) && defined(PXPEC) && defined(PXPE)
 #if defined(CONFIG_X1000)
	writel(func & 0x10? pins : 0, base + PXPEC);
	writel(func & 0x10? 0 : pins, base + PXPES);
 #else
	writel(func & 0x10? pins : 0, base + PXPES);
	writel(func & 0x10? 0 : pins, base + PXPEC);
 #endif
#elif defined(PXPUS) && defined(PXPUC) && defined(PXPU)
	writel(func & 0x10? pins : 0, base + PXPUS);
	writel(func & 0x10? 0 : pins, base + PXPUC);
#elif defined(PXPEHS) && defined(PXPEHC) && defined(PXPE_PULLUP)
	if (func & 0x10)
		gpio_set_driver_state(n, pins, PXPE_PULLUP);
#endif

/* pull down */
#if defined(PXPDS) && defined(PXPDC) && defined(PXPD)
	writel(func & 0x20? pins : 0, base + PXPDS);
	writel(func & 0x20? 0 : pins, base + PXPDC);
#elif defined(PXPELS) && defined(PXPELC) && defined(PXPE_PULLDN)
	if (func & 0x20)
		gpio_set_driver_state(n, pins, PXPE_PULLDN);
#endif

/* pull hiz */
#if defined(PXPES) && defined(PXPEC) && defined(PXPE)
 #if defined(CONFIG_X1000)
	writel(func & 0x40? pins : 0, base + PXPES);
	writel(func & 0x40? 0 : pins, base + PXPEC);
 #else
	writel(func & 0x40? pins : 0, base + PXPEC);
	writel(func & 0x40? 0 : pins, base + PXPES);
 #endif
#elif defined(PXPDS) && defined(PXPDC) && defined(PXUS) && defined(PXUC)
	writel(func & 0x40? pins : 0, base + PXPUC);
	writel(func & 0x40? pins : 0, base + PXPDC);
#elif defined(PXPEHC) && defined(PXPELC) && defined(PXPE_PULLHZ)
	if (func & 0x40)
		gpio_set_driver_state(n, pins, PXPE_PULLHZ);
#endif
}

int gpio_request(unsigned gpio, const char *label)
{
	printf("%s lable = %s gpio = %d\n",__func__,label,gpio);
	return gpio;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

void gpio_port_set_value(int port, int pin, int value)
{
	if (value)
		writel(1 << pin, GPIO_PXPAT0S(port));
	else
		writel(1 << pin, GPIO_PXPAT0C(port));
}

void gpio_port_direction_input(int port, int pin)
{
	writel(1 << pin, GPIO_PXINTC(port));
	writel(1 << pin, GPIO_PXMSKS(port));
	writel(1 << pin, GPIO_PXPAT1S(port));
}

void gpio_port_direction_output(int port, int pin, int value)
{
	writel(1 << pin, GPIO_PXINTC(port));
	writel(1 << pin, GPIO_PXMSKS(port));

	gpio_port_set_value(port, pin, value);
	writel(1 << pin, GPIO_PXPAT1C(port));

}

int gpio_set_value(unsigned gpio, int value)
{
	int port = gpio / 32;
	int pin = gpio % 32;
	if(is_gpio_from_chip(gpio)) {
		gpio_port_set_value(port, pin, value);
	} else {
#ifdef CONFIG_JZ_PCA953X
	pca953x_set_value(gpio, value);
#endif
	}
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	unsigned port = gpio / 32;
	unsigned pin = gpio % 32;
	if(is_gpio_from_chip(gpio)) {
	return !!(readl(GPIO_PXPIN(port)) & (1 << pin));
	} else {
#ifdef CONFIG_JZ_PCA953X
	return pca953x_get_value(gpio);
#endif
	}
	return 0;
}

int gpio_get_flag(unsigned int gpio)
{
	unsigned port = gpio / 32;
	unsigned pin = gpio % 32;

	return (readl(GPIO_PXFLG(port)) & (1 << pin));
}

int gpio_clear_flag(unsigned gpio)
{
	int port = gpio / 32;
	int pin = gpio % 32;
	writel(1 << pin, GPIO_PXFLGC(port));
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	unsigned port = gpio / 32;
	unsigned pin = gpio % 32;
	if(is_gpio_from_chip(gpio)) {
		gpio_port_direction_input(port, pin);
	} else {
#ifdef CONFIG_JZ_PCA953X
	pca953x_direction_input(TO_PCA953X_GPIO(gpio));
#endif
	}

	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	unsigned port = gpio / 32;
	unsigned pin = gpio % 32;
	if(is_gpio_from_chip(gpio)) {
		gpio_port_direction_output(port, pin, value);
	} else {
#ifdef CONFIG_JZ_PCA953X
	pca953x_direction_output(TO_PCA953X_GPIO(gpio), value);
#endif
	}
	return 0;
}

void gpio_enable_pull(unsigned gpio)
{
	unsigned port= gpio / 32;
	unsigned pin = gpio % 32;
#if defined(PXPES) && defined(PXPEC) && defined(PXPE)
	writel(1 << pin, GPIO_PXPEC(port));
#elif defined(PXPUS) && defined(PXPUC) && defined(PXPU)
	writel(1 << pin, GPIO_PXPUC(port));
#elif defined(PXPEL) && defined(PXPEH) && defined(PXPE_PULLUP)
	gpio_set_driver_state(port, pin, PXPE_PULLUP);
#else
	printf("ERROR: %s nothing!\n",__func__);
#endif
}

void gpio_disable_pull(unsigned gpio)
{
	unsigned port= gpio / 32;
	unsigned pin = gpio % 32;
#if defined(PXPES) && defined(PXPEC) && defined(PXPE)
	writel(1 << pin, GPIO_PXPES(port));
#elif defined(PXPUS) && defined(PXPUC) && defined(PXPU)
        writel(1 << pin, GPIO_PXPUS(port));
#elif defined(PXPEL) && defined(PXPEH) && defined(PXPE_PULLUP)
	gpio_set_driver_state(port, pin, PXPE_PULLHZ);
#else
	printf("ERROR: %s nothing!\n",__func__);
#endif
}

void gpio_as_irq_high_level(unsigned gpio)
{
	unsigned port = gpio / 32;
	unsigned pin = gpio % 32;

	writel(1 << pin, GPIO_PXINTS(port));
	writel(1 << pin, GPIO_PXMSKC(port));
	writel(1 << pin, GPIO_PXPAT1C(port));
	writel(1 << pin, GPIO_PXPAT0S(port));
}

void gpio_as_irq_low_level(unsigned gpio)
{
	unsigned port = gpio / 32;
	unsigned pin = gpio % 32;

	writel(1 << pin, GPIO_PXINTS(port));
	writel(1 << pin, GPIO_PXMSKC(port));
	writel(1 << pin, GPIO_PXPAT1C(port));
	writel(1 << pin, GPIO_PXPAT0C(port));
}

void gpio_as_irq_rise_edge(unsigned gpio)
{
	unsigned port = gpio / 32;
	unsigned pin = gpio % 32;

	writel(1 << pin, GPIO_PXINTS(port));
	writel(1 << pin, GPIO_PXMSKC(port));
	writel(1 << pin, GPIO_PXPAT1S(port));
	writel(1 << pin, GPIO_PXPAT0S(port));
}

void gpio_as_irq_fall_edge(unsigned gpio)
{
	unsigned port = gpio / 32;
	unsigned pin = gpio % 32;

	writel(1 << pin, GPIO_PXINTS(port));
	writel(1 << pin, GPIO_PXMSKC(port));
	writel(1 << pin, GPIO_PXPAT1S(port));
	writel(1 << pin, GPIO_PXPAT0C(port));
}

void gpio_ack_irq(unsigned gpio)
{
	unsigned port = gpio / 32;
	unsigned pin = gpio % 32;

	writel(1 << pin, GPIO_PXFLGC(port));
}

#ifdef CONFIG_AD100
void gpio_set_driver_strength_ad100(enum gpio_port gpio, int value, unsigned int pins)
{
    if(value & BIT(0))
        writel(pins, GPIO_PXDS0S(gpio));
    else
        writel(pins, GPIO_PXDS0C(gpio));

    if(value & BIT(1))
        writel(pins, GPIO_PXDS1S(gpio));
    else
        writel(pins, GPIO_PXDS1C(gpio));
}
#endif

#ifdef CONFIG_X1600
void gpio_set_driver_strength_x1600(enum gpio_port gpio, int value, unsigned int pins)
{
	/* x1600 is not supported in setting driver strength */
}
#endif

#ifdef CONFIG_X2000_V12
void gpio_set_driver_strength_x2000(enum gpio_port gpio, int value, unsigned int pins)
{
    if(value & BIT(0))
        writel(pins, GPIO_PXDS0S(gpio));
    else
        writel(pins, GPIO_PXDS0C(gpio));

    if(value & BIT(1))
        writel(pins, GPIO_PXDS1S(gpio));
    else
        writel(pins, GPIO_PXDS1C(gpio));

    if(value & BIT(2))
        writel(pins, GPIO_PXDS2S(gpio));
    else
        writel(pins, GPIO_PXDS2C(gpio));
}
#endif

#ifdef CONFIG_X2580
void gpio_set_driver_strength_x2580(enum gpio_port gpio, int value, unsigned int pins)
{
    if(value & BIT(0))
        writel(pins, GPIO_PXPDS0S(gpio));
    else
        writel(pins, GPIO_PXPDS0C(gpio));

    if(value & BIT(1))
        writel(pins, GPIO_PXPDS1S(gpio));
    else
        writel(pins, GPIO_PXPDS1C(gpio));

    if(value & BIT(2))
        writel(pins, GPIO_PXPDS2S(gpio));
    else
        writel(pins, GPIO_PXPDS2C(gpio));

    if(value & BIT(3))
        writel(pins, GPIO_PXPDS3S(gpio));
    else
        writel(pins, GPIO_PXPDS3C(gpio));
}
#endif

#ifdef CONFIG_X2600
void gpio_set_driver_strength_x2600(enum gpio_port gpio, int value, unsigned int pins)
{
    if(value & BIT(0))
        writel(pins, GPIO_PXDS0S(gpio));
    else
        writel(pins, GPIO_PXDS0C(gpio));

    if(value & BIT(1))
        writel(pins, GPIO_PXDS1S(gpio));
    else
        writel(pins, GPIO_PXDS1C(gpio));
}
#endif

void gpio_set_driver_strength(enum gpio_port gpio, int value, unsigned int pins)
{
#ifdef CONFIG_AD100
	gpio_set_driver_strength_ad100(gpio, value, pins);
#elif defined CONFIG_X1600
	gpio_set_driver_strength_x1600(gpio, value, pins);
#elif defined CONFIG_X2000_V12
	gpio_set_driver_strength_x2000(gpio, value, pins);
#elif defined CONFIG_X2580
	gpio_set_driver_strength_x2580(gpio, value, pins);
#elif defined CONFIG_X2600
	gpio_set_driver_strength_x2600(gpio, value, pins);
#endif
}

void dump_gpio_func( unsigned int gpio);
void gpio_init(void)
{
	int i, n;
	struct jz_gpio_func_def *g;
#ifndef CONFIG_BURNER
	n = ARRAY_SIZE(gpio_func);

	for (i = 0; i < n; i++) {
		g = &gpio_func[i];
		gpio_set_func(g->port, g->func, g->pins);
	}
	g = &uart_gpio_func[CONFIG_SYS_UART_INDEX];
#else
	n = gd->arch.gi->nr_gpio_func;

	for (i = 0; i < n; i++) {
		g = &gd->arch.gi->gpio[i];
		gpio_set_func(g->port, g->func, g->pins);
	}
//	g = &uart_gpio_func[gd->arch.gi->uart_idx];
	g = &gd->arch.gi->uart_gpio[0];
#endif
	gpio_set_func(g->port, g->func, g->pins);

#ifndef CONFIG_SPL_BUILD
#ifdef CONFIG_JZ_PCA953X
	pca953x_init();
#endif
#endif
}

void dump_gpio_func( unsigned int gpio)
{
	unsigned group = gpio / 32;
	unsigned pin = gpio % 32;
	int d = 0;
	unsigned int base = GPIO_BASE + JZGPIO_GROUP_OFFSET * group;
	d = d | ((readl(base + PXINT) >> pin) & 1) << 3;
	d = d | ((readl(base + PXMSK) >> pin) & 1) << 2;
	d = d | ((readl(base + PXPAT1) >> pin) & 1) << 1;
	d = d | ((readl(base + PXPAT0) >> pin) & 1) << 0;
    printf("gpio[%d] fun %x\n",gpio,d);
}
