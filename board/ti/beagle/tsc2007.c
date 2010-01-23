/*
 * read values from TSC2007 touch screen controller connected to I2C2
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
#include "tsc2007.h"

#define TSC2007 0x48

int tsc2007_init(void)
{
	unsigned char buf[16];
	uint addr=0;
	if(i2c_set_bus_num(2-1))	// I2C2
		{
			printf ("could not select I2C2\n");
			return -1;
		}
	
	if (i2c_read(TSC2007, addr, 1, buf, sizeof(buf)) != 0)
		{
			printf ("Error reading the TSC.\n");
			return -1;
		}

	// initialize
	
	printf("did tsc2007_init()\n");

	return 0;
}

static int read_adc(int adcnum)
{
	// read value from given ADC
	return adcnum;
}

static void print_adc(void)
{
	printf("0: %03d 1:%03d 2:%03d 3:%03d 4: %03d 5:%03d 6:%03d 7:%03d",
		   read_adc(0),
		   read_adc(1),
		   read_adc(2),
		   read_adc(3),
		   read_adc(4),
		   read_adc(5),
		   read_adc(6),
		   read_adc(7));
}

static int do_tsc_init(int argc, char *argv[])
{
	tsc2007_init();
	return 0;
}

static int do_tsc_get(int argc, char *argv[])
{
	print_adc();
	printf("\n");
	return 0;
}

static int do_tsc_loop(int argc, char *argv[])
{
	printf("permanently reading ADCs of TSC.\n"
		   "Press any key to stop\n\n");
	while (!tstc())
		{
			print_adc();
			printf("\r");
		}
	getc();
	printf("\n");
	return 0;
}

static int do_tsc_choose(int argc, char *argv[])
{
	int value=0;
	printf("choosing by waiting for touch.\n"
		   "Press any key to stop\n\n");
	while (!tstc())
		{
			int x=read_adc(0);
			int y=read_adc(1);
			// check if pressed - then print receptive field (e.g. 1 of 8 or 16) and return 0
			printf("did choose %d/%d\n", x, y);
			return 0;
		}
	getc();
	return 0;
}

static int do_tsc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int len;
	
	if (argc < 2) {
		printf ("led: missing subcommand.\n");
		return (-1);
	}
	
	len = strlen (argv[1]);
	if (strncmp ("ge", argv[1], 2) == 0) {
		return do_tsc_get (argc, argv);
	} else if (strncmp ("lo", argv[1], 2) == 0) {
		return do_tsc_loop (argc, argv);
	} else if (strncmp ("ch", argv[1], 2) == 0) {
		return do_tsc_choose (argc, argv);
	} else if (strncmp ("in", argv[1], 2) == 0) {
		return do_tsc_init (argc, argv);
	} else {
		printf ("tsc: unknown operation: %s\n", argv[1]);
	}
	
	return (0);
}


U_BOOT_CMD(tsc, 3, 0, do_tsc, "TSC2007 sub-system",
		   "in[it] - initialize TSC2007\n"
		   "ge[t] - read ADCs\n"
		   "lo[op] - loop and display x/y coordinates\n"
		   "ch[oose] - chosse item\n"
		   );
