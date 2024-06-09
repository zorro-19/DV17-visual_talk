#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imb.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#include "os/os_compat.h"

#ifdef MIPI_LCD_1920x1080_HX8399C
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list

const static u8 init_cmd_list[] = {
    _W, DELAY(0), PACKET_DCS, SIZE(4), 0xB9, 0xFF, 0x83, 0x99,
    _W, DELAY(0), PACKET_DCS, SIZE(13), 0xB1, 0x02, 0x04, 0x74, 0x94, 0x01, 0x32, 0x33, 0x11, 0x11, 0xAB, 0x4D, 0x06,
    _W, DELAY(0), PACKET_DCS, SIZE(13), 0xB2, 0x00, 0x80, 0x80, 0xCC, 0x05, 0x07, 0x5A, 0x11, 0x00, 0x00, 0x10, 0x0E,
    _W, DELAY(0), PACKET_DCS, SIZE(45), 0xB4, 0x00, 0xFF, 0x02, 0xAE, 0x02, 0xA7, 0x02, 0xA7, 0x02, 0x00, 0x03, 0x05, 0x00, 0x2D, 0x03, 0x0E, 0x0A, 0x21, 0x03, 0x02, 0x00, 0x0B, 0xA5, 0x87, 0x02, 0xA7, 0x02, 0xA7, 0x02, 0xA7, 0x02, 0x00, 0x03, 0x05, 0x00, 0x2D, 0x03, 0x0E, 0x0A, 0x02, 0x00, 0x0B, 0xA5, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(34), 0xD3, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x06, 0x00, 0x32, 0x10, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x05, 0x05, 0x07, 0x00, 0x00, 0x00, 0x05, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(33), 0xD5, 0x18, 0x18, 0x19, 0x19, 0x58, 0x58, 0x20, 0x21, 0x00, 0x01, 0x06, 0x07, 0x04, 0x05, 0x02, 0x03, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x2F, 0x2F, 0x30, 0x30, 0x31, 0x31, 0x58, 0x58, 0x58, 0x58,
    _W, DELAY(0), PACKET_DCS, SIZE(33), 0xD6, 0x18, 0x18, 0x19, 0x19, 0x18, 0x18, 0x21, 0x20, 0x03, 0x02, 0x05, 0x04, 0x07, 0x06, 0x01, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x2F, 0x2F, 0x30, 0x30, 0x31, 0x31, 0x18, 0x18, 0x18, 0x18,
    _W, DELAY(0), PACKET_DCS, SIZE(9), 0xD8, 0xA2, 0xAA, 0x02, 0xA0, 0xA2, 0xAA, 0x02, 0xA0,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBD, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(9), 0xD8, 0xE2, 0xAA, 0x03, 0xF0, 0xE2, 0xAA, 0x03, 0xF0,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBD, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(9), 0xD8, 0xE2, 0xAA, 0x03, 0xF0, 0xE2, 0xAA, 0x03, 0xF0,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBD, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(55), 0xE0, 0x00, 0x2B, 0x38, 0x32, 0x69, 0x70, 0x7B, 0x75, 0x7B, 0x83, 0x8A, 0x8F, 0x93, 0x9A, 0xA0, 0xA3, 0xA7, 0xAF, 0xB1, 0xB9, 0xAC, 0xB8, 0xB9, 0x5F, 0x59, 0x63, 0x7F, 0x00, 0x2B, 0x38, 0x32, 0x69, 0x70, 0x7B, 0x75, 0x7B, 0x83, 0x8A, 0x8F, 0x93, 0x9A, 0xA0, 0xA3, 0xA7, 0xAF, 0xB1, 0xB9, 0xAC, 0xB8, 0xB9, 0x5F, 0x59, 0x63, 0x7F,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD2, 0x44,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCC, 0x09,
    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xB6, 0x87, 0x87,
    _W, DELAY(150), PACKET_DCS, SIZE(1), 0x11,
    _W, DELAY(20), PACKET_DCS, SIZE(1), 0x29,
};

#define freq (800*60/58.5)

#define lane_num 4
/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24

#define vsa_line 4
#define vbp_line 3
#define vda_line 1920
#define vfp_line 9

#define hsa_pixel  8
#define hbp_pixel  42
#define hda_pixel  1080
#define hfp_pixel  44

REGISTER_MIPI_DEVICE_BEGIN(mipi_dev_t) = {
    .info = {
        .xres           = LCD_DEV_WIDTH,
        .yres           = LCD_DEV_HIGHT,
        .target_xres 	= LCD_DEV_WIDTH,
        .target_yres 	= 1080/* LCD_DEV_HIGHT */,
        .buf_addr       = LCD_DEV_BUF,
        .buf_num        = LCD_DEV_BNUM,
        .sample         = LCD_DEV_SAMPLE,
        .test_mode      = false,
        .test_mode_color = 0x0000ff,
        .canvas_color   = 0x0000ff,
        .format         = FORMAT_RGB888,
        .len            = LEN_256,
        .interlaced_mode = INTERLACED_NONE,

        .rotate_en      = true,             // 旋转使能
        .hori_mirror_en = false,             // 水平镜像使能
        .vert_mirror_en = false,            // 垂直镜像使能

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
        //develop board
        /* .x0_lane = MIPI_LANE_EN  | MIPI_LANE_D0, */
        /* .x1_lane = MIPI_LANE_EN  | MIPI_LANE_D1, */
        /* .x2_lane = MIPI_LANE_EN  | MIPI_LANE_CLK, */
        /* .x3_lane = MIPI_LANE_EN  | MIPI_LANE_D2, */
        /* .x4_lane = MIPI_LANE_EN  | MIPI_LANE_D3, */

        //样机
        .x0_lane = MIPI_LANE_EN | MIPI_LANE_D3 | MIPI_LANE_EX,
        .x1_lane = MIPI_LANE_EN | MIPI_LANE_D2 | MIPI_LANE_EX,
        .x2_lane = MIPI_LANE_EN | MIPI_LANE_CLK | MIPI_LANE_EX,
        .x3_lane = MIPI_LANE_EN | MIPI_LANE_D1 | MIPI_LANE_EX,
        .x4_lane = MIPI_LANE_EN | MIPI_LANE_D0 | MIPI_LANE_EX,
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

        /* .dsi_vdo_hsa_v  = (lane_num * hsa_pixel) - 10, */
        /* .dsi_vdo_hbp_v  = (lane_num * hbp_pixel) - 10, */
        /* .dsi_vdo_hact_v = (bpp_num  * hda_pixel) / 8, */
        /* .dsi_vdo_hfp_v  = (lane_num * hfp_pixel) - 6, */
        /* .dsi_vdo_bllp0_v = hsa_pixel + hbp_pixel + hda_pixel + hfp_pixel, */

        .dsi_vdo_hsa_v   = ((bpp_num * hsa_pixel) / 8) - 10,
        .dsi_vdo_hbp_v   = ((bpp_num * hbp_pixel) / 8) - 10,
        .dsi_vdo_hact_v  = ((bpp_num * hda_pixel) / 8),
        .dsi_vdo_hfp_v   = ((bpp_num * hfp_pixel) / 8) - 6,

        .dsi_vdo_bllp0_v = ((bpp_num * (hbp_pixel + hda_pixel + hfp_pixel) / 8) - 10),
        .dsi_vdo_bllp1_v = ((bpp_num * hda_pixel) / 8),
    },
    .timing = {
        /* 以下参数只需修改freq */
        .tval_lpx   = ((80     * freq / 1000) / 2 - 1),
        .tval_wkup  = ((100000 * freq / 1000) / 8 - 1),
        .tval_c_pre = ((40     * freq / 1000) / 2 - 1),
        .tval_c_sot = ((300    * freq / 1000) / 2 - 1),
        .tval_c_eot = ((100    * freq / 1000) / 2 - 1),
        .tval_c_brk = ((150    * freq / 1000) / 2 - 1),
        .tval_d_pre = ((60     * freq / 1000) / 2 - 1),
        .tval_d_sot = ((160    * freq / 1000) / 2 - 1),
        .tval_d_eot = ((100    * freq / 1000) / 2 - 1),
        .tval_d_brk = ((150    * freq / 1000) / 2 - 1),
        .tval_c_rdy = 400/* 64 */,
    },
    .pll4 = {
        .pll_freq = freq,
        .source = PLL4_SRC_x12M,
    },

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = true,
},
REGISTER_MIPI_DEVICE_END()

static int mipi_lcd_hx8399c_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u8 lcd_standby  = data->lcd_io.lcd_standby;

    printf("mipi_lcd_hx8399c_init...\n");

    if ((u8) - 1 != lcd_standby) { /*4 lane*/
        /* gpio_direction_output(lcd_standby, 1); */
    }

    /*
     * lcd reset
     */
    printf("lcd_reset : %d,[%d-%d]\n", lcd_reset, lcd_reset / 16, lcd_reset % 16);
    if ((u8) - 1 != lcd_reset) {
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(5);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(5);
    }


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

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type      = LCD_MIPI,
    .dev       = &mipi_dev_t,
    .init      = mipi_lcd_hx8399c_init,
    .bl_ctrl   = mipi_backlight_ctrl,
    .esd = {
        /* .interval = 1000, */
        /* .esd_check_isr = ek79030_esd_check, */
    }
};

#endif
