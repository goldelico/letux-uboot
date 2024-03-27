#ifndef __NAND_PARAM_H
#define __NAND_PARAM_H
int ato_nand_register_func(void);
int dosilicon_nand_register_func(void);
int fm_nand_register_func(void);
int foresee_nand_register_func(void);
int gd_nand_register_func(void);
int issi_nand_register_func(void);
int mxic_nand_register_func(void);
int tc_nand_register_func(void);
int toshiba_nand_register_func(void);
int winbond_nand_register_func(void);
int xcsp_nand_register_func(void);
int xtx_mid0b_nand_register_func(void);
int xtx_mid2c_nand_register_func(void);
int xtx_nand_register_func(void);
int yhy_nand_register_func(void);
int zetta_nand_register_func(void);
static void *nand_param[] = {
/*##################*/
(void *)ato_nand_register_func,
/*##################*/
/*##################*/
(void *)dosilicon_nand_register_func,
/*##################*/
/*##################*/
(void *)fm_nand_register_func,
/*##################*/
/*##################*/
(void *)foresee_nand_register_func,
/*##################*/
/*##################*/
(void *)gd_nand_register_func,
/*##################*/
/*##################*/
(void *)issi_nand_register_func,
/*##################*/
/*##################*/
(void *)mxic_nand_register_func,
/*##################*/
/*##################*/
(void *)tc_nand_register_func,
/*##################*/
/*##################*/
(void *)toshiba_nand_register_func,
/*##################*/
/*##################*/
(void *)winbond_nand_register_func,
/*##################*/
/*##################*/
(void *)xcsp_nand_register_func,
/*##################*/
/*##################*/
(void *)xtx_mid0b_nand_register_func,
/*##################*/
/*##################*/
(void *)xtx_mid2c_nand_register_func,
/*##################*/
/*##################*/
(void *)xtx_nand_register_func,
/*##################*/
/*##################*/
(void *)yhy_nand_register_func,
/*##################*/
/*##################*/
(void *)zetta_nand_register_func,
/*##################*/
};
#endif
