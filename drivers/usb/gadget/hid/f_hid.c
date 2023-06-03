/*
 * f_hid.c -- USB HID function driver
 *
 * Copyright (C) 2010 Fabien Chouteau <fabien.chouteau@barco.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <malloc.h>
#include <linux/usb/g_hid.h>
#include <linux/list.h>

#include <version.h>
#include <asm/unaligned.h>
#include <linux/err.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/composite.h>
#include <usb/lin_gadget_compat.h>
#include <linux/usb/cdc.h>
#include <linux/usb/composite.h>

#include "f_hid.h"

#define GFP_ATOMIC ((gfp_t) 0)

#define HID_REQ_GET_REPORT              0x01
#define HID_REQ_GET_IDLE                0x02
#define HID_REQ_GET_PROTOCOL            0x03
#define HID_REQ_SET_REPORT              0x09
#define HID_REQ_SET_IDLE                0x0A
#define HID_REQ_SET_PROTOCOL            0x0B

/*
 * HID class descriptor types
 */

#define HID_DT_HID                      (USB_TYPE_CLASS | 0x01)
#define HID_DT_REPORT                   (USB_TYPE_CLASS | 0x02)
#define HID_DT_PHYSICAL                 (USB_TYPE_CLASS | 0x03)

#define HID_MAX_DESCRIPTOR_SIZE         4096

struct hid_class_descriptor {
	__u8  bDescriptorType;
	__le16 wDescriptorLength;

} __attribute__ ((packed));

struct hid_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__le16 bcdHID;
	__u8  bCountryCode;
	__u8  bNumDescriptors;

	struct hid_class_descriptor desc[1];

} __attribute__ ((packed));

/*-------------------------------------------------------------------------*/
/*                            HID gadget struct                            */

struct f_hidg_req_list {
	struct usb_request	*req;
	unsigned int		pos;
	struct list_head 	list;
};

struct f_hidg {
	/* configuration */
	unsigned char			bInterfaceSubClass;
	unsigned char			bInterfaceProtocol;
	unsigned short			report_desc_length;
	char				*report_desc;
	unsigned short			report_length;

	/* recv report */
	struct list_head		completed_out_req;

	unsigned int			qlen;

	/* send report */
	struct list_head                completed_in_req;

	struct usb_function		func;

	struct usb_ep			*in_ep;
	struct usb_ep			*out_ep;

	enum hid_type                   label;
	struct list_head       		node;
};

static inline struct f_hidg *func_to_hidg(struct usb_function *f)
{
	return container_of(f, struct f_hidg, func);
}

/*-------------------------------------------------------------------------*/
/*                           Static descriptors                            */

static struct usb_interface_descriptor hidg_interface_desc = {
	.bLength		= sizeof hidg_interface_desc,
	.bDescriptorType	= USB_DT_INTERFACE,
	.bAlternateSetting	= 0,
	.bNumEndpoints		= 2,
	.bInterfaceClass	= USB_CLASS_HID,
};

static struct hid_descriptor hidg_desc = {
	.bLength			= sizeof hidg_desc,
	.bDescriptorType		= HID_DT_HID,
	.bcdHID				= 0x0101,
	.bCountryCode			= 0x00,
	.bNumDescriptors		= 0x1,
};

/* High-Speed Support */

static struct usb_endpoint_descriptor hidg_hs_in_ep_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_IN,
	.bmAttributes		= USB_ENDPOINT_XFER_INT,
	.bInterval		= 4, /* FIXME: Add this field in the
				      * HID gadget configuration?
				      * (struct hidg_func_descriptor)
				      */
};

static struct usb_endpoint_descriptor hidg_hs_out_ep_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_OUT,
	.bmAttributes		= USB_ENDPOINT_XFER_INT,
	.bInterval		= 4, /* FIXME: Add this field in the
				      * HID gadget configuration?
				      * (struct hidg_func_descriptor)
				      */
};

static struct usb_descriptor_header *hidg_hs_descriptors[] = {
	(struct usb_descriptor_header *)&hidg_interface_desc,
	(struct usb_descriptor_header *)&hidg_desc,
	(struct usb_descriptor_header *)&hidg_hs_in_ep_desc,
	(struct usb_descriptor_header *)&hidg_hs_out_ep_desc,
	NULL,
};

/* Full-Speed Support */

static struct usb_endpoint_descriptor hidg_fs_in_ep_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_IN,
	.bmAttributes		= USB_ENDPOINT_XFER_INT,
	.bInterval		= 10, /* FIXME: Add this field in the
				       * HID gadget configuration?
				       * (struct hidg_func_descriptor)
				       */
};

static struct usb_endpoint_descriptor hidg_fs_out_ep_desc = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_OUT,
	.bmAttributes		= USB_ENDPOINT_XFER_INT,
	.bInterval		= 10, /* FIXME: Add this field in the
				       * HID gadget configuration?
				       * (struct hidg_func_descriptor)
				       */
};

static struct usb_descriptor_header *hidg_fs_descriptors[] = {
	(struct usb_descriptor_header *)&hidg_interface_desc,
	(struct usb_descriptor_header *)&hidg_desc,
	(struct usb_descriptor_header *)&hidg_fs_in_ep_desc,
	(struct usb_descriptor_header *)&hidg_fs_out_ep_desc,
	NULL,
};

/*-------------------------------------------------------------------------*/
/*                                usb_function                             */

static struct usb_request *hidg_alloc_ep_req(struct usb_ep *ep, unsigned length)
{
        struct usb_request *req = NULL;

        req = usb_ep_alloc_request(ep, GFP_ATOMIC);
        if (req) {
                req->length = length;
                req->buf = kmalloc(length, GFP_ATOMIC);
                if (!req->buf) {
                        usb_ep_free_request(ep, req);
                        req = NULL;
		}
	}

      return req;
}

static inline void hidg_free_ep_req(struct usb_ep *ep, struct usb_request *req)
{
	if (req && req->buf) {
		kfree(req->buf);
		req->buf = NULL;
	}
	if (req && ep)
		usb_ep_free_request(ep, req);
}

static void hidg_set_report_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_hidg *hidg = (struct f_hidg *) req->context;
	struct f_hidg_req_list *req_list;
	req_list = kzalloc(sizeof(*req_list), 0);
	if (!req_list)
		return;

	req_list->req = req;

	list_add_tail(&req_list->list, &hidg->completed_out_req);

}

static int hidg_setup(struct usb_function *f,
		const struct usb_ctrlrequest *ctrl)
{
	struct f_hidg			*hidg = func_to_hidg(f);
	struct usb_composite_dev	*cdev = f->config->cdev;
	struct usb_request		*req  = cdev->req;
	int status = 0;
	__u16 value, length;

	value	= __le16_to_cpu(ctrl->wValue);
	length	= __le16_to_cpu(ctrl->wLength);

	PRINT_DEBUG("hid_setup crtl_request : bRequestType:0x%x bRequest:0x%x "
		"Value:0x%x\n", ctrl->bRequestType, ctrl->bRequest, value);

	switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {
	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
		  | HID_REQ_GET_REPORT):
		PRINT_DEBUG("get_report\n");

		/* send an empty report */
		length = min_t(unsigned, length, hidg->report_length);
		memset(req->buf, 0x0, length);

		goto respond;
		break;

	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
		  | HID_REQ_GET_PROTOCOL):
		PRINT_DEBUG("get_protocol\n");
		goto stall;
		break;

	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
		  | HID_REQ_SET_REPORT):
		PRINT_DEBUG("set_report | wLenght=%d\n", ctrl->wLength);
		goto stall;
		break;

	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
		  | HID_REQ_SET_PROTOCOL):
		PRINT_DEBUG("set_protocol\n");
		goto stall;
		break;

	case ((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8
		  | USB_REQ_GET_DESCRIPTOR):
		switch (value >> 8) {
		case HID_DT_HID:
			PRINT_DEBUG("USB_REQ_GET_DESCRIPTOR: HID\n");
			length = min_t(unsigned short, length,
						   hidg_desc.bLength);
			memcpy(req->buf, &hidg_desc, length);
			goto respond;
			break;
		case HID_DT_REPORT:
			PRINT_DEBUG("USB_REQ_GET_DESCRIPTOR: REPORT\n");
			length = min_t(unsigned short, length,
						   hidg->report_desc_length);
			memcpy(req->buf, hidg->report_desc, length);
			goto respond;
			break;

		default:
			PRINT_DEBUG("Unknown descriptor request 0x%x\n",
				 value >> 8);
			goto stall;
			break;
		}
		break;

	default:
		goto stall;
		break;
	}

stall:
	return -EOPNOTSUPP;

respond:
	req->length = length;
	status = usb_ep_queue(cdev->gadget->ep0, req, 0);
	if (status < 0)
		sprintf(stderr, "usb_ep_queue error on ep0 %x\n", value);
	return status;
}

static void hidg_disable(struct usb_function *f)
{
	struct f_hidg *hidg = func_to_hidg(f);
	struct f_hidg_req_list *list, *next;

	usb_ep_disable(hidg->in_ep);
	hidg->in_ep->driver_data = NULL;

	usb_ep_disable(hidg->out_ep);
	hidg->out_ep->driver_data = NULL;

	list_for_each_entry_safe(list, next, &hidg->completed_out_req, list) {
		hidg_free_ep_req(hidg->out_ep, list->req);

		list_del(&list->list);

		kfree(list);
	}

	list_for_each_entry_safe(list, next, &hidg->completed_in_req, list) {
		hidg_free_ep_req(hidg->in_ep, list->req);

		list_del(&list->list);

		kfree(list);
	}

}

static int hidg_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_hidg	*hidg = func_to_hidg(f);
	int i, status = 0;

	PRINT_DEBUG("hidg_set_alt intf:%d alt:%d\n", intf, alt);

	if (hidg->in_ep != NULL) {
		/* restart endpoint */
		if (hidg->in_ep->driver_data != NULL)
			usb_ep_disable(hidg->in_ep);

		status = usb_ep_enable(hidg->in_ep, &hidg_hs_in_ep_desc);
		if (status < 0) {
			sprintf(stderr, "Enable IN endpoint FAILED!\n");
			goto fail;
		}
		hidg->in_ep->driver_data = hidg;
	}


	if (hidg->out_ep != NULL) {
		/* restart endpoint */
		if (hidg->out_ep->driver_data != NULL)
			usb_ep_disable(hidg->out_ep);

		status = usb_ep_enable(hidg->out_ep, &hidg_hs_out_ep_desc);
		if (status < 0) {
			sprintf(stderr, "Enable IN endpoint FAILED!\n");
			goto fail;
		}
		hidg->out_ep->driver_data = hidg;

		/*
		 * allocate a bunch of read buffers and queue them all at once.
		 */
		for (i = 0; i < hidg->qlen && status == 0; i++) {
			struct usb_request *req =
					hidg_alloc_ep_req(hidg->out_ep, hidg->report_length);
			if (req) {
				req->complete = hidg_set_report_complete;
				req->context  = hidg;
				status = usb_ep_queue(hidg->out_ep, req,
						      0);
				if (status) {
					sprintf(stderr, "%s queue req --> %d\n",
							hidg->out_ep->name, status);
					hidg_free_ep_req(hidg->out_ep, req);
				}
			} else {
				usb_ep_disable(hidg->out_ep);
				hidg->out_ep->driver_data = NULL;
				status = -ENOMEM;
				goto fail;
			}
		}
	}

fail:
	return status;
}

static inline void usb_free_descriptors(struct usb_descriptor_header **v)
{
        kfree(v);
}


static void usb_free_all_descriptors(struct usb_function *f)
{
        usb_free_descriptors(f->hs_descriptors);
}

extern int usb_assign_descriptors(struct usb_function *f,
		struct usb_descriptor_header **fs,
		struct usb_descriptor_header **hs,
		struct usb_descriptor_header **ss);

static int hidg_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_ep		*ep;
	struct f_hidg		*hidg = func_to_hidg(f);
	int			status;

	/* allocate instance-specific interface IDs, and patch descriptors */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	hidg_interface_desc.bInterfaceNumber = status;

	/* allocate instance-specific endpoints */
	status = -ENODEV;
	ep = usb_ep_autoconfig(c->cdev->gadget, &hidg_fs_in_ep_desc);
	if (!ep)
		goto fail;
	ep->driver_data = c->cdev;	/* claim */
	hidg->in_ep = ep;

	ep = usb_ep_autoconfig(c->cdev->gadget, &hidg_fs_out_ep_desc);
	if (!ep)
		goto fail;
	ep->driver_data = c->cdev;	/* claim */
	hidg->out_ep = ep;

	/* set descriptor dynamic values */
	hidg_interface_desc.bInterfaceSubClass = hidg->bInterfaceSubClass;
	hidg_interface_desc.bInterfaceProtocol = hidg->bInterfaceProtocol;
	hidg_hs_in_ep_desc.wMaxPacketSize = cpu_to_le16(hidg->report_length);
	hidg_fs_in_ep_desc.wMaxPacketSize = cpu_to_le16(hidg->report_length);
	hidg_hs_out_ep_desc.wMaxPacketSize = cpu_to_le16(hidg->report_length);
	hidg_fs_out_ep_desc.wMaxPacketSize = cpu_to_le16(hidg->report_length);
	hidg_desc.desc[0].bDescriptorType = HID_DT_REPORT;
	hidg_desc.desc[0].wDescriptorLength =
		cpu_to_le16(hidg->report_desc_length);

	hidg_hs_in_ep_desc.bEndpointAddress =
		hidg_fs_in_ep_desc.bEndpointAddress;
	hidg_hs_out_ep_desc.bEndpointAddress =
		hidg_fs_out_ep_desc.bEndpointAddress;

	status = usb_assign_descriptors(f, hidg_fs_descriptors,
			hidg_hs_descriptors, NULL);
	if (status)
		goto fail;

	return 0;

fail:
	fprintf(stderr, "hidg_bind FAILED\n");

	usb_free_all_descriptors(f);
	return status;
}

static void hidg_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_hidg *hidg = func_to_hidg(f);
	/* disable/free request and end point */
	usb_ep_disable(hidg->in_ep);
	usb_ep_disable(hidg->out_ep);

	usb_free_all_descriptors(f);

	kfree(hidg->report_desc);
	kfree(hidg);
}

/*-------------------------------------------------------------------------*/
/*                                 Strings                                 */

#define CT_FUNC_HID_IDX	0

static struct usb_string ct_func_string_defs[] = {
	[CT_FUNC_HID_IDX].s	= "HID Interface",
	{},			/* end of list */
};

static struct usb_gadget_strings ct_func_string_table = {
	.language	= 0x0409,	/* en-US */
	.strings	= ct_func_string_defs,
};

static struct usb_gadget_strings *ct_func_strings[] = {
	&ct_func_string_table,
	NULL,
};

static void *kmemdup(const void *src, size_t len, gfp_t gfp)
{
       void *p;

        p = malloc(len);
        if (p)
                memcpy(p, src, len);
	return p;
}

LIST_HEAD(hid_g);

/*-------------------------------------------------------------------------*/
/*                             usb_configuration                           */
int hidg_bind_config(struct usb_configuration *c,
			    struct hidg_func_descriptor *fdesc,
			    const enum hid_type index)
{
	struct f_hidg *hidg;
	int status;

	/* maybe allocate device-global string IDs, and patch descriptors */
	if (ct_func_string_defs[CT_FUNC_HID_IDX].id == 0) {
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		ct_func_string_defs[CT_FUNC_HID_IDX].id = status;
		hidg_interface_desc.iInterface = status;
	}

	/* allocate and initialize one new instance */
	hidg = malloc(sizeof(struct f_hidg));
	if (!hidg)
		return -ENOMEM;

	memset(hidg, 0, sizeof(struct f_hidg));
	INIT_LIST_HEAD(&hidg->completed_out_req);
	INIT_LIST_HEAD(&hidg->completed_in_req);

	hidg->label = index;
	hidg->bInterfaceSubClass = fdesc->subclass;
	hidg->bInterfaceProtocol = fdesc->protocol;
	hidg->report_length = fdesc->report_length;
	hidg->report_desc_length = fdesc->report_desc_length;
	hidg->report_desc = kmemdup(fdesc->report_desc,
				    fdesc->report_desc_length,
				    GFP_KERNEL);
	if (!hidg->report_desc) {
		free(hidg);
		return -ENOMEM;
	}

	hidg->func.name    = "hid";
	hidg->func.strings = ct_func_strings;
	hidg->func.bind    = hidg_bind;
	hidg->func.unbind  = hidg_unbind;
	hidg->func.set_alt = hidg_set_alt;
	hidg->func.disable = hidg_disable;
	hidg->func.setup   = hidg_setup;

	/* this could me made configurable at some point */
	hidg->qlen	   = 1;

	status = usb_add_function(c, &hidg->func);
	if (status)
		free(hidg);

	list_add_tail(&hidg->node, &hid_g);

	return status;
}

static void f_hidg_req_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_hidg *hidg = (struct f_hidg *) req->context;
	struct f_hidg_req_list *list = NULL;


	list = list_first_entry(&hidg->completed_in_req,
					struct f_hidg_req_list, list);
	if (list == NULL) {
		PRINT_DEBUG("is err\n");
		return;
	}

	list_del(&list->list);
	kfree(list);

	hidg_free_ep_req(hidg->in_ep, req);

	PRINT_DEBUG("send is over\n");
}

int submit_status(const unsigned char *buff, const int len, const enum hid_type  label)
{
	int status = 0;
	int count = 0;
	struct f_hidg *hidg = NULL;
	struct list_head *ops = NULL;
	struct f_hidg_req_list *req_list;

	list_for_each(ops, &hid_g) {
		struct f_hidg *temp = container_of(ops, struct f_hidg, node);
		if (temp->label == label) {
			hidg = temp;
			break;
		}
	}

	if (hidg == NULL) {
		fprintf(stderr, "not find device\n");
		return -1;
	}

	struct usb_request *req =
				hidg_alloc_ep_req(hidg->in_ep, hidg->report_length);
	if (req == NULL) {
		sprintf(stderr, "%s queue req --> %d\n",
					hidg->out_ep->name, status);
			hidg_free_ep_req(hidg->out_ep, req);
	}


	count  = min_t(unsigned, len, hidg->report_length);

	memcpy(req->buf, buff, count);

	req->length   = len;
	req->status   = 0;
	req->complete = f_hidg_req_complete;
	req->context  = hidg;

	status = usb_ep_queue(hidg->in_ep, req, 0);
	if (status < 0)
		fprintf(stderr, "usb_ep_queue error on int endpoint %zd\n", status);

	req_list = kzalloc(sizeof(*req_list), 0);
	if (!req_list)
		return -1;

	req_list->req = req;

	list_add_tail(&req_list->list, &hidg->completed_in_req);

	return status;

}

ssize_t f_hidg_read(const enum hid_type label)
{
	struct f_hidg *hidg = NULL;
	struct f_hidg_req_list *list;
	struct usb_request *req;
	int ret;
	struct list_head *ops = NULL;
	int index = 0;

	list_for_each(ops, &hid_g) {
		struct f_hidg *temp = container_of(ops, struct f_hidg, node);
		if (temp->label == label) {
			hidg = temp;
			break;
		}
	}

	if (hidg == NULL) {
		fprintf(stderr, "no find device\n");
		return -1;
	}

#define READ_COND (!list_empty(&hidg->completed_out_req))

	if (!READ_COND)
		return 0;


	/* pick the first one */
	list = list_first_entry(&hidg->completed_out_req,
				struct f_hidg_req_list, list);
	req = list->req;
	for (index = 0; index < req->length; index ++)
		PRINT_DEBUG("%x\n", ((unsigned char *)req->buf)[index]);

	/*
	 * if this request is completely handled and transfered to
	 * userspace, remove its entry from the list and requeue it
	 * again. Otherwise, we will revisit it again upon the next
	 * call, taking into account its current read position.
	 */
	list_del(&list->list);
	kfree(list);
	req->length = hidg->report_length;
	ret = usb_ep_queue(hidg->out_ep, req, GFP_KERNEL);
	if (ret < 0) {
		fprintf(stderr, "hidg out ep queue failed ret %d\n", ret);
		hidg_free_ep_req(hidg->out_ep, req);
		return -ENODEV;
	}

	return req->length;
}

