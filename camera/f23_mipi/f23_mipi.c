#ifndef F23_MIPI_C
#define  F23_MIPI_C


#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "f23_mipi.h"
#include "asm/isp_alg.h"


#define F23_SLAVE_ID    0x80

static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};

static u32 cur_expline = -1;
static u32 cur_again = -1;
static u32 cur_dgain = -1;
static u32 cur_expclk = -1;
static u32 cur_reg_idx = -1;
static u32 cur_vblank = -1;
static u32 next_reg_idx = -1;

static u8 cur_sensor_type = 0xff;

extern void *F23_mipi_get_ae_params();
extern void *F23_mipi_get_awb_params();
extern void *F23_mipi_get_iq_params();
extern void F23_mipi_ae_ev_init(u32 fps);

typedef struct {
    u8 addr;
    u8 value;
} Sensor_reg_ini;


/* struct stVrefh stVrefhTable[] = { */
/* {5, 263, 267, 0x22}, */
/* {4, 216, 220, 0x1c}, */
/* {3, 176, 180, 0x17}, */
/* {2, 130, 134, 0x12}, */
/* {1, 113, 117, 0x10} */
/* }; */

#define EXTERNAL_LDO_1V5 1

//如果系统VDDIO使用3.3V请定义此宏定义值为1
//如果系统VDDIO使用1.8V请定义此宏定义值为0
#define	VDDIO_VOLTAGE_3V3 0
#define DSC_ADDR_BASE 0x0400

static Sensor_reg_ini F23_MIPI_INI_REG[] = {
    0x0E, 0x11,
    0x0F, 0x14,
    0x10, 0x40,
    0x11, 0x80,
    0x48, 0x05,
    0x96, 0xAA,
    0x94, 0xC0,
    0x97, 0x8D,
    0x96, 0x00,
    0x12, 0x40,
    0x48, 0x8A,
    0x48, 0x0A,
    0x0E, 0x11,
    0x0F, 0x14,
    0x10, 0x23,
    0x11, 0x80,
    0x0D, 0xA0,
    0x5F, 0x41,
    0x60, 0x20,
    0x58, 0x12,
    0x57, 0x60,
    0x9D, 0x00,
    0x20, 0xDC,
    0x21, 0x04,
    0x22, 0x65,
    0x23, 0x04,
    0x24, 0xC0,
    0x25, 0x38,
    0x26, 0x43,
    0x27, 0x73,
    0x28, 0x19,
    0x29, 0x04,
    0x2C, 0x00,
    0x2D, 0x00,
    0x2E, 0x18,
    0x2F, 0x44,
    0x41, 0xC8,
    0x42, 0x13,
    0x46, 0x00,
    0x76, 0x60,
    0x77, 0x09,
    0x1D, 0x00,
    0x1E, 0x04,
    0x6C, 0x40,
    0x68, 0x00,
    0x6E, 0x2C,
    0x70, 0x6C,
    0x71, 0x6D,
    0x72, 0x6A,
    0x73, 0x36,
    0x74, 0x02,
    0x78, 0x9E,
    0x89, 0x01,
    0x2A, 0x64,
    0x2B, 0x24,
    0x31, 0x08,
    0x32, 0x4F,
    0x33, 0x20,
    0x34, 0x5E,
    0x35, 0x5E,
    0x3A, 0xAF,
    0x56, 0x32,
    0x59, 0xBF,
    0x5A, 0x04,
    0x85, 0x5A,
    0x8A, 0x04,
    0x8F, 0x90,
    0x91, 0x13,
    0x5B, 0xA0,
    0x5C, 0xF0,
    0x5D, 0xFC,
    0x5E, 0x1F,
    0x62, 0x04,
    0x63, 0x0F,
    0x64, 0xC0,
    0x66, 0x44,
    0x67, 0x73,
    0x69, 0x7C,
    0x6A, 0x28,
    0x7A, 0xC0,
    0x4A, 0x05,
    0x7E, 0xCD,
    0x49, 0x10,
    0x50, 0x02,
    0x7B, 0x4A,
    0x7C, 0x0C,
    0x7F, 0x57,
    0x90, 0x00,
    0x8E, 0x00,
    0x8C, 0xFF,
    0x8D, 0xC7,
    0x8B, 0x01,
    0x0C, 0x40,
    0x65, 0x02,
    0x80, 0x02,
    0x81, 0xC0,
    0x19, 0x20,
    0x12, 0x00,
    0x48, 0x8A,
    0x48, 0x0A,

};


static void *iic = NULL;

unsigned char wrF23_MIPIReg(unsigned char regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, F23_SLAVE_ID)) {
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

unsigned char rdF23_MIPIReg(unsigned char regID, unsigned char *regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, F23_SLAVE_ID)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, F23_SLAVE_ID + 1)) {
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


/*************************************************************************************************
 sensor api
 *************************************************************************************************/
void F23_MIPI_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u16 dsc_addr;

    for (i = 0; i < sizeof(F23_MIPI_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrF23_MIPIReg(F23_MIPI_INI_REG[i].addr, F23_MIPI_INI_REG[i].value);
    }


    /* dsc_addr = DSC_ADDR_BASE; */

    /* for (i = 0; i < 768; i++) { */
    /* wrF23_MIPIReg(dsc_addr + i, Tab_sensor_dsc[i]); */
    /* } */

    /* wrF23_MIPIReg(0x001d, 0x01); */

    /* *format = SEN_IN_FORMAT_GRBG; */

    /* F23_mipi_ae_ev_init(*frame_freq); */

    /* wrF23_MIPIReg(0x001d, 0x02); */

    return;
}

s32 F23_MIPI_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    return 0;
}

s32 F23_MIPI_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

void F23_MIPI_xclk_set(u8 isp_dev)
{

}

s32 F23_MIPI_ID_check(void)
{
    u8 PIDH = 0x00;
    u8 PIDL = 0x00;

    rdF23_MIPIReg(0x0A, &PIDH);
    rdF23_MIPIReg(0x0B, &PIDL);

    /*puts("Sensor ID \n");
    put_u8hex(PIDH);
    put_u8hex(PIDL);
    puts("\n");*/

    if (PIDH != 0x0F || PIDL != 0x23) {
        puts("----not F23_MIPI-----\n");
        return -1;
    }
    puts("----hello F23_MIPI-----\n");
    return 0;
}

void F23_MIPI_reset(u8 isp_dev)
{
    puts("F23_MIPI reset \n");

    u32 reset_gpio;
    u32 pwdn_gpio;

    if (isp_dev == ISP_DEV_0) {
        reset_gpio = reset_gpios[0];
        pwdn_gpio = pwdn_gpios[0];
    } else {
        reset_gpio = reset_gpios[1];
        pwdn_gpio = pwdn_gpios[1];
    }

    gpio_direction_output(pwdn_gpio, 0);
    gpio_direction_output(reset_gpio, 0);
    delay(40000);
    gpio_direction_output(reset_gpio, 1);
    delay(40000);

}

s32 F23_MIPI_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{
    printf("\n\n F23_MIPI_check reset pin :%d\n\n", reset_gpio);
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
    printf("\n\n isp_dev =%d\n\n", isp_dev);

    reset_gpios[isp_dev] = reset_gpio;
    pwdn_gpios[isp_dev] = pwdn_gpio;

    F23_MIPI_reset(isp_dev);

    if (0 != F23_MIPI_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;


    return 0;
}


s32 F23_MIPI_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    F23_MIPI_reset(isp_dev);
    puts("\n\n F23_MIPI_init \n\n");

    cur_expline = -1;
    cur_again = -1;
    cur_dgain = -1;
    cur_expclk = -1;
    cur_reg_idx = -1;
    cur_vblank = -1;
    next_reg_idx = -1;

    F23_MIPI_config_SENSOR(width, height, format, frame_freq);
    return 0;
}



static void set_again(u32 again)
{
    again &= 0xff;
    if (cur_again == again) {
        //return;
    }
    cur_again = again;
    // total gain = 2 ^ PGA[6:4] * (1 + PGA[3:0] / 16)
    F23_MIPI_W_Reg(0x00, (u8)again);
    return;
}

static void set_dgain(u32 dgain)
{
    dgain &= 0x03;
    if (cur_dgain == dgain) {
        return;
    }
    cur_dgain = dgain;
    //dgain = DVP2[1:0] 0x00:1x  0x01,0x10:2x  0x11:4x
    F23_MIPI_W_Reg(0x0D, 0xA0 | (u8)dgain);
}
static void cal_texp_mck(u32 exp_time, u32 *texp, u32 *mck)
{
    u32 tmp1;
    u32 tmp2;
//line_time_sec  =  1/(30fps * frame_len)    //frame_len  = 1660
//line_time_us  =  1/(30fps * frame_len)/(1000*1000)
//texp = exp_time/(line_time_us)
//texp = exp_time*30*1660/1000*1000
//
//30FPS!
    tmp1 = exp_time * 100 / 2963;
    if (tmp1 < 1) {
        tmp1 = 1;
    }
    *texp = tmp1;

    tmp1 = tmp1 * 2963 / 100;
    tmp2 = exp_time * (1 << 10) / tmp1;
    if (tmp2 < 1024) {
        tmp2 = 1024;
    }
    *mck = tmp2;
    return;
}

//q10
static void calc_gain(u32 gain, u32 *_again, u32 *_dgain)
{
    u8 PGA6_4 = 0;
    u8 PGA3_0 = 0;
    u32 _gain;
    u32 tmp;
    if (gain > 248 * 1024) {
        gain = 248 * 1024;
    }
    if (gain < 1024) {
        gain = 1024;
    }
    _gain = (gain * 16) >> 10;
    tmp = _gain / 32;
    while (tmp) {
        tmp /= 2;
        PGA6_4 ++;
    }
    PGA3_0 = (_gain >> PGA6_4) - 16;
    *_again = (PGA6_4 << 4) | PGA3_0;
    *_dgain = 0;
}

static void set_shutter(u32 texp, u32 texp_mck)
{
    F23_MIPI_W_Reg(0x01, (u8)(texp));
    F23_MIPI_W_Reg(0x02, (u8)(texp >> 8));
}


u32 F23_mipi_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;
    u32 mck;

    cal_texp_mck(exp_time_us, &texp, &mck);

    //printf(" \n exp_time_us %d\n", exp_time_us); // ppk added on 20170609

    shutter->ae_exp_line =  texp;
    shutter->ae_gain = gain * mck / 1024;
    shutter->ae_exp_clk = 0;//mck;


    return 0;

}



u32 F23_mipi_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 again, dgain;
    calc_gain(shutter->ae_gain, &again, &dgain);

    set_shutter(shutter->ae_exp_line, shutter->ae_exp_clk);
    set_again(again);
    //set_dgain(dgain);


    return 0;
}


void F23_MIPI_sleep()
{

}

void F23_MIPI_wakeup()
{

}

void F23_MIPI_W_Reg(u16 addr, u16 val)
{
    wrF23_MIPIReg((u16) addr, (u8) val);
}

u16 F23_MIPI_R_Reg(u16 addr)
{
    u8 val;
    rdF23_MIPIReg((u16) addr, &val);
    return val;
}



REGISTER_CAMERA(F23_MIPI) = {
    .logo 				= 	"F23M",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_BGGR,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_CSI2,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_CSI2_2_LANE,
    .fps         		= 	30, // 30,

    .sen_size 			= 	{F23_MIPI_OUTPUT_W, F23_MIPI_OUTPUT_H},
    .isp_size 			= 	{F23_MIPI_OUTPUT_W, F23_MIPI_OUTPUT_H},

    .cap_fps         		= 	30,
    .sen_cap_size 			= 	{F23_MIPI_OUTPUT_W, F23_MIPI_OUTPUT_H},
    .isp_cap_size 			= 	{F23_MIPI_OUTPUT_W, F23_MIPI_OUTPUT_H},

#if F23_RGBIR_EN
    .rgbir_en         = 1,
    .rgbir_pattern      = RGBIR_PATTERN_4X4,
    .rgbir_phase        = RGBIR_PHASE_4X4_BGGI,
#endif

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	F23_MIPI_check,
        .init 		        = 	F23_MIPI_init,
        .set_size_fps 		=	F23_MIPI_set_output_size,
        .power_ctrl         =   F23_MIPI_power_ctl,

        .get_ae_params 	    =	F23_mipi_get_ae_params,
        .get_awb_params 	=	F23_mipi_get_awb_params,
        .get_iq_params 	    =	F23_mipi_get_iq_params,

        .sleep 		        =	F23_MIPI_sleep,
        .wakeup 		    =	F23_MIPI_wakeup,
        .write_reg 		    =	F23_MIPI_W_Reg,
        .read_reg 		    =	F23_MIPI_R_Reg,

    }
};


#endif

