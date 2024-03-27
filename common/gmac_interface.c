#include <common.h>
#include <nand.h>
#include <net.h>
#include <netdev.h>
#include <asm/gpio.h>
#include <asm/arch/clk.h>

static const int hex2bin_tbl[256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

/* Does the reverse of bin2hex but does not allocate any ram */
int hex2bin(unsigned char *p, const unsigned char *hexstr, size_t len)
{
	int nibble1, nibble2;
	unsigned char idx;
	int ret = -1;

	while (*hexstr && len) {
		if (!hexstr[1]) {
			printf("hex2bin str truncated\n");
			return ret;
		}

		idx = *hexstr++;
		nibble1 = hex2bin_tbl[idx];
		idx = *hexstr++;
		nibble2 = hex2bin_tbl[idx];

		if ((nibble1 < 0) || (nibble2 < 0)) {
			printf("hex2bin scan failed\n");
			return ret;
		}

		*p++ = (((unsigned char)nibble1) << 4) | ((unsigned char)nibble2);
		--len;
	}

	if (len == 0)
		ret = 0;
	return ret;
}

void gmac_set_ext_clk(void){
#ifdef CONFIG_NET_EXT_CLK
	clk_set_rate(MACPHY, CONFIG_GMAC_EXT_CLK);
	gpio_set_func(CONFIG_GMAC0_CRLT_PORT, CONFIG_GMAC0_CRTL_PORT_INIT_FUNC,\
		      CONFIG_GMAC0_CRLT_PORT_PINS);
	gpio_set_func(CONFIG_GMAC1_CRLT_PORT, CONFIG_GMAC1_CRTL_PORT_INIT_FUNC,\
		      CONFIG_GMAC1_CRLT_PORT_PINS);
#endif

	return;
}

#ifdef CONFIG_SYS_MAX_NAND_DEVICE
extern nand_info_t nand_info[CONFIG_SYS_MAX_NAND_DEVICE];
#endif
int gmac_set_nand_mac(void){
#if defined(CONFIG_MTD_DEVICE) && defined(CONFIG_XBURST2_GEM_NAND_MAC_OFFSET)
    unsigned char gem_buf[CONFIG_XBURST2_GEM_NAND_MAC_LEN * 2 + 1] = {0};
    nand_info_t *nand = &nand_info[0];
    unsigned char ethaddr[6];
    int read_size = CONFIG_XBURST2_GEM_NAND_MAC_LEN*2;
	char *get_bootargs = NULL;
	char set_bootargs[512] = {0};

    if (nand_read_skip_bad(nand, CONFIG_XBURST2_GEM_NAND_MAC_OFFSET, &read_size,
			   NULL, CONFIG_XBURST2_GEM_NAND_MAC_OFFSET+CONFIG_XBURST2_GEM_NAND_MAC_LEN*2,
			   gem_buf)) {
        printf("Failed to get MAC address!\n");
        return -1;
    }
	/*printf("read nand mac gem_buf:%s\n", gem_buf);*/
    if (hex2bin(ethaddr, gem_buf, 6)) {
        printf("hex2bin mac address error\n");
        return -1;
    }

    if (is_valid_ether_addr(ethaddr) == 0){
		printf("mac invalid\n");
		return -1;
    }
    
	get_bootargs = getenv("bootargs");
	sprintf(set_bootargs, "%s %s%pM", get_bootargs, "ethaddr1=", ethaddr);

	setenv("bootargs", set_bootargs);
#endif

	return 0;
}
