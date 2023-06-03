/*
 * Ingenic Fastboot Command Explain CMD
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

#include <linux/usb/g_hid.h>

#ifdef DEBUG
#define PRINT_DEBUG(frame, argv...) fprintf(stderr, "%s %s %d " frame, __FILE__, __func__, __LINE__, ##argv)
#else
#define PRINT_DEBUG(frame, argv...)
#endif

enum function {
	OUTPUT_LOW,
	OUTPUT_HIGH,
	INPUT,
	LOW_INT,
	HIGH_INT,
	FALL_INT,
	RISE_INT,
};

enum gpio_group {
	GPIO_GROUP_A,
	GPIO_GROUP_B,
	GPIO_GROUP_C,
	GPIO_GROUP_D,
};

enum hid_type {
	KEYBOARD,
	MOUSE,
	HID_TYPE_MAX,
};

struct hid_info {
	const unsigned char *buff;
	const unsigned char *buff_clear;
	const struct hidg_func_descriptor *hid_f;
	int len;
	enum hid_type type;

};

extern int init_gpio(const enum gpio_group gpio_group_t, const int offset, const enum function f);
extern int is_press_down(const enum gpio_group gpio_group_t, const int offset);
extern int jz_usb_hid_register(const struct hidg_func_descriptor *hid_des, const enum hid_type type);
extern int usb_gadget_handle_interrupts(void);
extern void jz_usb_hid_unregister(void);
extern int submit_status(const unsigned char *buff, const int len, const enum hid_type label);
extern ssize_t f_hidg_read(const enum hid_type label);

static  struct hidg_func_descriptor keyboard_hid_data = {
	.subclass               = 0, /* No subclass */
	.protocol               = 1, /* Keyboard */
	.report_length          = 8,
	.report_desc_length     = 63,
	.report_desc            = {
		0x05, 0x01,     /* USAGE_PAGE (Generic Desktop)           */
		0x09, 0x06,     /* USAGE (Keyboard)                       */
		0xa1, 0x01,     /* COLLECTION (Application)               */
		0x05, 0x07,     /*   USAGE_PAGE (Keyboard)                */
		0x19, 0xe0,     /*   USAGE_MINIMUM (Keyboard LeftControl) */
		0x29, 0xe7,     /*   USAGE_MAXIMUM (Keyboard Right GUI)   */
		0x15, 0x00,     /*   LOGICAL_MINIMUM (0)                  */
		0x25, 0x01,     /*   LOGICAL_MAXIMUM (1)                  */
		0x75, 0x01,     /*   REPORT_SIZE (1)                      */
		0x95, 0x08,     /*   REPORT_COUNT (8)                     */
		0x81, 0x02,     /*   INPUT (Data,Var,Abs)                 */
		0x95, 0x01,     /*   REPORT_COUNT (1)                     */
		0x75, 0x08,     /*   REPORT_SIZE (8)                      */
		0x81, 0x03,     /*   INPUT (Cnst,Var,Abs)                 */
		0x95, 0x05,     /*   REPORT_COUNT (5)                     */
		0x75, 0x01,     /*   REPORT_SIZE (1)                      */
		0x05, 0x08,     /*   USAGE_PAGE (LEDs)                    */
		0x19, 0x01,     /*   USAGE_MINIMUM (Num Lock)             */
		0x29, 0x05,     /*   USAGE_MAXIMUM (Kana)                 */
		0x91, 0x02,     /*   OUTPUT (Data,Var,Abs)                */
		0x95, 0x01,     /*   REPORT_COUNT (1)                     */
		0x75, 0x03,     /*   REPORT_SIZE (3)                      */
		0x91, 0x03,     /*   OUTPUT (Cnst,Var,Abs)                */
		0x95, 0x06,     /*   REPORT_COUNT (6)                     */
		0x75, 0x08,     /*   REPORT_SIZE (8)                      */
		0x15, 0x00,     /*   LOGICAL_MINIMUM (0)                  */
		0x25, 0x65,     /*   LOGICAL_MAXIMUM (101)                */
		0x05, 0x07,     /*   USAGE_PAGE (Keyboard)                */
		0x19, 0x00,     /*   USAGE_MINIMUM (Reserved)             */
		0x29, 0x65,     /*   USAGE_MAXIMUM (Keyboard Application) */
		0x81, 0x00,     /*   INPUT (Data,Ary,Abs)                 */
		0xc0            /* END_COLLECTION                         */
	},
};

const static unsigned char keyboard_buff[] = {0x00, 0x00, 0x59, 0x5a, 0x5b, 0x00, 0x00, 0x00}; //按键 123
const static unsigned char keyboard_buff_clear[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};



static struct hidg_func_descriptor mouse_hid_data = {
	.subclass = 0,  /*NO SubClass*/
	.protocol = 2,  /*Mouse*/
	.report_length = 6,
	.report_desc_length = 79,
	.report_desc={
		0x05,0x01,  /*Usage Page (Generic Desktop Controls)*/
		0x09,0x02,  /*Usage (Mouse)*/
		0xa1,0x01,  /*Collction (Application)*/
		0x09,0x01,  /*Usage (pointer)*/
		0xa1,0x00,  /*Collction (Physical)*/
		0x05,0x09,  /*Usage Page (Button)*/
		0x19,0x01, /*UsageMinimum(1)*/
		0x29,0x05,  /*Usage Maximum(3) */
		0x15,0x00,  /*Logical Minimum(1)*/
		0x25,0x01,  /*Logical Maximum(1)*/
		0x95,0x05,  /*Report Count(5)  */
		0x75,0x01,  /*Report Size(1)*/
		0x81,0x02,  /*Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)*/
		0x95,0x01,  /*Report Count(1)*/
		0x75,0x03,  /*Report Size(3) */
		0x81,0x01,  /*Input (Cnst,Ary,Abs) */
		0x05,0x01,  /*Usage Page (Generic Desktop Controls)*/
		0x09,0x30,  /*Usage(x)*/
		0x09,0x31,  /*Usage(y)*/
		0x16,0x00,0xF8,  /*Logical Minimum (-2048)*/
		0x26,0xFF,0x07,  /*Logical Maximum (2047)*/
		0x75,0x0C, /*Report Size(12)*/
		0x95,0x02, /*Report Count(2) */
		0x81,0x06, /*Input (Data,Var,Rel,NWrp,Lin,Pref,NNul,Bit)*/
		0x09,0x38,  /*Usage(Wheel)*/
		0x15,0x81,  /*Logical Minimum(-127)*/
		0x25,0x7f,  /*Logical Maximum(127)*/
		0x75,0x08,  /*Report Size(8)*/
		0x95,0x01,  /*Report Count(1)  */
		0x81,0x06,  /*Input (Data,Var,Rel,NWrp,Lin,Pref,NNul,Bit)*/
		0x05,0x0C, /*Usage Page (Consumer Devices) */
		0x0A,0x38,0x02, /*Usage (AC Pan)*/
		0x95,0x01, /*Report Count (1) */
		0x75,0x08, /*Report Size (8)*/
		0x15,0x81, /*Logical Minimum (-127) */
		0x25,0x7F, /*Logical Maximum (127)*/
		0x81,0x06, /*Input (Data,Var,Rel,NWrp,Lin,Pref,NNul,Bit) */
		0xc0,   /*End Collection*/
		0xc0    /*End Collection*/
	}
};

const static unsigned char mouse_buff[] = {0x01, 0x00, 0x00, 0x00}; /*左键按下*/
const static unsigned char mouse_buff_clear[] = {0x00, 0x00, 0x00, 0x00};



const static struct hid_info hid_info_t[HID_TYPE_MAX] = {
	[KEYBOARD] = {
		.buff = keyboard_buff,
		.buff_clear = keyboard_buff_clear,
		.hid_f = &keyboard_hid_data,
		.len = sizeof(keyboard_buff),
		.type = KEYBOARD,
	},

	[MOUSE] = {
		.buff = mouse_buff,
		.buff_clear = mouse_buff_clear,
		.hid_f = &mouse_hid_data,
		.len = sizeof(mouse_buff),
		.type = MOUSE,
	},
};

#define KEY_NUM 11
const static enum hid_type hid_current_type = KEYBOARD;

static int do_hid(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	init_gpio(GPIO_GROUP_A, KEY_NUM, INPUT);

	jz_usb_hid_register(hid_info_t[hid_current_type].hid_f, hid_info_t[hid_current_type].type);

	while(1)
	{
		if (ctrlc())
			goto exit;

		usb_gadget_handle_interrupts();

		f_hidg_read(hid_info_t[hid_current_type].type);

		if (is_press_down(GPIO_GROUP_A, KEY_NUM) == 0) {
			submit_status(hid_info_t[hid_current_type].buff, hid_info_t[hid_current_type].len, hid_info_t[hid_current_type].type);
			submit_status(hid_info_t[hid_current_type].buff_clear, hid_info_t[hid_current_type].len, hid_info_t[hid_current_type].type);
			PRINT_DEBUG("key is pressed\n");
		}
	}

exit:
	jz_usb_hid_unregister();

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	hid, 1, 1, do_hid,
	"enter hid mode",
	"enter hid mode"
);

