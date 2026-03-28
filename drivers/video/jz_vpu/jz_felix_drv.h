#ifndef __JZ_FELIX_DRV_H__
#define __JZ_FELIX_DRV_H__

#include "./libh264/include/avcodec.h"
#include "./libh264/include/frame.h"
#include "./libh264/include/h264dec.h"


#define INGENIC_VCODEC_DEC_NAME "felix-vdec"
#define INGENIC_VCODEC_MAX_PLANES	3

enum ingenic_fmt_type {
	INGENIC_FMT_FRAME = 0,
	INGENIC_FMT_DEC = 1,
};

enum felix_raw_format {
	FELIX_TILE_MODE = 0,
	FELIX_420P_MODE = 4,	/* not support */
	FELIX_NV12_MODE = 8,
	FELIX_NV21_MODE = 12,	/* not support */
	FELIX_FORMAT_NONE,
};

struct ingenic_video_fmt {
	u32 fourcc;
	enum ingenic_fmt_type type;
	u32 num_planes;
	enum felix_raw_format format;
};

enum ingenic_vcodec_state {
	INGENIC_STATE_IDLE = 0,
	INGENIC_STATE_HEADER,
	INGENIC_STATE_RUNNING,
	INGENIC_STATE_ABORT,
};


enum ingenic_q_type {
	INGENIC_Q_DATA_SRC = 0,
	INGENIC_Q_DATA_DST = 1,
};

struct ingenic_vdec_ctx {

	int id;	/* used for debug ?*/

	int output_stopped;
	int capture_stopped;

	int int_cond;
	struct list_head src_queue_entry;		// for full buffer
	AVCodecContext *avctx;
	H264Context *h;

};


/******** new add *******************/

typedef struct {
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int size;
	int index;
}Jz_Felix_BufferInfo_t;



typedef struct {
	int width;
	int height;
	short num_src_buf;		// max 2
	short num_dst_buf;		// max 2
	int   out_fmt;			// out_fmt : VPU_FORMAT_NV12 / VPU_FORMAT_NV21 / VPU_FORMAT_TILE420

}Jz_Felix_InitStruct_t;


typedef void Jz_FelixHandle_t;

typedef struct {
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int size;
	AVFrame  frame;		// 在申请完内存后，需要将此字段Frame地址进行填充
	int index;
	struct list_head entry;
}felix_buffer_t;

typedef struct {

	int sWidth;
	int sHeight;

	// src & dst buffer mannager
	struct list_head src_done_list;
	struct list_head src_queue_list;
	struct list_head dst_done_list;
	struct list_head dst_queue_list;

	struct ingenic_vdec_ctx *vdec_ctx;

	felix_buffer_t src_buf[3];
	felix_buffer_t dst_buf[3];

	AVFrame dec_frame;
}felix_prv_ctx_t;

felix_prv_ctx_t* jz_felix_init(Jz_Felix_InitStruct_t *init);

int jz_felix_deinit(felix_prv_ctx_t* handle);

int jz_felix_send_packet(felix_prv_ctx_t* handle,char* pack,unsigned int size);

int jz_felix_get_frame(felix_prv_ctx_t* handle, Jz_Felix_BufferInfo_t *buf);

int jz_felix_put_frame(felix_prv_ctx_t *handle, Jz_Felix_BufferInfo_t *buf);
#endif
