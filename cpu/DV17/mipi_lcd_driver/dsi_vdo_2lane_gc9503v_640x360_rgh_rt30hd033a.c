#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imd.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#include "os/os_compat.h"

#ifdef LCD_DSI_VDO_2LANE_MIPI_GC9503V_640X360_RGH_RT30HD033A

//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list

const static u8 init_cmd_list[] = {
    _W, DELAY(0), PACKET_DCS, SIZE(6), 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00,

	_W, DELAY(0), PACKET_DCS, SIZE(3), 0xF6, 0x5A, 0x87,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xC1, 0x3F,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xC2, 0x0E,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xC6, 0xF8,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xC9, 0x10,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD, 0x25,

	_W, DELAY(0), PACKET_DCS, SIZE(4), 0xFA, 0x08, 0x08, 0x08,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xF8, 0x8A,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x71, 0x48,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x72, 0x48,

	_W, DELAY(0), PACKET_DCS, SIZE(3), 0x73, 0x00, 0x44,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x97, 0xEE,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x83, 0x93,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xA3, 0x22,

	_W, DELAY(0), PACKET_DCS, SIZE(4), 0xFD, 0x28, 0x3C, 0x00,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xAC, 0x65,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xA7, 0x47,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xA0, 0xDD,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x9A, 0xdf, // 0xda, // 0xd5, // 0xC0,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x9B, 0x5f, // 0x5a, // 0x55, // 0x40,

	_W, DELAY(0), PACKET_DCS, SIZE(3), 0x82, 0x5d, 0x5d, // 0x58, 0x58, // 0x53, 0x53, // 0x3E, 0x3E,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0xB1, 0x10,

	_W, DELAY(0), PACKET_DCS, SIZE(3), 0x7A, 0x13, 0x1A,

	_W, DELAY(0), PACKET_DCS, SIZE(3), 0x7B, 0x13, 0x1A,

	_W, DELAY(0), PACKET_DCS, SIZE(33), 0x6D, 0x00, 0x1F, 0x19, 0x1A, 0x10, 0x0E, 0x0C, 0x0A, 0x02, 0x08, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x07, 0x01, 0x09, 0x0B, 0x0D, 0x0F, 0x1A, 0x19, 0x1F, 0x00,

	_W, DELAY(0), PACKET_DCS, SIZE(17), 0x64, 0x28, 0x04, 0x02, 0x7C, 0x03, 0x03, 0x28, 0x03, 0x02, 0x7D, 0x03, 0x03, 0x7A, 0x7A, 0x7A, 0x7A,

	_W, DELAY(0), PACKET_DCS, SIZE(17), 0x65, 0x28, 0x02, 0x02, 0x7E, 0x03, 0x03, 0x28, 0x01, 0x02, 0x7F, 0x03, 0x03, 0x7A, 0x7A, 0x7A, 0x7A,

	_W, DELAY(0), PACKET_DCS, SIZE(17), 0x66, 0x28, 0x00, 0x02, 0x80, 0x03, 0x03, 0x20, 0x01, 0x02, 0x81, 0x03, 0x03, 0x7A, 0x7A, 0x7A, 0x7A,

	_W, DELAY(0), PACKET_DCS, SIZE(17), 0x67, 0x20, 0x02, 0x02, 0x82, 0x03, 0x03, 0x20, 0x03, 0x02, 0x83, 0x03, 0x03, 0x7A, 0x7A, 0x7A, 0x7A,

	_W, DELAY(0), PACKET_DCS, SIZE(14), 0x68, 0x77, 0x08, 0x06, 0x08, 0x05, 0x7A, 0x7A, 0x08, 0x06, 0x08, 0x05, 0x7A, 0x7A,

	_W, DELAY(0), PACKET_DCS, SIZE(9), 0x60, 0x28, 0x06, 0x7A, 0x7A, 0x28, 0x05, 0x7A, 0x7A,

	_W, DELAY(0), PACKET_DCS, SIZE(9), 0x63, 0x22, 0x7E, 0x7A, 0x7A, 0x22, 0x7F, 0x7A, 0x7A,

	_W, DELAY(0), PACKET_DCS, SIZE(8), 0x69, 0x14, 0x22, 0x14, 0x22, 0x14, 0x22, 0x08,

	_W, DELAY(0), PACKET_DCS, SIZE(2), 0x6B, 0x07,

	_W, DELAY(0), PACKET_DCS, SIZE(53), 0xD1, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x00, 0xF8, 0x01, 0x20, 0x01, 0x4D, 0x01, 0x68, 0x01, 0x88, 0x01, 0xB8, 0x01, 0xF0, 0x02, 0x28, 0x02, 0x7D, 0x02, 0xB8, 0x02, 0xBA, 0x03, 0x00, 0x03, 0x4D, 0x03, 0x6A, 0x03, 0x88, 0x03, 0xA0, 0x03, 0xB5, 0x03, 0xC8, 0x03, 0xD0, 0x03, 0xE8, 0x03, 0xF0, 0x03, 0xF8, 0x03, 0xFF,

	_W, DELAY(0), PACKET_DCS, SIZE(53), 0xD2, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x00, 0xF8, 0x01, 0x20, 0x01, 0x4D, 0x01, 0x68, 0x01, 0x88, 0x01, 0xB8, 0x01, 0xF0, 0x02, 0x28, 0x02, 0x7D, 0x02, 0xB8, 0x02, 0xBA, 0x03, 0x00, 0x03, 0x4D, 0x03, 0x6A, 0x03, 0x88, 0x03, 0xA0, 0x03, 0xB5, 0x03, 0xC8, 0x03, 0xD0, 0x03, 0xE8, 0x03, 0xF0, 0x03, 0xF8, 0x03, 0xFF,

	_W, DELAY(0), PACKET_DCS, SIZE(53), 0xD3, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x00, 0xF8, 0x01, 0x20, 0x01, 0x4D, 0x01, 0x68, 0x01, 0x88, 0x01, 0xB8, 0x01, 0xF0, 0x02, 0x28, 0x02, 0x7D, 0x02, 0xB8, 0x02, 0xBA, 0x03, 0x00, 0x03, 0x4D, 0x03, 0x6A, 0x03, 0x88, 0x03, 0xA0, 0x03, 0xB5, 0x03, 0xC8, 0x03, 0xD0, 0x03, 0xE8, 0x03, 0xF0, 0x03, 0xF8, 0x03, 0xFF,

	_W, DELAY(0), PACKET_DCS, SIZE(53), 0xD4, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x00, 0xF8, 0x01, 0x20, 0x01, 0x4D, 0x01, 0x68, 0x01, 0x88, 0x01, 0xB8, 0x01, 0xF0, 0x02, 0x28, 0x02, 0x7D, 0x02, 0xB8, 0x02, 0xBA, 0x03, 0x00, 0x03, 0x4D, 0x03, 0x6A, 0x03, 0x88, 0x03, 0xA0, 0x03, 0xB5, 0x03, 0xC8, 0x03, 0xD0, 0x03, 0xE8, 0x03, 0xF0, 0x03, 0xF8, 0x03, 0xFF,

	_W, DELAY(0), PACKET_DCS, SIZE(53), 0xD5, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x00, 0xF8, 0x01, 0x20, 0x01, 0x4D, 0x01, 0x68, 0x01, 0x88, 0x01, 0xB8, 0x01, 0xF0, 0x02, 0x28, 0x02, 0x7D, 0x02, 0xB8, 0x02, 0xBA, 0x03, 0x00, 0x03, 0x4D, 0x03, 0x6A, 0x03, 0x88, 0x03, 0xA0, 0x03, 0xB5, 0x03, 0xC8, 0x03, 0xD0, 0x03, 0xE8, 0x03, 0xF0, 0x03, 0xF8, 0x03, 0xFF,

	_W, DELAY(0), PACKET_DCS, SIZE(53), 0xD6, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x00, 0xF8, 0x01, 0x20, 0x01, 0x4D, 0x01, 0x68, 0x01, 0x88, 0x01, 0xB8, 0x01, 0xF0, 0x02, 0x28, 0x02, 0x7D, 0x02, 0xB8, 0x02, 0xBA, 0x03, 0x00, 0x03, 0x4D, 0x03, 0x6A, 0x03, 0x88, 0x03, 0xA0, 0x03, 0xB5, 0x03, 0xC8, 0x03, 0xD0, 0x03, 0xE8, 0x03, 0xF0, 0x03, 0xF8, 0x03, 0xFF,

	_W, DELAY(120), PACKET_DCS, SIZE(1), 0x11,

	_W, DELAY(120), PACKET_DCS, SIZE(1), 0x29,
};

#define real_freq 700

#define freq 327

/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24
#define lane_num 2

#define vsa_line 8
#define vbp_line 30
#define vda_line 640
#define vfp_line 30


#define hsa_pixel  8
#define hbp_pixel  60
#define hda_pixel  480
#define hfp_pixel  60

REGISTER_MIPI_DEVICE_BEGIN(mipi_dev_t) = {
    .info = {
		.x = 56,
		.y = 0,
        .xres 			= LCD_DEV_WIDTH,
        .yres 			= LCD_DEV_HIGHT,
        .target_xres 	= LCD_DEV_WIDTH,
        .target_yres 	= LCD_DEV_HIGHT,

        .buf_addr 		= LCD_DEV_BUF,
        .buf_num 		= LCD_DEV_BNUM,
        .sample         = LCD_DEV_SAMPLE,
        .test_mode 		= 0,
        .test_mode_color = 0x0000ff,
        .canvas_color   = 0xff0000,
        .format 		= FORMAT_RGB888,
        .len 			= LEN_256,
        .interlaced_mode = INTERLACED_NONE,

        .rotate_en 		= false,				// 旋转使能
        .hori_mirror_en = false,		        // 水平镜像使能
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
        .x0_lane = MIPI_LANE_EN | MIPI_LANE_D0,
        .x1_lane = MIPI_LANE_EN | MIPI_LANE_D1,
        .x2_lane = MIPI_LANE_EN | MIPI_LANE_CLK,
        .x3_lane = MIPI_LANE_DIS,//  | MIPI_LANE_D1,
        .x4_lane = MIPI_LANE_DIS,//  | MIPI_LANE_D0,
    },
/*
//同步事件
enum SYNC_MODE {
    SYNC_PULSE_MODE,
    SYNC_EVENT_MODE,
    BURST_MODE,
};
*/
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

        /* .dsi_vdo_bllp0_v = hsa_pixel + hbp_pixel + hda_pixel + hfp_pixel,//?+hda_pixel */


        .dsi_vdo_hsa_v   = ((bpp_num * hsa_pixel) / 8) - 10,
        .dsi_vdo_hbp_v   = ((bpp_num * hbp_pixel) / 8) - 10,
        .dsi_vdo_hact_v  = ((bpp_num * hda_pixel) / 8),
        .dsi_vdo_hfp_v   = ((bpp_num * hfp_pixel) / 8) - 6,

        .dsi_vdo_bllp0_v = ((bpp_num * (hbp_pixel + hda_pixel + hfp_pixel) / 8) - 10),
        .dsi_vdo_bllp1_v = ((bpp_num * hda_pixel) / 8),
    },
    .timing = {
        /* 以下参数只需修改freq */
        .tval_lpx   = ((80     * real_freq / 1000) / 2 - 1),
        .tval_wkup  = ((100000 * real_freq / 1000) / 8 - 1),
        .tval_c_pre = ((40     * real_freq / 1000) / 2 - 1),
        .tval_c_sot = ((300    * real_freq / 1000) / 2 - 1),
        .tval_c_eot = ((100    * real_freq / 1000) / 2 - 1),
        .tval_c_brk = ((150    * real_freq / 1000) / 2 - 1),
        .tval_d_pre = ((60     * real_freq / 1000) / 2 - 1),
        .tval_d_sot = ((160    * real_freq / 1000) / 2 - 1),
        .tval_d_eot = ((100    * real_freq / 1000) / 2 - 1),
        .tval_d_brk = ((150    * real_freq / 1000) / 2 - 1),
        .tval_c_rdy = 400/* 64 */,
    },
    .pll4 = {
        .pll_freq = freq,
        .source = PLL4_SRC_x12M,
    },
    /* .pll_freq = freq * 2,[> 配置PLL频率的最佳范围为600MHz~1.2GHz,少于600MHz的频率通过二分频获得 <] */
    /* .pll_division = MIPI_PLL_DIV2, */

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = 1,
},
REGISTER_MIPI_DEVICE_END()

#if 0
static void read_data_func(void *p)
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
#endif

static int dsi_vdo_2lane_gc9503v_640x360_rgh_rt30hd033a(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u8 lcd_standby = data->lcd_io.lcd_standby;
    /* u8 lcd_lane  = data->lcd_io.lcd_lane; */

    printf("gc9503v_640x360_rgh_rt30hd033a init...\n");
    if ((u8) - 1 != lcd_standby) { /*2 lane*/
        printf("lcd_standby = %d\n", lcd_standby);
        gpio_direction_output(lcd_standby, 0);
    }

    /*
     * lcd reset
     */
    if (-1 != (s8)lcd_reset) {
        printf("chili lcd_reset\n");
//        gpio_direction_output(lcd_reset, 1);
//        delay_2ms(10);
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(10);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(10);
        printf("chili lcd_reset oo\n");
    }

//    dsi_dev_init(&mipi_dev_t);

	#ifdef CONFIG_PWM_BACKLIGHT_ENABLE
    pwm_ch0_backlight_init(data->lcd_io.backlight);
	#endif

    /* os_task_create(read_data_func, NULL, 20, 4000, 0, "read_data_task"); */

    return 0;
}

static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (-1 == (s8)data->lcd_io.backlight) {
        return;
    }
    if (on) {
		#ifdef CONFIG_PWM_BACKLIGHT_ENABLE
        pwm_ch0_backlight_on();
		#else
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
		#endif
    } else {
        #ifdef CONFIG_PWM_BACKLIGHT_ENABLE
        pwm_ch0_backlight_off();
		#else
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
		#endif
    }
}


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .logo 	 = "gc9503v_640x360_rgh_rt30hd033a",
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_2lane_gc9503v_640x360_rgh_rt30hd033a,
    .bl_ctrl = mipi_backlight_ctrl,
    .esd = {

    }
};

#endif
