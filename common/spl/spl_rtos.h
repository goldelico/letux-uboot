#ifndef _SPL_RTOS_H_
#define _SPL_RTOS_H_

#include <common.h>

struct rtos_header {
	unsigned int code[2];
	unsigned int tag;
	unsigned int version;
	unsigned long img_start;
	unsigned long img_end;
	unsigned long heap_start;
	unsigned long heap_end;
	unsigned long mapped_rtosdata_size;
};

extern void flush_cache_all(void);

static inline int rtos_check_header(struct rtos_header *rtos)
{
#ifdef CONFIG_JZ_SECURE_SUPPORT
		int sec = is_security_boot();
		if(sec){
			if(rtos->tag != 0x52544f53){
				printf("sec rtos bad tag: %x\n", rtos->tag);
				return -1;
			}
		}else if(rtos->tag != 0x534f5452){
			printf("rtos bad tag: %x\n", rtos->tag);
			return -1;
		}
#else
	if (rtos->tag != 0x534f5452) {
		printf("rtos bad tag: %x\n", rtos->tag);
		return -1;
	}
#endif

	if (rtos->img_start >= rtos->img_end) {
		printf("rtos bad off: %lx %lx\n", rtos->img_start, rtos->img_end);
		return -1;
	}

	return 0;
}

static inline void rtos_raw_start(struct rtos_header *rtos, void *arg)
{
#ifndef CONFIG_RTOS_CAN_RETURN
	__attribute__ ((noreturn))
#endif
	void (*func)(void *arg) = (void *)rtos->img_start;
	func(arg);
}

static inline void rtos_start(struct rtos_header *rtos, void *arg)
{
	void (*func)(void *arg) = (void *)rtos->img_start;
	flush_cache_all();
	func(arg);
}

struct rtos_ota_header {
	unsigned int tag0;
	unsigned int boot_id;
	unsigned int os0_state;
	unsigned int os1_state;
	unsigned int tag1;
	unsigned int reserve[4];
};

static inline int rtos_ota_check_header(struct rtos_ota_header *ota)
{
	if (ota->tag0 != 0x534f5452) {
		debug("ota bad tag0: %x\n", ota->tag0);
		return 0;
	}

	if (ota->tag1 != 0x3041544f) {
		debug("ota bad tag1: %x\n", ota->tag1);
		return 0;
	}

	if (ota->boot_id == 0) {
		if (ota->os0_state != 0) {
			printf("ota state 0 not match: %x\n", ota->os0_state);
			return -1;
		}
	}

	if (ota->boot_id == 1) {
		if (ota->os1_state != 0) {
			printf("ota state 1 not match: %x\n", ota->os1_state);
			return -1;
		}
	}

	return ota->boot_id;
}


#endif /* _SPL_RTOS_H_ */
