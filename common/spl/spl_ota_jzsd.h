#ifndef __SPL_OTA_JZSD__
#define __SPL_OTA_JZSD__

struct jzsd_ota_ops {
	u32 (*jzsd_read)(u32 start, u32 blkcnt, u32 *dst);
	int (*jzsd_load_img_from_partition)(const char *name);
};

char* spl_jzsd_ota_load_image(void);
void register_jzsd_ota_ops(struct jzsd_ota_ops *ops);

#endif	/* __SPL_OTA_JZSD__ */
