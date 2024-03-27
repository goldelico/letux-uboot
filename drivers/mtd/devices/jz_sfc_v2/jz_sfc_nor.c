#include <common.h>
#include <malloc.h>
#include <ubi_uboot.h>
#include <asm/io.h>

#include <asm/arch/sfc.h>
#include <asm/arch/spinor.h>
#include "jz_sfc_common.h"


struct sfc_flash *flash = NULL;
struct burner_params params;

struct mini_spi_nor_info mini_params;

#ifdef CONFIG_BURNER
unsigned int burn_mode = 0;
#endif

//#define SFC_NOR_CLONER_DEBUG
//#define SFC_REG_DEBUG

#define MULTI_DIE_FLASH_NUM 1

#define ACTIVE_DIE(addr)					\
({								\
	uint8_t die_id = addr >> flash->die_shift;		\
	if (die_id != flash->current_die_id) {			\
		sfc_active_die(die_id);				\
		flash->current_die_id = die_id;			\
	}							\
	if (die_id)						\
		addr = addr & ((1 << flash->die_shift) - 1);	\
	addr;							\
})								\

static int sfc_die_select(uint8_t die_id)
{
	struct sfc_cdt_xfer xfer;
	memset(&xfer, 0, sizeof(xfer));

	/* set Index */
	xfer.cmd_index = NOR_DIE_SELECT;

	/* set addr */
	xfer.rowaddr = 0;
	xfer.columnaddr = 0;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = 1;
	xfer.config.data_dir = GLB_TRAN_DIR_WRITE;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = &die_id;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	return 0;
}

static int sfc_read_active_die_id(uint8_t *value)
{
	struct sfc_cdt_xfer xfer;
	memset(&xfer, 0, sizeof(xfer));

	/* set Index */
	xfer.cmd_index = NOR_READ_ACTIVE_DIE_ID;

	/* set addr */
	xfer.rowaddr = 0;
	xfer.columnaddr = 0;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = 1;
	xfer.config.data_dir = GLB_TRAN_DIR_READ;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = value;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	return 0;
}

static int sfc_active_die(uint8_t die_id)
{
	uint8_t die_id_read;

	sfc_die_select(die_id);
	do {
		sfc_read_active_die_id(&die_id_read);
	}while(die_id != die_id_read);

	return 0;
}


int sfc_nor_reset(void)
{
	struct sfc_cdt_xfer xfer;

	memset(&xfer, 0, sizeof(xfer));

	/* set Index */
	xfer.cmd_index = NOR_RESET_ENABLE;

	/* set addr */
	xfer.rowaddr = 0;
	xfer.columnaddr = 0;

	/* set transfer config */
	xfer.dataen = DISABLE;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	udelay(100);
	return 0;
}

unsigned int sfc_nor_read_id(void)
{
	struct sfc_cdt_xfer xfer;
	unsigned char buf[3];
	unsigned int chip_id = 0;

	memset(&xfer, 0, sizeof(xfer));

	/* set Index */
	xfer.cmd_index = NOR_READ_ID;

	/* set addr */
	xfer.rowaddr = 0;
	xfer.columnaddr = 0;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = 3;
	xfer.config.data_dir = GLB_TRAN_DIR_READ;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = buf;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	chip_id = ((buf[0] & 0xff) << 16) | ((buf[1] & 0xff) << 8) | (buf[2] & 0xff);
	return chip_id;
}

unsigned int get_norflash_id(void)
{
	unsigned int id = sfc_nor_read_id();
	return id;
}


static unsigned int sfc_nor_read_params(unsigned int addr, unsigned char *buf, unsigned int len)
{
	struct sfc_cdt_xfer xfer;
	memset(&xfer, 0, sizeof(xfer));

	/* set Index */
	xfer.cmd_index = NOR_READ_STANDARD;

	/* set addr */
	xfer.columnaddr = 0;
	xfer.rowaddr = addr;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = len;
	xfer.config.data_dir = GLB_TRAN_DIR_READ;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = buf;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	return xfer.config.cur_len;
}

static unsigned int sfc_do_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
	struct sfc_cdt_xfer xfer;
	memset(&xfer, 0, sizeof(xfer));

	/* set Index */
	if (flash->quad_succeed) {
		xfer.cmd_index = NOR_READ_QUAD;
	} else {
		xfer.cmd_index = NOR_READ_STANDARD;
	}

	/* active die */
	if (flash->die_num > 1)
		addr = ACTIVE_DIE(addr);

	/* set addr */
	xfer.columnaddr = 0;
	xfer.rowaddr = addr;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = len;
	xfer.config.data_dir = GLB_TRAN_DIR_READ;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = buf;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	return xfer.config.cur_len;
}

static unsigned  int sfc_do_write(unsigned int addr, unsigned int len, unsigned char *buf)
{
	struct sfc_cdt_xfer xfer;
	memset(&xfer, 0, sizeof(xfer));

	/* set Index */
	if (flash->quad_succeed) {
		xfer.cmd_index = NOR_WRITE_QUAD_ENABLE;
	} else {
		xfer.cmd_index = NOR_WRITE_STANDARD_ENABLE;
	}

	/* active die */
	if (flash->die_num > 1)
		addr = ACTIVE_DIE(addr);

	/* set addr */
	xfer.columnaddr = 0;
	xfer.rowaddr = addr;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = len;
	xfer.config.data_dir = GLB_TRAN_DIR_WRITE;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = (uint8_t *)buf;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	return xfer.config.cur_len;
}

static int sfc_do_erase(uint32_t addr)
{
	struct sfc_cdt_xfer xfer;

	memset(&xfer, 0, sizeof(xfer));

	/* set Index */
	xfer.cmd_index = NOR_ERASE_WRITE_ENABLE;

	/* active die */
	if (flash->die_num > 1)
		addr = ACTIVE_DIE(addr);

	/* set addr */
	xfer.rowaddr = addr;

	/* set transfer config */
	xfer.dataen = DISABLE;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	return 0;
}

static int sfc_read(unsigned int from, unsigned int len, unsigned char *buf)
{
	int tmp_len = 0, current_len = 0;

	while((int)len > 0) {
		tmp_len = sfc_do_read((unsigned int)from + current_len, &buf[current_len], len);
		current_len += tmp_len;
		len -= tmp_len;
	}

	return current_len;
}


int sfc_nor_read(unsigned int from, unsigned int len, unsigned char *buf)
{

#ifndef CONFIG_BURNER
	int i;
	if(flash->norflash_partitions->num_partition_info && (flash->norflash_partitions->num_partition_info != 0xffffffff)) {
		for(i = 0; i < flash->norflash_partitions->num_partition_info; i++){
			if(from >= flash->norflash_partitions->nor_partition[i].offset && \
					from < (flash->norflash_partitions->nor_partition[i].offset + \
						flash->norflash_partitions->nor_partition[i].size) && \
					(flash->norflash_partitions->nor_partition[i].mask_flags & NORFLASH_PART_WO)){
				printf("the partiton can't read,please check the partition RW mode\n");
				return 0;
			}
		}
	}
#endif

	sfc_read(from, len, buf);
	return 0;
}

int sfc_nor_page_write(unsigned int to, unsigned int len, unsigned char *buf)
{
	unsigned int page_offset, actual_len;
	int ret;
	struct spi_nor_info *spi_nor_info;
	int writesize;

	spi_nor_info = flash->g_nor_info;
	writesize = spi_nor_info->page_size;

#ifndef CONFIG_BURNER
	int i;
	if(flash->norflash_partitions->num_partition_info && (flash->norflash_partitions->num_partition_info != 0xffffffff)) {
		for(i = 0; i < flash->norflash_partitions->num_partition_info; i++){
			if(to >= flash->norflash_partitions->nor_partition[i].offset && \
					to < (flash->norflash_partitions->nor_partition[i].offset + \
						flash->norflash_partitions->nor_partition[i].size) && \
					(flash->norflash_partitions->nor_partition[i].mask_flags & NORFLASH_PART_RO)){
				printf("the partiton can't write,please check the partition RW mode\n");
				return 0;
			}
		}
	}
#endif

	page_offset = to & (spi_nor_info->page_size - 1);
	/* do all the bytes fit onto one page? */
	if (page_offset + len <= spi_nor_info->page_size) {
		ret = sfc_do_write(to, len, buf);
	} else {
		u32 i;

		/* the size of data remaining on the first page */
		actual_len = spi_nor_info->page_size - page_offset;
		ret = sfc_do_write(to, actual_len, buf);

		/* write everything in flash->page_size chunks */
		for (i = actual_len; i < len; i += writesize) {
			actual_len = len - i;
			if (actual_len >= writesize)
				actual_len = writesize;
			ret = sfc_do_write(to + i, actual_len, buf + i);
		}
	}
	return ret;

}


#ifdef CONFIG_BURNER

static struct legacy_params g_legacy_params;
struct legacy_params *params_compatibility()
{
	int val, mask, bit_shift;
	struct legacy_params *p = &g_legacy_params;

	memset(p, 0, sizeof(struct legacy_params));

	p->magic = params.magic;
	p->version = 1;

	memcpy(p->nor_params.name, params.spi_nor_info.name, sizeof(SIZEOF_NAME));
	p->nor_params.pagesize = params.spi_nor_info.page_size;
	p->nor_params.sectorsize = 4096;
	p->nor_params.chipsize = params.spi_nor_info.chip_size;
	p->nor_params.erasesize = params.spi_nor_info.erase_size;
	p->nor_params.id = params.spi_nor_info.id;

	if (p->nor_params.chipsize > 0x1000000 ) {
		p->nor_params.addrsize = 4;
	} else {
		p->nor_params.addrsize = 3;
	}

	p->nor_params.block_info.cmd_blockerase = params.spi_nor_info.sector_erase.cmd;
	p->nor_params.quad_mode.dummy_byte = params.spi_nor_info.read_quad.dummy_byte;
	p->nor_params.quad_mode.RDSR_CMD = params.spi_nor_info.quad_get.cmd;
	p->nor_params.quad_mode.WRSR_CMD = params.spi_nor_info.quad_set.cmd;

	val = params.spi_nor_info.quad_get.val;
	mask = params.spi_nor_info.quad_get.mask;
	bit_shift = params.spi_nor_info.quad_get.bit_shift;
	p->nor_params.quad_mode.RDSR_DATA = (val & mask) << bit_shift;
	p->nor_params.quad_mode.RD_DATA_SIZE = params.spi_nor_info.quad_set.bit_shift / 8 + 1;

	val = params.spi_nor_info.quad_set.val;
	mask = params.spi_nor_info.quad_set.mask;
	bit_shift = params.spi_nor_info.quad_set.bit_shift;
	p->nor_params.quad_mode.WRSR_DATA = (val & mask) << bit_shift;
	p->nor_params.quad_mode.WD_DATA_SIZE = params.spi_nor_info.quad_set.bit_shift / 8 + 1;

	p->nor_params.quad_mode.cmd_read = params.spi_nor_info.read_quad.cmd;
	p->nor_params.quad_mode.sfc_mode = params.spi_nor_info.read_quad.transfer_mode;

	memcpy(&p->norflash_partitions, &params.norflash_partitions, sizeof(struct norflash_partitions));

	return p;
}
#endif


int sfc_nor_write(unsigned int to, unsigned int len, unsigned char *buf)
{
	sfc_nor_page_write(to, len, buf);

	return 0;
}

int sfc_nor_erase(unsigned int addr, unsigned int len)
{
	int ret;
	uint32_t end;
	int erasesize;

	struct spi_nor_info *spi_nor_info;

	spi_nor_info = flash->g_nor_info;
	erasesize = spi_nor_info->erase_size;

#ifndef CONFIG_BURNER
	int i;
	if(flash->norflash_partitions->num_partition_info && (flash->norflash_partitions->num_partition_info != 0xffffffff)) {
		for(i = 0; i < flash->norflash_partitions->num_partition_info; i++){
			if(addr >= flash->norflash_partitions->nor_partition[i].offset && \
					addr < (flash->norflash_partitions->nor_partition[i].offset + \
						flash->norflash_partitions->nor_partition[i].size) && \
					(flash->norflash_partitions->nor_partition[i].mask_flags & NORFLASH_PART_RO)){
				printf("the partiton can't erase,please check the partition RW mode\n");
				return 0;
			}
		}
	}
#endif
	if(len % erasesize != 0){
		len = len - (len % erasesize) + erasesize;
	}

	end = addr + len;
	while (addr < end){
		ret = sfc_do_erase(addr);
		if (ret) {
			printf("erase error !\n");
			return ret;
		}
		addr += erasesize;
	}

	return 0;
}

static struct multi_die_flash die_flash[MULTI_DIE_FLASH_NUM] = {
	[0] = {0xc84019, 2, "GD25S512MD"},
};

void sfc_nor_do_special_func(void)
{
	int tchsh;
	int tslch;
	int tshsl_rd;
	int tshsl_wr;
	int i;
	struct spi_nor_info *spi_nor_info;

	spi_nor_info = flash->g_nor_info;

	tchsh = spi_nor_info->tCHSH;
	tslch = spi_nor_info->tSLCH;
	tshsl_rd = spi_nor_info->tSHSL_RD;
	tshsl_wr = spi_nor_info->tSHSL_WR;
	set_flash_timing(flash->sfc, tchsh, tslch, tshsl_rd, tshsl_wr);

	sfc_nor_get_special_ops(flash);

	flash->quad_succeed = 0;
#ifndef CONFIG_BURNER
	if (params.uk_quad) {
#else
	if (burn_mode) {
#endif
		if (flash->nor_flash_ops->set_quad_mode) {
			flash->nor_flash_ops->set_quad_mode(flash);
		}
		if (flash->quad_succeed)
			printf("nor flash quad mode is set, now use quad mode!\n");
	}

	/* if nor flash size is greater than 16M, use 4byte mode */
	if(spi_nor_info->chip_size > 0x1000000) {
		if (flash->nor_flash_ops->set_4byte_mode) {
			flash->nor_flash_ops->set_4byte_mode(flash);
		}
	}

	/* Multi Die support */
	flash->die_num = 1;
	for (i = 0; i < MULTI_DIE_FLASH_NUM; i++) {
		if(!(strcmp(die_flash[i].flash_name, spi_nor_info->name))) {
			flash->die_num = die_flash[i].die_num;
			uint32_t die_size = spi_nor_info->chip_size / flash->die_num;

			flash->die_shift = ffs(die_size) - 1;
			flash->current_die_id = 0;

			printf("Flash :%s support multi die, die number:%d\n", die_flash[i].flash_name, die_flash[i].die_num);
			break;
		}
	}
}

/*
 *MK_CMD(cdt, cmd, LINK, ADDRMODE, DATA_EN)
 *MK_ST(cdt, st, LINK, ADDRMODE, ADDR_WIDTH, POLL_EN, DATA_EN, TRAN_MODE)
 */
static inline void params_to_cdt(struct spi_nor_info *params, struct sfc_cdt *cdt)
{
	/* 4.nor singleRead */
	MK_CMD(cdt[NOR_READ_STANDARD], params->read_standard, 0, ROW_ADDR, ENABLE);

	/* 5.nor quadRead */
	MK_CMD(cdt[NOR_READ_QUAD], params->read_quad, 0, ROW_ADDR, ENABLE);

	/* 6. nor writeStandard */
	MK_CMD(cdt[NOR_WRITE_STANDARD_ENABLE], params->wr_en, 1, DEFAULT_ADDRMODE, DISABLE);
	MK_CMD(cdt[NOR_WRITE_STANDARD], params->write_standard, 1, ROW_ADDR, ENABLE);
	MK_ST(cdt[NOR_WRITE_STANDARD_FINISH], params->busy, 0, DEFAULT_ADDRMODE, 0, ENABLE, DISABLE, TM_STD_SPI);

	/* 7. nor writeQuad */
	MK_CMD(cdt[NOR_WRITE_QUAD_ENABLE], params->wr_en, 1, DEFAULT_ADDRMODE, DISABLE);
	MK_CMD(cdt[NOR_WRITE_QUAD], params->write_quad, 1, ROW_ADDR, ENABLE);
	MK_ST(cdt[NOR_WRITE_QUAD_FINISH], params->busy, 0, DEFAULT_ADDRMODE, 0, ENABLE, DISABLE, TM_STD_SPI);

	/* 8. nor erase */
	MK_CMD(cdt[NOR_ERASE_WRITE_ENABLE], params->wr_en, 1, DEFAULT_ADDRMODE, DISABLE);
	MK_CMD(cdt[NOR_ERASE], params->sector_erase, 1, ROW_ADDR, DISABLE);
	MK_ST(cdt[NOR_ERASE_FINISH], params->busy, 0, DEFAULT_ADDRMODE, 0, ENABLE, DISABLE, TM_STD_SPI);

	/* 9. quad mode */
	if(params->quad_ops_mode){
		MK_CMD(cdt[NOR_QUAD_SET_ENABLE], params->wr_en, 1, DEFAULT_ADDRMODE, DISABLE);
		MK_ST(cdt[NOR_QUAD_SET], params->quad_set, 1, DEFAULT_ADDRMODE, 0, DISABLE, ENABLE, TM_STD_SPI);  //disable poll, enable data

		MK_ST(cdt[NOR_QUAD_FINISH], params->busy, 1, DEFAULT_ADDRMODE, 0, ENABLE, DISABLE, TM_STD_SPI);

		MK_ST(cdt[NOR_QUAD_GET], params->quad_get, 0, DEFAULT_ADDRMODE, 0, ENABLE, DISABLE, TM_STD_SPI);
	}

	/* 10. nor write ENABLE */
	MK_CMD(cdt[NOR_WRITE_ENABLE], params->wr_en, 0, DEFAULT_ADDRMODE, DISABLE);

	/* 11. entry 4byte mode */
	MK_CMD(cdt[NOR_EN_4BYTE], params->en4byte, 0, DEFAULT_ADDRMODE, DISABLE);

	/* 12. chip erase */
	MK_CMD(cdt[NOR_CHIP_ERASE_WRITE_ENABLE], params->wr_en, 1, DEFAULT_ADDRMODE, DISABLE);
	cdt[NOR_CHIP_ERASE].link = CMD_LINK(1, DEFAULT_ADDRMODE, TM_STD_SPI);
	cdt[NOR_CHIP_ERASE].xfer = CMD_XFER(0, DISABLE, 0, DISABLE, params->chip_erase_cmd);
	cdt[NOR_CHIP_ERASE].staExp = 0;
	cdt[NOR_CHIP_ERASE].staMsk = 0;
	MK_ST(cdt[NOR_CHIP_ERASE_FINISH], params->busy, 0, DEFAULT_ADDRMODE, 0, ENABLE, DISABLE, TM_STD_SPI);

	/* 13. die select */
	cdt[NOR_DIE_SELECT].link = CMD_LINK(0, DEFAULT_ADDRMODE, TM_STD_SPI);
	cdt[NOR_DIE_SELECT].xfer = CMD_XFER(0, DISABLE, 0, ENABLE, SPINOR_OP_DIE_SEL);
	cdt[NOR_DIE_SELECT].staExp = 0;
	cdt[NOR_DIE_SELECT].staMsk = 0;

	/* 14. read active die ID */
	cdt[NOR_READ_ACTIVE_DIE_ID].link = CMD_LINK(0, DEFAULT_ADDRMODE, TM_STD_SPI);
	cdt[NOR_READ_ACTIVE_DIE_ID].xfer = CMD_XFER(0, DISABLE, 0, ENABLE, SPINOR_OP_READ_DIE_ID);
	cdt[NOR_READ_ACTIVE_DIE_ID].staExp = 0;
	cdt[NOR_READ_ACTIVE_DIE_ID].staMsk = 0;

}

static inline void create_cdt_table(struct sfc_flash *flash, uint32_t flag)
{
	struct spi_nor_info *nor_flash_info;
	struct sfc_cdt cdt[INDEX_MAX_NUM];

	memset(cdt, 0, sizeof(cdt));

	/* 1.nor reset */
	cdt[NOR_RESET_ENABLE].link = CMD_LINK(1, DEFAULT_ADDRMODE, TM_STD_SPI);
	cdt[NOR_RESET_ENABLE].xfer = CMD_XFER(0, DISABLE, 0, DISABLE, SPINOR_OP_RSTEN);
	cdt[NOR_RESET_ENABLE].staExp = 0;
	cdt[NOR_RESET_ENABLE].staMsk = 0;

	cdt[NOR_RESET].link = CMD_LINK(0, DEFAULT_ADDRMODE, TM_STD_SPI);
	cdt[NOR_RESET].xfer = CMD_XFER(0, DISABLE, 0, DISABLE, SPINOR_OP_RST);
	cdt[NOR_RESET].staExp = 0;
	cdt[NOR_RESET].staMsk = 0;


	/* 2.nor read id */
	cdt[NOR_READ_ID].link = CMD_LINK(0, DEFAULT_ADDRMODE, TM_STD_SPI);
	cdt[NOR_READ_ID].xfer = CMD_XFER(0, DISABLE, 0, ENABLE, SPINOR_OP_RDID);
	cdt[NOR_READ_ID].staExp = 0;
	cdt[NOR_READ_ID].staMsk = 0;


	/* 3. nor get status */
	cdt[NOR_GET_STATUS].link = CMD_LINK(0, DEFAULT_ADDRMODE, TM_STD_SPI);
	cdt[NOR_GET_STATUS].xfer = CMD_XFER(0, DISABLE, 0, ENABLE, SPINOR_OP_RDSR);
	cdt[NOR_GET_STATUS].staExp = 0;
	cdt[NOR_GET_STATUS].staMsk = 0;

	cdt[NOR_GET_STATUS_1].link = CMD_LINK(0, DEFAULT_ADDRMODE, TM_STD_SPI);
	cdt[NOR_GET_STATUS_1].xfer = CMD_XFER(0, DISABLE, 0, ENABLE, SPINOR_OP_RDSR_1);
	cdt[NOR_GET_STATUS_1].staExp = 0;
	cdt[NOR_GET_STATUS_1].staMsk = 0;

	cdt[NOR_GET_STATUS_2].link = CMD_LINK(0, DEFAULT_ADDRMODE, TM_STD_SPI);
	cdt[NOR_GET_STATUS_2].xfer = CMD_XFER(0, DISABLE, 0, ENABLE, SPINOR_OP_RDSR_2);
	cdt[NOR_GET_STATUS_2].staExp = 0;
	cdt[NOR_GET_STATUS_2].staMsk = 0;

	if(flag == DEFAULT_CDT){
		/* 4.nor singleRead */
		cdt[NOR_READ_STANDARD].link = CMD_LINK(0, ROW_ADDR, TM_STD_SPI);
		cdt[NOR_READ_STANDARD].xfer = CMD_XFER(DEFAULT_ADDRSIZE, DISABLE, 0, ENABLE, SPINOR_OP_READ);
		cdt[NOR_READ_STANDARD].staExp = 0;
		cdt[NOR_READ_STANDARD].staMsk = 0;

		/* first create cdt table */
		write_cdt(flash->sfc, cdt, NOR_RESET_ENABLE, NOR_READ_STANDARD);
	}


	if(flag == UPDATE_CDT){
		nor_flash_info = flash->g_nor_info;
		params_to_cdt(nor_flash_info, cdt);

		/* second create cdt table */
		write_cdt(flash->sfc, cdt, NOR_READ_STANDARD, NOR_READ_ACTIVE_DIE_ID);
	}
#ifdef SFC_REG_DEBUG
	dump_cdt(flash->sfc);
#endif
}

int sfc_nor_flash_init(void)
{
	uint32_t sfc_rate = 200000000;
	flash = malloc(sizeof(struct sfc_flash));
	if (!flash) {
		printf("ERROR: %s %d kzalloc() error !\n",__func__,__LINE__);
		return -1;
	}
	memset(flash, 0, sizeof(struct sfc_flash));

#ifdef CONFIG_SFC_NOR_INIT_RATE
	sfc_rate = CONFIG_SFC_NOR_INIT_RATE;
#endif

	flash->sfc = sfc_res_init(sfc_rate);

	/* try creating default CDT table */
	create_cdt_table(flash, DEFAULT_CDT);

	sfc_nor_reset();

#ifndef CONFIG_BURNER
	set_flash_timing(flash->sfc, DEF_TCHSH, DEF_TSLCH, DEF_TSHSL_R, DEF_TSHSL_W);
	/* Note: make sure the flash parameter are on die0. */
	sfc_nor_read_params(CONFIG_SPIFLASH_PART_OFFSET, (unsigned char *)&params, sizeof(struct burner_params));
	printf("params.magic : 0x%x   params.version : 0x%x\n", params.magic, params.version);
	if((params.magic != NOR_MAGIC) || (params.version != NOR_VERSION)) {
		printf("sfc nor read params error\n");
		return -1;
	}

	flash->g_nor_info = &(params.spi_nor_info);
	flash->norflash_partitions = &params.norflash_partitions;

	/* Update to private CDT table */
	create_cdt_table(flash, UPDATE_CDT);

	/* update sfc rate */
	sfc_clk_set(flash->sfc, CONFIG_SFC_NOR_RATE);

	sfc_nor_do_special_func();

#else
	flash->g_nor_info = malloc(sizeof(struct spi_nor_info));
	flash->norflash_partitions = malloc(sizeof(struct norflash_partitions));
#endif
	return 0;

}

static int sfc_do_chip_erase(void)
{
	struct sfc_cdt_xfer xfer;
	memset(&xfer, 0, sizeof(xfer));

	/* set Index */
	xfer.cmd_index = NOR_CHIP_ERASE_WRITE_ENABLE;

	/* set addr */
	xfer.columnaddr = 0;
	xfer.rowaddr = 0;

	/* set transfer config */
	xfer.dataen = DISABLE;

	if(sfc_sync_cdt(flash->sfc, &xfer)) {
		printf("sfc_sync_cdt error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}
	return 0;

}

int jz_sfc_chip_erase(void)
{
	uint8_t die_id = 0;
	int ret;

	do {
		printf("chip erasing...die%d\n", die_id);

		if (flash->die_num > 1) {
			sfc_active_die(die_id);
			flash->current_die_id = die_id;
		}

		ret = sfc_do_chip_erase();
		if (ret < 0) {
			printf("chip erase error ! %s %s %d\n",__FILE__,__func__,__LINE__);
			return -EIO;
		}

	} while(++die_id < flash->die_num);

	return 0;
}

#ifdef CONFIG_BURNER

struct nor_partition *get_partition_index(u32 offset,u32 length, int *pt_offset, int *pt_size)
{
	int i;
	struct spi_nor_info *spi_nor_info;

	spi_nor_info = flash->g_nor_info;

	for(i = 0; i < flash->norflash_partitions->num_partition_info; i++){
		if(offset >= flash->norflash_partitions->nor_partition[i].offset && \
				(offset + length) <= (flash->norflash_partitions->nor_partition[i].offset + \
					flash->norflash_partitions->nor_partition[i].size)){
			*pt_offset = flash->norflash_partitions->nor_partition[i].offset;
			*pt_size = flash->norflash_partitions->nor_partition[i].size;
			break;
		}else if(offset >= flash->norflash_partitions->nor_partition[i].offset && \
				offset < (spi_nor_info->chip_size) && \
				(flash->norflash_partitions->nor_partition[i].size == 0xffffffff)){ /*size == -1*/
			*pt_offset = flash->norflash_partitions->nor_partition[i].offset;
			*pt_size = spi_nor_info->chip_size - flash->norflash_partitions->nor_partition[i].offset;
			flash->norflash_partitions->nor_partition[i].size = *pt_size;
			break;
		}
	}
	if(i == flash->norflash_partitions->num_partition_info){
		*pt_offset = -1;
		*pt_size = -1;
		printf("partition size not align with write transfer size \n");
		return -1;
	}
	return &flash->norflash_partitions->nor_partition[i];
}

int check_offset(u32 offset,u32 length)
{
	int i;

	for(i = 1; i < flash->norflash_partitions->num_partition_info; i++){
		if(offset < flash->norflash_partitions->nor_partition[i].offset && \
				offset > (flash->norflash_partitions->nor_partition[i-1].offset + \
					flash->norflash_partitions->nor_partition[i-1].size)){
			break;
		}
	}
	if(i >= flash->norflash_partitions->num_partition_info){
		return i;
	}else if((offset+length) > flash->norflash_partitions->nor_partition[i].offset ){
		return -1;
	}else{
		return i;
	};
}

#ifdef SFC_NOR_CLONER_DEBUG
static void dump_cloner_params()
{
	struct spi_nor_info *spi_nor_info;

	spi_nor_info = &params.spi_nor_info;

	printf("name=%s\n", spi_nor_info->name);
	printf("id=0x%x\n", spi_nor_info->id);

	printf("read_standard->cmd=0x%x\n",		spi_nor_info->read_standard.cmd);
	printf("read_standard->dummy=0x%x\n",		spi_nor_info->read_standard.dummy_byte);
	printf("read_standard->addr_nbyte=0x%x\n",	spi_nor_info->read_standard.addr_nbyte);
	printf("read_standard->transfer_mode=0x%x\n",	spi_nor_info->read_standard.transfer_mode);

	printf("read_quad->cmd=0x%x\n",			spi_nor_info->read_quad.cmd);
	printf("read_quad->dummy=0x%x\n",		spi_nor_info->read_quad.dummy_byte);
	printf("read_quad->addr_nbyte=0x%x\n",		spi_nor_info->read_quad.addr_nbyte);
	printf("read_quad->transfer_mode=0x%x\n",	spi_nor_info->read_quad.transfer_mode);

	printf("write_standard->cmd=0x%x\n",		spi_nor_info->write_standard.cmd);
	printf("write_standard->dummy=0x%x\n",		spi_nor_info->write_standard.dummy_byte);
	printf("write_standard->addr_nbyte=0x%x\n",	spi_nor_info->write_standard.addr_nbyte);
	printf("write_standard->transfer_mode=0x%x\n",	spi_nor_info->write_standard.transfer_mode);

	printf("write_quad->cmd=0x%x\n",		spi_nor_info->write_quad.cmd);
	printf("write_quad->dummy=0x%x\n",		spi_nor_info->write_quad.dummy_byte);
	printf("write_quad->addr_nbyte=0x%x\n",		spi_nor_info->write_quad.addr_nbyte);
	printf("write_quad->transfer_mode=0x%x\n",	spi_nor_info->write_quad.transfer_mode);

	printf("sector_erase->cmd=0x%x\n",		spi_nor_info->sector_erase.cmd);
	printf("sector_erase->dummy=0x%x\n",		spi_nor_info->sector_erase.dummy_byte);
	printf("sector_erase->addr_nbyte=0x%x\n",	spi_nor_info->sector_erase.addr_nbyte);
	printf("sector_erase->transfer_mode=0x%x\n",	spi_nor_info->sector_erase.transfer_mode);

	printf("wr_en->cmd=0x%x\n",		spi_nor_info->wr_en.cmd);
	printf("wr_en->dummy=0x%x\n",		spi_nor_info->wr_en.dummy_byte);
	printf("wr_en->addr_nbyte=0x%x\n",	spi_nor_info->wr_en.addr_nbyte);
	printf("wr_en->transfer_mode=0x%x\n",	spi_nor_info->wr_en.transfer_mode);

	printf("en4byte->cmd=0x%x\n",		spi_nor_info->en4byte.cmd);
	printf("en4byte->dummy=0x%x\n",		spi_nor_info->en4byte.dummy_byte);
	printf("en4byte->addr_nbyte=0x%x\n",	spi_nor_info->en4byte.addr_nbyte);
	printf("en4byte->transfer_mode=0x%x\n",	spi_nor_info->en4byte.transfer_mode);

	printf("quad_set->cmd=0x%x\n",		spi_nor_info->quad_set.cmd);
	printf("quad_set->bit_shift=0x%x\n",		spi_nor_info->quad_set.bit_shift);
	printf("quad_set->mask=0x%x\n",		spi_nor_info->quad_set.mask);
	printf("quad_set->val=0x%x\n",		spi_nor_info->quad_set.val);
	printf("quad_set->len=0x%x\n",		spi_nor_info->quad_set.len);
	printf("quad_set->dummy=0x%x\n",	spi_nor_info->quad_set.dummy);

	printf("quad_get->cmd=0x%x\n",		spi_nor_info->quad_get.cmd);
	printf("quad_get->bit_shift=0x%x\n",		spi_nor_info->quad_get.bit_shift);
	printf("quad_get->mask=0x%x\n",		spi_nor_info->quad_get.mask);
	printf("quad_get->val=0x%x\n",		spi_nor_info->quad_get.val);
	printf("quad_get->len=0x%x\n",		spi_nor_info->quad_get.len);
	printf("quad_get->dummy=0x%x\n",	spi_nor_info->quad_get.dummy);

	printf("busy->cmd=0x%x\n",		spi_nor_info->busy.cmd);
	printf("busy->bit_shift=0x%x\n",		spi_nor_info->busy.bit_shift);
	printf("busy->mask=0x%x\n",		spi_nor_info->busy.mask);
	printf("busy->val=0x%x\n",		spi_nor_info->busy.val);
	printf("busy->len=0x%x\n",		spi_nor_info->busy.len);
	printf("busy->dummy=0x%x\n",		spi_nor_info->busy.dummy);

	printf("quad_ops_mode=%d\n",	spi_nor_info->quad_ops_mode);
	printf("addr_ops_mode=%d\n",	spi_nor_info->addr_ops_mode);

	printf("tCHSH=%d\n",	spi_nor_info->tCHSH);
	printf("tSLCH=%d\n",	spi_nor_info->tSLCH);
	printf("tSHSL_RD=%d\n", spi_nor_info->tSHSL_RD);
	printf("tSHSL_WR=%d\n", spi_nor_info->tSHSL_WR);

	printf("chip_size=%d\n",	spi_nor_info->chip_size);
	printf("page_size=%d\n",	spi_nor_info->page_size);
	printf("erase_size=%d\n",	spi_nor_info->erase_size);

	printf("chip_erase_cmd=0x%x\n",	spi_nor_info->chip_erase_cmd);
}
static void dump_mini_cloner_params()
{
	struct mini_spi_nor_info *spi_nor_info;

	spi_nor_info = &mini_params;

	printf("mini_name=%s\n", spi_nor_info->name);
	printf("mini_name=%x\n", spi_nor_info->name);
	printf("mini_id=0x%x\n", spi_nor_info->id);

	printf("mini_read_standard->cmd=0x%x\n",		spi_nor_info->read_standard.cmd);
	printf("mini_read_standard->dummy=0x%x\n",		spi_nor_info->read_standard.dummy_byte);
	printf("mini_read_standard->addr_nbyte=0x%x\n",	spi_nor_info->read_standard.addr_nbyte);
	printf("mini_read_standard->transfer_mode=0x%x\n",	spi_nor_info->read_standard.transfer_mode);

	printf("mini_read_quad->cmd=0x%x\n",			spi_nor_info->read_quad.cmd);
	printf("mini_read_quad->dummy=0x%x\n",		spi_nor_info->read_quad.dummy_byte);
	printf("mini_read_quad->addr_nbyte=0x%x\n",		spi_nor_info->read_quad.addr_nbyte);
	printf("mini_read_quad->transfer_mode=0x%x\n",	spi_nor_info->read_quad.transfer_mode);

	printf("mini_wr_en->cmd=0x%x\n",		spi_nor_info->wr_en.cmd);
	printf("mini_wr_en->dummy=0x%x\n",		spi_nor_info->wr_en.dummy_byte);
	printf("mini_wr_en->addr_nbyte=0x%x\n",	spi_nor_info->wr_en.addr_nbyte);
	printf("mini_wr_en->transfer_mode=0x%x\n",	spi_nor_info->wr_en.transfer_mode);

	printf("mini_en4byte->cmd=0x%x\n",		spi_nor_info->en4byte.cmd);
	printf("mini_en4byte->dummy=0x%x\n",		spi_nor_info->en4byte.dummy_byte);
	printf("mini_en4byte->addr_nbyte=0x%x\n",	spi_nor_info->en4byte.addr_nbyte);
	printf("mini_en4byte->transfer_mode=0x%x\n",	spi_nor_info->en4byte.transfer_mode);

	printf("mini_quad_set->cmd=0x%x\n",		spi_nor_info->quad_set.cmd);
	printf("mini_quad_set->bit_shift=0x%x\n",		spi_nor_info->quad_set.bit_shift);
	printf("mini_quad_set->mask=0x%x\n",		spi_nor_info->quad_set.mask);
	printf("mini_quad_set->val=0x%x\n",		spi_nor_info->quad_set.val);
	printf("mini_quad_set->len=0x%x\n",		spi_nor_info->quad_set.len);
	printf("mini_quad_set->dummy=0x%x\n",	spi_nor_info->quad_set.dummy);

	printf("mini_quad_get->cmd=0x%x\n",		spi_nor_info->quad_get.cmd);
	printf("mini_quad_get->bit_shift=0x%x\n",		spi_nor_info->quad_get.bit_shift);
	printf("mini_quad_get->mask=0x%x\n",		spi_nor_info->quad_get.mask);
	printf("mini_quad_get->val=0x%x\n",		spi_nor_info->quad_get.val);
	printf("mini_quad_get->len=0x%x\n",		spi_nor_info->quad_get.len);
	printf("mini_quad_get->dummy=0x%x\n",	spi_nor_info->quad_get.dummy);

	printf("mini_busy->cmd=0x%x\n",		spi_nor_info->busy.cmd);
	printf("mini_busy->bit_shift=0x%x\n",		spi_nor_info->busy.bit_shift);
	printf("mini_busy->mask=0x%x\n",		spi_nor_info->busy.mask);
	printf("mini_busy->val=0x%x\n",		spi_nor_info->busy.val);
	printf("mini_busy->len=0x%x\n",		spi_nor_info->busy.len);
	printf("mini_busy->dummy=0x%x\n",		spi_nor_info->busy.dummy);

	printf("mini_quad_ops_mode=%d\n",	spi_nor_info->quad_ops_mode);
	printf("addr_ops_mode=%d\n",	spi_nor_info->addr_ops_mode);

	printf("mini_chip_size=%d\n",	spi_nor_info->chip_size);
	printf("mini_page_size=%d\n",	spi_nor_info->page_size);
	printf("mini_erase_size=%d\n",	spi_nor_info->erase_size);
}
#endif


int norflash_get_params_from_burner(uint32_t sfc_frequency, unsigned char *addr)
{
	unsigned int chip_id ,chipnum,i;
	struct spi_nor_info *spi_nor_info;
	struct mini_spi_nor_info *mini_spi_nor_info;
	unsigned int id_len = 3;
	unsigned int id_addr = 0;
	unsigned int id_addr_len = 0;
	unsigned int dummy = 0;
	struct spiflash_info *spiflash_info;

	spiflash_info = (struct spiflash_info *)addr;

	chip_id = sfc_nor_read_id();
	printf("spi nor flash chip_id is : %x\n", chip_id);

	memcpy(&params, spiflash_info, sizeof(struct burner_params));
	memcpy(&mini_params, &spiflash_info->mini_spi_nor_info, sizeof(struct mini_spi_nor_info));
	burn_mode = spiflash_info->b_quad;

#ifdef SFC_NOR_CLONER_DEBUG
	dump_cloner_params();
	dump_mini_cloner_params();
	printf("fs_erase_size=%d\n", params.fs_erase_size);
	printf("uk_quad=%d\n", params.uk_quad);
	printf("burner_quad_mode=%d\n",spiflash_info->b_quad);
#endif

	memcpy(flash->g_nor_info, &params.spi_nor_info, sizeof(struct spi_nor_info));
	memcpy(flash->norflash_partitions, &params.norflash_partitions, sizeof(struct norflash_partitions));

	/* Update to private CDT table */
	create_cdt_table(flash, UPDATE_CDT);

	/* update sfc rate */
#ifdef CONFIG_BURNER
	if(sfc_frequency) {
		sfc_clk_set(flash->sfc, sfc_frequency);
		printf("cloner set sfc frequency:%d\n",sfc_frequency);
	}
	else {
		sfc_clk_set(flash->sfc, CONFIG_SFC_NOR_RATE);
		printf("cloner set sfc frequency fail\n");
	}
#endif

	sfc_nor_do_special_func();

#ifdef SFC_NOR_CLONER_DEBUG
	printf("partition num=%d\n", flash->norflash_partitions->num_partition_info);
	for (i = 0; i < flash->norflash_partitions->num_partition_info; i++) {
		printf("p[%d].name=%s\n", i, flash->norflash_partitions->nor_partition[i].name);
		printf("p[%d].size=%x\n", i, flash->norflash_partitions->nor_partition[i].size);
		printf("p[%d].offset=%x\n", i, flash->norflash_partitions->nor_partition[i].offset);
	}
#endif
	return 0;
}

#endif
