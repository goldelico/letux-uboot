#include <common.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include "../../ti/beagle/beagle.h"
#include "beagleb2.h"

// make us initialize using both pinmux sets

void muxinit(void)
{
	MUX_BEAGLE();
	MUX_BEAGLE_EXPANDER();
}

#undef MUX_BEAGLE
#define MUX_BEAGLE() muxinit()

// take the original beagle.c code
#include "../../ti/beagle/beagle.c"
