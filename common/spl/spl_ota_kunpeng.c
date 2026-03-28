#include <common.h>
#include <config.h>
#include <spl.h>
#include <asm/io.h>
#include <errno.h>
#include <linux/err.h>
#include <malloc.h>
#include <div64.h>
#include <asm/arch/cpm.h>
#include "spl_ota_kunpeng.h"

#define ENV_DATA_SIZE 1024

typedef struct env_flags {
    unsigned char   flags;                    /* active/obsolete flags    */
    unsigned char   data[ENV_DATA_SIZE];      /* Environment data     */
} env_t;


struct nv_flags {
    unsigned int version;
    unsigned int boot;
    unsigned int step;
    unsigned int start;
    unsigned int finish;
    unsigned int needfullpkg;
    unsigned int rot_angle;
    unsigned int partition;
    unsigned int slave_rot_angle;
    unsigned int reservedspace[13];
    env_t env;
};

static struct ota_ops *ota_ops = NULL;
void register_ota_ops(struct ota_ops *ops)
{
	ota_ops = ops;
}

static void ota_init(void)
{
	if (ota_ops->flash_init)
		ota_ops->flash_init();
}

static void nv_read(unsigned int src, unsigned int dst, unsigned int len)
{
	ota_ops->flash_read(src, len, dst);
}

static int get_signature(const int signature)
{
	unsigned int flag = cpm_get_scrpad();

	//printf("RECOVERY_SIGNATURE: %x\n", flag);
	if ((flag & 0xffff) == signature) {
		/*
		 * Clear the signature,
		 * reset the signature to force into normal boot after factory reset
		 */
		cpm_set_scrpad(flag & ~(0xffff));
		return 1;
	}

	return 0;
}


char* spl_ota_load_image(void)
{
	char *cmdargs = NULL;
	unsigned int addr = 0;
	unsigned int bootimg_addr = 0;
	struct jz_sfcnand_partition_param *partitions;
	struct nv_flags nv;
	int len;
	unsigned short degree;
	unsigned int dec_degree;
	unsigned int hightBits;
	unsigned char *kname;
	ota_init();
	partitions = ota_ops->flash_get_partitions();
	addr = ota_ops->flash_get_part_offset_by_name(partitions, CONFIG_PAT_NV_NAME);
	nv_read(addr, (unsigned int)&nv, sizeof(struct nv_flags));

	printf("NV FLAGS:\n nv.boot \t%x\n nv.step \t%x\n nv.start \t%x\n nv.end \t%x\n nv.needfullpkg \t%x\n nv.rot_angle \t%x\n nv.partition \t%x\n",
			nv.boot, nv.step, nv.start, nv.finish, nv.needfullpkg, nv.rot_angle, nv.partition);

#ifdef CONFIG_OTA_ABUPDATE
    /* AB partition upgrade */

#define SLPC_BASIC_COUNT 0xaa55aa00
#define CHANGE_NUM 6
#define MAX_NUM 12
#define PARTITIONA 0
#define PARTITIONB 0xa5

#ifdef CONFIG_OTA_ABUPDATE_ROLLBACK
	unsigned int rsr_data;
	unsigned int slpc_data;

	/*CPM_SLPC software restart without loss*/
	slpc_data = cpm_readl(CPM_SLPC);
	printf("slpc:%x\n",slpc_data);
	if ((slpc_data >> 8) == (SLPC_BASIC_COUNT >> 8)){
		if ((slpc_data & 0xff) >= CHANGE_NUM) {
			nv.partition = nv.partition == PARTITIONA ? PARTITIONB : PARTITIONA;
			if ((slpc_data & 0xff) >= MAX_NUM) {
				while(1){
					printf("Both partitions A/B failed to start!!! \n");
				}
			}
		}
		cpm_writel(++slpc_data, CPM_SLPC);
	} else {
		cpm_writel(SLPC_BASIC_COUNT, CPM_SLPC);
	}
#else
	cpm_writel(SLPC_BASIC_COUNT, CPM_SLPC);
#endif

	if (nv.partition == PARTITIONB) {
		kname = CONFIG_PATB_KERNEL_NAME;
		cmdargs = CONFIG_SPL_BOOT_PARTITION_B;
		printf("The startup area for this time is partitionB !!! \n");
	} else {
		kname = CONFIG_PATA_KERNEL_NAME;
		cmdargs = CONFIG_SPL_BOOT_PARTITION_A;
		printf("The startup area for this time is partitionA !!! \n");
	}

	bootimg_addr = ota_ops->flash_get_part_offset_by_name(partitions, kname);
	ota_ops->flash_load_kernel(bootimg_addr, kname);

#else
	/* recovery Upgrade method */
	if(get_signature(RECOVERY_SIGNATURE) || (nv.start == 0x5a5a5a5a)) {
		if(nv.boot) {
			bootimg_addr = ota_ops->flash_get_part_offset_by_name(partitions, CONFIG_PAT_RECOVERY_NAME);
			cmdargs = CONFIG_SYS_SPL_OTA_ARGS_ADDR;
		} else {
			bootimg_addr = ota_ops->flash_get_part_offset_by_name(partitions, CONFIG_PAT_KERNEL_NAME);
			cmdargs = CONFIG_SYS_SPL_ARGS_ADDR;
		}
	} else {
		bootimg_addr = ota_ops->flash_get_part_offset_by_name(partitions, CONFIG_PAT_KERNEL_NAME);
		cmdargs = CONFIG_SYS_SPL_ARGS_ADDR;
	}

	ota_ops->flash_load_kernel(bootimg_addr, CONFIG_PAT_KERNEL_NAME);
#endif

#if defined(USE_NV_CMDARGS) && defined(CONFIG_NV_ROTATE)
#error "Both CONFIG_NV_ROTATE and USE_NV_CMDARGS are defined. This is not allowed."
#endif

#ifdef CONFIG_NV_ROTATE
    {
        static char buffer[512];
        hightBits = nv.rot_angle & 0xFFFF0000;
        if (hightBits == 0xEEEE0000) {
            degree = nv.rot_angle & 0x0000FFFF;
            dec_degree = (int)degree;
            if (dec_degree == 0 || dec_degree == 90 || dec_degree == 180 || dec_degree == 270) {
                len = snprintf(buffer, sizeof(buffer), "%s rot_angle=%d", cmdargs, dec_degree);
                if (len >= sizeof(buffer)) {
                    printf("nv rot_angle failed!\n", len);
                } else {
                    cmdargs = buffer;
                }
            }
        }
    }
#elif defined(USE_NV_CMDARGS)
#define ENV_FLAG 0xA5
    static env_t env;
    env_t *env_p = &nv.env;
    if (nv.env.flags == ENV_FLAG) {
        if (nv.env.data[0]!=0 && nv.env.data[0]!=0xff) {
            memcpy(&env, env_p, sizeof(env_t));
            cmdargs = env.data;
            printf("use env new cmdargs\n");
        } else {
            printf("env cmdargs error,use default old cmdargs\n");
        }
    }
#endif

    return cmdargs;
}
