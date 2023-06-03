#include <common.h>
#include <config.h>
#include <spl.h>
#include <asm/io.h>
#include <errno.h>
#include <linux/err.h>
#include <malloc.h>
#include <div64.h>
#include <asm/arch/cpm.h>
#include "spl_ota_jzsd.h"
#include "spl_gpt_partition.h"

struct nv_flags {
    unsigned int version;
    unsigned int boot;
    unsigned int step;
    unsigned int start;
    unsigned int finish;
	unsigned int needfullpkg;
};

static struct jzsd_ota_ops *ota_ops = NULL;
void register_jzsd_ota_ops(struct jzsd_ota_ops *ops)
{
	ota_ops = ops;
}

static void nv_read(unsigned int start, unsigned int blkcnt, unsigned int *dst)
{
	ota_ops->jzsd_read(start, blkcnt, dst);
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

char* spl_jzsd_ota_load_image(void)
{
	char *cmdargs = NULL;
	unsigned int nvdata[512]={0};
	unsigned int start_sector;
	int ret;

	ret = spl_get_built_in_gpt_partition("nv", &start_sector, NULL);
	if (ret) {
		printf("mmc: failed to get partition nv\n");
		return NULL;
	}
	nv_read(start_sector, 1, nvdata);
#if 0
	int i;
	for(i=0;i<512;i++){
		printf("%x ",nvdata[i]);
		if(!(i%8))
			printf("\n");
	}
#endif
	printf("NV FLAGS:\n nv.boot \t%x\n nv.step \t%x\n nv.start \t%x\n nv.end \t%x\n nv.needfullpkg \t%x\n",
			((struct nv_flags*)nvdata)->boot, ((struct nv_flags*)nvdata)->step, ((struct nv_flags*)nvdata)->start, ((struct nv_flags*)nvdata)->finish, ((struct nv_flags*)nvdata)->needfullpkg);

	if(get_signature(RECOVERY_SIGNATURE) || (((struct nv_flags*)nvdata)->start == 0x5a5a5a5a)) {
		if(((struct nv_flags*)nvdata)->boot) {
			ota_ops->jzsd_load_img_from_partition("recovery");
			cmdargs = CONFIG_SYS_SPL_OTA_ARGS_ADDR;
		} else {
			ota_ops->jzsd_load_img_from_partition("kernel");
			cmdargs = CONFIG_SYS_SPL_ARGS_ADDR;
		}
	} else {
		ota_ops->jzsd_load_img_from_partition("kernel");
		cmdargs = CONFIG_SYS_SPL_ARGS_ADDR;
	}

	return cmdargs;
}
