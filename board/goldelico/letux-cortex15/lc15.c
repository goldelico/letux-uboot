/*
 * basically the same as the OMAP5432EVM
 */

/* move away definition by included file */
#ifndef sysinfo
#define sysinfo sysinfo_disabled

#include "../../ti/omap5_uevm/evm.c"

#undef sysinfo

const struct omap_sysinfo sysinfo = {
	"Board: Letux Cortex 15\n"
};

#else

#include "../../ti/omap5_uevm/evm.c"

#endif

// add eMMC/uSD switch logic here
// so that we can boot from the internal uSD
// and release the boot button
// (maybe do it in a way that it is only relevant for MLO)
