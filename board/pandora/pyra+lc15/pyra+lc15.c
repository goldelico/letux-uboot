/*
 * basically the same as the Letux Cortex 15
 * except that we change the UART3 pinmux because it is wired up
 * differently
 */

/* move away definition by included file */
#define board_init board_init_overwritten
#define set_muxconf_regs_essential set_muxconf_regs_essential_inherited
#define sysinfo sysinfo_disabled

#include "../../goldelico/letux-cortex15/lc15.c"

#undef sysinfo
#undef set_muxconf_regs_essential
#undef board_init

const struct omap_sysinfo sysinfo = {
	"Board: Pyra with Letux Cortex 15\n"
};

const struct pad_conf_entry core_padconf_array_essential_pyra[] = {

	/* enable SDIO4 (right SD slot) */
	{UART3_RX_IRRX, (PTU | IEN| M4)}, /*  SDIO4_CLK */
	{UART3_TX_IRTX, (PTU | IEN| M4)},  /*  SDIO4_CMD */
	{UART5_RX, (PTU | IEN| M4)},  /*  SDIO4_DATA1 */
	{UART5_TX, (PTU | IEN| M4)},  /*  SDIO4_DATA2 */
	{UART5_CTS, (PTU | IEN| M4)},  /*  SDIO4_DATA0 */
	{UART5_RTS, (PTU | IEN| M4)},  /*  SDIO4_DATA3 */
	/* switch UART3 to different pins */
	{RFBI_DATA1, (PTU | IEN | M4)},  /*  UART3_RX_IRRX */
	{RFBI_DATA2, (M4)}, /*  UART3_TX_IRTX */
	/* enable I2C1..5 pullups */
	{I2C1_PMIC_SCL, (PTU | IEN | M0)}, /* I2C1_SCL */
	{I2C1_PMIC_SDA, (PTU | IEN | M0)}, /* I2C1_SDA */
	{I2C2_SCL, (PTU | IEN | M0)}, /* I2C2_SCL */
	{I2C2_SDA, (PTU | IEN | M0)}, /* I2C2_SDA */
	{I2C3_SCL, (PTU | IEN | M0)}, /* I2C3_SCL */
	{I2C3_SDA, (PTU | IEN | M0)}, /* I2C3_SDA */
	{I2C4_SCL, (PTU | IEN | M0)}, /* I2C4_SCL */
	{I2C4_SDA, (PTU | IEN | M0)}, /* I2C4_SDA */
	/* some resets */
	{MCSPI1_CS1, (PTU | IEN | M6)}, /* GPIO5_144: peripheral reset */
	{HSI2_CAFLAG, (PTU | IEN | M6)}, /* GPIO3_80: usb hub reset */
	{TIMER8_PWM_EVT, (PTU | IEN | M6)}, /* GPIO8_230: keyboard backlight - pulled high */
};

void set_muxconf_regs_essential(void)
{
	set_muxconf_regs_essential_inherited();
	do_set_mux((*ctrl)->control_padconf_core_base,
		   core_padconf_array_essential_pyra,
		   sizeof(core_padconf_array_essential_pyra) /
		   sizeof(struct pad_conf_entry));
}

/* U-Boot only code */
#if !defined(CONFIG_SPL_BUILD)

// FIXME: add a bq2427x driver?

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
		printf("bq24297: no response from REG9\n");
		return 0;
	}

	printf("  r9=%02x\n", reg);

	return !(reg & 0x03);	/* no NTC fault - assume battery is inserted */
}

/**
 * @brief tca642x_init - Pyra default values for the GPIO expander
 * input reg, output reg, polarity reg, configuration reg (0=output)
 */
struct tca642x_bank_info pyra_tca642x_init[] = {
	{ .input_reg = 0x00,
	  .output_reg = 0x00,
	  .polarity_reg = 0x00,
	  .configuration_reg = 0x00 },	/* drive fault low (red LED on) */
	{ .input_reg = 0x00,
	  .output_reg = 0x00,
	  .polarity_reg = 0x00,
	  .configuration_reg = 0x48 },	/* mic-pres and otg-int are inputs */
	{ .input_reg = 0x00,
	  .output_reg = 0x00,
	  .polarity_reg = 0x00,
	  .configuration_reg = 0x03 },	/* driver fault low (red LED on), open-drain (input) en-hs-amp, chg-stat = input */
};

/**
 * @brief board_init for Pyra
 *
 * @return 0
 */
int board_init(void)
{
	/* do the same as on LC15 board/EVM */
	gpmc_init();
	gd->bd->bi_arch_number = MACH_TYPE_OMAP5_SEVM;
	gd->bd->bi_boot_params = (0x80000000 + 0x100); /* boot param addr */

#if 1
	printk("reset peripherals\n");
	/* but now reset peripherals */
	gpio_request(144, "peripheral-reset");
	gpio_direction_output(144, 0);	/* reset all peripheral chips (incl. tca6424) */
	udelay(1000);	/* 1ms should suffice */
	gpio_direction_output(144, 1);
	gpio_free(144);
	udelay(5000);	/* 5ms should suffice */
#endif

// UNDERSTAND ME: it is important that we program the bq2429x first before
// we initialize the tca6424! Why? What is turned on and draws too much energy?

	if (bq24297_i2c_write_u8(0x05, 0x8a))
		printf("bq24297: could not turn off 40 sec watchdog\n");

	if (!bq2429x_battery_present()) {
		u8 reg;
		/* increase bq24297 current limit to 2 A if we operate from no battery */
		/* otherwise we can not safely boot into Linux */

		if (bq24297_i2c_read_u8(0x00, &reg))
			printf("bq24297: no response from REG0\n");
		else {
			int ilim = reg & 0x7; /* bit 0..2 are IINLIM */
			printf("Iin_lim detected %d\n", ilim);
			if (ilim < 6)
				{
				ilim=6;	/* increase to min. of 2A */
				reg &= ~0x7;
				reg |= ilim;
				printf("Iin_lim changed %d\n", ilim);
				if (bq24297_i2c_write_u8(0x00, reg))
					printf("bq24297: could not set REG0 to %02x\n", reg);
			}
		}
	}

#if defined(CONFIG_TCA642X)
	extern int tca642x_info(uchar chip);
	tca642x_info(CONFIG_SYS_I2C_TCA642X_ADDR);
	printk("tca6424 init\n");
	tca642x_set_inital_state(CONFIG_SYS_I2C_TCA642X_ADDR, pyra_tca642x_init);
	printk("tca6424 initialized\n");
	tca642x_info(CONFIG_SYS_I2C_TCA642X_ADDR);
#endif

	return 0;
}

#endif
