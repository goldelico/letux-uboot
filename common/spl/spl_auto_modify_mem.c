/*
 * This file is used for adapting DDR during the SPL phase
 */
#include <common.h>

#if (CONFIG_BOOTARGS_AUTO_MODIFY == 1)
unsigned char linux_argp[200];
static char* linux_cmdline_set(char *arg, const char *value, size_t len)
{
	memset(linux_argp, 0, sizeof(linux_argp));
	memcpy(linux_argp, arg, strlen(arg));
	strcat(linux_argp, " ");
	strcat(linux_argp, value);

	return linux_argp;
}


static char *board_process_mem_arg(char *arg)
{
	unsigned long ram_size = initdram(0) >> 20;

	if(ram_size == 8) {
#ifdef CONFIG_BOOTARGS_MEM_8M
		arg = linux_cmdline_set(arg, CONFIG_BOOTARGS_MEM_8M, strlen(CONFIG_BOOTARGS_MEM_8M));
#endif
        } else if(ram_size == 16) {
#ifdef CONFIG_BOOTARGS_MEM_16M
		arg = linux_cmdline_set(arg, CONFIG_BOOTARGS_MEM_16M, strlen(CONFIG_BOOTARGS_MEM_16M));
#endif
        } else if(ram_size == 32) {
#ifdef CONFIG_BOOTARGS_MEM_32M
		arg = linux_cmdline_set(arg, CONFIG_BOOTARGS_MEM_32M, strlen(CONFIG_BOOTARGS_MEM_32M));
#endif
        } else if(ram_size == 64) {
#ifdef CONFIG_BOOTARGS_MEM_64M
		arg = linux_cmdline_set(arg, CONFIG_BOOTARGS_MEM_64M, strlen(CONFIG_BOOTARGS_MEM_64M));
#endif
	} else if(ram_size == 128) {
#ifdef CONFIG_BOOTARGS_MEM_128M
		arg = linux_cmdline_set(arg, CONFIG_BOOTARGS_MEM_128M, strlen(CONFIG_BOOTARGS_MEM_128M));
#endif
	} else if(ram_size == 256) {
#ifdef CONFIG_BOOTARGS_MEM_256M
		arg = linux_cmdline_set(arg, CONFIG_BOOTARGS_MEM_256M, strlen(CONFIG_BOOTARGS_MEM_256M));
#endif
	} else if(ram_size == 512) {
#ifdef CONFIG_BOOTARGS_MEM_512M
		arg = linux_cmdline_set(arg, CONFIG_BOOTARGS_MEM_512M, strlen(CONFIG_BOOTARGS_MEM_512M));
#endif
	} else {
	}

	return arg;

}
#endif


char *spl_board_process_bootargs(char *arg)
{
#if (CONFIG_BOOTARGS_AUTO_MODIFY == 1)
	arg = board_process_mem_arg(arg);
#endif

	return arg;
}



