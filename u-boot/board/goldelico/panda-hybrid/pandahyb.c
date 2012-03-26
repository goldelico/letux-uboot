#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include "../../ti/panda/panda.h"
#include "pandahyb.h"

/*
 somehow redefine:
 const struct omap_sysinfo sysinfo = {
	"Board: OMAP4 Panda\n"
};
*/

// tamper with set_muxconf_regs() to extend its functionality

void do_set_mux(u32 base, struct pad_conf_entry const *array, int size);
void set_muxconf_regs_inherited(void);

void set_muxconf_regs(void)
{
	set_muxconf_regs_inherited();	// call "superclass method"
	do_set_mux(CONTROL_PADCONF_CORE, hybrid_padconf_array,
			   sizeof(hybrid_padconf_array) /
			   sizeof(struct pad_conf_entry));
	
}

// rename original definition in panda.c
#define set_muxconf_regs set_muxconf_regs_inherited

#include "../../ti/panda/panda.c"
