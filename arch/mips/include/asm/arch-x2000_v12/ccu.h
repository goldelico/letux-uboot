#ifndef _CCU_H_
#define _CCU_H_

#define CCU_IO_BASE 0x12200000

#define CCU_CCCR 0x0000
#define CCU_CSSR 0x0020
#define CCU_CSRR 0x0040
#define CCU_MSCR 0x0060
#define CCU_MSIR 0x0064
#define CCU_CCR  0x0070
#define CCU_PIPR 0x0100
#define CCU_PIMR 0x0120
#define CCU_MIPR 0x0140
#define CCU_MIMR 0x0160
#define CCU_OIPR 0x0180
#define CCU_OIMR 0x01a0
#define CCU_DIPR 0x01c0
#define CCU_GDIMR 0x01e0
#define CCU_LDIMR(N) (0x0300+(N)*32)
#define CCU_RER  0x0f00
#define CCU_CSLR 0x0fa0
#define CCU_CSAR 0x0fa4
#define CCU_GIMR 0x0fc0
#define CCU_CFCR 0x0fe0
#define CCU_MBR(N) (0x1000+(N)*4)
#define CCU_BCER 0x1f00

#endif