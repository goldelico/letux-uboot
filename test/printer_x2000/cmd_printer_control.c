#include <common.h>
#include <command.h>
#include <config.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/jz_uart.h>
#include <asm/arch/base.h>
#include <asm/arch/cpm.h>
#include "Ingenic_logo_384x128_1bit.h"

unsigned int firmware[] ={
#include "loader.hex"
};
static struct printer_param{
	void (*mcu_init)(void);
	int (*mcu_irqhandler)(int);
	int (*send)(unsigned int *buffer,int stride,int lines);
	int (*get_state)(void);
	void *priv;
}*printer = NULL;

typedef void (*LOADER)(struct printer_param*,void*);

void load_firmware(void)
{
	if(!printer){
		LOADER f = (LOADER)firmware;
		printer = (struct printer_param*)firmware;
		f(printer,printf);

		printf("%p %p %p\n",firmware,printer->mcu_init,printer->mcu_irqhandler);
	}
}

void my_printer (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("mcu printer init\n");
	int count = 0;
	int ret = 0;
	load_firmware();
	printer->mcu_init();
	ret = printer->send((unsigned int *)printer_logo_bitmap,384/8,sizeof(printer_logo_bitmap)/(384/8));
	if(ret == 0)
		count++;
	printf("ret = %d\n",ret);

	while(1){
		int d;
		d = printer->mcu_irqhandler(1);
		if(d == -1){
			break;
		}
	}
	mdelay(3000);
	printf("finish...\n");
}

U_BOOT_CMD(my_printer,1,2,my_printer,
		           "my printer command","jim gao add printer command!\n");
