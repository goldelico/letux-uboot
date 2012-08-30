#ifndef _TD028TTEC1_H
#define _TD028TTEC1_H

#define JBT_TX_BUF_SIZE
struct jbt_info {
	u_int16_t tx_buf[4];
	struct spi_device *spi_dev;
	int state;
};

#define JBT_COMMAND	0x000
#define JBT_DATA	0x100

int jbt_reg_init(void);
int jbt_reg_write_nodata(struct jbt_info *jbt, u_int8_t reg);
int jbt_reg_write(struct jbt_info *jbt, u_int8_t reg, u_int8_t data);
int jbt_reg_write16(struct jbt_info *jbt, u_int8_t reg, u_int16_t data);
int jbt_check(void);

#endif
