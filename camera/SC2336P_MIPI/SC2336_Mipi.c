
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "SC2336_mipi.h"
#include "asm/isp_alg.h"

static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};


extern void *SC2336_mipi_get_ae_params();
extern void *SC2336_mipi_get_awb_params();
extern void *SC2336_mipi_get_iq_params();
extern void SC2336_mipi_ae_ev_init(u32 fps);

#define INPUT_CLK  24
#define PCLK  	81//1200*2250*30

#define LINE_LENGTH_CLK     0x898
#define FRAME_LENGTH30      1200
#define FRAME_LENGTH25      1440
#define FRAME_LENGTH27      1309
#define ROW_TIME_NS         27777

#define LINE_LENGTH_CLK_25FPS     0xb40
#define LINE_LENGTH_CLK_20FPS     0xe10
#define LINE_LENGTH_CLK_15FPS     0x12c0
#define ROW_TIME_NS_25FPS         35556
#define ROW_TIME_NS_20FPS         44445
#define ROW_TIME_NS_15FPS         59260

static u32 cur_again = -1;
static u32 cur_dgain = -1;
static u32 cur_expline = -1;
static u32 cur_line_length_clk = -1;

static u32 line_length_clk = LINE_LENGTH_CLK;

static void *iic = NULL;
static u32 uframelen = FRAME_LENGTH30;
static u8 WRCMD = 0x60;
static u8 RDCMD = 0x61;

static  u8  sensor_type;   //0 为SC2336P,  1为SC2336

typedef struct {
    u16 addr;
    u8 value;
} Sensor_reg_ini;


 //#if  SC2336P

  // SC2336P
Sensor_reg_ini SC2336P_mipi_INI_REG[] =
{

    /*

    mipi 2 lane mode
    SensorName=sc2336_30fps
    width=1920
    height=1080
    port=0
    type=6
    pin=3
    SlaveID=0x60
    mode=3
    FlagReg=0x36ff
    FlagMask=0xff
    FlagData=0x00
    FlagReg1=0x36ff
    FlagMask1=0xff
    FlagData1=0x00
    outformat=3
    mclk=24
    avdd=2.800000
    dovdd=1.800000
    dvdd=1.30
    */

    0x0103,0x01,
    0x0100,0x00,
    0x36e9,0x80,
    0x37f9,0x80,
    0x301f,0x09,
    0x3106,0x05,
    0x320e,0x04,
    0x320f,0xb0,

     //  0x3221,0x60,//¾µÏñµ¹ÖÃ
    0x3221,0x66,//¾µÏñµ¹ÖÃ


    0x3248,0x04,
    0x3249,0x0b,
    0x3253,0x08,
    0x3301,0x09,
    0x3302,0xff,
    0x3303,0x10,
    0x3306,0x80,
    0x3307,0x02,
    0x3309,0xc8,
    0x330a,0x01,
    0x330b,0x30,
    0x330c,0x16,
    0x330d,0xff,
    0x3318,0x02,
    0x331f,0xb9,
    0x3321,0x0a,
    0x3327,0x0e,
    0x332b,0x12,
    0x3333,0x10,
    0x3334,0x40,
    0x335e,0x06,
    0x335f,0x0a,
    0x3364,0x1f,
    0x337c,0x02,
    0x337d,0x0e,
    0x3390,0x09,
    0x3391,0x0f,
    0x3392,0x1f,
    0x3393,0x20,
    0x3394,0x20,
    0x3395,0xe0,
    0x33a2,0x04,
    0x33b1,0x80,
    0x33b2,0x68,
    0x33b3,0x42,
    0x33f9,0x90,
    0x33fb,0xd0,
    0x33fc,0x0f,
    0x33fd,0x1f,
    0x349f,0x03,
    0x34a6,0x0f,
    0x34a7,0x1f,
    0x34a8,0x42,
    0x34a9,0x18,
    0x34aa,0x01,
    0x34ab,0x43,
    0x34ac,0x01,
    0x34ad,0x80,
    0x3630,0xf4,
    0x3632,0x44,
    0x3633,0x22,
    0x3639,0xf4,
    0x363c,0x47,
    0x3670,0x09,
    0x3674,0xf4,
    0x3675,0xfb,
    0x3676,0xed,
    0x367c,0x09,
    0x367d,0x0f,
    0x3690,0x22,
    0x3691,0x22,
    0x3692,0x22,
    0x3698,0x89,
    0x3699,0x96,
    0x369a,0xd0,
    0x369b,0xd0,
    0x369c,0x09,
    0x369d,0x0f,
    0x36a2,0x09,
    0x36a3,0x0f,
    0x36a4,0x1f,
    0x36d0,0x01,
    0x36ea,0x0b,
    0x36eb,0x0c,
    0x36ec,0x1c,
    0x36ed,0x18,
    0x3722,0xc1,
    0x3724,0x41,
    0x3725,0xc1,
    0x3728,0x20,
    0x37fa,0xcb,
    0x37fb,0x32,
    0x37fc,0x11,
    0x37fd,0x07,
    0x3900,0x0d,
    0x3905,0x98,
    0x3919,0x04,
    0x391b,0x81,
    0x391c,0x10,
    0x3933,0x81,
    0x3934,0xd0,
    0x3940,0x75,
    0x3941,0x00,
    0x3942,0x01,
    0x3943,0xd1,
    0x3952,0x02,
    0x3953,0x0f,
    0x3e01,0x4a,
    0x3e02,0xa0,
    0x3e08,0x1f,
    0x3e1b,0x14,
    0x440e,0x02,
    0x4509,0x38,
    0x4819,0x06,
    0x481b,0x03,
    0x481d,0x0b,
    0x481f,0x02,
    0x4821,0x08,
    0x4823,0x03,
    0x4825,0x02,
    0x4827,0x03,
    0x4829,0x04,
    0x5799,0x06,
    0x5ae0,0xfe,
    0x5ae1,0x40,
    0x5ae2,0x30,
    0x5ae3,0x28,
    0x5ae4,0x20,
    0x5ae5,0x30,
    0x5ae6,0x28,
    0x5ae7,0x20,
    0x5ae8,0x3c,
    0x5ae9,0x30,
    0x5aea,0x28,
    0x5aeb,0x3c,
    0x5aec,0x30,
    0x5aed,0x28,
    0x5aee,0xfe,
    0x5aef,0x40,
    0x5af4,0x30,
    0x5af5,0x28,
    0x5af6,0x20,
    0x5af7,0x30,
    0x5af8,0x28,
    0x5af9,0x20,
    0x5afa,0x3c,
    0x5afb,0x30,
    0x5afc,0x28,
    0x5afd,0x3c,
    0x5afe,0x30,
    0x5aff,0x28,
    0x36e9,0x53,
    0x37f9,0x33,
    0x0100,0x01,

    };

//#else
 // SC2336

Sensor_reg_ini SC2336_mipi_INI_REG[] =
{
	//mipi 2 lane mode
    0x0103,0x01,
    0x0100,0x00,
    0x36e9,0x80,
    0x37f9,0x80,
    0x301f,0x02,
    0x3106,0x05,
    0x320c,0x08,
    0x320d,0xca,
    0x320e,0x04,
    0x320f,0xb0,

  //  0x3221,0x60,//¾µÏñµ¹ÖÃ
    0x3221,0x66,//¾µÏñµ¹ÖÃ

    0x3248,0x04,
    0x3249,0x0b,
    0x3253,0x08,
    0x3301,0x09,
    0x3302,0xff,
    0x3303,0x10,
    0x3306,0x60,
    0x3307,0x02,
    0x330a,0x01,
    0x330b,0x10,
    0x330c,0x16,
    0x330d,0x90,
    0x3318,0x02,
    0x3321,0x0a,
    0x3327,0x0e,
    0x332b,0x12,
    0x3333,0x10,
    0x3334,0x40,
    0x335e,0x06,
    0x335f,0x0a,
    0x3364,0x1f,
    0x337c,0x02,
    0x337d,0x0e,
    0x3390,0x09,
    0x3391,0x0f,
    0x3392,0x1f,
    0x3393,0x20,
    0x3394,0x20,
    0x3395,0xff,
    0x33a2,0x04,
    0x33b1,0x80,
    0x33b2,0x68,
    0x33b3,0x42,
    0x33f9,0x70,
    0x33fb,0xd0,
    0x33fc,0x0f,
    0x33fd,0x1f,
    0x349f,0x03,
    0x34a6,0x0f,
    0x34a7,0x1f,
    0x34a8,0x42,
    0x34a9,0x06,
    0x34aa,0x01,
    0x34ab,0x23,
    0x34ac,0x01,
    0x34ad,0x84,
    0x3630,0xf4,
    0x3633,0x22,
    0x3639,0xf4,
    0x3670,0x09,
    0x3674,0xf8,
    0x3675,0xfb,
    0x3676,0xed,
    0x367c,0x09,
    0x367d,0x0f,
    0x3690,0x22,
    0x3691,0x32,
    0x3692,0x33,
    0x3698,0x89,
    0x3699,0x95,
    0x369a,0xbb,
    0x369b,0xbb,
    0x369c,0x09,
    0x369d,0x0f,
    0x36a2,0x09,
    0x36a3,0x0f,
    0x36a4,0x1f,
    0x36d0,0x01,
    0x36ea,0x09,
    0x36eb,0x0c,
    0x36ec,0x1c,
    0x36ed,0x28,
    0x3722,0xe1,
    0x3724,0x11,
    0x3725,0x91,
    0x3728,0x20,
    0x37fa,0x09,
    0x37fb,0x32,
    0x37fc,0x11,
    0x37fd,0x37,
    0x3900,0x0d,
    0x3905,0x98,
    0x391b,0x81,
    0x391c,0x10,
    0x3933,0x81,
    0x3934,0xc9,
    0x3940,0x6d,
    0x3942,0x01,
    0x3943,0xcb,
    0x3952,0x02,
    0x3953,0x0f,
    0x3e01,0x4a,
    0x3e02,0xb0,
    0x3e08,0x1f,
    0x3e1b,0x14,
    0x440e,0x02,
    0x4509,0x38,
    0x4819,0x06,
    0x481b,0x03,
    0x481d,0x0b,
    0x481f,0x03,
    0x4821,0x08,
    0x4823,0x03,
    0x4825,0x03,
    0x4827,0x03,
    0x4829,0x05,
    0x5799,0x06,
    0x5ae0,0xfe,
    0x5ae1,0x40,
    0x5ae2,0x30,
    0x5ae3,0x28,
    0x5ae4,0x20,
    0x5ae5,0x30,
    0x5ae6,0x28,
    0x5ae7,0x20,
    0x5ae8,0x3c,
    0x5ae9,0x30,
    0x5aea,0x28,
    0x5aeb,0x3c,
    0x5aec,0x30,
    0x5aed,0x28,
    0x5aee,0xfe,
    0x5aef,0x40,
    0x5af4,0x30,
    0x5af5,0x28,
    0x5af6,0x20,
    0x5af7,0x30,
    0x5af8,0x28,
    0x5af9,0x20,
    0x5afa,0x3c,
    0x5afb,0x30,
    0x5afc,0x28,
    0x5afd,0x3c,
    0x5afe,0x30,
    0x5aff,0x28,
    0x36e9,0x53,
    0x37f9,0x53,
    0x0100,0x01,
};
//#endif


unsigned char wrSC2336_mipi_Reg(u16 regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD)) {
        ret = 0;
        goto __wend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
        ret = 0;
        goto __wend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID & 0xff)) {
        ret = 0;
        goto __wend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        goto __wend;
    }

__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    if (ret == 0)
        printf("wreg iic fail\n");
    return ret;

}

unsigned char rdSC2336_mipi_Reg(u16 regID, unsigned char *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
        ret = 0;
        goto __rend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID & 0xff)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, RDCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(1000);

    dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);

__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    if (ret == 0)
        printf("rreg iic fail\n");
    return ret;
}


/*************************************************************************************************
    sensor api
*************************************************************************************************/

void SC2336_mipi_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u8 v;


    SC2336_mipi_set_output_size(width, height, frame_freq);
    switch(sensor_type){

    case 0:

     for (i = 0; i < sizeof(SC2336P_mipi_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrSC2336_mipi_Reg(SC2336P_mipi_INI_REG[i].addr, SC2336P_mipi_INI_REG[i].value);
     }


      break ;
    case 1:


    for (i = 0; i < sizeof(SC2336_mipi_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrSC2336_mipi_Reg(SC2336_mipi_INI_REG[i].addr, SC2336_mipi_INI_REG[i].value);
    }


    break ;

    default:
        printf("\n no sensor type \n");
        break;
    }

    SC2336_mipi_ae_ev_init(*frame_freq);

    if (*frame_freq == 25)
    {
        wrSC2336_mipi_Reg(0x320e,0x05);
        wrSC2336_mipi_Reg(0x320f,0xa0);
        uframelen = FRAME_LENGTH25;

    }
    else if (*frame_freq == 27)
    {
		wrSC2336_mipi_Reg(0x320e,0x05);
		wrSC2336_mipi_Reg(0x320f,0x1d);
		uframelen = FRAME_LENGTH27;
    }
    else
        uframelen = FRAME_LENGTH30;


    *format = SEN_IN_FORMAT_BGGR;
    return;
}


s32 SC2336_mipi_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    *width = SC2336_MIPI_OUTPUT_W;
    *height = SC2336_MIPI_OUTPUT_H;

    return 0;
}


s32 SC2336_mipi_power_ctl(u8 isp_dev, u8 is_work)
{

    return 0;
}

s32 SC2336_mipi_ID_check(void)
{
    #if 1
    u8 pid = 0x00;
    u8 ver = 0x00;
    //u8 exid = 0x00;
    u8 i ;

	u8 retry = 0;


	while(retry <= 5)
	{
	    wrSC2336_mipi_Reg(0x301a,0xf8);
        wrSC2336_mipi_Reg(0x0100,0x01);
       // delay(200000); //  delay 1ms
        delay(1000000); // delay 5ms
        delay(1000000); // delay 5ms
        delay(1000000); // delay 5ms
        delay(1000000); // delay 5ms
        delay(1000000); // delay 5ms
        delay(1000000); // delay 5ms
         ;
        rdSC2336_mipi_Reg(0x3107, &pid);
        rdSC2336_mipi_Reg(0x3108, &ver);
		//rdSC2336_mipi_Reg(0x3109, &exid);

		printf("%s %d 0x%x 0x%x \n", __FUNCTION__, retry, pid, ver);

		if (pid == 0x9b && ver == 0x3a ) {

			puts("\n----hello SC2336p_mipi-----\n");
			sensor_type=0;
			return 0;
		}else if (pid == 0xcb && ver == 0x3a ) {

			puts("\n----hello SC2336_mipi-----\n");
			sensor_type=1;
			return 0;
		}
		retry++;
		delay(500);
	}
	puts("\n----not SC2336p_mipi and not SC2336_mipi -----\n");
	return -1;

	#else
#if  0
	 u8 pid = 0x00;
    u8 ver = 0x00;
    u8 exid = 0x00;
    u8 i ;

	u8 retry = 0;
	while(retry <= 5)
	{
        rdSC2336_mipi_Reg(0x3107, &pid);
        rdSC2336_mipi_Reg(0x3108, &ver);
		rdSC2336_mipi_Reg(0x3109, &exid);

		printf("%s %d 0x%x 0x%x 0x%x\n", __FUNCTION__, retry, pid, ver, exid);

		if (pid == 0xcb && ver == 0x3a && exid == 0x01) {

			puts("\n----hello SC2336_mipi-----\n");
			return 0;
		}
		retry++;
		delay(500);
	}
	puts("\n----not SC2336_mipi-----\n");

	return -1;
#endif
	#endif

}

void SC2336_mipi_reset(u8 isp_dev)
{
    u32 reset_gpio;
    u32 pwdn_gpio;

    if (isp_dev == ISP_DEV_0) {
        reset_gpio = reset_gpios[0];
        pwdn_gpio = pwdn_gpios[0];
    } else {
        reset_gpio = reset_gpios[1];
        pwdn_gpio = pwdn_gpios[1];
    }
    gpio_direction_output(IO_PORTA_14, 0);
    //gpio_direction_output(IO_PORTA_12, 1);
    delay(40000);
	puts("2053_MIPI IO_PORTA_14 1 \n");
    gpio_direction_output(IO_PORTA_14, 1);
    //gpio_direction_output(IO_PORTA_12, 0);
    delay(40000);
}

void SC2336_mipi_change_fps(u8 sence, u16 lv)
{
	static s8 curSence = -1;

	//printf("%s: %d %d\n", __FUNCTION__, sence, lv);
}


static u8 cur_sensor_type = 0xff;

s32 SC2336_mipi_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{
	//if(!mipi_SC2336)
	//	return -1;

    puts("\n\n SC2336_mipi_check \n\n");

    #if 1
    if (!iic) {
            iic = dev_open("iic0", 0);
        if (!iic) {
                printf("iic 000000000000000000000000000\r\n");
            return -1;
        }
    } else {
        if (cur_sensor_type != isp_dev) {
            return -1;
        }
    }
    #else

     if (!iic) {
        if (isp_dev == ISP_DEV_0) {
            iic = dev_open("iic0", 0);
        } else {
            iic = dev_open("iic1", 0);
        }
        if (!iic) {
            return -1;
        }
    } else {
        if (cur_sensor_type != isp_dev) {
            return -1;
        }
    }

    #endif
    printf("\n\n isp_dev =%d\n\n", isp_dev);

    reset_gpios[isp_dev] = reset_gpio;
    pwdn_gpios[isp_dev] = pwdn_gpio;

	// ===============================
	//DVDD 1.2
//	gpio_direction_output(IO_PORTG_13, 0);
	//2.8 AVDD
	//avdd28_ctrl(AVDD28_28, true);

//	SC2336_mipi_reset(isp_dev);
    if (0 != SC2336_mipi_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }
	// ================================

    cur_sensor_type = isp_dev;

    return 0;
}

void resetStatic();
s32 SC2336_mipi_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n SC2336_mipi_init22 \n\n");

    SC2336_mipi_config_SENSOR(width, height, format, frame_freq);

    return 0;
}

static void set_again(u32 again)
{
    if (cur_again == again)
    {
        return;
    }

    wrSC2336_mipi_Reg(0x3e09, again & 0xff);
	cur_again = again;
    return;
}

static void set_dgain(u32 dgain)
{
    if (cur_dgain == dgain)
    {
        return;
    }

    wrSC2336_mipi_Reg(0x3e06, ((dgain>>8)&0xff));
    wrSC2336_mipi_Reg(0x3e07, (dgain&0xff));
	cur_dgain = dgain;
}

#define AGAIN_STEP (0x800)
#define MAX_DGAIN 4096
static void calc_gain(u32 gain, u32 *_again, u32 *_dgain)
{
    int i;
    u32 reg0, reg1, reg2;
    u32 dgain, againTndex, again;
    u8 againMap[] = {0x00,0x08,0x09,0x0b,0x0f,0x1f};

    if(gain <1024) gain = 1024;
    if(gain > 180*1024) gain = 180*1024;

    againTndex = 0;
    while (gain>=AGAIN_STEP)
    {
        gain *= 1024;
        gain /=AGAIN_STEP;
        againTndex++;

        if (againTndex == 0x05)
            break;
    }

    dgain = 0x00;
    if (gain >MAX_DGAIN)
        gain = MAX_DGAIN;
    while(gain >= 2048)
    {
        gain/=2;
        dgain*=2;
        dgain++;
    }

    *_again = againMap[againTndex];
    *_dgain = (dgain<<8) +(gain>>3);
    //printf("again = %x dgain = %x\n",*_again,*_dgain);
}


static void set_shutter(u32 texp)
{
    if (cur_expline == texp)
    {
        return;
    }
    cur_expline  = texp;
    wrSC2336_mipi_Reg(0x3e00, (texp >> 12)& 0xff);
    wrSC2336_mipi_Reg(0x3e01, (texp >> 4)& 0xff);
    wrSC2336_mipi_Reg(0x3e02, (texp << 4) & 0xf0);
	//delay_2ms(15);  //95-105
}


u32 SC2336_mipi_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;
    u32 texp_align;
    u32 ratio;
    u32 fLen;
    static u8 bLow = 0;

    fLen = uframelen;

    texp = exp_time_us * PCLK / LINE_LENGTH_CLK;

    if (texp < 4)
    {
        texp = 4;
    }
    if (texp > fLen  - 8)
    {
        texp = fLen  - 8;
    }
    texp_align = (texp) * LINE_LENGTH_CLK / (PCLK );

    if (texp_align < exp_time_us)
    {
        ratio = (exp_time_us) * (1 << 10) / texp_align;
        //printf("ratio = %d\n",ratio);
    }
    else
    {
        ratio = (1 << 10);
    }

    shutter->ae_exp_line =  texp;
    shutter->ae_gain = (gain * ratio) >> 10;
    shutter->ae_exp_clk = 0;

    //printf("exp_time_us=%d, texp=%d, gain=%d->%d  gain_val = %d\n", exp_time_us, texp, gain,shutter->ae_gain,gain>>10);
    return 0;

}
static void set_log()
{

}

u32 SC2336_mipi_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 again, dgain;

    calc_gain((shutter->ae_gain), &again, &dgain);
    set_shutter(shutter->ae_exp_line);
    set_again(again);
    set_dgain(dgain);

    return 0;
}


void SC2336_mipi_sleep()
{


}

void SC2336_mipi_wakeup()
{


}

void SC2336_mipi_W_Reg(u16 addr, u16 val)
{
    /*printf("update reg%x with %x\n", addr, val);*/
    wrSC2336_mipi_Reg((u16)addr, (u8)val);
}
u16 SC2336_mipi_R_Reg(u16 addr)
{
    u8 val;
    rdSC2336_mipi_Reg((u16)addr, &val);
    //printf("read reg%x val %x\n", addr, val);
    return val;
}

REGISTER_CAMERA(SC2336_mipi) = {
    .logo 				= 	"SC2336m",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_BGGR,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_CSI2,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_CSI2_2_LANE,
    .fps         		= 	25, // 30

    .sen_size 			= 	{SC2336_MIPI_OUTPUT_W, SC2336_MIPI_OUTPUT_H},
    .isp_size 			= 	{SC2336_MIPI_OUTPUT_W, SC2336_MIPI_OUTPUT_H},

    .cap_fps         		= 	25, // 30
    .sen_cap_size 			= 	{SC2336_MIPI_OUTPUT_W, SC2336_MIPI_OUTPUT_H},
    .isp_cap_size 			= 	{SC2336_MIPI_OUTPUT_W, SC2336_MIPI_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	SC2336_mipi_check,
        .init 		        = 	SC2336_mipi_init,
        .set_size_fps 		=	SC2336_mipi_set_output_size,
        .power_ctrl         =   SC2336_mipi_power_ctl,

        .get_ae_params  	=	SC2336_mipi_get_ae_params,
        .get_awb_params 	=	SC2336_mipi_get_awb_params,
        .get_iq_params 	    	=	SC2336_mipi_get_iq_params,

        .sleep 		        =	SC2336_mipi_sleep,
        .wakeup 		    =	SC2336_mipi_wakeup,
        .write_reg 		    =	SC2336_mipi_W_Reg,
        .read_reg 		    =	SC2336_mipi_R_Reg,

    }
};
