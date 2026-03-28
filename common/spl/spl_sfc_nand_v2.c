#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/sfc.h>
#include <asm/arch/spinand.h>
#include "spl.h"
#include <generated/sfc_timing_val.h>
#include <generated/sfc_nand_params.h>
#include "spl_rtos.h"
#include "spl_rtos_argument.h"
#include "spl_riscv.h"

#ifdef CONFIG_OTA_VERSION30
#include "spl_ota_kunpeng.h"
#endif

#define SPINAND_PARAM_SIZE			1024

static struct spl_rtos_argument spl_rtos_args;
static struct rtos_boot_os_args os_boot_args;

static struct spl_nand_param *curr_device;

#ifdef CONFIG_X2580
/* nand 未经测试 */
static int x2580_sfc_change_io_function(int is_quad)
{
	if (!is_quad)
		return 0;

	/*
	 * 解决X2580 SFC quad模式读写异常
	 * 更改SFC0控制器 CLK/D0~D3 output1-output0-input,再将恢复为func1功能, CE管脚不操作
	 *
	 * PA23 : SFC_DT_IO0
	 * PA24 : SFC_DR_IO1
	 * PA25 : SFC_HOLD_IO4
	 * PA26 : SFC_WP_IO2
	 * PA27 : SFC_CLK
	 * PA28 : SFC_CE
	 *            0x10  0x20  0x30  0x40
	 *            INT   MASK  PAT1  PAT0
	 * func1       0     0     0     1
	 * output0     0     1     0     0
	 */
	gpio_set_func(0, GPIO_OUTPUT1, 0x1f << 23);
	gpio_set_func(0, GPIO_OUTPUT0, 0x1f << 23);
	gpio_set_func(0, GPIO_INPUT, 0x1f << 23);
	gpio_set_func(0, GPIO_FUNC_1, 0x1f << 23);

	return 0;
}
/* nand 未经测试 */
static int ingenic_sfc_gpio_slew_driver_strength(void)
{
	unsigned int base = GPIO_BASE + 0x1000 * 0;
	/*
	 * SFC: PA23 ~ PA28
	 * Slew : 0x10010160  ===> 1 : Fast mode
	 * Driver strength
	 * DS1 DS0
	 *  0   0    ===> 2mA
	 *  0   1    ===> 4mA
	 *  1   0    ===> 8mA
	 *  1   1    ===> 12mA
	 */
	writel(0x1F800000, base + PXPSLWS);  /* Slew===> 1 */
	writel(0x1F800000, base + PXPDS0S);  /* DS0 ===> 1 */
	writel(0x1F800000, base + PXPDS1C);  /* DS1 ===> 0 */
}
#endif

static inline void sfc_writel(unsigned int value, unsigned short offset)
{
	writel(value, SFC_BASE + offset);
}

static inline unsigned int sfc_readl(unsigned short offset)
{
	return readl(SFC_BASE + offset);
}

static void sfc_set_mode(unsigned int channel, unsigned int value)
{
	unsigned int tmp;

	tmp = sfc_readl(SFC_TRAN_CONF1(channel));
	tmp &= ~TRAN_CONF1_TRAN_MODE_MSK;
	tmp |= (value << TRAN_CONF1_TRAN_MODE_OFFSET);
	sfc_writel(tmp, SFC_TRAN_CONF1(channel));
}

static inline void sfc_set_length(unsigned int value)
{
	sfc_writel(value, SFC_TRAN_LEN);
}

static void sfc_transfer_direction(unsigned int value)
{
	unsigned int tmp;

	tmp = sfc_readl(SFC_GLB);

	if(value == 0)
		tmp &= ~GLB_TRAN_DIR;
	else
		tmp |= GLB_TRAN_DIR;

	sfc_writel(tmp, SFC_GLB);
}

static inline void sfc_dev_addr(unsigned int channel, unsigned int value)
{
	sfc_writel(value, SFC_DEV_ADDR(channel));
}

static inline void sfc_dev_addr_plus(unsigned int channel, unsigned int value)
{
	sfc_writel(value, SFC_DEV_ADDR_PLUS(channel));
}

static inline void set_flash_timing(void)
{
	sfc_writel(DEF_TIM_VAL, SFC_DEV_CONF);
}

static inline void sfc_tranconf_init(struct jz_sfc *sfc, unsigned int channel)
{
	sfc_writel(sfc->tranconf.d32, SFC_TRAN_CONF0(channel));
}

static void sfc_set_transfer(struct jz_sfc *sfc, unsigned int dir)
{
	sfc_set_mode(0, sfc->tran_mode);
	sfc_tranconf_init(sfc, 0);

	if(dir == 1)
		sfc_transfer_direction(GLB_TRAN_DIR_WRITE);
	else
		sfc_transfer_direction(GLB_TRAN_DIR_READ);
	sfc_set_length(sfc->len);
	sfc_dev_addr(0, sfc->addr);
}

static void clear_end(void)
{
	while(!(sfc_readl(SFC_SR) & END));
        sfc_writel(CLR_END, SFC_SCR);
}

static void sfc_send_cmd(struct jz_sfc *sfc, unsigned char dir)
{
	sfc_writel(1 << 1, SFC_TRIG);
	sfc_set_transfer(sfc, dir);
	sfc_writel(1 << 2, SFC_TRIG);
	sfc_writel(TRIG_START, SFC_TRIG);

	/*this must judge the end status*/
	//clear_end();
}

static void sfc_write_data(unsigned int *data, unsigned int length)
{
	while (!(sfc_readl(SFC_SR) & TRAN_REQ));
	sfc_writel(CLR_TREQ,SFC_SCR);
	sfc_writel(*data, SFC_RM_DR);
	clear_end();
}

static int spl_sfc_write_data(unsigned int *data, unsigned int length)
{
	unsigned int tmp_len = 0;
	unsigned int fifo_num = 0;
	unsigned int reg_tmp = 0;
	unsigned int len = (length + 3) / 4;
	int i;

	while(1){
		reg_tmp = sfc_readl(SFC_SR);
		if (reg_tmp & TRAN_REQ) {
			sfc_writel(CLR_TREQ, SFC_SCR);
			if ((len - tmp_len) > THRESHOLD)
				fifo_num = THRESHOLD;
			else
				fifo_num = len - tmp_len;

			for (i = 0; i < fifo_num; i++) {
				sfc_writel(*data, SFC_RM_DR);
				data++ ;
				tmp_len++;
			}
		}

		if (tmp_len == len)
			break;
	}

	clear_end();

	return 0;
}

static int sfc_read_data(unsigned int *data, unsigned int length)
{
	unsigned int tmp_len = 0;
	unsigned int fifo_num = 0;
	unsigned int reg_tmp = 0;
	unsigned int len = (length + 3) / 4;
	int i;

	while(1){
		reg_tmp = sfc_readl(SFC_SR);
		if (reg_tmp & RECE_REQ) {
			sfc_writel(CLR_RREQ, SFC_SCR);
			if ((len - tmp_len) > THRESHOLD)
				fifo_num = THRESHOLD;
			else
				fifo_num = len - tmp_len;

			for (i = 0; i < fifo_num; i++) {
				*data++ = sfc_readl(SFC_RM_DR);
				tmp_len++;
			}
		}
		if (tmp_len == len)
			break;
	}
	clear_end();

	return 0;
}

static void sfc_do_erase_blk(unsigned int page_addr)
{
    unsigned int read_buf = 0;
    struct jz_sfc sfc;
    memset(&sfc, 0, sizeof(sfc));

    /* write enable */
    SFC_SEND_COMMAND(&sfc, SPINAND_CMD_WREN, 0, 0, 0, 0, 0, 0);
    clear_end();

    /* erase block */
    SFC_SEND_COMMAND(&sfc, SPINAND_CMD_ERASE_128K, 0, page_addr, 3, 0, 0, 0);
    clear_end();

    /* get feature */
    do {
        SFC_SEND_COMMAND(&sfc, SPINAND_CMD_GET_FEATURE, 1, SPINAND_ADDR_STATUS, 1, 0, 1, 0);
        sfc_read_data(&read_buf, 1);
    }while(read_buf & 0x1);
}

static void sfc_controler_init(void)
{
	unsigned int tmp;
#ifdef CONFIG_SFC_NAND_INIT_RATE
	clk_set_rate(SFC, CONFIG_SFC_NAND_INIT_RATE);
#else
	/* default: SFC rate 50MHz */
	clk_set_rate(SFC, 200000000L);
#endif

	tmp = sfc_readl(SFC_GLB);
	tmp &= ~(GLB_THRESHOLD_MSK);
	tmp |= (THRESHOLD << GLB_THRESHOLD_OFFSET);
	sfc_writel(tmp, SFC_GLB);

#ifdef CONFIG_X2580
	ingenic_sfc_gpio_slew_driver_strength();
#endif

	/* default: tSH--5cycle, tSETUP--1/2cycle, tHOLD--1/2cycle */
	set_flash_timing();
}

static int spinand_bad_block_check(int len, unsigned char *check_buf)
{
	int i;

	for(i = 0; i < len; i++)
		if(check_buf[i] != 0xff)
			return 1;
	return 0;
}

static int spinand_read_page(unsigned int page, unsigned int column, unsigned char *dst_addr,
		unsigned int len, unsigned int pagesize)
{
	struct jz_sfc sfc;
	unsigned int read_buf = 0;
	int oob_flag = 0;
	unsigned char i;
	unsigned char checklen = 1;

read_oob:
	if (oob_flag) {
		column = pagesize;
		len = 4;
		dst_addr = (unsigned char *)&read_buf;
	}

	SFC_SEND_COMMAND(&sfc, SPINAND_CMD_PARD, 0, page, 3, 0, 0, 0);
	clear_end();
	do {
		SFC_SEND_COMMAND(&sfc, SPINAND_CMD_GET_FEATURE, 1, SPINAND_ADDR_STATUS, 1, 0, 1, 0);
		sfc_read_data(&read_buf, 1);
	}while(read_buf & 0x1);
	/*ecc check*/

	for(i = 0; i < curr_device->eccstat_count; i++) {
		read_buf = read_buf & 0xff;
		if(((read_buf >> curr_device->ecc_bit) &
		(~(0xff << curr_device->bit_counts))) == curr_device->eccerrstatus[i])
			return -1;
	}

	/* plane select */
	if(curr_device->plane_select)
		column |= (((page >> 6) & 1) << 12);

#ifdef CONFIG_SFC_QUAD
	#ifdef CONFIG_X2580
	x2580_sfc_change_io_function(1);
	#endif
	SFC_SEND_COMMAND(&sfc, SPINAND_CMD_RDCH_X4, len, column, curr_device->addrlen, 8, 1, 0);
#else
	SFC_SEND_COMMAND(&sfc, SPINAND_CMD_FRCH, len, column, curr_device->addrlen, 8, 1, 0);
#endif
	sfc_read_data((unsigned int *)dst_addr, len);

	if (!oob_flag && !(page % CONFIG_SPI_NAND_PPB)) {
		oob_flag = 1;
		goto read_oob;
	} else if (oob_flag) {
#if NAND_BUSWIDTH == NAND_BUSWIDTH_16
		checklen = 2;
#endif
		if (spinand_bad_block_check(checklen, (unsigned char *)&read_buf))
			return 1;
	}

	return 0;
}

static int  spinand_write_page(unsigned int page,unsigned int column,unsigned char * dst_addr,
		unsigned int len,unsigned int pagesize){
	/* sequence
	 * 1 program load
	 * 2 write enable
	 * 3 program execute
	 * 4 get feature
	*/

	struct jz_sfc sfc;
	unsigned int read_buf = 0;
	unsigned char i;

	unsigned int ret =0,block_addr = 0;
	unsigned int addr = 0;
	unsigned int bad_block_check_len = 4;
	unsigned int bad_block_check_page_addr = 0;
	unsigned int bad_block_check_column_addr = pagesize;


	/* check bad block*/
	block_addr = page / CONFIG_SPI_NAND_PPB;
	bad_block_check_page_addr = block_addr * CONFIG_SPI_NAND_PPB;

	ret = spinand_read_page(bad_block_check_page_addr,bad_block_check_column_addr,(unsigned char *)&addr,bad_block_check_len,pagesize);

	if (ret > 0){
		serial_debug("block %d is bad_bolck \n",block_addr);
		return 1;
	}

	/* write enable */
	SFC_SEND_COMMAND(&sfc, SPINAND_CMD_WREN, 0, 0, 0, 0, 0, 0);
	clear_end();

	/* plane select */
	if(curr_device->plane_select)
		column |= (((page >> 6) & 1) << 12);

	/*send write command*/
	SFC_SEND_COMMAND(&sfc, SPINAND_CMD_PRO_LOAD,len,column,2,0,1,1);
	spl_sfc_write_data((unsigned int *)dst_addr, len);

	/*send program execute command*/
	SFC_SEND_COMMAND(&sfc, SPINAND_CMD_PRO_EN,0,page, 3,0,0,0);
	clear_end();

	/*Wait for tCS time after sending page programming commands and tcS is 20ns*/
	udelay(1);

	/* get feature */
	do {
		SFC_SEND_COMMAND(&sfc, SPINAND_CMD_GET_FEATURE, 1, SPINAND_ADDR_STATUS, 1, 0, 1, 0);
		sfc_read_data(&read_buf, 1);
	}while(read_buf & 0x1);

	return 0;
}
static int probe_id_list(unsigned char* id)
{
	unsigned char i;

	for (i = 0; i < ARRAY_SIZE(nand_param); i++) {
		if (nand_param[i].device_id >0x0 && nand_param[i].device_id<=0xff &&
						nand_param[i].id_manufactory == id[0] &&
						nand_param[i].device_id == id[1]) {
			curr_device = &nand_param[i];
			break;
		}
		else if(nand_param[i].device_id >0xff && nand_param[i].device_id <= 0xffff &&
						nand_param[i].id_manufactory == id[0] &&
						nand_param[i].device_id == (id[2] | (id[1]<<8))) {
			curr_device = &nand_param[i];
			break;
		}
	}

	if (i == ARRAY_SIZE(nand_param))
		return -ENODEV;

	return 0;
}

static int spinand_probe_id(struct jz_sfc* sfc)
{
	/*
	 * cmd-->addr-->pid
	 */
	unsigned char addrlen[] = {0, 1};
	unsigned char id[3] = {0};
	unsigned char i;
	for(i = 0; i < sizeof(addrlen); i++) {
		SFC_SEND_COMMAND(sfc, SPINAND_CMD_RDID, 3, 0, addrlen[i], 0, 1, 0);
		sfc_read_data((unsigned int *)id, 3);
		if (!probe_id_list(id)){
			    break;
		}
	}
	if(i == sizeof(addrlen)) {
		debug("ERR: don`t support this kind of nand device, \
			please add it\n");
		return -ENODEV;
	}
#ifndef CONFIG_DDR_DRVODT_DEBUG
	serial_debug("%d, VID=0x%x, PID=0x%x\n", __LINE__, id[0], id[1]);
#endif
	return 0;
}

int spinand_init(void)
{
	struct jz_sfc sfc;
	unsigned int x;

	/*
	 * Probe nand vid/pid
	 */
	if(spinand_probe_id(&sfc))
		return -ENODEV;
	/* disable write protect */
	x = 0;
	SFC_SEND_COMMAND(&sfc, SPINAND_CMD_SET_FEATURE, 1, SPINAND_ADDR_PROTECT, 1, 0, 1, 1);
	sfc_write_data(&x, 1);

	x = BITS_QUAD_EN | BITS_ECC_EN | BITS_BUF_EN;
	SFC_SEND_COMMAND(&sfc, SPINAND_CMD_SET_FEATURE, 1, SPINAND_ADDR_FEATURE, 1, 0, 1, 1);
	sfc_write_data(&x, 1);

	return 0;
}

int sfc_nand_write(unsigned int flash_start_addr,unsigned int length,unsigned char *write_buffer)
{
	/*flash_start_addr是写入FLASH的起始地址,length是写入FLASH的数据长度,write_buffer是写入FLASH的数据*/
	unsigned int pageaddr,columnaddr,wlen;
	int ret;
	unsigned int pagesize = curr_device->pagesize;

	while(length) {
		pageaddr = flash_start_addr / pagesize;
		columnaddr = flash_start_addr % pagesize;
		wlen = (pagesize - columnaddr) < length ? (pagesize - columnaddr) : length;

		ret = spinand_write_page(pageaddr,columnaddr,write_buffer,wlen,pagesize);
		if(ret > 0) {
			debug("bad block %d\n",pageaddr / CONFIG_SPI_NAND_PPB);
			flash_start_addr += CONFIG_SPI_NAND_PPB * pagesize;
			continue;
		}

		write_buffer += wlen;
		flash_start_addr += wlen;
		length -= wlen;
	}

	return 0;
}

int sfc_nand_load(unsigned int src_addr, unsigned int count, unsigned int dst_addr)
{
	unsigned int pageaddr, columnaddr, rlen;
	int ret, retry_count = 5;
	unsigned char *buf = (unsigned char *)dst_addr;
	unsigned int pagesize = curr_device->pagesize;

	while (count) {
		pageaddr = src_addr / pagesize;
		columnaddr = src_addr % pagesize;
		rlen = (pagesize - columnaddr) < count ? (pagesize - columnaddr) : count;

		ret = spinand_read_page(pageaddr, columnaddr, buf, rlen, pagesize);
		if (ret > 0) {
			debug("bad block %d\n", pageaddr / CONFIG_SPI_NAND_PPB);
			src_addr += CONFIG_SPI_NAND_PPB * pagesize;
			continue;
		}

		if(ret < 0 && retry_count--) {
			continue;
		}

		/*
		 *  current block ecc cannot be corrected after 5 retries,
		 *  skip this block as bad block.
		 **/
		if(retry_count < 0) {
			debug("read page ecc error, pageaddr = %d, columnaddr = %d\n", pageaddr,
				columnaddr);
			/* bad block */
			retry_count = 5;
			src_addr += CONFIG_SPI_NAND_PPB * pagesize;
			continue;
		}

		buf += rlen;
		src_addr += rlen;
		count -= rlen;
		retry_count = 5;
	}

	return 0;
}

struct jz_sfcnand_partition_param *get_partitions(void)
{
	struct jz_sfcnand_burner_param *burn_param;
	static struct jz_sfcnand_partition_param partitions;

	/*read param*/
	sfc_nand_load(CONFIG_SPIFLASH_PART_OFFSET, SPINAND_PARAM_SIZE, CONFIG_SYS_TEXT_BASE);
	burn_param = (void *)(CONFIG_SYS_TEXT_BASE);
	partitions.num_partition = burn_param->partition_num;
	partitions.partition = (struct jz_sfcnand_partition *)&burn_param->partition;

	return &partitions;
}

unsigned int get_part_offset_by_name(struct jz_sfcnand_partition_param *partitions, char *name)
{
	int i = 0;

	for(i = 0; i < partitions->num_partition; i++) {
		if (!strcmp(partitions->partition[i].name, name)) {
			return partitions->partition[i].offset;
		}
	}

	return -1;
}

struct jz_sfcnand_partition *get_part_by_name(struct jz_sfcnand_partition_param *partitions, char *name)
{
	int i = 0;

	for(i = 0; i < partitions->num_partition; i++) {
		if (!strcmp(partitions->partition[i].name, name)) {
			return &partitions->partition[i];
		}
	}

	return NULL;
}

#ifdef CONFIG_JZ_WATCHDOG
#include <asm/arch/cpm.h>
#include <watchdog.h>

#define OPEN_WRITE_CPSPR	0x00005a5a
#define CLOSE_WRITE_CPSPR	0x0000a5a5

#define SPL_OTA_RUN_FLAG 		0x4f5441
#define SPL_OTA_FAIL_FLAG 		0x41544f
#define SYS_PANIC_SIGNATURE		0x004343

static inline void cpm_write_cpspr(int val)
{
	cpm_outl(OPEN_WRITE_CPSPR, CPM_CPSPPR);
	cpm_outl(val, CPM_CPSPR);
	cpm_outl(CLOSE_WRITE_CPSPR, CPM_CPSPPR);
}

static inline int spl_test_ota_result(void)
{
	unsigned int val = cpm_inl(CPM_CPSPR);
	if (val == SPL_OTA_RUN_FLAG || val == SYS_PANIC_SIGNATURE ) {
		cpm_write_cpspr(SPL_OTA_FAIL_FLAG);
		return 1;
	}

	return 0;
}

static inline int spl_ota_set_flag_and_boot_wdt(void)
{
	cpm_write_cpspr(SPL_OTA_RUN_FLAG);
	hw_watchdog_init();
}
#endif


#ifdef CONFIG_JZ_SECURE_SUPPORT
extern int secure_scboot (void *, void *);
extern int is_security_boot(void);

#ifdef CONFIG_JZ_SECURE_ROOTFS
extern unsigned int get_ddr_size(void);
static void secure_check_hash_rootfs(const char *name, void *buffer)
{
	struct jz_sfcnand_partition_param *partitions;
	unsigned int rootfs_offset;
	unsigned int code_len;
	unsigned int *ptr = (unsigned int *)(buffer - 2048);
	unsigned int ram_size = get_ddr_size() << 20;
	int ret;

	partitions = get_partitions();

#ifdef CONFIG_SPL_OS_OTA_BOOT
	if (!strncmp(name, CONFIG_SPL_OS_NAME2, strlen(CONFIG_SPL_OS_NAME2)))
		rootfs_offset = get_part_offset_by_name(partitions, CONFIG_SPL_ROOTFS_NAME2);
	else
		rootfs_offset = get_part_offset_by_name(partitions, CONFIG_SPL_ROOTFS_NAME);
#else
	rootfs_offset = get_part_offset_by_name(partitions, CONFIG_SPL_ROOTFS_NAME);
#endif

	if (rootfs_offset == -1) {
		serial_debug("rootfs partitions not found\n");
		hang();
	}

	code_len = ptr[128];
	if ((virt_to_phys(buffer) + code_len) > ram_size) {
		printf("rootfs load add + size exceed ram size, please check load rootfs addr and size!!!\n");
		hang();
	}

	sfc_nand_load(rootfs_offset, code_len, buffer);

	ret = secure_scboot(buffer - 2048, buffer);
	if(ret) {
		serial_debug("Error check rootfs hash.\n");
		hang();
	}
}
#endif
#endif

void spl_load_kernel(long offset, const char *name)
{
	struct image_header *header;

#ifdef CONFIG_JZ_SECURE_SUPPORT
	//load kernel head
	u32 image_size;
	int ret;
	unsigned int load_addr;
	header = (struct image_header *)(CONFIG_SYS_SC_TEXT_BASE);

	sfc_nand_load(offset, sizeof(struct image_header) + sizeof(int), CONFIG_SYS_TEXT_BASE);
	header->ih_name[IH_NMLEN - 1] = 0;

	spl_parse_image_header(header);
	image_size = spl_image.size;
	load_addr = spl_image.load_addr;

#ifdef CONFIG_JZ_SECURE_ROOTFS
	unsigned int pagesize = curr_device->pagesize;
	u32 unalign_size;
	u32 entry_addr = spl_image.entry_point;

	unalign_size = image_size & (pagesize - 1);

	//向下对齐
	u32 sig_offset_align = offset + (image_size & ~(pagesize - 1));
	u32 sig_size_align = (2048 + unalign_size + pagesize - 1) & ~(pagesize - 1);
	sfc_nand_load(sig_offset_align, sig_size_align, entry_addr);

	memcpy(entry_addr - 2048, entry_addr + unalign_size - sizeof(struct image_header), 2048);
#endif


#ifdef CONFIG_JZ_SECURE_ROOTFS
	/* signature address */
	secure_check_hash_rootfs(name, (void *)entry_addr);
#endif

	sfc_nand_load(offset, image_size, (void *)load_addr);
	ret = secure_scboot(load_addr, spl_image.load_addr);
	if(ret) {
		serial_debug("Error spl secure load kernel.\n");
		hang();
	}
#else
	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	sfc_nand_load(offset, sizeof(struct image_header), CONFIG_SYS_TEXT_BASE);
	header->ih_name[IH_NMLEN - 1] = 0;
	spl_parse_image_header(header);
	sfc_nand_load(offset, spl_image.size, spl_image.load_addr);
#endif

}

void sfc_erase_data(unsigned int addr, unsigned int len)
{
    unsigned int end;
    unsigned int page_addr;
    unsigned int blocksize = 0x20000;   /* CONFIG_SPI_NAND_BPP * CONFIG_SPI_NAND_PPB */


    if ((blocksize-1) & addr) {
        serial_debug("erase error: address isn't aligned with blocks_size.\n");
        hang();
    }

    if ((blocksize-1) & len) {
        serial_debug("erase error: len must be times of blocks_size.\n");
        hang();
	}


    end = addr + len;
    while (addr < end) {
        page_addr = addr / 2048;      /* CONFIG_SPI_NAND_BPP */
        sfc_do_erase_blk(page_addr);
        addr += blocksize;
    }
}

static volatile int sfc_is_inited = 0;

void sfc_init(void)
{
	if (sfc_is_inited)
		return;

	sfc_is_inited = 1;

	sfc_controler_init();
	spinand_init();
}

#ifdef CONFIG_SPL_OS_BOOT
void spl_sfc_nand_os_load(void)
{
	struct jz_sfcnand_partition_param *partitions;
	unsigned int bootimg_addr = 0;

	sfc_init();

	partitions = get_partitions();

#ifdef CONFIG_SPL_OF_LIBFDT
	bootimg_addr = get_part_offset_by_name(partitions, CONFIG_DTB_NAME);
	if (bootimg_addr == -1){
		serial_debug("dtb not found: "CONFIG_DTB_NAME"\n");
		hang();
	}

	sfc_nand_load(bootimg_addr, CONFIG_DTB_SIZE, (unsigned char *)CONFIG_DTB_ADRESS);
#endif /* CONFIG_SPL_OF_LIBFDT */

	bootimg_addr = get_part_offset_by_name(partitions, CONFIG_SPL_OS_NAME);
	if (bootimg_addr == -1){
		serial_debug("kernel not found: "CONFIG_SPL_OS_NAME"\n");
		hang();
	}

	/* read image head */
	spl_load_kernel(bootimg_addr, CONFIG_SPL_OS_NAME);
}
#endif

#ifdef CONFIG_SPL_ALIOS_BOOT
typedef struct alios_image_header {
	unsigned int header_size;/* Image Header Size	*/
	unsigned int image_crc;	/* Image CRC Checksum	*/
	unsigned int image_size;	/* Image Data Size		*/
} alios_image_header_t;

#define RTOSA 22
#define RTOSB 23

typedef struct alios_boot_info_param {
	uint32_t version;
	uint32_t state;
	uint32_t partition;
	uint32_t error_code;
	uint32_t update_size;
	uint32_t rtosa_start;
	uint32_t rtosa_size;
	uint32_t rtosb_start;
	uint32_t rtosb_size;
	uint32_t udisk_start;
	uint32_t udisk_size;
} alios_boot_info_param_t;

void spl_sfc_nand_alios_load(void)
{
	unsigned int aos_img_addr = 0;
	int crc1, crc2;

	sfc_init();
	
	unsigned int buffer[32] = {0};
	alios_image_header_t *header;
	unsigned int header_size = sizeof(buffer);

	unsigned int buffer1[48] = {0};
	alios_boot_info_param_t *param;
	unsigned int param_size = sizeof(buffer1);
	int crc_try = 3;

	//获取boot info
	sfc_nand_load(CONFIG_ALIOS_BOOT_INFO_OFFSET, sizeof(buffer1), CONFIG_SYS_TEXT_BASE);
	memcpy(buffer1, (alios_boot_info_param_t *)(CONFIG_SYS_TEXT_BASE), sizeof(alios_boot_info_param_t));
	param = (alios_boot_info_param_t *)buffer1;

	debug("param:\n");
	debug("param->version:   %x\n", param->version);
	debug("param->state:     %x\n", param->state);
	debug("param->partition: %x\n", param->partition);
	debug("param->error_code:%x\n", param->error_code);
	debug("param->update_size:%x\n",param->update_size);
	debug("param->rtosa_start:%x\n",param->rtosa_start);
	debug("param->rtosa_size:%x\n", param->rtosa_size);
	debug("param->rtosb_start:%x\n",param->rtosb_start);
	debug("param->rtosb_size:%x\n", param->rtosb_size);
	debug("param->udisk_start:%x\n",param->udisk_start);
	debug("param->udisk_size:%x\n", param->udisk_size);

change_part:
	if(param->partition == RTOSA) {
		serial_debug("boot rtos-A\n");
		aos_img_addr = param->rtosa_start;
	} else if (param->partition == RTOSB) {
		serial_debug("boot rtos-B\n");
		aos_img_addr = param->rtosb_start;
	} else {
		serial_debug("boot partition type error!\n");
		hang();
	}

	/* 获取alios头信息 */
	debug("aos_img_addr: 0x%x\n", aos_img_addr);
	sfc_nand_load(aos_img_addr, sizeof(buffer), CONFIG_SYS_TEXT_BASE);
	memcpy(buffer, (alios_image_header_t *)(CONFIG_SYS_TEXT_BASE), sizeof(alios_image_header_t));
	header = (alios_image_header_t *)buffer;

	debug("header:\n");
	debug("header_size: %x\n",	 header->header_size);
	debug("image_crc: %x\n",	 header->image_crc);
	debug("image_size: %x\n",	 header->image_size);

	spl_image.size = header->image_size;
	spl_image.entry_point = CONFIG_SYS_TEXT_BASE;
	spl_image.load_addr = CONFIG_SYS_TEXT_BASE;
	spl_image.os = IH_OS_ALIOS;
	spl_image.name = "Alios";
	crc1 = header->image_crc;
	sfc_nand_load(aos_img_addr + header->header_size, spl_image.size, spl_image.load_addr);
	crc2 = crc32(0, spl_image.load_addr, spl_image.size);
	if(crc1 != crc2){
		if(param->partition == RTOSA) {
			serial_debug("crc error !!! goto rtos-B\n");
			param->partition = RTOSB;
		} else if(param->partition == RTOSB) {
			serial_debug("crc error !!! goto rtos-A\n");
			param->partition = RTOSA;
		}
		if(crc_try--)
			goto change_part;

		serial_debug("crc error, boot failed!\n");
		hang();
	}
	jump_to_image_no_args(&spl_image);
}
#endif

#if defined(CONFIG_SPL_RTOS_BOOT) || defined(CONFIG_SPL_RTOS_LOAD_KERNEL) || defined(CONFIG_BOOT_RTOS_OTA)

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

static int spl_sfc_rtos_load(struct rtos_header *rtos, unsigned int offset)
{
	sfc_nand_load(offset, sizeof(*rtos), (unsigned int)rtos);
	if (rtos_check_header(rtos))
		return -1;

#ifdef CONFIG_JZ_SECURE_SUPPORT
	sfc_nand_load(offset, rtos->img_end - rtos->img_start, rtos->img_start);
	int ret = secure_scboot(rtos->img_start + sizeof(struct rtos_header), rtos->img_start);
	if(ret) {
		serial_debug("Error rtos decryption.\n");
		hang();
	}
#else
	sfc_nand_load(offset, rtos->img_end - rtos->img_start, rtos->img_start);
#endif
	return 0;
}

#endif

#ifdef CONFIG_SPL_RTOS_BOOT
static void spl_sfc_rtos_boot(void)
{
	unsigned int rtos_offset = CONFIG_RTOS_OFFSET;

	sfc_init();

#ifdef CONFIG_SPL_OS_OTA_BOOT
	const char *rtos_name = CONFIG_SPL_RTOS_NAME;
	struct jz_sfcnand_partition_param *partitions = get_partitions();

	rtos_offset = get_part_offset_by_name(partitions, CONFIG_SPL_OTA_NAME);
	if (rtos_offset != -1) {
		char buf[128];
		const char *kernel2 = "ota:"CONFIG_SPL_OS_NAME2;
		sfc_nand_load(rtos_offset, sizeof(buf), (unsigned int)buf);
		if (!strncmp(kernel2, buf, strlen(kernel2))) {
			rtos_name = CONFIG_SPL_RTOS_NAME2;
		}
	}

	rtos_offset = get_part_offset_by_name(partitions, rtos_name);
	if (rtos_offset == -1) {
		serial_debug("rtos not found: "CONFIG_SPL_RTOS_NAME"\n");
		hang();
	}

	debug("rtos:%s %x\n", rtos_name, rtos_offset);
#else
	#ifdef CONFIG_SPL_RTOS_NAME
	struct jz_sfcnand_partition_param *partitions = get_partitions();

	rtos_offset = get_part_offset_by_name(partitions, CONFIG_SPL_RTOS_NAME);
	if (rtos_offset == -1) {
		serial_debug("rtos not found: "CONFIG_SPL_RTOS_NAME"\n");
		serial_debug("use rtos default offset_addr:%d\n", CONFIG_RTOS_OFFSET);
		rtos_offset = CONFIG_RTOS_OFFSET;
	}
	#else
		rtos_offset = CONFIG_RTOS_OFFSET;
	#endif
#endif

	if (spl_sfc_rtos_load(&rtos_header, rtos_offset))
		hang();

	flush_cache_all();

#ifdef CONFIG_RTOS_BOOT_ON_SECOND_CPU
	start_second_cpu();
#else
	/* NOTE: not return */
	rtos_raw_start(&rtos_header, NULL);
#endif
}

#endif

#ifdef CONFIG_SPL_RTOS_LOAD_KERNEL

static void spl_sfc_nand_cfg_os_args(struct jz_sfcnand_partition_param *partitions, char *kernel_name, char *cmdargs)
{
	unsigned int img_addr = 0;

	img_addr = get_part_offset_by_name(partitions, kernel_name);
	if (img_addr == -1) {
		serial_debug("kernel not found: "CONFIG_SPL_OS_NAME"\n");
		hang();
	}
	debug("kernel:%s %x\n", kernel_name, img_addr);

	struct image_header *header;

#ifdef CONFIG_JZ_SECURE_SUPPORT
	header = (struct image_header *)(CONFIG_SYS_SC_TEXT_BASE);
	sfc_nand_load(img_addr, sizeof(struct image_header) + sizeof(int), CONFIG_SYS_TEXT_BASE);
#else
	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);
	sfc_nand_load(img_addr, sizeof(struct image_header), CONFIG_SYS_TEXT_BASE);
#endif

	header->ih_name[IH_NMLEN - 1] = 0;
	spl_parse_image_header(header);

	cmdargs = cmdargs ? cmdargs : CONFIG_SYS_SPL_ARGS_ADDR;
#ifdef CONFIG_SPL_AUTO_PROBE_ARGS_MEM
	cmdargs = spl_board_process_mem_bootargs(cmdargs);
#endif

	/* 由RTOS 加载OS镜像, SPL等待OS加载完成, 并由SPL完成后续引导 */
	os_boot_args.magic = 0x53475241;  /* ARGS */
	os_boot_args.offset = img_addr;
	os_boot_args.size = spl_image.size;
	os_boot_args.cmdargs = cmdargs;
	os_boot_args.entry_point = spl_image.entry_point;
	os_boot_args.load_addr = spl_image.load_addr;

	spl_rtos_args.os_boot_args = &os_boot_args;
}

/* not support rtos boot on second cpu */
static char *spl_sfc_nand_boot_rtos_load_os(void)
{
	struct jz_sfcnand_partition_param *partitions;
	const char *kernel_name = CONFIG_SPL_OS_NAME;
	const char *rtos_name = CONFIG_SPL_RTOS_NAME;
	char *cmdargs = CONFIG_SYS_SPL_ARGS_ADDR;
#ifdef CONFIG_SPL_OF_LIBFDT
	char *dtbname = CONFIG_DTB_NAME;
#endif

	sfc_init();

	partitions = get_partitions();

#ifdef CONFIG_SPL_OS_OTA_BOOT
	int is_kernel2 = 0;
	unsigned int ota_addr = 0;
	ota_addr = get_part_offset_by_name(partitions, CONFIG_SPL_OTA_NAME);
	if (ota_addr != -1) {
		char buf[128];
		const char *kernel2 = "ota:"CONFIG_SPL_OS_NAME2;
		sfc_nand_load(ota_addr, sizeof(buf), (unsigned int)buf);
		if (!strncmp(kernel2, buf, strlen(kernel2))) {
			is_kernel2 = 1;
#ifdef CONFIG_SPL_OF_LIBFDT
			dtbname = CONFIG_DTB_NAME2;
#endif /* CONFIG_SPL_OF_LIBFDT */
		}

#ifdef CONFIG_JZ_WATCHDOG
		if (spl_test_ota_result()) {
			serial_debug("ota fail!\n");
			is_kernel2 = is_kernel2 ? 0 : 1;
		} else
			spl_ota_set_flag_and_boot_wdt();
#endif
		if (is_kernel2) {
			kernel_name = CONFIG_SPL_OS_NAME2;
			rtos_name = CONFIG_SPL_RTOS_NAME2;
			cmdargs = CONFIG_SYS_SPL_ARGS_ADDR2;
		}
	}
#endif

#ifdef CONFIG_SPL_OF_LIBFDT
	unsigned int dtb_addr = get_part_offset_by_name(partitions, dtbname);
	if (dtb_addr == -1){
		serial_debug("dtb not found: %s\n", dtbname);
		hang();
	}

	sfc_nand_load(dtb_addr, CONFIG_DTB_SIZE, (unsigned char *)CONFIG_DTB_ADRESS);
#endif /* CONFIG_SPL_OF_LIBFDT */

	spl_sfc_nand_cfg_os_args(partitions, kernel_name, cmdargs);

	unsigned int rtos_offset = 0;
	rtos_offset = get_part_offset_by_name(partitions, rtos_name);
	if (rtos_offset == -1) {
		serial_debug("rtos not found: "CONFIG_SPL_RTOS_NAME"\n");
		serial_debug("use rtos default offset_addr:%d\n", CONFIG_RTOS_OFFSET);
		rtos_offset = CONFIG_RTOS_OFFSET;
	}
	debug("rtos:%s %x\n", rtos_name, rtos_offset);

	if (spl_sfc_rtos_load(&rtos_header, rtos_offset))
		hang();

	flush_cache_all();

	rtos_raw_start(&rtos_header, &spl_rtos_args);

#ifdef CONFIG_JZ_SECURE_SUPPORT
	int ret = 0;
	ret = secure_scboot(spl_image.load_addr, spl_image.load_addr);
	if (ret) {
		serial_debug("Error spl secure load kernel.\n");
		hang();
	}
#endif
	return cmdargs;
}

#endif

#ifdef CONFIG_BOOT_RTOS
void spl_sfc_nand_rtos_load(void)
{
	struct jz_sfcnand_partition_param *partitions;
	struct jz_sfcnand_partition *partition;

	sfc_init();

	partitions = get_partitions();
	partition = get_part_by_name(partitions, CONFIG_SPL_OS_NAME);
	if (partition == NULL) {
		serial_debug("kernel not found: "CONFIG_SPL_OS_NAME"\n");
		hang();
	}

	spl_image.entry_point = CONFIG_LOAD_ADDR;
	sfc_nand_load(partition->offset, partition->size, (unsigned int*)CONFIG_LOAD_ADDR);
}
#endif

#ifdef CONFIG_SPL_OS_OTA_BOOT
static char *spl_sfc_nand_os_ota_load(void)
{
	struct jz_sfcnand_partition_param *partitions;
	unsigned int img_addr = 0;
	int is_kernel2=0;
	const char *kernel_name=CONFIG_SPL_OS_NAME;

	sfc_init();

	partitions = get_partitions();
	img_addr = get_part_offset_by_name(partitions, CONFIG_SPL_OTA_NAME);
	if (img_addr != -1) {
		char buf[128];
		const char *kernel2 = "ota:"CONFIG_SPL_OS_NAME2;
		sfc_nand_load(img_addr, sizeof(buf), (unsigned int)buf);
		if (!strncmp(kernel2, buf, strlen(kernel2))) {
			is_kernel2 = 1;
			kernel_name=CONFIG_SPL_OS_NAME2;
		}
	}

#ifdef CONFIG_JZ_WATCHDOG
	if (spl_test_ota_result()) {
		serial_debug("ota fail!\n");
		if (is_kernel2) {
			kernel_name = CONFIG_SPL_OS_NAME;
			is_kernel2 = 0;
		} else {
			kernel_name = CONFIG_SPL_OS_NAME2;
			is_kernel2 = 1;
		}
	} else
		spl_ota_set_flag_and_boot_wdt();
#endif

	img_addr = get_part_offset_by_name(partitions, kernel_name);
	if (img_addr == -1) {
		serial_debug("kernel not found: "CONFIG_SPL_OS_NAME"\n");
		hang();
	}

	debug("kernel:%s %x\n", kernel_name, img_addr);

	spl_load_kernel(img_addr, kernel_name);

	if (is_kernel2)
		return CONFIG_SYS_SPL_ARGS_ADDR2;
	else
		return CONFIG_SYS_SPL_ARGS_ADDR;
}
#endif

#ifdef CONFIG_SPL_MCU_RTOS_BOOT
#ifdef CONFIG_X2600
#include "x2600_riscv.h"
#endif

void spl_nand_mcu_rtos_boot(void)
{
    unsigned int riscv_offset;

    struct jz_sfcnand_partition_param *partitions = get_partitions();

    struct jz_sfcnand_partition *riscv_part = get_part_by_name(partitions, "riscv");
    if (!riscv_part) {
        serial_debug("not found riscv\n");
        return;
    }

    spl_load_riscv(sfc_nand_load, riscv_part->offset);

    flush_cache_all();

    riscv_reset();
}

#endif

#ifdef CONFIG_BOOT_RTOS_OTA
static void spl_sfc_nand_rtos_ota_boot(void)
{
	sfc_init();

	unsigned int ota_offset;
	unsigned int offset;
	struct jz_sfcnand_partition_param *partitions = get_partitions();

	ota_offset = get_part_offset_by_name(partitions, CONFIG_SPL_OTA_NAME);
	if (ota_offset != -1) {
		char buf[128];
		const char *ota_part_info = CONFIG_SPL_RTOS_OTA_INFO;
		sfc_nand_load(ota_offset, sizeof(buf), (unsigned char *)buf);
		if (strncmp(ota_part_info, buf, strlen(ota_part_info))) {
			return;
		}

		offset = get_part_offset_by_name(partitions, CONFIG_SPL_RTOS_OTA_NAME);
		if (offset == -1) {
			serial_debug("rtos not found: "CONFIG_SPL_RTOS_OTA_NAME"\n");
			return;
		}

		if (spl_sfc_rtos_load(&rtos_header, offset))
			return;

        flush_cache_all();
		rtos_raw_start(&rtos_header, NULL);
	}
}
#endif

#ifdef CONFIG_OTA_VERSION30
static struct ota_ops ota_ops = {
	.flash_init = sfc_init,
	.flash_read = sfc_nand_load,
	.flash_get_partitions = get_partitions,
	.flash_get_part_offset_by_name = get_part_offset_by_name,
	.flash_load_kernel = spl_load_kernel,
};
#endif

char* spl_sfc_nand_load_image(void)
{
	spl_rtos_args.os_boot_args = NULL;
	spl_rtos_args.card_params = NULL;

#ifdef CONFIG_BOOT_RTOS_OTA
	spl_sfc_nand_rtos_ota_boot();
#endif

#ifdef CONFIG_SPL_RTOS_LOAD_KERNEL
	return spl_sfc_nand_boot_rtos_load_os();
#endif

#if CONFIG_SPL_RTOS_BOOT
	spl_sfc_rtos_boot();
#endif

#ifdef CONFIG_OTA_VERSION30
	register_ota_ops(&ota_ops);
	return spl_ota_load_image();
#elif defined(CONFIG_BOOT_RTOS)
	spl_sfc_nand_rtos_load();
	return NULL;
#elif defined(CONFIG_SPL_OS_OTA_BOOT)
	return spl_sfc_nand_os_ota_load();
#elif defined(CONFIG_SPL_OS_BOOT)
	spl_sfc_nand_os_load();
#ifdef CONFIG_SPL_RISCV
	spl_nand_load_riscv();
	spl_start_riscv();
#endif

#ifdef CONFIG_SPL_MCU_RTOS_BOOT
    spl_nand_mcu_rtos_boot();
#endif
	return NULL;
#elif defined(CONFIG_SPL_RTOS_BOOT)
	hang();
	return NULL;
#elif defined(CONFIG_SPL_ALIOS_BOOT)
	spl_sfc_nand_alios_load();
	return NULL;
#else
	{
		struct image_header *header;
		header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);
		sfc_init();

		sfc_nand_load(CONFIG_UBOOT_OFFSET, CONFIG_SYS_MONITOR_LEN, (unsigned int)CONFIG_SYS_TEXT_BASE);
		spl_parse_image_header(header);
	}
	return NULL;
#endif
}
