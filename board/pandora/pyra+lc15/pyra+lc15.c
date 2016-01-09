/*
 * basically the same as the Letux Cortex 15
 * except that we change the UART3 pinmux because it is wired up
 * differently
 */

/* move away definition by included file */
#define spl_start_uboot spl_start_uboot_overwritten
#define set_muxconf_regs_essential set_muxconf_regs_essential_disabled
#define sysinfo sysinfo_disabled

#include "../../goldelico/letux-cortex15/lc15.c"

#undef sysinfo
#undef set_muxconf_regs_essential
#undef spl_start_uboot

const struct omap_sysinfo sysinfo = {
	"Board: Pyra with Letux Cortex 15\n"
};

const struct pad_conf_entry core_padconf_array_essential_pyra[] = {

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
//	{UART3_RX_IRRX, (PTU | IEN | M0)}, /*  UART3_RX_IRRX    */
//	{UART3_TX_IRTX, (M0)},    /*  UART3_TX_IRTX    */
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
	// switch UART3 to different pins
	{UART3_RX_IRRX, (PTU | M4)}, /*  SDIO4_CLK */
	{UART3_TX_IRTX, (PTU | M4)},  /*  SDIO4_CMD */
	{RFBI_DATA1, (PTU | IEN | M4)},  /*  UART3_RX_IRRX */
	{RFBI_DATA2, (M4)}, /*  UART3_TX_IRTX */
	// enable I2C1..5 pullups
	{I2C1_SCL, (PTU | IEN | M0)}, /* I2C1_SCL */
	{I2C1_SDA, (PTU | IEN | M0)}, /* I2C1_SDA */
	{I2C2_SCL, (PTU | IEN | M0)}, /* I2C2_SCL */
	{I2C2_SDA, (PTU | IEN | M0)}, /* I2C2_SDA */
	{I2C3_SCL, (PTU | IEN | M0)}, /* I2C3_SCL */
	{I2C3_SDA, (PTU | IEN | M0)}, /* I2C3_SDA */
	{I2C4_SCL, (PTU | IEN | M0)}, /* I2C4_SCL */
	{I2C4_SDA, (PTU | IEN | M0)}, /* I2C4_SDA */
};

void set_muxconf_regs_essential(void)
{
	do_set_mux((*ctrl)->control_padconf_core_base,
		   core_padconf_array_essential_pyra,
		   sizeof(core_padconf_array_essential_pyra) /
		   sizeof(struct pad_conf_entry));

	do_set_mux((*ctrl)->control_padconf_wkup_base,
		   wkup_padconf_array_essential,
		   sizeof(wkup_padconf_array_essential) /
		   sizeof(struct pad_conf_entry));
}

/* SPL only code */
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_OS_BOOT)

// FIXME: we should write to i2c2 and not to i2c1!
// FIXME: add a driver?

/* I2C chip addresses, bq24297 */
#define BQ24297_ADDR	0x6b

static inline int bq24297_i2c_write_u8(u8 reg, u8 val)
{
	printf("bq24297 %02x[%02x] := %02x\n", BQ24297_ADDR, reg, val);
	return i2c_write(BQ24297_ADDR, reg, 1, &val, 1);
}

static inline int bq24297_i2c_read_u8(u8 reg, u8 *val)
{
	return i2c_read(BQ24297_ADDR, reg, 1, val, 1);
}

int bq2429x_set_ilim(int ilim)
{
	u8 reg;
	printf("bq2429x_set_ilim(%d) - set input current limit\n", ilim);

	bq24297_i2c_read_u8(0x00, &reg);
	/* bit 0..2 are IINLIM */
	printf("p  r0=%02x\n", reg);
	bq24297_i2c_read_u8(0x09, &reg);
	/* fault/status - can we decide battery presence? e.g. NTC fault? */
	printf("p  r0=%02x\n", reg);
	/* we should set IINLIM = 0x5 (1.5 A) if no battery */
	/* or IINLIM = 0x0 (100 mA) if battery */
	return 0;
}

int spl_start_uboot(void)
{
	printf("spl_start_uboot for Pyra+LC15 called\n");
	bq2429x_set_ilim(1000);	/* set bq24297 current to 1.0 A */
	return spl_start_uboot_overwritten();	/* do everything inherited from LC15 board */
}

#endif

// TODO: we can (re)enable tca6424 code (because we do have it on this board)
