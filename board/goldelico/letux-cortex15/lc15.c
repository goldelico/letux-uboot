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
	writel(0x02, 0x4A009120);	/* enable MMC3 module */
	writel(0x02, 0x4A009128);	/* enable MMC4 module */

	/* MMC1 = left SD */
	omap_mmc_init(0, uSD?MMC_MODE_8BIT:0, 0, -1, -1);
	/* MMC2 = eMMC (8 bit) / uSD (4 bit) */
	omap_mmc_init(1, 0, 0, -1, -1);
#if 0
	/* MMC3 = WLAN */
	omap_mmc_init(2, MMC_MODE_8BIT|MMC_MODE_4BIT, 0, -1, -1);
#endif
	/* SDIO4 = right SD */
	omap_mmc_init(3, 0, 0, -1, -1);

//	printf("%08x: %08x\n", 0x4A009120, readl(0x4A009120));
//	printf("%08x: %08x\n", 0x4A009128, readl(0x4A009128));
	return 0;
}

#endif

/*
 * Board Revision Detection
 *
 * gpio2_32 and gpio2_33 can optionally be pulled up or down
 * by 10k resistors. These are stronger than the internal
 * pull-up or pull-down resistors of the omap5 pads.
 * by trying to pull them up/down and check who wins, we
 * can find out which resistors are installed.
 * If no resistor is installed, the gpio value follows the
 * omap5 pull-up or -down.
 * Which resistors are installed changes from board revision
 * to board revision (see schematics).
 */

const struct pad_conf_entry padconf_version_pd_lc15[] = {
	{LLIB_WAKEREQOUT, (IEN | PTD | M6)}, /* gpio 2_32 */
	{C2C_CLKOUT0, (IEN | PTD | M6)}, /* gpio 2_33 */
};

const struct pad_conf_entry padconf_version_pu_lc15[] = {
	{LLIB_WAKEREQOUT, (IEN | PTU | M6)}, /* gpio 2_32 */
	{C2C_CLKOUT0, (IEN | PTU | M6)}, /* gpio 2_33 */
};

/* operational mode */
const struct pad_conf_entry padconf_version_operation_lc15[] = {
	{LLIB_WAKEREQOUT, (IEN | M6)}, /* gpio 2_32 */
	{C2C_CLKOUT0, (IEN | M6)}, /* gpio 2_33 */
};

const int versions[]={
	[0xc] = 49,	/* no resistors */
	[0xd] = 50,	/* gpio2_32 pu, gpio2_33 floating */
	[0x8] = 51,	/* gpio2_32 pd, gpio2_33 floating */
	[0xe] = 0,	/* gpio2_32 floating, gpio2_33 pu */
	[0x4] = 0,	/* gpio2_32 floating, gpio2_33 pd */
	[0x0] = 0,	/* gpio2_32 pd, gpio2_33 pd */
	[0x5] = 0,	/* gpio2_32 pu, gpio2_33 pd */
	[0xa] = 0,	/* gpio2_32 pd, gpio2_33 pu */
	[0xf] = 0,	/* gpio2_32 pu, gpio2_33 pu */
};

int get_board_version(void)
{ /* read get board version from resistors */
	static int vers;
	if (!vers) {
		gpio_request(32, "version-0");	/* version resistors */
		gpio_request(33, "version-1");
		do_set_mux((*ctrl)->control_padconf_core_base,
			   padconf_version_pd_lc15,
			   sizeof(padconf_version_pd_lc15) /
			   sizeof(struct pad_conf_entry));
		vers = gpio_get_value(32) | (gpio_get_value(33) << 1);
		do_set_mux((*ctrl)->control_padconf_core_base,
			   padconf_version_pu_lc15,
			   sizeof(padconf_version_pu_lc15) /
			   sizeof(struct pad_conf_entry));
		vers |= (gpio_get_value(32) << 2) | (gpio_get_value(33) << 3);
		gpio_free(32);
		gpio_free(33);
		do_set_mux((*ctrl)->control_padconf_core_base,
			   padconf_version_operation_lc15,
			   sizeof(padconf_version_operation_lc15) /
			   sizeof(struct pad_conf_entry));
#if 1
		printf("version code 0x%01x\n", vers);
#endif
		vers = versions[vers&0xf];
		printf("LC15 V%d.%d\n", vers/10, vers%10);
	}
	return vers;
}

/* SPL only code */
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_OS_BOOT)

/*
 * add eMMC/uSD switch logic here
 * so that we can boot from the internal uSD
 * and release/press the boot button without
 * throwing the eMMC/uSD switch.
 */

const struct pad_conf_entry wkupconf_mmcmux_lc15[] = {
	{DRM_EMU1, (IEN | PTU | M6)}, /* gpio 1_wk7 */
};

const struct pad_conf_entry padconf_mmcmux_lc15[] = {
	{HSI2_ACFLAG, (IEN | M6)}, /* gpio 3_82 */
	{HSI2_CAREADY, (IEN | M6)}, /* gpio 3_76 */
};

/* board revision 5.0 uses different gpios */

const struct pad_conf_entry padconf_mmcmux_lc15_50[] = {
};

int set_mmc_switch(void)
{
	int val;
	int vers = get_board_version();
	int hard_select = 7;	/* gpio to select uSD or eMMC by external hw signal */
	int soft_select = 86;	/* gpio to select uSD or eMMC */
	int control = 76;	/* control between SW and HW select */
#if 0
	printf("set_mmc_switch for LC15 called\n");
#endif
	if (vers <= 49)
		return 1;
	if (vers <= 50) {
		/* board revision 5.0 shares the revision gpios with mmc_switch control */
		soft_select = 32;
		control = 33;
	}

	gpio_request(hard_select, "bootsel");		/* BOOTSEL button */
	gpio_request(soft_select, "soft-select");	/* choose uSD and not eMMC */
	gpio_request(control, "mmc-control");	/* MMC switch control */

	/* make gpio1_wk7 an input so that we can read the state of the BOOTSEL button */
	do_set_mux((*ctrl)->control_padconf_wkup_base,
		   wkupconf_mmcmux_lc15,
		   sizeof(wkupconf_mmcmux_lc15) /
		   sizeof(struct pad_conf_entry));

	/* is BOOTSEL active? Then we did boot SPL from µSD */
	val = gpio_get_value(hard_select);

#if 1
	printk("  gpio%d = %s\n", hard_select, val?"eMMC":"uSD");
#endif

	/* pass hard-select to soft-select so that the user can release/press the button */
	gpio_direction_output(soft_select, !val);
	/* switch to soft control */
	gpio_direction_output(control, 1);

	/* make the control gpios active outputs */
	/* note: never make gpio3_82 an output on V5.0 boards */
	if (vers > 50)
		do_set_mux((*ctrl)->control_padconf_core_base,
			   padconf_mmcmux_lc15,
			   sizeof(padconf_mmcmux_lc15) /
			   sizeof(struct pad_conf_entry));

#if 1
	/* read back */
	printk("  gpio%d = %d (mmc1=%s)\n", soft_select, gpio_get_value(soft_select), gpio_get_value(soft_select)?"uSD":"eMMC");
	printk("  gpio%d = %d (ctrl=%s)\n", control, gpio_get_value(control), gpio_get_value(control)?"soft":"hard");
#endif
	gpio_free(hard_select);
	gpio_free(soft_select);
	gpio_free(control);
	return 0;
}

/* mis-use as a hook to inject code before U-Boot is started */

int spl_start_uboot(void)
{
	set_mmc_switch();
	return 1;	/* no boot to Linux */
}

#endif

