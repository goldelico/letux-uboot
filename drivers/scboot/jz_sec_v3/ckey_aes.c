#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>
#include <asm/reboot.h>
#include <asm/spl.h>
#include "secall.h"
#include "pdma.h"
#include "otp.h"
#include "aes.h"

/*
 * security boot.
 * 1. prepare a bin encrypted.
 *		|---------------|-------------------|					|
 *		| SC KEY(2048)	|  CODE encrypted   |
 *		|---------------|-------------------|					|
 * 2.
 *
 * */

#define SC_MAX_SIZE_PERTIME		(2048)
#define SC_MAGIC_SIZE			(512)
#define SC_KEY_SIZE			(1536)

#define SECURE_SCBOOT_MAGIC		0x54424353

#define SPL_KENOFFSET    ((SC_MAGIC_SIZE + 16) / 4)
#define SPL_CENOFFSET    ((SC_MAGIC_SIZE + 4) / 4)
#define SPL_NLENOFFSET   ((SC_MAGIC_SIZE + 8) / 4)
#define SPL_ULENOFFSET   ((SC_MAGIC_SIZE + 12) / 4)
#define SPL_CAESOFFSET   ((SC_MAGIC_SIZE + 24) / 4)
#define KN_OFFSET        (256)
#define KU_OFFSET        (KN_OFFSET + 256)
#define SCKEY_INFO_LEN   (256)

#define UBOOT_KENOFFSET  ((CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + 16) / 4)
#define UBOOT_CENOFFSET  ((CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + 4) / 4)
#define UBOOT_NLENOFFSET ((CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + 8) / 4)
#define UBOOT_ULENOFFSET ((CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + 12) / 4)
#define UBOOT_CAESOFFSET ((CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + 24) / 4)
#define UBOOT_LENOFFSET  ((CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE) / 4)


#if defined (CONFIG_SPL_MMC_SUPPORT) || defined(CONFIG_SPL_JZMMC_SUPPORT)
#define SPL_SCKEY_START   0x00004600 //mmc boot offset 17Kb + 512b
#define UBOOT_SCKEY_START 0x0000a600
#define IMAGE_START       0x00004400
#else
#define SPL_SCKEY_START   0x00000200
#define UBOOT_SCKEY_START 0x00006200
#define IMAGE_START       0x0
#endif

#define CRC_POSITION        9	/* 9th bytes */
#define SPL_LENGTH_POSITION 12	/* 11th */
#define HEAD_CRC_POSITION   15	/* 14th bytes */

#define NO_NEED_CKY_AES 0

#define USERKEY1_ENCRYPT  3
#define USERKEY_ENCRYPT  2
#define CHIPKEY_ENCRYPT  1

extern void pdma_wait(void);
extern void flush_cache_all(void);
extern int read_flash(unsigned int from, unsigned int len, unsigned char *buf);
extern int write_flash(unsigned int from, unsigned int len, unsigned char *buf);
extern inline phys_addr_t virt_to_phys(volatile void * address);

void ckey_aes(void)
{
	char *read_buf = NULL;
	int n_paaddr, ku_paaddr, code_paaddr;
	int ret = -1;
	read_buf = (char *)malloc(SCKEY_INFO_LEN);
	memset(read_buf, 0xff, SCKEY_INFO_LEN);

	ret = read_flash(SPL_SCKEY_START, SCKEY_INFO_LEN, read_buf);
	if (ret < 0)
		goto STEP_ERR;

	int spl_cencrypt = *((int *)read_buf + 1);
	int spl_len = *((int *)read_buf);
	int spl_nlen = *((int *)read_buf + 2);
	int spl_ulen = *((int *)read_buf + 3);
	int spl_kencrypt = *((int *)read_buf + 4);
	int is_need_ckey_s = *((int *)read_buf + 6);

	memset(read_buf, 0xff, SCKEY_INFO_LEN);
	ret = read_flash(UBOOT_SCKEY_START, SCKEY_INFO_LEN, read_buf);
	if (ret < 0)
		goto STEP_ERR;

	int uboot_len = *((int *)read_buf);
	int uboot_cencrypt = *((int *)read_buf + 1);
	int uboot_nlen = *((int *)read_buf + 2);
	int uboot_ulen = *((int *)read_buf + 3);
	int uboot_kencrypt = *((int *)read_buf + 4);
	int is_need_ckey_u = *((int *)read_buf + 6);

	free(read_buf);
	*read_buf = NULL;

	if (!(is_need_ckey_s || is_need_ckey_u))
		return;

	if (!(spl_kencrypt == USERKEY_ENCRYPT || spl_cencrypt == USERKEY_ENCRYPT ||
		uboot_kencrypt == USERKEY_ENCRYPT || uboot_cencrypt == USERKEY_ENCRYPT ||
		spl_kencrypt == USERKEY1_ENCRYPT || spl_cencrypt == USERKEY1_ENCRYPT ||
	        uboot_kencrypt == USERKEY1_ENCRYPT || uboot_cencrypt == USERKEY1_ENCRYPT))
		return;


	int image_len = uboot_len + CONFIG_UBOOT_OFFSET + SC_MAX_SIZE_PERTIME;

	read_buf = (char *)malloc(image_len);
	memset(read_buf, 0xff, image_len);

	if (read_flash(IMAGE_START, image_len, read_buf) != image_len)
		goto STEP_ERR;

#if !defined (CONFIG_SPL_MMC_SUPPORT) && !defined(CONFIG_SPL_JZMMC_SUPPORT)
	flush_cache_all();
#endif
        unsigned int *pdma_ins = (unsigned int *)pdma_wait;
	volatile unsigned int *pdma_bank0_off = (unsigned int *)TCSM_BANK0;
	int index = 0;

	for (index = 0; index < 6; index++)
		pdma_bank0_off[index] = pdma_ins[index];

	boot_up_mcu();

	if (is_need_ckey_s && (spl_kencrypt == USERKEY_ENCRYPT || spl_kencrypt == USERKEY1_ENCRYPT)) {
		spl_nlen = (spl_nlen / 8 + 15) & (~0x0f); //splen Bit to byte and 16 byte alignment and round up to an integer
		spl_ulen = (spl_ulen / 8 + 15) & (~0x0f);

		n_paaddr = virt_to_phys(read_buf + SC_MAGIC_SIZE + KN_OFFSET);
		ku_paaddr = virt_to_phys(read_buf + SC_MAGIC_SIZE + KU_OFFSET);
		if (do_aes_dma((int *)n_paaddr, (int *)n_paaddr, spl_nlen, AES_BY_UKEY, 1) != 0)
			goto STEP_ERR;

		if (do_aes_dma((int *)ku_paaddr, (int *)ku_paaddr, spl_ulen, AES_BY_UKEY, 1) != 0)
			goto STEP_ERR;

		*((int *)read_buf + SPL_KENOFFSET) = CHIPKEY_ENCRYPT;
		*((int *)read_buf + SPL_CAESOFFSET) = NO_NEED_CKY_AES;
	}

	if (is_need_ckey_s && (spl_cencrypt == USERKEY_ENCRYPT || spl_cencrypt == USERKEY1_ENCRYPT)) {
		code_paaddr = virt_to_phys(read_buf + SC_MAX_SIZE_PERTIME);
		if (do_aes_dma((int *)code_paaddr, (int *)code_paaddr, spl_len, AES_BY_UKEY, 1) != 0)
			goto STEP_ERR;

		*((int *)read_buf + SPL_CENOFFSET) = CHIPKEY_ENCRYPT;
		*((int *)read_buf + SPL_CAESOFFSET) = NO_NEED_CKY_AES;

#ifdef CONFIG_MTD_SFCNAND
		u8 crc = sec_crc(read_buf + SC_MAGIC_SIZE, spl_len + SC_KEY_SIZE);
		memcpy(read_buf + CRC_POSITION, &crc, 1);

		crc = sec_crc(read_buf, 15);
		memcpy(read_buf + HEAD_CRC_POSITION, &crc, 1);
#endif

	}

	if (is_need_ckey_u && (uboot_kencrypt == USERKEY_ENCRYPT || uboot_kencrypt == USERKEY1_ENCRYPT)) {
		uboot_nlen = (uboot_nlen / 8 + 15) & (~0x0f);
		uboot_ulen = (uboot_ulen / 8 + 15) & (~0x0f);

		n_paaddr = virt_to_phys(read_buf + CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + KN_OFFSET);
		ku_paaddr = virt_to_phys(read_buf + CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + KU_OFFSET);

		if (do_aes_dma((int *)n_paaddr, (int *)n_paaddr, uboot_nlen, AES_BY_UKEY, 1) != 0)
			goto STEP_ERR;

		if (do_aes_dma((int *)ku_paaddr, (int *)ku_paaddr, uboot_ulen, AES_BY_UKEY, 1) != 0)
			goto STEP_ERR;

		*((int *)read_buf + UBOOT_KENOFFSET) = CHIPKEY_ENCRYPT;
		*((int *)read_buf + UBOOT_CAESOFFSET) = NO_NEED_CKY_AES;
	}

	if (is_need_ckey_u && (uboot_cencrypt == USERKEY_ENCRYPT || uboot_cencrypt == USERKEY1_ENCRYPT)) {
		int ulen = (uboot_len + 15) & (~0x0f);

		code_paaddr = virt_to_phys(read_buf + CONFIG_UBOOT_OFFSET + SC_MAX_SIZE_PERTIME);

		if (do_aes_dma((int *)code_paaddr, (int *)code_paaddr, ulen, AES_BY_UKEY, 1) != 0)
			goto STEP_ERR;

		*((int *)read_buf + UBOOT_CENOFFSET) = CHIPKEY_ENCRYPT;
		*((int *)read_buf + UBOOT_CAESOFFSET) = NO_NEED_CKY_AES;
	}
	if (write_flash(IMAGE_START, image_len, read_buf) != image_len)
		goto STEP_ERR;

	memset(read_buf, 0xff , SCKEY_INFO_LEN);

	if (read_flash(SPL_SCKEY_START, SCKEY_INFO_LEN, read_buf) != SCKEY_INFO_LEN)
		goto STEP_ERR;

#if !defined (CONFIG_SPL_MMC_SUPPORT) && !defined(CONFIG_SPL_JZMMC_SUPPORT)
	flush_cache_all();
#endif

	spl_kencrypt = *((int *)read_buf + 4);
	spl_cencrypt = *((int *)read_buf + 1);
	is_need_ckey_s = *((int *)read_buf + 6);

	free(read_buf);
	read_buf = NULL;

	if ((spl_cencrypt == CHIPKEY_ENCRYPT) && (spl_kencrypt == CHIPKEY_ENCRYPT) &&
							(is_need_ckey_s == NO_NEED_CKY_AES)) {
		printf("spl chipkey aes success\n");
		_machine_restart();
	}else {
		goto ERR;
	}

	return;

STEP_ERR:
	free(read_buf);
ERR:
	printf("## ERROR ## ckey aes failed!! ##\n");
	hang();
}
