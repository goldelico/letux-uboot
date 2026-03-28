#include <common.h>
#include <command.h>
#include <errno.h>
#include <usb.h>

#define USB_MAX_load_DEV 5
#define STAGE_ADDR_MSB(addr) ((addr) >> 16)
#define STAGE_ADDR_LSB(addr) ((addr) & 0xffff)

#define VR_GET_CPU_INFO         0x00
#define VR_SET_DATA_ADDR        0x01
#define VR_SET_DATA_LEN         0x02
#define VR_FLUSH_CACHE          0x03
#define VR_PROG_STAGE1          0x04
#define VR_PROG_STAGE2          0x05

struct load_device {
	struct usb_device *pusb_dev;	 /* this usb_device */
	unsigned char	ifnum;			/* interface number */
	unsigned char	ep_in;			/* in endpoint */
	unsigned char	ep_out;			/* out endpoint */
	unsigned char	cpuinfo[32];
};

static int load_max_devs;
struct load_device usb_load[USB_MAX_load_DEV];

static int usb_load_flush_cache(struct load_device *ld)
{
	int status;
	status = usb_control_msg(ld->pusb_dev,
			      usb_rcvctrlpipe(ld->pusb_dev, 0),
			      VR_FLUSH_CACHE,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_OUT,
			      0, 0,
			      NULL, 0,
			      USB_CNTL_TIMEOUT * 20);

	if (status != 0) {
                printf("(%c)%s:%d flush cache error! (%d)\n",
                           ld->ifnum,__FUNCTION__,__LINE__,status);
                return status;
        }

        return 0;

}

static int usb_load_get_cpuinfo(struct load_device *ld)
{
	int len;
	len = usb_control_msg(ld->pusb_dev,
			      usb_rcvctrlpipe(ld->pusb_dev, 0),
			      VR_GET_CPU_INFO,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_IN,
			      0, 0,
			      ld->cpuinfo, 32,
			      USB_CNTL_TIMEOUT * 5);
	debug("Get CPU INFO -> len = %i, cpuinfo : %s\n", len, ld->cpuinfo);
	return len;
}

static int usb_load_set_data_address(struct load_device *ld, unsigned int address)
{
	int status;
	status = usb_control_msg(ld->pusb_dev,
			      usb_rcvctrlpipe(ld->pusb_dev, 0),
			      VR_SET_DATA_ADDR,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_OUT,
			      STAGE_ADDR_MSB(address), STAGE_ADDR_LSB(address),
			      NULL, 0,
			      USB_CNTL_TIMEOUT * 20);

	if (status != 0) {
                printf("(%c)%s:%d set data address error! (%d)\n",
                           ld->ifnum,__FUNCTION__,__LINE__,status);
                return status;
        }

        return 0;
}

static int usb_load_set_data_length(struct load_device *ld, int length)
{
	int status;
	status = usb_control_msg(ld->pusb_dev,
			      usb_rcvctrlpipe(ld->pusb_dev, 0),
			      VR_SET_DATA_LEN,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_OUT,
			      STAGE_ADDR_MSB(length), STAGE_ADDR_LSB(length),
			      NULL, 0,
			      USB_CNTL_TIMEOUT * 20);

	if (status != 0) {
                printf("(%c)%s:%d set data length error! (%d)\n",
                           ld->ifnum,__FUNCTION__,__LINE__,status);
                return status;
        }

        return 0;
}

static int usb_load_send_data(struct load_device *ld, char *buf, int length)
{
	int pipe;
	int max_size;
	int this_xfer;
	int result;
	int partial;
	int maxtry;
	int stat;

	if(buf == NULL || ld == NULL)
		return;
	pipe = usb_sndbulkpipe(ld->pusb_dev, ld->ep_out);
	/* determine the maximum packet size for these transfers */
	max_size = usb_maxpacket(ld->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	while (length) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = length > max_size ? max_size : length;
		length -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		do {
			/* transfer the data */
			debug("Bulk xfer 0x%x(%d) try #%d\n",
			      (unsigned int)buf, this_xfer, 11 - maxtry);
			result = usb_bulk_msg(ld->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      result, partial, this_xfer);
			if (ld->pusb_dev->status != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#ifdef DEBUG
				display_int_status(ld->pusb_dev->status);
#endif
				if (ld->pusb_dev->status & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = ld->pusb_dev->status;
					usb_clear_halt(ld->pusb_dev, pipe);
					ld->pusb_dev->status = stat;
					if (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      ld->pusb_dev->status);
						return 0;
					}
					else
						return result;
				}
				if (ld->pusb_dev->status & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					return result;
				}
				debug("bulk transferred with error");
				if (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      ld->pusb_dev->status);
					return 0;
				}
				/* if our try counter reaches 0, bail out */
					debug(" %ld, data %d\n",
					      ld->pusb_dev->status, partial);
				if (!maxtry--)
						return result;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} while (this_xfer);
	}

	/* if we get here, we're done and successful */
	return 0;
}

static int usb_load_run_program(struct load_device *ld, unsigned int address, bool is_stage1)
{
	int status;
	status = usb_control_msg(ld->pusb_dev,
			      usb_rcvctrlpipe(ld->pusb_dev, 0),
			      is_stage1?VR_PROG_STAGE1:VR_PROG_STAGE2,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_OUT,
			      STAGE_ADDR_MSB(address), STAGE_ADDR_LSB(address),
			      NULL, 0,
			      USB_CNTL_TIMEOUT * 20);

	if (status != 0) {
                printf("(%c)%s:%d run program error! (%d)\n",
                           ld->ifnum,__FUNCTION__,__LINE__,status);
                return status;
        }

        return 0;
}

int usb_load_run_stage1_firmware(unsigned char *data, unsigned int offset, int len)
{
	int ret;
	struct load_device *ld = &usb_load[0];

	debug(" addr: %x offset: %x len: %d\n",data, offset, len);
	ret = usb_load_set_data_address(ld, offset);
	if(ret)
		return ret;

	ret = usb_load_set_data_length(ld, len);
	if(ret)
		return ret;

	ret = usb_load_send_data(ld, data, len);
	if(ret != 0) {
                printf("(%c)%s:%d send data error!(%d)\n",
                                  ld->ifnum,__FUNCTION__,__LINE__,ret);
                return ret;
        }

	ret = usb_load_flush_cache(ld);
	if(ret)
                return ret;

	return usb_load_run_program(ld, offset, true);
}

int usb_load_run_stage2_firmware(unsigned char *data, unsigned int offset, int len)
{
	int ret;
	struct load_device *ld = &usb_load[0];

	debug(" addr: %x offset: %x len: %d\n",data, offset, len);
	ret = usb_load_set_data_address(ld, offset);
	if(ret)
		return ret;

	ret = usb_load_set_data_length(ld, len);
	if(ret)
		return ret;

	ret = usb_load_send_data(ld, data, len);
	if(ret != 0) {
                printf("(%c)%s:%d send data error!(%d)\n",
                                  ld->ifnum,__FUNCTION__,__LINE__,ret);
                return ret;
        }

	ret = usb_load_flush_cache(ld);
	if(ret)
                return ret;

	return usb_load_run_program(ld, offset, false);
}

static int usb_load_probe(struct usb_device *dev, unsigned int ifnum,
		      struct load_device *ld)
{
	int i;
	struct usb_interface *iface;
	struct usb_endpoint_descriptor *ep_desc;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	if (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_VENDOR_SPEC ) {
		return 0;
	}

	printf("Found load device\n");

	memset(ld, 0, sizeof(struct load_device));

	debug("\n\nUSB load device detected\n");

	ld->ifnum = ifnum;
	ld->pusb_dev = dev;

	for (i = 0; i < iface->desc.bNumEndpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		if ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			if (ep_desc->bEndpointAddress & USB_DIR_IN)
				ld->ep_in = ep_desc->bEndpointAddress &
						USB_ENDPOINT_NUMBER_MASK;
			else
				ld->ep_out =
					ep_desc->bEndpointAddress &
					USB_ENDPOINT_NUMBER_MASK;
		}
	}

	debug("Endpoints In %d Out %d\n",ld->ep_in, ld->ep_out);

	if (usb_set_interface(dev, iface->desc.bInterfaceNumber, 0) ||
	    !ld->ep_in || !ld->ep_out) {
		debug("Problems with device\n");
		return 0;
	}

	dev->privptr = (void *)ld;
	return 1;
}

int usb_load_scan(int mode)
{
	unsigned char i;
	struct usb_device *dev;

	if (mode == 1)
		printf("       scanning usb for load devices... ");

	usb_disable_asynch(1); /* asynch transfer not allowed */

	load_max_devs = 0;
	for (i = 0; i < USB_MAX_DEVICE; i++) {
		dev = usb_get_dev_index(i); /* get device */
		debug("i=%d\n", i);
		if (dev == NULL)
			break; /* no more devices available */

		if (usb_load_probe(dev, 0, &usb_load[load_max_devs])) {
			usb_load_get_cpuinfo(&usb_load[load_max_devs]);
			printf("cpuinfo: %s",usb_load[load_max_devs].cpuinfo);
			printf("(VID:%04x;PID:%04x)\n",le16_to_cpu(dev->descriptor.idVendor),
					le16_to_cpu(dev->descriptor.idProduct));
			load_max_devs ++;
		}
		/* if storage device */
		if (load_max_devs == USB_MAX_load_DEV) {
			printf("max USB load Device reached: %d stopping\n",
				load_max_devs);
			break;
		}
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
	printf("%d load Device(s) found\n", load_max_devs);

	if (load_max_devs > 0)
		return 0;
	return -1;
}

