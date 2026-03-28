#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <ddr/ddr_common.h>


#define EFUSE_BASE	0xB3540000
#define EFUSE_CTRL	EFUSE_BASE + 0x0
#define EFUSE_CFG	EFUSE_BASE + 0x4
#define EFUSE_STATE	EFUSE_BASE + 0x8
#define EFUSE_DATA	EFUSE_BASE + 0xC

#define EFUSE_CTRL_ADDR_POS	(21)
#define EFUSE_CTRL_LEN_POS	(16)

#define EFUSE_CTRL_RDEN         (1 << 0)
#define EFUSE_STAT_RDDONE       (1 << 0)

#define EFUSE_CFG_RD_ADJ_POS	(20)
#define EFUSE_CFG_RD_STROBE_POS	(16)

#define SOCINFO_ADDR	        0x2B

#define REG32(addr) *(volatile unsigned int *)(addr)

static enum soc_type {
	SOC_X1600   = 0x1b00,
	SOC_X1600E  = 0x0a00,
	SOC_X1600N  = 0x4b00,
	SOC_X1600EN = 0x4a00,
	SOC_X1600HN = 0x4900,
	SOC_X1600MN = 0x6500,
	SOC_X1660   = 0x1b01,
	SOC_X1660L  = 0x4600,
	SOC_UNKNOWN = 0xF,
};

static struct soc_desc {
	const enum soc_type soc;
	const char *chip;
};

static const struct soc_desc desc[] = {
	{SOC_X1600,   "X1600"  },
	{SOC_X1600E,  "X1600E" },
	{SOC_X1600N,  "X1600N" },
	{SOC_X1600EN, "X1600EN"},
	{SOC_X1600HN, "X1600HN"},
	{SOC_X1600MN, "X1600MN"},
	{SOC_X1660,   "X1660"  },
	{SOC_X1660L,  "X1660L" },
};

void read_socid(unsigned int *data)
{
	unsigned int val;

	REG32(EFUSE_CFG) = 0x2 << EFUSE_CFG_RD_ADJ_POS | 0x5 << EFUSE_CFG_RD_STROBE_POS;

	REG32(EFUSE_CTRL) = 0;
	REG32(EFUSE_CTRL) = SOCINFO_ADDR << EFUSE_CTRL_ADDR_POS | 2 << EFUSE_CTRL_LEN_POS | EFUSE_CTRL_RDEN;
	while(!(REG32(EFUSE_STATE) & EFUSE_STAT_RDDONE));

	val = REG32(EFUSE_DATA);

	*data = val & 0xFFFF;
}

unsigned int check_socid()
{
	unsigned int vendor = 0;
	unsigned int type = 0;
	unsigned int capacity = 0;
	unsigned int socid  = 0;
	unsigned int ddrid  = 0;
	int i = 0;

	read_socid(&socid);
	if (socid == 0) {
		printf("invalid soc id %x%x\n", socid);
		return -1;
	}
	vendor = socid >> 11 & 0x7;
	type   = socid >> 14 & 0x1;
	capacity = socid >> 8 & 0x7;
	ddrid = DDR_CHIP_ID(vendor, type, capacity);

	for (i = 0; i < ARRAY_SIZE(desc); i++) {
		if (desc[i].soc == socid) {
			printf("SOC: %s\n", desc[i].chip);
			break;
		}
	}

	return ddrid;
}
