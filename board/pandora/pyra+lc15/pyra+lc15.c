/*
 * basically the same as the Letux Cortex 15
 * except that we change the UART3 pinmux because it is wired up
 * differently
 */

/* move away definition by included file */
#define set_muxconf_regs_essential set_muxconf_regs_essential_disabled
#define sysinfo sysinfo_disabled

#include "../../goldelico/letux-cortex15/lc15.c"

#undef sysinfo
#undef set_muxconf_regs_essential

const struct omap_sysinfo sysinfo = {
	"Board: Pyra with Letux Cortex 15\n"
};

const struct pad_conf_entry core_padconf_array_essential_alternate[] = {

	{EMMC_CLK, (PTU | IEN | M0)}, /*  EMMC_CLK   */
	{EMMC_CMD, (PTU | IEN | M0)}, /*  EMMC_CMD   */
	{EMMC_DATA0, (PTU | IEN | M0)}, /*  EMMC_DATA0 */
	{EMMC_DATA1, (PTU | IEN | M0)}, /*  EMMC_DATA1 */
	{EMMC_DATA2, (PTU | IEN | M0)}, /*  EMMC_DATA2 */
	{EMMC_DATA3, (PTU | IEN | M0)}, /*  EMMC_DATA3 */
	{EMMC_DATA4, (PTU | IEN | M0)}, /*  EMMC_DATA4 */
	{EMMC_DATA5, (PTU | IEN | M0)}, /*  EMMC_DATA5 */
	{EMMC_DATA6, (PTU | IEN | M0)}, /*  EMMC_DATA6 */
	{EMMC_DATA7, (PTU | IEN | M0)}, /*  EMMC_DATA7 */
	{SDCARD_CLK, (PTU | IEN | M0)}, /*  SDCARD_CLK  */
	{SDCARD_CMD, (PTU | IEN | M0)}, /*  SDCARD_CMD  */
	{SDCARD_DATA0, (PTU | IEN | M0)}, /*  SDCARD_DATA0*/
	{SDCARD_DATA1, (PTU | IEN | M0)}, /*  SDCARD_DATA1*/
	{SDCARD_DATA2, (PTU | IEN | M0)}, /*  SDCARD_DATA2*/
	{SDCARD_DATA3, (PTU | IEN | M0)}, /*  SDCARD_DATA3*/
	{UART3_RX_IRRX, (PTU | M4)}, /*  SDIO4_CLK */
	{UART3_TX_IRTX, (PTU | M4)},  /*  SDIO4_CMD */
	{RFBI_DATA1, (PTU | IEN | M4)},  /*  UART3_RX_IRRX */
	{RFBI_DATA2, (M4)}, /*  UART3_TX_IRTX */
	{USBB1_HSIC_STROBE, (PTU | IEN | M0)},    /*  USBB1_HSIC_STROBE */
	{USBB1_HSIC_DATA, (PTU | IEN | M0)},    /*  USBB1_HSIC_DATA */
	{USBB2_HSIC_STROBE, (PTU | IEN | M0)},    /*  USBB2_HSIC_STROBE */
	{USBB2_HSIC_DATA, (PTU | IEN | M0)},    /*  USBB2_HSIC_DATA  */
	{USBB3_HSIC_STROBE, (PTU | IEN | M0)},    /*  USBB3_HSIC_STROBE*/
	{USBB3_HSIC_DATA, (PTU | IEN | M0)},    /*  USBB3_HSIC_DATA  */
	{USBD0_HS_DP, (IEN | M0)},	/*  USBD0_HS_DP */
	{USBD0_HS_DM, (IEN | M0)},	/*  USBD0_HS_DM */
	{USBD0_SS_RX, (IEN | M0)},	/*  USBD0_SS_RX */
	{I2C5_SCL, (IEN | M0)}, /* I2C5_SCL */
	{I2C5_SDA, (IEN | M0)}, /* I2C5_SDA */
	{HSI2_ACWAKE, (PTU | M6)},    /*  HSI2_ACWAKE */
	{HSI2_CAFLAG, (PTU | M6)},    /*  HSI2_CAFLAG */
};

void set_muxconf_regs_essential(void)
{
	do_set_mux((*ctrl)->control_padconf_core_base,
		   core_padconf_array_essential_alternate,
		   sizeof(core_padconf_array_essential_alternate) /
		   sizeof(struct pad_conf_entry));

	do_set_mux((*ctrl)->control_padconf_wkup_base,
		   wkup_padconf_array_essential,
		   sizeof(wkup_padconf_array_essential) /
		   sizeof(struct pad_conf_entry));
}



// (re)enable tca6424 (because we do have it on this board)