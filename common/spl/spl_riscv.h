#ifndef __SPL_RISCV_H
#define __SPL_RISCV_H

#include <linux/types.h>

#define XBURST2_CCU_CFCR            (0x0fe0)

#define CCU_CCSR                    (0x0 * 4)
#define CCU_CRER                    (0x1 * 4)
#define CCU_FROM_HOST               (0x2 * 4)
#define CCU_TO_HOST                 (0x3 * 4)
#define CCU_TIME_L                  (0x4 * 4)
#define CCU_TIME_H                  (0x5 * 4)
#define CCU_TIME_CMP_L              (0x6 * 4)
#define CCU_TIME_CMP_H              (0x7 * 4)
#define CCU_INTC_MASK_L             (0x8 * 4)
#define CCU_INTC_MASK_H             (0x9 * 4)
#define CCU_INTC_PEND_L             (0xa * 4)
#define CCU_INTC_PEND_H             (0xb * 4)

#define CCU_PMA_ADR0                (0x10 * 4)
#define CCU_PMA_ADR1                (0x11 * 4)
#define CCU_PMA_ADR2                (0x12 * 4)
#define CCU_PMA_ADR3                (0x13 * 4)
#define CCU_PMA_CFG0                (0x18 * 4)
#define CCU_PMA_CFG1                (0x19 * 4)
#define CCU_PMA_CFG2                (0x1a * 4)
#define CCU_PMA_CFG3                (0x1b * 4)


void spl_mmc_load_riscv(void);
void spl_nand_load_riscv(void);
void spl_start_riscv(void);

#endif
