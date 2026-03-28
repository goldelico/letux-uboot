#ifndef RESERVED_PART_H
#define RESERVED_PART_H

#include <asm/arch/sfc.h>

enum {
	SFC_NAND_FLASH = 0,
	SFC_NOR_FLASH
};

enum {
	RESERVED_PART_RW = 0,
	RESERVED_PART_R_ONLY,
};

struct reserved_partition{
	int index;
	const char *name;
	u32 start;
	u32 end;
	u32 rw_mode;
	u32 reserved;
	struct list_head list;
};

struct reserved_part_manager{
	struct list_head reserved_part_list_head;
	struct sfc_flash *flash_owner;
	struct list_head list;
};

struct sfc_flash_partition {
	char name[32];		/* identifier string */
	uint32_t size;          /* partition size */
	uint32_t offset;        /* offset within the master MTD space */
	uint32_t mask_flags;    /* master MTD flags to mask out for this partition */
	uint32_t manager_mode;  /* manager_mode mtd or ubi */
};

#define RESERVED_PART_SYMBOL_RO 0x30000000

static struct reserved_part_manager *get_reserved_part_mgr(struct sfc_flash *flash);
int scan_reserved_part(struct sfc_flash *flash, void *partitions, int nr_parts);
int add_reserved_part(struct sfc_flash *flash, int index, void *partitions, int flash_type);
int reserved_part_init(struct sfc_flash *flash);
static struct reserved_partition *offset_2_reserved_part(struct reserved_part_manager *reserved_part_mgr, uint32_t addr, uint32_t len);
bool check_partition_is_writable(struct sfc_flash *flash, uint32_t to, uint32_t len);
int flash_erase_skip_reserved_part(struct sfc_flash *flash, void *partitions, int nr_parts);

#endif /* RESERVED_PART_H */
