// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Lubomir Rintel <lkundrak@v3.sk>
 *
 * Skytone Alpha 400 board support routines.
 */

#include <common.h>
#include <net.h>
#include <nand.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int mac_read_from_eeprom(void)
{
	char ethaddr[ARP_HLEN_ASCII + 1] = { 0, };
	size_t ethaddr_len = ARP_HLEN_ASCII;
	int ret;

	nand_init();
	ret = nand_read(get_nand_dev_by_index(0),
			0x00400000, &ethaddr_len, (void *)ethaddr);
	if (ret) {
		pr_err("Unable to read MAC address from NAND.\n");
		return 0; /* Not serious enough to fail the boot.  */
	}

	env_set("ethaddr", ethaddr);
	return ret;
}

int dram_init(void)
{
	gd->ram_size = SZ_128M;
	return 0;
}

int checkboard(void)
{
	puts("Board: Skytone Alpha 400\n");
	return 0;
}
