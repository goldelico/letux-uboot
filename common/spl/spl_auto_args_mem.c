#include <common.h>

/* number to string */
static unsigned int int_to_string(char *str, unsigned int value, int base)
{
	int j = 19;
	int data, len;
	char buf[20];

	if (base == 10) {
		do {
			buf[j--] = (value % 10) + '0';
			value = value / 10;
		} while (value);
	}

	if (base == 16) {
		do {
			data = value % 16;

			if (data > 9)
				buf[j--] = 'a' + data - 10;
			else
				buf[j--] = '0' + data;

			value = value >> 4;
		} while (value);
	}

	len = 19 - j;

	memcpy(str, buf + j + 1, len);

	return len;
}

static unsigned int string_copy(char *dest, char *src, unsigned int size)
{
	memcpy(dest, src, size);

	return size;
}

static char *add_mem(char *str, char *tag,
	unsigned int start, unsigned int size)
{
	if (start != 0)
		str += string_copy(str, " ", 1);
	str += string_copy(str, tag, strlen(tag));
	str += int_to_string(str, size, 10);
	str += string_copy(str, "M@0x", 4);
	str += int_to_string(str, start*1024*1024, 16);

	return str;
}

static char* process_mem_bootargs(char *cmdargs, int ram_size)
{
	char *args_mem = NULL;
	char *args_mem_end = NULL;
	unsigned int mem_start;
	unsigned int rmem_start;
	unsigned int rtos_start;
	unsigned int real_size = ram_size;

	args_mem = strstr(cmdargs, "[mem-start");

	args_mem_end = strstr(cmdargs, "mem-end]") + strlen("mem-end]");

	if (ram_size >= 256)
		ram_size = 256;

	ram_size = ram_size - CONFIG_RMEM_MB - CONFIG_RTOS_SIZE_MB;

	/* mem=xxxM@0x0*/
	args_mem = add_mem(args_mem, "mem=", 0, ram_size);

	if (CONFIG_RMEM_MB)
		args_mem = add_mem(args_mem, "rmem=", ram_size, CONFIG_RMEM_MB);

	if (CONFIG_RTOS_SIZE_MB)
		args_mem = add_mem(args_mem, "rtos_size=", ram_size+CONFIG_RMEM_MB, CONFIG_RTOS_SIZE_MB);

	if (real_size >= 256)
		args_mem = add_mem(args_mem, "mem=", 768, real_size-256);

	memmove(args_mem, args_mem_end, strlen(args_mem_end) + 1);

	return cmdargs;
}


extern unsigned int get_ddr_size(void);

char* spl_board_process_mem_bootargs(char *cmdargs)
{
	unsigned int ram_size = get_ddr_size();

	return process_mem_bootargs(cmdargs, ram_size);
}