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

int misc_init_r(void)
{
	my_misc_init_r();		// initializes board revision dependent mux (e.g. MUX_BEAGLE_C())
	MUX_BEAGLE_EXPANDER();	// append our own pinmux
	return 0;
}

#define misc_init_r my_misc_init_r

// take the original beagle.c code
#include "../../ti/beagle/beagle.c"
