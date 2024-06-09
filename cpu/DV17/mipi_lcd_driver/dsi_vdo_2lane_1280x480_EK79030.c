#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#include "os/os_compat.h"

#ifdef LCD_DSI_VDO_2LANE_MIPI_EK79030
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list


const static u8 init_cmd_list[] = {
    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD,0xAA, */
    /* _W, DELAY(120), PACKET_DCS, SIZE(1), 0x11, */
    /* _R, DELAY(0), PACKET_DCS, SIZE(2), 0xFA,0x01, */

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD, 0xAA,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x65, 0x08,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3A, 0x14,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x32, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x36, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x67, 0x82,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x69, 0x20,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6D, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x68, 0x00,

    _W, DELAY(0), PACKET_DCS, SIZE(20), 0x53, 0x19, 0x17, 0x15, 0x12, 0x12, 0x12, 0x13, 0x15, 0x15, 0x10, 0x0C, 0x0A, 0x0A, 0x0C, 0x0B, 0x0C, 0x09, 0x07, 0x06,
    _W, DELAY(0), PACKET_DCS, SIZE(20), 0x54, 0x19, 0x16, 0x14, 0x11, 0x11, 0x11, 0x13, 0x15, 0x15, 0x10, 0x0B, 0x09, 0x09, 0x0B, 0x0B, 0x0C, 0x09, 0x07, 0x06,

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x29, 0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2A, 0x0C,

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6C, 0x81,
    _W, DELAY(0), PACKET_DCS, SIZE(9), 0x55, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F,
    _W, DELAY(0), PACKET_DCS, SIZE(17), 0x56, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F,
    _W, DELAY(0), PACKET_DCS, SIZE(5), 0x57, 0x00, 0x00, 0x00, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x30, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x39, 0x11,

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x33, 0x08,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x35, 0x25,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4F, 0x3D,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4E, 0x35,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x41, 0x35,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x73, 0x30,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x74, 0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x76, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x77, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x28, 0x31,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7c, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2e, 0x04,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4c, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x47, 0x16,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x48, 0x6A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x50, 0xc0,

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x78, 0x6E,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2D, 0x31,

    /* _W, DELAY(0), PACKET_DCS, SIZE(2), 0x63, 0x04, //4 lane */
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x63, 0x02, // 2 lane

    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4D, 0x00,

    _W, DELAY(120), PACKET_DCS, SIZE(1), 0x11,
};

#define real_freq 700
#define freq (750*real_freq/700)

#define lane_num 2
/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24

#define vsa_line 2
#define vbp_line 10//17
#define vda_line 1280
#define vfp_line 12//10

#define hsa_pixel  24//8
#define hbp_pixel  100//42
#define hda_pixel  720
#define hfp_pixel  60//44

REGISTER_MIPI_DEVICE_BEGIN(mipi_dev_t) = {
    .info = {
        .xres 			= LCD_DEV_WIDTH,
        .yres 			= LCD_DEV_HIGHT,
        .target_xres    = LCD_DEV_WIDTH,
        .target_yres    = LCD_DEV_HIGHT,
        .buf_addr 		= LCD_DEV_BUF,
        .buf_num 		= LCD_DEV_BNUM,
        .sample         = LCD_DEV_SAMPLE,
        .test_mode 		= false,
        .test_mode_color = 0x0000ff,
        .canvas_color   = 0x000000,
        .format 		= FORMAT_RGB888,
        .len 			= LEN_256,

        .rotate_en 		= true,				// 旋转使能
        .hori_mirror_en = true,		        // 水平镜像使能
        .vert_mirror_en = false,			// 垂直镜像使能

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
    .lane_mapping = {
        .x0_lane = MIPI_LANE_DIS  | MIPI_LANE_D3,
        .x1_lane = MIPI_LANE_DIS  | MIPI_LANE_D2,
        .x2_lane = MIPI_LANE_EN  | MIPI_LANE_CLK,
        .x3_lane = MIPI_LANE_EN  | MIPI_LANE_D1,
        .x4_lane = MIPI_LANE_EN  | MIPI_LANE_D0,
    },
    .video_timing = {
        .video_mode = VIDEO_STREAM_VIDEO,
        .sync_mode  = SYNC_PULSE_MODE,
        .color_mode = COLOR_FORMAT_RGB888,
        .pixel_type = PIXEL_RGB888,
        .virtual_ch = 0,
        .hs_eotp_en = true,

        .dsi_vdo_vsa_v  = vsa_line,
        .dsi_vdo_vbp_v  = vbp_line,
        .dsi_vdo_vact_v = vda_line,
        .dsi_vdo_vfp_v  = vfp_line,

        .dsi_vdo_hsa_v  = (lane_num * hsa_pixel) - 10,
        .dsi_vdo_hbp_v  = (lane_num * hbp_pixel) - 10,
        .dsi_vdo_hact_v = (bpp_num  * hda_pixel) / 8,
        .dsi_vdo_hfp_v  = (lane_num * hfp_pixel) - 6,
        .dsi_vdo_bllp0_v = hsa_pixel + hbp_pixel + hda_pixel + hfp_pixel,//?+hda_pixel

        /* .dsi_vdo_hsa_v   = ((bpp_num * hsa_pixel) / 8) - 10, */
        /* .dsi_vdo_hbp_v   = ((bpp_num * hbp_pixel) / 8) - 10, */
        /* .dsi_vdo_hact_v  = ((bpp_num * hda_pixel) / 8), */
        /* .dsi_vdo_hfp_v   = ((bpp_num * hfp_pixel) / 8) - 6, */
        /* .dsi_vdo_bllp0_v = ((bpp_num * (hbp_pixel + hda_pixel + hfp_pixel) / 8) - 10), */
        /* .dsi_vdo_bllp1_v = ((bpp_num * hda_pixel) / 8), */
    },
    .timing = {
        /* 以下参数只需修改freq */
        .tval_lpx   = ((80     * freq / 1000) / 2 - 1),
        .tval_wkup  = ((100000 * freq / 1000) / 8 - 1),
        .tval_c_pre = ((40     * freq / 1000) / 2 - 1),
        .tval_c_sot = ((300    * freq / 1000) / 2 - 1),
        .tval_c_eot = ((100    * freq / 1000) / 2 - 1),
        .tval_c_brk = ((200    * freq / 1000) / 2 - 1),
        .tval_d_pre = ((60     * freq / 1000) / 2 - 1),
        .tval_d_sot = ((160    * freq / 1000) / 2 - 1),
        .tval_d_eot = ((100    * freq / 1000) / 2 - 1),
        .tval_d_brk = ((200    * freq / 1000) / 2 - 1),
        .tval_c_rdy = 400/* 64 */,
    },
    .pll4 = {
        .pll_freq = real_freq,
        .source = PLL4_SRC_x12M,
    },

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = true,
},
REGISTER_MIPI_DEVICE_END()

void read_data_func(void *p)
{
    u8 status;
    u8 param;
    static u8 cnt = 0;
    while (1) {
        dsi_task_con |= BIT(7);
        status = dcs_send_short_p1_bta(0xCD, 0xAA);
        printf("staus0=%d\n", status);
        delay(0x100);
        status = dcs_send_short_p0_bta(0x11);
        printf("staus1=%d\n", status);
        delay_10ms(120 / 10 + 1);

        status = dcs_read_parm(0xfa, &param, 0x01);
        delay(0x100);
        printf("\nparam=%d 0x%x\n", param, status);
        if (status == 10) {
            cnt++;
            printf("read data ok %d\n", cnt);
        }
        dsi_task_con &= ~BIT(7);

    }

}
static int dsi_vdo_ek79030_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u8 lcd_standby = data->lcd_io.lcd_standby;

    if ((u8) - 1 != lcd_standby) { /*2 lane*/
        printf("lcd_standby = %d\n", lcd_standby);
        gpio_direction_output(lcd_standby, 0);
    }
    /*
     * lcd reset
     */
    if ((u8) - 1 != lcd_reset) {
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(5);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(5);
    }


    /* thread_fork("read_data_task",20,4000,0,0,read_data_func,NULL); */

    return 0;
}

static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if ((u8) - 1 == data->lcd_io.backlight) {
        return;
    }
    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}
/*  */
/* void ek79030_esd_check(struct esd_check *esd) */
/* { */
/*     static u32 errcnt = 0; */
/*     u8 param = 0; */
/*     u8 buf[32]; */
/*     u8 status = 0; */
/*  */
/*     if (esd->en) { */
/*         esd->count ++; */
/*         #<{(| dcs_read_parm(0x04,buf,3); |)}># */
/*         #<{(| dcs_read_parm(0x30,buf,5); |)}># */
/*         #<{(| generic_read_parm(0x45,buf,2); |)}># */
/*         #<{(| generic_read_parm(0x0a,buf,1); |)}># */
/*         #<{(| dcs_send_short_p1_bta(0xCD,0xAA); |)}># */
/*         #<{(| memset(buf,0,32); |)}># */
/*         dsi_task_con |= BIT(7); */
/*         #<{(| dcs_send_short_p1_bta(0xCD, 0xAA); |)}># */
/*         #<{(| delay(0x100); |)}># */
/*         #<{(| dcs_send_short_p0_bta(0x11); |)}># */
/*         #<{(| delay_10ms(120 / 10 + 1); |)}># */
/*  */
/*         status = dcs_read_parm(0xfa, &param, 0x01); */
/*         #<{(| delay(0x100); |)}># */
/*         dsi_task_con &= ~BIT(7); */
/*         printf("param=%d 0x%x\n", param, status); */
/*  */
/*         if (param != 15) { */
/*             errcnt ++; */
/*             #<{(| printf("errcnt : 0x%x\n", errcnt); |)}># */
/*             if (errcnt == 6) { */
/*                 errcnt = 0; */
/*  */
/*                 lcd_reinit(); */
/*             } */
/*         } */
/*  */
/*         if (esd->count > 10) { //dsi中断没来 */
/*             lcd_reinit(); */
/*             #<{(| chip_reset(); |)}># */
/*         } */
/*     } */
/* } */
REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_ek79030_init,
    .bl_ctrl = mipi_backlight_ctrl,
    .esd = {
        /*.interval = 1000,
        .esd_check_isr = ek79030_esd_check,*/
    }
};

#endif
