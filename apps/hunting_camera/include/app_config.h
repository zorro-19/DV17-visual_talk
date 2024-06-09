#ifndef APP_CONFIG_H
#define APP_CONFIG_H




//#define CONFIG_BOARD_DEV_WIFI_5701_20190809
// #define CONFIG_BOARD_DEV_WIFI_5712_20210316
#define CONFIG_HUNTING_CAMERA_5713_20210317





#ifdef CONFIG_BOARD_DEV_WIFI_5712_20210316
#define SDRAM_DDR1
#define SDRAM_SIZE                (16 * 1024 * 1024)

#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
// #define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
// #define CONFIG_UI_ENABLE                        [> UI开关 <]
// #define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
// #define CONFIG_UI_STYLE_JL02_ENABLE             [> UI风格 <]
// #define  LCD_480x272_8BITS					    [> LCD选择 <]
// #define  LCD_480x272_18BITS						[> LCD选择 <]
#endif


#ifdef CONFIG_HUNTING_CAMERA_5713_20210317

#define SDRAM_DDR1
#define SDRAM_SIZE                (32 * 1024 * 1024)

#define CONFIG_SD0_ENABLE                       /* SD卡选择 */
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
//#define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
#define CONFIG_UI_ENABLE                        [> UI开关 <]
#define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
//#define CONFIG_UI_STYLE_JL02_ENABLE             [> UI风格 <]
// #define LCD_DSI_VDO_4LANE_MIPI_EK79030
//#define LCD_DSI_VDO_2LANE_MIPI_ST7701S


#define MULTI_LCD_EN

#ifdef MULTI_LCD_EN
#define LCD_AVOUT
#define LCD_ILI9225G_8BIT_MCU

#else


#define LCD_ILI9225G_8BIT_MCU


#endif




#endif

#ifdef  CONFIG_BOARD_DEV_WIFI_5701_20190809
#define SDRAM_DDR2
#define SDRAM_SIZE                (64 * 1024 * 1024)

#define CONFIG_SD0_ENABLE                       [> SD卡选择 <]
#define CONFIG_VIDEO0_ENABLE 		            [> 前摄像头使能  <]
#define CONFIG_VIDEO1_ENABLE 		            [> 模拟后摄像头使能 <]
// #define CONFIG_VIDEO2_ENABLE 		            [> 模拟后摄像头使能 <]
// #define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
// #define CONFIG_VIDEO4_ENABLE 		            [> 4路使能  <]
#define CONFIG_UI_ENABLE                        [> UI开关 <]
// #define CONFIG_TOUCH_PANEL_ENABLE               [> 触摸屏开关 <]
// #define CONFIG_GT911_B                          [> 触摸屏选择 <]
// #define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
#define CONFIG_UI_STYLE_JL02_ENABLE               [> UI风格 <]
// #define  LCD_480x272_8BITS[> LCD选择 <]
// #define CONFIG_UI_STYLE_LY_LONGSCREEN_ENABLE
#define LCD_DSI_VDO_4LANE_MIPI_EK79030
// #define LCD_AVOUT
// #define MIPI_LCD_1280x720_RM68200GA1
// #define LCD_LVDS_1920x480
// #define CONFIG_PWM_BACKLIGHT_ENABLE            [>PWM 背光使能<]
#endif




#if ((defined CONFIG_VIDEO1_ENABLE) && (defined CONFIG_VIDEO3_ENABLE))
#define THREE_WAY_ENABLE
#endif


//#define CONFIG_SFC_ENABLE
// #define CONFIG_DATABASE_2_RTC                   [> 系统配置存RTC <]
#define CONFIG_DATABASE_2_FLASH                 /* 系统配置存flash */
#define CONFIG_DEBUG_ENABLE                     /* 打印开关 */
//#define CONFIG_PAP_ENABLE                       /* PAP模块使能  */
#define CONFIG_VIDEO_DEC_ENABLE                 /* 视频解码显示  */
#define CONFIG_ADKEY_ENABLE                      /*AD按键开关*/
// #define CONFIG_IOKEY_ENABLE                     [>IO按键开关<]
//#define CONFIG_GSENSOR_ENABLE                   /* 重力感应开关  */
//#define KEY_MODE_EXIT_MENU                      /* UI菜单MODE键退出  */
//#define CONFIG_VE_MOTION_DETECT_MODE_ISP		/* 移动侦测使用MODE_ISP, 否则使用MODE_NORMAL */
// #define CONFIG_EMR_DIR_ENABLE                   [> 紧急录像文件放在单独文件夹 <]
#define CONFIG_DISPLAY_ENABLE                   [> 摄像头显示使能 <]
//#define CONFIG_FILE_PREVIEW_ENABLE              [> 预览图使能 <]
// #define CONFIG_PARK_ENABLE						[> 倒车检测使能 <]
// #define CONFIG_AUTO_TEST						[> 自动测试开关 <]
// #define CONFIG_FAST_CAPTURE						[> 快速启动拍照 <]
//#define PRODUCT_TEST_ENABLE						//产测功能

// #define CONFIG_USB_VIDEO_OUT					// UVC输出

//#define CONFIG_PIP_ENABLE                       //使能画中画
//#define CONFIG_STATIC_IPADDR_ENABLE          //记忆路由器分配的IP,下次直接使用记忆IP节省DHCP时间

#define CONFIG_HUNTING_CAMERA_ENABLE                   /*使能 hunting camera 工程 */
// #define CONFIG_IO_MEASURE_TIMER_ENABLE                   /* IO 测试 卡时间用 */
//#define CONFIG_IO_LED_DISP_ENABLE                   /*PIR 触发LED指示灯 测试*/
//#define CONFIG_MEASURE_SENSOR_FPS_ENABLE                   /*使能RAW sensor 帧率检测*/


//#define RTOS_STACK_CHECK_ENABLE

#define __CAR_CAMERA__         0
#define __WIFI_CAR_CAMERA__    1
#define __WIFI_IPCAM__         2

// #define APP_CASE  __CAR_CAMERA__
#define APP_CASE  __WIFI_CAR_CAMERA__ //TODO  到时要去掉这宏
// #define APP_CASE  __WIFI_IPCAM__




#ifdef CONFIG_PARK_ENABLE
#if defined CONFIG_VIDEO4_ENABLE
#define	CONFIG_VIDEO_PARK_DECT		1
#elif defined CONFIG_VIDEO3_ENABLE
#define	CONFIG_VIDEO_PARK_DECT		3
#elif defined CONFIG_VIDEO1_ENABLE
#define	CONFIG_VIDEO_PARK_DECT		1
#endif
#endif


#ifdef CONFIG_FAST_CAPTURE
#undef CONFIG_PAP_ENABLE
#undef CONFIG_UI_ENABLE
#undef CONFIG_UI_STYLE_JL02_ENABLE
#undef CONFIG_UI_STYLE_LY_ENABLE
#undef CONFIG_TOUCH_PANEL_ENABLE
#undef CONFIG_VIDEO1_ENABLE
#undef CONFIG_VIDEO2_ENABLE
#undef CONFIG_VIDEO3_ENABLE
#undef CONFIG_VIDEO_DEC_ENABLE
#undef CONFIG_ADKEY_ENABLE
#undef CONFIG_IOKEY_ENABLE
#undef CONFIG_PARK_ENABLE
#undef CONFIG_DISPLAY_ENABLE
#undef CONFIG_GSENSOR_ENABLE
#undef CONFIG_USE_UDISK_0
#endif



// #ifdef CONFIG_VIDEO1_ENABLE
// #define CONFIG_AV10_SPI_ENABLE                  [> AV10 SPI开关  <]
// #endif

#define CONFIG_VIDEO_REC_NUM    4

#ifdef CONFIG_SD0_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd0"
#define SDX_DEV					"sd0"
#endif

#ifdef CONFIG_SD1_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd1"
#define SDX_DEV					"sd1"
#endif

#ifdef CONFIG_SD2_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd2"
#define SDX_DEV					"sd2"
#endif

#define CONFIG_REC_DIR_0       "DCIM/1/"
#define CONFIG_REC_DIR_1       "DCIM/2/"
#ifndef CONFIG_VIDEO1_ENABLE
#ifdef CONFIG_VIDEO4_ENABLE
#define CONFIG_REC_DIR_2       "DCIM/3/"
#else
#define CONFIG_REC_DIR_2       "DCIM/2/"
#endif
#else
#define CONFIG_REC_DIR_2       "DCIM/3/"
#endif

#ifdef CONFIG_VIDEO4_ENABLE
#define CONFIG_REC_DIR_3       "DCIM/4/"
#else
#define CONFIG_REC_DIR_3       "DCIM/3/"
#endif


#define CONFIG_ROOT_PATH     	CONFIG_STORAGE_PATH"/C/"
#define CONFIG_REC_PATH_0       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_0
#define CONFIG_REC_PATH_1       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_1
#define CONFIG_REC_PATH_2       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_1
#define CONFIG_REC_PATH_3       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_1

#ifdef CONFIG_EMR_DIR_ENABLE

#define CONFIG_EMR_REC_DIR         "EMR/"
#define CONFIG_EMR_REC_DIR_0       "DCIM/1/"CONFIG_EMR_REC_DIR
#define CONFIG_EMR_REC_DIR_1       "DCIM/2/"CONFIG_EMR_REC_DIR
#ifndef CONFIG_VIDEO1_ENABLE
#ifdef CONFIG_VIDEO4_ENABLE
#define CONFIG_EMR_REC_DIR_2       "DCIM/3/"CONFIG_EMR_REC_DIR
#else
#define CONFIG_EMR_REC_DIR_2       "DCIM/2/"CONFIG_EMR_REC_DIR
#endif
#else
#define CONFIG_EMR_REC_DIR_2       "DCIM/3/"CONFIG_EMR_REC_DIR
#endif
#ifdef CONFIG_VIDEO4_ENABLE
#define CONFIG_EMR_REC_DIR_3       "DCIM/4/"CONFIG_EMR_REC_DIR
#else
#define CONFIG_EMR_REC_DIR_3       "DCIM/3/"CONFIG_EMR_REC_DIR
#endif
#define CONFIG_EMR_REC_PATH_0   CONFIG_STORAGE_PATH"/C/"CONFIG_EMR_REC_DIR_0
#define CONFIG_EMR_REC_PATH_1   CONFIG_STORAGE_PATH"/C/"CONFIG_EMR_REC_DIR_1
#define CONFIG_EMR_REC_PATH_2   CONFIG_STORAGE_PATH"/C/"CONFIG_EMR_REC_DIR_2
#define CONFIG_EMR_REC_PATH_3   CONFIG_STORAGE_PATH"/C/"CONFIG_EMR_REC_DIR_3

#endif


#define CAMERA0_CAP_PATH        CONFIG_REC_PATH_0
#define CAMERA1_CAP_PATH        CONFIG_REC_PATH_1
#define CAMERA2_CAP_PATH        CONFIG_REC_PATH_2
#define CAMERA3_CAP_PATH        CONFIG_REC_PATH_3

#define CONFIG_DEC_ROOT_PATH 	CONFIG_ROOT_PATH"DCIM/"
#define CONFIG_DEC_PATH_1 	    CONFIG_REC_PATH_0
#define CONFIG_DEC_PATH_2 	    CONFIG_REC_PATH_1
#define CONFIG_DEC_PATH_3 	    CONFIG_REC_PATH_2

#define MAX_FILE_NAME_LEN       64
#define FILE_SHOW_NUM           12  /* 一页显示文件数 */



#ifndef __LD__
#include "cpu_config.h"
#include "board.h"
#endif

//*********************************************************************************//
////                                  FCC测试相关配置                                //
////*********************************************************************************//
#ifdef CONFIG_WIFI_ENABLE
//#define RF_FCC_TEST_ENABLE//使能RF_FCC测试，详细配置见"apps/common/rf_fcc_tool/include/rf_fcc_main.h"
#endif





























#endif

