#ifndef __CLONER_H__
#define __CLONER_H__

#define BURNNER_DEBUG 0

#include <common.h>
#include <errno.h>
#include <rtc.h>
#include <spi.h>
#include <ingenic_nand_mgr/nand_param.h>

/*
 *	cloner argument
 */


#define SSI_IDX 0

#define VR_GET_CPU_INFO		0x00  /*bootrom stage request*/
#define VR_SET_DATA_ADDR	0x01
#define VR_SET_DATA_LEN		0x02
#define VR_FLUSH_CACHE		0x03
#define VR_PROG_STAGE1		0x04
#define VR_PROG_STAGE2		0x05
#define VR_GET_ACK		0x10	/*firmware stage request*/
#define VR_INIT			0x11
#define VR_WRITE		0x12
#define VR_READ			0x13
#define VR_UPDATE_CFG		0x14
#define VR_SYNC_TIME		0x15
#define VR_REBOOT		0x16
#define VR_POWEROFF		0x17	/*reboot and poweroff*/
#define VR_CHECK		0x18
#define VR_GET_CRC		0x19
#define VR_GET_FLASH_INFO	0x26

/*************** security boot ****************/
#ifdef CONFIG_JZ_SCBOOT
#define VR_SEC_INIT                     0x20		/*security boot*/
#define VR_SEC_BURN_RKCK                0x21
#define VR_SEC_GET_CK_LEN               0x23
#define VR_SEC_BURN_SECBOOT_EN	        0x24
#define VR_SEC_SEDEN                    0x25

#define OPS_BURN_NKU	1
#define OPS_BURN_ENUK	2
#define OPS_GET_ENCK	1

#define RSA_KEY_LEN     128		/* byte */

#define SPL_CRC_OFFSET	9

#define SCKEY_SPLBINLEN_OFFSET          0x80 /* word */
#define SCKEY_SPLBINCRYPT_OFFSET        0x81 /* word */
#define SCKEY_SPLKEYCRYPT_OFFSET        0x84 /* word */
#define SCKEY_RSAN_OFFSET               0x300
#define SCKEY_RSAKU_OFFSET              0x400
#define SCKEY_SPLCODE_OFFSET            0x800

#define AES_ENCRYPT	0
#define AES_DECRYPT	1
#endif
/**********************************************/

#define MMC_ERASE_ALL	1
#define MMC_ERASE_PART	2
#define MMC_ERASE_CNT_MAX	10

#define MMC_MAX_ENH_AREA	1
#define MMC_ENH_PART	        2
#define MMC_GPP_AREA_MAX	4

#ifndef CONFIG_SF_DEFAULT_SPEED
# define CONFIG_SF_DEFAULT_SPEED    20000000
#endif
#ifndef CONFIG_SF_DEFAULT_MODE
# define CONFIG_SF_DEFAULT_MODE     SPI_MODE_3
#endif
#ifndef CONFIG_SF_DEFAULT_CS
# define CONFIG_SF_DEFAULT_CS       0
#endif
#ifndef CONFIG_SF_DEFAULT_BUS
# define CONFIG_SF_DEFAULT_BUS      0
#endif


union cmd {
	struct update {
		uint32_t length;
	}update;

	struct write {
		uint64_t partition;
		uint64_t offset;
		uint64_t length;
		uint32_t ops;
		uint32_t crc;
	}write;

	struct read {
		uint64_t partition;
		uint64_t offset;
		uint64_t length;
		uint32_t ops;
	}read;

	struct check {
		uint64_t partition;
		uint64_t offset;
		uint32_t check;
		uint32_t ops;
	} check;


#ifdef CONFIG_JZ_SCBOOT
	struct security {
		uint32_t security_en;
	}security;
#endif
	struct rtc_time rtc;
};


enum medium_type {
	MEMORY = 0,
	NAND,
	MMC,
	I2C,
	EFUSE,
	REGISTER,
	SPISFC,
	EXT_POL,
};

enum spisfc_sub_type {
	SFC_NOR = 0,
	SFC_NAND,
	SFC_NAND_SN_WRITE,
	SFC_NAND_MAC_WRITE,
	SFC_NAND_SN_READ,
	SFC_NAND_MAC_READ,
	SPI_NAND,
	SPI_NOR,
	SFC_NAND_LICENSE_WRITE,
	SFC_NAND_LICENSE_READ,
};

enum data_type {
	RAW = 0,
	OOB,
	IMAGE,
	MTD_RAW,
	MTD_UBI,
};

struct i2c_args {
	int clk;
	int data;
	int device;
	int value_count;
	int value[0];
};

/*module args struct start*/

#define MAGIC_INFO	(('B' << 24) | ('D' << 16) | ('I' << 8) | ('F' << 0))
#define MAGIC_DDR	(('D' << 24) | ('D' << 16) | ('R' << 8) | 0         )
#define MAGIC_DEBUG	(('D' << 24) | ('B' << 16) | ('G' << 8) | 0         )
#define MAGIC_GPIO	(('G' << 24) | ('P' << 16) | ('I' << 8) | ('O' << 0))
#define MAGIC_MMC	(('M' << 24) | ('M' << 16) | ('C' << 8) | 0         )
#define MAGIC_NAND	(('N' << 24) | ('A' << 16) | ('N' << 8) | ('D' << 0))
#define MAGIC_POLICY	(('P' << 24) | ('O' << 16) | ('L' << 8) | ('I' << 0))
#define MAGIC_SFC	(('S' << 24) | ('F' << 16) | ('C' << 8) | 0         )
#define MAGIC_EXPY	(('E' << 24) | ('X' << 16) | ('P' << 8) | ('Y' << 0))
#define MAGIC_EFUSE	(('E' << 24) | ('F' << 16) | ('U' << 8) | ('S' << 0))

struct ParameterInfo
{
	uint32_t magic;
	uint32_t size;
	uint32_t data[0];
};
struct spi_param {
	int download_params;
	int sfc_quad_mode;
	uint32_t spi_erase_block_size;
	int spi_erase;
	uint32_t sfc_frequency;
	int reserve_space;
	int reserve_space_protect;
	int param_offset;
	char* flash_info[0];
};
struct policy_param{
	int use_nand_mgr;
	int use_nand_mtd;
	int use_mmc0;
	int use_mmc1;
	int use_mmc2;
	uint32_t use_sfc_nor;
	uint32_t use_sfc_nand;
	uint32_t use_spi_nand;
	uint32_t use_spi_nor;
	uint32_t offsets[32];
};
struct efuse_param{
	uint32_t efuse_en_gpio;
	uint32_t efuse_en_active;
};
struct debug_param{
	uint32_t log_enabled;
	uint32_t transfer_data_chk;
	uint32_t write_back_chk;
	uint32_t transfer_size;
	uint32_t stage2_timeout;
};
struct mmc_erase_range {
	uint64_t start;
	uint64_t end;
};

struct mmc_uda_enh_area_range {
	uint32_t start_kib;
	uint32_t length_kib;
};
struct mmc_gpp_area_info {
	uint32_t length_kib;
	uint32_t enh_attr;
	uint32_t ext_attr;
};

struct mmc_param{
	int mmc_open_card;
	int mmc_erase;
	uint32_t mmc_erase_range_count;
	struct mmc_erase_range mmc_erase_range[MMC_ERASE_CNT_MAX];
#ifdef CONFIG_MMC_CREATE_GPP_AND_ENH
	int mmc_uda_enh_area;
	struct mmc_uda_enh_area_range uda_enh_area;
	int mmc_gpp_area;
	uint32_t mmc_gpp_area_count;
	struct mmc_gpp_area_info gpp_area[MMC_GPP_AREA_MAX];
#endif
};
struct nand_param{
	int nand_erase_count;
	int nand_erase;
	int nr_nand_args;
	PartitionInfo PartInfo;
	MTDPartitionInfo MTDPartInfo;
	nand_flash_param nand_params[0];
};
struct ddr_param{
	int ddr_type;
};

extern struct policy_param	*policy_args;
extern struct efuse_param	*efuse_args;
extern struct debug_param	*debug_args;
extern struct spi_param		*spi_args;
extern struct mmc_param		*mmc_args;
extern struct nand_param	*nand_args;
extern struct ddr_param		*ddr_args;


extern int buf_compare(void *s, void *d, int len, int offs);

#endif
