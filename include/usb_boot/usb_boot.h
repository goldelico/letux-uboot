/*
 * usb_boot.h
 *
 * Internal BootROM c-code for JZ4780B.
 * Supports booting USB device.
 *
 * Copyright (C) 2005 - 2012 Ingenic Semiconductor Inc.
 *
 * Authors: cli <cli@ingenic.cn>
 */
#ifndef __USB_BOOT_H__
#define __USB_BOOT_H__


#ifndef le16
#define le16 u16
#endif

/* Standard requests, for the bRequest field of a SETUP packet. */
#define USB_REQ_GET_STATUS		0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE		0x03
#define USB_REQ_SET_ADDRESS		0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME		0x0C

/* Vendor requests. */
#define EP0_GET_CPU_INFO		0x00
#define EP0_SET_DATA_ADDRESS		0x01
#define EP0_SET_DATA_LENGTH		0x02
#define EP0_FLUSH_CACHES		0x03
#define EP0_PROG_START1			0x04
#define EP0_PROG_START2			0x05
#define EP0_BOOTROM_TEST		0x07
#define EP0_EXIT_LOOP                   0x08

/* Descriptor types ... USB 2.0 spec table 9.5 */
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05
#define USB_DT_DEVICE_QUALIFIER		0x06
#define USB_DT_OTHER_SPEED_CONFIG	0x07
#define USB_DT_INTERFACE_POWER		0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG			0x09
#define USB_DT_DEBUG			0x0a
#define USB_DT_INTERFACE_ASSOCIATION	0x0b


#define USB_DT_DEVICE_SIZE		18
#define USB_DT_CONFIG_SIZE		9

#define MIN(m, n)	(((m) < (n)) ? (m) : (n))

/*---------------------------------------------------------*/
typedef struct usb_status {
	u8 *addr_in;		// save of operate data address
	u8 *addr_out;		// save of operate data address
	u32 setup_packet[2];
	int length;		// save of operate data length
	u32 fw_len;		// save firmware length
} USB_STATUS;

/*---------------------------------------------------------*/

static u8 cpu_info_data[] = "ingenic";

int usb_boot_loop(void);

#endif  /* __USB_BOOT_H__ */
