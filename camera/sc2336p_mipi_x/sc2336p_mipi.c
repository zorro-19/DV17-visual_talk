
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "sc2336p_mipi.h"
#include "asm/isp_alg.h"

static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};

extern void *sc2336p_mipi_get_ae_params();
extern void *sc2336p_mipi_get_awb_params();
extern void *sc2336p_mipi_get_iq_params();
extern void sc2336p_mipi_ae_ev_init(u32 fps);

#define INPUT_CLK  24
#define PCLK    79.2    //1200*2200*30

#define LINE_LENGTH_CLK     0x898   //2200
#define FRAME_LENGTH30      1200
#define FRAME_LENGTH25      1440
#define FRAME_LENGTH27      1333
#define FRAME_LENGTH15      2400
#define FRAME_LENGTH12      2880

static u32 cur_again = -1;
static u32 cur_dgain = -1;
static u32 cur_expline = -1;
static u32 cur_line_length_clk = -1;
static u32 cur_frame_length_clk = -1;

static u32 line_length_clk = LINE_LENGTH_CLK;
static u32 frame_length_clk = FRAME_LENGTH25;

static void *iic = NULL;
static u8 WRCMD = 0x60;
static u8 RDCMD = 0x61;

typedef struct {
    u16 addr;
    u8 value;
} sensor_reg_ini;

sensor_reg_ini sc2336p_mipi_ini_reg[] =
{
    //cleaned_0x09_SC2336P_MIPI_24Minput_2Lane_396Mbps_10bit_1920x1080_30fps_内供.ini
                0x0103,0x01,
                0x0100,0x00,
                0x36e9,0x80,
                0x37f9,0x80,
                0x301f,0x09,
                0x3106,0x05,
                0x320e,0x05,    //0x04
                0x320f,0xa0,    //0xb0
                0x3221,0x66,  // 0x00 mirror&flip
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

unsigned char wr_sc2336p_mipi_reg(u16 regID, unsigned char regDat)
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

unsigned char rd_sc2336p_mipi_reg(u16 regID, unsigned char *regDat)
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


void sensor_sc2336p_flip_rotate( u8 state){

   printf("\n state================%d,%s\n",state,__FUNCTION__);
    if(!iic){

     printf( "\n not this sensor ");
      return ;
    }
    switch(state)
    {

    case 1:

      wr_sc2336p_mipi_reg(0x3221,0x66);// 0000  0000

    break ;

    case 2:
       wr_sc2336p_mipi_reg(0x3221,0x60);// 0000  0000

    break ;

    case 3:

         wr_sc2336p_mipi_reg(0x3221,0x06);// 0000  0000
    break ;

    case 4:


  // 0x3221,0x66,  // 0x00 mirror&flip

     wr_sc2336p_mipi_reg(0x3221,0x00);// 0000  0000

    break ;


    default:
        break ;

    }



}

void sc2336p_mipi_config_sensor(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u8 v;

    sc2336p_mipi_set_output_size(width, height, frame_freq);

    for (i = 0; i < sizeof(sc2336p_mipi_ini_reg) / sizeof(sensor_reg_ini); i++) {
        wr_sc2336p_mipi_reg(sc2336p_mipi_ini_reg[i].addr, sc2336p_mipi_ini_reg[i].value);
    }
     u8 state = db_select("flip");
    sensor_sc2336p_flip_rotate( state);

    sc2336p_mipi_ae_ev_init(*frame_freq);

    *format = SEN_IN_FORMAT_BGGR;

    return;
}


s32 sc2336p_mipi_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    *width = SC2336P_MIPI_OUTPUT_W;
    *height = SC2336P_MIPI_OUTPUT_H;

    return 0;
}


s32 sc2336p_mipi_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

s32 sc2336p_mipi_id_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    //u8 exid = 0x00;
    u8 i ;

    u8 retry = 0;

    while(retry <= 5)
    {
        wr_sc2336p_mipi_reg(0x301a,0xf8);
        wr_sc2336p_mipi_reg(0x0100,0x01);
       // delay(200000); //  delay 1ms
        delay(1000000); // delay 5ms
        delay(1000000); // delay 5ms
        delay(1000000); // delay 5ms
        rd_sc2336p_mipi_reg(0x3107, &pid);
        rd_sc2336p_mipi_reg(0x3108, &ver);

        printf("%d,%s %d 0x%x 0x%x \n",__LINE__, __FUNCTION__, retry, pid, ver);

        if (pid == 0x9b && ver == 0x3a ) {

            puts("\n----hello SC2336p_mipi-----\n");
            return 0;
        }
        retry++;
        delay(500);
    }
    puts("\n----not SC2336p_mipi-----\n");

    return -1;
}

void sc2336p_mipi_reset(u8 isp_dev)
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
    delay(40000);
    puts("2053_MIPI IO_PORTA_14 1 \n");
    gpio_direction_output(IO_PORTA_14, 1);
    delay(40000);
}

void sc2336p_mipi_change_fps(u8 sence, u16 lv)
{
    static s8 curSence = -1;

    //printf("%s: %d %d\n", __FUNCTION__, sence, lv);
}


static u8 cur_sensor_type = 0xff;

s32 sc2336p_mipi_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{

    printf("\n\n sc2336p_mipi_check:%d \n\n",iic);

  /// return -1;

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
    printf("\n\n isp_dev =%d\n\n", isp_dev);

    reset_gpios[isp_dev] = reset_gpio;
    pwdn_gpios[isp_dev] = pwdn_gpio;

    // ===============================
    //DVDD 1.2
    //gpio_direction_output(IO_PORTG_13, 0);
    //2.8 AVDD
    //avdd28_ctrl(AVDD28_28, true);

    //sc2336p_mipi_reset(isp_dev);
    if (0 != sc2336p_mipi_id_check()) {
        dev_close(iic);
        iic = NULL;
        printf("\n no id \n ");
        return -1;
    }
    // ================================

    cur_sensor_type = isp_dev;

    return 0;
}



#if 1  //不是2336P
s32 SC2336_mipi_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{
	//if(!mipi_SC2336)
	//	return -1;

    puts("\n\n SC2336_mipi_check \n\n");
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

    printf("\n\n isp_dev =%d\n\n", isp_dev);

    reset_gpios[isp_dev] = reset_gpio;
    pwdn_gpios[isp_dev] = pwdn_gpio;

	//SC2336_mipi_reset(isp_dev);
    if (0 != SC2336_mipi_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }
	// ================================

    cur_sensor_type = isp_dev;

    return 0;
}


s32 SC2336_mipi_ID_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    u8 exid = 0x00;
    u8 i ;

	u8 retry = 0;
	while(retry <= 5)
	{
        rd_sc2336p_mipi_reg(0x3107, &pid);  //
        rd_sc2336p_mipi_reg(0x3108, &ver);
		rd_sc2336p_mipi_reg(0x3109, &exid);

		printf("%s %d 0x%x 0x%x 0x%x\n", __FUNCTION__, retry, pid, ver, exid);

		if (pid == 0x9b && ver == 0x3a) {

			puts("\n----hello SC2336_mipi-----\n");
			return 0;
		}
		retry++;
		delay(500);
	}
	puts("\n----not SC2336_mipi-----\n");
	return -1;
}

#endif


s32 sc2336p_mipi_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n sc2336p_mipi_init22 \n\n");

    sc2336p_mipi_config_sensor(width, height, format, frame_freq);

    return 0;
}

static void set_again(u32 again)
{
    if (cur_again == again)
    {
        return;
    }

    wr_sc2336p_mipi_reg(0x3e09, again & 0xff);
    cur_again = again;
    return;
}

static void set_dgain(u32 dgain)
{
    if (cur_dgain == dgain)
    {
        return;
    }

    wr_sc2336p_mipi_reg(0x3e06, ((dgain>>8)&0xff));
    wr_sc2336p_mipi_reg(0x3e07, (dgain&0xff));
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

    if(gain < 1024) gain = 1024;
    if(gain > 64*1024) gain = 64*1024;

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
  //  printf("xzpin again = %x dgain = %x\n",*_again,*_dgain);
}


static void set_shutter(u32 texp)
{
 //   printf("xzpin cur_expline = %d, texp = %d\n", cur_expline, texp);
    if (cur_expline == texp)
    {
        return;
    }
    cur_expline  = texp;
    wr_sc2336p_mipi_reg(0x3e00, (texp >> 12)& 0xff);
    wr_sc2336p_mipi_reg(0x3e01, (texp >> 4)& 0xff);
    wr_sc2336p_mipi_reg(0x3e02, (texp << 4) & 0xf0);
    //delay_2ms(15);  //95-105
}


u32 sc2336p_mipi_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;
    u32 texp_align;
    u32 ratio;

    if (frame_length_clk != cur_frame_length_clk) {
        cur_frame_length_clk = frame_length_clk;
        wr_sc2336p_mipi_reg(0x320e, frame_length_clk >> 8);
        wr_sc2336p_mipi_reg(0x320f, frame_length_clk & 0xff);
    }

    if (exp_time_us <= 40000) {
        frame_length_clk = FRAME_LENGTH25;

    } else {
        frame_length_clk = FRAME_LENGTH15;
    }

    texp = exp_time_us * PCLK / LINE_LENGTH_CLK;

    if (texp < 1) {
        texp = 1;
    }

    if (texp > frame_length_clk  - 6) {
        texp = frame_length_clk  - 6;
    }
    texp_align = (texp) * LINE_LENGTH_CLK / (PCLK);

    if (texp_align < exp_time_us) {
        ratio = (exp_time_us) * (1 << 10) / texp_align;
        //printf("ratio = %d\n",ratio);
    } else {
        ratio = (1 << 10);
    }

    shutter->ae_exp_line =  texp;
    shutter->ae_gain = (gain * ratio) >> 10;
    shutter->ae_exp_clk = 0;

 //   printf("xzpin exp_time_us=%d, texp=%d, gain=%d->%d  gain_val = %d\n", exp_time_us, texp, gain,shutter->ae_gain,gain>>10);
    return 0;

}
static void set_log()
{

}

u32 sc2336p_mipi_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 again, dgain;

    calc_gain((shutter->ae_gain), &again, &dgain);
    set_shutter(shutter->ae_exp_line);
    set_again(again);
    set_dgain(dgain);

    return 0;
}

void sc2336p_mipi_sleep()
{


}

void sc2336p_mipi_wakeup()
{


}

void sc2336p_mipi_w_reg(u16 addr, u16 val)
{
    /*printf("update reg%x with %x\n", addr, val);*/
    wr_sc2336p_mipi_reg((u16)addr, (u8)val);
}

u16 sc2336p_mipi_r_reg(u16 addr)
{
    u8 val;
    rd_sc2336p_mipi_reg((u16)addr, &val);
    //printf("read reg%x val %x\n", addr, val);
    return val;
}

REGISTER_CAMERA(sc2336p_mipi) = {
    .logo               =   "sc2336p",
    .isp_dev            =   ISP_DEV_NONE,
    .in_format          =   SEN_IN_FORMAT_BGGR,
    .out_format         =   ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_CSI2,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_CSI2_2_LANE,
    .fps                =   25,

    .sen_size           =   {SC2336P_MIPI_OUTPUT_W, SC2336P_MIPI_OUTPUT_H},
    .isp_size           =   {SC2336P_MIPI_OUTPUT_W, SC2336P_MIPI_OUTPUT_H},

    .cap_fps                =   25,
    .sen_cap_size           =   {SC2336P_MIPI_OUTPUT_W, SC2336P_MIPI_OUTPUT_H},
    .isp_cap_size           =   {SC2336P_MIPI_OUTPUT_W, SC2336P_MIPI_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check       =   sc2336p_mipi_check,
        .init               =   sc2336p_mipi_init,
        .set_size_fps       =   sc2336p_mipi_set_output_size,
        .power_ctrl         =   sc2336p_mipi_power_ctl,

        .get_ae_params      =   sc2336p_mipi_get_ae_params,
        .get_awb_params     =   sc2336p_mipi_get_awb_params,
        .get_iq_params      =   sc2336p_mipi_get_iq_params,

        .sleep              =   sc2336p_mipi_sleep,
        .wakeup             =   sc2336p_mipi_wakeup,
        .write_reg          =   sc2336p_mipi_w_reg,
        .read_reg           =   sc2336p_mipi_r_reg,
    }
};
