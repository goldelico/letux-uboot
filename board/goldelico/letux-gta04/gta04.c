/*
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 * Authos:
 *  Nikolaus Schaller <hns@goldelico.com>
 *  adapted to GTA04
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <dm.h>
#include <ns16550.h>
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif
#include <twl4030.h>
#include <linux/mtd/nand.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <asm/omap_musb.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/musb.h>
#include "gta04.h"
#include <command.h>

#ifdef CONFIG_USB_EHCI
#include <usb.h>
#include <asm/ehci-omap.h>
#endif

char *muxname="unknown";
char *devicetree="unknown";
char *peripheral="";

static const struct ns16550_platdata gta04_serial = {
	.base = OMAP34XX_UART3,
	.reg_shift = 2,
	.clock = V_NS16550_CLK
};

U_BOOT_DEVICE(gta04_uart) = {
	"ns16550_serial",
	&gta04_serial
};

#if 0	/* testing tool; you can call notify() anywhere even before initialization to see how far the code comes */

/******************************************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 *****************************************************************************/
static inline void mydelay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
					  "bne 1b":"=r" (loops):"0"(loops));
}

static inline void myudelay(unsigned long us)
{
	mydelay(us * 200); /* approximate */
}

#define BLON()	MUX_VAL(CP(GPMC_NCS6),      (IEN | PTU | EN  | M4)) /*GPT_PWM11/GPIO57*/
#define BLOFF()	MUX_VAL(CP(GPMC_NCS6),      (IEN | PTD | EN  | M4)) /*GPT_PWM11/GPIO57*/

void notify(int number)
{ // flash LCD backlight (on A3 board only or remove R209!)
	BLOFF();
	myudelay(200*1000);
	BLON();
	myudelay(50*1000);	// flash
	BLOFF();
	myudelay(1500*1000);
}

#endif


/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_GTA04;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);
	
	return 0;
}

/*
 * Routine: get_board_revision
 * Description: Detect if we are running on a Beagle revision Ax/Bx,
 *		C1/2/3, C4 or xM. This can be done by reading
 *		the level of GPIO173, GPIO172 and GPIO171. This should
 *		result in
 *		GPIO173, GPIO172, GPIO171: 1 1 1 => Ax/Bx
 *		GPIO173, GPIO172, GPIO171: 1 1 0 => C1/2/3
 *		GPIO173, GPIO172, GPIO171: 1 0 1 => C4
 *		GPIO173, GPIO172, GPIO171: 0 0 0 => xM
 */

int get_board_revision(void)
{
	return 6;	// on GTA04, configure default pinmux for C1/2/3
}

int isXM(void)
{
	return 0;
}

#define GTA04A2	 	7
#define GTA04A3	 	3
#define GTA04Ab2	1
#define GTA04A4	 	5
#define GTA04A5	 	6

int get_gta04_revision(void)
{
	int rev = -1;

	static char revision[8] = {	/* revision table defined by pull-down R305, R306, R307 */
		9,
		6,
		7,
		3,
		8,
		4,
		5,
		2
	};

	if (!gpio_request(171, "version-0") &&
		!gpio_request(172, "version-1") &&
		!gpio_request(173, "version-2")) {

		gpio_direction_input(171);
		gpio_direction_input(172);
		gpio_direction_input(173);

		rev = gpio_get_value(173) << 2 |
			gpio_get_value(172) << 1 |
			gpio_get_value(171);

		/* should make gpios pulled down to save a little energy */

		gpio_free(171);
		gpio_free(172);
		gpio_free(173);
		rev = revision[rev];	/* 000 means GTA04A2 */
	}

	printk("Found GTA04A%d\n", rev);

	return rev;
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */

#define TCA6507_BUS			(2-1)	// I2C2
#define TCA6507_ADDRESS		0x45

/* register numbers */
#define TCA6507_SELECT0						0
#define TCA6507_SELECT1						1
#define TCA6507_SELECT2						2

int misc_init_r(void)
{
	char devtree[256];

	/* ITG3200 & HMC5883L VAUX2 = 2.8V */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
							/*TWL4030_PM_RECEIVER_VAUX2_VSEL_28*/ 0x09,
							TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
							TWL4030_PM_RECEIVER_DEV_GRP_P1);
	/* Camera VAUX3 = 2.5V */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX3_DEDICATED,
							/*TWL4030_PM_RECEIVER_VAUX3_VSEL_25*/ 0x02,
							TWL4030_PM_RECEIVER_VAUX3_DEV_GRP,
							TWL4030_PM_RECEIVER_DEV_GRP_P1);

	i2c_set_bus_num(TCA6507_BUS);	// write I2C2
	i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT0, 0);
	i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT1, 0);
	i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT2, 0x40);	// pull down reset for WLAN&BT chip

	i2c_set_bus_num(0);	// write I2C1

#if 0 // FIXME: enable only on demand if we want to test BT/WIFI - and apply RESET
	/* Bluetooth VAUX4 = 3.3V -- CHECKME: 3.3 V is not officially supported! We use 0x09 = 2.8V here*/
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX4_DEDICATED,
							/*TWL4030_PM_RECEIVER_VAUX4_VSEL_33*/ 0x09,
							TWL4030_PM_RECEIVER_VAUX4_DEV_GRP,
							TWL4030_PM_RECEIVER_DEV_GRP_P1);
#endif

	/* we must load different device trees depending
	   on the board revision */

	if(strcmp(devicetree, "omap3-gta04") == 0) {
		int revision = get_gta04_revision();

		switch(revision) {
			case GTA04A2:
				strcpy(devtree, "omap3-gta04a2");
				break;
			case GTA04Ab2:	/* GTA04 Custom for +b2 */
			case GTA04A3:
				strcpy(devtree, "omap3-gta04a3");
				break;
			case GTA04A4:
				strcpy(devtree, "omap3-gta04a4");
				break;
			case GTA04A5:
				strcpy(devtree, "omap3-gta04a5");
				break;
			default:
				printf("Error: unknown revision GPIOs: %x\n", revision);
				break;
			case -1:
				printf("Error: unable to acquire board revision GPIOs\n");
				break;
			}
		strcat(devtree, peripheral);	/* append potential +b2/b3 suffix for peripheral board(s) */
		devicetree=devtree;
	}

	setenv("mux", muxname);
	setenv("devicetree", devicetree);
	printf("Device Tree: %s\n", devicetree);

	switch (get_cpu_family()) {
		case CPU_OMAP34XX:
			if ((get_cpu_rev() >= CPU_3XX_ES31) &&
				(get_sku_id() == SKUID_CLK_720MHZ))
				setenv("mpurate", "720");
			else
				setenv("mpurate", "600");
			break;
		case CPU_OMAP36XX:
			/* check the "Speed Binned" bit for AM/DM37xx
			 in the Control Device Status Register */
			if(readw(0x4800244C) & (1<<9))
				setenv("mpurate", "1000");
			else
				setenv("mpurate", "800");
			break;
	}
	twl4030_power_init();
	
	// we have no LEDs on TPS on GTA04
	// but a power on/off button (8 seconds)
	twl4030_power_reset_init();

#if 0
	{
	// FIXME: check this!!!
	struct gpio *gpio5_base = (struct gpio *)OMAP34XX_GPIO5_BASE;
	struct gpio *gpio6_base = (struct gpio *)OMAP34XX_GPIO6_BASE;
	
	/* Configure GPIOs to output */
	writel(~(GPIO23 | GPIO10 | GPIO8 | GPIO2 | GPIO1), &gpio6_base->oe);
	writel(~(GPIO31 | GPIO30 | GPIO29 | GPIO28 | GPIO22 | GPIO21 |
			 GPIO15 | GPIO14 | GPIO13 | GPIO12), &gpio5_base->oe);
	
	/* Set GPIOs */
	writel(GPIO23 | GPIO10 | GPIO8 | GPIO2 | GPIO1,
		   &gpio6_base->setdataout);
	writel(GPIO31 | GPIO30 | GPIO29 | GPIO28 | GPIO22 | GPIO21 |
		   GPIO15 | GPIO14 | GPIO13 | GPIO12, &gpio5_base->setdataout);
	}
#endif
	
#if 1	// duplicate with twl4030-additions
	
#define TWL4030_BB_CFG_BBCHEN		(1 << 4)
#define TWL4030_BB_CFG_BBSEL_3200MV	(3 << 2)
#define TWL4030_BB_CFG_BBISEL_500UA	2
	
	/* Enable battery backup capacitor (3.2V, 0.5mA charge current) */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER,
						 TWL4030_BB_CFG_BBCHEN | TWL4030_BB_CFG_BBSEL_3200MV |
						 TWL4030_BB_CFG_BBISEL_500UA, TWL4030_PM_RECEIVER_BB_CFG);
#endif
	
	omap_die_id_display();

#if 0	// check if watchdog is switched off
	{	
		struct _watchdog {
			u32 widr;	/* 0x00 r */
			u8 res1[0x0c];
			u32 wd_sysconfig;	/* 0x10 rw */
			u32 ws_sysstatus;	/* 0x14 r */
			u32 wisr;
			u32 wier;
			u8 res2[0x04];
			u32 wclr;	/* 0x24 rw */
			u32 wcrr;
			u32 wldr;
			u32 wtgr;
			u32 wwps;	/* 0x34 r */
			u8 res3[0x10];
			u32 wspr;	/* 0x48 rw */
		};
		struct _watchdog *wd2_base = (struct watchdog *)WD2_BASE;
		struct prcm *prcm_base = (struct prcm *)PRCM_BASE;

		printf("idlest_wkup = %08x\n", readl(&prcm_base->idlest_wkup));
		printf("widr = %08x\n", readl(&wd2_base->widr));
		printf("wd_sysconfig = %08x\n", readl(&wd2_base->wd_sysconfig));
		printf("ws_sysstatus = %08x\n", readl(&wd2_base->ws_sysstatus));
		printf("wisr = %08x\n", readl(&wd2_base->wisr));
		printf("wier = %08x\n", readl(&wd2_base->wier));
		printf("wclr = %08x\n", readl(&wd2_base->wclr));
		printf("wcrr = %08x\n", readl(&wd2_base->wcrr));
		printf("wldr = %08x\n", readl(&wd2_base->wldr));
		printf("wtgr = %08x\n", readl(&wd2_base->wtgr));
		printf("wwps = %08x\n", readl(&wd2_base->wwps));
		printf("wspr = %08x\n", readl(&wd2_base->wspr));
				  
		writel(WD_UNLOCK2, &wd2_base->wspr);
		myudelay(100);
		writel(WD_UNLOCK1, &wd2_base->wspr);
		myudelay(100);
		writel(WD_UNLOCK2, &wd2_base->wspr);
		myudelay(100);
		writel(WD_UNLOCK1, &wd2_base->wspr);
		myudelay(100);
		writel(WD_UNLOCK2, &wd2_base->wspr);

		myudelay(10000); // so that we can see if the counter counts...
		
		printf("idlest_wkup = %08x\n", readl(&prcm_base->idlest_wkup));
		printf("widr = %08x\n", readl(&wd2_base->widr));
		printf("wd_sysconfig = %08x\n", readl(&wd2_base->wd_sysconfig));
		printf("ws_sysstatus = %08x\n", readl(&wd2_base->ws_sysstatus));
		printf("wisr = %08x\n", readl(&wd2_base->wisr));
		printf("wier = %08x\n", readl(&wd2_base->wier));
		printf("wclr = %08x\n", readl(&wd2_base->wclr));
		printf("wcrr = %08x\n", readl(&wd2_base->wcrr));
		printf("wldr = %08x\n", readl(&wd2_base->wldr));
		printf("wtgr = %08x\n", readl(&wd2_base->wtgr));
		printf("wwps = %08x\n", readl(&wd2_base->wwps));
		printf("wspr = %08x\n", readl(&wd2_base->wspr));

		writel(readl(&wd2_base->wtgr) + 1, &wd2_base->wtgr);
		
		printf("idlest_wkup = %08x\n", readl(&prcm_base->idlest_wkup));
		printf("widr = %08x\n", readl(&wd2_base->widr));
		printf("wd_sysconfig = %08x\n", readl(&wd2_base->wd_sysconfig));
		printf("ws_sysstatus = %08x\n", readl(&wd2_base->ws_sysstatus));
		printf("wisr = %08x\n", readl(&wd2_base->wisr));
		printf("wier = %08x\n", readl(&wd2_base->wier));
		printf("wclr = %08x\n", readl(&wd2_base->wclr));
		printf("wcrr = %08x\n", readl(&wd2_base->wcrr));
		printf("wldr = %08x\n", readl(&wd2_base->wldr));
		printf("wtgr = %08x\n", readl(&wd2_base->wtgr));
		printf("wwps = %08x\n", readl(&wd2_base->wwps));
		printf("wspr = %08x\n", readl(&wd2_base->wspr));
		
	}
#endif				  
				  
	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_BEAGLE();
	MUX_GTA04();
}

#if defined(CONFIG_GENERIC_MMC) && !defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, 0, 0, -1, -1);
	return 0;
}
#endif

#if defined(CONFIG_GENERIC_MMC)
void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(0);
}
#endif

#if defined(CONFIG_USB_EHCI) && !defined(CONFIG_SPL_BUILD)
/* Call usb_stop() before starting the kernel */
void show_boot_progress(int val)
{
	if (val == BOOTSTAGE_ID_RUN_OS)
		usb_stop();
}

static struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED
};

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	return omap_ehci_hcd_init(index, &usbhs_bdata, hccr, hcor);
}

int ehci_hcd_stop(int index)
{
	return omap_ehci_hcd_stop();
}

#endif /* CONFIG_USB_EHCI */

#if defined(CONFIG_USB_ETHER) && defined(CONFIG_USB_MUSB_GADGET)
int board_eth_init(bd_t *bis)
{
	return usb_eth_initialize(bis);
}
#endif

#if defined(CONFIG_SPL_BUILD)
/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on both banks.
 */
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	int pop_mfr, pop_id;

	/*
	 * We need to identify what PoP memory is on the board so that
	 * we know what timings to use.  If we can't identify it then
	 * we know it's an xM.  To map the ID values please see nand_ids.c
	 */
	identify_nand_chip(&pop_mfr, &pop_id);

	timings->mr = MICRON_V_MR_165;

	switch (get_board_revision()) {
#if FIXME
	// decode GTA04 variations
	// get_gta04_revision()
	case REVISION_C4:
		if (pop_mfr == NAND_MFR_STMICRO && pop_id == 0xba) {
			/* 512MB DDR */
			timings->mcfg = NUMONYX_V_MCFG_165(512 << 20);
			timings->ctrla = NUMONYX_V_ACTIMA_165;
			timings->ctrlb = NUMONYX_V_ACTIMB_165;
			timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
			break;
		} else if (pop_mfr == NAND_MFR_MICRON && pop_id == 0xba) {
			/* Beagleboard Rev C4, 512MB Nand/256MB DDR*/
			timings->mcfg = MICRON_V_MCFG_165(128 << 20);
			timings->ctrla = MICRON_V_ACTIMA_165;
			timings->ctrlb = MICRON_V_ACTIMB_165;
			timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
			break;
		} else if (pop_mfr == NAND_MFR_MICRON && pop_id == 0xbc) {
			/* Beagleboard Rev C5, 256MB DDR */
			timings->mcfg = MICRON_V_MCFG_200(256 << 20);
			timings->ctrla = MICRON_V_ACTIMA_200;
			timings->ctrlb = MICRON_V_ACTIMB_200;
			timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
			break;
		}
	case REVISION_XM_AB:
	case REVISION_XM_C:
		if (pop_mfr == 0) {
			/* 256MB DDR */
			timings->mcfg = MICRON_V_MCFG_200(256 << 20);
			timings->ctrla = MICRON_V_ACTIMA_200;
			timings->ctrlb = MICRON_V_ACTIMB_200;
			timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
		} else {
			/* 512MB DDR */
			timings->mcfg = NUMONYX_V_MCFG_165(512 << 20);
			timings->ctrla = NUMONYX_V_ACTIMA_165;
			timings->ctrlb = NUMONYX_V_ACTIMB_165;
			timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
		}
		break;
#endif
	default:
		/* Assume 128MB and Micron/165MHz timings to be safe */
		timings->mcfg = MICRON_V_MCFG_165(128 << 20);
		timings->ctrla = MICRON_V_ACTIMA_165;
		timings->ctrlb = MICRON_V_ACTIMB_165;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
	}
}
#endif
