/*
 * Ingenic usb printer Command Explain CMD
 *
 *  Copyright (C) 2013 Ingenic Semiconductor Co., LTD.
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

void gprinter_read_callback(unsigned char *buf, int len)
{
	(void)buf;
	printf("gprinter read %d bytes data\n",len);
}

static int do_gprinter(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rc;

	if (argc <= 1)
		return cmd_usage(cmdtp);

	if(strcmp(argv[1], "start") == 0){
		usb_gprinter_register();
	}else if(strcmp(argv[1], "stop") == 0){
		usb_gprinter_unregister();
	}else{
		rc = cmd_usage(cmdtp);
	}
	return rc;
}

U_BOOT_CMD(
	gprinter, 2, 1, do_gprinter,
	"enter gprinter mode",
	"start: connect gprinter to the controller.\n"
	"gprinter stop: disconnect gprinter to the controller."
);


