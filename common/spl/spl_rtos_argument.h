#ifndef _SPL_RTOS_ARGUMENT_H_
#define _SPL_RTOS_ARGUMENT_H_

#include <common.h>

/* SPL与RTOS中的结构体保持一致, mmc 专用 */
struct card_info_params {
    unsigned char ext_csd[512]; /* 64字节 cache line对齐读取速度更块 */
    unsigned int magic;         /* "RTOS" */
    unsigned int version;       /* 结构体版本 */
    int type;                   /* =0 mmc, =1 SD, =2 SDIO, =3 SD_COMBO  */
    int highcap;                /* =0 <2GB, =1 >2GB */
    int rca;                    /* address << 16 */
    int bus_width;              /* =0 1bit,
                                 * =2 4bit
                                 * =3 8bit
                                 */
    unsigned int host_index;    /* 控制器索引号 */
    unsigned int max_speed;
    unsigned int raw_cid[4];
    unsigned int raw_csd[4];
};

/*
 * SPL传递参数到RTOS， 可在RTOS中load OS镜像
 */
struct rtos_boot_os_args {
    unsigned int magic; /* "ARGS" */
    unsigned int load_addr;
    unsigned int offset; /* OS 偏移地址 单位:Byte */
    unsigned int size; /* OS 大小 单位:Byte */
    unsigned int entry_point;
    char *cmdargs;
};

struct spl_rtos_argument {
    struct rtos_boot_os_args *os_boot_args;
    struct card_info_params *card_params;
};

#endif /* _SPL_RTOS_ARGUMENT_H_ */
