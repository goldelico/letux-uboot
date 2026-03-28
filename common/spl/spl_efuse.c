#include <common.h>
#include <exports.h>
#include <malloc.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/err.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/errno.h>
#include <asm/arch/base.h>
#include <asm/arch/clk.h>
#include <asm/arch/efuse.h>
#include <efuse.h>

static int efuse_debug = 0;
static int efuse_gpio = -1;
static int efuse_en_active = 0;

static struct seg_info info;

static uint32_t efuse_readl(uint32_t reg_off)
{
    return readl(EFUSE_BASE + reg_off);
}

static void efuse_writel(uint32_t val, uint32_t reg_off)
{
    writel(val, EFUSE_BASE + reg_off);
}

static void otp_r_efuse(uint32_t addr, uint32_t wlen)
{
    unsigned int val;
    int n;

    efuse_writel(0, EFUSE_CTRL);

    for (n = 0; n < 8; n++)
        efuse_writel(0, EFUSE_DATA(n));

    /* set read address and data length */
    val = addr << EFUSE_CTRL_ADDR | (wlen - 1) << EFUSE_CTRL_LEN;
    efuse_writel(val, EFUSE_CTRL);

    val = efuse_readl(EFUSE_CTRL);
    val &= ~EFUSE_CTRL_PD;
    efuse_writel(val, EFUSE_CTRL);

    /* enable read */
    val = efuse_readl(EFUSE_CTRL);
    val |= EFUSE_CTRL_RDEN;
    efuse_writel(val, EFUSE_CTRL);

    // debug("efuse ctrl regval=0x%x\n",val);
    /* wait read done status */
    while (!(efuse_readl(EFUSE_STATE) & EFUSE_STA_RD_DONE))
        ;

    efuse_writel(0, EFUSE_CTRL);
    efuse_writel(EFUSE_CTRL_PD, EFUSE_CTRL);
}

static void rir_r(void)
{
    unsigned int val;

    efuse_writel(0, EFUSE_CTRL);
    efuse_writel(0, EFUSE_DATA(0));
    efuse_writel(0, EFUSE_DATA(1));

    /* set rir read address and data length */
    val = 0x1f << EFUSE_CTRL_ADDR | 0x1 << EFUSE_CTRL_LEN;
    efuse_writel(val, EFUSE_CTRL);

    val = efuse_readl(EFUSE_CTRL);
    val &= ~EFUSE_CTRL_PD;
    efuse_writel(val, EFUSE_CTRL);

    val = efuse_readl(EFUSE_CTRL);
    val &= ~EFUSE_CTRL_PS;
    efuse_writel(val, EFUSE_CTRL);

    val = efuse_readl(EFUSE_CTRL);
    val |= EFUSE_CTRL_RWL;
    efuse_writel(val, EFUSE_CTRL);

    val = efuse_readl(EFUSE_CTRL);
    val |= EFUSE_CTRL_RDEN;
    efuse_writel(val, EFUSE_CTRL);

    /* wait read done status */
    while (!(efuse_readl(EFUSE_STATE) & EFUSE_STA_RD_DONE))
        ;

    efuse_writel(0, EFUSE_CTRL);
    efuse_writel(EFUSE_CTRL_PD, EFUSE_CTRL);

    debug("RIR0=0x%x\n", efuse_readl(EFUSE_DATA(0)));
    debug("RIR1=0x%x\n", efuse_readl(EFUSE_DATA(1)));
}

static int adjust_efuse()
{

    uint32_t val, ns;
    int i, rd_strobe, wr_strobe;
    uint32_t rd_adj, wr_adj;
    int flag;

    ns = 1000000000 / CONFIG_SYS_AHB2_FREQ;
    debug("rate = %d, ns = %d\n", CONFIG_SYS_AHB2_FREQ, ns);

    for (i = 0; i < 0x4; i++)
        if (((i + 1) * ns) > 4)
            break;
    if (i == 0x4)
    {
        debug("get efuse cfg rd_adj fail!\n");
        return -1;
    }
    rd_adj = wr_adj = i;

    for (i = 0; i < 0x8; i++)
        if (((rd_adj + i + 30) * ns) > 100)
            break;
    if (i == 0x8)
    {
        debug("get efuse cfg rd_strobe fail!\n");
        return -1;
    }
    rd_strobe = i;

    for (i = 0; i < 0x3ff; i++)
    {
        val = (wr_adj + i + 3000) * ns;
        if (val >= 13 * 1000)
        {
            val = (wr_adj - i + 3000) * ns;
            flag = 1;
        }
        if (val > 11 * 1000 && val < 13 * 1000)
            break;
    }
    if (i >= 0x3ff)
    {
        debug("get efuse cfg wd_strobe fail!\n");
        return -1;
    }

    if (flag)
        i |= 1 << 10;

    wr_strobe = i;

    debug("rd_adj = %d | rd_strobe = %d | "
           "wr_adj = %d | wr_strobe = %d\n",
           rd_adj, rd_strobe,
           wr_adj, wr_strobe);

    /*set configer register*/
    val = rd_adj << EFUSE_CFG_RD_ADJ | rd_strobe << EFUSE_CFG_RD_STROBE;
    val |= wr_adj << EFUSE_CFG_WR_ADJ | wr_strobe;
    efuse_writel(val, EFUSE_CFG);
    debug("h2clk is %d, efuse_reg 0x%x\n", CONFIG_SYS_AHB2_FREQ, efuse_readl(EFUSE_CFG));
    return 0;
}

static int spl_efuse_init()
{
    debug("eFuse --v1.0 20240313\n");

    if (adjust_efuse() < 0)
        return -1;
    return 0;
}

#if 0
static int spl_efuse_read_socinfo()
{
    uint32_t val[8] = {0};
    int n;

    uint32_t word_address = SOCINFO_WORD_ADDR;
    uint32_t word_num = SOCINFO_WORD_NUM;

    rir_r();
    otp_r_efuse(word_address, word_num);

    for (n = 0; n < word_num; n++)
    {
        val[n] = efuse_readl(EFUSE_DATA(n));
    }
    if ((((val[0] >> 24) & 0xff) | (val[1] & 0xfff) << 8) != (val[1] >> 12) & 0xfffff)
    {
        printf("Socinfo %x%x\n", val[1], val[0]);
    }
    else
    {
        if (0x28820 == (val[1] >> 12) & 0xfffff) // 2882028820
            printf("Board info: AD101P\n");
        else if (0x10840 == (val[1] >> 12) & 0xfffff) // 1084010840
            printf("Board info: AD100N\n");
        else
            printf("Board info: not support %x\n", (val[1] >> 12) & 0xfffff);
    }
    /* clear read done status */
    efuse_writel(0, EFUSE_STATE);

    return 0;
}
#endif

int spl_efuse_read_ddrpar(unsigned int *data)
{
    uint32_t val[8] = {0};
    int n;

    uint32_t word_address = CUSTID0_WORD_ADDR;
    uint32_t word_num = CUSTID0_WORD_NUM;


    spl_efuse_init();

    rir_r();
    otp_r_efuse(word_address, word_num);

    for (n = 0; n < word_num; n++)
    {
        val[n] = efuse_readl(EFUSE_DATA(n));
    }
    data[0] = ((val[0] >> 8) & 0xffffff) | ((val[1] & 0xff) << 24);
    data[1] = (val[1] >> 8) & 0xff;

    /* clear read done status */
    efuse_writel(0, EFUSE_STATE);

    // spl_efuse_read_socinfo(); //通过efuse判断soc型号

    return 0;
}
