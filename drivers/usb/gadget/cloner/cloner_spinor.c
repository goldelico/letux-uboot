#ifdef CONFIG_JZ_SPI
#include <spi.h>
#include <spi_flash.h>
#include <cloner/cloner.h>
#include "cloner_moudle.h"
#include "cloner_log.h"


static struct spi_flash *flash = NULL;

int spi_erase()
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;

	spi_init();

	if(flash == NULL){
		flash = spi_flash_probe(bus, cs, ssi_rate, mode);
		if (!flash) {
			printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
			return 1;
		}
	}
	jz_erase_all(flash);
	printf("spi erase ok\n");
	return 0;
}

int spinor_read(struct cloner *cloner)
{
	int ret = 0;
	u32 addr = cloner->cmd->read.offset;
	u32 len = cloner->read_req->length;
	void *buf = (void *)cloner->read_req->buf;
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;

	spi_init();

	if(flash == NULL){
		flash = spi_flash_probe(bus, cs, ssi_rate, mode);
		if (!flash) {
			printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
			return 1;
		}
	}

	ret = spi_flash_read(flash, addr, len, buf);
	if(ret < 0)
		printf("%s error\n",__func__);

	return ret;
}


int spi_program(struct cloner *cloner)
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	u32 offset = cloner->cmd->write.partition + cloner->cmd->write.offset;
	u32 length = cloner->cmd->write.length;
	int blk_size = spi_args->spi_erase_block_size;
	void *addr = (void *)cloner->write_req->buf;
	unsigned int ret;
	int len = 0;

	spi_init();

#ifdef CONFIG_INGENIC_SOFT_SPI
	spi_init_jz(&spi);
#endif

	if(flash == NULL){
		flash = spi_flash_probe(bus, cs, ssi_rate, mode);
		if (!flash) {
			printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
			return 1;
		}
	}

	BURNNER_PRI("the offset = %x\n",offset);
	BURNNER_PRI("the length = %x\n",length);


	if (length < blk_size || length%blk_size == 0){
		len = length;
		BURNNER_PRI("the length = %x,blk_size = %x\n",length,blk_size);
	}
	else{
		BURNNER_PRI("the length = %x, is no enough %x\n",length,blk_size);
		len = (length/blk_size)*blk_size + blk_size;
	}

	if (!spi_args->spi_erase) {
		ret = spi_flash_erase(flash, offset, len);
		BURNNER_PRI("SF: %zu bytes @ %#x Erased: %s\n", (size_t)len, (u32)offset,
				ret ? "ERROR" : "OK");
	}

	ret = spi_flash_write(flash, offset, len, addr);
	BURNNER_PRI("SF: %zu bytes @ %#x write: %s\n", (size_t)len, (u32)offset,
			ret ? "ERROR" : "OK");


	if (debug_args->write_back_chk) {
		spi_flash_read(flash, offset,len, addr);

		uint32_t tmp_crc = local_crc32(0xffffffff,addr,cloner->cmd->write.length);
		BURNNER_PRI("SF: %d bytes @ %#x check: %s\n",len,offset,(cloner->cmd->write.crc == tmp_crc) ? "OK" : "ERROR");
		if (cloner->cmd->write.crc != tmp_crc) {
			printf("src_crc32 = %08x , dst_crc32 = %08x\n",cloner->cmd->write.crc,tmp_crc);
			return -EIO;
		}
	}

	return 0;
}
#endif
