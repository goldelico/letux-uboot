/*
 * blink leds connected to GPIOs
 * and poll buttons
 */

#include <common.h>
#include <command.h>
#include <spi.h>
#include <video_fb.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include <twl4030.h>
#include "led.h"

// Note: on GTA04 the LEDs will be connected to GPIOs of TPS65950
#define GPIO_LED_AUX_RED		70		// AUX
#define GPIO_LED_AUX_GREEN		71		// AUX
#define GPIO_LED_POWER_RED		78		// Power
#define GPIO_LED_POWER_GREEN	79		// Power

// Note: on GTA04 the GPIOs will be different and the state of the POWER button is only available through the TPS65950

#define GPIO_AUX		136
#define GPIO_POWER		137
#define GPIO_GPSEXT		138		// external GPS antenna plugged in
#define GPIO_PENIRQ		157		// TSC must be set up to provide PENIRQ

void led_set_led(int value)
{
	omap_set_gpio_dataout(GPIO_LED_AUX_RED, (value&(1 << 0)));
	omap_set_gpio_dataout(GPIO_LED_AUX_GREEN, (value&(1 << 1)));
	omap_set_gpio_dataout(GPIO_LED_POWER_RED, (value&(1 << 2)));
	omap_set_gpio_dataout(GPIO_LED_POWER_GREEN, (value&(1 << 3)));
}

int led_get_buttons(void)
{
	return
		((!omap_get_gpio_datain(GPIO_AUX)) << 0) |
		((omap_get_gpio_datain(GPIO_GPSEXT)) << 1) |
		((!omap_get_gpio_datain(GPIO_POWER)) << 2) |
		((omap_get_gpio_datain(GPIO_PENIRQ)) << 3);
}

int led_init(void)
{
	MUX_VAL(CP(DSS_DATA0),		(IEN | PTD | EN | M4)); /*GPIO */
	MUX_VAL(CP(DSS_DATA1),		(IEN | PTD | EN | M4)); /*GPIO */
	MUX_VAL(CP(DSS_DATA8),		(IEN | PTD | EN | M4)); /*GPIO */
	MUX_VAL(CP(DSS_DATA9),		(IEN | PTD | EN | M4)); /*GPIO */
	
	omap_request_gpio(GPIO_LED_AUX_GREEN);
	omap_request_gpio(GPIO_LED_AUX_RED);
	omap_request_gpio(GPIO_LED_POWER_GREEN);
	omap_request_gpio(GPIO_LED_POWER_RED);
	omap_request_gpio(GPIO_AUX);
	omap_request_gpio(GPIO_POWER);
	omap_request_gpio(GPIO_GPSEXT);
	omap_request_gpio(GPIO_PENIRQ);
	
	omap_set_gpio_direction(GPIO_LED_AUX_GREEN, 0);		// output
	omap_set_gpio_direction(GPIO_LED_AUX_RED, 0);		// output
	omap_set_gpio_direction(GPIO_LED_POWER_GREEN, 0);		// output
	omap_set_gpio_direction(GPIO_LED_POWER_RED, 0);		// output

	omap_set_gpio_direction(GPIO_AUX, 1);		// input
	omap_set_gpio_direction(GPIO_POWER, 1);		// input
	omap_set_gpio_direction(GPIO_AUX, 1);		// input
	omap_set_gpio_direction(GPIO_GPSEXT, 1);		// input

	// when sould we do omap_free_gpio(GPIO_LED_AUX_GREEN);
	printf("did led_init()\n");

	return 0;
}

static int do_led_init(int argc, char *argv[])
{
	led_init();
	return 0;
}

static void print_buttons(int status)
{
	printf("AUX: %s Power: %s Antenna: %s Pen: %s", (status&0x01)?"on":"off", (status&0x04)?"on":"off", (status&0x02)?"EXT":"INT", (status&0x08)?"1":"0");
}

static int do_led_get(int argc, char *argv[])
{
	int status=led_get_buttons();
	printf("button status: %01x\n", status);
	print_buttons(status);
	printf("\n");
	return 0;
}

static int do_led_set(int argc, char *argv[])
{ // led set hh
	static int state;
	if(argc == 2)
		state++;
	else
		state=simple_strtoul(argv[2], NULL, 16);
	led_set_led(state);
	return 0;
}

static int do_led_loop(int argc, char *argv[])
{
	printf("mirroring buttons to LEDs.\n"
		   "Press any key to stop\n\n");
	while (!tstc())
		{
			int state=led_get_buttons();
			print_buttons(state);
			printf("\r");
			led_set_led(state);	// mirror to LEDs
			udelay(100000);	// 0.1 seconds
		}
	getc();
	printf("\n");
	return 0;
}

static int do_led_blink(int argc, char *argv[])
{
	int value=0;
	printf("blinking LEDs.\n"
		   "Press any key to stop\n\n");
	while (!tstc())
		{
			led_set_led(value++);	// mirror to LEDs
			udelay(500000);	// 0.5 seconds
		}
	getc();
	return 0;
}

static int do_led(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int len;
	
	if (argc < 2) {
		printf ("led: missing subcommand.\n");
		return (-1);
	}
	
	len = strlen (argv[1]);
	if (strncmp ("ge", argv[1], 2) == 0) {
		return do_led_get (argc, argv);
	} else if (strncmp ("se", argv[1], 2) == 0) {
		return do_led_set (argc, argv);
	} else if (strncmp ("mi", argv[1], 2) == 0) {
		return do_led_loop (argc, argv);
	} else if (strncmp ("bl", argv[1], 2) == 0) {
		return do_led_blink (argc, argv);
	} else if (strncmp ("in", argv[1], 2) == 0) {
		return do_led_init (argc, argv);
	} else {
		printf ("led: unknown operation: %s\n", argv[1]);
	}
	
	return (0);
}


U_BOOT_CMD(led, 3, 0, do_led, "LED and Buttons sub-system",
		   "init - initialize GPIOs\n"
		   "get - read button status\n"
		   "set value - set LEDs state\n"
		   "mirror - read buttons and mirror to LEDs\n"
		   "blink - blink LEDs\n"
		   );
