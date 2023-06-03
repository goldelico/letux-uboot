#include <cloner/cloner.h>
#include "burn_printf.h"

static char *readbuf = NULL;
extern struct ParameterInfo	*global_args;
struct spi_param *spi_args;
struct ddr_param *ddr_args;

#ifdef CONFIG_MTD_SFCNOR
#include "cloner_sfcnor.c"
#endif
#ifdef CONFIG_MTD_SPINAND
extern unsigned int ssi_rate;
#include "cloner_spinand.c"
#endif
#ifdef CONFIG_MTD_SFCNAND
#include "cloner_sfcnand.c"
#endif
#ifdef CONFIG_JZ_SPI
extern unsigned int ssi_rate;
#include "cloner_spinor.c"
#endif


int clmd_spisfc_info(struct cloner *cloner)
{
	int id_code = 0;

	if(global_args->magic == MAGIC_POLICY){
		policy_args = global_args->data;
	} else {
		printf("ERR: cloner send policy data error!\n");
		memset(global_args->data, 0, sizeof(*policy_args));
	}
#ifdef CONFIG_MTD_SFCNOR
	if(policy_args->use_sfc_nor){
		id_code = get_norflash_id();
	}
#endif
	if(id_code < 0) {
		printf("ERR : (get flash_info) try id err, %d\n", id_code);
		id_code = 0;
	}

	memcpy(cloner->ep0req->buf, &id_code, sizeof(unsigned int));
	return id_code;
}

int clmd_spisfc_init(struct cloner *cloner, void *args, void *ops_data)
{
	spi_args = (struct spi_param *)args;
	if(!spi_args)
	{
		printf("Not found sfc parameters (%s)\n",__func__);
		return -EINVAL;
	}
	int ret = 0;

	if(!policy_args)
	{
		printf("Not fount policy parameters (%s)\n",__func__);
		return -EINVAL;
	}

#ifdef CONFIG_MTD_SFCNOR
	if(policy_args->use_sfc_nor){
		ret = norflash_get_params_from_burner(spi_args->sfc_frequency,(unsigned char *)spi_args + sizeof(struct spi_param));
		if (spi_args->spi_erase == SPI_ERASE_PART) {
			sfc_erase();
		}
	}

#endif
#ifdef CONFIG_MTD_SFCNAND
	if(policy_args->use_sfc_nand){
		ret = mtd_sfcnand_probe_burner(&(spi_args->spi_erase),spi_args->sfc_quad_mode,spi_args->sfc_frequency,
				debug_args->write_back_chk, spi_args->flash_info);
		if (!ret)
			get_burner_nandinfo(spi_args->flash_info);
	}
#endif
#ifdef CONFIG_MTD_SPINAND
	ssi_rate = CONFIG_SPI_RATE;
	if(policy_args->use_spi_nand){
		get_burner_nandinfo(spi_args->flash_info, &nand_param_from_burner);
		mtd_spinand_probe_burner(&(spi_args->spi_erase),&nand_param_from_burner);
	}
#endif
#ifdef CONFIG_JZ_SPI
	ssi_rate = CONFIG_SPI_RATE;
	if(policy_args->use_spi_nor){
		if (spi_args->spi_erase == SPI_ERASE_PART) {
			spi_erase(cloner);
		}
	}
#endif
	return ret;
}

int clmd_spisfc_write(struct cloner *cloner, int sub_type, void *ops_data)
{
	int ret = 0;
	switch(sub_type)
	{
#ifdef CONFIG_JZ_SPI
		case SPI_NOR:
			ret = spi_program(cloner);
			break;
#endif
#ifdef CONFIG_MTD_SFCNOR
		case SFC_NOR:
			ret = sfc_program(cloner);
			break;
#endif
#if defined(CONFIG_MTD_SPINAND) || defined(CONFIG_MTD_SFCNAND)
		case SPI_NAND:
		case SFC_NAND:
			ret = spinand_program(cloner);
			break;
#endif

#ifdef CONFIG_JZ_SPINAND_SN
		case SFC_NAND_SN_WRITE:
			ret = spinand_sn_program(cloner);
			break;
#endif

#ifdef CONFIG_JZ_SPINAND_MAC
		case SFC_NAND_MAC_WRITE:
			ret = spinand_mac_program(cloner);
			break;
#endif

#ifdef CONFIG_JZ_SPINAND_LICENSE
		case SFC_NAND_LICENSE_WRITE:
			ret = spinand_license_program(cloner);
			break;
#endif
		default:
			printf("Not found sfc sub_type!\n");
			return -EINVAL;
	}
	return ret;
}

static int32_t clmd_spisfc_read(struct cloner *cloner, int sub_type, void *ops_data) {

	int32_t ret = 0;

	switch(sub_type)
	{
#ifdef CONFIG_JZ_SPI
		case SPI_NOR:
			ret = spinor_read(cloner);
			break;
#endif
#ifdef CONFIG_MTD_SFCNOR
		case SFC_NOR:
			ret = sfcnor_read(cloner);
			break;
#endif
#if defined(CONFIG_MTD_SPINAND) || defined(CONFIG_MTD_SFCNAND)
		case SPI_NAND:
		case SFC_NAND:
			ret = spinand_read(cloner);
			break;
#endif
#ifdef CONFIG_JZ_SPINAND_SN
		case SFC_NAND_SN_READ:
			ret = spinand_sn_read(cloner);
			break;
#endif

#ifdef CONFIG_JZ_SPINAND_MAC
		case SFC_NAND_MAC_READ:
			ret = spinand_mac_read(cloner);
			break;
#endif

#ifdef CONFIG_JZ_SPINAND_LICENSE
		case SFC_NAND_LICENSE_READ:
			ret = spinand_license_read(cloner);
			break;
#endif
		default:
			printf("Not found sfc sub_type!\n");
			return -EINVAL;
	}

	return ret;
}

static int32_t clmd_spisfc_reset(struct cloner *cloner) {

	int32_t ret = 0;

	if(!policy_args)
	{
		printf("Not fount policy parameters (%s)\n",__func__);
		return -EINVAL;
	}

#ifdef CONFIG_MTD_SFCNOR
	if(policy_args->use_sfc_nor){
		ret = sfc_reset();
	}
#endif

	return ret;
}

int cloner_spisfc_init(void)
{
	struct cloner_moudle *clmd = malloc(sizeof(struct cloner_moudle));
	int ret;

	if (!clmd)
		return -ENOMEM;
	clmd->medium = MAGIC_SFC;
	clmd->ops = SPISFC;
	clmd->write = clmd_spisfc_write;
	clmd->init = clmd_spisfc_init;
	clmd->info = clmd_spisfc_info;
	clmd->read = clmd_spisfc_read;
	clmd->check = NULL;
	clmd->reset = clmd_spisfc_reset;
	clmd->data = NULL;
	printf("cloner spisfc register\n");
	return register_cloner_moudle(clmd);
}
CLONER_MOUDLE_INIT(cloner_spisfc_init);
