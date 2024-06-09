#ifndef BG0806_MIPI_C
#define  BG0806_MIPI_C


#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "bg0806_mipi.h"
#include "asm/isp_alg.h"

#define ROWTIME         0x8ae
#define VBLANK_30FPS    0x27 // 0x25
#define VBLANK_25FPS    0xfe
#define VBLANK_20FPS    0x250
#define VBLANK_16FPS    0x3f5
#define VBLANK_15FPS    0x482

#define MAX_EXP_LINE_30FPS  1131
#define MAX_EXP_LINE_25FPS  1348
#define MAX_EXP_LINE_20FPS  1685
#define MAX_EXP_LINE_16FPS  2107
#define MAX_EXP_LINE_15FPS  (2250-2)

#define INPUT_CLK       24

#define PCLK            75000

#define ROW_TIME_NS     29630


//PCLK   ((INPUT_CLK) * (PLM+2)/((PLN+2)*(2+pclkdiv)))
//Tfrm = ROWTIME*(8+BG0806_MIPI_OUTPUT_H+VBLANK)/PCLK;
//Texp = (TEXP * ROWTIME + TEXP_MCK) /PCLK



static u32 cur_expline = -1;
static u32 cur_again = -1;
static u32 cur_dgain = -1;
static u32 cur_expclk = -1;
static u32 cur_reg_idx = -1;
static u32 cur_vblank = -1;
static u32 next_reg_idx = -1;

//dsc_k blcc_k different with chip pid
#define BG0806A		0x01
#define BG0806C1	0x07
#define BG0806C2	0x0b
#define BG0806B1	0x03
#define BG0806B2	0x0f


#define  FRAME_RATE  30
static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};



#if (FRAME_RATE == 30)
#define GVBLANK      VBLANK_30FPS
#define GEXPLINEMAX  MAX_EXP_LINE_30FPS
#elif (FRAME_RATE == 15)
#define GVBLANK      VBLANK_15FPS
#define GEXPLINEMAX  MAX_EXP_LINE_15FPS
#else
#define GVBLANK      VBLANK_30FPS
#define GEXPLINEMAX  MAX_EXP_LINE_30FPS
#error FRAME_RATE ERR
#endif
static u32 gRowTime = ROWTIME;
static u32 gVblank = GVBLANK;// VBLANK_30FPS;
static u32 gExplineMax = GEXPLINEMAX;

static u8 cur_sensor_type = 0xff;

static u8 vrefh_min_tlb = 0x0c;
static u8 pre_index = 0;

//static 	u8        reg_0x2b = 0x30;// REMOVE PPK 20170623
static 	u8        reg_0x30 = 0x00;
static 	u8        reg_0x31 = 0xC0; // ADDED PPK 20170623
static 	u8        reg_0x34 = 0x00;
static 	u8        reg_0x35 = 0xC0;// ADDED PPK 20170623
static 	u8        reg_0x4d = 0x00;
static 	u8        reg_0x4f = 0x09;
static 	u8        reg_0x61 = 0x04;
static 	u8        reg_0x67 = 0x01;
static 	u8        reg_0x68 = 0x90;

extern void *bg0806_mipi_get_ae_params();
extern void *bg0806_mipi_get_awb_params();
extern void *bg0806_mipi_get_iq_params();
extern void bg0806_mipi_ae_ev_init(u32 fps);

typedef struct {
    u16 addr;
    u8 value;
} Sensor_reg_ini;

struct stVrefh {
    u16 index;
    u16 th_low;
    u16 th_high;
    u8 vrefh;
};

struct stVrefh stVrefhTable[] = {
    {5, 263, 267, 0x22},
    {4, 216, 220, 0x1c},
    {3, 176, 180, 0x17},
    {2, 130, 134, 0x12},
    {1, 113, 117, 0x10}
};

#define EXTERNAL_LDO_1V5 1

//如果系统VDDIO使用3.3V请定义此宏定义值为1
//如果系统VDDIO使用1.8V请定义此宏定义值为0
#define	VDDIO_VOLTAGE_3V3 0
#define DSC_ADDR_BASE 0x0400

static Sensor_reg_ini BG0806_MIPI_INI_REG[] = {
    //MCLK=24M，PCLK=75M，10bit，1928x1088,30fps
    0x0200, 0x0001,
#if (BG0806_MIPI_OUTPUT_W == 1920)
    0x0003, 0x0027,

    0x0006, 0x0007,
    0x0007, 0x0080,
    0x0008, 0x0004,
#else
    0x0002, 0x0001,  //  hstart 279
    0x0003, 0x0017,

    0x0006, 0x0005,  // 1440
    0x0007, 0x00a0,
    0x0008, 0x0004,  // 1088
    /* 0x0009, 0x0040, */
#endif

    0x000e, 0x0008,//0806_4times_75M_30fps
    0x000f, 0x00ae,//row time F_W=2222
    0x0013, 0x0001,
#if 1
    0x0020, 0x03,
    0x001d, 0x02,
#endif

#if (FRAME_RATE == 30)
    0x0021, 0x0000,
    0x0022, 0x0027,//vblank // 0x0025 // 0x0027 for 30fps // 0x00fe for 25fps //0x250 for 20fps // 0x482 for 15fps
#elif(FRAME_RATE == 15)
    0x0021, 0x0004,
    0x0022, 0x0082,//vblank // 0x0025 // 0x0027 for 30fps // 0x00fe for 25fps //0x250 for 20fps // 0x482 for 15fps
#endif


    0x0028, 0x0000,
    0x0029, 0x0030,
    0x002a, 0x0000,
    0x002b, 0x0050, // 0x30 // change PPK 20170623
    0x0030, 0x0000,
    0x0031, 0x00c0,//rstb2rmp1 gap //0x00d0// change PPK 20170623
    0x0034, 0x0000,
    0x0035, 0x00c0,//tx2rmp2 gap //0x00d0// change PPK 20170623
    0x003c, 0x0001,
    0x003d, 0x0084,//rmp1_w@pclk domain
    0x003e, 0x0005,//ncp
    0x0042, 0x0001,
#if 0//VDDIO_VOLTAGE_3V3
    0x005c, 0x07, //lsh_io ctrlbit for 3.3VDDIO
#else
    0x005c, 0x00, //lsh_io ctrlbit for 1.8VDDIO
#endif
    0x0061, 0x0004,
    0x0062, 0x005c,//rmp2_w@pclk domain
    0x0064, 0x0000,
    0x0065, 0x0080,//rmp1_w@rmpclk domain
    0x0067, 0x0001,
    0x0068, 0x0090,//rmp2_w@rmpclk domain
    0x006c, 0x0003,//pd mipi dphy&dphy ldo
    0x007f, 0x0003,
    0x0080, 0x0001, //dot en disable
    0x0081, 0x0000,
    0x0082, 0x000b,
    0x0084, 0x0008,
    0x0088, 0x0005,//pclk dly
    0x008e, 0x0000,
    0x008f, 0x0000,
    0x0090, 0x0001,
    0x0094, 0x0001,//rmp div1
    0x0095, 0x0001,//rmp div4
    0x009e, 0x0003,//4 times
    0x009f, 0x0020,//rmp2gap@rmpclk
    0x00b1, 0x007f,
    0x00b2, 0x0002,
    0x00bc, 0x0002,
    0x00bd, 0x0000,
    0x0120, 0x0001,//blc on 01 direct
    0x0132, 0x0001,//k
    0x0133, 0x0060,
    0x0139, 0x0007,
    0x0139, 0x00ff,
    0x013b, 0x0008,
    0x01a5, 0x0007,//row noise on 07
    0x0160, 0x0000,
    0x0161, 0x0030,
    0x0162, 0x0000,
    0x0163, 0x0030,
    0x0164, 0x0000,
    0x0165, 0x0030,
    0x0166, 0x0000,
    0x0167, 0x0030,
    0x0206, 0x0000,
    0x0207, 0x0000,
    0x0053, 0x0000,
    0x0054, 0x0028,//plm_cnt
    0x0055, 0x0000,//pln_cnt

    0x00f3, 0x0000,
    0x00f4, 0x007b,//plm
    0x00f5, 0x0006,//pln,371.25M pll

    0x006d, 0x0003,//pclk_ctrl, 03 for 1p25,0c for 1p5
    0x006c, 0x0000,//ldo_ctrl,00
    0x008d, 0x003f,//
    0x008e, 0x000c,//oen_ctrl,0c
    0x008f, 0x0003,//io_sel_ctrl,03
    0x00fa, 0x00cf,//ispc,c7
    0x0391, 0x0000,//mipi_ctrl1,(raw10)
    0x0392, 0x0000,//mipi_ctrl2,default
    0x0393, 0x0001,//mipi_ctrl3,default
    0x0398, 0x000a,//28(25M),14(50M),0a(100M),08(111.375)
    0x0390, 0x0006,//mipi_ctrl0,bit[1],mipi enable

    0x001d, 0x0001
};



const u8 Tab_sensor_dsc[768] = {
    //20170206 update dsc	external ldo
    0x0a, 0x5c, 0x09, 0x4a, 0x08, 0x07, 0x06, 0xee, 0x06, 0x23, 0x05, 0x7e, 0x05, 0x21, 0x05, 0x01, 0x05, 0xcc, 0x06, 0xf5,
    0x08, 0x0b, 0x09, 0x0e, 0x0a, 0x0e, 0x0b, 0x34, 0x0c, 0x42, 0x0e, 0x9b, 0x09, 0xe0, 0x08, 0x71, 0x06, 0xce, 0x05, 0xd7,
    0x05, 0x19, 0x04, 0x9c, 0x04, 0x54, 0x04, 0x4d, 0x04, 0xd2, 0x05, 0xf8, 0x07, 0x04, 0x07, 0xe3, 0x08, 0xe2, 0x09, 0xfe,
    0x0b, 0x69, 0x0d, 0xfa, 0x09, 0x62, 0x07, 0x83, 0x05, 0xd8, 0x04, 0xe6, 0x04, 0x44, 0x03, 0xe5, 0x03, 0x82, 0x03, 0x88,
    0x04, 0x07, 0x05, 0x09, 0x05, 0xfe, 0x06, 0xc5, 0x07, 0xbf, 0x08, 0xe2, 0x0a, 0x68, 0x0d, 0x59, 0x08, 0xf7, 0x06, 0xea,
    0x05, 0x43, 0x04, 0x55, 0x03, 0xbf, 0x03, 0x43, 0x03, 0x09, 0x02, 0xf7, 0x03, 0x83, 0x04, 0x75, 0x05, 0x5c, 0x06, 0x09,
    0x06, 0xf5, 0x08, 0x1c, 0x09, 0xa6, 0x0c, 0xc6, 0x08, 0x6a, 0x06, 0x5e, 0x04, 0xa9, 0x03, 0xc7, 0x03, 0x36, 0x02, 0xd1,
    0x02, 0x99, 0x02, 0x91, 0x02, 0xea, 0x03, 0xdb, 0x04, 0xad, 0x05, 0x4f, 0x06, 0x48, 0x07, 0x62, 0x08, 0xf4, 0x0b, 0x71,
    0x07, 0xbd, 0x05, 0xd0, 0x04, 0x2a, 0x03, 0x43, 0x02, 0xa1, 0x02, 0x4d, 0x02, 0x08, 0x01, 0xf8, 0x02, 0x58, 0x03, 0x3f,
    0x03, 0xec, 0x04, 0xa1, 0x05, 0x87, 0x06, 0xa5, 0x08, 0x31, 0x0a, 0xc0, 0x07, 0x5a, 0x05, 0x54, 0x03, 0xa0, 0x02, 0xca,
    0x02, 0x4a, 0x01, 0xd9, 0x01, 0x9c, 0x01, 0x98, 0x01, 0xf0, 0x02, 0xba, 0x03, 0x70, 0x04, 0x02, 0x04, 0xe5, 0x06, 0x0b,
    0x07, 0x84, 0x0a, 0x2f, 0x06, 0xdf, 0x04, 0xe4, 0x03, 0x3f, 0x02, 0x6d, 0x01, 0xe9, 0x01, 0x84, 0x01, 0x47, 0x01, 0x3f,
    0x01, 0x8e, 0x02, 0x31, 0x02, 0xea, 0x03, 0x78, 0x04, 0x5e, 0x05, 0x68, 0x06, 0xea, 0x09, 0x27, 0x06, 0x82, 0x04, 0x81,
    0x02, 0xe8, 0x02, 0x20, 0x01, 0x9c, 0x01, 0x2c, 0x00, 0xfa, 0x00, 0xea, 0x01, 0x37, 0x01, 0xdc, 0x02, 0x76, 0x02, 0xf6,
    0x03, 0xd5, 0x04, 0xd7, 0x06, 0x49, 0x08, 0xb0, 0x06, 0x4b, 0x04, 0x42, 0x02, 0xaf, 0x01, 0xde, 0x01, 0x4f, 0x00, 0xe7,
    0x00, 0xa2, 0x00, 0x92, 0x00, 0xb6, 0x01, 0x5a, 0x01, 0xe4, 0x02, 0x7e, 0x03, 0x3b, 0x04, 0x31, 0x05, 0x88, 0x07, 0xb8,
    0x06, 0x25, 0x04, 0x0d, 0x02, 0x7a, 0x01, 0xa1, 0x01, 0x1b, 0x00, 0xab, 0x00, 0x5e, 0x00, 0x4e, 0x00, 0x68, 0x00, 0xf2,
    0x01, 0x62, 0x01, 0xec, 0x02, 0x9e, 0x03, 0x8f, 0x04, 0xe3, 0x07, 0x01, 0x06, 0x0e, 0x04, 0x0d, 0x02, 0x66, 0x01, 0x94,
    0x01, 0x10, 0x00, 0x99, 0x00, 0x44, 0x00, 0x21, 0x00, 0x34, 0x00, 0x97, 0x01, 0x13, 0x01, 0x94, 0x02, 0x49, 0x03, 0x40,
    0x04, 0x75, 0x06, 0x92, 0x06, 0x54, 0x04, 0x31, 0x02, 0x96, 0x01, 0xc5, 0x01, 0x18, 0x00, 0x99, 0x00, 0x39, 0x00, 0x21,
    0x00, 0x21, 0x00, 0x65, 0x00, 0xd6, 0x01, 0x5f, 0x02, 0x0c, 0x02, 0xfa, 0x04, 0x31, 0x06, 0x5b, 0x06, 0x5c, 0x04, 0x41,
    0x02, 0x9e, 0x01, 0xc5, 0x01, 0x18, 0x00, 0x99, 0x00, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x59, 0x00, 0xc6, 0x01, 0x4b,
    0x01, 0xf9, 0x02, 0xf3, 0x04, 0x1d, 0x06, 0x37, 0x06, 0x0e, 0x04, 0x1a, 0x02, 0x92, 0x01, 0xbd, 0x01, 0x18, 0x00, 0xa1,
    0x00, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x59, 0x00, 0xce, 0x01, 0x4b, 0x02, 0x14, 0x02, 0xfa, 0x04, 0x25, 0x06, 0x3f,
    0x05, 0x82, 0x03, 0xf2, 0x02, 0x87, 0x01, 0xa0, 0x01, 0x28, 0x00, 0xaa, 0x00, 0x62, 0x00, 0x31, 0x00, 0x3c, 0x00, 0x89,
    0x00, 0xfe, 0x01, 0x7b, 0x02, 0x3a, 0x03, 0x26, 0x04, 0x69, 0x06, 0x82, 0x05, 0xb3, 0x04, 0x02, 0x02, 0x97, 0x01, 0xd2,
    0x01, 0x43, 0x00, 0xdb, 0x00, 0x99, 0x00, 0x5d, 0x00, 0x75, 0x00, 0xd6, 0x01, 0x4b, 0x01, 0xc7, 0x02, 0x71, 0x03, 0x73,
    0x04, 0xc7, 0x06, 0xe2, 0x05, 0xcf, 0x04, 0x3c, 0x02, 0xe4, 0x02, 0x1f, 0x01, 0x90, 0x01, 0x30, 0x00, 0xe6, 0x00, 0xcb,
    0x00, 0xd6, 0x01, 0x2b, 0x01, 0xb5, 0x02, 0x25, 0x02, 0xf3, 0x03, 0xe4, 0x05, 0x42, 0x07, 0xb2, 0x05, 0xe0, 0x04, 0x83,
    0x03, 0x22, 0x02, 0x67, 0x01, 0xed, 0x01, 0x7d, 0x01, 0x3f, 0x01, 0x14, 0x01, 0x27, 0x01, 0x93, 0x02, 0x10, 0x02, 0x8d,
    0x03, 0x4c, 0x04, 0x3f, 0x05, 0x9c, 0x07, 0xc1, 0x06, 0x1d, 0x04, 0xce, 0x03, 0x7d, 0x02, 0xb5, 0x02, 0x42, 0x01, 0xd1,
    0x01, 0x85, 0x01, 0x74, 0x01, 0x88, 0x01, 0xe8, 0x02, 0x71, 0x02, 0xe1, 0x03, 0xc0, 0x04, 0xb6, 0x06, 0x04, 0x08, 0x7f,
    0x06, 0x40, 0x05, 0x10, 0x03, 0xe0, 0x03, 0x1b, 0x02, 0xab, 0x02, 0x3a, 0x01, 0xe4, 0x01, 0xd4, 0x01, 0xdc, 0x02, 0x50,
    0x02, 0xce, 0x03, 0x60, 0x04, 0x29, 0x05, 0x32, 0x06, 0x88, 0x09, 0x16, 0x06, 0x63, 0x05, 0x8c, 0x04, 0x66, 0x03, 0xbc,
    0x03, 0x1b, 0x02, 0xc2, 0x02, 0x79, 0x02, 0x41, 0x02, 0x69, 0x02, 0xde, 0x03, 0x52, 0x03, 0xdf, 0x04, 0xc9, 0x05, 0xc8,
    0x06, 0xfd, 0x09, 0x46, 0x06, 0x88, 0x05, 0xf8, 0x05, 0x2a, 0x04, 0x42, 0x03, 0xb0, 0x03, 0x4b, 0x02, 0xf3, 0x02, 0xcf,
    0x02, 0xc5, 0x03, 0x46, 0x03, 0xdf, 0x04, 0x6d, 0x05, 0x46, 0x06, 0x54, 0x07, 0x69, 0x09, 0xe9, 0x06, 0x9d, 0x05, 0xe7,
    0x06, 0x75, 0x04, 0x68, 0x04, 0x10, 0x03, 0x5c, 0x03, 0x0f, 0x02, 0xeb, 0x02, 0xf6, 0x03, 0x7d, 0x04, 0x0e, 0x04, 0xa7,
    0x05, 0xc7, 0x06, 0x72, 0x07, 0x84, 0x0a, 0x0d

};


static void *iic = NULL;

unsigned char wrBG0806_MIPIReg(unsigned short regID, unsigned char regDat)
{
    u8 ret = 1;
    u8 high, low;

    high = regID >> 8;
    low = regID & 0xff;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x64)) {
        ret = 0;
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX, high)) {
        ret = 0;
        goto __wend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX, low)) {
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

unsigned char rdBG0806_MIPIReg(unsigned short regID, unsigned char *regDat)
{
    u8 ret = 1;
    u8 high, low;

    high = regID >> 8;
    low = regID & 0xff;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x64)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, high)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, low)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x65)) {
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
void BG0806_MIPI_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u16 dsc_addr;
    u8 pid;

    rdBG0806_MIPIReg(0x0045, &pid);
    pid = pid & 0x3f;

    printf("bg0806 pid = %x\n", pid);

    for (i = 0; i < sizeof(BG0806_MIPI_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrBG0806_MIPIReg(BG0806_MIPI_INI_REG[i].addr, BG0806_MIPI_INI_REG[i].value);
    }

    switch (pid) {
    case BG0806A:
        wrBG0806_MIPIReg(0x0206, 0x02);
        wrBG0806_MIPIReg(0x0207, 0xc8);
        wrBG0806_MIPIReg(0x0132, 0x01);
        wrBG0806_MIPIReg(0x0133, 0x38);
        break;
    case BG0806C1:
    case BG0806C2:
        wrBG0806_MIPIReg(0x0206, 0x02);
        wrBG0806_MIPIReg(0x0207, 0xaa);
        wrBG0806_MIPIReg(0x0132, 0x01);
        wrBG0806_MIPIReg(0x0133, 0x56);
        break;
    case BG0806B1:
    case BG0806B2:
    default:
        wrBG0806_MIPIReg(0x0206, 0x02);
        wrBG0806_MIPIReg(0x0207, 0xde);
        wrBG0806_MIPIReg(0x0132, 0x01);
        wrBG0806_MIPIReg(0x0133, 0x22);
        break;
    }

    dsc_addr = DSC_ADDR_BASE;

    for (i = 0; i < 768; i++) {
        wrBG0806_MIPIReg(dsc_addr + i, Tab_sensor_dsc[i]);
    }

    wrBG0806_MIPIReg(0x001d, 0x01);

    *format = SEN_IN_FORMAT_GRBG;

    bg0806_mipi_ae_ev_init(*frame_freq);

    wrBG0806_MIPIReg(0x001d, 0x02);

    return;
}

s32 BG0806_MIPI_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    return 0;
}

s32 BG0806_MIPI_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

void BG0806_MIPI_xclk_set(u8 isp_dev)
{

}

s32 BG0806_MIPI_ID_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    u8 i;

    for (i = 0; i < 3; i++) {
        rdBG0806_MIPIReg(0x0000, &pid);
        rdBG0806_MIPIReg(0x0001, &ver);
    }

    log_d("Sensor ID: %x,%x\n", pid, ver);

    if (pid != 0x08 || ver != 0x06) {
        puts("----not BG0806_MIPI-----\n");
        return -1;
    }
    puts("----hello BG0806_MIPI-----\n");
    return 0;
}

void BG0806_MIPI_reset(u8 isp_dev)
{
    puts("BG0806_MIPI reset \n");

    u32 reset_gpio;
    u32 pwdn_gpio;

    if (isp_dev == ISP_DEV_0) {
        reset_gpio = reset_gpios[0];
        pwdn_gpio = pwdn_gpios[0];
    } else {
        reset_gpio = reset_gpios[1];
        pwdn_gpio = pwdn_gpios[1];
    }

    printf("gpio=%d\n", reset_gpio);
    gpio_direction_output(pwdn_gpio, 0);
    gpio_direction_output(reset_gpio, 0);
    delay(40000);
    gpio_direction_output(reset_gpio, 1);
    delay(40000);

}

s32 BG0806_MIPI_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{
    printf("\n\n BG0806_MIPI_check reset pin :%d\n\n", reset_gpio);
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

    BG0806_MIPI_reset(isp_dev);

    if (0 != BG0806_MIPI_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;


    return 0;
}


s32 BG0806_MIPI_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    BG0806_MIPI_reset(isp_dev);
    puts("\n\n BG0806_MIPI_init \n\n");

    cur_expline = -1;
    cur_again = -1;
    cur_dgain = -1;
    cur_expclk = -1;
    cur_reg_idx = -1;
    cur_vblank = -1;
    next_reg_idx = -1;

    BG0806_MIPI_config_SENSOR(width, height, format, frame_freq);
    return 0;
}



static void set_again(u32 again)
{
    if (cur_again == again) {
        return;
    }
    cur_again = again;
    wrBG0806_MIPIReg(0x00b1, again);
    return;
}

static void set_dgain(u32 dgain)
{
    if (cur_dgain == dgain) {
        return;
    }
    cur_dgain = dgain;

    wrBG0806_MIPIReg(0x00bc, dgain >> 8);
    wrBG0806_MIPIReg(0x00bd, dgain & 0xff);
}
static void cal_texp_mck(u32 exp_time, u32 *texp, u32 *mck)
{

    u32 tmp2 = 0;
    s32 remain_ns = 0;

#if BG0806_VARIABLE_FPS
    if (exp_time > 50000) {
        gVblank = VBLANK_15FPS;
        gExplineMax = MAX_EXP_LINE_15FPS;
    } else if (exp_time > 50000) {
        gVblank = VBLANK_16FPS;
        gExplineMax = MAX_EXP_LINE_16FPS;
    } else if (exp_time > 40000) {
        gVblank = VBLANK_20FPS;
        gExplineMax = MAX_EXP_LINE_20FPS;
    } else if (exp_time > 30000) {
        gVblank = VBLANK_25FPS;
        gExplineMax = MAX_EXP_LINE_25FPS;
    } else {
        gVblank = VBLANK_30FPS;
        gExplineMax = MAX_EXP_LINE_30FPS;
    }
#endif

    *texp = exp_time * 1000 / ROW_TIME_NS;

    if (*texp >= gExplineMax) {
        *texp = gExplineMax;
    } else if (*texp < 1) {
        *texp = 1;
    }

    tmp2 = (*texp) * ROW_TIME_NS;
    remain_ns = (s32)exp_time * 1000 - (s32)tmp2;
    if (remain_ns > 0) {
        if (remain_ns >= ROW_TIME_NS) {
            *mck = gRowTime;
        } else {
            *mck = (remain_ns * gRowTime / ROW_TIME_NS);
        }
    } else {
        *mck = 0;
    }

    return;
}

//q10
static void calc_gain(u32 gain, u32 *_again, u32 *_dgain)
{

    u32 total_gain;
    u8 vrefh, temp;
    u16 dgain;

    u32 dark_ave;

    u16 vrefh_old;
    u16 rmp_gain;
    u8 i, rdRegDat;

#if 1

    rdBG0806_MIPIReg(0x00b1, &rdRegDat) ;
    vrefh_old = rdRegDat ;
    rmp_gain = 128 / (vrefh_old + 1);

    dark_ave = 0 ;
    rdBG0806_MIPIReg(0x012b, &rdRegDat) ;
    dark_ave = rdRegDat ;
    dark_ave <<= 8 ;
    rdBG0806_MIPIReg(0x012c, &rdRegDat) ;
    dark_ave |= rdRegDat ;

    if (dark_ave > 0xfff) {
        dark_ave = 0;
    }

    dark_ave = dark_ave / rmp_gain;

    if (dark_ave < stVrefhTable[4].th_low) {
        pre_index = 0;
        vrefh_min_tlb = 0x0c;
    } else {
        i = 0;
        while (i < 5) {
            if (pre_index < stVrefhTable[i].index && dark_ave > stVrefhTable[i].th_high) {
                pre_index = stVrefhTable[i].index;
                vrefh_min_tlb = stVrefhTable[i].vrefh;
                break;
            }
            i++;
        }

        i = 0;
        for (i = 0; i < 5; i++) {
            if (pre_index >=  stVrefhTable[i].index && dark_ave < stVrefhTable[i].th_low) {
                pre_index = stVrefhTable[i - 1].index;
                vrefh_min_tlb = stVrefhTable[i - 1].vrefh;
            }
        }
    }
#endif

    total_gain = gain;
    if (total_gain >= (128 << 10)) {
        vrefh = 0;
    } else {
        vrefh = (128 << 10) / total_gain - 1;
    }

    if (vrefh > 0x7f) {
        vrefh = 0x7f ;
    } else if (vrefh < vrefh_min_tlb) {
        vrefh = vrefh_min_tlb ;
    }

    dgain = (total_gain * (vrefh + 1) * 512) / (128 * 1024); //dgain
    if (dgain < 0x200) {
        dgain = 0x200;
    }
    if (dgain > 0xfff) {
        dgain = 0xfff;
    }

    if (vrefh == vrefh_min_tlb || vrefh <= 0x0f) {
        // reg_0x2b = 0x10;// remove ppk 20170623
        reg_0x30 = 0x01;
        reg_0x31 = 0xb0; // add ppk 20170623
        reg_0x34 = 0x01;
        reg_0x35 = 0xb0;// add ppk 20170623
        reg_0x4d = 0x03;
        reg_0x4f = 0x0c;
        reg_0x61 = 0x02;
        reg_0x67 = 0x00;
        reg_0x68 = 0x80;
        next_reg_idx = 1;
    } else if ((vrefh > vrefh_min_tlb) && (vrefh <= 0x7f)) {
        // reg_0x2b = 0x30; // remove ppk 20170623
        reg_0x30 = 0x00;
        reg_0x31 = 0xc0; // add ppk 20170623
        reg_0x34 = 0x00;
        reg_0x35 = 0xc0;// add ppk 20170623
        reg_0x4d = 0x00;
        reg_0x4f = 0x09;
        reg_0x61 = 0x04;
        reg_0x67 = 0x01;
        reg_0x68 = 0x90;
        next_reg_idx = 0;
    }

    *_again = vrefh ;
    *_dgain = dgain ;

    //printf("_again=%d, _dgain=%d\n", *_again, *_dgain);
}

static void set_shutter(u32 texp, u32 texp_mck)
{


    if (cur_vblank != gVblank) {
        cur_vblank = gVblank;
        wrBG0806_MIPIReg(0x0021, gVblank >> 8);
        wrBG0806_MIPIReg(0x0022, gVblank & 0xff);
    }

    if (cur_expline != texp) {
        cur_expline = texp;
        wrBG0806_MIPIReg(0x000c, texp >> 8);
        wrBG0806_MIPIReg(0x000d, texp & 0xff); //
    }
    if (cur_expclk != texp_mck) {
        cur_expclk = texp_mck;
        wrBG0806_MIPIReg(0x0026, texp_mck >> 8);
        wrBG0806_MIPIReg(0x0027, texp_mck & 0xff);
    }
}


u32 bg0806_mipi_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;
    u32 mck;

    cal_texp_mck(exp_time_us, &texp, &mck);

    //printf(" \n exp_time_us %d\n", exp_time_us); // ppk added on 20170609

    shutter->ae_exp_line =  texp;
    shutter->ae_gain = gain;
    shutter->ae_exp_clk = mck;


    return 0;

}



u32 bg0806_mipi_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 again, dgain;
    calc_gain(shutter->ae_gain, &again, &dgain);

#if 1

    //printf("reg_0x2b(%x)\n", reg_0x2b);
    //printf("reg_0x30(%x)\n", reg_0x30);
    //printf("reg_0x34(%x)\n", reg_0x34);
    //printf("reg_0x4d(%x)\n", reg_0x4d);
    //printf("reg_0x4f(%x)\n", reg_0x4f);
    //printf("reg_0x61(%x)\n", reg_0x61);
    //printf("reg_0x67(%x)\n", reg_0x67);
    //printf("reg_0x68(%x)\n", reg_0x68);

    if (cur_reg_idx != next_reg_idx) {
        cur_reg_idx = next_reg_idx;
        wrBG0806_MIPIReg(0x0030, reg_0x30);
        wrBG0806_MIPIReg(0x0031, reg_0x31); // add ppk 20170623
        wrBG0806_MIPIReg(0x0034, reg_0x34);
        wrBG0806_MIPIReg(0x0035, reg_0x35);// add ppk 20170623
        wrBG0806_MIPIReg(0x004d, reg_0x4d);
        wrBG0806_MIPIReg(0x004f, reg_0x4f);
        wrBG0806_MIPIReg(0x0061, reg_0x61);

        //wrBG0806_MIPIReg(0x002b, reg_0x2b);// remove ppk 20170623
        wrBG0806_MIPIReg(0x0067, reg_0x67);
        wrBG0806_MIPIReg(0x0068, reg_0x68);
    }

#endif

    set_again(again);
    set_dgain(dgain);

    set_shutter(shutter->ae_exp_line, shutter->ae_exp_clk);


    wrBG0806_MIPIReg(0x001d, 0x02);
    return 0;
}


void BG0806_MIPI_sleep()
{

}

void BG0806_MIPI_wakeup()
{

}

void BG0806_MIPI_W_Reg(u16 addr, u16 val)
{
    wrBG0806_MIPIReg((u16) addr, (u8) val);
}

u16 BG0806_MIPI_R_Reg(u16 addr)
{
    u8 val;
    rdBG0806_MIPIReg((u16) addr, &val);
    return val;
}



REGISTER_CAMERA(BG0806_MIPI) = {
    .logo 				= 	"BG0806M",
    .isp_dev 			= 	ISP_DEV_NONE,
//   .in_format 		= 	SEN_IN_FORMAT_GRBG,//不镜像
//  .in_format 			= 	SEN_IN_FORMAT_BGGR,//垂直镜像
//  .in_format 			= 	SEN_IN_FORMAT_RGGB,//水平镜像
    .in_format 			= 	SEN_IN_FORMAT_GBRG,//水平加垂直镜像
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_CSI2,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_CSI2_1_LANE,
    /* .real_fps 			=   (u32)(30.0 * 65536), */
    .fps         		= 	FRAME_RATE, // 30,

    .sen_size 			= 	{BG0806_MIPI_OUTPUT_W, BG0806_MIPI_OUTPUT_H},
    .isp_size 			= 	{BG0806_MIPI_OUTPUT_W, BG0806_MIPI_OUTPUT_H},

    .cap_fps         		= 	FRAME_RATE,
    .sen_cap_size 			= 	{BG0806_MIPI_OUTPUT_W, BG0806_MIPI_OUTPUT_H},
    .isp_cap_size 			= 	{BG0806_MIPI_OUTPUT_W, BG0806_MIPI_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	BG0806_MIPI_check,
        .init 		        = 	BG0806_MIPI_init,
        .set_size_fps 		=	BG0806_MIPI_set_output_size,
        .power_ctrl         =   BG0806_MIPI_power_ctl,

        .get_ae_params 	    =	bg0806_mipi_get_ae_params,
        .get_awb_params 	=	bg0806_mipi_get_awb_params,
        .get_iq_params 	    =	bg0806_mipi_get_iq_params,

        .sleep 		        =	BG0806_MIPI_sleep,
        .wakeup 		    =	BG0806_MIPI_wakeup,
        .write_reg 		    =	BG0806_MIPI_W_Reg,
        .read_reg 		    =	BG0806_MIPI_R_Reg,

    }
};


#endif

