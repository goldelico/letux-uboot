#ifndef _SPL_RTOS_ARGUMENT_H_
#define _SPL_RTOS_ARGUMENT_H_

#include <common.h>

typedef enum {
    SPL_RTOS_TYPE_LOAD_OS       = 1,
    SPL_RTOS_TYPE_BOOT_OS       = 2,  /* 暂未实现RTOS引导OS的参数传递 */
} spl_rtos_boot_type;

/* 与RTOS中的结构体保持一致 */
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
    unsigned int magic;         /* "ARGS" */
    char boot_type;             /* =0:RTOS只加载镜像不引导, =1:RTOS完成加载和引导OS功能 */
    char is_loading;            /* =0:OS镜像加载中, =1:OS镜像加载完成 */
    char *command_line;
    unsigned int offset_sector;/* OS 偏移地址 单位:Sector 512Byte */
    unsigned int size_sector;  /* OS 偏移地址 单位:Sector 512Byte */
    void *spl_image_info;      /* RTOS 传递信息回SPL(OS image entery) RTOS中给该变量赋值 */
};

struct spl_rtos_argument {
    struct card_info_params *card_params;
    struct rtos_boot_os_args *os_boot_args;
};

#endif /* _SPL_RTOS_ARGUMENT_H_ */
