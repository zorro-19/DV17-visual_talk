#include "generic/typedef.h"
#include "asm/imb.h"
#include "asm/imd.h"
#include "asm/imb.h"
#include "asm/pap.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "os/os_api.h"
#include "gpio.h"
#include "asm/rtc.h"

#define delay2ms(t) delay_2ms(t)

#ifdef LCD_ST7789P3_MCU


#define SOFT_SIM  //use pap module

#if  0

	u8 read_RTC_reg(u8 cmd);
	void write_RTC_reg(u8 cmd, u8 reg);
	u8 RTC_reg = 0;
	RTC_reg = read_RTC_reg(0xA0 + 6);
	RTC_reg &= 0xFE;
	write_RTC_reg(0x20 + 6, RTC_reg);

#endif

//PC5->PV5 RESET
#define LCD_RST_OUT()
#define LCD_RST_H()        { gpio_direction_output(WKUP_IO_PR1, 1) ;	u8 read_RTC_reg(u8 cmd);\
	void write_RTC_reg(u8 cmd, u8 reg);\
	u8 RTC_reg = 0;\
	RTC_reg = read_RTC_reg(0xA0 + 2);\
	RTC_reg &= 0xDF;\
	RTC_reg |= 0x02;\
	write_RTC_reg(0x20 + 2, RTC_reg); }  // // 1101 0010


#define LCD_RST_L()          { gpio_direction_output(WKUP_IO_PR1, 0) ;	u8 read_RTC_reg(u8 cmd);\
	void write_RTC_reg(u8 cmd, u8 reg);\
	u8 RTC_reg = 0;\
	RTC_reg = read_RTC_reg(0xA0 + 2);\
	RTC_reg &= 0xDF;\
	RTC_reg &= 0xFD;\
	write_RTC_reg(0x20 + 2, RTC_reg); }  // // 1101 0000



//PC6->PV6 CS
#define LCD_CS_OUT()    PORTG_DIR &=~ BIT(7)
#define LCD_CS_H()      LCD_CS_OUT();PORTG_OUT |=  BIT(7)
#define LCD_CS_L()      LCD_CS_OUT();PORTG_OUT &=~ BIT(7)

//PC7->PV7  RS
#define LCD_RS_OUT()    PORTH_DIR &=~ BIT(2)
#define LCD_RS_H()      LCD_RS_OUT();PORTH_OUT |=  BIT(2)
#define LCD_RS_L()      LCD_RS_OUT();PORTH_OUT &=~ BIT(2)


#ifdef SOFT_SIM

    #define LCD_WR_OUT()    PORTH_DIR &=~ BIT(3)
    #define LCD_WR_H()      LCD_WR_OUT();PORTH_OUT |= BIT(3)
    #define LCD_WR_L()      LCD_WR_OUT();PORTH_OUT &=~BIT(3)

    #define LCD_RD_OUT()   // PORTH_DIR &=~ BIT(4)
    #define LCD_RD_H()     // LCD_RD_OUT();PORTH_OUT |= BIT(4)
    #define LCD_RD_L()    //  LCD_RD_OUT();PORTH_OUT &=~BIT(4)

    #define LCD_PORT_OUT()  PORTG_DIR &=~ 0xff00;PORTG_PU |= 0xff00
    #define LCD_PORT_IN()   PORTG_DIR |=  0xff00;PORTG_PU |= 0xff00
    #define LCD_PORT_W      PORTG_OUT
    #define LCD_PORT_R      PORTG_IN
#endif//end of SOFT_SIM

typedef struct
{
    u8 cmd;
    u8 cnt;
    u8 dat[128];
} InitCode;

#define REGFLAG_DELAY 0xFF

static const InitCode code1[] =
{
    {0x01, 0},//soft reset
    {REGFLAG_DELAY, 120},
    //{0x11, 0},
    //{REGFLAG_DELAY, 120},
    {0x36, 1, {0x00}},
    {0x21, 0},
    {0xB2, 5, {0x05, 0x05, 0x00, 0x33, 0x33}},
    {0xB7, 1, {0x75}},
    {0xBB, 1, ( 0x22 )},
    {0xC0, 1, {0x2C}},
    {0xC2, 1, {0x01}},
    {0xC3, 1, {0x13}},
    {0xC4, 1, {0x20}},
    {0xC6, 1, {0x11}},
    {0xD0, 2, {0xA4, 0xA1}},
    {0xD6, 1, {0xA1}},
    {0xE0, 14, {0xD0, 0x05, 0x0A, 0x09, 0x08, 0x05, 0x2E, 0x44, 0x45, 0x0F, 0x17, 0x16, 0x2B, 0x33}},
    {0xE1, 14, {0xD0, 0x05, 0x0A, 0x09, 0x08, 0x05, 0x2E, 0x43, 0x45, 0x0F, 0x16, 0x16, 0x2B, 0x33}},
    {0x3A, 1, {0x05}},
    {0x35, 1},

    {0x11, 0},//Sleep out
    {REGFLAG_DELAY, 150},
    {0x29, 0},
    {REGFLAG_DELAY, 100},

};
static void *pap = NULL;

extern void delay( volatile u32  t );

static void LCD_Reset()
{
    printf( "\n <<<<<<<<<<<<<lcd reset >>>>>>>\n" );
    LCD_RST_OUT();
    LCD_RS_OUT();
    LCD_CS_OUT();
    #ifdef SOFT_SIM
    LCD_RD_OUT();
    LCD_WR_OUT();
    LCD_RD_H();
    LCD_WR_H();
    #else
    struct pap_info pap_info_t;
    pap_info_t.datawidth    = PAP_PORT_8BITS;
    pap_info_t.endian       = PAP_BE;//8bit必须为大端
    pap_info_t.cycle        = PAP_CYCLE_ONE;
    pap_info_t.group_sel    = PAP_GROUP_PG_PH;
    pap_info_t.wr_rd_sel    = PAP_PH3_PH4;
    pap_info_t.timing_setup = 2;
    pap_info_t.timing_hold  = 2;
    pap_info_t.timing_width = 5;
    pap = dev_open( "pap", &pap_info_t );
    #endif
    LCD_RST_H();
    delay2ms( 60 );
    delay2ms( 60 );
    LCD_RST_L();
    delay2ms( 10 );
    delay2ms( 10 );
    delay2ms( 10 );
    LCD_RST_H();
    delay2ms( 100 );
}

#ifdef SOFT_SIM

static void LCDWrite( u16 dat )
{
    LCD_PORT_OUT();
    LCD_WR_L();
    LCD_PORT_W &= ~0xff00;
    LCD_PORT_W |= ( dat & 0xff ) << 8;
    LCD_WR_H();
}

static u16 LCDRead()
{
    LCD_PORT_IN();
    LCD_RD_L();
    LCD_RD_H();
    return ( LCD_PORT_R >> 8 ) & 0xff;
}

#endif

static void WriteComm( u16 cmd )
{
    LCD_CS_L();
    LCD_RS_L();
    #ifndef SOFT_SIM
    dev_write( pap, &cmd, sizeof( cmd ) );
    #else
    LCD_RD_H();
    LCDWrite( cmd );
    #endif
    LCD_CS_H();
}

static void WriteData( u16 dat )
{
    LCD_CS_L();
    LCD_RS_H();
    #ifndef SOFT_SIM
    dev_write( pap, &dat, sizeof( dat ) );
    #else
    LCD_RD_H();
    LCDWrite( dat );
    #endif
    LCD_CS_H();
}

static u16 ReadData()
{
    u16 ret = 0;
    LCD_CS_L();
    LCD_RS_H();
    #ifndef SOFT_SIM
    dev_read( pap, &ret, sizeof( ret ) );
    #else
    LCD_WR_H();
    ret = LCDRead();
    #endif
    LCD_CS_H();
    return ret;
}

static void st7789s_init_code( const InitCode *code, u8 cnt )
{
    u8 i, j;

    u16 para = 0;

    printf( " \n............7789P3 initial..........\n" );
    LCD_Reset();
    delay_2ms( 25 ); /* delay 50 ms */
    delay_2ms( 25 ); /* delay 50 ms */

#if  1


    delay_2ms(120);

    WriteComm(0x11);

    delay_2ms(120);

    WriteComm(0xB2);
    WriteData(0x0C);
    WriteData(0x0C);
    WriteData(0x00);
    WriteData(0x33);
    WriteData(0x33);

    WriteComm(0x35);     //TE
    WriteData(0x00);

    WriteComm(0x36);
    WriteData(0x00);

    WriteComm(0x3A);
    WriteData(0x05);

    WriteComm(0xB7);
    WriteData(0x56);

    // WriteComm(0xE7);    //3?2DATA ????
    //WriteData(0x10);


    WriteComm(0xBB);
    WriteData(0x0C);

    WriteComm(0xC0);
    WriteData(0x2C);

    WriteComm(0xC2);
    WriteData(0x01);

    WriteComm(0xC3);
    WriteData(0x0F);

    WriteComm(0xC6);
    WriteData(0x0F);

    WriteComm(0xD0);
    WriteData(0xA7);

    delay_2ms(10);

    WriteComm(0xD0);
    WriteData(0xA4);
    WriteData(0xA1);

    WriteComm(0xD6);
    WriteData(0xA1);

    WriteComm(0xE0);
    WriteData(0xF0);
    WriteData(0x01);
    WriteData(0x08);
    WriteData(0x04);
    WriteData(0x05);
    WriteData(0x14);
    WriteData(0x33);
    WriteData(0x44);
    WriteData(0x49);
    WriteData(0x36);
    WriteData(0x11);
    WriteData(0x14);
    WriteData(0x2E);
    WriteData(0x36);

    WriteComm(0xE1);
    WriteData(0xF0);
    WriteData(0x0C);
    WriteData(0x10);
    WriteData(0x0E);
    WriteData(0x0C);
    WriteData(0x08);
    WriteData(0x32);
    WriteData(0x43);
    WriteData(0x49);
    WriteData(0x28);
    WriteData(0x12);
    WriteData(0x12);
    WriteData(0x2C);
    WriteData(0x33);

    WriteComm(0x21);

    WriteComm(0x29);
    delay_2ms(50);
    WriteComm(0x2C);
#else

    delay_2ms(120);                //ms

    WriteComm( 0x11);

    delay_2ms(120);                //ms

    WriteComm( 0x36);
    WriteData( 0x00);

    WriteComm( 0x3A);
    WriteData( 0x05);

    WriteComm( 0xB2);
    WriteData( 0x0C);
    WriteData( 0x0C);
    WriteData( 0x00);
    WriteData( 0x33);
    WriteData( 0x33);

    WriteComm( 0xB7);
    WriteData( 0x46);   //VGH=13.65V,VGL=-11.38V

    WriteComm( 0xBB);
    WriteData( 0x1B);

    WriteComm( 0xC0);
    WriteData( 0x2C);

    WriteComm( 0xC2);
    WriteData( 0x01);

    WriteComm( 0xC3);
    WriteData( 0x0F);

    WriteComm( 0xC4);
    WriteData( 0x20);

    WriteComm( 0xC6);
    WriteData( 0x0F);

    WriteComm( 0xD0);
    WriteData( 0xA7);
    WriteData( 0xA1);

    WriteComm( 0xD0);
    WriteData( 0xA4);
    WriteData( 0xA1);

    WriteComm( 0xD6);
    WriteData( 0xA1);

    WriteComm( 0xE0);
    WriteData( 0xF0);
    WriteData( 0x00);
    WriteData( 0x06);
    WriteData( 0x04);
    WriteData( 0x05);
    WriteData( 0x05);
    WriteData( 0x31);
    WriteData( 0x44);
    WriteData( 0x48);
    WriteData( 0x36);
    WriteData( 0x12);
    WriteData( 0x12);
    WriteData( 0x2B);
    WriteData( 0x34);

    WriteComm( 0xE1);
    WriteData( 0xF0);
    WriteData( 0x0B);
    WriteData( 0x0F);
    WriteData( 0x0F);
    WriteData( 0x0D);
    WriteData( 0x26);
    WriteData( 0x31);
    WriteData( 0x43);
    WriteData( 0x47);
    WriteData( 0x38);
    WriteData( 0x14);
    WriteData( 0x14);
    WriteData( 0x2C);
    WriteData( 0x32);

    WriteComm( 0x21);

    WriteComm( 0x29);

    WriteComm( 0x2C);

#endif

    //第二套代码效果
    LCD_CS_L();
    LCD_RS_H();







}

static void lcd_st7789s_setxy( u16 Xstart, u16 Xend, u16 Ystart, u16 Yend )
{
    //HX8352-C
    /* printf("%x:%x:%x:%x\n",Xstart,Xend,Ystart,Yend); */
    WriteComm( 0x2a );
    WriteData( Xstart >> 8 );
    WriteData( Xstart & 0xff );
    WriteData( Xend >> 8 );
    WriteData( Xend & 0xff );
    WriteComm( 0x2b );
    WriteData( Ystart >> 8 );
    WriteData( Ystart & 0xff );
    WriteData( Yend >> 8 );
    WriteData( Yend & 0xff );
    WriteComm( 0x2c );
    LCD_CS_L();
    LCD_RS_H();
}


static void lcd_st7789s_clear( u16 w, u16 h, u32 color )
{
    u32 dat;
    u32 len;
    u16 i, j; // 0X FF0000
    lcd_st7789s_setxy( 0, w - 1, 0, h - 1 );
    dat = ( ( color >> 19 ) << 11 ) & 0xf800 | ( ( color >> 10 ) << 5 ) & 0x07e0 | ( color >> 3 ) & 0x001f;
    len = w * h;
    while( len-- )
    {
        WriteData( dat >> 8 );
        WriteData( dat );
    }
}

static void lcd_st7789s_init( void *_data )
{
    struct lcd_platform_data *data = ( struct lcd_platform_data * )_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u16 ret;
    u8 i;
    printf( "\n lcd_st7789s_init...\n" );
    LCD_Reset();

    st7789s_init_code( code1, sizeof( code1 ) / sizeof( code1[0] ) );

    #if 0
     gpio_direction_output( IO_PORTG_06,1 );
     lcd_st7789s_clear(240, 320, 0xff0000);
     printf("\n 1111111111111111 \n");
     os_time_dly(100);
     lcd_st7789s_clear(240, 320, 0x00ff00);
     printf("\n 2222222222222222 \n");
     os_time_dly(100);
     lcd_st7789s_clear(240, 320, 0x0000ff);
     printf("\n 333333333333333 \n");
     os_time_dly(100);

     while(0){


       os_time_dly(1);
       printf("\n lcd\n");
     }
    #endif

    lcd_st7789s_setxy( 0, 240, 0, 320 );

    #if 0
     gpio_direction_output( IO_PORTG_06,1 );
     lcd_st7789s_clear(240, 320, 0xff0000);
     printf("\n 1111111111111111 \n");
     os_time_dly(100);
     lcd_st7789s_clear(240, 320, 0x00ff00);
     printf("\n 2222222222222222 \n");
     os_time_dly(100);
     lcd_st7789s_clear(240, 320, 0x0000ff);
     printf("\n 333333333333333 \n");
     os_time_dly(100);
    #endif

    LCD_CS_L();
    LCD_RS_H();

    #ifdef SOFT_SIM
   // LCD_RD_H();
    #endif


    printf( " \n............7789P3 initial end..........\n" );
}
u8 backlight_flag = 0;
extern struct  lcd_platform_data get_lcd_type;
void lcd_light_timeout(void *priv){


   printf("\n lcd_priv=================%s\n",priv);
   if(!strcmp(priv,"on")){
    putchar('n');
    gpio_direction_output( get_lcd_type.lcd_io.backlight, get_lcd_type.lcd_io.backlight_value );
   }else if(!strcmp(priv,"off")) {

    putchar('o');
    gpio_direction_output( get_lcd_type.lcd_io.backlight, !get_lcd_type.lcd_io.backlight_value );
   }else{


   }
}
static void lcd_st7789s_backctrl( void *_data, u8 on )
{
    struct lcd_platform_data *data = ( struct lcd_platform_data * )_data;




        if( on )
        {
             putchar('O');
           // gpio_direction_output( data->lcd_io.backlight, data->lcd_io.backlight_value );
         //  sys_timeout_add("on", lcd_light_timeout, 500);

           sys_hi_timeout_add("on", lcd_light_timeout, 500);
        }
        else
        {   putchar('N');
            //gpio_direction_output( data->lcd_io.backlight, !data->lcd_io.backlight_value );
          //  sys_timeout_add("off", lcd_light_timeout, 500);

            sys_hi_timeout_add("off", lcd_light_timeout, 500);
        }

}
/*
struct imd_dmm_info info;
struct te_mode_ctrl te_mode;
struct pll4_info pll4;
enum LCD_MODE drive_mode;       // 驱动模式
enum NCYCLE ncycle;             // 每像素时钟数
enum PHASE raw_odd_phase;       // 奇行相位
enum PHASE raw_even_phase;      // 偶行相位
enum RAW_MODE raw_mode;         // RAW模式选择
enum LCD_PORT data_width;       // 数据位宽
enum LCD_GROUP port_sel;        // 输出端口选择
u8 avout_mode;                  // AVOUT制式(PAL/NTSC/TESTMODE)
u8 dclk_set;                    // dclk使能以及极性配置
u8 sync0_set;                   // AA:PH3/AB:PG2/B:PD3 (DE/HSYNC/VSYNC)
u8 sync1_set;                   // AA:PH4/AB:PG4/B:PD4 (DE/HSYNC/VSYNC)
u8 sync2_set;                   // AA:PH5/AB:PG5/B:PD5 (DE/HSYNC/VSYNC)
u8 sync3_set;                   // Reversed
u32 clk_cfg;                    // clk时钟分频配置
struct imd_dpi_timing timing;   // 时序参数

*/
REGISTER_IMD_DEVICE_BEGIN( lcd_st7789s_dev ) =
{
    .info = {
        .test_mode       = false,
        .test_mode_color = 0x000000,
        .canvas_color    = 0xff0000,
        .xres            = LCD_DEV_WIDTH,
        .yres            = LCD_DEV_HIGHT,
        .target_xres     = LCD_DEV_WIDTH,
        .target_yres     = LCD_DEV_HIGHT,
        .buf_num         = LCD_DEV_BNUM,
        .buf_addr        = LCD_DEV_BUF,
        .sample          = LCD_DEV_SAMPLE,
        .len             = LEN_256,
        .format          = FORMAT_RGB565,
        .interlaced_mode = INTERLACED_NONE,

        .rotate_en       = false,        // 旋转使能
        .hori_mirror_en  = false,        // 水平镜像使能
        .vert_mirror_en  = false,       // 垂直镜像使能

        .adjust = {
            .y_gain = 0x100,
            .u_gain = 0x100,
            .v_gain = 0x100,
            .r_gain = 0x80,
            .g_gain = 0x80,
            .b_gain = 0x80,
            .r_coe0 = 0x80,
            .g_coe1 = 0x80,
            .b_coe2 = 0x80,
            .r_gma  = 100,
            .g_gma  = 100,
            .b_gma  = 100,
        },
    },
    .te_mode = {

         .te_mode_en = false,
        .event = EVENT_IO_2,

        .edge = EDGE_NEGATIVE,
        .gpio = IO_PORTG_07,//IO_PORTH_01,
    },
    .drive_mode      = MODE_MCU,
    .ncycle          = CYCLE_TWO,
    .data_width      = PORT_8BITS,

    .dclk_set        = /*CLK_EN |*/CLK_NEGATIVE,
    .sync0_set       = CLK_EN | SIGNAL_DEN /*| CLK_NEGATIVE*/,
    .sync1_set       = /*CLK_EN|*/SIGNAL_HSYNC /*| CLK_NEGATIVE*/,
    .sync2_set       = /*CLK_EN|*/SIGNAL_VSYNC /*| CLK_NEGATIVE*/,
    #if  0

     * PORT_GROUP_AA:
     *    DAT0~DAT23 : PG0~PG15,PH0~PH1,PH6~PH11
     *    PH2 : DCLK
     *    PH3 : DE(Default,configurable)
     *    PH4 : HSYNC(Default,configurable)
     *    PH5 : VSYNC(Default,configurable)
    #endif



    .port_sel        = PORT_GROUP_AA,
    #if 1
    .clk_cfg         = PLL2_CLK_480M | DIVA_5 | DIVB_4,
    #else
    .clk_cfg         = IMD_INTERNAL | OSC_32K | PLL1_CLK | DIVA_3 | DIVB_1 | DIVC_1,
    .pll1_nr         = 3662,/* 3662*0.032768=120M */
    .pll1_nf         = 1,
    #endif
    .timing = {
        #if 1
        .hori_total             = ( 240 + 20 ) * 2,
        .hori_sync              = 10 * 2,
        .hori_back_porth        = 10 * 2,
        .hori_pixel             = 240,

        .vert_total             = 320 + 40,
        .vert_sync              = 20,
        .vert_back_porth_odd    = 20,
        .vert_back_porth_even   = 0,
        .vert_pixel             = 320,
        #else
        .hori_total             = 240 * 2 + 40,
        .hori_sync              = 10 * 2,
        .hori_back_porth        = 10 * 2 + 1,
        .hori_pixel             = 240,

        .vert_total             = 320 + 80,
        .vert_sync              = 20,
        .vert_back_porth_odd    = 21,
        .vert_back_porth_even   = 12,
        .vert_pixel             = 320,
        #endif
    },
},
REGISTER_IMD_DEVICE_END()


REGISTER_LCD_DEVICE_DRIVE(dev)  =
{
    .logo      = "lcd_7789_s",
    .type    = LCD_DVP_MCU,
    .init    = lcd_st7789s_init,
    .dev     = &lcd_st7789s_dev,
    .setxy   = lcd_st7789s_setxy,
    .bl_ctrl_flags = BL_CTRL_BACKLIGHT_AND_CONTROLER,
    .bl_ctrl = lcd_st7789s_backctrl,
    .te_mode_dbug =  FALSE,
};

#endif

