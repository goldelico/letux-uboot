#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <ddr/ddr_common.h>

#define EFUSE_BASE	0xB3480000
#define EFUSE_CTRL	EFUSE_BASE + 0x0
#define EFUSE_CFG	EFUSE_BASE + 0x4
#define EFUSE_STAT	EFUSE_BASE + 0x8
#define EFUSE_DATA(n)   (EFUSE_BASE + 0xC + (n) * 4)

#define EFUSE_CTRL_ADDR_POS     (21)
#define EFUSE_CTRL_LEN_POS      (16)
#define EFUSE_CTRL_PD           (1 << 8)
#define EFUSE_CTRL_RDEN         (1 << 0)

#define EFUSE_CFG_RD_ADJ_POS    (24)
#define EFUSE_CFG_RD_STROBE_POS (16)

#define EFUSE_STAT_RDDONE       (1 << 0)

#define SOCINFO_WORD_ADDR       0x1A
#define SOCINFO_BYTE_ADDR       0x6B
#define SOCINFO_BITS            (32 + 8)

#define WORD_ALIGNED(_val)      ((_val) & ~(sizeof(int) - 1))
#define BITS_TO_WORD(_bits)     (((_bits) + 31) / 32)
#define BITS_TO_BYTE(_bits)     (((_bits) + 7) / 8)
#define BYTE_TO_BITS(_byte)     ((_byte) * 8)
#define MASK_BITS(_val,_bits)   ((_val) & (1 << (_bits)) - 1)

#define REG32(addr) *(volatile unsigned int *)(addr)

static enum soc_type {
	SOC_X2600,
	SOC_X2600E,
	SOC_X2600M,
	SOC_X2600H,
	SOC_X2660,
	SOC_X2670,
	SOC_X2670M,
	SOC_UNKNOWN = 0xF,
};

static struct soc_desc {
	const enum soc_type soc;
	const char *chip;
};

static const struct soc_desc desc[] = {
	{SOC_X2600,  "X2600" },
	{SOC_X2600E, "X2600E"},
	{SOC_X2600M, "X2600M"},
	{SOC_X2600H, "X2600H"},
	{SOC_X2660,  "X2660" },
	{SOC_X2670,  "X2670" },
	{SOC_X2670M, "X2670M"},
};

void read_socid(unsigned int *data)
{
	int word_num = BITS_TO_WORD(SOCINFO_BITS);
	int i = 0;

	REG32(EFUSE_CFG) = 0x4 << EFUSE_CFG_RD_ADJ_POS | 0x0 << EFUSE_CFG_RD_STROBE_POS;

	REG32(EFUSE_CTRL) = 0;
	REG32(EFUSE_CTRL) = SOCINFO_WORD_ADDR << EFUSE_CTRL_ADDR_POS | (word_num - 1) << EFUSE_CTRL_LEN_POS;
	REG32(EFUSE_CTRL)|= EFUSE_CTRL_RDEN;

	while(!(REG32(EFUSE_STAT) & EFUSE_STAT_RDDONE));
	REG32(EFUSE_CTRL) = EFUSE_CTRL_PD;

	for(i = 0; i < word_num; i++) {
		data[i] = REG32(EFUSE_DATA(i));
	}
}

static int checkbit(unsigned int *s,unsigned int *d,int ss,int ds,int bsz)
{
        int sg32,sb32,dg32,db32;

        while(bsz > 0){
                sg32 = ss / 32;
                sb32 = ss % 32;
                dg32 = ds / 32;
                db32 = ds % 32;
                if(((s[sg32] >> sb32) & 1) != ((d[dg32] >> db32) & 1))
			break;
                ss++;
                ds++;
                bsz--;
        }
        return bsz;
}

unsigned int check_socid()
{
	unsigned int vendor = 0;
	unsigned int type = 0;
	unsigned int capacity = 0;
	unsigned int ddrid  = 0;
	unsigned int socid = 0;
	unsigned int data[BITS_TO_WORD(SOCINFO_BITS)] = {0};
	unsigned int start_pos = SOCINFO_BYTE_ADDR - WORD_ALIGNED(SOCINFO_BYTE_ADDR);
	int ret = 0;
	int i = 0;
	enum soc_type soc = SOC_UNKNOWN;

	read_socid(data);
	ret = checkbit(data, data, BYTE_TO_BITS(start_pos), BYTE_TO_BITS(start_pos) + SOCINFO_BITS / 2, SOCINFO_BITS / 2);
	socid  = data[1] >> (32 - SOCINFO_BITS / 2);
	if(ret != 0 || socid == 0) {
		printf("invalid soc id %x%x\n", data[1], data[0]);
		return -1;
	}

	soc = socid >> 16 & 0xf;
	vendor = socid >> 11 & 0xf;
	type   = (socid >> 15 & 0x1) ? TYPE_DDR3 : TYPE_DDR2;
	capacity = socid >> 7 & 0xf;
	ddrid = DDR_CHIP_ID(vendor, type, capacity);

	for (i = 0; i < ARRAY_SIZE(desc); i++) {
		if (desc[i].soc == soc) {
			printf("SOC: %s\n", desc[i].chip);
			break;
		}
	}

	return ddrid;
}
