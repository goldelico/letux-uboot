/*
 * (C) Copyright 2006
 * Ingenic Semiconductor, <jlwei@ingenic.cn>
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
#include <net.h>
#include <asm/mipsregs.h>
#include <asm/jz4730.h>

static void gpio_init(void)
{
	__harb_usb0_uhc();
	__gpio_as_uart0();
//	__gpio_as_uart1();
	__gpio_as_uart2();
//	__gpio_as_uart3();
	__gpio_as_emc();
	__gpio_as_dma();
	__gpio_as_msc();
	__gpio_as_lcd_master();
	__gpio_as_usb();
	__gpio_as_ac97();
	__gpio_as_cim();
	__gpio_as_eth();

#if 0
	/* First PW_I output high */
	__gpio_as_output(GPIO_PW_I);
	__gpio_set_pin(GPIO_PW_I);

	/* Then PW_O output high */
	__gpio_as_output(GPIO_PW_O);
	__gpio_set_pin(GPIO_PW_O);

	/* Last PW_I output low and as input */
	__gpio_clear_pin(GPIO_PW_I);
	__gpio_as_input(GPIO_PW_I);

	/* make PW_I work properly */
	__gpio_disable_pull(GPIO_PW_I);
#endif
	/* USB clock enable */
	__gpio_as_output(GPIO_USB_CLK_EN);
	__gpio_set_pin(GPIO_USB_CLK_EN);

	/* LCD display off */
	__gpio_as_output(GPIO_DISP_OFF_N);
	__gpio_clear_pin(GPIO_DISP_OFF_N);
	
	__gpio_as_input(1);		// SHIFT-L key in the keybaod ,row 2 ,gpio 1
	__gpio_disable_pull(1);

//wjx 2008.3.31 add press Fn + SHIFT-L + SHIFT-R three keys to load minifs from sd card to nand flash
	__gpio_as_input(2);		// SHIFT-R key in the keybaod ,row 3 ,gpio 2
	__gpio_disable_pull(2);
	
	__gpio_as_input(6);		// F1 key  row 7 ,gpio 6
	__gpio_disable_pull(6);
	
	__gpio_as_input(7);		//Fn key row 8 ,gpio 7
	__gpio_disable_pull(7);

	__gpio_as_output(125);		// SHIFT-L SHIFT-R F1 Fn three keys in the same cur 16, gpio 125
	__gpio_clear_pin(125);
		
//hjf 2008.6.19 add press SHIFT-L + Fn + Esc  three keys to download boot-kernelx2-mac-minifs form 128-tftp-server to nand flash
	__gpio_as_input(3);		// Esc key in the keybaod ,row 4 ,gpio 3
	__gpio_disable_pull(3);

	__gpio_as_output(97);		// Esc keys in the same cur 2, gpio 97
	__gpio_clear_pin(97);

// hjf 2008.6.24 add Ctrl-L
	__gpio_as_input(6);		// Ctrl-L in the keyboard row 7 ,gpio 6 
	__gpio_disable_pull(6);

	__gpio_as_output(96);		// Ctrl-L in the keyboard cur 1 ,gpio 96
	__gpio_clear_pin(96);

//hjf 2008.6.24 add press Ctrl-L + Fn + B three keys to download bootloader from 128-tftp-server to nand flash
	__gpio_as_input(5);		// B key in the keyboard row 6 ,gpio 5
	__gpio_disable_pull(5);

	__gpio_as_output(100);		// B key in the keyboard cur 5 ,gpio 100
	__gpio_clear_pin(100);	

//hjf 2008.6.24 add press Ctrl-L + Fn + K three keys to download kernel+kernel from 128-tftp-server to nand flash
	__gpio_as_input(2);		// K key in the keyboard row 3 ,gpio 2
	__gpio_disable_pull(2);
	
	__gpio_as_output(102);		// K key in the keyboard cur 7 ,gpio 102
	__gpio_clear_pin(102);

//hjf 2008.6.24 add press Ctrl-L + Fn + M three keys to download minifs from 128-tftp-server to nand flash
	__gpio_as_input(4);		// M key in the key board row 5 ,gpio 4
	__gpio_disable_pull(4);
	
	__gpio_as_output(101);		// M key in the key board cur 6 ,gpio 101
	__gpio_clear_pin(101);

	__gpio_as_output(GPIO_CAPSLOCK_LED);		
	__gpio_clear_pin(GPIO_CAPSLOCK_LED);			//wjx open the capslock led
	
	__gpio_as_output(GPIO_NUMLOCK_LED);	
	__gpio_set_pin(GPIO_NUMLOCK_LED);			//wjx close the numlock led
	
	__gpio_as_output(GPIO_NETWORK_LED);	
	__gpio_set_pin(GPIO_NETWORK_LED);			//wjx close the network led

#if 0
	/* No backlight */
	__gpio_as_output(94); /* PWM0 */
	__gpio_clear_pin(94);
	/* RTC IRQ input */
	__gpio_as_input(GPIO_RTC_IRQ);

#endif

#if 0
	/* CHARG_STAT input */
	__gpio_as_input(GPIO_CHARG_STAT);
	__gpio_disable_pull(GPIO_CHARG_STAT);
#endif
}

//----------------------------------------------------------------------
// board early init routine

void board_early_init(void)
{
	gpio_init();
}

void board_led(char ch)
{
}

//----------------------------------------------------------------------
// U-Boot common routines

int checkboard(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	printf("Board: Ingenic PMP Ver 1.x (CPU Speed %d MHz)\n",
	       gd->cpu_clk/1000000);

	return 0; /* success */
}
