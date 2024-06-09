
#ifndef APP_CONFIG_H
#define APP_CONFIG_H




//#define CONFIG_BOARD_DEV_WIFI_5701_20190809
// #define CONFIG_BOARD_DEV_WIFI_5712_20210316
//#define CONFIG_BOARD_DEV_WIFI_5713_20210317
// #define CONFIG_BOARD_DEV_5713_20210901
//#define CONFIG_BOARD_DOORBELL_WIFI_5713_20220325
//#define CONFIG_BOARD_DEV_5713_20210901
//#define CONFIG_BOARD_DEV_WIFI_5701_20211230
//#define CONFIG_BOARD_DEV_WIFI_5713_20210408
//#define CONFIG_BOARD_DEV_WIFI_5713_20221027

//#define CONFIG_BOARD_DEV_WIFI_5713_20230825
//#define XD_CONFIG_BOARD_DEV_WIFI_5713_20231208
#define LP_CONFIG_BOARD_DEV_WIFI_5713_20240426


#ifdef XD_CONFIG_BOARD_DEV_WIFI_5713_20231208

#define SDRAM_DDR1
#define SDRAM_SIZE                (32 * 1024 * 1024)

#define __CPU_AC571X__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
// #define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
#define CONFIG_UI_ENABLE                        [> UI开关 <]
#define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
//#define CONFIG_UI_STYLE_JL02_ENABLE             [> UI风格 <]
//#define LCD_DSI_VDO_4LANE_MIPI_EK79030
//#define LCD_DSI_VDO_2LANE_MIPI_EK79030
#define LCD_DSI_VDO_1LANE_MIPI_ST7785M
//#define LCD_DSI_VDO_2LANE_MIPI_GC9503V_640X360_RGH_RT30HD033A
//#define LCD_DSI_DCS_ST7785M_1LANE_24BIT
//#define LCD_WT400X400

#define LONG_POWER_IPC  // 长电工程IPC 宏
#define  IPC_DEVELOP_BOARD_DEMO  //develop demo

#endif


#ifdef LP_CONFIG_BOARD_DEV_WIFI_5713_20240426

#define SDRAM_DDR1
#define SDRAM_SIZE                (32 * 1024 * 1024)

#define __CPU_AC571X__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
// #define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
#define CONFIG_UI_ENABLE                        [> UI开关 <]
#define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
//#define CONFIG_UI_STYLE_JL02_ENABLE             [> UI风格 <]
//#define LCD_DSI_VDO_4LANE_MIPI_EK79030
//#define LCD_DSI_VDO_2LANE_MIPI_EK79030
//#define LCD_DSI_VDO_1LANE_MIPI_ST7785M
#define LCD_ST7789P3_MCU
//#define LCD_DSI_VDO_2LANE_MIPI_GC9503V_640X360_RGH_RT30HD033A
//#define LCD_DSI_DCS_ST7785M_1LANE_24BIT
//#define LCD_WT400X400

#define  LONG_POWER_IPC  // 长电工程IPC 宏
#define  IPC_DEVELOP_BOARD_DEMO  //develop demo
#define  IPC_50_NUMBER_TEST_DEMO  //50tai 2336p

#endif


#ifdef CONFIG_BOARD_DEV_WIFI_5713_20230825
#define SDRAM_DDR1
#define SDRAM_SIZE                (32 * 1024 * 1024)


#define __CPU_AC571X__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
// #define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
#define CONFIG_UI_ENABLE                        [> UI开关 <]
#define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
//#define CONFIG_UI_STYLE_JL02_ENABLE             [> UI风格 <]
// #define LCD_DSI_VDO_4LANE_MIPI_EK79030
// #define LCD_DSI_VDO_2LANE_MIPI_EK79030
#define LCD_DSI_VDO_2LANE_MIPI_GC9503V_640X360_RGH_RT30HD033A
#define LONG_POWER_IPC  // 长电工程IPC 宏

#endif


#ifdef  CONFIG_BOARD_DEV_WIFI_5713_20221027
#define SDRAM_DDR1
#define SDRAM_SIZE                (32 * 1024 * 1024)
#define __CPU_DV17__                            /* CPU */
#define CONFIG_SD0_ENABLE                       [> SD卡选择 <]
#define CONFIG_VIDEO0_ENABLE 		            [> 前摄像头使能  <]
//#define CONFIG_VIDEO1_ENABLE 		            [> 模拟后摄像头使能 <]
// #define CONFIG_VIDEO2_ENABLE 		            [> 模拟后摄像头使能 <]
//#define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
// #define CONFIG_VIDEO4_ENABLE 		            [> 4路使能  <]
#define CONFIG_UI_ENABLE                        [> UI开关 <]
// #define CONFIG_TOUCH_PANEL_ENABLE               [> 触摸屏开关 <]
// #define CONFIG_GT911_B                          [> 触摸屏选择 <]
 #define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
//#define CONFIG_UI_STYLE_JL02_ENABLE               [> UI风格 <]
// #define  LCD_480x272_8BITS[> LCD选择 <]
// #define CONFIG_UI_STYLE_LY_LONGSCREEN_ENABLE
//#define LCD_DSI_VDO_4LANE_MIPI_EK79030
//#define LCD_MIPI_ILI9806E_ST0397S_397IPS
#define LCD_DSI_VDO_2LANE_MIPI_ST7701S
// #define LCD_AVOUT
// #define MIPI_LCD_1280x720_RM68200GA1
// #define LCD_LVDS_1920x480
// #define CONFIG_PWM_BACKLIGHT_ENABLE            [>PWM 背光使能<]
#define CONFIG_WIFI_HI3861L_ENABLE
#endif


#ifdef CONFIG_BOARD_DEV_5713_20210901

#define SDRAM_DDR1

#define SDRAM_SIZE                (32 * 1024 * 1024)


#define __CPU_AC571X__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
// #define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
//#define CONFIG_UI_ENABLE                        [> UI开关 <]
// #define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
//#define CONFIG_UI_STYLE_JL02_ENABLE             [> UI风格 <]
// #define LCD_DSI_VDO_4LANE_MIPI_EK79030
// #define LCD_DSI_VDO_2LANE_MIPI_EK79030
//#define LCD_DSI_VDO_2LANE_MIPI_ST7701S
#define CONFIG_WIFI_HI3861L_ENABLE
#endif

#ifdef  CONFIG_BOARD_DEV_WIFI_5701_20211230
#define SDRAM_DDR2
#define SDRAM_SIZE                (64 * 1024 * 1024)
#define __CPU_DV17__                            /* CPU */
#define CONFIG_SD0_ENABLE                       [> SD卡选择 <]
//#define CONFIG_VIDEO0_ENABLE 		            [> 前摄像头使能  <]
//#define CONFIG_VIDEO1_ENABLE 		            [> 模拟后摄像头使能 <]
// #define CONFIG_VIDEO2_ENABLE 		            [> 模拟后摄像头使能 <]
#define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
// #define CONFIG_VIDEO4_ENABLE 		            [> 4路使能  <]
#define CONFIG_UI_ENABLE                        [> UI开关 <]
// #define CONFIG_TOUCH_PANEL_ENABLE               [> 触摸屏开关 <]
// #define CONFIG_GT911_B                          [> 触摸屏选择 <]
#define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
//#define CONFIG_UI_STYLE_JL02_ENABLE               [> UI风格 <]
// #define  LCD_480x272_8BITS[> LCD选择 <]
// #define CONFIG_UI_STYLE_LY_LONGSCREEN_ENABLE
//#define LCD_DSI_VDO_4LANE_MIPI_EK79030
#define LCD_DSI_VDO_2LANE_MIPI_ST7701S
// #define LCD_AVOUT
// #define MIPI_LCD_1280x720_RM68200GA1
// #define LCD_LVDS_1920x480
// #define CONFIG_PWM_BACKLIGHT_ENABLE            [>PWM 背光使能<]
#endif

#ifdef  CONFIG_BOARD_DEV_WIFI_5713_20210408
#define SDRAM_DDR1_ET
#define SDRAM_SIZE                (32 * 1024 * 1024)
#define __CPU_DV17__                            /* CPU */
#define CONFIG_SD0_ENABLE                       [> SD卡选择 <]
//#define CONFIG_VIDEO0_ENABLE 		            [> 前摄像头使能  <]
//#define CONFIG_VIDEO1_ENABLE 		            [> 模拟后摄像头使能 <]
// #define CONFIG_VIDEO2_ENABLE 		            [> 模拟后摄像头使能 <]
#define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
// #define CONFIG_VIDEO4_ENABLE 		            [> 4路使能  <]
//#define CONFIG_UI_ENABLE                        [> UI开关 <]
// #define CONFIG_TOUCH_PANEL_ENABLE               [> 触摸屏开关 <]
// #define CONFIG_GT911_B                          [> 触摸屏选择 <]
// #define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
//#define CONFIG_UI_STYLE_JL02_ENABLE               [> UI风格 <]
// #define  LCD_480x272_8BITS[> LCD选择 <]
// #define CONFIG_UI_STYLE_LY_LONGSCREEN_ENABLE
//#define LCD_DSI_VDO_4LANE_MIPI_EK79030
#define LCD_MIPI_ILI9806E_ST0397S_397IPS
//#define LCD_DSI_VDO_2LANE_MIPI_ST7701S
// #define LCD_AVOUT
// #define MIPI_LCD_1280x720_RM68200GA1
// #define LCD_LVDS_1920x480
// #define CONFIG_PWM_BACKLIGHT_ENABLE            [>PWM 背光使能<]
#endif





#ifdef CONFIG_BOARD_DOORBELL_WIFI_5713_20220325
#define SDRAM_DDR1_ET
#define SDRAM_SIZE                (16 * 1024 * 1024)


#define __CPU_AC571X__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
// #define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
//#define CONFIG_UI_ENABLE                        [> UI开关 <]
// #define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
//#define CONFIG_UI_STYLE_JL02_ENABLE             [> UI风格 <]
// #define LCD_DSI_VDO_4LANE_MIPI_EK79030
// #define LCD_DSI_VDO_2LANE_MIPI_EK79030
//#define LCD_DSI_VDO_2LANE_MIPI_ST7701S
#endif


#ifdef CONFIG_BOARD_DEV_WIFI_5712_20210316
#define SDRAM_DDR1
#define SDRAM_SIZE                (16 * 1024 * 1024)

#define __CPU_AC571X__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
// #define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
// #define CONFIG_UI_ENABLE                        [> UI开关 <]
// #define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
// #define CONFIG_UI_STYLE_JL02_ENABLE             [> UI风格 <]
// #define  LCD_480x272_8BITS						[> LCD选择 <]
// #define  LCD_480x272_18BITS						[> LCD选择 <]
#endif


#ifdef CONFIG_BOARD_DEV_WIFI_5713_20210317
#define SDRAM_DDR1
#define SDRAM_SIZE                (32 * 1024 * 1024)


#define __CPU_AC571X__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
// #define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
#define CONFIG_UI_ENABLE                        [> UI开关 <]
#define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
//#define CONFIG_UI_STYLE_JL02_ENABLE             [> UI风格 <]
// #define LCD_DSI_VDO_4LANE_MIPI_EK79030
// #define LCD_DSI_VDO_2LANE_MIPI_EK79030
#define LCD_DSI_VDO_2LANE_MIPI_ST7701S
#endif





#ifdef  CONFIG_BOARD_DEV_WIFI_5701_20190809
#define SDRAM_DDR2
#define SDRAM_SIZE                (64 * 1024 * 1024)

#define __CPU_DV17__                            /* CPU */
#define CONFIG_SD0_ENABLE                       [> SD卡选择 <]
//#define CONFIG_VIDEO0_ENABLE 		            [> 前摄像头使能  <]
//#define CONFIG_VIDEO1_ENABLE 		            [> 模拟后摄像头使能 <]
// #define CONFIG_VIDEO2_ENABLE 		            [> 模拟后摄像头使能 <]
#define CONFIG_VIDEO3_ENABLE 		            [> UVC后摄像头使能 <]
// #define CONFIG_VIDEO4_ENABLE 		            [> 4路使能  <]
//#define CONFIG_UI_ENABLE                        [> UI开关 <]
// #define CONFIG_TOUCH_PANEL_ENABLE               [> 触摸屏开关 <]
// #define CONFIG_GT911_B                          [> 触摸屏选择 <]
// #define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
//#define CONFIG_UI_STYLE_JL02_ENABLE               [> UI风格 <]
// #define  LCD_480x272_8BITS[> LCD选择 <]
// #define CONFIG_UI_STYLE_LY_LONGSCREEN_ENABLE
//#define LCD_DSI_VDO_4LANE_MIPI_EK79030
// #define LCD_AVOUT
#define MIPI_LCD_1280x720_RM68200GA1
// #define LCD_LVDS_1920x480
// #define CONFIG_PWM_BACKLIGHT_ENABLE            [>PWM 背光使能<]
#endif




#if ((defined CONFIG_VIDEO1_ENABLE) && (defined CONFIG_VIDEO3_ENABLE))
#define THREE_WAY_ENABLE
#endif


//#define CONFIG_SFC_ENABLE                       /*使能系统跑SFC模式*/
//#define CONFIG_MPU_ENABLE

// #define CONFIG_DATABASE_2_RTC                   [> 系统配置存RTC <]
#define CONFIG_DATABASE_2_FLASH                 /* 系统配置存flash */
#define CONFIG_DEBUG_ENABLE                     /* 打印开关 */
#define CONFIG_PAP_ENABLE                       /* PAP模块使能  */
#define CONFIG_VIDEO_DEC_ENABLE                 /* 视频解码显示  */
#ifdef LP_CONFIG_BOARD_DEV_WIFI_5713_20240426
#define CONFIG_ADKEY_ENABLE                      /*AD按键开关*/
#endif
#ifdef XD_CONFIG_BOARD_DEV_WIFI_5713_20231208
#define CONFIG_IOKEY_ENABLE                     [>IO按键开关<]
#endif
//#define CONFIG_GSENSOR_ENABLE                   /* 重力感应开关  */
//#define KEY_MODE_EXIT_MENU                      /* UI菜单MODE键退出  */
#define CONFIG_VE_MOTION_DETECT_MODE_ISP		/* 移动侦测使用MODE_ISP, 否则使用MODE_NORMAL */
// #define CONFIG_EMR_DIR_ENABLE                   [> 紧急录像文件放在单独文件夹 <]
#define CONFIG_DISPLAY_ENABLE                   /* 摄像头显示使能 */
#define CONFIG_FILE_PREVIEW_ENABLE              [> 预览图使能 <]
// #define CONFIG_PARK_ENABLE						[> 倒车检测使能 <]
// #define CONFIG_AUTO_TEST						[> 自动测试开关 <]
//#define CONFIG_FAST_CAPTURE						[> 快速启动拍照 <]

//#define CONFIG_USE_UDISK_0                      #<{(| 使用U盘驱动port0, port指的是拓扑结构中的接口号，硬件上usb0和usb1共用 |)}>#
// #define CONFIG_USB_VIDEO_OUT					// UVC输出

//#define CONFIG_PIP_ENABLE                       //使能画中画
#define PRODUCT_TEST_ENABLE                    //产测功能使能

#define ENABLE_CONTROL_USB_SEPAK_MUTE  //使能从机 mute控制
#define MOTOR_AUTO_CALIBRATION      //使能上电自动电机校准

#ifdef LONG_POWER_IPC
#define CONFIG_STATIC_IPADDR_ENABLE          //记忆路由器分配的IP,下次直接使用记忆IP节省DHCP时间
#endif


#define  USER_UART_UPDATE_ENABLE
#define  UART_UPDATE_ROLE           UART_UPDATE_MASTER


#ifdef CONFIG_UI_ENABLE
#define  CONFIG_IPC_UI_ENABLE               //可视对讲UI逻辑
#endif // CONFIG_UI_ENABLE


#define __CAR_CAMERA__         0
#define __WIFI_CAR_CAMERA__    1
#define __WIFI_IPCAM__         2

// #define APP_CASE  __CAR_CAMERA__
// #define APP_CASE  __WIFI_CAR_CAMERA__ //TODO  到时要去掉这宏
#define APP_CASE  __WIFI_IPCAM__




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
#ifndef CONFIG_UI_ENABLE
#undef CONFIG_PAP_ENABLE
#undef CONFIG_UI_ENABLE
#undef CONFIG_UI_STYLE_JL02_ENABLE
#undef CONFIG_UI_STYLE_LY_ENABLE
#undef CONFIG_TOUCH_PANEL_ENABLE
//#undef CONFIG_DISPLAY_ENABLE
#endif // CONFIG_UI_ENABLE

//#undef CONFIG_VIDEO_DEC_ENABLE
//#undef CONFIG_ADKEY_ENABLE
//#undef CONFIG_IOKEY_ENABLE
#undef CONFIG_PARK_ENABLE
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

//#ifndef CONFIG_VIDEO1_ENABLE
//#ifdef CONFIG_VIDEO4_ENABLE
#define CONFIG_REC_DIR_2       "DCIM/3/"
//#else
//#define CONFIG_REC_DIR_2       "DCIM/2/"
//#endif
//#else
//#define CONFIG_REC_DIR_2       "DCIM/3/"
//#endif

//#ifdef CONFIG_VIDEO4_ENABLE
#define CONFIG_REC_DIR_3       "DCIM/4/"
//#else
//#define CONFIG_REC_DIR_3       "DCIM/3/"
//#endif

#define CONFIG_ROOT_PATH_DID    CONFIG_STORAGE_PATH"/C/CDID/"
#define CONFIG_ROOT_PATH     	CONFIG_STORAGE_PATH"/C/"
#define CONFIG_REC_PATH_0       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_0
#define CONFIG_REC_PATH_1       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_1
#define CONFIG_REC_PATH_2       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_2
#define CONFIG_REC_PATH_3       CONFIG_STORAGE_PATH"/C/"CONFIG_REC_DIR_3

#define  ENTER_PRODUCT_MODE_FILE_NAME     "product.bin"  // 产测识别文件
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


#define CONFIG_NETCONFIG_TIMEOUT        3 * 60 * 1000  //3分钟
#define CONFIG_PLATFORM_CONNECT_TIMEOUT   10 * 1000    //10s
#define CONFIG_STA_CONNECT_TIMEOUT      60 * 1000  //3分钟

//#define CONFIG_UI_WAIT_TIMEOUT      3 * 60 * 1000  //长时间未操作返回时间：3分钟
#define CONFIG_UI_WAIT_TIMEOUT      10 * 1000  //长时间未操作返回时间：3分钟


#define GET_VIDEO_WIDTH             1920  //写卡录像分辨率
#define GET_VIDEO_HEIGHT            1088

#ifdef CONFIG_VIDEO3_ENABLE

#define CONFIG_USB_UVC_AND_UAC_ENABLE
#define CONFIG_ONLY_DNS_ENABLE   //仅使能环境降噪
#define CONFIG_UVCX_ENABLE   //从机采用多UVC设备

#ifdef CONFIG_UVCX_ENABLE

//#define CONFIG_NET_DUAL_STREAM_ENABLE


#define CONFIG_NO_UVC_DEVICE        0
#define CONFIG_UVC0_DEVICE      BIT(0)
#define CONFIG_UVC1_DEVICE      BIT(1)
#define CONFIG_UVC2_DEVICE      BIT(2)
#define CONFIG_UVC3_DEVICE      BIT(3)

#define CONFIG_NET_STREAM_DEVICE   (CONFIG_UVC0_DEVICE)     //选择网络数据采用那一路uvc的数据
#define CONFIG_DISP_DEVICE         (CONFIG_NO_UVC_DEVICE)


#endif // CONFIG_UVCX_ENABLE

//#define CONFIG_UVCX_TEST_ENABLE                //UVCX测试使能

#else

#define CONFIG_AC7016_ENABLE
#ifdef CONFIG_AC7016_ENABLE
#define CONFIG_USB_UVC_AND_UAC_ENABLE
#endif
//#define CONFIG_AEC_DNS_ENABLE  //使能回声消除

#endif // CONFIG_VIDEO3_ENABLE



#if (defined CONFIG_AEC_DNS_ENABLE || defined CONFIG_ONLY_DNS_ENABLE)
#define CONFIG_MPU_ENABLE
#endif

//#define CONFIG_IPERF_ENABLE    //使能iperf测试
//#define MP_TEST_ENABLE
//#define AP_TEST_ENABLE    //使能AP模式测试


//#define CONFIG_SHARED_REC_HDL_ENABLE    //定义共用录像句柄
//#define RTOS_STACK_CHECK_ENABLE
//#define CONFIG_AEC_DNS_TEST_ENABLE
#define CONFIG_MASS_PRODUCTION_ENABLE
#define CONFIG_AUTO_ISP_SCENES_ENABLE
//#define FACE_DECT_ENABLE
//#define ENABLE_VE_ENGINER

//#define ENABLE_SAVE_FILE_FOR_AEC_DNS


//#define CONFIG_ISP_PCCAMERA_MODE_ENABLE  // ISP 调试模式
//#define CONFIG_SCREEN_LCD_DEBUG_MODE_ENABLE


//#define CONFIG_OSD_DISPLAY_CUR_SENCE
#define CONFIG_CLOUD_STORAGE_NUM       1
#define CONFIG_IRCUT_ENABLE

#define CONFIG_USB_UAC_DEBUG

#ifdef CONFIG_IPC_UI_ENABLE
#define CONFIG_VIDEO_TALK_PATH_1 	    CONFIG_ROOT_PATH"DCIM/1/"
#define CONFIG_VIDEO_TALK_PATH_2 	    CONFIG_ROOT_PATH"DCIM/2/"
#define CONFIG_VIDEO_TALK_PATH_3 	    CONFIG_ROOT_PATH"DCIM/3/"
#define CONFIG_VIDEO_TALK_PATH_4 	    CONFIG_ROOT_PATH"DCIM/4/"
#endif //

#ifdef CONFIG_SHARED_REC_HDL_ENABLE

#define CONFIG_H264_STD_HEAD_ENABLE 0

#else

#define CONFIG_H264_STD_HEAD_ENABLE 1

#endif



#ifdef CONFIG_VIDEO0_ENABLE
//#define CONFIG_VIDEO0_INIT_ASYNC
#endif // CONFIG_VIDEO0_ENABLE

#define CONFIG_NET_SUB_ID        1
#define CONFIG_LOCAL_MSG_SUB_ID 2

/***=== 定义对讲的音频格式 ===***/
#define AUDIO_FORMAT_AAC   0
#define AUDIO_FORMAT_PCM   1

#ifndef CONFIG_SHARED_REC_HDL_ENABLE

#define CONFIG_AUDIO_FORMAT_FOR_MIC         AUDIO_FORMAT_AAC
#else
#define CONFIG_AUDIO_FORMAT_FOR_MIC         AUDIO_FORMAT_PCM
#endif


#define CONFIG_AUDIO_FORMAT_FOR_SPEARK      AUDIO_FORMAT_AAC

#define CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC       16000
#define CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK    16000        //为了兼容小草小程序AAC只能编码48k的音频，统一设置为48k


#if (CONFIG_AUDIO_FORMAT_FOR_MIC ==  AUDIO_FORMAT_AAC && defined CONFIG_SHARED_REC_HDL_ENABLE)
#error "When the shared handle is enabled, the mic record AAC function cannot be used"
#endif





//#define  DOORBELL_VERSION       "20240326_V0.0001_test1"
//#define  DOORBELL_VERSION       "20240326_LPV0.0001_test1"
//#define  DOORBELL_VERSION       "20240415_LP1.3V.0001_test1"
#define    DOORBELL_VERSION       "20240524_V1.3.0001_test"





#ifndef __LD__
#include "cpu_config.h"
#include "board.h"
#endif










#endif

