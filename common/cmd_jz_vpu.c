#include <common.h>
#include <lcd.h>
#include <bmp_layout.h>
#include <command.h>
#include <asm/byteorder.h>
#include <malloc.h>
#include <splash.h>
#include <video.h>
#include <nand.h>
#include "jz_felix_drv.h"

DECLARE_GLOBAL_DATA_PTR;

typedef struct {
	unsigned char *stream;
	unsigned char type;
	unsigned int size;
} nal_unit_t;
#define MAX_NALS_PER_TIME       9000	//25fps@60min
typedef struct {
        nal_unit_t nal[MAX_NALS_PER_TIME];
        int nal_count;
} nal_units_t;

nal_units_t nal_units;

void tile420_y_uv_to_rgb888(unsigned char *src_y, unsigned char *src_uv, int src_width, int src_height,
	int dst_width, int dst_height, int dst_stride, unsigned char* dstAddr)
{

	int line = 0, col = 0, linewidth = 0;
	int y = 0, u = 0, v = 0, yy = 0, vr = 0, ug = 0, vg = 0, ub = 0;
	int r = 0, g = 0, b = 0;
	const unsigned char *py = NULL, *pu = NULL, *pv = NULL;
	int width = src_width;
	int height = src_height;
	int ySize = width * height;
	ySize = (ySize + 255)/256;

	unsigned int *dst = (unsigned int *)dstAddr;

	py = src_y;

	pu = src_uv;
	pv = pu + 8;

	int iLoop = 0, jLoop = 0, kLoop = 0, dxy = 0;
	int stride_y = width*16;
	int stride_uv = width*8;

	/* fix stride. */
	dst_stride = dst_stride == 0 ? dst_width:dst_stride;

	for (line = 0; line < height; line++) {
		/*ignore*/
		if(line >= dst_height) {
			line = height;
			continue;
		}
		for (col = 0; col < width; col++) {
			if ( iLoop > 0 && iLoop % 16 == 0 ) {
				jLoop++;
				iLoop = 0;
				dxy = jLoop*256;
				iLoop++;
			} else {
				dxy = iLoop + jLoop * 256;
				iLoop++;
			}

			y = *(py+dxy);
			yy = y << 8;
			u = *(pu+dxy/2) - 128;
			ug = 88 * u;
			ub = 454 * u;
			v = *(pv+dxy/2) - 128;
			vg = 183 * v;
			vr = 359 * v;

			r = (yy + vr) >> 8;
			g = (yy - ug - vg) >> 8;
			b = (yy + ub ) >> 8;

			if (r < 0) r = 0;
			if (r > 255) r = 255;
			if (g < 0) g = 0;
			if (g > 255) g = 255;
			if (b < 0) b = 0;
			if (b > 255) b = 255;
			/*ignore*/
			if(col >= dst_width) {
				col = width;
				dst += dst_stride - dst_width;
				continue;
			}

			*dst++ = ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);

		}
		if ( kLoop > 0 && kLoop % 15 == 0 ) {
			py += stride_y + 16 - 256;
			pu += stride_uv + 16 - 128;
			pv = pu + 8;
			iLoop = 0; jLoop = 0; kLoop = 0;
		}
		else if ( kLoop & 1 ) {
			py += 16;
			pu += 16;
			pv = pu + 8;
			iLoop = 0; jLoop = 0; kLoop++;
		}
		else {
			py += 16;
			iLoop = 0; jLoop = 0;
			kLoop++;
		}

	}
}


static int m_find_start_code(unsigned char *buf, unsigned int len, unsigned int *prefix_bytes)
{
        unsigned char *start = buf;
        unsigned char *end = buf + len;
        int index = 0;


        if(start >= end)
                return -1;

        while(start < end) {
#if 1
                /* next24bits */
                index = start - buf;

                if (start[0] == 0 && start[1] == 0 && start[2] == 1) {
                        *prefix_bytes = 3;
                        return index + 3;
                }

		/* next32bits */
                if (start[0] == 0 && start[1] == 0 && start[2] == 0 && start[3] == 1) {
                        *prefix_bytes = 4;
                        return index + 4;
                }

                start++;


                /* end of data buffer, no need to find. */
		if((start + 3) == end) {
                        return -1;
                }
#endif
        }

        return -1;
}

static unsigned int get_nal_size(char *buf, unsigned int len)
{
        char *start = buf;
        char *end = buf + len;
	unsigned int size = 0;

        if(start >= end)
                return -1;

        while(start < end) {

                if (start[0] == 0 && start[1] == 0 && start[2] == 1) {
			break;
                }

                start++;
        }

	size = (unsigned int) (start - buf);

        return size;
}


void dumphex(unsigned char *buf, int len)
{
        int i;

        for (i = 0; i < len; i++) {
                if ((i % 16) == 0)
                        printf("%s%08x: ", i ? "\n" : "",
                                        (unsigned int)&buf[i]);
                printf("%02x ", buf[i]);
        }
        printf("\n");
}


static void dump_nal_units(nal_units_t *nals)
{
	int i;
	nal_unit_t *nal;

	for(i = 0; i < nals->nal_count; i++) {
		printf("nal: %d\n", i);
		nal = &nals->nal[i];
		dumphex(nal->stream, nal->size < 32 ? nal->size : 32);
	}

}

int extract_nal_units(nal_units_t *nals, unsigned char *buf, unsigned int len)
{
	int index = 0;
	int next_nal = 0;
	unsigned char *start = buf;
	unsigned char *end = buf + len;
	int size = 0;
	unsigned int prefix_bytes; /*000001/00000001*/
	nal_unit_t *nal;

	unsigned int left = len;
	int i = 0;

	int eos = 0;
	nals->nal_count = 0;

	for(i = 0; i < MAX_NALS_PER_TIME; i++) {

		nal = &nals->nal[i];

		index = m_find_start_code(start, left, &prefix_bytes);
		if(index < 0 ) {
			printf("no start code found!\n");
			return -EINVAL;
		}

		nal->stream = start;
		nal->type = nal->stream[index];

		nal->size = get_nal_size(start + index, left - index) + index;
		nals->nal_count++;

		start += nal->size;
		left -= nal->size;

		if(left <= 0) {
			/*EOS*/
			break;
		}
	}

	if(left) {
		printf("too many nals extraced!, %d bytes left\n", left);
	}

	//dump_nal_units(nals);

	return len - left;
}

static int do_jzvpu_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	unsigned char *paddr = NULL;
	unsigned long *h264 = (unsigned long*)0x80a00000;//测试文件地址
	unsigned int datasize = 1181613;//测试文件大小
	paddr = (unsigned char*)malloc(datasize);
	memcpy(paddr, h264, datasize);

	Jz_Felix_InitStruct_t init;
	init.width = 1280;
	init.height = 720;
	init.num_src_buf = 3;
	init.num_dst_buf = 3;
	felix_prv_ctx_t *handle = jz_felix_init(&init);
	Jz_Felix_BufferInfo_t buf;
	int ret = 0;
	int packet_len = 0;
	nal_unit_t *nal = NULL;
	nal_unit_t *last_nal = NULL;
	unsigned long current_offset = 0;
	void *slice_data = NULL;
	unsigned int slice_size = 0;

	packet_len = datasize;
	nal_units.nal_count = 0;
	ret = extract_nal_units(&nal_units, paddr, datasize);
	slice_size = 0;
	if(ret < packet_len) {
		printf("packet not extracted completely!\n");
	}

	int need_merge = 0;
	int nal_count = 0;
	int i = 0;
	int x = 0;
	int display_w;
	int display_h;
	int fb_w;
	int pic_w;
	int pic_h;
	unsigned char *y;
	unsigned char *uv;

	last_nal = &nal_units.nal[nal_units.nal_count - 1];
	nal_count = nal_units.nal_count;
	printf("nal count = %d\n", nal_units.nal_count);
	printf("=============== h264_decoder start ===============\n");
	for(i = 0; i < nal_count; i++){
		nal = &nal_units.nal[i];

		if((nal->type & 0x1f) == 0x7) { /* SPS */
			need_merge = 1;
			slice_data = nal->stream;
			slice_size = nal->size;
			continue;
		}
		if((nal->type & 0x1f) == 0x8) { /* PPS */
			slice_size += nal->size;
			continue;
		}
		if((nal->type & 0x1f) == 0xc) {	//filler na, 0xff
			slice_size += nal->size;
			continue;
		}
		if((nal->type & 0x1f) == 0x06) { //vui (SEI)
			slice_size += nal->size;
			continue;
		}

		if((nal->type & 0x1f) == 0x5 && need_merge) { /* IDR */
			need_merge = 0;
			slice_size += nal->size;
		} else {
			slice_data = nal->stream;
			slice_size = nal->size;
		}

		ret = jz_felix_send_packet(handle, slice_data, slice_size);
		if(ret)
			printf("send packet failed!\n");

		ret = jz_felix_get_frame(handle, &buf);
		if(ret){
			printf("get frame failed!\n");
			return 0;
		}

		void *lcd_base = (void *)gd->fb_base;

		y = (char *)buf.vaddr;
		uv = (char *)buf.vaddr + init.width * init.height;

		tile420_y_uv_to_rgb888(y, uv, init.width, init.height, 720, 1280, 720, lcd_base);

		jz_felix_put_frame(handle, &buf);
	}
	printf("=============== h264_decoder done ===============\n");
	return 0;
}

U_BOOT_CMD(
	jzvpu,	5,	1,	do_jzvpu_test,
	"manipulate BMP image data",
	"info <imageAddr>          - display image info\n"
	"bmp display <imageAddr> [x y] - display image at x,y"
);

