#include <common.h>
#include <asm/io.h>
#include "spl_riscv.h"

#define RISCV_BIN_LOAD_ADDR 0x0c600000
#define RISCV_BIN_MMC_ADDR 0xbb800000
#define RISCV_BIN_NAND_ADDR 0x06900000
#define RISCV_BIN_SIZE 300*1024

#define SENSOR_BIN_LOAD_ADDR 0x0c6a0000
#define SENSOR_BIN_MMC_ADDR 0xbb880000
#define SENSOR_BIN_NAND_ADDR 0x06a00000
#define	SENSOR_BIN_SIZE 201*1024
#define LEP_RISCV_RESET_ENTRY       (RISCV_BIN_LOAD_ADDR + 0x80)

static uint32_t ccu_readl(uint32_t off)
{
	return readl(CCU_BASE + off);
}

static void ccu_writel(uint32_t value, uint32_t off)
{
	writel(value, CCU_BASE + off);
}

static uint32_t riscv_readl(uint32_t off)
{
	return readl(RISCV_BASE + off);
}

static void riscv_writel(uint32_t value, uint32_t off)
{
	writel(value, RISCV_BASE + off);
}

extern u32 mmc_block_read(u32 start, u32 blkcnt, u32 *dst);
extern int sfc_nand_load(unsigned int src_addr, unsigned int count, unsigned int dst_addr);

void spl_nand_load_riscv(void)
{
	sfc_nand_load(RISCV_BIN_NAND_ADDR, RISCV_BIN_SIZE, RISCV_BIN_LOAD_ADDR);
	sfc_nand_load(SENSOR_BIN_NAND_ADDR, SENSOR_BIN_SIZE, SENSOR_BIN_LOAD_ADDR);
}

void spl_mmc_load_riscv(void)
{
	int i = 0;
	for(i=0; i < 132 ;i++){ /*1024*512 = 512k*/
		mmc_block_read(RISCV_BIN_MMC_ADDR/512+i, 1, RISCV_BIN_LOAD_ADDR+i*512);
	}
	for(i=0; i < 401 ;i++){ /*1024*512 = 512k*/
		mmc_block_read(SENSOR_BIN_MMC_ADDR/512+i, 1, SENSOR_BIN_LOAD_ADDR+i*0x200);
	}
}

void spl_start_riscv(void)
{
	unsigned int val = 0;

	val = riscv_readl(CCU_CCSR);
	val &= ~(1 << 1);
	riscv_writel(val, CCU_CCSR);

	/*XBURST2_LEP_STOP*/
	val = ccu_readl(XBURST2_CCU_CFCR);
	val |= (1 << 31);
	ccu_writel(val, XBURST2_CCU_CFCR);

	// Reset value
	riscv_writel(0x43, CCU_CCSR);
	riscv_writel(LEP_RISCV_RESET_ENTRY, CCU_CRER);
	riscv_writel(0, CCU_FROM_HOST);
	riscv_writel(0, CCU_TO_HOST     );
	riscv_writel(0, CCU_INTC_MASK_L );
	riscv_writel(0, CCU_INTC_MASK_H );
	riscv_writel(0, CCU_TIME_L      );
	riscv_writel(0, CCU_TIME_H      );
	riscv_writel(-1, CCU_TIME_CMP_L  );
	riscv_writel(-1, CCU_TIME_CMP_H  );
	riscv_writel(0x1e01, CCU_PMA_CFG0    );
	riscv_writel(0x2F03, CCU_PMA_CFG1    );
	riscv_writel(0x1f01, CCU_PMA_CFG2    );
	riscv_writel(0x1e01, CCU_PMA_CFG3    );
	riscv_writel(0x04000000, CCU_PMA_ADR0    );
	riscv_writel(0x05ffffff, CCU_PMA_ADR1    );
	riscv_writel(0x1fffffff, CCU_PMA_ADR2    );
	riscv_writel(0x00000000, CCU_PMA_ADR3    );

	/*XBURST2_LEP_RUN*/
	val = ccu_readl(XBURST2_CCU_CFCR);
	val &= ~(1 << 31);
	ccu_writel(val, XBURST2_CCU_CFCR);
}
