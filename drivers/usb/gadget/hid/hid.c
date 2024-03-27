/*
 * hid.c -- HID Composite driver
 *
 * Based on multi.c
 *
 * Copyright (C) 2010 Fabien Chouteau <fabien.chouteau@barco.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <errno.h>
#include <common.h>
#include <malloc.h>

#include <linux/usb/ch9.h>
#include <linux/list.h>
#include <linux/usb/composite.h>
#include <linux/usb/g_hid.h>

#include "../gadget_chips.h"
#include "f_hid.h"

#define DRIVER_DESC		"HID Gadget"
#define DRIVER_VERSION		"2010/03/16"

/*-------------------------------------------------------------------------*/

#define HIDG_VENDOR_NUM		0x0525	/* XXX NetChip */
#define HIDG_PRODUCT_NUM	0xa4ac	/* Linux-USB HID gadget */

/*-------------------------------------------------------------------------*/

enum {
        USB_GADGET_MANUFACTURER_IDX     = 0,
        USB_GADGET_PRODUCT_IDX,
        USB_GADGET_SERIAL_IDX,
        USB_GADGET_FIRST_AVAIL_IDX,

};

struct hidg_func_node {
	struct list_head node;
	enum hid_type type;
	struct hidg_func_descriptor *func;
};

static LIST_HEAD(hidg_func_list);

/*-------------------------------------------------------------------------*/
USB_GADGET_COMPOSITE_OPTIONS();

static struct usb_device_descriptor device_desc = {
	.bLength =		sizeof device_desc,
	.bDescriptorType =	USB_DT_DEVICE,

	.bcdUSB =		cpu_to_le16(0x0200),

	.bDeviceClass =		USB_CLASS_PER_INTERFACE,
	.bDeviceSubClass =	0,
	.bDeviceProtocol =	0,

	/* Vendor and product id can be overridden by module parameters.  */
	.idVendor =		cpu_to_le16(HIDG_VENDOR_NUM),
	.idProduct =		cpu_to_le16(HIDG_PRODUCT_NUM),
	/* NO SERIAL NUMBER */
	.bNumConfigurations =	1,
};

static struct usb_otg_descriptor otg_descriptor = {
	.bLength =		sizeof otg_descriptor,
	.bDescriptorType =	USB_DT_OTG,

	/* REVISIT SRP-only hardware is possible, although
	 * it would not be called "OTG" ...
	 */
	.bmAttributes =		USB_OTG_SRP | USB_OTG_HNP,
};

static const struct usb_descriptor_header *otg_desc[] = {
	(struct usb_descriptor_header *) &otg_descriptor,
	NULL,
};


/* string IDs are assigned dynamically */
static struct usb_string strings_dev[] = {
	[USB_GADGET_MANUFACTURER_IDX].s = "",
	[USB_GADGET_PRODUCT_IDX].s = DRIVER_DESC,
	[USB_GADGET_SERIAL_IDX].s = "",
	{  } /* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};



/****************************** Configurations ******************************/
extern int hidg_bind_config(struct usb_configuration *c, struct hidg_func_descriptor *fdesc, const enum hid_type index);

static int do_config(struct usb_configuration *c)
{
	struct hidg_func_node *e;
	int status = 0;

	if (gadget_is_otg(c->cdev->gadget)) {
		c->descriptors = otg_desc;
		c->bmAttributes |= USB_CONFIG_ATT_WAKEUP;
	}

	list_for_each_entry(e, &hidg_func_list, node) {
		status = hidg_bind_config(c, e->func, e->type);
		if (status)
			break;
	}

	return status;
}

static struct usb_configuration config_driver = {
	.label			= "HID Gadget",
	.bConfigurationValue	= 1,
	.bmAttributes		= USB_CONFIG_ATT_SELFPOWER,
	.bind                   = do_config,
};

/****************************** Gadget Bind ******************************/

static int hid_bind(struct usb_composite_dev *cdev)
{
	struct usb_gadget *gadget = cdev->gadget;
	struct list_head *tmp;
	int gcnum;
	int status, funcs = 0;

	list_for_each(tmp, &hidg_func_list)
		funcs++;

	if (!funcs)
		return -ENODEV;

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */

	status = usb_string_ids_tab(cdev, strings_dev);
	if (status < 0)
		return status;
	device_desc.iManufacturer = strings_dev[USB_GADGET_MANUFACTURER_IDX].id;
	device_desc.iProduct = strings_dev[USB_GADGET_PRODUCT_IDX].id;
	/* register our configuration */
	status = usb_add_config(cdev, &config_driver);
	if (status < 0)
		return status;
	gcnum = usb_gadget_controller_number(gadget);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(0x0200 + gcnum);
	else {
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	}

	usb_gadget_connect(gadget);

	PRINT_DEBUG(DRIVER_DESC ", version: " DRIVER_VERSION "\n");

	return 0;
}

static int hid_unbind(struct usb_composite_dev *cdev)
{
	struct usb_gadget *gadget = cdev->gadget;

	usb_gadget_disconnect(gadget);

	return 0;
}

static int hidg_plat_driver_probe(struct hidg_func_descriptor *hidg_func, const enum hid_type type)
{
	struct hidg_func_descriptor *func = hidg_func;
	struct hidg_func_node *entry;

	if (!func) {
		fprintf(stderr, "Platform data missing\n");
		return -1;
	}

	entry = malloc(sizeof(*entry));
	if (!entry)
		return -ENOMEM;

	entry->func = func;
	entry->type = type;
	list_add_tail(&entry->node, &hidg_func_list);

	return 0;
}

static int hidg_plat_driver_remove(void)
{
	struct hidg_func_node *e, *n;

	list_for_each_entry_safe(e, n, &hidg_func_list, node) {
		list_del(&e->node);
		free(e);
	}

	return 0;
}


/****************************** Some noise ******************************/


static struct usb_composite_driver hidg_driver = {
	.name		= "g_hid",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.bind		= hid_bind,
	.unbind		= hid_unbind,
};


int jz_usb_hid_register(const struct hidg_func_descriptor *hid_des, const enum hid_type type)
{
	hidg_plat_driver_probe(hid_des, type);

	return usb_composite_register(&hidg_driver);
}

void jz_usb_hid_unregister(void)
{
	usb_composite_unregister(&hidg_driver);

	hidg_plat_driver_remove();
}

