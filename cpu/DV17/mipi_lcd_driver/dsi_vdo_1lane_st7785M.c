#include "generic/typedef.h"
#include "asm/dsi.h"
#include "asm/imd.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"
#include "os/os_compat.h"

#ifdef LCD_DSI_VDO_1LANE_MIPI_ST7785M

//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list


const static u8 init_cmd_list[] =
{




    _W, DELAY(120), PACKET_GENERIC, SIZE(1),0x11,



    _W, DELAY(120), PACKET_GENERIC,   SIZE(9),0xDF,0x00,0x5A,0x00,0x69,0x00,0x02,0x00,0x01,



    _W, DELAY(0), PACKET_GENERIC, SIZE(3),0x36,  0x00,0x00,



    _W, DELAY(0), PACKET_GENERIC, SIZE(3), 0x3A,  0x00,0x66,


    _W, DELAY(0), PACKET_GENERIC, SIZE(3),0xB0,  0x00,0x10,

    _W, DELAY(0), PACKET_GENERIC, SIZE(11),0xB2, 0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x33,0x00,0x33,




    _W, DELAY(0), PACKET_GENERIC, SIZE(3),0xB7,  0x00,0x51,



    _W, DELAY(0), PACKET_GENERIC, SIZE(3), 0xBB,  0x00,0x18,



    _W, DELAY(0), PACKET_GENERIC, SIZE(3),  0xC0, 0x00,0x2C ,


    _W, DELAY(0), PACKET_GENERIC, SIZE(3), 0xC2,  0x00,0x01 ,



    _W, DELAY(0), PACKET_GENERIC, SIZE(3),  0xC3,  0x00, 0x13,




    _W, DELAY(0), PACKET_GENERIC, SIZE(3),  0xC6,  0x00,0x0F,




    _W, DELAY(0), PACKET_GENERIC, SIZE(3),0xD0,  0x00, 0xA7,



    _W, DELAY(0), PACKET_GENERIC, SIZE(5),  0xD0, 0x00, 0xA4,0x00,0xA1,



    _W, DELAY(0), PACKET_GENERIC, SIZE(29), 0xE0, 0x00,0xF0,0x00,0x00,0x00,0x08,0x00,0x04,0x00,0x06,0x00,0x05,0x00,0x34,0x00,0x44,0x00,0x4A,0x00,0x38,0x00,0x13,0x00,0x13,0x00,0x2F,0x00,0x36,




    _W, DELAY(0), PACKET_GENERIC, SIZE(29),0xE1, 0x00,0xF0,0x00,0x0F,00,0x13,0x00,0x11,0x00,0x0E,0x00,0x27,0x00,0x34,0x00,0x44,0x00,0x4A,0x00,0x36,0x00,0x10,0x00,0x10,0x00,0x2C,0x00,0x33,

    _W, DELAY(0), PACKET_GENERIC, SIZE(1), 0x21,

    _W, DELAY(0), PACKET_GENERIC, SIZE(1), 0x29,


};


#define freq 200//407

#define lane_num 1
/*
 *  bpp_num
 *  16: PIXEL_RGB565_COMMAND/PIXEL_RGB565_VIDEO
 *  18: PIXEL_RGB666/PIXEL_RGB666_LOOSELY
 *  24: PIXEL_RGB888
 */
#define bpp_num  24

#if  0

#define vsa_line 4//2
#define vbp_line 8//4
#define vda_line 240//800
#define vfp_line 8//10

#define hsa_pixel  8//8
#define hbp_pixel  20//42
#define hda_pixel  320//480
#define hfp_pixel  40//44

#else


#define vsa_line 12//4//2
#define vbp_line 20//6//4
#define vda_line 320//800
#define vfp_line 32//8//10

#define hsa_pixel 12 //4//8
#define hbp_pixel  30//20//42
#define hda_pixel  240//480
#define hfp_pixel  40//44

#endif

REGISTER_MIPI_DEVICE_BEGIN( mipi_dev_t ) =
{
    .info = {
        .xres           =LCD_DEV_WIDTH,
        .yres           =LCD_DEV_HIGHT,
        .target_xres    =LCD_DEV_WIDTH,
        .target_yres    =LCD_DEV_HIGHT,
        .buf_addr       = LCD_DEV_BUF,
        .buf_num        = LCD_DEV_BNUM,
        .sample         = LCD_DEV_SAMPLE,
        .test_mode      = false,
        .test_mode_color = 0xff0000,
        .canvas_color   = 0xffffff,
        .format         = FORMAT_RGB888,
        .len            = LEN_256,
        .interlaced_mode = INTERLACED_NONE,
    #if 0
        .rotate_en      = false,             // 旋转使能
        .hori_mirror_en = false,             // 水平镜像使能
        .vert_mirror_en = false,            // 垂直镜像使能
    #else
        .rotate_en      = 0,             // 旋转使能
        .hori_mirror_en = 0,             // 水平镜像使能
        .vert_mirror_en = 0,            // 垂直镜像使能
    #endif

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
#if 1
	   .lane_mapping = {

	       #if  0

		   .x0_lane = MIPI_LANE_EN | MIPI_LANE_EX |MIPI_LANE_D0  ,
		   .x1_lane = MIPI_LANE_EN | MIPI_LANE_EX | MIPI_LANE_CLK,
		   .x2_lane = MIPI_LANE_DIS,
		   .x3_lane = MIPI_LANE_DIS,
		   .x4_lane = MIPI_LANE_DIS,
		  #else

            .x0_lane = MIPI_LANE_EN  |MIPI_LANE_D0  ,
		   .x1_lane = MIPI_LANE_EN  | MIPI_LANE_CLK,
		   .x2_lane = MIPI_LANE_DIS,
		   .x3_lane = MIPI_LANE_DIS,
		   .x4_lane = MIPI_LANE_DIS,

		  #endif
	   },
#else
		  .lane_mapping = {
		   //develop board
		   .x0_lane = MIPI_LANE_EN	| MIPI_LANE_CLK,//MIPI_LANE_EN	| MIPI_LANE_D0,
		   .x1_lane = MIPI_LANE_EN	| MIPI_LANE_D0,//MIPI_LANE_EN  | MIPI_LANE_CLK,
		   .x2_lane = MIPI_LANE_DIS,
		   .x3_lane = MIPI_LANE_DIS,
		   .x4_lane = MIPI_LANE_DIS,

		   //样机
		   /* .x0_lane = MIPI_LANE_EN  | MIPI_LANE_D3, */
		   /* .x1_lane = MIPI_LANE_EN  | MIPI_LANE_D2, */
		   /* .x2_lane = MIPI_LANE_EN  | MIPI_LANE_CLK, */
		   /* .x3_lane = MIPI_LANE_EN  | MIPI_LANE_D1, */
		   /* .x4_lane = MIPI_LANE_EN  | MIPI_LANE_D0, */
	   },
#endif

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
    /* .pll_freq = freq * 2,[> 配置PLL频率的最佳范围为600MHz~1.2GHz,少于600MHz的频率通过二分频获得 <] */
    /* .pll_division = MIPI_PLL_DIV2, */

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = true,
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

static int dsi_vdo_4lane_st7701s_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;
    u8 lcd_standby = data->lcd_io.lcd_standby;
    /* u8 lcd_lane  = data->lcd_io.lcd_lane; */

    printf("ST7701S init...\n");
    if ((u8) - 1 != lcd_standby) { /*2 lane*/
        printf("lcd_standby = %d\n", lcd_standby);
        gpio_direction_output(lcd_standby, 0);
    }

    /*
     * lcd reset
     */
    if (-1 != (s8)lcd_reset) {
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(5);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(5);
    }

    dsi_dev_init(&mipi_dev_t);

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
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}


REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .logo 	 = "ST7785",
    .type 	 = LCD_MIPI,
    .dev  	 = &mipi_dev_t,
    .init 	 = dsi_vdo_4lane_st7701s_init,
    .bl_ctrl = mipi_backlight_ctrl,
    .esd = {

    }
};

#endif
