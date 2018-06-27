/*
 * basically the same as the Letux GTA15 (aka Pyra Phone)
 * except that we change the UART3 pinmux because it is wired up
 * differently
 */

/* move away definition by included file */
#define board_init board_init_overwritten
#define set_muxconf_regs set_muxconf_regs_inherited

#define CONFIG_MORE_FDT more_set_fdtfile

extern void set_muxconf_regs_inherited(void);
#define sysinfo sysinfo_disabled

#include "../../goldelico/letux-cortex15/lc15.c"

#undef sysinfo
#undef set_muxconf_regs
#undef board_init

const struct omap_sysinfo sysinfo = {
	"Board: GTA15\n"
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
	{RFBI_DATA5, (PTD | IEN | M6)}, /* GPIO6_171: pull down modem ignite */
	{HSI2_CAFLAG, (PTU | IEN | M6)}, /* GPIO3_80: usb hub reset */
	{TIMER8_PWM_EVT, (PTU | IEN | M6)}, /* GPIO8_230: keyboard backlight - pulled high */
	/* main board revision detection */
	{RFBI_HSYNC0, (PTU | IEN | M6)}, /* GPIO6_160: rev1 */
	{GPIO6_182, (PTU | IEN | M6)}, /* GPIO6_182: rev2 */
	{GPIO6_185, (PTU | IEN | M6)}, /* GPIO6_185: rev3 */
};

void set_muxconf_regs(void)
{
	set_muxconf_regs_inherited();
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

#if 0
	printf("bq24297: r9 = %02x\n", reg);
#endif

	return !(reg & 0x03);	/* no NTC fault - assume battery is inserted */
}

/**
 * @brief tca642x_init - Pyra default values for the GPIO expander
 * input reg, output reg, polarity reg, configuration reg (0=output)
 */
#define P00_HDMI_CT_HPD	0x01
#define P01_HDMI_LS_OE	0x02
#define P02_NONE	0x04
#define P03_NONE	0x08
#define P04_NONE	0x10
#define P05_FAULT2	0x20	/* also L-RED */
#define P06_NONE	0x40
#define P07_NONE	0x80

#define P10_EN_USB	0x01
#define P11_EN_HOST1	0x02
#define P12_EN_HOST2	0x04
#define P13_CHG_INT	0x08
#define P14_NONE	0x10
#define P15_NONE	0x20
#define P16_MICINT	0x40
#define P17_EN_MODEM	0x80

#define P20_SD_HS_AMP	0x01
#define P21_CHG_STAT	0x02	/* also PWR-RED */
#define P22_NONE	0x04
#define P23_NONE	0x08
#define P23_EN_OTG	0x08	/* EN_OTG in Revision 5.1 and later */
#define P24_NONE	0x10
#define P25_FAULT1	0x20	/* also R-RED */
#define P26_NONE	0x40
#define P27_NONE	0x80

struct tca642x_bank_info pyra_tca642x_init[] = {
	{ .input_reg = 0x00,	/* not really initialized */
	  .output_reg = 0x00,
	  .polarity_reg = 0x00,
	  .configuration_reg = 0x00 },	/* all others are outputs */
	{ .input_reg = 0x00,
	  .output_reg = 0x00,
	  .polarity_reg = 0x00,
	  .configuration_reg = P16_MICINT | P13_CHG_INT },	/* all others are outputs */
	{ .input_reg = 0x00,
	  .output_reg = 0x00,
	  .polarity_reg = 0x00,
	  .configuration_reg = P20_SD_HS_AMP | P21_CHG_STAT | P22_PWR_GREEN | P23_PWR_BLUE },	/* all others are outputs */
};

/*
 * Board Revision Detection
 *
 * gpio6_160, gpio6_182 and gpio6_185 can optionally be pulled down
 * by 10k resistors.
 */

static int get_pyra_mainboard_revision(void)
{
	static int revision = -1;

	static char revtable[8] = {	/* revision table defined by pull-down R1901, R1902, R1903 */
		[7] = 10,
		[6] = 11,
		[5] = 12,
		[3] = 13,
		[4] = 14,
		[2] = 15,
		[1] = 16,
		[0] = 17,
	};

	if (revision == -1) {
		if (!gpio_request(160, "rev1") &&
		    !gpio_request(182, "rev2") &&
		    !gpio_request(185, "rev3")) {
			gpio_direction_input(160);
			gpio_direction_input(182);
			gpio_direction_input(185);

			revision = gpio_get_value(185) << 2 |
				gpio_get_value(182) << 1 |
				gpio_get_value(160);
#if 0
			printf("version code 0x%01x\n", revision);
#endif
			revision = revtable[revision];
		} else {
			printf("Error: unable to get board revision GPIOs\n");
		}
	}

	/* FIXME: turn off pull-up to save up ca. 50-750µA */

	printf("Found GTA15 MB V%d.%d\n", revision/10, revision%10);
	return revision;
}

/* called from set_fdtfile in lc15.c */
void CONFIG_MORE_FDT(char *devtree)
{
	int len;
	int rev = get_pyra_mainboard_revision();

	len = strlen(devtree);
	if (len < 5)
		return;	/* some error */

	/* extend by overwriting ".dtb" */
	sprintf(devtree+len-4, "+%s-v%d.%d.dtb", "gta15", rev/10, rev%10);
}

/**
 * @brief board_init for Pyra Phone
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
	printk("Reset:  peripherals\n");
	/* but now reset peripherals */
	gpio_request(144, "peripheral-reset");
	gpio_direction_output(144, 0);	/* reset all peripheral chips (incl. tca6424) */
	udelay(1000);	/* 1ms should suffice */
	gpio_direction_output(144, 1);
	gpio_free(144);
	udelay(5000);	/* 5ms should suffice */
#endif

// UNDERSTAND ME: it is important that we program the bq2429x first before
// we initialize the tca6424! Why?
// What is (sometimes) turned on and draws too much energy?
// WWAN-Module? USB DC/DC?

	if (bq24297_i2c_write_u8(0x05, 0x8a))
		printf("bq24297: could not turn off 40 sec watchdog\n");

	if (!bq2429x_battery_present()) {
		u8 reg;

		printf("bq24297: no battery found\n");

		/* if we operate from no battery:
		 * assume we are powered elsewhere and not through standard USB
		 *
		 * -> increase bq24297 current limit to at least 2A
		 * -> reduce VINDPM minimum VBUS voltage to 3.88 V
		 *
		 * Otherwise we can not safely boot into Linux, especially
		 * with higher resistance of the USB cable.
		 */

		if (bq24297_i2c_read_u8(0x00, &reg))
			printf("bq24297: no response from REG0\n");
		else {
			u8 nreg = reg;
			int ilim = reg & 0x7; /* bit 0..2 are IINLIM */
#if 1
			printf("Iin_lim found: %d\n", ilim);
#endif
			if (ilim < 6) {
				ilim = 6;	/* increase to 2A */
				nreg &= ~0x7;
				nreg |= ilim;
#if 1
				printf("Iin_lim changed %d\n", nreg & 0x7);
#endif
			}
#if 1
			printf("Vin_dpm found: %d\n", (reg >> 3) & 0x0f);
#endif
			nreg &= ~0x78;	/* bits 3..6 are VINDPM */
			nreg |= (0 << 3);	/* set level 0 = 3.88V */
#if 1
			printf("Vin_dpm changed: %d\n", (nreg >> 3) & 0x0f);
#endif
			if (nreg != reg) {
				if (bq24297_i2c_write_u8(0x00, nreg))
					printf("bq24297: could not set REG0 to %02x\n", nreg);
#if 1
				else
					printf("bq24297: r0 := %02x\n", nreg);
#endif
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
