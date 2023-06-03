/*
 * Ingenic sc test command
 *
 * Copyright (c) 2013 pzqi <aric.pzqi@ingenic.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/errno.h>
#include <asm/io.h>
#include "sc.h"
#include "otp.h"
#include "jz_pdma.h"
#include "secall.h"


static unsigned int ukey[16] = {
	   0xa9901e81,0xa34a4800,0xa3ba14b4,0x7b20d6df,
	   0x68a135b8,0x851ea66b,0x492dfe47,0x3944a0dc,
	   0x9d1633d1,0x5f3cc5ee,0xcc07ff5f,0x1926e2e1,
	   0x1310a869,0x3a716c21,0xcf321748,0xe0656ef4,
};


static unsigned int nku[64 + 64 + 2] = {0x40, 0x40,
	0xed19e044,0xe39195c2,0xf9865834,0xf71f5e4c,
	0x254e42e3,0xe2152a64,0xcedf12f5,0x90368c26,
	0x8e45a322,0x32dcb23f,0xdc93ba6c,0x1f413023,
	0xc572c8d9,0xbf32d4da,0x8abfc305,0x34073d4c,
	0x68cc7971,0xd2528711,0x502aba47,0xb746dcc2,
	0xfd6ca9ef,0x502781ac,0x7865995a,0xa28061e3,
	0x83a86f69,0xe97ad4c7,0x215acc4a,0x9b1f84c3,
	0x0aacd5e5,0xebe243bb,0x07373439,0xc51bb560,
	0x1ee0b212,0x7c6adceb,0x443915e6,0x954b43a8,
	0xc81d2341,0x9e7139bf,0xa0883018,0xb9fe557f,
	0x6b7e04e5,0x670e85fe,0x824215c7,0x41beb5bb,
	0xaddd9ea1,0x0a8dbfe2,0x17a25cfe,0xdc0384c7,
	0xee3ab9aa,0x629408b6,0xb0f4f830,0xaf8cd49b,
	0x083329dc,0xe9b861ba,0x1bd6336f,0x66e0006f,
	0x673c2d51,0x046a242a,0x817724a6,0x204c3daa,
	0x705bef57,0x7c494b98,0x7a1cfc4c,0x71c0dcc3,

	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00010001
};

/*sc_test */
#define SC_OTP_SEL_UKEY 0x2
#define SC_OTP_SEL_UKEY1 0x4

static unsigned int a = 0x12345678;
static unsigned int test_hash[8] = {0};
static unsigned int b[4] = {0x12345678,};
static unsigned int c[4] = {0};
static unsigned int k[8] = {0x01};
static unsigned int chipkey[8] = {0};
static unsigned int ukey_en[8] = {0};
static unsigned int ukey1_en[8] = {0};
static unsigned int nkusig_en[8] = {0};
static unsigned int nkusig_cmp[8] = {0};

static unsigned int ckey[8] = {0};
static unsigned int userkey[8] = {0};
static unsigned int userkey1[8] = {0};
static unsigned int nkusig[8] = {0};

static void bitcpy(const unsigned int *s,unsigned int *d,
			        const int ss,const int ds,int bsz)
{
	int ss_int = ss / 32;
	int ss_bit = ss % 32;

	int ds_int = ds / 32;
	int ds_bit = ds % 32;
#define MMIN(a,b) (a) > (b) ? (b) : (a)
	while(bsz != 0){
		unsigned int src,dst,tmp,bmsk,smsk,dmsk;
		int min = MMIN(32 - ss_bit,32 - ds_bit);
		min = MMIN(min,bsz);
		bmsk = 0xffffffff >> (32 - min);
		src = s[ss_int] >> ss_bit;
		src &= bmsk;

		dst = d[ds_int];
		dst &= ~(bmsk << ds_bit);
		dst |= src << ds_bit;

		d[ds_int] = dst;
		ds_bit += min;
		if(ds_bit >= 32){
			ds_int++;
			ds_bit = 0;

		}
		ss_bit += min;
		if(ss_bit >= 32){
			ss_int++;
			ss_bit = 0;

		}
		//              printf("bsz = %d min = %d\n",bsz,min);
		bsz -= min;

	}
}

static int decode(unsigned int *s,int bits,unsigned int *d)
{
	int i,p,j;
	int xor = 0;
	i = 0;
	p = 0;
	j = 0;
	while(j < bits){
		int curindex,bsz;
		curindex = (1 << p);
		if(j + 1 != curindex){
			bsz = (curindex - j - 1);
			if((bits - j) < bsz){
				bsz = bits - j;

			}
			bitcpy(s,d,j,i,bsz);
			i += bsz;
			j += bsz;
		}else{
			j++;
			p++;
		}
	}

	return i;
}

static int redundancy_rd(void)
{
	REG32(EFUSE_REG_DAT1) = 0;

	REG32(EFUSE_REG_CTRL) = (0x1f << EFUSE_REGOFF_CRTL_ADDR) | (1 << EFUSE_REGOFF_CRTL_LENG) | EFUSE_REG_CTRL_RWL;
	REG32(EFUSE_REG_CTRL) |= EFUSE_REG_CTRL_RDEN;

	while(!(REG32(EFUSE_REG_STAT) & EFUSE_REG_STAT_RDDONE));
	REG32(EFUSE_REG_CTRL) = 0;
	printf("rir1 = 0x%08x\n", REG32(EFUSE_REG_DAT1));
	printf("rir2 = 0x%08x\n", REG32(EFUSE_REG_DAT1 + 4));
}


static int read_ckey()
{
	unsigned int ret, i;
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();
	volatile unsigned int *output = (volatile unsigned int *)(MCU_TCSM_OUTDATA);
	memset(output, 0, BANK_SIZE);

	redundancy_rd();
	args->arg[0] = SC_OTP_SEL_CKEY;
	args->arg[1] = MCU_TCSM_PADDR(output);
	ret = secall(args,SC_FUNC_SCOTP,0,1);
	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC) {
		fprintf(stderr, "%s failed\n", __func__);
		return -1;
	}

	printf("key setted(actual): %08x-%08x-%08x-%08x\n", output[0], output[1], output[2], output[3]);
	printf("                    %08x-%08x-%08x-%08x-%08x\n", output[4], output[5], output[6], output[7], output[8]);
	decode(output, 34 * 8, ckey);

	printf("ckey:\n");
	for (i = 0; i < 8; i++) {
		printf("%04x ", ckey[i]);
	}

	printf("\nckey:end\n");

	return 0;
}



static int read_ukey(int ukey_flag, unsigned int *ukey)
{
	unsigned int ret, i;
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();
	volatile unsigned int *output = (volatile unsigned int *)(MCU_TCSM_OUTDATA);

	memset(output, 0, BANK_SIZE);

	redundancy_rd();
	args->arg[0] = ukey_flag;
	args->arg[1] = MCU_TCSM_PADDR(output);
	ret = secall(args,SC_FUNC_SCOTP,0,1);

	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC) {
		fprintf(stderr, "%s failed\n", __func__);
		return -1;
	}

	printf("key setted(actual): %08x-%08x-%08x-%08x\n", output[0], output[1], output[2], output[3]);
	printf("                    %08x-%08x-%08x-%08x-%08x\n", output[4], output[5], output[6], output[7], output[8]);

	if(ukey_flag == SC_OTP_SEL_UKEY) {
		unsigned char *tmp1 = (unsigned char *)output;
		memcpy(output, &tmp1[2], 34);
	}
	printf("key setted(actual): %08x-%08x-%08x-%08x\n", output[0], output[1], output[2], output[3]);
	printf("                    %08x-%08x-%08x-%08x-%08x\n", output[4], output[5], output[6], output[7], output[8]);
	decode(output, 34 * 8, ukey);

	printf("ukey:\n");

	for (i = 0; i < 8; i++) {
		printf("%04x\n", ukey[i]);

	}

	printf("\nukey end\n");

	return 0;
}


static int read_nkusig()
{
	unsigned int ret, i;
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();
	volatile unsigned int *output = (volatile unsigned int *)(MCU_TCSM_OUTDATA);
	*((volatile unsigned int *)(MCU_TCSM_OUTDATA)) = 1;

	memset(output, 0, BANK_SIZE);

	redundancy_rd();
	args->arg[0] = SC_OTP_SEL_NKU;
	args->arg[1] = MCU_TCSM_PADDR(output);
	ret = secall(args, SC_FUNC_SCOTP, 0,1);

	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC) {
		fprintf(stderr, "%s failed\n", __func__);
		return -1;
	}

	printf("ukusig _burn(actual): %08x-%08x-%08x-%08x\n", output[0], output[1], output[2], output[3]);
	printf("                    %08x-%08x-%08x-%08x-%08x\n", output[4], output[5], output[6], output[7], output[8]);

	unsigned char *tmp1 = (unsigned char *)output;
	memcpy(output, &tmp1[2], 34);
	printf("ukusig _burn(actual): %08x-%08x-%08x-%08x\n", output[0], output[1], output[2], output[3]);
	printf("                    %08x-%08x-%08x-%08x-%08x\n", output[4], output[5], output[6], output[7], output[8]);
	decode(output, 34 * 8, nkusig);

	printf("nkusig:\n");
	for (i = 0; i < 8; i++) {
		printf("%04x ", nkusig[i]);

	}

	printf("\nnkusig end\n");

	return 0;
}



static int hash(const void *in, void *out, const size_t len)
{
	int index = 0;
	unsigned int *in_s = in;
	unsigned int *out_s = out;
	volatile unsigned int *in_t = (volatile unsigned int *)(MCU_TCSM_INDATA);
	volatile unsigned int *out_t = (volatile unsigned int *)(MCU_TCSM_OUTDATA);
	volatile struct sc_args *args = (volatile struct sc_args *)GET_SC_ARGS();
	for (index = 0; index < len; index++) {
		in_t[index] = in_s[index];
	//	printf("%x\n", in_t[index]);
	}

	args->arg[0] = len | 0x01 << 16 | 0x01 << 18 | 0x03 << 19/*hash 256*/;
	args->arg[1] = MCU_TCSM_PADDR(in_t);
	args->arg[2] = MCU_TCSM_PADDR(out_t);

	secall(args, SC_FUNC_HASH, 0,1);
	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC) {
		fprintf(stderr, "%s failed\n", __func__);
		return -1;
	}

	for (index = 0; index < 8; index++)
		out_s[index] = out_t[index];

	printf("hash\n");
	for (index = 0; index < 256 / 8 / 4; index++) {
		if (index != 0 && index % 4 == 0)
			printf("\n");

		printf("%x ", out_s[index]);
	}

	printf("\nhash end\n");
	return 0;
}

static int check_nku()
{
	unsigned int ret;
	unsigned int iLoop;
	unsigned int rsa_key_word = 0;
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();
	volatile unsigned int *tcsm_nku = (volatile unsigned int *)MCU_TCSM_NKU;

	secall(args, SC_FUNC_INIT, 0, 1);

	tcsm_nku[0] = nku[0] * 4 * 8;
	tcsm_nku[1] = nku[1] * 4 * 8;
	rsa_key_word = nku[0];

	printf("N %d BITS\n",tcsm_nku[0]);
	for (iLoop = 0; iLoop < rsa_key_word; iLoop++) {
		tcsm_nku[iLoop + 2] = nku[iLoop + 2];

		printf("%08x ", tcsm_nku[iLoop + 2]);
		if((iLoop + 1) % 4 == 0)
			printf("\n");
	}

	printf("KU %d BITS\n",tcsm_nku[1]);
	for (iLoop = 0; iLoop < rsa_key_word; iLoop++) {
		tcsm_nku[iLoop + 2 + rsa_key_word] = nku[iLoop + 2 + rsa_key_word];

		printf("%08x ", tcsm_nku[iLoop + 2 + rsa_key_word]);
		if((iLoop + 1) % 4 == 0)
			printf("\n");
	}

	REG32(EFUSE_REG_CTRL) = 0;
	args->arg[0] = MCU_TCSM_PADDR(tcsm_nku);
	ret = secall(args, SC_FUNC_CHECKNKU, 0, 1);

	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC) {
		printf("SC_FUNC_CHECKNKU failed! ret=0x%08x\n", *(volatile unsigned int *)(MCU_TCSM_RETVAL));
		return -1;
	}
	printf("SC_FUNC_CHECKNKU Success\n");
	return 0;
}

static int aes(const void *key, const void *in, void *out, const size_t len)
{
	int index = 0;
	unsigned int *in_s = in;
	unsigned int *out_s = out;
	unsigned int *key_s = key;
	volatile unsigned int *key_t = (volatile unsigned int *)(MCU_TCSM_INDATA);
	volatile struct sc_args *args = (volatile struct sc_args *)GET_SC_ARGS();
	for (index = 0; index < 8; index++) {
		key_t[index] = key_s[index];
		printf("%x\n", key_t[index]);
	}

	args->arg[0] =  0x01 << 1 /*dma*/ | 0x02 << 12;
	args->arg[1] = MCU_TCSM_PADDR(key_t);
	args->arg[2] = virt_to_phys(in_s);
	args->arg[3] = virt_to_phys(out_s);
	args->arg[4] = len;
	args->arg[6]  = 0x01 | 0x01 << 1;

	flush_cache_all();
	secall(args, SC_FUNC_AES, 0, 1);
	flush_cache_all();

	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC) {
		fprintf(stderr, "%s failed\n", __func__);
		return -1;
	}

	printf("aes\n");
	for (index = 0; index < len; index++) {
		if (index != 0 && index % 4 == 0)
			printf("\n");

		printf("%x ", out_s[index]);
	}

	printf("\naes end\n");
	return 0;
}
static unsigned int serom_code[] = {
         #include "./mcu_sc.hex"
};

static int load_serom_firmware(struct pdma_message *pdma_msg)
{
	int i;
	unsigned int *src_ptr = serom_code;
	unsigned int *dst_ptr = (unsigned int *)(TCSM_BANK1);//cacheable
	unsigned int *debug_ptr = (unsigned int *)(TCSM_BANK1);//cacheable

#define SEROM_ADDR_START        (0xf5000000)
	unsigned int serom_addr = SEROM_ADDR_START;
	unsigned int serom_end  = SEROM_ADDR_START + sizeof(serom_code);

#define FINISH(x) ((((x)) & 0x80000000) == 0x80000000)
#define RETURN(x) (((x)) & (~0x80000000))
#define TRANSFER_SIZE   (4096)

	while(serom_addr < serom_end) {
		dst_ptr = (unsigned int *)(TCSM_BANK1);//cacheable
		debug_ptr = (unsigned int *)(TCSM_BANK1);

		/* every 4096Bytes a time */
		for(i = 0; i < (TRANSFER_SIZE)/ 4;i++) {
			*dst_ptr++ = *src_ptr++;

		}

		reset_mcu();
		pdma_msg->msg_id = 0;
		pdma_msg->msg[0] = 0xf4001000;                          /*src: bank1 for mcu*/
		pdma_msg->msg[1] = serom_addr;                          /*dst: serom for mcu*/
		pdma_msg->msg[2] = TRANSFER_SIZE;                       /*cnt: size per time */
		pdma_msg->ret = 0;
		boot_up_mcu();

		printf("boot_up_mcu end\n");
		while (!FINISH(pdma_msg->ret)){
			udelay(1000*1000);
			printf("wait copy done!\n");

		}
		printf("FINISH ret end\n");

		if(RETURN(pdma_msg->ret) == 0) {
			printf("success.\n");

		}else{
			printf("fail!\n");
			return -1;

		}
                serom_addr += TRANSFER_SIZE;


	}
	printf("ok!\n");
	return 0;
}

#ifdef CONFIG_X2000_FPGA
static unsigned int pdma_code[] = {
         #include "./pdma.hex"

};

static void load_pdma_firmware()
{
	int i;
	unsigned int *src_ptr = pdma_code;
	unsigned int *dst_ptr = (unsigned int *)(TCSM_BANK0);//cacheable

	printf("xxx load pdma firmware!\n");
	for(i=0; i < ARRAY_SIZE(pdma_code); i++)
		*dst_ptr++ = *src_ptr++;
}

static int init_seboot_t()
{
	unsigned int ret, i;
	volatile struct sc_args *args;
	volatile struct pdma_message *pdma_msg;
	args = (volatile struct sc_args *)GET_SC_ARGS();
	pdma_msg = (volatile struct pdma_message *)GET_PDMA_MESSAGE();

	reset_mcu();
	load_pdma_firmware();
	boot_up_mcu();

	mdelay(30);

	if(load_serom_firmware(pdma_msg)) {
		printf("load serom firmware error!!!!!!\n");
		return -1;

	}

	return 0;
}
#endif

static int otp_r()
{
	REG32(EFUSE_REG_CTRL) = (0x1a << EFUSE_REGOFF_CRTL_ADDR | 0x0 << EFUSE_REGOFF_CRTL_LENG);

	REG32(EFUSE_REG_CTRL) |= EFUSE_REG_CTRL_RDEN;

	while(!(REG32(EFUSE_REG_STAT) & EFUSE_REG_STAT_RDDONE));

	printf("REG32(EFUSE_REG_DAT1) = %x\n",REG32(EFUSE_REG_DAT1));
	return 0;
}

static int do_sct(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int index = 0;
	if(argc < 2) {
		return CMD_RET_USAGE;
	}

	if(strcmp(argv[1], "init") == 0) {
		if(init_seboot() < 0) {
			printf("init seboot fialed.\n");
			return 0;
		}

	} else if(strcmp(argv[1], "read_ckey") == 0) {
		if (read_ckey() < 0) {
			printf("read ckey failed\n");
			return 0;
		}
		if (hash(ckey, chipkey, 8) < 0) {
			printf("get chip key failed\n");
			return 0;
		}
	} else if(strcmp(argv[1], "read_ukey") == 0) {
		if (read_ukey(SC_OTP_SEL_UKEY, userkey) < 0) {
			printf("read_ukey failed\n");
			return 0;
		}
		if (read_ukey(SC_OTP_SEL_UKEY1, userkey1) < 0) {
			printf("read_ukey1 failed\n");
			return 0;
		}

	} else if(strcmp(argv[1], "read_nkusig") == 0) {
		if (read_nkusig() < 0) {
			printf("read nkusig failed\n");
			return 0;
		}

	} else if(strcmp(argv[1], "read_bootmode") == 0) {

		otp_r();

	} else if(strcmp(argv[1], "cmp_ukey") == 0) {
		if (aes(chipkey, ukey, ukey_en, 8) < 0) {
			printf("cmp_ukey failed\n");
			return 0;
		}

	} else if(strcmp(argv[1], "cmp_ukey1") == 0) {
		if (aes(chipkey, &ukey[8], ukey1_en, 8) < 0) {
			printf("cmp_ukey1 failed\n");
			return 0;
		}

	} else if(strcmp(argv[1], "cmp_nkusig") == 0) {
		if (hash(nku + 2, nkusig_cmp, 64 + 64) < 0) {
			printf("cmp_nkusig failed\n");
			return 0;
		}

		if (aes(chipkey, nkusig_cmp, nkusig_en, 8) < 0) {
			printf("cmp_nkusig aes failed\n");
			return 0;
		}

	} else if(strcmp(argv[1], "burn_ckey") == 0) {
		if (cpu_burn_rckey() < 0) {
			printf("burn_ckey failed\n");
			return 0;
		}
	} else if(strcmp(argv[1], "burn_ukey") == 0) {
		if (cpu_burn_ukey(ukey) < 0) {
			printf("burn_ukey failed\n");
			return 0;
		}

	} else if(strcmp(argv[1], "burn_nkusig") == 0) {
		if (cpu_burn_nku(nku, 520) < 0) {
			printf("burn_nkusig failed\n");
			return 0;
		}
	} else if(strcmp(argv[1], "burn_scen") == 0) {
		if (cpu_burn_secboot_enable() < 0) {
			printf("burn_sc_en failed\n");
			return 0;
		}
	} else if(strcmp(argv[1], "check_nku") == 0) {
		if (check_nku() < 0) {
			printf("check_nku failed\n");
			return 0;
		}
	} else if (strcmp(argv[1], "test") == 0) {
		//hash(&a, test_hash, 1);
		for (index = 0; index < 4; index++)
			printf("%x\n", b[index]);
		aes(k, b, c, 4);
	}else {
		printf("cmd error!!\n");
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(sct, 2, 1, do_sct,
	"Ingenic security test program",
	"sct init    -- load firmware to pdma and se-rom.\n"
	"sct xxx     -- test to be add!!\n"
);
