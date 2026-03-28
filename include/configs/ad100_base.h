#ifndef __AD100_BASE_H__
#define	__AD100_BASE_H__

#define CONFIG_ROOTFS_SQUASHFS
#define CONFIG_ROOTFS2_SQUASHFS
#define CONFIG_ARG_QUIET
#define CONFIG_SPL_SERIAL_SUPPORT

#include "ad100_base_common.h"
#ifdef CONFIG_AD_SLT
#include "ad100_slt_common.h"
#endif

#endif /* __AD100_BASE_H__ */
