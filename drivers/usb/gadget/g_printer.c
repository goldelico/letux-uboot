/*
 * printer.c -- Printer gadget driver
 *
 * Copyright (C) 2003-2005 David Brownell
 * Copyright (C) 2006 Craig W. Nadler
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <errno.h>
#include <linux/list.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/ctype.h>

#include <asm/byteorder.h>
#include <asm/unaligned.h>

#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>

#include "gadget_chips.h"
#include "composite.c"

USB_GADGET_COMPOSITE_OPTIONS();

#define GFP_ATOMIC ((gfp_t) 0)

#define DRIVER_DESC		"Printer Gadget"
#define DRIVER_VERSION		"2007 OCT 06"

static const char shortname [] = "printer";
static const char driver_desc [] = DRIVER_DESC;

enum {
        USB_GADGET_MANUFACTURER_IDX     = 0,
        USB_GADGET_PRODUCT_IDX,
        USB_GADGET_SERIAL_IDX,
        USB_GADGET_FIRST_AVAIL_IDX,
};

/*-------------------------------------------------------------------------*/

struct printer_dev {
	/* lock buffer lists during read/write calls */
	struct usb_gadget	*gadget;
	char			interface;
	struct usb_ep		*in_ep, *out_ep;

	struct list_head	rx_reqs;	/* List of free RX structs */
	struct list_head	rx_reqs_active;	/* List of Active RX xfers */
	struct list_head	rx_buffers;	/* List of completed xfers */
	/* wait until there is data to be read. */
	struct list_head	tx_reqs;	/* List of free TX structs */
	struct list_head	tx_reqs_active; /* List of Active TX xfers */
	/* Wait until there are write buffers available to use. */
	struct usb_request	*current_rx_req;
	size_t			current_rx_bytes;
	u8			*current_rx_buf;
	u8			printer_status;
	u8			reset_printer;
	struct device		*pdev;
	struct usb_function	function;
	uint8_t			connect_flag;
	char                    *string;
};

static struct printer_dev usb_printer_gadget;

/*-------------------------------------------------------------------------*/

/* DO NOT REUSE THESE IDs with a protocol-incompatible driver!!  Ever!!
 * Instead:  allocate your own, using normal USB-IF procedures.
 */

/* Thanks to NetChip Technologies for donating this product ID.
 */
#define PRINTER_VENDOR_NUM	0x0525		/* NetChip */
#define PRINTER_PRODUCT_NUM	0xa4a8		/* Linux-USB Printer Gadget */

/* Number of requests to allocate per endpoint, not used for ep0. */
static unsigned qlen = 2;

#define QLEN	qlen

/*-------------------------------------------------------------------------*/

/*
 * DESCRIPTORS ... most are static, but strings and (full) configuration
 * descriptors are built on demand.
 */

/* holds our biggest descriptor */
#define USB_BUFSIZE			8192

static struct usb_device_descriptor device_desc = {
	.bLength =		sizeof device_desc,
	.bDescriptorType =	USB_DT_DEVICE,
	.bcdUSB =		cpu_to_le16(0x0200),
	.bDeviceClass =		USB_CLASS_PER_INTERFACE,
	.bDeviceSubClass =	0,
	.bDeviceProtocol =	0,
	.idVendor =		cpu_to_le16(PRINTER_VENDOR_NUM),
	.idProduct =		cpu_to_le16(PRINTER_PRODUCT_NUM),
	.bNumConfigurations =	1
};

static struct usb_interface_descriptor intf_desc = {
	.bLength =		sizeof intf_desc,
	.bDescriptorType =	USB_DT_INTERFACE,
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_PRINTER,
	.bInterfaceSubClass =	1,	/* Printer Sub-Class */
	.bInterfaceProtocol =	2,	/* Bi-Directional */
	.iInterface =		0
};

static struct usb_endpoint_descriptor fs_ep_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK
};

static struct usb_endpoint_descriptor fs_ep_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK
};

static struct usb_descriptor_header *fs_printer_function[] = {
	(struct usb_descriptor_header *) &intf_desc,
	(struct usb_descriptor_header *) &fs_ep_in_desc,
	(struct usb_descriptor_header *) &fs_ep_out_desc,
	NULL
};

/*
 * usb 2.0 devices need to expose both high speed and full speed
 * descriptors, unless they only run at full speed.
 */

static struct usb_endpoint_descriptor hs_ep_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512)
};

static struct usb_endpoint_descriptor hs_ep_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512)
};

static struct usb_qualifier_descriptor dev_qualifier = {
	.bLength =		sizeof dev_qualifier,
	.bDescriptorType =	USB_DT_DEVICE_QUALIFIER,
	.bcdUSB =		cpu_to_le16(0x0200),
	.bDeviceClass =		USB_CLASS_PRINTER,
	.bNumConfigurations =	1
};

static struct usb_descriptor_header *hs_printer_function[] = {
	(struct usb_descriptor_header *) &intf_desc,
	(struct usb_descriptor_header *) &hs_ep_in_desc,
	(struct usb_descriptor_header *) &hs_ep_out_desc,
	NULL
};

static struct usb_otg_descriptor otg_descriptor = {
	.bLength =              sizeof otg_descriptor,
	.bDescriptorType =      USB_DT_OTG,
	.bmAttributes =         USB_OTG_SRP,
};

static const struct usb_descriptor_header *otg_desc[] = {
	(struct usb_descriptor_header *) &otg_descriptor,
	NULL,
};

/* maxpacket and other transfer characteristics vary by speed. */
#define ep_desc(g, hs, fs) (((g)->speed == USB_SPEED_HIGH)?(hs):(fs))

/*-------------------------------------------------------------------------*/

/* descriptors that are built on-demand */

static char				product_desc [40] = DRIVER_DESC;
static char				serial_num [40] = "0123456789";
static char				pnp_string [1024] =
	"MFG:linux;MDL:g_printer;CLS:PRINTER;SN:1;";

/* static strings, in UTF-8 */
static struct usb_string		strings [] = {
	[USB_GADGET_MANUFACTURER_IDX].s = "INGENIC",
	[USB_GADGET_PRODUCT_IDX].s = product_desc,
	[USB_GADGET_SERIAL_IDX].s = serial_num,
	{  }		/* end of list */
};

static struct usb_gadget_strings	stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

/*-------------------------------------------------------------------------*/

static struct usb_request *
printer_req_alloc(struct usb_ep *ep, unsigned len, gfp_t gfp_flags)
{
	struct usb_request	*req;

	req = usb_ep_alloc_request(ep, gfp_flags);

	if (req != NULL) {
		req->length = len;
		req->buf = kmalloc(len, gfp_flags);
		if (req->buf == NULL) {
			usb_ep_free_request(ep, req);
			return NULL;
		}
	}

	return req;
}

static inline void
usb_free_descriptors(struct usb_descriptor_header **v)
{
        kfree(v);
}

static void
usb_free_all_descriptors(struct usb_function *f)
{
        usb_free_descriptors(f->hs_descriptors);
}

static void
printer_req_free(struct usb_ep *ep, struct usb_request *req)
{
	if (ep != NULL && req != NULL) {
		kfree(req->buf);
		usb_ep_free_request(ep, req);
	}
}

/*-------------------------------------------------------------------------*/

static void rx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct printer_dev	*dev = ep->driver_data;
	int			status = req->status;

	list_del_init(&req->list);	/* Remode from Active List */

	switch (status) {

	/* normal completion */
	case 0:
		if (req->actual > 0) {
			list_add_tail(&req->list, &dev->rx_buffers);
			debug("G_Printer : rx length %d\n", req->actual);
		} else {
			list_add(&req->list, &dev->rx_reqs);
		}
		break;

	/* software-driven interface shutdown */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		debug("rx shutdown, code %d\n", status);
		list_add(&req->list, &dev->rx_reqs);
		break;

	/* for hardware automagic (such as pxa) */
	case -ECONNABORTED:		/* endpoint reset */
		debug("rx %s reset\n", ep->name);
		list_add(&req->list, &dev->rx_reqs);
		break;

	/* data overrun */
	case -EOVERFLOW:
		break;
		/* FALLTHROUGH */

	default:
		debug("rx status %d\n", status);
		list_add(&req->list, &dev->rx_reqs);
		break;
	}
}

static void tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct printer_dev	*dev = ep->driver_data;

	switch (req->status) {
	default:
		debug("tx err %d\n", req->status);
		/* FALLTHROUGH */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		break;
	case 0:
		break;
	}

	/* Take the request struct off the active list and put it on the
	 * free list.
	 */
	list_del_init(&req->list);
	list_add(&req->list, &dev->tx_reqs);
}

/*-------------------------------------------------------------------------*/
/* This function must be called with interrupts turned off. */
static void
setup_rx_reqs(struct printer_dev *dev)
{
	struct usb_request              *req;

	while (likely(!list_empty(&dev->rx_reqs))) {
		int error;

		req = container_of(dev->rx_reqs.next,
				struct usb_request, list);
		list_del_init(&req->list);

		/* The USB Host sends us whatever amount of data it wants to
		 * so we always set the length field to the full USB_BUFSIZE.
		 * If the amount of data is more than the read() caller asked
		 * for it will be stored in the request buffer until it is
		 * asked for by read().
		 */
		req->length = USB_BUFSIZE;
		req->complete = rx_complete;

		error = usb_ep_queue(dev->out_ep, req, GFP_ATOMIC);
		if (error) {
			debug("rx submit --> %d\n", error);
			list_add(&req->list, &dev->rx_reqs);
			break;
		} else if(list_empty(&req->list)){
			list_add(&req->list, &dev->rx_reqs_active);
		}
	}
}

int printer_read(uint8_t *buffer, uint32_t len)
{
	size_t				size;
	size_t			bytes_copied;
	struct usb_request		*req;
	/* This is a pointer to the current USB rx request. */
	struct usb_request		*current_rx_req;
	/* This is the number of bytes in the current rx buffer. */
	size_t				current_rx_bytes;
	/* This is a pointer to the current rx buffer. */
	u8				*current_rx_buf;

	if (len == 0 || buffer == NULL)
		return -EINVAL;

	debug("printer_read trying to read %d bytes\n", (int)len);

	if (!usb_printer_gadget.connect_flag) {
		return -ENOLINK;
	}

	/* We will use this flag later to check if a printer reset happened
	 * after we turn interrupts back on.
	 */
	usb_printer_gadget.reset_printer = 0;

	setup_rx_reqs(&usb_printer_gadget);

	bytes_copied = 0;
	current_rx_req = usb_printer_gadget.current_rx_req;
	current_rx_bytes = usb_printer_gadget.current_rx_bytes;
	current_rx_buf = usb_printer_gadget.current_rx_buf;
	usb_printer_gadget.current_rx_req = NULL;
	usb_printer_gadget.current_rx_bytes = 0;
	usb_printer_gadget.current_rx_buf = NULL;

	/* Check if there is any data in the read buffers. Please note that
	 * current_rx_bytes is the number of bytes in the current rx buffer.
	 * If it is zero then check if there are any other rx_buffers that
	 * are on the completed list. We are only out of data if all rx
	 * buffers are empty.
	 */
	if ((current_rx_bytes == 0) &&
			(likely(list_empty(&usb_printer_gadget.rx_buffers)))) {
		return -ENODATA;
	}

	/* We have data to return then copy it to the caller's buffer.*/
	while ((current_rx_bytes || likely(!list_empty(&usb_printer_gadget.rx_buffers)))
			&& len) {
		if (current_rx_bytes == 0) {
			req = container_of(usb_printer_gadget.rx_buffers.next,
					struct usb_request, list);
			list_del_init(&req->list);

			if (req->actual && req->buf) {
				current_rx_req = req;
				current_rx_bytes = req->actual;
				current_rx_buf = req->buf;
			} else {
				list_add(&req->list, &usb_printer_gadget.rx_reqs);
				continue;
			}
		}

		if (len > current_rx_bytes)
			size = current_rx_bytes;
		else
			size = len;

		memcpy(buffer, current_rx_buf, size);
		bytes_copied += size;
		len -= size;
		buffer += size;

		/* We've disconnected or reset so return. */
		if (usb_printer_gadget.reset_printer) {
			list_add(&current_rx_req->list, &usb_printer_gadget.rx_reqs);
			return -EAGAIN;
		}

		/* If we not returning all the data left in this RX request
		 * buffer then adjust the amount of data left in the buffer.
		 * Othewise if we are done with this RX request buffer then
		 * requeue it to get any incoming data from the USB host.
		 */
		if (size < current_rx_bytes) {
			current_rx_bytes -= size;
			current_rx_buf += size;
		} else {
			list_add(&current_rx_req->list, &usb_printer_gadget.rx_reqs);
			current_rx_bytes = 0;
			current_rx_buf = NULL;
			current_rx_req = NULL;
		}
	}

	usb_printer_gadget.current_rx_req = current_rx_req;
	usb_printer_gadget.current_rx_bytes = current_rx_bytes;
	usb_printer_gadget.current_rx_buf = current_rx_buf;

	debug("printer_read returned %d bytes\n", (int)bytes_copied);

	if (bytes_copied)
		return bytes_copied;
	else
		return -EAGAIN;
}

int printer_write(uint8_t *buffer, uint32_t len)
{
	size_t			size;	/* Amount of data in a TX request. */
	size_t			bytes_copied = 0;
	struct usb_request	*req;
	int			value;

	debug("printer_write trying to send %d bytes\n", (int)len);

	if (len == 0 || buffer == NULL)
		return -EINVAL;

	if (!usb_printer_gadget.connect_flag) {
		return -ENOLINK;
	}

	/* Check if a printer reset happens while we have interrupts on */
	usb_printer_gadget.reset_printer = 0;

	/* Check if there is any available write buffers */
	if (likely(list_empty(&usb_printer_gadget.tx_reqs))) {
		return -ENODATA;
	}

	while (likely(!list_empty(&usb_printer_gadget.tx_reqs)) && len) {

		if (len > USB_BUFSIZE)
			size = USB_BUFSIZE;
		else
			size = len;

		req = container_of(usb_printer_gadget.tx_reqs.next, struct usb_request,
				list);
		list_del_init(&req->list);

		req->complete = tx_complete;
		req->length = size;

		/* Check if we need to send a zero length packet. */
		if (len > size)
			/* They will be more TX requests so no yet. */
			req->zero = 0;
		else
			/* If the data amount is not a multple of the
			 * maxpacket size then send a zero length packet.
			 */
			req->zero = ((len % usb_printer_gadget.in_ep->maxpacket) == 0);

		memcpy(req->buf, buffer, size);

		bytes_copied += size;
		len -= size;
		buffer += size;

		/* We've disconnected or reset so free the req and buffer */
		if (usb_printer_gadget.reset_printer) {
			list_add(&req->list, &usb_printer_gadget.tx_reqs);
			return -EAGAIN;
		}

		list_add(&req->list, &usb_printer_gadget.tx_reqs_active);

		value = usb_ep_queue(usb_printer_gadget.in_ep, req, GFP_ATOMIC);
		if (value) {
			list_move(&req->list, &usb_printer_gadget.tx_reqs);
			return -EAGAIN;
		}

	}

	debug("printer_write sent %d bytes\n", (int)bytes_copied);

	if (bytes_copied)
		return bytes_copied;
	else
		return -EAGAIN;
}

int printer_get_connect_status(void)
{
    int status;

    if (&usb_printer_gadget == NULL) {
        printf("%s: device not initialized\n", __func__);
        return -ENODEV;
    }

    if (usb_printer_gadget.connect_flag)
        status = 1;
    else
        status = 0;

    return status;
}

/*-------------------------------------------------------------------------*/

static int
set_printer_interface(struct printer_dev *dev)
{
	int			result = 0;

	dev->in_ep->driver_data = dev;
	dev->out_ep->driver_data = dev;

	result = usb_ep_enable(dev->in_ep, &hs_ep_in_desc);
	if (result != 0) {
		debug("enable %s --> %d\n", dev->in_ep->name, result);
		goto done;
	}

	result = usb_ep_enable(dev->out_ep, &hs_ep_out_desc);
	if (result != 0) {
		debug("enable %s --> %d\n", dev->in_ep->name, result);
		goto done;
	}

done:
	/* on error, disable any endpoints  */
	if (result != 0) {
		(void) usb_ep_disable(dev->in_ep);
		(void) usb_ep_disable(dev->out_ep);
	} else {
//		dev->connect_flag = 1;
	}

	/* caller is responsible for cleanup on error */
	return result;
}

static void printer_reset_interface(struct printer_dev *dev)
{
	if (dev->interface < 0)
		return;

	debug("%s\n", __func__);

	usb_ep_disable(dev->in_ep);
	usb_ep_disable(dev->out_ep);

	dev->connect_flag = 0;
	dev->interface = -1;
}

/* Change our operational Interface. */
static int set_interface(struct printer_dev *dev, unsigned number)
{
	int			result = 0;

	/* Free the current interface */
	printer_reset_interface(dev);

	result = set_printer_interface(dev);
	if (result)
		printer_reset_interface(dev);
	else
		dev->interface = number;

	if (!result)
		printf("Using interface %x\n", number);

	return result;
}

static void printer_soft_reset(struct printer_dev *dev)
{
	struct usb_request	*req;

	printf("Received Printer Reset Request\n");

	if (usb_ep_disable(dev->in_ep))
		debug("Failed to disable USB in_ep\n");
	if (usb_ep_disable(dev->out_ep))
		debug("Failed to disable USB out_ep\n");

	if (dev->current_rx_req != NULL) {
		list_add(&dev->current_rx_req->list, &dev->rx_reqs);
		dev->current_rx_req = NULL;
	}
	dev->current_rx_bytes = 0;
	dev->current_rx_buf = NULL;
	dev->reset_printer = 1;

	while (likely(!(list_empty(&dev->rx_buffers)))) {
		req = container_of(dev->rx_buffers.next, struct usb_request,
				list);
		list_del_init(&req->list);
		list_add(&req->list, &dev->rx_reqs);
	}

	while (likely(!(list_empty(&dev->rx_reqs_active)))) {
		req = container_of(dev->rx_buffers.next, struct usb_request,
				list);
		list_del_init(&req->list);
		list_add(&req->list, &dev->rx_reqs);
	}

	while (likely(!(list_empty(&dev->tx_reqs_active)))) {
		req = container_of(dev->tx_reqs_active.next,
				struct usb_request, list);
		list_del_init(&req->list);
		list_add(&req->list, &dev->tx_reqs);
	}

	if (usb_ep_enable(dev->in_ep, &hs_ep_in_desc))
		debug("Failed to enable USB in_ep\n");
	if (usb_ep_enable(dev->out_ep, &hs_ep_out_desc))
		debug("Failed to enable USB out_ep\n");
}

/*-------------------------------------------------------------------------*/

/*
 * The setup() callback implements all the ep0 functionality that's not
 * handled lower down.
 */
static int printer_func_setup(struct usb_function *f,
		const struct usb_ctrlrequest *ctrl)
{
	struct printer_dev *dev = container_of(f, struct printer_dev, function);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request	*req = cdev->req;
	u8		        *buf = req->buf;
	int			value = -EOPNOTSUPP;
	u16			wIndex = le16_to_cpu(ctrl->wIndex);
	u16			wValue = le16_to_cpu(ctrl->wValue);
	u16			wLength = le16_to_cpu(ctrl->wLength);
	int			len;

	debug("ctrl req%02x.%02x v%04x i%04x l%d\n",
		ctrl->bRequestType, ctrl->bRequest, wValue, wIndex, wLength);

	switch (ctrl->bRequestType&USB_TYPE_MASK) {
	case USB_TYPE_CLASS:
		switch (ctrl->bRequest) {
		case 0: /* Get the IEEE-1284 PNP String */
			/* Only one printer interface is supported. */
			if ((wIndex>>8) != dev->interface)
				break;

			value = strlen(dev->string);
			buf[0] = (value >> 8) & 0xFF;
			buf[1] = value & 0xFF;
			memcpy(buf + 2, dev->string, value);
			debug("1284 PNP String: %x %s\n", value,
					dev->string);
			break;

		case 1: /* Get Port Status */
			/* Only one printer interface is supported. */
			if (wIndex != dev->interface)
				break;

			*(u8 *)req->buf = dev->printer_status;
			value = min(wLength, (u16) 1);
			break;

		case 2: /* Soft Reset */
			/* Only one printer interface is supported. */
			if (wIndex != dev->interface)
				break;

			printer_soft_reset(dev);

			value = 0;
			break;

		default:
			goto unknown;
		}
		break;

	default:
unknown:
		debug("unknown ctrl req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			wValue, wIndex, wLength);
		break;
	}
	/* host either stalls (value < 0) or reports success */
	if (value >= 0) {
		req->length = value;
		req->zero = value < wLength;
		value = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
		if (value < 0) {
			printf("%s:%d Error!\n", __func__, __LINE__);
			req->status = 0;
		}
			dev->connect_flag = 1;
	}
	return value;
}

static int printer_func_bind(struct usb_configuration *c,
		struct usb_function *f)
{
	struct printer_dev *dev = container_of(f, struct printer_dev, function);
	struct usb_composite_dev *cdev = c->cdev;
	struct usb_ep *in_ep;
	struct usb_ep *out_ep = NULL;
	int id;
	int ret;

	id = usb_interface_id(c, f);
	if (id < 0)
		return id;
	intf_desc.bInterfaceNumber = id;

	/* all we really need is bulk IN/OUT */
	in_ep = usb_ep_autoconfig(cdev->gadget, &fs_ep_in_desc);
	if (!in_ep) {
autoconf_fail:
		debug("can't autoconfigure on %s\n",
			cdev->gadget->name);
		return -ENODEV;
	}
	in_ep->driver_data = in_ep;	/* claim */

	out_ep = usb_ep_autoconfig(cdev->gadget, &fs_ep_out_desc);
	if (!out_ep)
		goto autoconf_fail;
	out_ep->driver_data = out_ep;	/* claim */

	/* assumes that all endpoints are dual-speed */
	hs_ep_in_desc.bEndpointAddress = fs_ep_in_desc.bEndpointAddress;
	hs_ep_out_desc.bEndpointAddress = fs_ep_out_desc.bEndpointAddress;

	ret = usb_assign_descriptors(f, fs_printer_function,
			hs_printer_function, NULL);
	if (ret)
		return ret;

	dev->in_ep = in_ep;
	dev->out_ep = out_ep;
	return 0;
}

static void printer_func_unbind(struct usb_configuration *c,
		struct usb_function *f)
{
	struct printer_dev *dev = container_of(f, struct printer_dev, function);
	dev->connect_flag = 0;
	usb_free_all_descriptors(f);
}

static int printer_func_set_alt(struct usb_function *f,
		unsigned intf, unsigned alt)
{
	struct printer_dev *dev = container_of(f, struct printer_dev, function);
	int ret = -ENOTSUPP;

	if (!alt)
		ret = set_interface(dev, intf);

	return ret;
}

static void printer_func_disable(struct usb_function *f)
{
	struct printer_dev *dev = container_of(f, struct printer_dev, function);
	debug("%s\n", __func__);

	printer_reset_interface(dev);
}

static void printer_cfg_unbind(struct usb_configuration *c)
{
	struct printer_dev	*dev;
	struct usb_request	*req;

	dev = &usb_printer_gadget;

	debug("%s\n", __func__);

	/* we must already have been disconnected ... no i/o may be active */
	WARN_ON(!list_empty(&dev->tx_reqs_active));
	WARN_ON(!list_empty(&dev->rx_reqs_active));

	/* Free all memory for this driver. */
	while (!list_empty(&dev->tx_reqs)) {
		req = container_of(dev->tx_reqs.next, struct usb_request,
				list);
		list_del(&req->list);
		printer_req_free(dev->in_ep, req);
	}

	if (dev->current_rx_req != NULL)
		printer_req_free(dev->out_ep, dev->current_rx_req);

	while (!list_empty(&dev->rx_reqs)) {
		req = container_of(dev->rx_reqs.next,
				struct usb_request, list);
		list_del(&req->list);
		printer_req_free(dev->out_ep, req);
	}

	while (!list_empty(&dev->rx_buffers)) {
		req = container_of(dev->rx_buffers.next,
				struct usb_request, list);
		list_del(&req->list);
		printer_req_free(dev->out_ep, req);
	}
}

static int printer_bind_config(struct usb_configuration *c)
{
	struct usb_gadget	*gadget = c->cdev->gadget;
	struct printer_dev	*dev;
	int			status = -ENOMEM;
	size_t			len;
	u32			i;
	struct usb_request	*req;

	usb_ep_autoconfig_reset(gadget);

	dev = &usb_printer_gadget;

	dev->string = pnp_string;
	dev->function.name = shortname;
	dev->function.bind = printer_func_bind;
	dev->function.setup = printer_func_setup;
	dev->function.unbind = printer_func_unbind;
	dev->function.set_alt = printer_func_set_alt;
	dev->function.disable = printer_func_disable;

	status = usb_add_function(c, &dev->function);
	if (status)
		return status;

	usb_gadget_set_selfpowered(gadget);

	INIT_LIST_HEAD(&dev->tx_reqs);
	INIT_LIST_HEAD(&dev->tx_reqs_active);
	INIT_LIST_HEAD(&dev->rx_reqs);
	INIT_LIST_HEAD(&dev->rx_reqs_active);
	INIT_LIST_HEAD(&dev->rx_buffers);

	dev->interface = -1;
//	dev->printer_status = PRINTER_NOT_ERROR;
	dev->current_rx_req = NULL;
	dev->current_rx_bytes = 0;
	dev->current_rx_buf = NULL;

	for (i = 0; i < QLEN; i++) {
		req = printer_req_alloc(dev->in_ep, USB_BUFSIZE, GFP_KERNEL);
		if (!req) {
			while (!list_empty(&dev->tx_reqs)) {
				req = container_of(dev->tx_reqs.next,
						struct usb_request, list);
				list_del(&req->list);
				printer_req_free(dev->in_ep, req);
			}
			return -ENOMEM;
		}
		list_add(&req->list, &dev->tx_reqs);
	}

	for (i = 0; i < QLEN; i++) {
		req = printer_req_alloc(dev->out_ep, USB_BUFSIZE, GFP_KERNEL);
		if (!req) {
			while (!list_empty(&dev->rx_reqs)) {
				req = container_of(dev->rx_reqs.next,
						struct usb_request, list);
				list_del(&req->list);
				printer_req_free(dev->out_ep, req);
			}
			return -ENOMEM;
		}
		list_add(&req->list, &dev->rx_reqs);
	}

	/* finish hookup to lower layer ... */
	dev->gadget = gadget;

	printf("%s, version: " DRIVER_VERSION "\n", driver_desc);
	return 0;

fail:
	printer_cfg_unbind(c);
	return status;
}

static struct usb_configuration printer_cfg_driver = {
	.label			= "printer",
	.bind			= printer_bind_config,
	.unbind			= printer_cfg_unbind,
	.bConfigurationValue	= 1,
	.bmAttributes		= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
};

static int printer_unbind(struct usb_composite_dev *cdev)
{
	return 0;
}

static int printer_bind(struct usb_composite_dev *cdev)
{
	int ret;

	ret = usb_string_ids_tab(cdev, strings);
	if (ret < 0)
		return ret;
	device_desc.iManufacturer = strings[USB_GADGET_MANUFACTURER_IDX].id;
	device_desc.iProduct = strings[USB_GADGET_PRODUCT_IDX].id;
	device_desc.iSerialNumber = strings[USB_GADGET_SERIAL_IDX].id;

	ret = usb_add_config(cdev, &printer_cfg_driver);
	if (ret)
		return ret;
	return ret;
}

static struct usb_composite_driver printer_driver = {
	.name           = shortname,
	.dev            = &device_desc,
	.strings        = dev_strings,
	.bind		= printer_bind,
	.unbind		= printer_unbind,
};

int usb_gprinter_register(void)
{
	int status;

	status = usb_composite_register(&printer_driver);
	if (status) {
		printf("usb_gadget_probe_driver %x\n", status);
	}

	return status;
}

void usb_gprinter_unregister(void)
{
	usb_composite_unregister(&printer_driver);
}
