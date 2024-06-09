#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "gc2043.h"
#include "asm/isp_alg.h"

static u32 reset_gpio[2] = {-1, -1};


extern void *GC2043_get_ae_params();
extern void *gc2043_get_awb_params();
extern void *gc2043_get_iq_params();

extern void gc2043_ae_ev_init(u32 fps);

#define PCLK        48
#define ROW_TIME    59170

typedef struct {
    u8 addr;
    u8 value;
} Sensor_reg_ini;

Sensor_reg_ini GC2043_INI_REG[] = {
    //MCLK=24Hz,PCLK=48Mhz
    //Actual_window_size=1920*1080,frame_rate=15 fps
    //Pixel_line=2840,Line_frame=1127,row_time=59.17us
    /////////////////////////////////////////////////////
    //////////////////////   SYS   //////////////////////
    /////////////////////////////////////////////////////
    0xf2, 0x00,
    0xf3, 0x00,
    0xf6, 0x00,
    0xfc, 0x06,
    0xf7, 0x03,
    0xf8, 0x07,
    0xf9, 0x06,
    0xfa, 0x00,
    0xfc, 0x0e,
    /////////////////////////////////////////////////
    ////////////   ANALOG & CISCTL   ////////////////
    /////////////////////////////////////////////////
    0xfe, 0x00,
    0x03, 0x03,
    0x04, 0xf6,
    0x05, 0x02, //HB
    0x06, 0xc6,
    0x07, 0x00, //VB
    0x08, 0x17,
    0x09, 0x00,
    0x0a, 0x00, //row start
    0x0b, 0x00,
    0x0c, 0x00, //col start
    0x0d, 0x04,
    0x0e, 0x40, //height 1088
    0x0f, 0x07,
    0x10, 0x88, //width 1928
    0x12, 0xe2,
    0x17, 0x54,
    0x18, 0x02,
    0x19, 0x0d,
    0x1a, 0x18,
    0x1c, 0x6c,
    0x1d, 0x12,
    0x20, 0x54,
    0x21, 0x2c,
    0x23, 0xf0,
    0x24, 0xc1,
    0x25, 0x18,
    0x26, 0x64,
    0x28, 0x20,
    0x29, 0x08,
    0x2a, 0x08,
    0x2b, 0x48,
    0x2d, 0x1c,
    0x2f, 0x40,
    0x30, 0x99,
    0x34, 0x00,
    0x38, 0x80,
    0x3b, 0x12,
    0x3d, 0xb0,
    0xcc, 0x8a,
    0xcd, 0x99,
    0xcf, 0x70,
    0xd0, 0xcb,
    0xd2, 0xc1,
    0xd8, 0x80,
    0xda, 0x14,
    0xdc, 0x24,
    0xe1, 0x14,
    0xe3, 0xf0,
    0xe4, 0xfa,
    0xe6, 0x1f,
    0xe8, 0x02,
    0xe9, 0x02,
    0xea, 0x03,
    0xeb, 0x03,
    /////////////////////////////////////////////////
    //////////////////   ISP   //////////////////////
    /////////////////////////////////////////////////
    0xfe, 0x00,
    0x80, 0x4c,
    0x88, 0x23,
    0x89, 0x03,
    0x90, 0x01,
    0x92, 0x04, //2<= y <=6
    0x94, 0x04, //2<= x <=6
    0x95, 0x04, //crop win height
    0x96, 0x38,
    0x97, 0x07, //crop win width
    0x98, 0x80,
    /////////////////////////////////////////////////
    //////////////////   BLK   //////////////////////
    /////////////////////////////////////////////////
    0xfe, 0x00,
    0x40, 0x22,
    0x43, 0x07,
    0x4e, 0x3c,
    0x4f, 0x00,
    0x60, 0x00,
    0x61, 0x80,
    /////////////////////////////////////////////////
    //////////////////   GAIN   /////////////////////
    /////////////////////////////////////////////////
    0xfe, 0x00,
    0xb0, 0x58,
    0xb1, 0x01,
    0xb2, 0x00,
    0xb6, 0x00,
    0xfe, 0x01,
    0x01, 0x00,
    0x02, 0x01,
    0x03, 0x02,
    0x04, 0x03,
    0x05, 0x04,
    0x06, 0x05,
    0x07, 0x06,
    0x08, 0x0e,
    0x09, 0x16,
    0x0a, 0x1e,
    0x0b, 0x36,
    0x0c, 0x3e,
    0x0d, 0x56,
    0x0e, 0x5e,
    /////////////////////////////////////////////////
    //////////////////   DNDD   /////////////////////
    /////////////////////////////////////////////////
    0xfe, 0x02,
    0x81, 0x05,
    /////////////////////////////////////////////////
    //////////////////   dark sun   /////////////////
    /////////////////////////////////////////////////
    0xfe, 0x01,
    0x54, 0x77,
    0x58, 0x00,
    0x5a, 0x05,
    /////////////////////////////////////////////////
    //////////////////	 MIPI	/////////////////////
    /////////////////////////////////////////////////
    0xfe, 0x03,
    0x01, 0x00,
    0x02, 0x00,
    0x03, 0x00,
    0x06, 0x00,
    0x10, 0x00,
    0x15, 0x00,
    0x36, 0x83,
    0x8f, 0x64,
    /////////////////////////////////////////////////
    //////////////////   pad enable   ///////////////
    /////////////////////////////////////////////////
    0xfe, 0x00,
    0xf3, 0x01,
    0xfa, 0x00,
    0xf2, 0x0f,
};


static void *iic = NULL;


unsigned char wrGC2043Reg(unsigned char regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x6e)) {
        ret = 0;
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX, regID)) {
        ret = 0;
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        goto __wend;
    }
__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);

    return ret;

}

unsigned char rdGC2043Reg(unsigned char regID, unsigned char *regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x6e)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x6f)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat)) {
        ret = 0;
        goto __rend;
    }
__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;

}

unsigned char wrGC2043RegArray(u8 *array, u32 size)
{

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    dev_ioctl(iic, IIC_IOCTL_SET_NORMAT_RATE, 2);
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    dev_write(iic, array, size);
    return 1;

}


/*************************************************************************************************
    sensor api
*************************************************************************************************/



void GC2043_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;


    for (i = 0; i < sizeof(GC2043_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrGC2043Reg(GC2043_INI_REG[i].addr, GC2043_INI_REG[i].value);
    }

    return;
}


s32 GC2043_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    return 0;
}


s32 GC2043_power_ctl(u8 isp_dev, u8 is_work)
{

    return 0;
}

s32 GC2043_ID_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    u8 i ;

    for (i = 0; i < 3; i++) { //
        rdGC2043Reg(0xf0, &pid);
        rdGC2043Reg(0xf1, &ver);
    }

    log_d("Sensor PID: %x,%x\n", pid, ver);

    if (pid != 0x20 || ver != 0x33) {
        puts("----not GC2043-----\n");
        return -1;
    }
    puts("----hello GC2043-----\n");
    return 0;
}


static u8 cur_sensor_type = 0xff;

s32 GC2043_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio)
{
    puts("GC2043_check\n");
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
    printf("isp_dev =%d\n", isp_dev);

    reset_gpio[isp_dev] = _reset_gpio;

    puts("gc2043_id_check\n");
    if (0 != GC2043_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;

    return 0;
}

s32 GC2043_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("gc2043_init22\n");

    GC2043_config_SENSOR(width, height, format, frame_freq);


    return 0;
}

s32 gc2043_get_mipi_clk(u32 *mipi_clk,  u32 *tval_hstt, u32 *tval_stto, u16 width, u16 height, u8 frame_freq)
{
    *mipi_clk = 378;
    *tval_hstt = 0;
    *tval_stto = 30;
    return 0;
}


u32 gc2043_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 line = exp_time_us * 1000 / ROW_TIME;
    u32 line_time = (line * ROW_TIME / 1000);
    u32 ratio = (exp_time_us) * (1 << 10) / line_time;

    shutter->ae_exp_line =  line;
    shutter->ae_gain = (((gain * ratio) >> 10) & (~0xf));
    shutter->ae_exp_clk = 0;

    return 0;

}

#define GC2043_ANALOG_GAIN_0 1024 // 1.00x
#define GC2043_ANALOG_GAIN_1 1454 // 1.42x
#define GC2043_ANALOG_GAIN_2 2038 // 1.99x
#define GC2043_ANALOG_GAIN_3 2929 // 2.86x
#define GC2043_ANALOG_GAIN_4 4147 // 4.05x
#define GC2043_ANALOG_GAIN_5 5919 // 5.78x
#define GC2043_ANALOG_GAIN_6 8428 // 8.23x
#define GC2043_ANALOG_GAIN_7 12001 // 11.72x
#define GC2043_ANALOG_GAIN_8 16947 // 16.55x
#define GC2043_ANALOG_GAIN_9 23839 // 23.28x
#define GC2043_ANALOG_GAIN_10 34068 // 33.27x

#define  GC2043_DGAIN_PRECISE  64


static u32 gc2043_gain_steps[] = {
    GC2043_ANALOG_GAIN_0,
    GC2043_ANALOG_GAIN_1,
    GC2043_ANALOG_GAIN_2,
    GC2043_ANALOG_GAIN_3,
    GC2043_ANALOG_GAIN_4,
    GC2043_ANALOG_GAIN_5,
    GC2043_ANALOG_GAIN_6,
    GC2043_ANALOG_GAIN_7,
    GC2043_ANALOG_GAIN_8,
    GC2043_ANALOG_GAIN_9,
    GC2043_ANALOG_GAIN_10,
};

static void decompose_gain(int total_gain/*q10*/, u32 *a_gain, u32 *d_gain)
{
    int i = 0;
    int size = sizeof(gc2043_gain_steps) / sizeof(u32);

    for (i = 0; i < size; i++) {
        if (gc2043_gain_steps[i] > total_gain) {
            break;
        }
    }

    i = i - 1;
    if (i < 0) {
        i = 0;
    }

    *a_gain = i;
    *d_gain = GC2043_DGAIN_PRECISE * total_gain / gc2043_gain_steps[i];

}

static u8 gc2043_shutter_cmd[6][3] = {
    {0x6e, 0xb6, 0x00},  //again
    {0x6e, 0xb1, 0x00},  //dgainL
    {0x6e, 0xb2, 0x00},  //dgainH
    {0x6e, 0xb0, 0x00},  //ggaim
    {0x6e, 0x04, 0x00},  //exptime L
    {0x6e, 0x03, 0x00},   //exptime H
} ;


u32 gc2043_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 gain;
    u32 again, dgain;

    gain = (shutter->ae_gain);

    decompose_gain(gain, &again, &dgain);


    gc2043_shutter_cmd[0][2] = again ;
    gc2043_shutter_cmd[1][2] = dgain >> 6 ;

    gc2043_shutter_cmd[2][2] = (dgain << 2) & 0xfc;

    gc2043_shutter_cmd[3][2] = 0x58 ;

    gc2043_shutter_cmd[4][2] = shutter->ae_exp_line & 0xFF ;
    gc2043_shutter_cmd[5][2] = (shutter->ae_exp_line >> 8) & 0x1F;

    wrGC2043RegArray(gc2043_shutter_cmd[0], 18);



    return 0;
}

void GC2043_sleep()
{
    wrGC2043Reg(0xf6, 0x01);
}

void GC2043_wakeup()
{

    wrGC2043Reg(0xf6, 0x00);
}

void GC2043_W_Reg(u16 addr, u16 val)
{
    printf("update reg%x with %x\n", addr, val);
    wrGC2043Reg((u8)addr, (u8)val);
}

u16 GC2043_R_Reg(u16 addr)
{
    u8 val;
    rdGC2043Reg((u8)addr, &val);
    return val;
}


REGISTER_CAMERA(GC2043) = {
    .logo 				= 	"GC2023M",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_RGGB,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_PARALLEL,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_PCLK_SAMPLE_FALLING,
    .fps         		= 	30,

    .sen_size 			= 	{GC2043_OUTPUT_W, GC2043_OUTPUT_H},
    .isp_size 			= 	{GC2043_OUTPUT_W, GC2043_OUTPUT_H},

    .cap_fps         		= 	30,
    .sen_cap_size 			= 	{GC2043_OUTPUT_W, GC2043_OUTPUT_H},
    .isp_cap_size 			= 	{GC2043_OUTPUT_W, GC2043_OUTPUT_H},

    .rgbir_en         = 1,
    .rgbir_pattern      = RGBIR_PATTERN_4X4,
    .rgbir_phase        = RGBIR_PHASE_4X4_BGGI,

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	GC2043_check,
        .init 		        = 	GC2043_init,
        .set_size_fps 		=	GC2043_set_output_size,
        .get_mipi_clk       =   gc2043_get_mipi_clk,
        .power_ctrl         =   GC2043_power_ctl,

        .get_ae_params  	=	GC2043_get_ae_params,
        .get_awb_params 	=	gc2043_get_awb_params,
        .get_iq_params 	    =	gc2043_get_iq_params,

        .sleep 		        =	GC2043_sleep,
        .wakeup 		    =	GC2043_wakeup,
        .write_reg 		    =	GC2043_W_Reg,
        .read_reg 		    =	GC2043_R_Reg,

    }
};


