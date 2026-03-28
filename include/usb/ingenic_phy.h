#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpm.h>

#define phy_inl(off)		readl(OTGPHY_BASE + (off))
#define phy_outl(val,off)	writel(val,OTGPHY_BASE + (off))

void otg_phy_init(enum otg_mode_t mode,unsigned extclk);

static inline void set_usb_iddig(enum otg_mode_t mode) {
	unsigned int usbrdt = cpm_inl(CPM_USBRDT);
	switch (mode) {
		case OTG_MODE:
			usbrdt &= ~USBRDT_IDDIG_EN;
			break;
		case HOST_ONLY_MODE:
			usbrdt = (usbrdt & ~USBRDT_IDDIG_REG) | USBRDT_IDDIG_EN;
			break;
		case DEVICE_ONLY_MODE:
			usbrdt |= (USBRDT_IDDIG_REG | USBRDT_IDDIG_EN);
			break;
	}
	cpm_outl(usbrdt, CPM_USBRDT);
}
