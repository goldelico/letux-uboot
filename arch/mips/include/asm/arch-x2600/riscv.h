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

#define CCU_PMA_ADR(x)                (0x40 + x* 4)
#define CCU_PMA_CFG(x)                (0x60 + x* 4)

#define CACHE_EN		5
#define ADDR_TYPE_OFF		0x0
#define ADDR_TYPE_TOR		0x1
#define ADDR_TYPE_NA4		0x2
#define ADDR_TYPE_NAPOT		0x3
#define ADDR_TYPE		3
#define WRITE_EN		2
#define READ_EN			1
#define EXECUTE_EN		0

void do_start_riscv(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

#endif
