#include <common.h>
#include <config.h>
#include <spl.h>
#include <asm/io.h>
#include <errno.h>
#include <linux/err.h>
#include <malloc.h>
#include <asm/arch/clk.h>
#include <div64.h>

#include <asm/arch/sfc.h>
#include <asm/arch/spinor.h>
#include <generated/sfc_timing_val.h>
#include "spl_rtos.h"


#define STATUS_MAX_LEN  4      //4 * byte = 32 bit

//#define SFC_NOR_DEBUG
#ifdef SFC_NOR_DEBUG
#define sfc_debug(fmt, args...)			\
	do {					\
		printf(fmt, ##args);		\
	} while(0)
#else
#define sfc_debug(fmt, args...)			\
	do {					\
	} while(0)
#endif


struct sfc_flash *flash = (struct sfc_flash *)(CONFIG_SYS_TEXT_BASE + 0x500000);
struct sfc *sfc = (struct sfc *)(CONFIG_SYS_TEXT_BASE + 0x504000);

#ifdef SFC_NOR_DEBUG
void dump_cdt(struct sfc *sfc)
{
	struct sfc_cdt *cdt;
	int i;

	if(sfc->cdt_addr == NULL){
		sfc_debug("%s error: sfc res not init !\n", __func__);
		return;
	}

	cdt = sfc->cdt_addr;

	for(i = 0; i < 32; i++){
		sfc_debug("\nnum------->%d\n", i);
		sfc_debug("link:%x, ENDIAN:%x, WORD_UINT:%x, TRAN_MODE:%x, ADDR_KIND:%x\n",
				(cdt[i].link >> 31) & 0x1, (cdt[i].link >> 18) & 0x1,
				(cdt[i].link >> 16) & 0x3, (cdt[i].link >> 4) & 0xf,
				(cdt[i].link >> 0) & 0x3
				);
		sfc_debug("CLK_MODE:%x, ADDR_WIDTH:%x, POLL_EN:%x, CMD_EN:%x,PHASE_FORMAT:%x, DMY_BITS:%x, DATA_EN:%x, TRAN_CMD:%x\n",
				(cdt[i].xfer >> 29) & 0x7, (cdt[i].xfer >> 26) & 0x7,
				(cdt[i].xfer >> 25) & 0x1, (cdt[i].xfer >> 24) & 0x1,
				(cdt[i].xfer >> 23) & 0x1, (cdt[i].xfer >> 17) & 0x3f,
				(cdt[i].xfer >> 16) & 0x1, (cdt[i].xfer >> 0) & 0xffff
				);
		sfc_debug("DEV_STA_EXP:%x\n", cdt[i].staExp);
		sfc_debug("DEV_STA_MSK:%x\n", cdt[i].staMsk);
	}
}
#endif

static inline void sfc_writel(unsigned short offset, u32 value)
{
	writel(value, SFC_BASE + offset);
}

static inline unsigned int sfc_readl(unsigned short offset)
{
	return readl(SFC_BASE + offset);
}

static inline void sfc_flush_and_start(struct sfc *sfc)
{
	sfc_writel(SFC_TRIG, TRIG_FLUSH);
	sfc_writel(SFC_TRIG, TRIG_START);
}

static inline void sfc_clear_all_intc(struct sfc *sfc)
{
	sfc_writel(SFC_SCR, 0x1f);
}

static inline void sfc_mask_all_intc(struct sfc *sfc)
{
	sfc_writel(SFC_INTC, 0x1f);
}

static inline void sfc_set_mem_addr(struct sfc *sfc,unsigned int addr)
{
	sfc_writel(SFC_MEM_ADDR, addr);
}

static inline void sfc_set_length(struct sfc *sfc, int value)
{
	sfc_writel(SFC_TRAN_LEN, value);
}

static inline unsigned int sfc_read_rxfifo(struct sfc *sfc)
{
	return sfc_readl(SFC_RM_DR);
}

static inline void sfc_write_txfifo(struct sfc *sfc, const unsigned int value)
{
	sfc_writel(SFC_RM_DR, value);
}

static inline unsigned int get_sfc_ctl_sr(void)
{
	return sfc_readl(SFC_SR);
}

static unsigned int cpu_read_rxfifo(struct sfc *sfc)
{
	int i;
	unsigned long align_len = 0;
	unsigned int fifo_num = 0;
	struct sfc_cdt_xfer *xfer;

	xfer = sfc->xfer;
	align_len = ALIGN(xfer->config.datalen, 4);

	if (((align_len - xfer->config.cur_len) / 4) > THRESHOLD) {
		fifo_num = THRESHOLD;
	} else {
		fifo_num = (align_len - xfer->config.cur_len) / 4;
	}

	for (i = 0; i < fifo_num; i++) {
		*(unsigned int *)xfer->config.buf = sfc_read_rxfifo(sfc);
		xfer->config.buf += 4;
		xfer->config.cur_len += 4;
	}

	return 0;
}

static void cpu_write_txfifo(struct sfc *sfc)
{
	/**
	 * Assuming that all data is less than one word,
	 * if len large than one word, unsupport!
	 **/

	sfc_write_txfifo(sfc, *(unsigned int *)sfc->xfer->config.buf);
}

static void sfc_sr_handle(struct sfc *sfc)
{
	unsigned int reg_sr = 0;
	unsigned int tmp = 0;
	while (1) {
		reg_sr = get_sfc_ctl_sr();
		if(reg_sr & CLR_END){
			tmp = CLR_END;
			break;
		}

		if (reg_sr & CLR_RREQ) {
			sfc_writel(SFC_SCR, CLR_RREQ);
			cpu_read_rxfifo(sfc);
		}

		if (reg_sr & CLR_TREQ) {
			sfc_writel(SFC_SCR, CLR_TREQ);
			cpu_write_txfifo(sfc);
		}

		if (reg_sr & CLR_UNDER) {
			tmp = CLR_UNDER;
			sfc_debug("UNDR!\n");
			break;
		}

		if (reg_sr & CLR_OVER) {
			tmp = CLR_OVER;
			sfc_debug("OVER!\n");
			break;
		}
	}
	if (tmp)
		sfc_writel(SFC_SCR, tmp);
}

static void sfc_start_transfer(struct sfc *sfc)
{
	sfc_clear_all_intc(sfc);
	sfc_mask_all_intc(sfc);
	sfc_flush_and_start(sfc);

	sfc_sr_handle(sfc);

}

static void sfc_use_cdt(struct sfc *sfc)
{
	uint32_t tmp = sfc_readl(SFC_GLB);
	tmp |= GLB_CDT_EN;
	sfc_writel(SFC_GLB, tmp);
}

static void write_cdt(struct sfc *sfc, struct sfc_cdt *cdt, uint16_t start_index, uint16_t end_index)
{
	uint32_t cdt_num, cdt_size;

	cdt_num = end_index - start_index + 1;
	cdt_size = sizeof(struct sfc_cdt);

	memcpy((void *)sfc->cdt_addr + (start_index * cdt_size), (void *)cdt + (start_index * cdt_size), cdt_num * cdt_size);
	sfc_debug("create CDT index: %d ~ %d,  index number:%d.\n", start_index, end_index, cdt_num);
}

static void sfc_set_index(struct sfc *sfc, unsigned short index)
{

	uint32_t tmp = sfc_readl(SFC_CMD_IDX);
	tmp &= ~CMD_IDX_MSK;
	tmp |= index;
	sfc_writel(SFC_CMD_IDX, tmp);
}

static void sfc_set_dataen(struct sfc *sfc, uint8_t dataen)
{

	uint32_t tmp = sfc_readl(SFC_CMD_IDX);
	tmp &= ~CDT_DATAEN_MSK;
	tmp |= (dataen << CDT_DATAEN_OFF);
	sfc_writel(SFC_CMD_IDX, tmp);
}

static void sfc_set_datadir(struct sfc *sfc, uint8_t datadir)
{

	uint32_t tmp = sfc_readl(SFC_CMD_IDX);
	tmp &= ~CDT_DIR_MSK;
	tmp |= (datadir << CDT_DIR_OFF);
	sfc_writel(SFC_CMD_IDX, tmp);
}

static void sfc_set_addr(struct sfc *sfc, struct sfc_cdt_xfer *xfer)
{
	sfc_writel(SFC_COL_ADDR, xfer->columnaddr);
	sfc_writel(SFC_ROW_ADDR, xfer->rowaddr);
	sfc_writel(SFC_STA_ADDR0, xfer->staaddr0);
	sfc_writel(SFC_STA_ADDR1, xfer->staaddr1);
}

static void sfc_transfer_mode(struct sfc *sfc, int value)
{
	unsigned int tmp;
	tmp = sfc_readl(SFC_GLB);
	if (value == CPU_OPS)
		tmp &= ~GLB_OP_MODE;
	else
		tmp |= GLB_OP_MODE;
	sfc_writel(SFC_GLB, tmp);
}

static void sfc_set_data_config(struct sfc *sfc, struct sfc_cdt_xfer *xfer)
{
	sfc_set_dataen(sfc, xfer->dataen);

	sfc_set_length(sfc, 0);
	if(xfer->dataen){
		sfc_set_datadir(sfc, xfer->config.data_dir);
		sfc_transfer_mode(sfc, xfer->config.ops_mode);
		sfc_set_length(sfc, xfer->config.datalen);

		/* default use cpu mode */
		sfc_set_mem_addr(sfc, 0);
	}
}

static void sfc_sync_cdt(struct sfc *sfc)
{
	struct sfc_cdt_xfer *xfer;
	xfer = sfc->xfer;

	/* 1. set cmd index */
	sfc_set_index(sfc, xfer->cmd_index);

	/* 2. set addr */
	sfc_set_addr(sfc, xfer);

	/* 3. config data config */
	sfc_set_data_config(sfc, xfer);

	/* 4. start transfer */
	sfc_start_transfer(sfc);
}

void sfc_threshold(struct sfc *sfc)
{
	unsigned int tmp;
	tmp = sfc_readl(SFC_GLB);
	tmp &= ~GLB_THRESHOLD_MSK;
	tmp |= sfc->threshold << GLB_THRESHOLD_OFFSET;
	sfc_writel(SFC_GLB, tmp);
}

static void write_enable(void)
{
	struct sfc_cdt_xfer xfer;
	memset(&xfer, 0, sizeof(xfer));

	/* set index */
	xfer.cmd_index = NOR_WRITE_ENABLE;

	/* set addr */
	xfer.columnaddr = 0;

	/* set transfer config */
	xfer.dataen = DISABLE;

	flash->sfc->xfer = &xfer;
	sfc_sync_cdt(flash->sfc);
}

static void enter_4byte(void)
{
	struct sfc_cdt_xfer xfer;
	memset(&xfer, 0, sizeof(xfer));

	/* set index */
	xfer.cmd_index = NOR_EN_4BYTE;

	/* set addr */
	xfer.columnaddr = 0;

	/* set transfer config */
	xfer.dataen = DISABLE;

	flash->sfc->xfer = &xfer;
	sfc_sync_cdt(flash->sfc);
}

static void inline set_quad_mode_cmd(void)
{
	flash->cur_r_cmd = NOR_READ_QUAD;
}

/* write nor flash status register QE bit to set quad mode */
static void set_quad_mode_reg(void)
{
	struct mini_spi_nor_info *spi_nor_info;
	struct spi_nor_st_info *quad_set;

	struct sfc_cdt_xfer xfer;
	unsigned int data;

	spi_nor_info = &flash->g_nor_info;
	quad_set = &spi_nor_info->quad_set;
	data = (quad_set->val & quad_set->mask) << quad_set->bit_shift;

	/* 1. set nor quad */
	memset(&xfer, 0, sizeof(xfer));
	/* set index */
	xfer.cmd_index = NOR_QUAD_SET_ENABLE;

	/* set addr */
	xfer.columnaddr = 0;
	xfer.rowaddr = 0;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = quad_set->len;
	xfer.config.data_dir = GLB_TRAN_DIR_WRITE;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = (uint8_t *)&data;

	flash->sfc->xfer = &xfer;
	sfc_sync_cdt(flash->sfc);
}

static void sfc_nor_read_params(unsigned int addr, unsigned char *buf, unsigned int len)
{
	struct sfc_cdt_xfer xfer;
	memset(&xfer, 0, sizeof(xfer));

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

	flash->sfc->xfer = &xfer;
	sfc_sync_cdt(flash->sfc);
}

static inline void set_flash_timing(void)
{
	sfc_writel(SFC_DEV_CONF, DEF_TIM_VAL);
}

static void reset_nor(void)
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

	flash->sfc->xfer = &xfer;
	sfc_sync_cdt(flash->sfc);

	udelay(100);
}

static void params_to_cdt(struct mini_spi_nor_info *params, struct sfc_cdt *cdt)
{
	/* 4.nor singleRead */
	MK_CMD(cdt[NOR_READ_STANDARD], params->read_standard, 0, ROW_ADDR, ENABLE);

	/* 5.nor quadRead */
	MK_CMD(cdt[NOR_READ_QUAD], params->read_quad, 0, ROW_ADDR, ENABLE);

#if 0
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
#endif

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

}

static void create_cdt_table(struct sfc_flash *flash, uint32_t flag)
{
	struct mini_spi_nor_info *nor_flash_info;
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

#if 0
	/* 2.nor read id */
	cdt[NOR_READ_ID].link = CMD_LINK(0, DEFAULT_ADDRMODE, TM_STD_SPI);
	cdt[NOR_READ_ID].xfer = CMD_XFER(0, DISABLE, 0, DISABLE, SPINOR_OP_RDID);
	cdt[NOR_READ_ID].staExp = 0;
	cdt[NOR_READ_ID].staMsk = 0;

#endif

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
		nor_flash_info = &flash->g_nor_info;
		params_to_cdt(nor_flash_info, cdt);
		write_cdt(flash->sfc, cdt, NOR_READ_STANDARD, NOR_EN_4BYTE);
	}
#ifdef SFC_NOR_DEBUG
	dump_cdt(flash->sfc);
#endif
}

unsigned int get_part_offset_by_name(struct norflash_partitions partition, char *name)
{
	int i = 0;

	for (i = 0; i < partition.num_partition_info; i++) {
		if (!strncmp(partition.nor_partition[i].name, name, sizeof(name))) {
			return partition.nor_partition[i].offset;
		}
	}

	return -1;
}

unsigned int get_part_size_by_name(struct norflash_partitions partition, char *name)
{
	int i = 0;

	for (i = 0; i < partition.num_partition_info; i++) {
		if (!strncmp(partition.nor_partition[i].name, name, sizeof(name))) {
			return partition.nor_partition[i].size;
		}
	}

	return -1;
}

void spl_load_kernel(long offset)
{
	struct image_header *header;
	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	sfc_read_data(offset, sizeof(struct image_header), (unsigned char *)CONFIG_SYS_TEXT_BASE);
	header->ih_name[IH_NMLEN - 1] = 0;
	spl_parse_image_header(header);
	sfc_read_data(offset, spl_image.size, (unsigned char *)spl_image.load_addr);
}

void sfc_init(void)
{
	struct mini_spi_nor_info *spi_nor_info;
#ifdef CONFIG_SFC_NOR_INIT_RATE
	clk_set_rate(SFC, CONFIG_SFC_NOR_INIT_RATE);
#else
	/* default: sfc rate 50MHz */
	clk_set_rate(SFC, 200000000L);
#endif

	sfc->threshold = THRESHOLD;
	flash->sfc = sfc;

	/* use CDT mode */
	sfc_use_cdt(flash->sfc);
	sfc_debug("Enter 'CDT' mode.\n");

	/* try creating default CDT table */
	flash->sfc->cdt_addr = (volatile void *)(SFC_BASE + SFC_CDT);
	create_cdt_table(flash, DEFAULT_CDT);

	/* reset nor flash */
	reset_nor();

	/* config sfc */
	set_flash_timing();
	sfc_threshold(flash->sfc);

	/* get nor flash params */
	sfc_nor_read_params(CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct burner_params), (unsigned char *)&flash->g_nor_info, sizeof(struct mini_spi_nor_info));
	sfc_debug("%s %x\n", flash->g_nor_info.name, flash->g_nor_info.id);

	/* update to private CDT table */
	create_cdt_table(flash, UPDATE_CDT);

	spi_nor_info = &flash->g_nor_info;

	flash->cur_r_cmd = NOR_READ_STANDARD;

#ifdef CONFIG_SFC_QUAD
	switch (spi_nor_info->quad_ops_mode) {
		case 0:
			set_quad_mode_cmd();
			break;
		case 1:
			set_quad_mode_reg();
			break;
		default:
			break;
	}
#endif

	if (spi_nor_info->chip_size > 0x1000000) {
		switch (spi_nor_info->addr_ops_mode) {
			case 0:
				enter_4byte();
				break;
			case 1:
				write_enable();
				enter_4byte();
				break;
			default:
				break;
		}
	}
}

static unsigned int sfc_do_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
	struct sfc_cdt_xfer xfer;
	memset(&xfer, 0, sizeof(xfer));

	/* set Index */
	xfer.cmd_index = flash->cur_r_cmd;

	/* set addr */
	xfer.columnaddr = 0;
	xfer.rowaddr = addr;

	/* set transfer config */
	xfer.dataen = ENABLE;
	xfer.config.datalen = len;
	xfer.config.data_dir = GLB_TRAN_DIR_READ;
	xfer.config.ops_mode = CPU_OPS;
	xfer.config.buf = buf;

	flash->sfc->xfer = &xfer;
	sfc_sync_cdt(flash->sfc);

	return len;
}

int sfc_read_data(unsigned int from, unsigned int len, unsigned char *buf)
{
	int tmp_len = 0, current_len = 0;

	while((int)len > 0) {
		tmp_len = sfc_do_read((unsigned int)from + current_len, &buf[current_len], len);
		current_len += tmp_len;
		len -= tmp_len;
	}

	return current_len;

}


#ifdef CONFIG_OTA_VERSION20
static void nv_map_area(unsigned int *base_addr, unsigned int nv_addr, unsigned int nv_size)
{
	unsigned int buf[6][2];
	unsigned int tmp_buf[4];
	unsigned int nv_off = 0, nv_count = 0;
	unsigned int addr, i;
	unsigned int blocksize = flash->g_nor_info.erase_size;
	unsigned int nv_num = nv_size / blocksize;

	if(nv_num > 6) {
	//	sfc_debug("%s,bigger\n",__func__);
		while(1);
	}

	for(i = 0; i < nv_num; i++) {
		addr = nv_addr + i * blocksize;
		sfc_read_data(addr, 4, (unsigned char *)buf[i]);
		if(buf[i][0] == 0x5a5a5a5a) {
			sfc_read_data(addr + 1 *1024,  16, (unsigned char *)tmp_buf);
			addr += blocksize - 8;
			sfc_read_data(addr, 8, (unsigned char *)buf[i]);
			if(buf[i][1] == 0xa5a5a5a5) {
				if(nv_count < buf[i][0]) {
					nv_count = buf[i][0];
					nv_off = i;
				}
			}
		}
	}

	*base_addr = nv_addr + nv_off * blocksize;
}
#endif

#ifdef CONFIG_SPL_RTOS_BOOT

struct rtos_header rtos_header;

#ifdef CONFIG_RTOS_BOOT_ON_SECOND_CPU
#include <asm/arch/ccu.h>

unsigned char second_cpu_little_stack[128] __attribute__((aligned(8)));

__attribute__ ((noreturn)) void do_boot_second_cpu(void)
{
	rtos_raw_start(&rtos_header, NULL);
	while (1);
}

static void boot_second_cpu(void)
{
  asm volatile (
    "    .set    push                        \n"
    "    .set    reorder                     \n"
    "    .set    noat                        \n"
    "    la    $29, (second_cpu_little_stack+128)   \n"
    "    j do_boot_second_cpu   \n"
    "    nop                    \n"
    "    .set    pop                         \n"
    :
    :
    : "memory"
    );
}

static void start_second_cpu(void)
{
	unsigned long value;
	volatile unsigned long *rtos_start = (unsigned long *)rtos_header.img_start;

	value = *rtos_start;

	writel((unsigned long)boot_second_cpu, CCU_IO_BASE+CCU_RER);
	writel(0, CCU_IO_BASE+CCU_CSRR);

	if (rtos_header.version & (1 << 0)) {
		while(*rtos_start == value) {
			mdelay(1);
		}
		writel(1 << 1, CCU_IO_BASE+CCU_CSRR);
	}
}
#endif

static int spl_sfc_nor_rtos_load(struct rtos_header *rtos, unsigned int offset)
{
	sfc_read_data(offset, sizeof(*rtos), (unsigned char *)rtos);
	if (rtos_check_header(rtos))
		return -1;

	int size = rtos->img_end - rtos->img_start;
	printf("size = %d tag = 0x%x 0x%x\n",size,rtos->tag,offset);
#ifdef CONFIG_JZ_SCBOOT
	int start = rtos->img_end + 4096;
	sfc_read_data(offset, size, start);
	int ret = secure_scboot((void *)(start + sizeof(struct rtos_header)), (void*)rtos->img_start);
	if(ret) {
		printf("Error spl secure load freertos.\n");
		return -1;
	}
#else
	sfc_read_data(offset, size, (unsigned char *)rtos->img_start);
#endif

	return 0;
}

static void spl_sfc_nor_rtos_boot(void)
{
	unsigned int rtos_addr = CONFIG_RTOS_OFFSET;
	struct norflash_partitions partition;

	sfc_read_data(CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct spi_nor_info) + sizeof(int) * 2, sizeof(struct norflash_partitions), (unsigned char *)&partition);

#ifdef CONFIG_SPL_OS_OTA_BOOT
	const char *rtos_name = CONFIG_SPL_RTOS_NAME;

	rtos_addr = get_part_offset_by_name(partition, CONFIG_SPL_OTA_NAME);
	if (rtos_addr != -1) {
		char buf[128];
		const char *kernel2 = "ota:"CONFIG_SPL_OS_NAME2;
		sfc_read_data(rtos_addr, sizeof(buf), (unsigned char *)buf);
		if (!strncmp(kernel2, buf, strlen(kernel2))) {
			rtos_name = CONFIG_SPL_RTOS_NAME2;
		}
	}

	rtos_addr = get_part_offset_by_name(partition, rtos_name);
	if (rtos_addr == -1) {
		printf("rtos not found: "CONFIG_SPL_RTOS_NAME"\n");
		hang();
	}

	debug("rtos:%s %x\n", rtos_name, rtos_addr);
#else
    #ifdef CONFIG_SPL_RTOS_NAME
	rtos_addr = get_part_offset_by_name(partition, CONFIG_SPL_RTOS_NAME);
	if (rtos_addr == -1) {
		printf("rtos not found: "CONFIG_SPL_RTOS_NAME"\n");
		printf("use rtos default offset_addr:%d\n", CONFIG_RTOS_OFFSET);
		rtos_addr = CONFIG_RTOS_OFFSET;
	}
    #else
    rtos_addr = CONFIG_RTOS_OFFSET;
    #endif
#endif

	if (spl_sfc_nor_rtos_load(&rtos_header, rtos_addr))
		hang();

	flush_cache_all();

#ifdef CONFIG_RTOS_BOOT_ON_SECOND_CPU
	start_second_cpu();
#else
	/* NOTE: not return */
	rtos_raw_start(&rtos_header, NULL);
#endif
}

void *spl_rtos_get_spl_image_info(void)
{
	return NULL;
}
#endif

#ifdef CONFIG_SPL_OS_BOOT
void spl_sfc_nor_os_load(void)
{
	struct norflash_partitions partition;
	unsigned int bootimg_addr = 0;

	sfc_read_data(CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct spi_nor_info) + sizeof(int) * 2, sizeof(struct norflash_partitions), (unsigned char*)&partition);
	bootimg_addr = get_part_offset_by_name(partition, CONFIG_SPL_OS_NAME);
	if (bootimg_addr == -1){
		printf("kernel not found: "CONFIG_SPL_OS_NAME"\n");
		hang();
	}

	spl_load_kernel(bootimg_addr);
}
#endif

#ifdef CONFIG_OTA_VERSION20
void spl_ota_load_image(void)
{
	struct image_header *header;

	unsigned int bootimg_addr = 0;
	unsigned int bootimg_size = 0;
	struct norflash_partitions partition;
	int i;

	unsigned int nv_rw_addr;
	unsigned int nv_rw_size;
	unsigned int src_addr, updata_flag;
	unsigned nv_buf[2];
	int count = 8;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);
	//memset(header, 0, sizeof(struct image_header));

	sfc_read_data(CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct spi_nor_info) + sizeof(int) * 2, sizeof(struct norflash_partitions), (unsigned char*)&partition);

	bootimg_addr = get_part_offset_by_name(partition, CONFIG_SPL_OS_NAME);
	if (bootimg_addr == -1){
		printf("kernel not found: "CONFIG_SPL_OS_NAME"\n");
		hang();
	}

	bootimg_size = get_part_size_by_name(partition, CONFIG_SPL_OS_NAME);
	if (bootimg_size == -1){
		printf("kernel not found: "CONFIG_SPL_OS_NAME"\n");
		hang();
	}

	nv_rw_addr = get_part_offset_by_name(partition, CONFIG_PAR_NV_NAME);
	if (nv_rw_addr == -1){
		printf("nv_rw not found: "CONFIG_PAR_NV_NAME"\n");
		hang();
	}

	nv_rw_size = get_part_size_by_name(partition, CONFIG_PAR_NV_NAME);
	if (nv_rw_size == -1){
		printf("nv_rw not found: "CONFIG_PAR_NV_NAME"\n");
		hang();
	}

	nv_map_area((unsigned int)&src_addr, nv_rw_addr, nv_rw_size);
	sfc_read_data(src_addr, count, (unsigned char *)nv_buf);
	updata_flag = nv_buf[1];
	if((updata_flag & 0x3) != 0x3)
	{
		spl_load_kernel(bootimg_addr);
	} else {
		header->ih_name[IH_NMLEN - 1] = 0;
		spl_parse_image_header(header);
		sfc_read_data(CONFIG_UBOOT_OFFSET, CONFIG_SYS_MONITOR_LEN,(unsigned char *)CONFIG_SYS_TEXT_BASE);
	}
}
#endif

#ifdef CONFIG_BOOT_VMLINUX
void spl_vmlinux_load(void)
{
	unsigned int bootimg_addr = 0;
	unsigned int bootimg_size = 0;
	struct norflash_partitions partition;

	sfc_read_data(CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct spi_nor_info) + sizeof(int) * 2, sizeof(struct norflash_partitions), (unsigned char *)&partition);

	bootimg_addr = get_part_offset_by_name(partition, CONFIG_SPL_OS_NAME);
	if (bootimg_addr == -1) {
		printf("kernel not found: "CONFIG_SPL_OS_NAME"\n");
		hang();
	}

	bootimg_size = get_part_size_by_name(partition, CONFIG_SPL_OS_NAME);
	if (bootimg_size == -1) {
		printf("kernel not found: "CONFIG_SPL_OS_NAME"\n");
		hang();
	}

	spl_image.os = IH_OS_LINUX;
	spl_image.entry_point = CONFIG_LOAD_ADDR;
	sfc_read_data(bootimg_addr, bootimg_size, (unsigned char *)CONFIG_LOAD_ADDR);
}
#endif


#ifdef CONFIG_SPL_OS_OTA_BOOT
static char *spl_sfc_nand_os_ota_load(void)
{
	struct norflash_partitions partition;
	unsigned int img_addr = 0;
	int is_kernel2=0;
	const char *kernel_name=CONFIG_SPL_OS_NAME;

	sfc_read_data(CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct spi_nor_info) + sizeof(int) * 2, sizeof(struct norflash_partitions), (unsigned char *)&partition);
	img_addr = get_part_offset_by_name(partition, CONFIG_SPL_OTA_NAME);
	if (img_addr != -1) {
		char buf[128];
		const char *kernel2 = "ota:"CONFIG_SPL_OS_NAME2;
		sfc_read_data(img_addr, sizeof(buf), (unsigned char *)buf);
		if (!strncmp(kernel2, buf, strlen(kernel2))) {
			is_kernel2 = 1;
			kernel_name=CONFIG_SPL_OS_NAME2;
		}
	}

	img_addr = get_part_offset_by_name(partition, kernel_name);
	if (img_addr == -1) {
		printf("kernel not found: "CONFIG_SPL_OS_NAME"\n");
		hang();
	}

	debug("kernel:%s %x\n", kernel_name, img_addr);

	spl_load_kernel(img_addr);

	if (is_kernel2)
		return CONFIG_SYS_SPL_ARGS_ADDR2;
	else
		return CONFIG_SYS_SPL_ARGS_ADDR;
}
#endif

char* spl_sfc_nor_load_image(void)
{
	sfc_init();

#ifdef CONFIG_BOOT_VMLINUX
	spl_vmlinux_load();
	return NULL;
#elif defined(CONFIG_OTA_VERSION20)
	return spl_ota_load_image();
	return NULL;
#elif defined(CONFIG_SPL_OS_OTA_BOOT)
	return spl_sfc_nor_os_ota_load();
#elif defined(CONFIG_SPL_OS_BOOT)
	spl_sfc_nor_os_load();
	return NULL;
#elif defined(CONFIG_SPL_RTOS_BOOT)
	spl_sfc_nor_rtos_boot();
	return NULL;
#else
	{
		struct image_header *header;
		header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

		header->ih_name[IH_NMLEN - 1] = 0;
		spl_parse_image_header(header);
		sfc_read_data(CONFIG_UBOOT_OFFSET, CONFIG_SYS_MONITOR_LEN,(unsigned char *)CONFIG_SYS_TEXT_BASE);
	}
	return NULL;
#endif
}

