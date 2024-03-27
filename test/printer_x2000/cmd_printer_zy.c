#include <common.h>
#include <command.h>
#include <config.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/jz_uart.h>
#include <asm/arch/base.h>
#include <asm/arch/cpm.h>
#include "Ingenic_logo_384x128_1bit.h"
#include <asm/arch-x2000/tcu.h>
#include <asm/arch-x2000/spi.h>

//moto gpio
#define GPIO_MOTO_STEP            GPIO_PC(24)
#define GPIO_MOTO_RESET            GPIO_PA(15)
#define GPIO_MOTO_MODE0            GPIO_PA(22)
#define GPIO_MOTO_MODE1            GPIO_PA(21)
#define GPIO_MOTO_MODE2           GPIO_PA(20)
#define GPIO_MOTO_FAULT            GPIO_PA(12)
#define GPIO_MOTO_SLEEP            GPIO_PA(13)
#define GPIO_MOTO_DECAY            GPIO_PA(25)
#define GPIO_MOTO_EN            GPIO_PA(23)
#define GPIO_MOTO_DIR            GPIO_PA(24)

//printer head enable
#define GPIO_PRINTER_HEAD_EN   GPIO_PD(1)

//printer head gpio
#define GPIO_BIT_NO_DI2             GPIO_PD(2)
#define GPIO_BIT_NO_CLK             GPIO_PD(0)
#define GPIO_BIT_NO_LAT             GPIO_PB(2)
#define GPIO_BIT_NO_STB1             GPIO_PB(1)
#define GPIO_BIT_NO_STB2             GPIO_PB(5)
#define GPIO_BIT_NO_STB3             GPIO_PB(3)
#define GPIO_POWER_EN           GPIO_PB(21)

//adc
#define GPIO_ADC_SCK            GPIO_PC(26)
#define GPIO_ADC_SDA            GPIO_PC(27)

//paper detect
#define GPIO_CHECK_PAPER_SEND_EN   GPIO_PA(11)
#define GPIO_P_BACKFEED   GPIO_PA(10)

//cover open gpio
#define GPIO_COVER_OPEN   GPIO_PA(9)

//led light gpio
#define GPIO_ERROR   GPIO_PB(22)
#define GPIO_PAPER   GPIO_PB(24)

//bell
#define GPIO_BELL_DRIVER   GPIO_PC(25)

#define PRINT_PWM_CLK 5000000 //pwm clock 2ms
#define PRINT_PWM_FULL 256
#define PRINT_PWM_CHN 4

//moto step
#define DRV8825_MICROSTEP_FULL 1
#define DRV8825_MICROSTEP_HALF 2
#define DRV8825_MICROSTEP_QUARTER 4
#define DRV8825_MICROSTEP_EIGHTH 8
#define DRV8825_MICROSTEP_SIXTEENTH 16
#define DRV8825_MICROSTEP_THIRTYSECOND 32

//moto direction
#define DRV8825_DIR_FORWARD 1
#define DRV8825_DIR_BACKWARD 2

//moto decay
#define DRV8825_DECAY_MODE_SLOW  0
#define DRV8825_DECAY_MODE_FAST  1
#define DRV8825_DECAY_MODE_MIXED 2

#define SSI_BASE SSI0_BASE
static void inline reg_bit_set(unsigned int reg_addr, int bit)
{
        unsigned long val;
        val = readl(reg_addr);
        val &= ~bit;
        writel(val, reg_addr);
}
static uint32_t jz_spi_readl(unsigned int offset)
{
	return readl(SSI_BASE + offset);
}

static void jz_spi_writel(unsigned int value, unsigned int offset)
{
	writel(value, SSI_BASE + offset);
}

static void spi_send_cmd(unsigned char *cmd, unsigned int count)
{
	unsigned int sum = count;
	while(count) {
		jz_spi_writel(*cmd, SSI_DR);
		cmd++;
		count--;
	}
	jz_spi_writel(SSI_CR0_SSIE | jz_spi_readl(SSI_CR0), SSI_CR0);
}

static void spi_init(void)
{
	printf("spi_init\n");

	reg_bit_set(CPM_BASE + CPM_CLKGR0, (1 << 19));
	jz_spi_writel(~SSI_CR0_SSIE & jz_spi_readl(SSI_CR0), SSI_CR0);
	jz_spi_writel(0, SSI_GR);
	jz_spi_writel(SSI_CR0_EACLRUN | SSI_CR0_RFLUSH | SSI_CR0_TFLUSH, SSI_CR0);
	jz_spi_writel(SSI_FRMHL_CE0_LOW_CE1_LOW | SSI_GPCMD | SSI_GPCHL_HIGH |  SSI_CR1_FLEN_8BIT | (1 << 28), SSI_CR1);
}

static void gpio_init_spi(void)
{
	gpio_set_func(GPIO_PORT_D, GPIO_FUNC_0, 1 << (GPIO_BIT_NO_DI2 % 32));
	gpio_set_func(GPIO_PORT_D, GPIO_FUNC_0, 1 << (GPIO_BIT_NO_CLK % 32));
	spi_init();
}

void printer_start_pwm()
{
	int _val = 0;
	int _half;
	int _period;
	int prescaler = 1;

	_val = 128;
	_period = PRINT_PWM_CLK;
	_period = (unsigned long long)CONFIG_SYS_EXTAL * _period / 1000000000;
	while (_period > 0xffff && prescaler < 6) {
		_period >>= 2;
		++prescaler;
	}
	_half =_period * _val / PRINT_PWM_FULL;

	gpio_set_func(GPIO_PORT_C, GPIO_FUNC_0,1 << (GPIO_MOTO_STEP % 32));
	struct pwm pwm_printer = {PRINT_PWM_CHN,prescaler,EXTAL,_period,_half};
	pwm_init(&pwm_printer);

}

void printer_pwm_close(void)
{
	//pwm_disable(PRINT_PWM_CHN);
        gpio_direction_output(GPIO_MOTO_STEP,0);
}

void set_moto_decay_mode(int mode)
{
	switch (mode)
	{
	case DRV8825_DECAY_MODE_SLOW:
		gpio_direction_output(GPIO_MOTO_DECAY,0);
		break;

	case DRV8825_DECAY_MODE_FAST:
		gpio_direction_output(GPIO_MOTO_DECAY,1);
		break;

	case DRV8825_DECAY_MODE_MIXED:
		break;

	}
}

void set_moto_dir(int dir)
{
	switch (dir)
	{
	case DRV8825_DIR_FORWARD:
		gpio_direction_output(GPIO_MOTO_DIR,0);
		break;

	case DRV8825_DIR_BACKWARD:
		gpio_direction_output(GPIO_MOTO_DIR,1);
		break;

	}
}

void set_moto_step(int step)
{
	switch (step)
	{
	case DRV8825_MICROSTEP_FULL:
		gpio_direction_output(GPIO_MOTO_MODE0,0);
		gpio_direction_output(GPIO_MOTO_MODE1,0);
		gpio_direction_output(GPIO_MOTO_MODE2,0);
		break;

	case DRV8825_MICROSTEP_HALF:
		gpio_direction_output(GPIO_MOTO_MODE0,0);
		gpio_direction_output(GPIO_MOTO_MODE1,0);
		gpio_direction_output(GPIO_MOTO_MODE2,1);
		break;

	case DRV8825_MICROSTEP_QUARTER:
		gpio_direction_output(GPIO_MOTO_MODE0,0);
		gpio_direction_output(GPIO_MOTO_MODE1,1);
		gpio_direction_output(GPIO_MOTO_MODE2,0);
		break;

	case DRV8825_MICROSTEP_EIGHTH:
		gpio_direction_output(GPIO_MOTO_MODE0,0);
		gpio_direction_output(GPIO_MOTO_MODE1,1);
		gpio_direction_output(GPIO_MOTO_MODE2,1);
		break;

	case DRV8825_MICROSTEP_SIXTEENTH:
		gpio_direction_output(GPIO_MOTO_MODE0,1);
		gpio_direction_output(GPIO_MOTO_MODE1,0);
		gpio_direction_output(GPIO_MOTO_MODE2,0);
		break;

	case DRV8825_MICROSTEP_THIRTYSECOND:
		gpio_direction_output(GPIO_MOTO_MODE0,1);
		gpio_direction_output(GPIO_MOTO_MODE1,0);
		gpio_direction_output(GPIO_MOTO_MODE2,1);
		break;
	}
}

void moto_init()
{
	printf("zy printer moto init !!\n");
	gpio_direction_output(GPIO_MOTO_EN,1);
	gpio_direction_output(GPIO_MOTO_SLEEP,0);
	gpio_direction_output(GPIO_MOTO_RESET,0);

        gpio_direction_output(GPIO_BIT_NO_LAT, 1);
      	gpio_direction_output(GPIO_BIT_NO_STB1, 0);
      	gpio_direction_output(GPIO_BIT_NO_STB2, 0);
      	gpio_direction_output(GPIO_BIT_NO_STB3, 0);
}

void moto_start()
{
	printf("zy printer moto start !!\n");
	moto_init();

	gpio_direction_output(GPIO_MOTO_RESET,1);
	udelay(1);
	gpio_direction_output(GPIO_MOTO_SLEEP,1);
	udelay(1);
	gpio_direction_output(GPIO_MOTO_EN,0);
	udelay(1);

	set_moto_dir(DRV8825_DIR_FORWARD);
	//set_moto_dir(DRV8825_DIR_BACKWARD);
	udelay(1);

	set_moto_step(DRV8825_MICROSTEP_FULL);
	//set_moto_step(DRV8825_MICROSTEP_THIRTYSECOND);
	udelay(1);

}


void moto_stop()
{
	printf("zy printer moto stop !!\n");
	printer_pwm_close();
	gpio_direction_output(GPIO_MOTO_EN,1);
	gpio_direction_output(GPIO_MOTO_RESET,0);
	gpio_direction_output(GPIO_MOTO_SLEEP,0);

}

void paper_detect_power_on()
{
	printf("zy printer paper detect power on !!\n");
	gpio_direction_output(GPIO_CHECK_PAPER_SEND_EN,0);

	gpio_direction_input(GPIO_P_BACKFEED);
	gpio_direction_input(GPIO_COVER_OPEN);
}


int get_cover_status()
{
	return gpio_get_value(GPIO_COVER_OPEN);
}

int get_paper_status()
{
	return gpio_get_value(GPIO_P_BACKFEED);
}

static void start_heat_paper()
{
	int heat_time = 0;
	heat_time = 500;

        gpio_direction_output(GPIO_BIT_NO_LAT, 0);
        udelay(10);
	gpio_direction_output(GPIO_BIT_NO_LAT, 1);
	udelay(1);

	gpio_direction_output(GPIO_BIT_NO_STB1, 1);
	udelay(heat_time);
	gpio_direction_output(GPIO_BIT_NO_STB1, 0);

	gpio_direction_output(GPIO_BIT_NO_STB2, 1);
	udelay(heat_time);
	gpio_direction_output(GPIO_BIT_NO_STB2, 0);

	gpio_direction_output(GPIO_BIT_NO_STB3, 1);
	udelay(heat_time);
	gpio_direction_output(GPIO_BIT_NO_STB3, 0);

	udelay(8);
}

static void start_heat_paper_one_line()
{
	int heat_time = 0;
	heat_time = 450;

        gpio_direction_output(GPIO_BIT_NO_LAT, 0);
        udelay(1);
	gpio_direction_output(GPIO_BIT_NO_LAT, 1);
	udelay(1);

	gpio_direction_output(GPIO_BIT_NO_STB1, 1);
	udelay(heat_time);
	gpio_direction_output(GPIO_BIT_NO_STB1, 0);

	gpio_direction_output(GPIO_BIT_NO_STB2, 1);
	udelay(heat_time);
	gpio_direction_output(GPIO_BIT_NO_STB2, 0);

	gpio_direction_output(GPIO_BIT_NO_STB3, 1);
	udelay(heat_time);
	gpio_direction_output(GPIO_BIT_NO_STB3, 0);

	udelay(8);
}

unsigned char printer_logo_bitmap3[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        };

unsigned char printer_logo_bitmap2[] = {
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
        };

static pos = 0;
static void set_spi_data(void)
{
	int size = 832;
        int count, i, j;
	int flag = 0;
	char data;

        gpio_direction_output(GPIO_BIT_NO_LAT, 1);
      	gpio_direction_output(GPIO_BIT_NO_STB1, 0);
      	gpio_direction_output(GPIO_BIT_NO_STB2, 0);
      	gpio_direction_output(GPIO_BIT_NO_STB3, 0);

	memset(printer_logo_bitmap2,0,48);
	memcpy(printer_logo_bitmap2,printer_logo_bitmap+pos,48);

	for(i=0;i<48;i++)
	{
				data= printer_logo_bitmap2[i];
				for(j=0;j<8;j++)
				{
					if((data>>(7-j))&1)
					{
						gpio_direction_output(GPIO_BIT_NO_DI2, 1);
					}else
					{
						gpio_direction_output(GPIO_BIT_NO_DI2, 0);
					}

					gpio_direction_output(GPIO_BIT_NO_CLK, 1);
					udelay(1);
					gpio_direction_output(GPIO_BIT_NO_CLK, 0);
					udelay(1);
				}
	}

	pos = pos + 48;

        for (count = 385; count < size; count++) {
		if(flag)
		{
			flag = 0;
			gpio_direction_output(GPIO_BIT_NO_DI2, 1);
		}
		else
		{
			flag = 1;
			gpio_direction_output(GPIO_BIT_NO_DI2, 0);
		}

		gpio_direction_output(GPIO_BIT_NO_CLK, 1);
	 	udelay(1);
		gpio_direction_output(GPIO_BIT_NO_CLK, 0);
		udelay(1);
        }

}

int open_printer_power()
{
	gpio_direction_output(GPIO_POWER_EN,1);
	gpio_direction_output(GPIO_PRINTER_HEAD_EN,0);
	udelay(2000);
}


unsigned short moto_table[] = {
	3500,3150,2900,2700,
	2500,2360,2220,2100,
	1990,1890,1800,1720,
	1645,1575,1510,1450,
	1395,1345,1300,1260,
	1225,1195,1170,1150,
	1132,1116,1102,1090,
	1078,1068,1058,1050,
	1042,1036,1030,1025,
	1020,1016,1012,1009,
	1006,1004,1002,1001,
	1000};

void printer_start_pwm_gpio()
{
	int count;
	unsigned short step_time = 1000;
	int size = 200;

	open_printer_power();

	gpio_direction_output(GPIO_MOTO_STEP,0);
	for(count = 0; count < size; count++)
	{
		if(count < 44)
			step_time = moto_table[count]/2;

		set_spi_data();
		start_heat_paper();
		gpio_direction_output(GPIO_MOTO_STEP,1);
		udelay(step_time);
		gpio_direction_output(GPIO_MOTO_STEP,0);
		udelay(step_time);

	}

	gpio_direction_output(GPIO_POWER_EN,0);
}


void printer_start_test_one_line()
{
	open_printer_power();
	set_spi_data();
	start_heat_paper_one_line();
	gpio_direction_output(GPIO_POWER_EN,0);
}

void zy_printer (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int status=0;
	printf("zy printer init\n");
	pos = 0;
	gpio_init_spi();
	printer_pwm_close();

//	open_printer_power();

#if 0
	printer_start_test_one_line();
#else
	moto_start();
	printer_start_pwm_gpio();
	//printer_start_pwm();
#endif
	while(1){
		mdelay(2000);
		break;
	}
#if 0
	paper_detect_power_on();

	status = get_cover_status();
	if(status)
		printf("cover is open,close it!\n");
	else
		printf("cover is close, ok!!\n");

	status = get_paper_status();
	if(status)
		printf("no paper!\n");
	else
		printf("paper ready!!\n");
#endif
	printf("finish...\n");
	moto_stop();
}

U_BOOT_CMD(zy_printer,1,2,zy_printer,
		           "zy printer command","jim gao add zy printer command!\n");
