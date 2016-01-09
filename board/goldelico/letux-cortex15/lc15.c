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
	int uSD = 0;	// could ask GPIO3_82 state
	debug("special board_mmc_init for LC15\n");
	omap_mmc_init(0, uSD?MMC_MODE_4BIT:0, 0, -1, -1);
	omap_mmc_init(1, MMC_MODE_4BIT, 0, -1, -1);
/* FIXME: we might need to modify the HSMMC3_BASE in omap_hsmmc.c to make this work */
/* here we need an interface called SDIO4 with controller base address 480d1000 */
	omap_mmc_init(2, MMC_MODE_4BIT, 0, -1, -1);
	return 0;
}

#endif

/* SPL only code */
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_OS_BOOT)

/*
 * add eMMC/uSD switch logic here
 * so that we can boot from the internal uSD
 * and release/press the boot button without
 * throwing the eMMC/uSD switch.
 */

const struct pad_conf_entry wkupconf_mmcmux_pyra[] = {
	{DRM_EMU0, (IEN | M6)}, /* gpio 1_wk7 */
};

const struct pad_conf_entry padconf_mmcmux_pyra[] = {
	{HSI2_ACFLAG, (IDIS | M6)}, /* gpio 3_82 */
};

int set_mmc_switch(void)
{
	int val;
	printf("set_mmc_switch for LC15 called\n");
	/* make gpio1_wk7 an input */
	do_set_mux((*ctrl)->control_padconf_wkup_base,
		   wkupconf_mmcmux_pyra,
		   sizeof(wkupconf_mmcmux_pyra) /
		   sizeof(struct pad_conf_entry));
	gpio_request(7, "gpio1_wk7");	/* BOOTSEL button */
	gpio_request(82, "gpio3_82");	/* MMC switch control */
	val = gpio_get_value(7);	/* BOOTSEL pressed? */
	printk("  gpio7 = %d\n", val);
	gpio_direction_output(82, !val);	/* pass button setting to output */
	/* go from High-Z to low L to make it really an output
	 * overriding the hardware defined state from the BOOTSEL button
	 */
	do_set_mux((*ctrl)->control_padconf_core_base,
		   padconf_mmcmux_pyra,
		   sizeof(padconf_mmcmux_pyra) /
		   sizeof(struct pad_conf_entry));
	/* read back */
	printk("  gpio82 = %d\n", gpio_get_value(82));
	gpio_free(7);
	gpio_free(82);
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

