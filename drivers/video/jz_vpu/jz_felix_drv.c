#include <asm/io.h>
#include <config.h>
#include <serial.h>
#include <common.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <asm/gpio.h>
#include <asm/jz_cache.h>

#include "jz_felix_drv.h"

#define vpu_readl(offset)  \
    readl(VPU_BASE + (offset))
#define vpu_writel(offset, data)   \
	    writel((data), VPU_BASE + (offset))

#define REG_VPU_STATUS ( *(volatile unsigned int*)0xb3200034 )
#define REG_VPU_LOCK ( *(volatile unsigned int*)0xb329004c )
#define REG_VPUCDR ( *(volatile unsigned int*)0xb0000030 )
#define REG_CPM_VPU_SWRST ( *(volatile unsigned int*)0xb00000c4 )
#define REG_VPU_TLBBASE (*(volatile unsigned int *)(0x30 + 0xb3200000))
#define CPM_VPU_SR           (0x1<<31)
#define CPM_VPU_STP          (0x1<<30)
#define CPM_VPU_ACK          (0x1<<29)



#define REG_VPU_GLBC      0x00000
#define VPU_INTE_ACFGERR     (0x1<<20)
#define VPU_INTE_TLBERR      (0x1<<18)
#define VPU_INTE_BSERR       (0x1<<17)
#define VPU_INTE_ENDF        (0x1<<16)

#define REG_VPU_STAT      0x00034
#define VPU_STAT_ENDF    (0x1<<0)
#define VPU_STAT_BPF     (0x1<<1)
#define VPU_STAT_ACFGERR (0x1<<2)
#define VPU_STAT_TIMEOUT (0x1<<3)
#define VPU_STAT_JPGEND  (0x1<<4)
#define VPU_STAT_BSERR   (0x1<<7)

#define VPU_STAT_TLBERR  (0x1F<<10)
#define VPU_STAT_SLDERR  (0x1<<16)

#define REG_VPU_JPGC_STAT 0xE0008
#define JPGC_STAT_ENDF   (0x1<<31)

#define REG_VPU_SDE_STAT  0x90000
#define SDE_STAT_BSEND   (0x1<<1)

#define REG_VPU_DBLK_STAT 0x70070
#define DBLK_STAT_DOEND  (0x1<<0)

#define REG_VPU_AUX_STAT  0xA0010
#define AUX_STAT_MIRQP   (0x1<<0)


/********************************************
  SW_RESET (VPU software reset)
*********************************************/
#define REG_CFGC_SW_RESET    0x00000
#define REG_CFGC_RST         (0x1<<30)
#define REG_CFGC_RST_CLR     (0x0<<30)
#define REG_CFGC_EARB_STAT   0x0000d
#define REG_CFGC_EARB_EMPT   (0x20000)

static void vpu_dump_regs()
{
#if 1
	printf("======================================= >>>>>>>>>> %s\n",__func__);
	printf("REG_SDE_STAT:           0x%x\n", vpu_readl(REG_SDE_STAT));   //0x13290000
	printf("REG_SDE_CFG0:           0x%x\n", vpu_readl(REG_SDE_CFG0));   //0x13290014
	printf("REG_SDE_CFG1:           0x%x\n", vpu_readl(REG_SDE_CFG1));   //0x13290018
	printf("REG_SDE_CFG13:          0x%x\n", vpu_readl(REG_SDE_CFG13));  //0x13290048
	printf("REG_SDE_CFG14:          0x%x\n", vpu_readl(REG_SDE_CFG14));  //0x1329004c
	printf("REG_DBLK_GSTA:          0x%x\n", vpu_readl(REG_DBLK_GSTA));  //0x13270070
	printf("REG_VMAU_POS:           0x%x\n", vpu_readl(REG_VMAU_POS));   //0x13280060
	printf("REG_SCH_GLBC:           0x%x\n", vpu_readl(REG_SCH_GLBC));   //0x13200000
	printf("REG_SCH_STAT:           0x%x\n", vpu_readl(REG_SCH_STAT));   //0x13200034
	printf("REG_VDMA_TASKRG:        0x%x\n", vpu_readl(REG_VDMA_TASKRG));//0x13210008
	printf("REG_VDMA_TASKST:        0x%x\n", vpu_readl(REG_VDMA_TASKST));//0x1321000c
#endif
}

void vpu_dump_data(char* pack, unsigned int size)
{
	int i = 0;
	printf("================> dump buf data <================\n");
	printf("pack : 0x%08x, size : %d\n", pack, size);
	for(i = 0; i < size; i++){
		if((i != 0) && (i % 8 == 0))
			printf("%02x \n",pack[i]);
		else
			printf("%02x ",pack[i]);
	}
	printf("\n");
	printf("================> dump      end <================\n");
}

static void inline vpu_clear_bits(unsigned int offset, unsigned int bits)
{
	unsigned int val = vpu_readl(offset);
	val &= ~bits;
	vpu_writel(offset, val);
}

static int ingenic_vpu_status_handle(struct ingenic_vdec_ctx *ctx)
{
	H264Context *h = ctx->h;
	vpu_ctx_t *vpu_ctx = &h->vpu_ctx;
	unsigned long flags;

	unsigned int vpu_stat;
	unsigned int sde_stat;
	int err = 0;

#define check_vpu_status(STAT, fmt, args...) do {       \
		if(vpu_stat & STAT)                     \
		printf(fmt, ##args);				\
	}while(0)


	vpu_stat = vpu_readl(REG_SCH_STAT);
	sde_stat = vpu_readl(REG_SDE_STAT);

	if(vpu_stat) {
		if(vpu_stat & VPU_STAT_ENDF) {
			if(vpu_stat & VPU_STAT_JPGEND) {
				printf("JPG successfully done!\n");
				vpu_stat = vpu_readl(REG_VPU_JPGC_STAT);
				vpu_clear_bits(REG_VPU_JPGC_STAT, JPGC_STAT_ENDF);
			} else {
				printf("SCH successfully done!\n");
				vpu_clear_bits(REG_VPU_SDE_STAT, SDE_STAT_BSEND);
				vpu_clear_bits(REG_VPU_DBLK_STAT, DBLK_STAT_DOEND);
			}
		} else {
			err = 1;
			check_vpu_status(VPU_STAT_SLDERR, "SHLD error!\n");
			check_vpu_status(VPU_STAT_TLBERR, "TLB error! Addr is 0x%08x\n",
					vpu_readl(REG_VPU_STAT));
			check_vpu_status(VPU_STAT_BSERR, "BS error!\n");
			check_vpu_status(VPU_STAT_ACFGERR, "ACFG error!\n");
			check_vpu_status(VPU_STAT_TIMEOUT, "TIMEOUT error!\n");
			vpu_clear_bits(REG_VPU_GLBC, (VPU_INTE_ACFGERR |
						VPU_INTE_TLBERR | VPU_INTE_BSERR |
						VPU_INTE_ENDF));
		}
	} else {
		if(vpu_readl(REG_VPU_AUX_STAT) & AUX_STAT_MIRQP) {
			printf("AUX successfully done!\n");
			vpu_clear_bits(REG_VPU_AUX_STAT, AUX_STAT_MIRQP);
		} else {
			printf("illegal interrupt happened!\n");
			err = 1;
			return err;
		}
	}

	ctx->int_cond = 1;
	vpu_ctx->error = err;
	vpu_ctx->sch_stat = vpu_stat;
	vpu_ctx->sde_stat = sde_stat;
	//ctx->int_status = vpu_stat;
	return err;
}

static int vpu_reset_x2000(void)
{
	int timeout = 0x8ffff;
	vpu_writel(REG_CFGC_SW_RESET, REG_CFGC_RST);
	while((vpu_readl(REG_CFGC_EARB_STAT) & REG_CFGC_EARB_EMPT) && --timeout);
	if(!timeout) {
		printf("vpu reset timeout!!!!!!!\r\n");
		return -1;
	}

	return 0;
}

/*global clk on.*/
static int vpu_on(void)
{
	*(volatile unsigned int*)0xb0000020 = 0;
	*(volatile unsigned int*)0xb0000028 = 0;
	__asm__ __volatile__ (
			"mfc0  $2, $16,  7   \n\t"
			"ori   $2, $2, 0x340 \n\t"
			"andi  $2, $2, 0x3ff \n\t"
			"mtc0  $2, $16,  7  \n\t"
			"nop                  \n\t");

	return 0;
}

/* shutdown */
static int vpu_off(struct ingenic_vdec_dev *dev)
{
	unsigned int tmp = *(volatile unsigned int*)0xb0000028;
	tmp |= 1 << 4;
	*(volatile unsigned int*)0xb0000028 = tmp;

	return 0;
}



static int ingenic_vpu_start(void *priv)
{
	// 在这里只控制硬件的启动工作，不做等待
	// 中断禁用
	struct ingenic_vdec_ctx *ctx = (struct ingenic_vdec_ctx *)priv;
	H264Context *h = ctx->h;
	vpu_ctx_t *vpu_ctx = &h->vpu_ctx;

	unsigned int sch_glbc = 0;
	unsigned int des_pa;
	unsigned int vpu_stat;
	unsigned int sde_stat;


	des_pa = vpu_ctx->desc_pa;
	vpu_reset_x2000();

	sch_glbc = SCH_GLBC_HIAXI | SCH_INTE_ACFGERR | SCH_INTE_BSERR | SCH_INTE_ENDF;

	vpu_writel(REG_SCH_GLBC, sch_glbc);
	/*trigger start.*/
	vpu_writel(REG_VDMA_TASKRG, VDMA_ACFG_DHA(des_pa) | VDMA_ACFG_RUN);
//	mdelay(100);
	ingenic_vpu_status_handle(ctx);
	return 0;
}

static int ingenic_vpu_wait(void *priv)
{
	return 0;
}

static int ingenic_vpu_stop(void *priv)
{
	// 无操作
	return 0;
}

struct vpu_ops ingenic_vpu_ops = {
	.start = ingenic_vpu_start,
	.wait = ingenic_vpu_wait,
	.end = ingenic_vpu_stop,
};

static int _init_buffer(felix_prv_ctx_t* fctx, int dst_src, unsigned int y_size, unsigned int uv_size, int num)
{
	if(y_size <= 0)
		return -1;

	felix_buffer_t *buf = NULL;
	int j = 0,i = 0;
	if(dst_src == 0){	// 0 is creat src buf
		for(i = 0; i < num; i++){
			buf = &fctx->src_buf[i];
			buf->vaddr = (unsigned int)av_malloc(y_size + uv_size);		// malloc in kseg0
			buf->paddr = buf->vaddr & 0x7FFFFFFF;
			buf->size  = y_size + uv_size;
			buf->index = i;
			buf->frame.buf[0] = av_buffer_create((char*)buf->vaddr,(char*)buf->paddr,y_size);	// src is h264 only one plane
		}
	} else {	// dst buf create
		for(j = 0; j < num; j++){
			buf = &fctx->dst_buf[j];
			buf->vaddr = (unsigned int)av_malloc(y_size + uv_size);		// malloc in kseg0
			buf->paddr = buf->vaddr & 0x7FFFFFFF;
			buf->size  = y_size + uv_size;
			buf->index = j;
			// only nv12 / nv21
			buf->frame.buf[0] = av_buffer_create((char*)buf->vaddr,(char*)buf->paddr,y_size);
			buf->frame.buf[1] = av_buffer_create((char*)(buf->vaddr + y_size),(char*)(buf->paddr+y_size),uv_size);
		}
	}

	return 0;
}

felix_prv_ctx_t* jz_felix_init(Jz_Felix_InitStruct_t *init)
{
	unsigned int tmp = 0;
	int ret = 0;

	// felix low power disable
	tmp = *(volatile unsigned int*)0xb0000004;
	tmp = 0 ;
	*(volatile unsigned int*)0xb0000004 = tmp;

	tmp = *(volatile unsigned int*)0xb0000004;
	// s1 some struct resource prepare
	felix_prv_ctx_t *fctx = (felix_prv_ctx_t*)malloc(sizeof(felix_prv_ctx_t));

	struct ingenic_vdec_ctx *ctx = (struct ingenic_vdec_ctx*)malloc(sizeof(struct ingenic_vdec_ctx));
	memset(ctx,0,sizeof(struct ingenic_vdec_ctx));

	ctx->avctx = malloc(sizeof(AVCodecContext));
	memset(ctx->avctx,0,sizeof(AVCodecContext));

	ctx->h = malloc(sizeof(H264Context));
	memset(ctx->h,0,sizeof(H264Context));

	ctx->avctx->priv_data = ctx->h;
	ctx->avctx->width = init->width;
	ctx->avctx->height = init->height;

	ret = h264_decode_init(ctx->avctx);
	if(ret < 0){
		free(ctx->h);
		free(ctx->avctx);
		free(ctx);
		free(fctx);
		return -1;
	}


	h264_set_vpu_ops(ctx->h, ctx, &ingenic_vpu_ops);
	fctx->vdec_ctx = ctx;
	// VPU ON
	vpu_on();

	h264_set_output_format(ctx->h, VPU_FORMAT_TILE420, init->width, init->height);
	// src & dst buffer prepare
	INIT_LIST_HEAD(&fctx->src_done_list);
	INIT_LIST_HEAD(&fctx->src_queue_list);
	INIT_LIST_HEAD(&fctx->dst_done_list);
	INIT_LIST_HEAD(&fctx->dst_queue_list);

	// src_buf create
	ret = _init_buffer(fctx,0,init->width * init->height * 4 / 3, 0, 3);
	//dst_buf create
	ret = _init_buffer(fctx,1,init->width * init->height, init->width * init->height / 2, 3);

	flush_cache_all();
	// add src buffer to done list,affter user fill h264 data .then add to queue list
	// 在用户填充h264数据后，将src缓冲区添加到完成列表中。然后添加到队列列表中
	list_add_tail(&fctx->src_buf[0].entry,&fctx->src_done_list);
	list_add_tail(&fctx->src_buf[1].entry,&fctx->src_done_list);
	list_add_tail(&fctx->src_buf[2].entry,&fctx->src_done_list);
	// add dst buffer to queue list, affter fill data ,add to done list
	// 将dst缓冲区添加到队列列表，填充数据后，添加到完成列表
	list_add_tail(&fctx->dst_buf[0].entry,&fctx->dst_queue_list);
	list_add_tail(&fctx->dst_buf[1].entry,&fctx->dst_queue_list);
	list_add_tail(&fctx->dst_buf[2].entry,&fctx->dst_queue_list);

	fctx->sWidth = init->width;
	fctx->sHeight = init->height;

	return fctx;
}

int jz_felix_deinit(felix_prv_ctx_t* handle)
{
	felix_prv_ctx_t *fctx = (felix_prv_ctx_t *)handle;
	// TODO Deinit
	// S1 close clk
	// S2 deinit decoder
	// S3 vpu stop
	// S4 release struct resource
	free(fctx->vdec_ctx->h);
	free(fctx->vdec_ctx->avctx);
	free(fctx->vdec_ctx);
	free(fctx);
	return 0;
}


void dd(unsigned char *buf, int len)
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

int jz_felix_send_packet(felix_prv_ctx_t *handle,char* pack, unsigned int size)
{
	// send packet && wait decode end
	int ret = 0;
	int retval = 0;
	felix_prv_ctx_t *fctx = (felix_prv_ctx_t*)handle;
	AVPacket avpkt;
	AVCodecContext *avctx = fctx->vdec_ctx->avctx;
	H264Context *h = fctx->vdec_ctx->h;
	int got_frame = 0;
	felix_buffer_t *dst_buf;
	felix_buffer_t *src_buf;
	AVFrame *f = NULL;

	src_buf = list_first_entry_or_null(&fctx->src_done_list, felix_buffer_t, entry);
	if(!src_buf){
		printf("get src-buffer failed\r\n");
		return -1;
	}
	if(size > src_buf->size){
		printf("the pack size is invalid !!! size = %d, src_buf->size = %d\r\n", size, src_buf->size);
		list_add_tail(&src_buf->entry, &fctx->src_done_list);
		return -1;
	}
	list_del(&src_buf->entry);

	memcpy(src_buf->vaddr, pack, size);
	avpkt.size = size;
	avpkt.data_pa = src_buf->paddr;
	avpkt.data = src_buf->vaddr;


	// prepare dst buf
	dst_buf = list_first_entry_or_null(&fctx->dst_queue_list, felix_buffer_t, entry);
	if(!dst_buf){
		printf("get dst-buffer failed\r\n");
		list_add_tail(&src_buf->entry,&fctx->src_done_list);
		return -1;
	} else {
		list_del(&dst_buf->entry);
		h264_enqueue_frame(h,&dst_buf->frame);
	}

	//TODO flush cache
	flush_cache_all();
	ret = h264_decode_frame(avctx, NULL, &got_frame, &avpkt);
	if(ret < 0){
		printf("decode failed !!!!!!\r\n");
		list_add_tail(&src_buf->entry,&fctx->src_done_list);
		return -2;
	}
	if(got_frame){
		f = h264_dequeue_frame(h);
		if(f){
			felix_buffer_t *out = container_of(f, felix_buffer_t, frame);
			list_add_tail(&out->entry, &fctx->dst_done_list);
		}
	} else {
		printf("got frame failed !!!!!!\r\n");
		retval = -2;
	}
	list_add_tail(&src_buf->entry, &fctx->src_done_list);
	return retval;
}

int jz_felix_get_frame(felix_prv_ctx_t* handle, Jz_Felix_BufferInfo_t *buf)
{
	felix_prv_ctx_t *fctx = (felix_prv_ctx_t*)handle;
	felix_buffer_t *dst_buf;
	dst_buf = list_first_entry_or_null(&fctx->dst_done_list, felix_buffer_t, entry);
	if(!dst_buf){
		printf("no dst done buf\n");
		return -1;
	}

	buf->vaddr = dst_buf->vaddr;
	buf->paddr = dst_buf->paddr;
	buf->size = dst_buf->size;
	buf->index = dst_buf->index;

	list_del(&dst_buf->entry);

	return 0;
}

int jz_felix_put_frame(felix_prv_ctx_t *handle, Jz_Felix_BufferInfo_t *buf)
{
	felix_prv_ctx_t *fctx = (felix_prv_ctx_t*)handle;
	felix_buffer_t *dst_buf = &fctx->dst_buf[buf->index];

	if(dst_buf){
		list_add_tail(&dst_buf->entry, &fctx->dst_queue_list);
	} else {
		return -1;
	}

}

