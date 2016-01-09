/*
 * basically the same as the OMAP5432EVM
 */

/* move away definition by included file */
#define board_mmc_init board_mmc_init_overwritten
#if !defined(sysinfo)
#define sysinfo sysinfo_disabled

#include "../../ti/omap5_uevm/evm.c"

#undef sysinfo

const struct omap_sysinfo sysinfo = {
	"Board: Letux Cortex 15\n"
};

#else

#include "../../ti/omap5_uevm/evm.c"

#endif
#undef board_mmc_init

/* U-Boot only code */
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_GENERIC_MMC)

/*
 * the Letux Cortex 15
 * supports 3 MMC interfaces
 */

#include <mmc.h>

int board_mmc_init(bd_t *bis)
{
	int uSD = 0;	// ask GPIO3_82 value
	debug("special board_mmc_init for LC15\n");
	omap_mmc_init(0, uSD?MMC_MODE_4BIT:0, 0, -1, -1);
	omap_mmc_init(1, MMC_MODE_4BIT, 0, -1, -1);
/* FIXME: we might need to modify the HSMMC3_BASE in omap_hsmmc.c to make this work */
	omap_mmc_init(2, MMC_MODE_4BIT, 0, -1, -1);
	return 0;
}

#endif

/* SPL only code */
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_OS_BOOT)

/*
 * add eMMC/uSD switch logic here
 * so that we can boot from the internal uSD
 * and release the boot button.
 * Should be done only in SPL!
 */

int set_mmc_switch(void)
{
	printf("set_mmc_switch for LC15 called\n");
// maybe check if GPIO3_82 is already output -> ignore
/*
	read GPIO1_WK7
	set GPIO3_82 from high Z to output mode
	set GPIO3_82 value to !GPIO1_WK7 value - i.e. make initial button setting persist
*/
	return 0;
}

/* mis-use as a hook to inject code before U-Boot is started */

int spl_start_uboot(void)
{
	printf("spl_start_uboot for LC15 called\n");
	set_mmc_switch();
	return 1;	/* no direct Linux boot */
}

#endif

