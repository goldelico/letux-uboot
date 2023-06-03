#ifndef SFC_COMMON_H
#define SFC_COMMON_H

#include <asm/arch/sfc.h>

void sfc_start(struct sfc *sfc);
void sfc_flush_fifo(struct sfc *sfc);
void sfc_dev_sta_exp(struct sfc *sfc, unsigned int value);
void sfc_dev_sta_msk(struct sfc *sfc, unsigned int value);
void sfc_clear_all_intc(struct sfc *sfc);
void sfc_enable_all_intc(struct sfc *sfc);
void sfc_set_data_length(struct sfc *sfc, int value);
unsigned int sfc_get_sta_rt(struct sfc *sfc);

void dump_sfc_reg(struct sfc *sfc);
void dump_cdt(struct sfc *sfc);
void write_cdt(struct sfc *sfc, struct sfc_cdt *cdt, uint16_t start_index, uint16_t end_index);

int sfc_sync_cdt(struct sfc *sfc, struct sfc_cdt_xfer *xfer);
struct sfc *sfc_res_init(uint32_t sfc_rate);
struct sfc *sfc_clk_init(uint32_t sfc_rate);

int set_flash_timing(struct sfc *sfc, unsigned int t_hold, unsigned int t_setup, unsigned int t_shslrd, unsigned int t_shslwr);

int sfc_nor_get_special_ops(struct sfc_flash *flash);

#endif
