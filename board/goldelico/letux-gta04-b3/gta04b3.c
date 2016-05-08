#include <common.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include "../gta04/gta04.h"
#include "gta04b3.h"

// make us initialize using both pinmux sets

void muxinit(void)
{
	MUX_GTA04();
	MUX_EXPANDER_B3();
}

#undef MUX_GTA04
#define MUX_GTA04() muxinit()

// take the original beagle.c code
#include "../gta04/gta04.c"
