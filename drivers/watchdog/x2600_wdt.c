#include <common.h>
#include <asm/io.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpm.h>
#include <asm/arch/base.h>
#include <asm/arch/wdt.h>
#include <asm/arch/tcu.h>
#include <asm/arch/rtc.h>

#define wdt_write(value, reg) writel(value, WDT_BASE + reg)
#define wdt_read(reg) readl(WDT_BASE + reg)

#define tcu_writel(value, reg) writel(value, reg)
#define tcu_readl(reg) readl(reg)

#define WDT_CLK_DIV_1       0
#define WDT_CLK_DIV_4       1
#define WDT_CLK_DIV_16      2
#define WDT_CLK_DIV_64      3
#define WDT_CLK_DIV_256     4
#define WDT_CLK_DIV_1024    5

#define WDT_MAX_COUNT       (0xFFFF)
#define RTC_EN              2

#define OPCR_ERCS_BIT       2

unsigned long get_rtc_internal_clk_rate(void)
{
    unsigned int rtc_32k_is_on = cpm_test_bit(OPCR_ERCS_BIT, CPM_OPCR);

    if (!rtc_32k_is_on)
        return 24000000 / 512;

    return 32768;
}

static int jz_wdt_set_timeout(unsigned long ms)
{
    unsigned int val, us;
    unsigned long count = ms;
    unsigned int clock_div = 0;

    unsigned long rate = get_rtc_internal_clk_rate();

    us = 1000000 / rate;

    count = ms * 1000 / us;

    while (count > WDT_MAX_COUNT) {
        if (clock_div == WDT_CLK_DIV_1024)
            return -1;

        count /= 4;
        clock_div += 1;
    }

    cpm_clear_bit(CPM_CLKGR1_TCU0, CPM_CLKGR1);
    tcu_writel(1 << 16, TCU_TSCR);

    wdt_write(0, WDT_TCER);

    val = (clock_div << 3) | RTC_EN;

    wdt_write(val, WDT_TCSR);

    wdt_write(count, WDT_TDR);

    wdt_write(0, WDT_TCNT);

    wdt_write(1, WDT_TCER);

    return 0;
}

void hw_watchdog_disable(void)
{
    wdt_write(0, WDT_TCER);
    tcu_writel(1 << 16, TCU_TSSR);
}

void hw_watchdog_reset(void)
{
    debug("watchdog reset\n");
    hw_watchdog_disable();

    if (jz_wdt_set_timeout(0)) {
        serial_debug("wdt set time error\n");
    }
}

void hw_watchdog_init(void)
{
    if (jz_wdt_set_timeout(CONFIG_WDT_TIMEOUT_BY_MS)) {
        serial_debug("wdt set time error\n");
    }
}
