/*
 * basically the same as the Letux Cortex 15
 * except that we change the UART3 pinmux because it is wired up
 * differently
 */

/* move away definition by included file */
#define board_init board_init_overwritten
#define set_muxconf_regs_essential set_muxconf_regs_essential_disabled
#define sysinfo sysinfo_disabled

#include "../../goldelico/letux-cortex15/lc15.c"

#undef sysinfo
#undef set_muxconf_regs_essential
#undef board_init

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
	{I2C1_PMIC_SCL, (PTU | IEN | M0)}, /* I2C1_SCL */
	{I2C1_PMIC_SDA, (PTU | IEN | M0)}, /* I2C1_SDA */
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

/* U-Boot only code */
#if !defined(CONFIG_SPL_BUILD)

// FIXME: we should write to i2c2 and not to i2c1!
// FIXME: add a driver?

/* I2C chip addresses, bq24297 */
#define BQ24297_BUS	1	/* I2C2 */
#define BQ24297_ADDR	0x6b

static inline int bq24297_i2c_write_u8(u8 reg, u8 val)
{
	int org_bus_num;
	int ret;

	org_bus_num = i2c_get_bus_num();
	i2c_set_bus_num(BQ24297_BUS);	/* select I2C2 */

	ret = i2c_write(BQ24297_ADDR, reg, 1, &val, 1);

	i2c_set_bus_num(org_bus_num);
	return ret;
}

static inline int bq24297_i2c_read_u8(u8 reg, u8 *val)
{
	int org_bus_num;
	int ret;

	org_bus_num = i2c_get_bus_num();
	i2c_set_bus_num(BQ24297_BUS);	/* select I2C2 */
	ret = i2c_read(BQ24297_ADDR, reg, 1, val, 1);

	i2c_set_bus_num(org_bus_num);
	return ret;
}

int bq2429x_battery_present(void)
{
	u8 reg;

	if (bq24297_i2c_read_u8(0x09, &reg)) {
		printf("no response from bq24297\n");
		return 0;
	}

	printf("  r9=%02x\n", reg);

	return !(reg & 0x03);	/* no NTC fault - assume battery is inserted */
}

/* from our Linux kernel driver */
static int bq24296_get_limit_current(int value)
{
	u8 data;
	if (value < 120)
		data = 0;
	else if(value < 400)
		data = 1;
	else if(value < 700)
		data = 2;
	else if(value < 1000)
		data = 3;
	else if(value < 1200)
		data = 4;
	else if(value < 1800)
		data = 5;
	else if(value < 2200)
		data = 6;
	else
		data = 7;
	return data;

}

int bq2429x_set_iinlim(int mA)
{
	u8 reg;

	if (bq24297_i2c_read_u8(0x00, &reg))
		printf("no response from bq24297\n");
	else { /* bit 0..2 are IINLIM */
		reg &= ~0x7;
		reg |= bq24296_get_limit_current(mA);
		if (bq24297_i2c_write_u8(0x00, reg))
			printf("bq24297: could not set %d mA\n", mA);
	}

	return 0;
}

int board_init(void)
{
	int ilim;
	board_init_overwritten();	/* do everything inherited from LC15 board */
	/* set bq24297 current limit to 2 A if we operate from no battery and 100 mA if we have */
	ilim = bq2429x_battery_present() ? 100 : 2000;
	bq2429x_set_iinlim(ilim);
	return 0;
}

#endif

// TODO: we can (re)enable tca6424 code (because we do have it on this board)
