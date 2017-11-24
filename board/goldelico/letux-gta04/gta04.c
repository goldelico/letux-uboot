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
 * Author:
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

DECLARE_GLOBAL_DATA_PTR;

char *muxname="unknown";
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
	myudelay(1500*1000);
	while(number-- > 0)
		{ // flash n times
		BLON();
		myudelay(200*1000);
		BLOFF();
		myudelay(200*1000);
		}
	myudelay(1500*1000);
}

#endif


/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_GTA04;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);
	
	return 0;
}

/*
 * Routine: get_board_revision
 */


int get_gta04_revision(void)
{
	static int revision = -1;

	static char revtable[8] = {	/* revision table defined by pull-down R305, R306, R307 */
		9,
		6,
		7,
		3,
		8,
		4,
		5,
		2
	};

	if (revision == -1) {
		if (!gpio_request(171, "rev0") &&
		    !gpio_request(172, "rev1") &&
		    !gpio_request(173, "rev2")) {
			gpio_direction_input(171);
			gpio_direction_input(172);
			gpio_direction_input(173);

			revision = gpio_get_value(173) << 2 |
				gpio_get_value(172) << 1 |
				gpio_get_value(171);
			revision = revtable[revision];
		} else {
			printf("Error: unable to acquire board revision GPIOs\n");
		}
	}

	printf("Found GTA04A%d\n", revision);
	return revision;
}

/* BeagleBoard revisions */
#define REVISION_AXBX	0x7
#define REVISION_CX	0x6
#define REVISION_C4	0x5
#define REVISION_XM_AB	0x0
#define REVISION_XM_C	0x2

int get_board_revision(void)
{
	return REVISION_C4;	// initialize like BB C4
}

int isXM(void)
{
	return 0;
}

#if defined(CONFIG_SPL_BUILD)
/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on both banks.
 */
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	int pop_mfr = 0, pop_id = 0;

	/*
	 * We need to identify what PoP memory is on the board so that
	 * we know what timings to use.  If we can't identify it then
	 * we know it's an xM.  To map the ID values please see nand_ids.c
	 */

	identify_nand_chip(&pop_mfr, &pop_id);

	timings->mr = MICRON_V_MR_165;

	if (pop_mfr == NAND_MFR_STMICRO && pop_id == 0xba) {
		/* 512MB DDR */
		timings->mcfg = NUMONYX_V_MCFG_165(512 << 20);
		timings->ctrla = NUMONYX_V_ACTIMA_165;
		timings->ctrlb = NUMONYX_V_ACTIMB_165;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
	} else if (pop_mfr == NAND_MFR_MICRON && pop_id == 0xba) {
		/* Beagleboard Rev C4, 512MB Nand/256MB DDR*/
		timings->mcfg = MICRON_V_MCFG_165(128 << 20);
		timings->ctrla = MICRON_V_ACTIMA_165;
		timings->ctrlb = MICRON_V_ACTIMB_165;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
	} else if (pop_mfr == NAND_MFR_MICRON && pop_id == 0xbc) {
		/* Beagleboard Rev C5, 256MB DDR */
		timings->mcfg = MICRON_V_MCFG_200(256 << 20);
		timings->ctrla = MICRON_V_ACTIMA_200;
		timings->ctrlb = MICRON_V_ACTIMB_200;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
	} else 	if (pop_mfr == 0 && pop_id == 0) {
		// FIXME: SAMSUNG_MCP with 1GB DDR
		// should do this: http://git.goldelico.com/?p=gta04-xloader.git;a=blob;f=board/omap3530beagle/omap3530beagle.c;h=97e9ffc9c5296073a6e81354548bea9c243a23e5;hb=f1cbec2c777e82b7375b3a931dc51db5ccf61e83#l688
		/*
		 * this is a working set of parameters for the SAMSUNG_MCP on GTA04A5
		 * can certainly be optimized
		 */
#define SAMSUNG_MCP(size) (((SAMSUNG_V_MCFG_165(size)) & \
					 ~(V_MCFG_CASWIDTH_10B | \
					   V_MCFG_BANKALLOCATION_RBC)) | \
				 V_MCFG_CASWIDTH(11) | \
				 V_MCFG_B32NOT16_32 | \
				 V_MCFG_DEEPPD_EN)
		/* 512MB DDR */
		timings->mcfg = SAMSUNG_MCP(512 << 20);
		timings->ctrla = NUMONYX_V_ACTIMA_165;
		timings->ctrlb = NUMONYX_V_ACTIMB_165;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
	} else if (pop_mfr == 0) {
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

	// debugging/developing Samsung RAM+OneNAND for GTA04A5

#if 0
	/* 512MB DDR - boots but fails in kernel */
	timings->mcfg = NUMONYX_V_MCFG_165(512 << 20);
	timings->ctrla = NUMONYX_V_ACTIMA_165;
	timings->ctrlb = NUMONYX_V_ACTIMB_165;
	timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
#endif
#if 0
	/* 2x256MB DDR - works */
	timings->mcfg = NUMONYX_V_MCFG_165(256 << 20);
	timings->ctrla = NUMONYX_V_ACTIMA_165;
	timings->ctrlb = NUMONYX_V_ACTIMB_165;
	timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
#endif
#if 0
	/* 2x256MB DDR - works? */
	timings->mcfg = MICRON_V_MCFG_200(256 << 20);
	timings->ctrla = MICRON_V_ACTIMA_200;
	timings->ctrlb = MICRON_V_ACTIMB_200;
	timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
#endif
}

int spl_start_uboot(void)
{
	// check if AUX button is pressed and return 1 to always start U-Boot
	return 1;	/* set to 0 to try Falcon boot to Linux */
}

#endif	/* defined(CONFIG_SPL_BUILD) */

#ifdef CONFIG_USB_MUSB_OMAP2PLUS
static struct musb_hdrc_config musb_config = {
	.multipoint     = 1,
	.dyn_fifo       = 1,
	.num_eps        = 16,
	.ram_bits       = 12,
};

static struct omap_musb_board_data musb_board_data = {
	.interface_type	= MUSB_INTERFACE_ULPI,
};

static struct musb_hdrc_platform_data musb_plat = {
#if defined(CONFIG_USB_MUSB_HOST)
	.mode           = MUSB_HOST,
#elif defined(CONFIG_USB_MUSB_GADGET)
	.mode		= MUSB_PERIPHERAL,
#else
#error "Please define either CONFIG_USB_MUSB_HOST or CONFIG_USB_MUSB_GADGET"
#endif
	.config         = &musb_config,
	.power          = 100,
	.platform_ops	= &omap2430_ops,
	.board_data	= &musb_board_data,
};
#endif
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_DFU_SUPPORT)
void spl_gadget_init(void)
{
#ifdef CONFIG_USB_MUSB_OMAP2PLUS
	musb_register(&musb_plat, &musb_board_data, (void *)MUSB_BASE);
#endif
}
#endif
/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */

#define TCA6507_BUS			(2-1)	// I2C2
#define TCA6507_ADDRESS		0x45
#define ITG3200_BUS			(2-1)	// I2C2
#define ITG3200_ADDRESS		0x68

/* register numbers */
#define TCA6507_SELECT0						0
#define TCA6507_SELECT1						1
#define TCA6507_SELECT2						2

static void tps65950_init(void)
{
	twl4030_power_init();

	// we have no LEDs on TPS65950 on GTA04
	// but a power on/off button (8 seconds)
	twl4030_power_reset_init();

#if 0 // FIXME: enable only on demand if we want to test BT/WIFI - and apply RESET
	/* Bluetooth VAUX4 = 3.3V -- CHECKME: 3.3 V is not officially supported! We use 0x09 = 2.8V here*/
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX4_DEDICATED,
							/*TWL4030_PM_RECEIVER_VAUX4_VSEL_33*/ 0x09,
							TWL4030_PM_RECEIVER_VAUX4_DEV_GRP,
							TWL4030_PM_RECEIVER_DEV_GRP_P1);
#endif

#if 1	// duplicate with twl4030-additions
	
#define TWL4030_BB_CFG_BBCHEN		(1 << 4)
#define TWL4030_BB_CFG_BBSEL_3200MV	(3 << 2)
#define TWL4030_BB_CFG_BBISEL_500UA	2
	
	/* Enable battery backup capacitor (3.2V, 0.5mA charge current) */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, TWL4030_PM_RECEIVER_BB_CFG,
						 TWL4030_BB_CFG_BBCHEN | TWL4030_BB_CFG_BBSEL_3200MV |
						 TWL4030_BB_CFG_BBISEL_500UA);
#endif
	
}

static void tca6507_reset(void)
{
	i2c_set_bus_num(TCA6507_BUS);	// write I2C2
	i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT0, 0);
	i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT1, 0);
	i2c_reg_write(TCA6507_ADDRESS, TCA6507_SELECT2, 0x40);	// pull down reset for WLAN&BT chip
	i2c_set_bus_num(0);	// write I2C1
}

static void itg3200_reset(void)
{
	/*
	 * if the GTA04 is connected to USB power first and
	 * the battery is inserted after this, all power rails
	 * have oscillated until the battery is available
	 * this makes the ITG3200 Power On Reset fail.
	 * (GTA04A3 and GTA04A4 only)
	 */

	i2c_set_bus_num(ITG3200_BUS);	// write I2C2
	if(i2c_probe(ITG3200_ADDRESS))
		{ // ITG3200 does not respond
		printf("ITG3200 does not respond\n");
		// FIXME: temporarily switch off VAUX2
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
								/*TWL4030_PM_RECEIVER_VAUX2_VSEL_28*/ 0x09,
								TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
								TWL4030_PM_RECEIVER_DEV_GRP_P1);
		udelay(30*1000);	// wait a little until power drops
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
								/*TWL4030_PM_RECEIVER_VAUX2_VSEL_28*/ 0x09,
								TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
								TWL4030_PM_RECEIVER_DEV_GRP_P1);
		udelay(10*1000);	// wait a little until power stabilizes and ITG did reset
		if(i2c_probe(ITG3200_ADDRESS))	 // ITG3200 does not respond
			printf("ITG3200 still does not respond\n");
		else
			printf("ITG3200 now ok\n");
		}
	i2c_set_bus_num(0);	// write I2C1
}

static void check_watchdog(void)
{
#if 0	// check if watchdog is switched off
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
		
#endif				  
}

int misc_init_r(void)
{
	bool generate_fake_mac = false;
	char devtree[50];

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

	tca6507_reset();

	tps65950_init();

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

	omap_die_id_display();

	check_watchdog();

#ifdef CONFIG_USB_MUSB_OMAP2PLUS
	musb_register(&musb_plat, &musb_board_data, (void *)MUSB_BASE);
#endif

	if (generate_fake_mac)
		omap_die_id_usbethaddr();

	/* we must load different device trees depending
	   on the board revision */

	strcpy(devtree, "unknown");

	switch(get_gta04_revision()) {
		case 2:
			strcpy(devtree, "omap3-gta04a2");
			break;
		case 3:
			itg3200_reset();
			strcpy(devtree, "omap3-gta04a3");
			generate_fake_mac = true;
			break;
		case 4:
			itg3200_reset();
			strcpy(devtree, "omap3-gta04a4");
			generate_fake_mac = true;
			break;
		case 5:
			strcpy(devtree, "omap3-gta04a5");
			generate_fake_mac = true;
			break;
		default:
			printf("Error: unknown GTA04 revision\n");
			break;
		case -1:
			printf("Error: unable to acquire board revision GPIOs\n");
			break;
		}
#if defined(CONFIG_ONENAND_BOOT)
	strcat(devtree, "one");	/* choose a DTB that tells kernel that we have OneNAND */
#endif
	strcat(devtree, peripheral);	/* append potential +b2/b3 suffix for peripheral board(s) */
	setenv("mux", muxname);
	setenv("devicetree", devtree);
	strcat(devtree, ".dtb");
	setenv("fdtfile", devtree);
	printf("Device Tree: %s\n", devtree);

	/* unset beaglerev variable to trick findfdt into idleness  */
	setenv("beaglerev", "undefined");

// FIXME: mangle into device tree file
// or make kernel auto-detect

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

#if 0	// debugging for GTA04A5 IrDA control
	gpio_request(175, "irda");
	gpio_direction_input(175);
	printf("gpio175 = %d\n", gpio_get_value(175));
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
	struct control_prog_io *prog_io_base = (struct control_prog_io *)OMAP34XX_CTRL_BASE;
	u32 value;

	MUX_BEAGLE();
	MUX_GTA04();

	/* Enable i2c2 pullup resisters */
	value = readl(&prog_io_base->io1);
	value &= ~(PRG_I2C2_PULLUPRESX);
	writel(value, &prog_io_base->io1);
}

#if defined(CONFIG_GENERIC_MMC) && !defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, 0, 0, -1, -1);
	return 0;
}
#endif

#if defined(CONFIG_GENERIC_MMC) && defined(CONFIG_SPL_BUILD)

static void gta04_power_init(void)
{
#if defined(CONFIG_SPL_BUILD)
	int pop_mfr = 0, pop_id = 0;
	identify_nand_chip(&pop_mfr, &pop_id);
#if 1
	printf("pop_mfr = %02x pop_id = %02x\n", pop_mfr, pop_id);
#endif
#endif
	// seems to be too late to avoid spurious RX characters through IrDA receiver
	// hence we do it already by MUX_GTA04() - but there we can't check the board revision!
	// so we coud better revert it for non-A5 boards
	return;
	if (get_gta04_revision() == 5) {
#if 1
		printf("gta04a5\n");
#endif
		/* turn off IrDA receiver */
		printf("turn off IrDA\n");
		MUX_VAL(CP(MCSPI1_CS1), (IEN  | PTU | EN | M4)) /*GPIO_175/MMC3CMD - IrDA (GTA04A5 only) */;
	}
}

#endif	/* defined(CONFIG_GENERIC_MMC) */

void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(0);
#if defined(CONFIG_SPL_BUILD)
	gta04_power_init();
#endif
}

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
#endif	/* CONFIG_USB_ETHER */
