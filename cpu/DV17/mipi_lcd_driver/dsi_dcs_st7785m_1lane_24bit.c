#include "generic/typedef.h"
#include "asm/cpu.h"
#include "asm/dsi.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_DSI_DCS_ST7785M_1LANE_24BIT

const static u8 init_cmd_list[] = {


#if 1

    _W, DELAY(120), PACKET_DCS, SIZE(1),0x11,

    _W, DELAY(120), PACKET_DCS, SIZE(1), 0xDF,    //EXTC

    _W, DELAY(0), PACKET_DCS,   SIZE(8),0x00,0x5A,0x00,0x69,0x00,0x02,0x00,0x01,


    _W, DELAY(0), PACKET_DCS, SIZE(1),  0x36,
    _W, DELAY(0), PACKET_DCS, SIZE(2),  0x00,0x00,

    _W, DELAY(0), PACKET_DCS, SIZE(1),  0x3A,

    _W, DELAY(0), PACKET_DCS, SIZE(2),  0x00,0x66,

    _W, DELAY(0), PACKET_DCS, SIZE(1),  0xB0,     //MIPI Video Mode

    _W, DELAY(0), PACKET_DCS, SIZE(2),  0x00,0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(1),  0xB2,
    _W, DELAY(0), PACKET_DCS, SIZE(10), 0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x33,0x00,0x33,


    _W, DELAY(0), PACKET_DCS, SIZE(1),  0xB7,

    _W, DELAY(0), PACKET_DCS, SIZE(2),  0x00,0x51,

    _W, DELAY(0), PACKET_DCS, SIZE(1),  0xBB,

    _W, DELAY(0), PACKET_DCS, SIZE(2),  0x00,0x18,

    _W, DELAY(0), PACKET_DCS, SIZE(1),  0xC0,

    _W, DELAY(0), PACKET_DCS, SIZE(2),  0x00,0x2C ,

    _W, DELAY(0), PACKET_DCS, SIZE(1),  0xC2,
    _W, DELAY(0), PACKET_DCS, SIZE(2),  0x00,0x01 ,


    _W, DELAY(0), PACKET_DCS, SIZE(1),  0xC3,

    _W, DELAY(0), PACKET_DCS, SIZE(2),   0x00, 0x13,


    _W, DELAY(0), PACKET_DCS, SIZE(1),   0xC6,

    _W, DELAY(0), PACKET_DCS, SIZE(2),   0x00,0x0F,

    _W, DELAY(0), PACKET_DCS, SIZE(1),   0xD0,


    _W, DELAY(0), PACKET_DCS, SIZE(2),  0x00, 0xA7,

    _W, DELAY(0), PACKET_DCS, SIZE(1),  0xD0,

    _W, DELAY(0), PACKET_DCS, SIZE(4),  0x00, 0xA4,0x00,0xA1,

    _W, DELAY(0), PACKET_DCS, SIZE(1),  0xE0,

    _W, DELAY(0), PACKET_DCS, SIZE(28), 0x00,0xF0,0x00,0x00,0x00,0x08,0x00,0x04,0x00,0x06,0x00,0x05,0x00,0x34,0x00,0x44,0x00,0x4A,0x00,0x38,0x00,0x13,0x00,0x13,0x00,0x2F,0x00,0x36,


    _W, DELAY(0), PACKET_DCS, SIZE(1),  0xE1,

    _W, DELAY(0), PACKET_DCS, SIZE(28), 0x00,0xF0,0x00,0x0F,00,0x13,0x00,0x11,0x00,0x0E,0x00,0x27,0x00,0x34,0x00,0x44,0x00,0x4A,0x00,0x36,0x00,0x10,0x00,0x10,0x00,0x2C,0x00,0x33,

    _W, DELAY(0), PACKET_DCS, SIZE(1), 0x21,

    _W, DELAY(0), PACKET_DCS, SIZE(1), 0x29,
#else




    _W, DELAY(120), PACKET_DCS, SIZE(1),0x11,



    _W, DELAY(0), PACKET_DCS,   SIZE(9),0xDF,0x00,0x5A,0x00,0x69,0x00,0x02,0x00,0x01,



    _W, DELAY(0), PACKET_DCS, SIZE(3),0x36,  0x00,0x00,



    _W, DELAY(0), PACKET_DCS, SIZE(3), 0x3A,  0x00,0x66,


    _W, DELAY(0), PACKET_DCS, SIZE(3),0xB0,  0x00,0x10,

    _W, DELAY(0), PACKET_DCS, SIZE(11),0xB2, 0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x33,0x00,0x33,




    _W, DELAY(0), PACKET_DCS, SIZE(3),0xB7,  0x00,0x51,



    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xBB,  0x00,0x18,



    _W, DELAY(0), PACKET_DCS, SIZE(3),  0xC0, 0x00,0x2C ,


    _W, DELAY(0), PACKET_DCS, SIZE(3), 0xC2,  0x00,0x01 ,



    _W, DELAY(0), PACKET_DCS, SIZE(3),  0xC3,  0x00, 0x13,




    _W, DELAY(0), PACKET_DCS, SIZE(3),  0xC6,  0x00,0x0F,




    _W, DELAY(0), PACKET_DCS, SIZE(3),0xD0,  0x00, 0xA7,



    _W, DELAY(0), PACKET_DCS, SIZE(5),  0xD0, 0x00, 0xA4,0x00,0xA1,



    _W, DELAY(0), PACKET_DCS, SIZE(29), 0xE0, 0x00,0xF0,0x00,0x00,0x00,0x08,0x00,0x04,0x00,0x06,0x00,0x05,0x00,0x34,0x00,0x44,0x00,0x4A,0x00,0x38,0x00,0x13,0x00,0x13,0x00,0x2F,0x00,0x36,




    _W, DELAY(0), PACKET_DCS, SIZE(29),0xE1, 0x00,0xF0,0x00,0x0F,00,0x13,0x00,0x11,0x00,0x0E,0x00,0x27,0x00,0x34,0x00,0x44,0x00,0x4A,0x00,0x36,0x00,0x10,0x00,0x10,0x00,0x2C,0x00,0x33,

    _W, DELAY(0), PACKET_DCS, SIZE(1), 0x21,

    _W, DELAY(0), PACKET_DCS, SIZE(1), 0x29,

#endif


};

#define freq 300

REGISTER_MIPI_DEVICE_BEGIN(mipi_dev_t) = {
    .info = {
        .xres 			= LCD_DEV_WIDTH,
        .yres 			= LCD_DEV_HIGHT,
        .target_xres 	= LCD_DEV_WIDTH,
       // .target_yres 	  LCD_DEV_HIGHT,
        .buf_addr 		= LCD_DEV_BUF,
        .buf_num 		= LCD_DEV_BNUM,
        .sample         = LCD_DEV_SAMPLE,
        .test_mode 		= true,
        .test_mode_color = 0x0000ff,
        .canvas_color   = 0x000000,
        .format 		= FORMAT_RGB888,
        .len 			= LEN_256,

        .rotate_en 		= true,			// 旋转使能
        .hori_mirror_en = true,			// 水平镜像使能
        .vert_mirror_en = false,		// 垂直镜像使能

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
        .x0_lane = MIPI_LANE_EN | MIPI_LANE_D0,
        .x1_lane = MIPI_LANE_EN | MIPI_LANE_CLK,
        .x2_lane = MIPI_LANE_DIS | MIPI_LANE_D0,
        .x3_lane = MIPI_LANE_DIS,//  | MIPI_LANE_D1,
        .x4_lane = MIPI_LANE_DIS,//  | MIPI_LANE_D0,
    },
    .video_timing = {
        .video_mode = VIDEO_STREAM_COMMAND,//视频模式 类似于dump panel
        .sync_mode  = SYNC_PULSE_MODE,//同步事件
        .color_mode = COLOR_FORMAT_RGB888,
        .pixel_type = PIXEL_RGB888,
        .virtual_ch = 0,
        .hs_eotp_en = true,

        .dsi_vdo_vsa_v  = 0,
        .dsi_vdo_vbp_v  = 16,
        .dsi_vdo_vact_v = 640,
        .dsi_vdo_vfp_v  = 0,

        .dsi_vdo_hsa_v  = 0,
        .dsi_vdo_hbp_v  = 0,
        .dsi_vdo_hact_v = 360 * 3 + 1, //+1 for DCS command
        .dsi_vdo_hfp_v  = 0,

        .dsi_vdo_bllp0_v = 360 * 3 * 2,
        .dsi_vdo_bllp1_v = 360 * 3 + 1,
    },
    .timing = {
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
    .pll_freq = 500,/* 配置PLL频率的最佳范围为600MHz~1.2GHz,少于600MHz的频率通过二分频获得 */

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = false,
},
REGISTER_MIPI_DEVICE_END()


static int dsi_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    /*
     * lcd reset
     */
    if (0xff != lcd_reset) {
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

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type = LCD_MIPI,
    .init = dsi_init,
    .dev  = &mipi_dev_t,
    .bl_ctrl = mipi_backlight_ctrl,
};
#endif
