// SPDX-License-Identifier: GPL-2.0+
/*
 * USB CDC serial (ACM) function driver
 *
 * Copyright (C) 2003 Al Borchers (alborchers@steinerpoint.com)
 * Copyright (C) 2008 by David Brownell
 * Copyright (C) 2008 by Nokia Corporation
 * Copyright (C) 2009 by Samsung Electronics
 * Copyright (c) 2021, Linaro Ltd <loic.poulain@linaro.org>
 */
#include <errno.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <linux/usb/cdc.h>

#include "composite.c"

/* holds our biggest descriptor */
#define USB_BUFSIZE	8192

struct f_acm {
	int ctrl_id;
	int data_id;

	struct usb_ep *ep_in;
	struct usb_ep *ep_out;
	struct usb_ep *ep_notify;

	struct usb_request *req_in;
	struct usb_request *req_out;
	struct usb_request *req_notify;

	bool connected;
	bool tx_on;

	struct usb_function usb_function;

	struct usb_cdc_line_coding line_coding;
	u16 handshake_bits;
#define ACM_CTRL_RTS	(1UL << (1))	/* unused with full duplex */
#define ACM_CTRL_DTR	(1UL << (0))	/* host is ready for data r/w */

	struct udevice *udc;

	struct list_head	rx_reqs;	/* List of free RX structs */
	struct list_head	rx_reqs_active;	/* List of Active RX xfers */
	struct list_head	rx_buffers;	/* List of completed xfers */
	struct list_head	tx_reqs;	/* List of free TX structs */
	struct list_head	tx_reqs_active; /* List of Active TX xfers */
	struct usb_request	*current_rx_req;
	size_t			current_rx_bytes;
	u8			*current_rx_buf;
};

static struct f_acm usb_acm_gadget;

static inline struct f_acm *func_to_acm(struct usb_function *f)
{
	return container_of(f, struct f_acm, usb_function);
}

/* notification endpoint uses smallish and infrequent fixed-size messages */
#define GS_NOTIFY_INTERVAL_MS           32
#define GS_NOTIFY_MAXPACKET             10      /* notification + 2 bytes */

static struct usb_interface_assoc_descriptor
acm_iad_descriptor = {
	.bLength =              sizeof(acm_iad_descriptor),
	.bDescriptorType =      USB_DT_INTERFACE_ASSOCIATION,
	.bInterfaceCount =      2,	// control + data
	.bFunctionClass =       USB_CLASS_COMM,
	.bFunctionSubClass =    USB_CDC_SUBCLASS_ACM,
	.bFunctionProtocol =    USB_CDC_ACM_PROTO_AT_V25TER,
};

static struct usb_interface_descriptor acm_control_intf_desc = {
	.bLength =              USB_DT_INTERFACE_SIZE,
	.bDescriptorType =      USB_DT_INTERFACE,
	.bNumEndpoints =        1,
	.bInterfaceClass =      USB_CLASS_COMM,
	.bInterfaceSubClass =   USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol =   USB_CDC_ACM_PROTO_AT_V25TER,
	.iInterface  =		5,
};

static struct usb_interface_descriptor acm_data_intf_desc = {
	.bLength =              sizeof(acm_data_intf_desc),
	.bDescriptorType =      USB_DT_INTERFACE,
	.bNumEndpoints =        2,
	.bInterfaceClass =      USB_CLASS_CDC_DATA,
	.iInterface  =		6,
};

static struct usb_cdc_header_desc acm_header_desc = {
	.bLength =		sizeof(acm_header_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_HEADER_TYPE,
	.bcdCDC =		cpu_to_le16(0x0110),
};

static struct usb_cdc_call_mgmt_descriptor acm_call_mgmt_desc = {
	.bLength =              sizeof(acm_call_mgmt_desc),
	.bDescriptorType =      USB_DT_CS_INTERFACE,
	.bDescriptorSubType =   USB_CDC_CALL_MANAGEMENT_TYPE,
	.bmCapabilities =       0,
};

static struct usb_cdc_acm_descriptor acm_desc = {
	.bLength =		sizeof(acm_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_ACM_TYPE,
	.bmCapabilities =	USB_CDC_CAP_LINE,
};

static struct usb_cdc_union_desc acm_union_desc = {
	.bLength =		sizeof(acm_union_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_UNION_TYPE,
};

static struct usb_endpoint_descriptor acm_fs_notify_desc = {
	.bLength =              USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =      USB_DT_ENDPOINT,
	.bEndpointAddress =     USB_DIR_IN,
	.bmAttributes =         USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(GS_NOTIFY_MAXPACKET),
	.bInterval =            GS_NOTIFY_INTERVAL_MS,
};

static struct usb_endpoint_descriptor acm_fs_in_desc = {
	.bLength =              USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =      USB_DT_ENDPOINT,
	.bEndpointAddress =     USB_DIR_IN,
	.bmAttributes =         USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor acm_fs_out_desc = {
	.bLength =              USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =      USB_DT_ENDPOINT,
	.bEndpointAddress =     USB_DIR_OUT,
	.bmAttributes =         USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *acm_fs_function[] = {
	(struct usb_descriptor_header *)&acm_iad_descriptor,
	(struct usb_descriptor_header *)&acm_control_intf_desc,
	(struct usb_descriptor_header *)&acm_header_desc,
	(struct usb_descriptor_header *)&acm_call_mgmt_desc,
	(struct usb_descriptor_header *)&acm_desc,
	(struct usb_descriptor_header *)&acm_union_desc,
	(struct usb_descriptor_header *)&acm_fs_notify_desc,
	(struct usb_descriptor_header *)&acm_data_intf_desc,
	(struct usb_descriptor_header *)&acm_fs_in_desc,
	(struct usb_descriptor_header *)&acm_fs_out_desc,
	NULL,
};

static struct usb_endpoint_descriptor acm_hs_notify_desc = {
	.bLength =              USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =      USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =         USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =       cpu_to_le16(GS_NOTIFY_MAXPACKET),
	.bInterval =            9,
};

static struct usb_endpoint_descriptor acm_hs_in_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= cpu_to_le16(512),
};

static struct usb_endpoint_descriptor acm_hs_out_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= cpu_to_le16(512),
};

static struct usb_descriptor_header *acm_hs_function[] = {
	(struct usb_descriptor_header *)&acm_iad_descriptor,
	(struct usb_descriptor_header *)&acm_control_intf_desc,
	(struct usb_descriptor_header *)&acm_header_desc,
	(struct usb_descriptor_header *)&acm_call_mgmt_desc,
	(struct usb_descriptor_header *)&acm_desc,
	(struct usb_descriptor_header *)&acm_union_desc,
	(struct usb_descriptor_header *)&acm_hs_notify_desc,
	(struct usb_descriptor_header *)&acm_data_intf_desc,
	(struct usb_descriptor_header *)&acm_hs_in_desc,
	(struct usb_descriptor_header *)&acm_hs_out_desc,
	NULL,
};

static struct usb_request *
acm_req_alloc(struct usb_ep *ep, unsigned len, gfp_t gfp_flags)
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

static inline void usb_free_descriptors(struct usb_descriptor_header **v)
{
        kfree(v);
}

static void usb_free_all_descriptors(struct usb_function *f)
{
        usb_free_descriptors(f->hs_descriptors);
}

static void acm_req_free(struct usb_ep *ep, struct usb_request *req)
{
	if (ep != NULL && req != NULL) {
		kfree(req->buf);
		usb_ep_free_request(ep, req);
	}
}

static inline struct usb_endpoint_descriptor *
ep_desc(struct usb_gadget *g, struct usb_endpoint_descriptor *hs,
	struct usb_endpoint_descriptor *fs)
{
	if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return hs;
	return fs;
}

bool acm_connected(void){ return usb_acm_gadget.connected;}
/*-------------------------------------------------------------------------*/

static void rx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_acm		*acm = ep->driver_data;
	int			status = req->status;

	list_del_init(&req->list);	/* Remode from Active List */

	switch (status) {

	/* normal completion */
	case 0:
		if (req->actual > 0) {
			list_add_tail(&req->list, &acm->rx_buffers);
			debug("acm: rx length %d\n", req->actual);
		} else {
			list_add(&req->list, &acm->rx_reqs);
		}
		break;

	/* software-driven interface shutdown */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		debug("rx shutdown, code %d\n", status);
		list_add(&req->list, &acm->rx_reqs);
		break;

	/* for hardware automagic (such as pxa) */
	case -ECONNABORTED:		/* endpoint reset */
		debug("rx %s reset\n", ep->name);
		list_add(&req->list, &acm->rx_reqs);
		break;

	/* data overrun */
	case -EOVERFLOW:
		break;
		/* FALLTHROUGH */

	default:
		debug("rx status %d\n", status);
		list_add(&req->list, &acm->rx_reqs);
		break;
	}
}

static void tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_acm		*acm = ep->driver_data;

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
	list_add(&req->list, &acm->tx_reqs);
}

static void setup_rx_reqs(struct f_acm *acm)
{
	struct usb_request              *req;

	while (likely(!list_empty(&acm->rx_reqs))) {
		int error;

		req = container_of(acm->rx_reqs.next,
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

		error = usb_ep_queue(acm->ep_out, req, 0);
		if (error) {
			debug("rx submit --> %d\n", error);
			list_add(&req->list, &acm->rx_reqs);
			break;
		} else if(list_empty(&req->list)){
			list_add(&req->list, &acm->rx_reqs_active);
		}
	}
}

int acm_read(uint8_t *buffer, uint32_t len)
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

	debug("acm_read trying to read %d bytes\n", (int)len);

	if (!usb_acm_gadget.connected) {
		return -ENOLINK;
	}

	setup_rx_reqs(&usb_acm_gadget);

	bytes_copied = 0;
	current_rx_req = usb_acm_gadget.current_rx_req;
	current_rx_bytes = usb_acm_gadget.current_rx_bytes;
	current_rx_buf = usb_acm_gadget.current_rx_buf;
	usb_acm_gadget.current_rx_req = NULL;
	usb_acm_gadget.current_rx_bytes = 0;
	usb_acm_gadget.current_rx_buf = NULL;

	/* Check if there is any data in the read buffers. Please note that
	 * current_rx_bytes is the number of bytes in the current rx buffer.
	 * If it is zero then check if there are any other rx_buffers that
	 * are on the completed list. We are only out of data if all rx
	 * buffers are empty.
	 */
	if ((current_rx_bytes == 0) &&
			(likely(list_empty(&usb_acm_gadget.rx_buffers)))) {
		return -ENODATA;
	}

	/* We have data to return then copy it to the caller's buffer.*/
	while ((current_rx_bytes || likely(!list_empty(&usb_acm_gadget.rx_buffers)))
			&& len) {
		if (current_rx_bytes == 0) {
			req = container_of(usb_acm_gadget.rx_buffers.next,
					struct usb_request, list);
			list_del_init(&req->list);

			if (req->actual && req->buf) {
				current_rx_req = req;
				current_rx_bytes = req->actual;
				current_rx_buf = req->buf;
			} else {
				list_add(&req->list, &usb_acm_gadget.rx_reqs);
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

		/* If we not returning all the data left in this RX request
		 * buffer then adjust the amount of data left in the buffer.
		 * Othewise if we are done with this RX request buffer then
		 * requeue it to get any incoming data from the USB host.
		 */
		if (size < current_rx_bytes) {
			current_rx_bytes -= size;
			current_rx_buf += size;
		} else {
			list_add(&current_rx_req->list, &usb_acm_gadget.rx_reqs);
			current_rx_bytes = 0;
			current_rx_buf = NULL;
			current_rx_req = NULL;
		}
	}

	usb_acm_gadget.current_rx_req = current_rx_req;
	usb_acm_gadget.current_rx_bytes = current_rx_bytes;
	usb_acm_gadget.current_rx_buf = current_rx_buf;

	debug("acm_read returned %d bytes\n", (int)bytes_copied);

	if (bytes_copied)
		return bytes_copied;
	else
		return -EAGAIN;
}

int acm_write(uint8_t *buffer, uint32_t len)
{
	size_t			size;	/* Amount of data in a TX request. */
	size_t			bytes_copied = 0;
	struct usb_request	*req;
	int			value;

	debug("acm_write trying to send %d bytes\n", (int)len);

	if (len == 0 || buffer == NULL)
		return -EINVAL;

	if (!usb_acm_gadget.connected) {
		return -ENOLINK;
	}

	/* Check if there is any available write buffers */
	if (likely(list_empty(&usb_acm_gadget.tx_reqs))) {
		return -ENODATA;
	}

	while (likely(!list_empty(&usb_acm_gadget.tx_reqs)) && len) {

		if (len > USB_BUFSIZE)
			size = USB_BUFSIZE;
		else
			size = len;

		req = container_of(usb_acm_gadget.tx_reqs.next, struct usb_request,
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
			req->zero = ((len % usb_acm_gadget.ep_in->maxpacket) == 0);

		memcpy(req->buf, buffer, size);

		bytes_copied += size;
		len -= size;
		buffer += size;

		list_add(&req->list, &usb_acm_gadget.tx_reqs_active);

		value = usb_ep_queue(usb_acm_gadget.ep_in, req, 0);
		if (value) {
			list_move(&req->list, &usb_acm_gadget.tx_reqs);
			return -EAGAIN;
		}

	}

	debug("acm_write sent %d bytes\n", (int)bytes_copied);

	if (bytes_copied)
		return bytes_copied;
	else
		return -EAGAIN;
}

static void acm_disable(struct usb_function *f)
{
	struct f_acm	*acm = func_to_acm(f);

	acm->connected = 0;
	usb_ep_disable(acm->ep_in);
	usb_ep_disable(acm->ep_out);
	usb_ep_disable(acm->ep_notify);

	if (acm->req_out) {
		free(acm->req_out->buf);
		usb_ep_free_request(acm->ep_out, acm->req_out);
		acm->req_out = NULL;
	}

	if (acm->req_in) {
		free(acm->req_in->buf);
		usb_ep_free_request(acm->ep_in, acm->req_in);
		acm->req_in = NULL;
	}
}

static int acm_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_acm		*acm = func_to_acm(f);
	struct usb_string	*us;
	struct usb_ep		*ep;
	int			status;

	status = usb_interface_id(c, f);
	if (status < 0)
		return status;
	acm->ctrl_id = status;
	acm_iad_descriptor.bFirstInterface = status;
	acm_control_intf_desc.bInterfaceNumber = status;
	acm_union_desc.bMasterInterface0 = status;

	status = usb_interface_id(c, f);
	if (status < 0)
		return status;
	acm->data_id = status;
	acm_data_intf_desc.bInterfaceNumber = status;
	acm_union_desc.bSlaveInterface0 = status;
	acm_call_mgmt_desc.bDataInterface = status;

	/* allocate instance-specific endpoints */
	ep = usb_ep_autoconfig(cdev->gadget, &acm_fs_in_desc);
	if (!ep)
		return -ENODEV;

	acm->ep_in = ep;
	acm->ep_in->driver_data = ep;

	ep = usb_ep_autoconfig(cdev->gadget, &acm_fs_out_desc);
	if (!ep)
		return -ENODEV;

	acm->ep_out = ep;
	acm->ep_out->driver_data = ep;

	ep = usb_ep_autoconfig(cdev->gadget, &acm_fs_notify_desc);
	if (!ep)
		return -ENODEV;

	acm->ep_notify = ep;
	acm->ep_notify->driver_data = ep;

	acm->req_notify = acm_req_alloc(ep,
			sizeof(struct usb_cdc_notification) + 2,
			GFP_KERNEL);
	if (!acm->req_notify)
		goto fail;

	/* Assume endpoint addresses are the same for both speeds */
	acm_hs_in_desc.bEndpointAddress = acm_fs_in_desc.bEndpointAddress;
	acm_hs_out_desc.bEndpointAddress = acm_fs_out_desc.bEndpointAddress;
	acm_hs_notify_desc.bEndpointAddress = acm_fs_notify_desc.bEndpointAddress;

	status = usb_assign_descriptors(f, acm_fs_function, acm_hs_function, NULL);
	if (status)
		goto fail;

	return 0;
fail:
	if (acm->ep_notify)
		acm_req_free(acm->ep_notify, acm->req_notify);

	printf("%s/%p: can't bind, err %d\n", f->name, f, status);

	return status;
}

static void acm_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_acm *acm = func_to_acm(f);
	acm->connected = 0;
	usb_free_all_descriptors(f);
	if (acm->req_notify)
		acm_req_free(acm->ep_notify, acm->req_notify);
}

static void acm_complete_set_line_coding(struct usb_ep *ep, struct usb_request *req)
{
	struct f_acm	*acm = ep->driver_data;

	if (req->status != 0) {
		debug("acm completion, err %d\n",req->status);
		return;
	}

	/* normal completion */
	if (req->actual != sizeof(acm->line_coding)) {
		debug("acm short resp, len %d\n",req->actual);
		usb_ep_set_halt(ep);
	} else {
		struct usb_cdc_line_coding	*value = req->buf;

		/* REVISIT:  we currently just remember this data.
		 * If we change that, (a) validate it first, then
		 * (b) update whatever hardware needs updating,
		 * (c) worry about locking.  This is information on
		 * the order of 9600-8-N-1 ... most of which means
		 * nothing unless we control a real RS232 line.
		 */
		acm->line_coding = *value;
		debug("dwDTERate:	%u\nbCharFormat:	%u\n"
			"bParityType:	%u\nbDataBits:	%u\n",
			le32_to_cpu(acm->line_coding.dwDTERate),
			acm->line_coding.bCharFormat,
			acm->line_coding.bParityType,
			acm->line_coding.bDataBits);
	}
}

static int acm_start_data(struct f_acm *acm, struct usb_gadget *gadget)
{
	const struct usb_endpoint_descriptor *d;
	int ret;

	/* EP IN */
	d = ep_desc(gadget, &acm_hs_in_desc, &acm_fs_in_desc);
	ret = usb_ep_enable(acm->ep_in, d);
	if (ret)
		return ret;

	/* EP OUT */
	d = ep_desc(gadget, &acm_hs_out_desc, &acm_fs_out_desc);
	ret = usb_ep_enable(acm->ep_out, d);
	if (ret)
		return ret;

	return 0;
}

static int acm_start_ctrl(struct f_acm *acm, struct usb_gadget *gadget)
{
	const struct usb_endpoint_descriptor *d;

	usb_ep_disable(acm->ep_notify);

	d = ep_desc(gadget, &acm_hs_notify_desc, &acm_fs_notify_desc);
	usb_ep_enable(acm->ep_notify, d);

	return 0;
}

static int acm_set_alt(struct usb_function *f, unsigned int intf, unsigned int alt)
{
	struct usb_gadget *gadget = f->config->cdev->gadget;
	struct f_acm *acm = func_to_acm(f);

	acm->ep_in->driver_data = acm;
	acm->ep_out->driver_data = acm;

	if (intf == acm->ctrl_id) {
		return acm_start_ctrl(acm, gadget);
	} else if (intf == acm->data_id) {
		acm_start_data(acm, gadget);
		acm->connected = 1;
		acm->tx_on = 1;
		printf("usb acm configed.\n");
		return 0;
	} else
		return -EINVAL;

	return 0;
}

static int acm_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct f_acm			*acm = func_to_acm(f);
	struct usb_composite_dev	*cdev = f->config->cdev;
	struct usb_request		*req = f->config->cdev->req;
	int				value = -EOPNOTSUPP;
	u16				w_index = le16_to_cpu(ctrl->wIndex);
	u16				w_value = le16_to_cpu(ctrl->wValue);
	u16				w_length = le16_to_cpu(ctrl->wLength);
	u8				*buf = req->buf;

	switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {

	/* SET_LINE_CODING ... just read and save what the host sends */
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_REQ_SET_LINE_CODING:
		if (w_length != sizeof(struct usb_cdc_line_coding)
				|| w_index != acm->ctrl_id)
			goto invalid;

		value = w_length;
		cdev->gadget->ep0->driver_data = acm;
		req->complete = acm_complete_set_line_coding;
		break;

	/* GET_LINE_CODING ... return what host sent, or initial value */
	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_REQ_GET_LINE_CODING:
		if (w_index != acm->ctrl_id)
			goto invalid;

		value = min_t(unsigned, w_length,
				sizeof(struct usb_cdc_line_coding));

		memcpy(buf, &acm->line_coding, value);
		break;

	/* SET_CONTROL_LINE_STATE ... save what the host sent */
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		if (w_index != acm->ctrl_id)
			goto invalid;

		value = 0;

		acm->handshake_bits = w_value;
		break;

	default:
invalid:
		printf("invalid control req%02x.%02x v%04x i%04x l%d\n",
		       ctrl->bRequestType, ctrl->bRequest, w_value, w_index,
		       w_length);
	}

	/* respond with data transfer or status phase? */
	if (value >= 0) {
		req->zero = 0;
		req->length = value;
		value = usb_ep_queue(cdev->gadget->ep0, req, 0);
		if (value < 0)
			printf("acm response err %d\n", value);
	}

	return value;
}

int acm_bind_config(struct usb_configuration *c)
{
	struct usb_gadget	*gadget = c->cdev->gadget;
	struct f_acm		*acm;
	struct usb_request	*req;
	int			status, i;

	acm = &usb_acm_gadget;
	if (!acm)
		return -ENOMEM;

	acm->usb_function.name = "f_acm";
	acm->usb_function.bind = acm_bind;
	acm->usb_function.unbind = acm_unbind;
	acm->usb_function.set_alt = acm_set_alt;
	acm->usb_function.disable = acm_disable;
	acm->usb_function.descriptors = acm_fs_function;
	acm->usb_function.hs_descriptors = acm_hs_function;
	acm->usb_function.setup = acm_setup;

	status = usb_add_function(c, &acm->usb_function);
	if (status)
		return status;

	INIT_LIST_HEAD(&acm->tx_reqs);
	INIT_LIST_HEAD(&acm->tx_reqs_active);
	INIT_LIST_HEAD(&acm->rx_reqs);
	INIT_LIST_HEAD(&acm->rx_reqs_active);
	INIT_LIST_HEAD(&acm->rx_buffers);

	acm->current_rx_req = NULL;
	acm->current_rx_bytes = 0;
	acm->current_rx_buf = NULL;

	for (i = 0; i < 2; i++) {
		req = acm_req_alloc(acm->ep_in, USB_BUFSIZE, GFP_KERNEL);
		if (!req) {
			while (!list_empty(&acm->tx_reqs)) {
				req = container_of(acm->tx_reqs.next,
						struct usb_request, list);
				list_del(&req->list);
				acm_req_free(acm->ep_in, req);
			}
			return -ENOMEM;
		}
		list_add(&req->list, &acm->tx_reqs);
	}

	for (i = 0; i < 2; i++) {
		req = acm_req_alloc(acm->ep_out, USB_BUFSIZE, GFP_KERNEL);
		if (!req) {
			while (!list_empty(&acm->rx_reqs)) {
				req = container_of(acm->rx_reqs.next,
						struct usb_request, list);
				list_del(&req->list);
				acm_req_free(acm->ep_out, req);
			}
			return -ENOMEM;
		}
		list_add(&req->list, &acm->rx_reqs);
	}

	return 0;

}

void acm_unbind_config(struct usb_configuration *c)
{
	struct f_acm		*acm;
	struct usb_request	*req;

	acm = &usb_acm_gadget;
	acm->connected = 0;

	debug("%s\n", __func__);

	/* we must already have been disconnected ... no i/o may be active */
	WARN_ON(!list_empty(&acm->tx_reqs_active));
	WARN_ON(!list_empty(&acm->rx_reqs_active));

	/* Free all memory for this driver. */
	while (!list_empty(&acm->tx_reqs)) {
		req = container_of(acm->tx_reqs.next, struct usb_request,
				list);
		list_del(&req->list);
		acm_req_free(acm->ep_in, req);
	}

	if (acm->current_rx_req != NULL)
		acm_req_free(acm->ep_out, acm->current_rx_req);

	while (!list_empty(&acm->rx_reqs)) {
		req = container_of(acm->rx_reqs.next,
				struct usb_request, list);
		list_del(&req->list);
		acm_req_free(acm->ep_out, req);
	}

	while (!list_empty(&acm->rx_buffers)) {
		req = container_of(acm->rx_buffers.next,
				struct usb_request, list);
		list_del(&req->list);
		acm_req_free(acm->ep_out, req);
	}
}

