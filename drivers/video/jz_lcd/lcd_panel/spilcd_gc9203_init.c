
#define SPILCD_DISPLAY_WIDTH  (128)
#define SPILCD_DISPLAY_HEIGHT  (220)
#define SPILCD_DRIVERIC_HOR_MAX  (176)
#define SPILCD_DRIVERIC_VER_MAX  (220)

#define SPILCD_HOR_START_OFFSET ((176-128)/2)


#define nrf_delay_ms mdelay

#define printk printf

#if 0
#define delay_us(us)				\
	do {					\
		udelay(us);			\
	} while (0)
#else
#define delay_us(us)				\
	do {					\
	} while (0)
#endif


#define RESB_H \
     gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_rst, 1)

#define RESB_L \
     gpio_direction_output(spilcd_gc9203_pdata.gpio_lcd_rst, 0)

/* SPI_CS   */
#define CS(n)								\
	do {								\
		if (spilcd_gc9203_pdata.gpio_spi_cs)					\
			gpio_direction_output(spilcd_gc9203_pdata.gpio_spi_cs, n);	\
	} while(0)

/* RS_DC   */
#define RS_DC(n)							\
	do {								\
		if (spilcd_gc9203_pdata.gpio_spi_rs)					\
			gpio_direction_output(spilcd_gc9203_pdata.gpio_spi_rs, n);	\
	} while(0)

#if 1
/* SPI_CLK   */
#define SCK(n)					\
     gpio_direction_output(spilcd_gc9203_pdata.gpio_spi_scl, n)
//     gpio_set_value(spilcd_gc9203_pdata.gpio_spi_scl, n)

/* SPI_MOSI */
#define SDO(n)					\
	 gpio_direction_output(spilcd_gc9203_pdata.gpio_spi_sda, n)
//#define SDO(n)				\
//	 gpio_set_value(spilcd_gc9203_pdata.gpio_spi_sda, n)

#else
/*
  PE 0x10010400 Address base of GPIO Port E
  0x44: PORT A Pattern 0 Set Register
  0x48: PORT A Pattern 0 Clear Register
  .gpio_spi_scl = GPIO_PE(3),
  .gpio_spi_sda = GPIO_PE(0),
 */
/*xtang :
 *GPIO_PC_BASE 0x10010200
 *GPIO_PE_BASE 0x10010400
 *SLCD_SDA->PC(19)
 *SLCD_PCLK->PC(08)
 * PXMASK :0x24
 * */
#define SDO(n)								\
	do {								\
		if (n)							\
			*((volatile unsigned int *)0xB0010244) = 1<<19; \
		else							\
			*((volatile unsigned int *)0xB0010248) = 1<<19;	\
		udelay(0);						\
	}while(0)

#define SCK(n)								\
	do {								\
			*((volatile unsigned int *)0xB0010224) = 1<<25;	\
		if (n)							\
			*((volatile unsigned int *)0xB0010244) = 1<<25;	\
		else							\
			*((volatile unsigned int *)0xB0010248) = 1<<25;	\
		udelay(0);						\
	}while(0)

#endif


/*sck->pc(08)
 *sda->pc19
 cs->pc(25)
 rs->pc(26)
 * */

/* SPI_MISO */
#define SDI()\
	gpio_get_value(spilcd_gc9203_pdata.gpio_spi_sda)

static int sck_status()
{
		int ret = 0;
		ret = gpio_get_value(spilcd_gc9203_pdata.gpio_spi_scl);
		printk("***==========================spilcd_gc9203_pdata.gpio_spi_scl = %d\n",ret);
}
static int rst_status()
{
	int ret = 0;
	ret = gpio_get_value(spilcd_gc9203_pdata.gpio_lcd_rst);
	printk("==========================spilcd_gc9203_pdata.gpio_lcd_rst = %d\n",ret);
}

static inline void SPI_Write(unsigned char d)
{
	unsigned char i;
	int ret = 0;

	CS(0);
	delay_us(10);
	for(i=0;i<8;i++)
	{
		SDO(((d&0x80)>>7));
		delay_us(10);
		SCK(1);
//		sck_status();
		delay_us(20);
		d=d<<1;
		SCK(0);
//		sck_status();
	}

	delay_us(10);
	CS(1);
}

static inline void Write_Command(unsigned char c)
{
	RS_DC(0);
	SPI_Write(c);
}

static inline void Write_Data(unsigned char d)
{
	RS_DC(1);
	SPI_Write(d);
}

static inline void Write_Pixel_Data(unsigned char d)
{
	//RS_DC(1);
	SPI_Write(d);
}

static unsigned char SPI_GET_REG_VAL(void)
{
	unsigned char i;
	unsigned char data = 0;
	int ret = 0;

	RS_DC(1);
	gpio_direction_input(spilcd_gc9203_pdata.gpio_spi_sda);

	delay_us(10);
	for(i=0;i<8;i++)
	{
		SCK(1);
		data <<= 1;
		data |=SDI();
		delay_us(10);
		SCK(0);
		delay_us(20);
		printk("sdi= 0x%x\n",data);
	}

	gpio_direction_output(spilcd_gc9203_pdata.gpio_spi_sda, 0);
	return data;
}

static int SPI_READ_REG(unsigned char reg)
{
	int data = 0;

	/* Write_Command(0x66); */
	/* Write_Data(0x00); */
	/* Write_Data(0x01); */

	Write_Command(reg);
	data = SPI_GET_REG_VAL();
	printk("reg(0x%x)=0x%x\n",reg,data);

	/* Write_Command(0x66); */
	/* Write_Data(0x00); */
	/* Write_Data(0x00); */

	return data;
}

static void spilcd_write_gram(void * vidmem, int bpp)
{
	int i,j;
	unsigned char * vid;

	printk("%s() enter, vidmem=%p, bpp=%d\n", __func__, vidmem, bpp);

	//vid = (unsigned char *)(((unsigned long)vidmem));
	vid = (unsigned char *)(((unsigned long)vidmem)&~0x20000000); // cached
	printk("%s() enter, vid=%p\n", __func__, vid);
	Write_Command(0x22);
	RS_DC(1);
	for(i=0;i<176;i++) {
		for(j=0;j<220;j++) {
			unsigned int r,g,b,p;
			r = *vid++;
			g = *vid++;
			b = *vid++;
			vid++;;
			p =  ((r&0xf8)<<8) | ((g&0xfc)<<3)  | ((b&0xf8)>>3);
			Write_Pixel_Data(p>>8);
			Write_Pixel_Data(p);
		}
	}

	printk("%s() leave...\n", __func__);
}

static void spilcd_write_gram_colorbar()
{
	int i,j;
	printk("%s() clear screen...\n", __func__);

	Write_Command(0x22);
	RS_DC(1);
	for(i=0;i<176;i++) {
		for(j=0;j<220;j++) {
			if(i<(176/2)) {
				Write_Pixel_Data(0xff);
				Write_Pixel_Data(0xff);
			}
			else {
				Write_Pixel_Data(0x00);
				Write_Pixel_Data(0x00);
			}
		}
	}

	printk("%s() color bar...\n", __func__);
	Write_Command(0x22);
	RS_DC(1);
	for(i=0;i<128;i++) {
		for(j=0;j<220;j++) {
			switch((i/10)%5) {
			case 4:
				Write_Pixel_Data(0x00);
				Write_Pixel_Data(0x00);
				break;
			case 3:
				Write_Pixel_Data(0x00);
				Write_Pixel_Data(0x1f);
				break;
			case 2:
				Write_Pixel_Data(0x07);
				Write_Pixel_Data(0xe0);
				break;
			case 1:
				Write_Pixel_Data(0xf8);
				Write_Pixel_Data(0x00);
				break;
			case 0:
			default:
				Write_Pixel_Data(0xff);
				Write_Pixel_Data(0xff);
				break;
			}
		}
	}
	printk("%s()\n", __func__);
}

static void spilcd_write_gram_test()
{
	int i,j;
	int loopcount;
	printk("%s() clear screen...\n", __func__);

	loopcount = 0;
	while(loopcount++ < 10) {
		printk("%s() color bar...\n", __func__);
		Write_Command(0x22);
		RS_DC(1);
		for(i=0;i<128;i++) {
			for(j=0;j<220;j++) {
				switch((loopcount)%5) {
				case 4:
					Write_Pixel_Data(0x00);
					Write_Pixel_Data(0x00);
					break;
				case 3:
					Write_Pixel_Data(0x00);
					Write_Pixel_Data(0x1f);
					break;
				case 2:
					Write_Pixel_Data(0x07);
					Write_Pixel_Data(0xe0);
					break;
				case 1:
					Write_Pixel_Data(0xf8);
					Write_Pixel_Data(0x00);
					break;
				case 0:
				default:
					Write_Pixel_Data(0xff);
					Write_Pixel_Data(0xff);
					break;
				}
			}
		}

		printk("%s() frame finish.\n", __func__);
		mdelay(2000);
		printk("%s() add extra 1 pixel.\n", __func__);
		//Write_Pixel_Data(0x00); Write_Pixel_Data(0x00);
		Write_Pixel_Data(0xff); Write_Pixel_Data(0xff);
		Write_Pixel_Data(0xff); Write_Pixel_Data(0xff);
		Write_Pixel_Data(0xff); Write_Pixel_Data(0xff);
		for(i=0;i<2;i++) {
			mdelay(2000); printk("%s() add extra 1 line.\n", __func__);
			for(j=0;j<220;j++) {
				Write_Pixel_Data(0xf8);
				Write_Pixel_Data(0x00);
			}
		}
		mdelay(2000);
	}
	printk("%s()\n", __func__);
}

static void Init_Oled(void)
{
	int cnt,ret;
	cnt=0;
	
	printk("%s()\n", __func__);
	CS(1);
	SCK(0);
//	sck_status();
	SDO(0);
	RS_DC(0);

RESB_H;
rst_status();
nrf_delay_ms(20); 
RESB_L; 
rst_status();
nrf_delay_ms(20);
RESB_H; 
rst_status();
nrf_delay_ms(120);

#if 0
while(cnt++<3){
	SPI_READ_REG(0x00);
	mdelay(1000);
}
#endif	

Write_Command(0xff);
Write_Data(0x5a);
Write_Data(0xa5);

Write_Command(0xf6);
Write_Data(0x01);
Write_Data(0x12);

Write_Command(0xef);
Write_Data(0x14);
Write_Data(0x52);

Write_Command(0xa5);
Write_Data(0x07);
Write_Data(0x80);

Write_Command(0x02);
Write_Data(0x01);
Write_Data(0x00);

/* 6.2.5. Entry Mode (R03h) */
/*Write_Command(0x03);
Write_Data(0x10);
Write_Data(0x38);*/
Write_Command(0x03);
Write_Data(0x10);  // 65k color, rgb565
//Write_Data(0x12); //262k color, rgb666
Write_Data(0x18);


Write_Command(0xf6);
Write_Data(0x01);
Write_Data(0x12);

Write_Command(0x11);
Write_Data(0x10);
Write_Data(0x00);

Write_Command(0xeb);
Write_Data(0x16);
Write_Data(0x0a);

Write_Command(0xec);
Write_Data(0x16);
Write_Data(0x0a);

Write_Command(0x50);
Write_Data(0xf9);
Write_Data(0x80);


Write_Command(0x51);
Write_Data(0x13);
Write_Data(0x06);

Write_Command(0x52);
Write_Data(0x0e);
Write_Data(0x09);

Write_Command(0x53);
Write_Data(0x33);
Write_Data(0x09);

Write_Command(0x54);
Write_Data(0x4e);
Write_Data(0x24);

Write_Command(0x55);
Write_Data(0x1b);
Write_Data(0x1c);

Write_Command(0x56);
Write_Data(0x37);
Write_Data(0x36);

Write_Command(0x57);
Write_Data(0xf5);
Write_Data(0x80);

Write_Command(0x58);
Write_Data(0x13);
Write_Data(0x06);

Write_Command(0x59);
Write_Data(0x0e);
Write_Data(0x13);


Write_Command(0x5a);
Write_Data(0x3f);
Write_Data(0x09);

Write_Command(0x5b);
Write_Data(0x52);
Write_Data(0x24);

Write_Command(0x5c);
Write_Data(0x0f);
Write_Data(0x0d);

Write_Command(0x5d);
Write_Data(0x37);
Write_Data(0x36);

Write_Command(0x07);
Write_Data(0x10);
Write_Data(0x13);

Write_Command(0x01);
Write_Data(0x01);
Write_Data(0x1c);

Write_Command(0xfe);


/* set display resolution */
#if 1
/* 6.2.18. Horizontal and Vertical RAM Address Position (R36h/R37h,R38h/R39h) */
Write_Command(0x36);
Write_Data((0>>8)&0xff);
Write_Data((128+SPILCD_HOR_START_OFFSET)&0xff); // -1, extra line for last pixel blink.
Write_Command(0x37);
Write_Data(0x00);
Write_Data(SPILCD_HOR_START_OFFSET);
/*
Write_Command(0x38);
Write_Data(((220-1)>>8)&0xff);
Write_Data((220-1)&0xff);
Write_Command(0x39);
Write_Data(0x00);
Write_Data(0x00);
*/
#else
/* 6.2.18. Horizontal and Vertical RAM Address Position (R36h/R37h,R38h/R39h) */
#endif

//spilcd_write_gram_test();
spilcd_write_gram_colorbar();
Write_Command(0x22);

}

static void EnterSLP(void)
{

	
///////////////////////////NEW
Write_Command(0xff);
Write_Data(0x5a);
Write_Data(0xa5);


Write_Command(0x07);
Write_Data(0x00);
Write_Data(0x00);

nrf_delay_ms(120);//120
	
Write_Command(0x11);
Write_Data(0x00);
Write_Data(0x00);

nrf_delay_ms(250);//850
	
Write_Command(0x10);
Write_Data(0x00);
Write_Data(0x01);

Write_Command(0xa5);
Write_Data(0xff);
Write_Data(0xff);

Write_Command(0xe8);
Write_Data(0x0f);
Write_Data(0x0f);

Write_Command(0xed);
Write_Data(0x37);
Write_Data(0x0f);


Write_Command(0xe6);
Write_Data(0x00);
Write_Data(0xc2);

}



static void ExitSLP(void)
{


 Init_Oled();

}
