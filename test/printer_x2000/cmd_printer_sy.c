#include <common.h>
#include <command.h>
#include <config.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/jz_uart.h>
#include <asm/arch-x2000_v12/base.h>
#include <asm/arch-x2000_v12/cpm.h>
#include "Ingenic_logo_384x128_1bit.h"
#include "adc.h"

#define GPIO_BIT_NO_MTA0            GPIO_PB(20)
#define GPIO_BIT_NO_MTA1            GPIO_PB(21)
#define GPIO_BIT_NO_MTB0            GPIO_PB(22)
#define GPIO_BIT_NO_MTB1            GPIO_PB(23)

//#define GPIO_BIT_NO_FAULT           GPIO_PA(22)
//#define GPIO_BIT_NO_SLEEP           GPIO_PA(23)

#define GPIO_BIT_NO_LAT             GPIO_PB(27)
#define GPIO_BIT_NO_DATAOUT             GPIO_PB(30)
#define GPIO_BIT_NO_CLK             GPIO_PB(31)

//#define GPIO_BIT_NO_PAPER_DETECT    GPIO_PB(3)

#define GPIO_BIT_NO_STB1             GPIO_PB(25)
#define GPIO_BIT_NO_STB2             GPIO_PB(26)
#define GPIO_BIT_NO_POWER            GPIO_PB(2)
//#define GPIO_BIT_ADC_PDA            GPIO_PB(0)
//#define GPIO_BIT_ADC_PCL            GPIO_PB(1)
//#define GPIO_BIT_UART1_RD            GPIO_PD(2)

/* paper test */
#define GPIO_BIT_NO_EN_PE             GPIO_PB(24)

/* cover test */
#define GPIO_BIT_NO_COVER_OPEN        GPIO_PB(14)

/* power key test */
#define GPIO_BIT_NO_TS_KEY        GPIO_PB(15)

/* LED1_R, PB07~PB12 */
#define GPIO_BIT_NO_LED1_R        GPIO_PB(7)

#define MAX_DOT_NUM_PER_LINE         (576) /* 576 dot one line */
#define BYTES_NUM_PER_LINE         (576/8) /* 576 dot one line */


#define CONFIG_DATA_UART_INDEX 1
#define CONFIG_DATA_UART_BAUD 9600

#define TS_TM 0
#define TS_VHD 3
static struct jz_uart *uart1;
static void inline reg_bit_set(unsigned int reg_addr, int bit)
{
	unsigned long val;
	val = readl(reg_addr);
	val &= ~bit;
	writel(val, reg_addr);
}

static void jz_serial_setbrg(void)
{
	printf("jz_serial_setbrg!!!\n");
	u32 baud_div, tmp;
	baud_div = CONFIG_SYS_EXTAL / 16 / CONFIG_DATA_UART_BAUD;

	tmp = readb(&uart1->lcr);
	tmp |= UART_LCR_DLAB;
	writeb(tmp, &uart1->lcr);

	writeb((baud_div >> 8) & 0xff, &uart1->dlhr_ier);
	writeb(baud_div & 0xff, &uart1->rbr_thr_dllr);

	tmp &= ~UART_LCR_DLAB;
	writeb(tmp, &uart1->lcr);
}

static int jz_serial_init(void)
{
	uart1 = (struct jz_uart *)(UART0_BASE + CONFIG_DATA_UART_INDEX * 0x1000);
	/* Disable port interrupts while changing hardware */
	writeb(0, &uart1->dlhr_ier);
	/* Disable UART unit function */
	writeb(~UART_FCR_UUE, &uart1->iir_fcr);

	/* Set both receiver and transmitter in UART mode (not SIR) */
	writeb(~(SIRCR_RSIRE), &uart1->isr);
	/*
	 * Set databits, stopbits and parity.
	 * (8-bit data, 1 stopbit, no parity)
	 */
	writeb(UART_LCR_WLEN_8 | UART_LCR_STOP_1, &uart1->lcr);
	/* Set baud rate */
	jz_serial_setbrg();
	/* Enable UART unit, enable and clear FIFO */
	writeb(UART_FCR_UUE | UART_FCR_FE | UART_FCR_TFLS | UART_FCR_RFLS,
	       &uart1->iir_fcr);
	return 0;
}

static int jz_serial_tstc(void)
{
	if (readb(&uart1->lsr) & UART_LSR_DR)
		return 1;
	return 0;
}

static int jz_serial_getc(void)
{
	while (!jz_serial_tstc())
		;
	return readb(&uart1->rbr_thr_dllr);
}

static void gpio_init_uart(void)
{
	reg_bit_set(CPM_BASE + CPM_CLKGR0, CPM_CLKGR_UART1);
	//gpio_set_func(GPIO_PORT_D, GPIO_FUNC_1,1 << (GPIO_BIT_UART1_RD % 32));
	//gpio_direction_input(GPIO_BIT_UART1_RD);
	jz_serial_init();
}

//heat time  
static int Tim_Index = 0;
//step time is us, heat time is us
unsigned short const StepM_Time[26][2] = {                     
 {4875,497},{3013,382},{2327,343},{1953,317}, {1712,297},{1541,281},{1412,268},{1310,257},
 {1227,247},{1158,238},{1099,230},{1048,223}, {1004,217},{964, 211},{929, 205},{898, 200},
 {869, 195},{843, 191},{819, 186},{797, 182}, {777, 178},{758, 175},{741, 171},{725,168},
 {709, 165},{694, 161} };        

static int get_head_voltage(void)
{
	int aa[20];
	int count =20;
	int vol;
	int i=0; 

	while(count > 0)
	{
		count--;
		aa[i]= jz_serial_getc();
		i++;	
//		printf("aa is %x\n",aa[i]);
	}

	for(i=0;i<20;i++)
	{
		if(aa[i]==0xf0)
		{
			vol= aa[i+2]<<8 | aa[i+3];
			break;
		}
	}
	return vol;

}

//得到加热时间, 30 Dot　行左右测温一次 ////////////////////////////////////
unsigned int update_pt_time(void)                                  
{
  unsigned int temp32;
  unsigned short temp16 ;
    
    //temp16 = get_head_voltage();
	temp16=get_adc_channel_voltage(TS_TM);
//   printf("themnal head voltage is %d\n",temp16);         

  if(temp16 > 2374){printf("head temprture is -20\n");temp32 = 1125;  }      
//<-10,=-15
  else if(temp16 > 2200){printf("head temprture is -10\n");temp32 = 1005; }      
// < 0,=-5
  else if(temp16 > 1820){printf("head temprture is 0\n");temp32 = 885; }                             
// 5
  else if(temp16 > 1434){printf("head temprture is 5\n");temp32 = 765; }                             
// 15
  else if(temp16 > 1118){printf("head temprture is 15\n");temp32 = 645; }                             
// 25
  else if(temp16 > 825){printf("head temprture is 25\n");temp32 = 529; }                             
// 35
  else if(temp16 > 618){printf("head temprture is 35\n");temp32 = 455; }                             
// 45
  else if(temp16 > 440){printf("head temprture is 45\n");temp32 = 380; }                             
// 55
  else if(temp16 > 375){printf("head temprture is 65\n");temp32 = 306; }                             
// 65
  else                 {printf("head temprture is 70\n");temp32 = 0; }                               
// 70
  return temp32 ;
}

static float getV_int(int resistance)
{
 	int V=0;
 	V = (3300*resistance)/(39+resistance);
 	return V;
}

static void getV_fortest_int()
{
	int res[10] = {175,100,78,48,30,20,13,9,6,5} ;
	int result;
	int i =0;
	for (i= 0;i<10;i++)
	{
	   result=getV_int(res[i]);
	   printf("result for %d is %d\n",res[i],result);	
	}
}

static unsigned int get_heat_time(unsigned char NumofOne, unsigned int PT)  
{    
  unsigned short const TabQc[8] = {236,246,256,269,282,294,307,323} ;      
// Qc = Tab/256
                    // 0.92, 0.96, 1.00, 1.05, 1.10, 1.15, 1.20, 1.26
  unsigned int temp = NumofOne >> 4 ;
  
  if(temp > 7) temp = 7 ;
printf("--TabQc[temp] is %d\n",TabQc[temp]);
  return (PT*TabQc[temp]*(StepM_Time[Tim_Index][1]))>>16 ;           
} 


static void gpio_init_printer(void)
{
	printf("gpio_init_test ! \n");

	//gpio_direction_output(GPIO_BIT_ADC_PDA, 0);
	udelay(1);
	gpio_direction_output(GPIO_BIT_NO_CLK, 0);

	gpio_direction_output(GPIO_BIT_NO_POWER, 1);
	gpio_direction_output(GPIO_BIT_NO_STB1, 0);
	gpio_direction_output(GPIO_BIT_NO_STB2, 0);
	gpio_direction_output(GPIO_BIT_NO_LAT, 0);
	gpio_direction_output(GPIO_BIT_NO_DATAOUT, 0);
	//gpio_direction_output(GPIO_BIT_NO_PAPER_DETECT, 0);
	//gpio_direction_output(GPIO_BIT_ADC_PCL, 0);

	gpio_direction_output(GPIO_BIT_NO_MTA0, 0);
	gpio_direction_output(GPIO_BIT_NO_MTA1, 0);
	gpio_direction_output(GPIO_BIT_NO_MTB0, 0);
	gpio_direction_output(GPIO_BIT_NO_MTB1, 0);

	//gpio_direction_output(GPIO_BIT_NO_FAULT, 0);
	//gpio_direction_output(GPIO_BIT_NO_SLEEP, 1);
	//mdelay(3000);
}

static void gpio_init_stop(void)
{
	printf("gpio_init_stop ! \n");

	//gpio_direction_output(GPIO_BIT_ADC_PDA, 0);
	udelay(1);
	gpio_direction_output(GPIO_BIT_NO_CLK, 0);

	gpio_direction_output(GPIO_BIT_NO_POWER,0);
	gpio_direction_output(GPIO_BIT_NO_STB1, 0);
	gpio_direction_output(GPIO_BIT_NO_STB2, 0);
	gpio_direction_output(GPIO_BIT_NO_LAT, 0);
	gpio_direction_output(GPIO_BIT_NO_DATAOUT, 0);
	//gpio_direction_output(GPIO_BIT_NO_PAPER_DETECT, 0);
	//gpio_direction_output(GPIO_BIT_ADC_PCL, 0);

	gpio_direction_output(GPIO_BIT_NO_MTA0, 0);
	gpio_direction_output(GPIO_BIT_NO_MTA1, 0);
	gpio_direction_output(GPIO_BIT_NO_MTB0, 0);
	gpio_direction_output(GPIO_BIT_NO_MTB1, 0);

	//gpio_direction_output(GPIO_BIT_NO_FAULT, 0);
	//gpio_direction_output(GPIO_BIT_NO_SLEEP, 0);
	//mdelay(3000);
}

static jim_delay(int ms)
{
	mdelay(ms);
}


unsigned char printer_logo_bitmap1[] = {
	0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
	0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
	0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
	0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
	0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
	0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
	};

static times =0;
static int get_one_num(char * printer_data)
{
	int one_number = 0;
	char data;
	int i,j;
	for(i=0;i<48;i++)
	{
		data= printer_data[i];
		for(j=0;j<8;j++)
		{
			
			if((data>>(7-j))&1)
			{
				one_number++;
			}
			
		} 
	}
	return one_number;
}

static void heat_paper(int OneCnt)
{
	int heat_time = 0;
	int p_tempera = 0;
	
	p_tempera= 500 + update_pt_time();
//printf("--OneCnt is %d\n",OneCnt);
	heat_time = get_heat_time(OneCnt,p_tempera);
	//printf("--heat_time is %d\n",heat_time);

        gpio_direction_output(GPIO_BIT_NO_LAT, 0);
        udelay(1); /* 延时 */  
	gpio_direction_output(GPIO_BIT_NO_LAT, 1);
	udelay(1);
	//gpio_direction_output(GPIO_BIT_NO_STB1, 1);
	gpio_direction_output(GPIO_BIT_NO_STB1, 0);
	udelay(heat_time);
	//mdelay(8);	
	gpio_direction_output(GPIO_BIT_NO_STB1, 0);
	udelay(8);
}

static void heat_paper_line(int OneCnt)
{
	int heat_time = 0;
	int p_tempera = 0;
	
	//p_tempera= 500 + update_pt_time();
//printf("--OneCnt is %d\n",OneCnt);
	//heat_time = get_heat_time(OneCnt,p_tempera);
	//printf("--heat_time is %d\n",heat_time);

	heat_time = 300;	/* heat 500us for test */
	gpio_direction_output(GPIO_BIT_NO_STB1, 1);
	gpio_direction_output(GPIO_BIT_NO_STB2, 1);
	udelay(heat_time);
	//mdelay(8);	
	gpio_direction_output(GPIO_BIT_NO_STB1, 0);
	gpio_direction_output(GPIO_BIT_NO_STB2, 0);
	udelay(8);
}

static void set_printer_data(char data)
{
	int j;	
		for(j=0;j<8;j++)
		{
			if((data>>(7-j))&1)
			{
				gpio_direction_output(GPIO_BIT_NO_DATAOUT, 1);
			}else
			{
				gpio_direction_output(GPIO_BIT_NO_DATAOUT, 0);
			}
			gpio_direction_output(GPIO_BIT_NO_CLK, 1);
	 		udelay(1);
			gpio_direction_output(GPIO_BIT_NO_CLK, 0);
			udelay(1);
		}
}

static int get_pic_data(void)
{
	int i = 0;
	int j = 0;
	char data;
	int pos;
	int one_number = 0;
	pos = 48*times;
	memset(printer_logo_bitmap1,0,48);
	memcpy(printer_logo_bitmap1,printer_logo_bitmap+pos,48);
	if(times>128)
	{
	 //printf("end of the data!!!!\n");
		return;
	}
	one_number=get_one_num(printer_logo_bitmap1);
	if(one_number<64) //heat one time
	{
		for(i=0;i<48;i++)
		{
			data=printer_logo_bitmap1[i];
			set_printer_data(data);
		}
		heat_paper(one_number);
	}else if(one_number<128) //heat two times
	{

		for(i=0;i<48;i++)
		{
			if(i<24)
				data=printer_logo_bitmap1[i];
			else
				data = 0;
			set_printer_data(data);

		}
		heat_paper(one_number);

		for(i=0;i<48;i++)
		{
			if(i>=24)
				data=printer_logo_bitmap1[i];
			else
				data = 0;
			set_printer_data(data);

		}	
		heat_paper(one_number);
	}else  //heat three times
	{

		for(i=0;i<48;i++)
		{
			if(i<16)
				data=printer_logo_bitmap1[i];
			else
				data = 0;
			set_printer_data(data);

		}
		heat_paper(one_number);

		for(i=0;i<48;i++)
		{
			if(i>=16 && i<32)
				data=printer_logo_bitmap1[i];
			else
				data = 0;
			set_printer_data(data);

		}
		heat_paper(one_number);

		for(i=0;i<48;i++)
		{
			if(i>=32)
				data=printer_logo_bitmap1[i];
			else
				data = 0;
			set_printer_data(data);

		}
		heat_paper(one_number);
	}

	times++;
	return one_number;
}

static void get_pic_data1(void)
{
	int i = 0;
	int j = 0;
	char data;
	for(i=0;i<48;i++)
	{
		data=printer_logo_bitmap1[i];
		//printf("get pic data %x\n",data);
		for(j=0;j<8;j++)
		{
			if((data>>j)&1)
			{
				gpio_direction_output(GPIO_BIT_NO_DATAOUT, 1);
			//	printf("%d bit for data %d is 1\n",j,data);
			}else
			{
				gpio_direction_output(GPIO_BIT_NO_DATAOUT, 0);
			//	printf("%d bit for data %d is 0\n",j,data);
			}
			gpio_direction_output(GPIO_BIT_NO_CLK, 1);
	 		udelay(1);
			gpio_direction_output(GPIO_BIT_NO_CLK, 0);
			udelay(1);
		} 

	}
}

static void print_bmp_data(void)
{
	int one_num=0;
	//printer_logo_bitmap 
        gpio_direction_output(GPIO_BIT_NO_LAT, 1);  
      	gpio_direction_output(GPIO_BIT_NO_STB1, 0);
      	gpio_direction_output(GPIO_BIT_NO_STB2, 0);
	one_num = get_pic_data();
	//printf("current print line one number is %d\n",one_num);
}

static void set_spi_data_io(int l_no)
{
	int size = MAX_DOT_NUM_PER_LINE;
        int count, i;
	int heat;

        gpio_direction_output(GPIO_BIT_NO_LAT, 1);  
      	gpio_direction_output(GPIO_BIT_NO_DATAOUT, 0);

        for (count = 0; count < size; count++) {
		if((count/16)&1)
			heat = 1;
		else
			heat = 0;

		if((l_no/16)&1)
			heat = !heat;

		if(heat)
			gpio_direction_output(GPIO_BIT_NO_DATAOUT, 1);
		else
			gpio_direction_output(GPIO_BIT_NO_DATAOUT, 0);
  
		gpio_direction_output(GPIO_BIT_NO_CLK, 1);
	 	udelay(1);
		gpio_direction_output(GPIO_BIT_NO_CLK, 0);
		udelay(1);
        }
      
        gpio_direction_output(GPIO_BIT_NO_LAT, 0);
        udelay(1); /* 延时 */  
	gpio_direction_output(GPIO_BIT_NO_LAT, 1);
	udelay(1);

}

unsigned char printer_bar_bitmap0[] = {
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, /* 8bytes*1 */
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, /* 8bytes*2 */
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, /* 8bytes*3 */
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, /* 8bytes*4 */
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, /* 8bytes*5 */
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, /* 8bytes*6 */
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, /* 8bytes*7 */
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, /* 8bytes*8 */
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, /* 8bytes*9 */
};

unsigned char printer_bar_bitmap1[] = {
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, /* 8bytes*1 */
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, /* 8bytes*2 */
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, /* 8bytes*3 */
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, /* 8bytes*4 */
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, /* 8bytes*5 */
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, /* 8bytes*6 */
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, /* 8bytes*7 */
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, /* 8bytes*8 */
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, /* 8bytes*9 */
};

static void set_spi_data_hw(int l_no)
{
	if ((l_no/16)&1)
		spi_trans_data(printer_bar_bitmap0,BYTES_NUM_PER_LINE);
	else
		spi_trans_data(printer_bar_bitmap1,BYTES_NUM_PER_LINE);
}

static void set_spi_data(int l_no)
{
	//set_spi_data_io(l_no);
	set_spi_data_hw(l_no);
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


static void print_start(void)
{
	int delay = 3000;
	//int step = 150;
	int step = 500;
	int temp = 0;
	int count = 0;
	int sizeof_moto_table = sizeof(moto_table)/sizeof(moto_table[0]);
	printf("print_start_test ! \n");
	printf("sizeof_moto_table=%d\n", sizeof_moto_table);

	while(1 && step > 0)
	{
		/* temp=update_pt_time(); */
		/* if(temp<306) */
		/* { */
		/* 	printf("temprature is too high stop!\n"); */
		/* 	break; */
		/* } */

		step --;

		set_spi_data(step);		/* set data */
		heat_paper_line(step);
		gpio_direction_output(GPIO_BIT_NO_MTA0, 1);
		gpio_direction_output(GPIO_BIT_NO_MTB0, 1);
		gpio_direction_output(GPIO_BIT_NO_MTA1, 0);
		gpio_direction_output(GPIO_BIT_NO_MTB1, 0);
		if(count < sizeof_moto_table)
			udelay(moto_table[count++]);
		else
			udelay(moto_table[sizeof_moto_table - 1]);

		set_spi_data(step);		/* set data */
		heat_paper_line(step);
		gpio_direction_output(GPIO_BIT_NO_MTA0, 0);
		gpio_direction_output(GPIO_BIT_NO_MTB0, 1);
		gpio_direction_output(GPIO_BIT_NO_MTA1, 1);
		gpio_direction_output(GPIO_BIT_NO_MTB1, 0);
		//udelay(delay);
		if(count < sizeof_moto_table)
			udelay(moto_table[count++]);
		else
			udelay(moto_table[sizeof_moto_table - 1]);


		set_spi_data(step);		/* set data */
		heat_paper_line(step);
		gpio_direction_output(GPIO_BIT_NO_MTA0, 0);
		gpio_direction_output(GPIO_BIT_NO_MTB0, 0);
		gpio_direction_output(GPIO_BIT_NO_MTA1, 1);
		gpio_direction_output(GPIO_BIT_NO_MTB1, 1);
		if(count < sizeof_moto_table)
			udelay(moto_table[count++]);
		else
			udelay(moto_table[sizeof_moto_table - 1]);

		set_spi_data(step);		/* set data */
		heat_paper_line(step);
		gpio_direction_output(GPIO_BIT_NO_MTA0, 1);
		gpio_direction_output(GPIO_BIT_NO_MTB0, 0);
		gpio_direction_output(GPIO_BIT_NO_MTA1, 0);
		gpio_direction_output(GPIO_BIT_NO_MTB1, 1);
		if(count < sizeof_moto_table)
			udelay(moto_table[count++]);
		else
			udelay(moto_table[sizeof_moto_table - 1]);

	}

}

static int dump_adc_channels(int chs)
{
	int ch,val;
	printf("%s() get_adc_channel_voltage\n", __func__);
	for (ch=0; ch<6;ch++) {
		val=get_adc_channel_voltage(ch);
		printf("   ch%d=%d\n", ch, val);
	}
}


static int printer_led_io_test(int OneCnt)
{
	int gpio;
	gpio = GPIO_BIT_NO_LED1_R;
	for (gpio; gpio<(GPIO_BIT_NO_LED1_R+6); gpio++) {
		printf("%s() gpio: %d\n", __func__, gpio);
		gpio_direction_output(gpio, 1);
		mdelay(500);
		gpio_direction_output(gpio, 0);
		mdelay(500);
	}
}

static int printer_io_test(int OneCnt)
{
	int value;
	printf("\n%s()\n", __func__);

	/* test COVER_OPEN */
	gpio_direction_input(GPIO_BIT_NO_COVER_OPEN);
	gpio_direction_input(GPIO_BIT_NO_TS_KEY);
	mdelay(1000);
	value = gpio_get_value(GPIO_BIT_NO_COVER_OPEN);
	printf("GPIO_BIT_NO_COVER_OPEN=%d\n", value);
	value = gpio_get_value(GPIO_BIT_NO_TS_KEY);
	printf("GPIO_BIT_NO_TS_KEY=%d\n", value);
	mdelay(1000);
	value = gpio_get_value(GPIO_BIT_NO_COVER_OPEN);
	printf("GPIO_BIT_NO_COVER_OPEN=%d\n", value);
	value = gpio_get_value(GPIO_BIT_NO_TS_KEY);
	printf("GPIO_BIT_NO_TS_KEY=%d\n", value);
	mdelay(1000);

	/* test paper empty */
	dump_adc_channels(1);
	mdelay(1000);
	printf("test paper empty()\n");
	printf("GPIO_BIT_NO_EN_PE=%d, 0\n", GPIO_BIT_NO_EN_PE);
	gpio_direction_output(GPIO_BIT_NO_EN_PE, 0);
	mdelay(1000);
	dump_adc_channels(1);
	mdelay(1000);

	printf("test paper empty()\n");
	printf("GPIO_BIT_NO_EN_PE=%d, 1\n", GPIO_BIT_NO_EN_PE);
	gpio_direction_output(GPIO_BIT_NO_EN_PE, 1);
	mdelay(1000);
	dump_adc_channels(1);
	mdelay(1000);
}

int sy_printer (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]) 
{
	 unsigned int tmp;
	 int count = 10;
	printf("shu yan printer test  by jim gao! \n");

	gpio_init_printer();
	printf("adc_init();\n");
	adc_init();
	printf("\n");
	ssi_init();

	/* test spi */
	while(0) {
		unsigned int data;
		data = 0x5a5a5a5a;
		//spi_send(&data,4);
		spi_trans_data(&data,4);
		udelay(30);
	}

	/* test paper and cover */
	while(0) {
		printer_io_test(1);
		mdelay(3000);
	}

	/* test paper and cover */
	while(0) {
		printer_led_io_test(1);
		//mdelay(1000);
	}

	while(1 && count > 0){
		count --;
		printf("start adc read data\n");	
		mdelay(20);
		tmp=get_adc_channel_voltage(TS_TM);
		printf("shu yan printer test  by jim gao head temprature! TS_TM=%d\n",tmp);
		tmp=get_adc_channel_voltage(TS_VHD);
		printf("shu yan printer test  by jim gao voltage! TS_VHD=%d\n",tmp);
	}
	//gpio_init_uart();
	//gpio_direction_output(GPIO_BIT_ADC_PDA, 1);
	//while(1);
	mdelay(2);
	print_start();
	gpio_init_stop();

	return 0;
}
 
U_BOOT_CMD(sy_printer,1,2,sy_printer,
		           "printer command","jim gao add printer command!\n");
 

