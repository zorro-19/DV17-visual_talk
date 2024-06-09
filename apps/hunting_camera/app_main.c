#include "system/includes.h"
#include "server/ui_server.h"
#include "action.h"
#include "ani_style.h"
#include "style.h"
#include "res_ver.h"
#include "app_config.h"
#include "gSensor_manage.h"
#include "video_rec.h"
#include "asm/rtc.h"
#include "storage_device.h"
#include "system/spinlock.h"
#include "asm/imc_driver.h"
#include "asm/mpu.h"
#include "key_commucation.h"
#include "app_ui_index.h"

#include "isp_effect_data.h"
#include "deal_function_state.h"

#define LOG_TAG "app_main"
#include "generic/log.h"
#if (APP_CASE == __WIFI_CAR_CAMERA__)
#include "server/ctp_server.h"
#include "server/net_server.h"
#include "net_config.h"
#endif
#if (APP_CASE == __WIFI_IPCAM__)
#include "device/wifi_dev.h"
#endif


u32 spin_lock_cnt[2] = {0};

int upgrade_detect(const char *sdcard_name);


/*任务列表 */
const struct task_info task_info_table[] = {
    {"poweroff",            31,     1024,   1024  },
    {"video_fb",            30,     1024,   1024  },
    {"ui_server",           24,     1024 * 2,   1024  },
    {"ui_task",             25,     4096 * 2,   512   },
    {"init",                30,     1024 * 2,   256   },
    {"sys_timer",           10,     512,        2048  },//qsize必须大于204
    {"app_core",            20,     2048 * 2,   4096},
    {"sys_event",           30,     1024 * 2,   0     },
    {"video_server",        27,     2048,   1024   },
    {"video_server_transfer",        28,     2048,   128},
    {"audio_server",        16,     1024,   256   },
    {"audio_decoder",       17,     1024,   0     },
    {"video_dec_server",    27,     1024,   1024  },
    {"video_preview_server", 27,     1024,   1024  },
    {"video0_rec0",         22,     2048,   512   },
    {"video0_rec1",         21,     4096,   512   },
    {"video0_rec2",         21,     2048,   512   },
    {"video0_rec3",         21,     2048,   512   },
    {"video0_rec4",         21,     2048,   512   },
    {"audio0_rec0",          24,     2048,   256   },
    {"audio0_rec1",          24,     2048,   256   },
    {"audio1_rec0",          24,     2048,   256   },
    {"audio1_rec1",          24,     2048,   256   },

    {"audio2_rec0",          24,     2048,   256   },
    {"audio2_rec1",          24,     2048,   256   },
    {"audio3_rec0",          24,     2048,   256   },
    {"audio3_rec1",          24,     2048,   256   },


    {"video1_rec0",         19,     2048,   512   },
    {"video1_rec1",         19,     2048,   256   },
    {"video2_rec0",         19,     2048,   512   },
    {"video3_rec0",         19,     2048,   512   },
    {"video3_rec1",         19,     2048,   512   },
    /*{"video3_rec2",         27,     2048,   256   },*/
    {"jlc_rec3",            18,     2048,   256   },
    {"isp_update",          27,     1024,   0     },
    {"vpkg_server",         26,     2048,   1024 * 2},
    {"vunpkg_server",       23,     1024,   128   },
    {"avi0",                29,     2048,   64    },
    {"avi1",                29,     2048,   64    },
    {"avi2",                29,     2048,   64    },
    {"mov0",                28,     2048,   64    },
    {"mov1",                28,     2048,   64    },
    {"mov2",                28,     2048,   64    },
    {"mov3",                28,     2028,   64    },
    {"video_engine_server", 14,     1024,   1024  },
    {"video_engine_task",   15,     2048,   0     },
    {"usb_server",          20,     2048,   128   },
    {"khubd",               25,     1024,   512    },

    {"uvc_transceiver",     26,     2048,   32    },
    {"uvc_transceiver1",    26,     2048,   32    },
    {"vimc_scale",          26,     2048,   32    },

    {"upgrade_core",        20,     1024,   32    },
    {"upgrade_server",      21,     1024,   32    },

    {"dynamic_huffman0",    15,		1024,	32    },
    {"dynamic_huffman1",    15,		1024,	32    },

    {"video0.0_sched",               15,     2048,   512    },
    {"video0.1_sched",               15,     2048,   512    },
    {"video1.0_sched",               15,     2048,   512    },
    {"video1.1_sched",               15,     2048,   512    },

    {"edis0",               26,     2048,   32    },
    {"edis0_as",            27,     2048,   32    },
    {"edis1",               26,     2048,   32    },
    {"edis1_as",            27,     2048,   32    },
    {"edis2",               26,     2048,   32    },
    {"edis2_as",            27,     2048,   32    },
    {"edis3",               26,     2048,   32    },
    {"edis3_as",            27,     2048,   32    },


    {"video4_rec0",         22,     2048,   512   },
    {"video4_rec1",         22,     2048,   512   },

    {"powerdet_task",       15,     1024,   1024  },
    /* {"audio_task",			15,		1024,	128	  }, */

    {"user_draw",           15,     1024,   1024  },
    {"user_audio",           15,     1024,   1024  },

#if (APP_CASE == __WIFI_CAR_CAMERA__)

    {"ctp_server",          27,     1024,   1024  },
    {"net_video_server",    27,     1024,   1024  },
#endif
    {"imr_rotate_task",     20,     2048,   32    },


    {0, 0},
};

void __attribute__((weak)) system_shutdown(u8 force)
{

}


#ifdef CONFIG_UI_ENABLE
/*
 * 开机动画播放完毕
 */
static void animation_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;

    server_close(ui);

    /*
     * 显示完开机画面后更新配置文件,避免效果调节过度导致开机图片偏色
     */
    void *imd = dev_open("imd", NULL);
    if (imd) {
        dev_ioctl(imd, IMD_SET_COLOR_CFG, (u32)"scr_auto.bin"); /* 更新配置文件  */
        dev_close(imd);
    }

    /*
     *按键消息使能
     */
#ifdef CONFIG_PARK_ENABLE
    if (!get_parking_status())
#endif
    {
        sys_key_event_enable();
        sys_touch_event_enable();//使能触摸事件
    }
}



#endif



/*
 * 关机动画播放完毕, 关闭电源
 */
static void power_off_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;
    u32 park_en;

    if (ui) {
        server_close(ui);
    }

#ifdef CONFIG_GSENSOR_ENABLE
    park_en = db_select("par");
    set_parking_guard_wkpu(park_en);
    sys_power_set_port_wakeup("wkup_gsen", park_en);
#endif
    sys_power_set_port_wakeup("wkup_usb", 1);

//    lowbat_power_off();
    close_pir_power_off(); // 低电关机需要关闭pir 唤醒，否则会开机
    sys_power_poweroff(0);

}

#ifdef MULTI_LCD_EN
extern int ui_platform_init();
static void switch_lcd()
{
    struct intent it;
    struct application *app;
    static u8 sw = 0;

    void *lcd_dev = lcd_get_cur_hdl();
    app = get_current_app();
    init_intent(&it);
    if (app) {
        it.name = app->name;
        it.action = ACTION_BACK;
        start_app(&it);
    }

    if (lcd_dev) {
        dev_close(lcd_dev);
    }

    if (sw == 0) {
        lcd_dev = dev_open("lcd", "lcd_avout");
    } else {
        lcd_dev = dev_open("lcd", "ST7789S_mcu");
    }
    sw = !sw;

    ui_platform_init();
    if (app) {
        it.name = app->name;
        if (!strcmp(app->name, "video_rec")) {
            it.action = ACTION_VIDEO_REC_MAIN;
        } else if (!strcmp(app->name, "video_dec")) {
            it.action = ACTION_VIDEO_DEC_MAIN;
        } else if (!strcmp(app->name, "video_photo")) {
            it.action = ACTION_PHOTO_TAKE_MAIN;
        }
        start_app(&it);
    }
}
#endif
u8 at_rec_start_key()
{

    struct sys_event event;
    puts("at_rec_start_key.\n");

    event.type = SYS_KEY_EVENT;
    event.u.key.event = KEY_EVENT_CLICK;
    event.u.key.value = KEY_REC_START;
    sys_event_notify(&event);

    return 0;
}
static int main_key_event_handler(struct key_event *key)
{
    struct intent it;
    struct application *app;

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
#if 0//def MULTI_LCD_EN
        case KEY_F1:
            switch_lcd();/*双屏切换*/
            break;
#endif

        case KEY_MODE:

            malloc_stats();

            init_intent(&it);
            app = get_current_app();

            if (app) {
                if (!strcmp(app->name, "usb_app")) {
                    break;
                }
                it.action = ACTION_BACK;
                start_app(&it);
#if  0
                if (!strcmp(app->name, "video_rec") || !strcmp(app->name, "video_dec")) {
                    it.name = "video_photo";
                    // it.action = ACTION_PHOTO_TAKE_MAIN;
                    it.action = ACTION_PHOTO_DISP_MAIN;
                } else if (!strcmp(app->name, "video_photo")) {

                    it.name = "video_rec";
                    it.action = ACTION_AUTO_VIDEO_REC_DISP;

                }
#else

                if (!strcmp(app->name, "video_rec")) {
                    it.name = "video_photo";
                    // it.action = ACTION_PHOTO_TAKE_MAIN;
                    it.action = ACTION_PHOTO_DISP_MAIN;
                } else if (!strcmp(app->name, "video_photo")) {

                    it.name = "video_dec";
                    it.action = ACTION_VIDEO_DEC_MAIN;

                } else if (!strcmp(app->name, "video_dec"))  {

                    it.name = "video_rec";
                    it.action = ACTION_AUTO_VIDEO_REC_DISP;

                }
#endif
                start_app(&it);
            }


            malloc_stats();
            break;
        case  REPLAY:

            init_intent(&it);
            app = get_current_app();

            if (app) {
                if (!strcmp(app->name, "usb_app")) {
                    break;
                }


                if (!strcmp(app->name, "video_rec") || !strcmp(app->name, "video_photo")) {

                    it.action = ACTION_BACK;
                    start_app(&it);

                    it.name = "video_dec";
                    it.action = ACTION_VIDEO_DEC_MAIN;

                    start_app(&it);
                }
            }

            break ;

        case KEY_REC_START:
            printf("\n  mode to  video>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            if (db_select("mode") == 2) { // 拍完照 切到录像模式 录像
                struct intent it;
                //struct application *app;
                init_intent(&it);
                it.action = ACTION_BACK;
                start_app(&it);
                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN ;
                start_app(&it);

            }
            break;
        default:
            return false;
        }
        break;
    case KEY_EVENT_LONG:


        printf("\n KEY_EVENT_LONG \n  ");
        if (key->value == KEY_POWER) {
            puts("---key_power\n");
            static u8 power_fi = 0;
            if (power_fi) {
                puts("re enter power off\n");
                break;
            }
            power_fi = 1;
            sys_key_event_disable();

            struct sys_event e;
            e.type = SYS_DEVICE_EVENT;
            e.arg = "sys_power";
            e.u.dev.event = DEVICE_EVENT_POWER_SHUTDOWN;
            sys_event_notify(&e);

//IPC用的，长按进入配网模式
#if (APP_CASE == __WIFI_IPCAM__)
            void *wifi_dev = dev_open("wifi", NULL);

            static u8 mac_addr[6];
            static char g_ssid[64];
            dev_ioctl(wifi_dev, DEV_GET_MAC, (u32)&mac_addr);
            sprintf(g_ssid, "wifi_camera_ac57_%02x%02x%02x%02x%02x%02x"
                    , mac_addr[0]
                    , mac_addr[1]
                    , mac_addr[2]
                    , mac_addr[3]
                    , mac_addr[4]
                    , mac_addr[5]);



            struct cfg_info info = {0};
            info.ssid = g_ssid;
            info.pwd = "12345678";
            info.mode = AP_MODE;
            info.force_default_mode = 1;
            dev_ioctl(wifi_dev, DEV_SAVE_DEFAULT_MODE, (u32)&info);

            dev_close(wifi_dev);


            db_reset();
            os_time_dly(100);
            cpu_reset();
#endif

        } else if (key->value == KEY_MENU) {


        }
        break;
    default:
        return false;
    }

    return true;
}



extern u8 get_usb_in_status();


static int main_dev_event_handler(struct sys_event *event)
{
    struct intent it;
    struct application *app;
#ifdef CONFIG_USB_VIDEO_OUT
    static u8 usb_app_flag = 0;
#endif

    init_intent(&it);
    app = get_current_app();
    // printf("\n event->u.dev.event:%d,event->arg:%s\n",event->u.dev.event,event->arg);
    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        break;
#ifdef CONFIG_UI_ENABLE
        if (!ASCII_StrCmp(event->arg, "usb0", 4)) {
#ifdef CONFIG_USB_VIDEO_OUT
            if (usb_app_flag) {
                break;
            }
            usb_app_flag = 1;
#endif
            if (app && strcmp(app->name, "usb_app") && strcmp(app->name, "sdcard_upgrade") && strcmp(app->name, "net_video_rec")) {
                it.action = ACTION_BACK;
                start_app(&it);

                it.name = "usb_app";
                it.action = ACTION_USB_SLAVE_MAIN;
                start_app(&it);
            }
#ifdef CONFIG_UI_STYLE_JL02_ENABLE
            else if (!app) { //主界面进入usb界面
                union uireq req;
                struct server *ui;
                ui = server_open("ui_server", NULL);
                req.hide.id = ID_WINDOW_MAIN_PAGE;
                server_request(ui, UI_REQ_HIDE, &req); /* 隐藏主界面ui */

                it.name = "usb_app";
                it.action = ACTION_USB_SLAVE_MAIN;
                start_app(&it);
            }
#endif
#ifdef CONFIG_PARK_ENABLE
            if (get_parking_status()) {
                sys_key_event_enable();
                sys_touch_event_enable();
            }
#endif
        }
#endif

#ifdef CONFIG_PARK_ENABLE
        if (!ASCII_StrCmp(event->arg, "parking", 7)) {
            if (app) {
                if (!strcmp(app->name, "video_rec")) {
                    break;
                }
                if ((!strcmp(app->name, "video_photo"))
                    || (!strcmp(app->name, "video_dec"))
                    || (!strcmp(app->name, "video_system"))
                    || (!strcmp(app->name, "usb_app"))) {

                    if (!strcmp(app->name, "usb_app")) {
                        if (get_usb_in_status()) {
                            puts("usb in status\n");
                            break;
                        }
                    }

                    it.action = ACTION_BACK;
                    start_app(&it);
                }
                puts("\n =============parking on eee video rec=========\n");
                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                start_app(&it);
            }
#ifdef CONFIG_UI_STYLE_JL02_ENABLE
            else if (!app) { //主界面进入倒车界面
                union uireq req;
                struct server *ui;
                ui = server_open("ui_server", NULL);
                req.hide.id = ID_WINDOW_MAIN_PAGE;
                server_request(ui, UI_REQ_HIDE, &req); /* 隐藏主界面ui */

                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                start_app(&it);
            }
#endif

        }
#endif
        break;
    case DEVICE_EVENT_OUT:
        if (!ASCII_StrCmp(event->arg, "usb0", 4)) {
            if (app && !strcmp(app->name, "usb_app")) {

#ifdef PRODUCT_TEST_ENABLE
                u8 is_product_mode(void);
                if (is_product_mode()) {
                    printf("exit product test mode , sys power off\n");
                    sys_power_poweroff(0);
                }
#endif

                it.action = ACTION_BACK;
                start_app(&it);

                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;

                start_app(&it);
#ifdef CONFIG_PARK_ENABLE
                if (get_parking_status()) {
                    sys_key_event_disable();
                    sys_touch_event_disable();
                }
#endif
            } else {
#ifdef CONFIG_USB_VIDEO_OUT
                int usb_disconnect();
                usb_disconnect();
#endif
            }
        }
        break;
    case DEVICE_EVENT_CHANGE:
#ifdef CONFIG_GSENSOR_ENABLE
        if (!strcmp(event->arg, "gsen_lock")) {
            if (get_gsen_active_flag()) {
                clear_gsen_active_flag();
            }
        }
#endif

        break;
    case DEVICE_EVENT_POWER_SHUTDOWN:

        if (usb_is_charging()) { //插usb 不关机

            break ;
        }
        if (!strcmp(event->arg, "sys_power")) {
            u32 park_en;

            set_low_batery_state(1);
            printf("\n  DEVICE_EVENT_POWER_SHUTDOWN \n");

#if  0
            if (get_workmode_bit() == 1) { //工作完直接关机，不走ui

                printf("\n not show  ui poweroff\n");
                power_off();//  直接调用关机
            }

#endif
            init_intent(&it);
            app = get_current_app();
            if (app) {
#if (APP_CASE == __WIFI_CAR_CAMERA__)
                if (!strcmp(app->name, "net_video_rec")) {
                    //断开所有客户端
                    ctp_srv_disconnect_all_cli();
                    cdp_srv_disconnect_all_cli();
                }
#endif
                it.action = ACTION_BACK;
                start_app(&it);
            }

#ifdef CONFIG_UI_ENABLE
            struct ui_style style;
            style.file = "mnt/spiflash/audlogo/ani.sty";
            style.version = ANI_UI_VERSION;
            struct server *ui = server_open("ui_server", &style);
            if (ui) {
                union uireq req;
                req.show.id = ANI_ID_PAGE_POWER_OFF;
                server_request_async(ui, UI_REQ_SHOW_SYNC | REQ_COMPLETE_CALLBACK, &req,
                                     power_off_play_end, ui);
            }
#else
#ifdef CONFIG_GSENSOR_ENABLE
            park_en = db_select("par");
            set_parking_guard_wkpu(park_en);  //gsensor parking guard */
            sys_power_set_port_wakeup("wkup_gsen", park_en);
#endif
            sys_power_set_port_wakeup("wkup_usb", 1);
            sys_power_poweroff(0);
#endif
        }
        break;
    case DEVICE_EVENT_SPECIAL_POWER_SHUTDOWN:
        if (!strcmp(event->arg, "sys_power")) {
            u32 park_en;


            printf("\n  DEVICE_EVENT_POWER_SHUTDOWN \n");
            if (get_workmode_bit() == 1) { //工作完直接关机，不走ui

                printf("\n not show  ui poweroff\n");
                power_off();//  直接调用关机
            }
            init_intent(&it);
            app = get_current_app();
            if (app) {
#if (APP_CASE == __WIFI_CAR_CAMERA__)
                if (!strcmp(app->name, "net_video_rec")) {
                    //断开所有客户端
                    ctp_srv_disconnect_all_cli();
                    cdp_srv_disconnect_all_cli();
                }
#endif
                it.action = ACTION_BACK;
                start_app(&it);
            }

#ifdef CONFIG_UI_ENABLE
            struct ui_style style;
            style.file = "mnt/spiflash/audlogo/ani.sty";
            style.version = ANI_UI_VERSION;
            struct server *ui = server_open("ui_server", &style);
            if (ui) {
                union uireq req;
                req.show.id = ANI_ID_PAGE_POWER_OFF;
                server_request_async(ui, UI_REQ_SHOW_SYNC | REQ_COMPLETE_CALLBACK, &req,
                                     special_power_off_play_end, ui);
            }
#else
#ifdef CONFIG_GSENSOR_ENABLE
            park_en = db_select("par");
            set_parking_guard_wkpu(park_en);  //gsensor parking guard */
            sys_power_set_port_wakeup("wkup_gsen", park_en);
#endif
            sys_power_set_port_wakeup("wkup_usb", 1);
            sys_power_poweroff(0);
#endif
        }
        break;


    case DEVICE_EVENT_STD_POWER_SHUTDOWN:




        printf("\n  DEVICE_EVENT_STD_POWER_SHUTDOWN  \n");
        if (!strcmp(event->arg, "sys_power")) {
            u32 park_en;


            // set_poweroff();


            init_intent(&it);
            app = get_current_app();
            if (app) {
#if (APP_CASE == __WIFI_CAR_CAMERA__)
                if (!strcmp(app->name, "net_video_rec")) {
                    //断开所有客户端
                    ctp_srv_disconnect_all_cli();
                    cdp_srv_disconnect_all_cli();
                }
#endif
                it.action = ACTION_BACK;
                start_app(&it);
            }

            printf("\n not show  ui poweroff\n");
            power_off();//  直接调用关机
        }
        break;




    }
    return 0;
}

#if (APP_CASE == __WIFI_CAR_CAMERA__)
extern int ctp_cmd_analysis(const char *topic, char *content, void *priv);
static int main_net_event_hander(struct sys_event *event)
{

#ifndef CONFIG_EG_HTTP_SERVER
    struct ctp_arg *event_arg = (struct ctp_arg *)event->arg;
#else
    struct eg_event_arg *event_arg = (struct eg_event_arg *)event->arg;
#endif
    struct net_event *net = &event->u.net;

    switch (net->event) {
    case NET_EVENT_CMD:
        /*定义一个函数指针统一写法*/
#ifndef CONFIG_EG_HTTP_SERVER
        ctp_cmd_analysis(event_arg->topic, event_arg->content, event_arg->cli);
#else
        eg_cmd_analysis(event_arg->cmd, event_arg->content, event_arg->cli);
#endif

        if (event_arg->content) {
            free(event_arg->content);
        }

        event_arg->content = NULL;

        if (event_arg) {
            free(event_arg);
        }

        event_arg = NULL;

        return true;
        break;

    case NET_EVENT_DATA:
        /* printf("IN NET_EVENT_DATA\n"); */
        break;
    }

    return false;
}
#endif

/*
 * 默认的系统事件处理函数
 * 当所有活动的app的事件处理函数都返回false时此函数会被调用
 */
void app_default_event_handler(struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        main_key_event_handler(&event->u.key);
        break;
    case SYS_TOUCH_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        main_dev_event_handler(event);
        break;
#if (APP_CASE == __WIFI_CAR_CAMERA__)
    case SYS_NET_EVENT:
        main_net_event_hander(event);
        break;
#endif

    default:
        ASSERT(0, "unknow event type: %s\n", __func__);
        break;
    }
}

#ifdef RTOS_STACK_CHECK_ENABLE
static void rtos_stack_check_func(void *p)
{
#if 0
    char *pWriteBuffer = malloc(2048);
    if (!pWriteBuffer) {
        return;
    }
    extern void vTaskList(char *pcWriteBuffer);
    vTaskList(pWriteBuffer);
    printf(" \n\ntask_name          task_state priority stack task_num\n%s\n", pWriteBuffer);
    free(pWriteBuffer);
#else
    extern void get_task_state(void *parm);
    get_task_state(NULL); //1分钟以内调用一次才准确
#endif

    /* dump_cpu_irq_usage(); */
    /* dump_os_sw_cnt(); */
    malloc_stats();
#if configCPU_USAGE_CALCULATE
    extern unsigned char ucGetCpuUsage(unsigned char cpuid);
    printf("cpu0 use: %d%%, cpu1 use: %d%%\n", ucGetCpuUsage(0), ucGetCpuUsage(1));
#endif
    /*cpu_effic_calc();*/


#if 0//defined CONFIG_WIFI_ENABLE && !defined CONFIG_MP_TX_TEST_ENABLE && !defined CONFIG_BT_TEST_ENABLE
    extern int wifi_is_on(void);
    if (wifi_is_on()) {
        printf("WIFI U= %d KB/s, D= %d KB/s, RSSI=%d\r\n", wifi_get_upload_rate() / 1024, wifi_get_download_rate() / 1024, wifi_get_rssi());
        void wl_get_statistics(void);
        /* wl_get_statistics(); */
    }
#endif
}
#endif



extern int isp_utest_capture_yuv_0(int id);
extern int isp_start(u8 *src_w, u8 *src_h);
extern void imc_verify(u16 des_w, u16 des_h);
extern int isp_test();
extern void imc_start(struct imc_iattr *attr);
extern void imc_mutipul_test();


extern s32 pr1000_check(u8 isp_dev, u32 _reset_gpio, u32 _power_gpio);
extern s32 pr1000_initialize(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);


extern s32 tp9930k_check(u8 isp_dev, u32 _reset_gpio, u32 _power_gpio);
extern s32 tp9930k_initialize(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);

//pb12 reset, pb14 clk, pb13 dat
static void pr1000_test(void *p)
{
    u16 width;
    u16 height;
    u8 format;
    u8 frame_freq;
    int ret;

    /* log_i("\n\n\npr1000 test task\n\n\n"); */

    /* ret = pr1000_check(1, IO_PORTB_12, -1); */
    /* if (ret == 0) { */
    /* pr1000_initialize(1, &width, &height, &format, &frame_freq); */
    /* } */

    log_i("\n\n\ntp9330 test task\n\n\n");

    ret = tp9930k_check(2, IO_PORTA_06, -1);
    if (ret == 0) {
        tp9930k_initialize(2, &width, &height, &format, &frame_freq);
    }

    while (1) {
        os_time_dly(2);
    }
}


/* #define DISP_TEST */

#ifdef DISP_TEST
extern int lcd_init_wait_done();
/* #include "layer_data.h" */
#include "yuv420_800x480.h"
#define IMAGE_W 800
#define IMAGE_H 480
static void lcd_disp_test(void)
{
    int err;

    lcd_init_wait_done();

    void *fb = dev_open("fb1", (void *)FB_COLOR_FORMAT_YUV420);
    ASSERT(fb, "open fb fail!");
    if (fb) {
        struct fb_var_screeninfo info;
        info.s_xoffset = (LCD_DEV_WIDTH - IMAGE_W) / 2;
        info.s_yoffset = (LCD_DEV_HIGHT - IMAGE_H) / 2;
        info.s_xres    = IMAGE_W;
        info.s_yres    = IMAGE_H;
        info.v_xoffset = 0;
        info.v_yoffset = 0;
        info.v_xres    = IMAGE_W;
        info.v_yres    = IMAGE_H;

        err = dev_ioctl(fb, FBIOSET_VSCREENINFO, (u32)&info);
        if (err) {
            printf("fb1 open fail:%d\n", err);
        }
        err = dev_ioctl(fb, FBIOSET_FBUFFER_NUM, 1);
        if (err) {
            printf("fb malloc buf fail:%d\n", err);
        }
        /* err = dev_ioctl(fb,FBIOSET_PALLET,0); */
        /* if(err) { */
        /* printf("set palette fail:%d\m",err); */
        /* } */
        int alpha = 0;
        err = dev_ioctl(fb, FBIOSET_TRANSP, alpha);
        if (err) {
            printf("set transter fail:%d\n", err);
        }

        struct fb_map_user map;
        dev_ioctl(fb, FBIOGETMAP, (u32)&map);
        memcpy(map.baddr, yuv420_800x480, map.width * map.height * ((map.format == FB_COLOR_FORMAT_YUV422) ? 2 * 2 : 3) / 2);
        flush_dcache(map.baddr, map.width * map.height * 3 / 2);

        /* FILE *fp = fopen("mnt/spiflash/res/480x1280.bin", "r"); */
        /* ASSERT(fp); */
        /* fread(fp, map.baddr, map.width * map.height * 3 / 2); */
        /* flush_dcache(map.baddr, map.width * map.height * 3 / 2); */
        /* fclose(fp); */

        err = dev_ioctl(fb, FBIOSET_ENABLE, true);
        if (err) {
            printf("set enable fail:%d\n", err);
        }

        dev_ioctl(fb, FBIOPUTMAP, (u32)&map);
    }
}
#endif

#define DMA_TSIZE	0//(2*1024*1024)
u8 dma_tbuf1[DMA_TSIZE] ALIGNE(32);
u8 dma_tbuf2[DMA_TSIZE] ALIGNE(32);
extern void dma_copy(void *dst, void *src, int len);
static void dma_test(void *p)
{
    u32 i;
    u8 cnum = 0;

    while (1) {
        for (i = 0; i < DMA_TSIZE; i++) {
            dma_tbuf1[i] = i;
        }
        /* flushinv_dcache(dma_tbuf2, DMA_TSIZE); */
        /* dma_copy((void *)NO_CACHE_ADDR(dma_tbuf2), dma_tbuf1, DMA_TSIZE);	 */
        dma_copy(dma_tbuf2, dma_tbuf1, DMA_TSIZE);
        for (i = 0; i < DMA_TSIZE; i++) {
            if (dma_tbuf2[i] != (i % 256)) {
                log_i("\n\n\n\n\n\n\ndma err1 %x, %x, %x!\n\n\n\n\n\n\n\n\n\n\n\n", dma_tbuf2[i], i % 256, &dma_tbuf2[i]);
                dma_copy(dma_tbuf2, dma_tbuf1, DMA_TSIZE);
                log_i("\n\n\n\n\n\n\ndma err1 %x, %x, %x!\n\n\n\n\n\n\n\n\n\n\n\n", dma_tbuf2[i], i % 256, &dma_tbuf2[i]);
                while (1);
            }
        }

        cnum++;
        memset(dma_tbuf2, cnum, DMA_TSIZE);
        dma_copy(dma_tbuf1, dma_tbuf2, DMA_TSIZE);
        for (i = 0; i < DMA_TSIZE; i++) {
            if (dma_tbuf1[i] != cnum) {
                log_i("\n\n\n\n\n\n\ndma err2 %x, %x!\n\n\n\n\n\n\n\n\n\n\n\n", dma_tbuf1[i], cnum);
                dma_copy(dma_tbuf1, dma_tbuf2, DMA_TSIZE);
                while (1);
            }
        }
        os_time_dly(2);
    }

}


void  test_gpio1()
{

    static u8 init = 0;

    init_gpio();


    printf("\n\n group \n");
    printf("gpio_read(MENU_ON_SET_GPIO)=%d\n", gpio_read(MENU_ON_SET_GPIO));
    printf("gpio_read(BL_OFF_GPIO)=%d\n", gpio_read(BL_OFF_GPIO));
    printf("gpio_read(LCD_DISP_BUSY_GPIO)=%d\n", gpio_read(LCD_DISP_BUSY_GPIO));
    printf("app_mian_gpio_read(WORKING_MODE_GPIO)=%d\n", gpio_read(WORKING_MODE_GPIO));




}
void test_gpio_state()
{

    static u8 number = 0;

    DAA_CON0 &= ~BIT(1);

    number ^= BIT(0);
    printf("\nnumber===============%d\n", number);
    if (number) {
        // gpio_direction_output(, 1);

    } else {
        //  gpio_direction_output(, 0);

    }


}
void sys_malloc_check_func()
{

    malloc_stats();

}

extern u32 mov_rcv_time ;
extern void thread_key_ctr();
/*
 * 应用程序主函数
 */
extern void imb_layer_disp();
/* #define DAC_TEST_ENABLE */
/* #define ADC_TEST_ENABLE */



void  test_power_off()
{

#if 1

    printf("\n zzzzzzzzzxxxxxxxxxxxxxxxxxxxxx \n");
    if (!get_wakeup_flag()) {
        if (get_menu_state() && (switch_work_on_state())) {
            analyize_poweron_state();
        }

    }
    set_wakeup_flag(0);

#endif

}











void disp_test_pirled()
{


    static u8 number = 0;



    number ^= BIT(0);
    printf("\nnumber===============%d\n", number);
    if (number) {
        gpio_direction_output(IO_PORTB_11, 1); // led 指示灯灯    1 关闭  0 打开

    } else {

        gpio_direction_output(IO_PORTB_11, 0); // led 指示灯灯    1 关闭  0 打开
    }



}

extern u32 avi_rcv_time;
extern u32 mov_rcv_time;
void app_main()
{
    struct intent it;
    int err;
    init_gpio();
    // mov_rcv_time=5000;
    printf("app_main\n");

    extern int get_battery_voltage();
    if (0) { //(get_battery_voltage()<928){

        sys_power_set_port_wakeup("wkup_gsen", 1);

        sys_power_set_port_wakeup("wkup_usb", 1);



        printf("\n   off >>>>>>>>>>>>>>>>>>>>>>>>\n");
        sys_power_poweroff(0);
        return ;
    }
//检测开机电压
    // sys_power_low_voltage_shutdown(928, 0);

#ifdef CONFIG_IO_LED_DISP_ENABLE
    if (get_wakeup_flag()) {


        sys_timeout_add(NULL, disp_test_pirled, 25);


    }
#endif
    printf("%s %s-%s\n", __FUNCTION__, __DATE__, __TIME__);
#ifdef CONFIG_IO_MEASURE_TIMER_ENABLE
    set_io_measure_state(1);
#endif
#if 1
    if (!lcd_disp_busy_state()) { // 非屏显状态, 则允许判断工作时间是否合法



        printf("\n switch_work_on_state():%d,!db_select_delay_pv:%d\n ", switch_work_on_state(), !db_select("delay_pv"));

        if (!Deal_work_timer() && switch_work_on_state() && (!db_select("delay_pv"))) { //判断是否在有效设置的时间内
            printf("\n work switch \n ,\
                delay_pv is 0 \n,\
                you should check rtc timer \n");
            power_off();//  直接调用关机
            return ;
        } else if (switch_off_state()) {

            printf("\n   switch is  off  state\n");
            close_pir_power_off();

            return ;
        }
    } else {
        extern void check_rtc_time();
        check_rtc_time();
    }

#endif
#if 1
    printf("\n !get_wakeup_flag:%d,%d\n ", !get_wakeup_flag(), !db_select("delay_pv"));
    if ((!get_wakeup_flag()) && (!db_select("delay_pv"))) {

        if (get_menu_state() && (switch_work_on_state())) {


            printf("\n work position,should be close alarm\n");
            alarm_wakeup_set_config(29, 0);// 先关闭alarm,不然关不了机

            printf("\n rtc_register=\n\
                        A0:0x%x,\n\
                        A1:0x%x,\n\
                        A2:0x%x,\n\
                        A3:0x%x,\n\
                        A4:0x%x,\n\
                        A5:0x%x,\n\
                        A6:0x%x,\n\
                        A7:0x%x,\n\
                        A8:0x%x,\n\
                        A9:0x%x", \
                   read_RTC_reg(0xa0), \
                   read_RTC_reg(0xa1), \
                   read_RTC_reg(0xa2), \
                   read_RTC_reg(0xa3), \
                   read_RTC_reg(0xa4), \
                   read_RTC_reg(0xa5), \
                   read_RTC_reg(0xa6), \
                   read_RTC_reg(0xa7), \
                   read_RTC_reg(0xa8), \
                   read_RTC_reg(0xa9));

            analyize_poweron_state();

        } else {

            if (!lcd_disp_busy_state()) { // 菜单档位上电则开机显示
                set_poweroff() ;
            }
        }

    }


#endif

//底层isp等多少帧 ，才允许拿数据
    void ispt_set_block_frame_count(u32 c);
    ispt_set_block_frame_count(6);
    // ispt_set_block_frame_count(3);
//收敛速度接口 para : c  前面多少帧加速曝光    slope  收敛斜率
    void ispt_set_ae_init_para(u32 c, u32 slope);
    ispt_set_ae_init_para(6, 512);
    void ispt_set_ae_branch(int branch);
    ispt_set_ae_branch(1);
//初始化sensor
    void *fd = dev_open("video0.0", NULL);

    if (fd) {

        isp_effect_start(fd);  // 加载isp效果

        os_time_dly(10);
        printf("\n add isp_effect\n");

    } else {
        printf("\n camera err \n");
    }


    //  sys_timer_add(NULL, test_gpio1, 1 * 1000);
    // sys_timer_add(NULL, test_gpio_state, 1 * 1000);
    // sys_timer_add(NULL, auto_test_ir_cut_state, 250);
#ifdef CONFIG_MEASURE_SENSOR_FPS_ENABLE
    extern void ispt_fps_meter_enable(u32 en)
    ispt_fps_meter_enable(1);

#endif
// 拨码开关按键线程
#if 1
    int pRet;

    //  if(!wakeup_flag){
    pRet = thread_fork("thread_key_ctr", 14, 0x1000, 64, NULL, thread_key_ctr, NULL);
    if (pRet != OS_NO_ERR) {
        printf("\n create %s err ,pRet = %d", "thread_key_ctr", pRet);
    }

    // }
#endif // 0
    /* LDO_CON &=~(0xFF << 4); */
    /* LDO_CON |= BIT(4) | BIT(8) | (1 << 5) | (2 << 9); */
    /* delay(10000); */



    printf("\n get_menu_state()======================%d,lcd_disp_busy_state():%d\n ", get_menu_state(), lcd_disp_busy_state());
//菜单状态开屏显
    if (get_menu_state() && (lcd_disp_busy_state())) {
        avi_rcv_time = 1000; //修复坏损文件
        mov_rcv_time = 1000; //修复坏损文件
#ifdef MULTI_LCD_EN

        printf("\n lcd_init...\n");
        //void *lcd_dev = dev_open("lcd", "ST7789S_mcu");
        void *lcd_dev = dev_open("lcd", "9225_mcu");
        lcd_set_cur_hdl(lcd_dev);
#endif
    }

    if (!fdir_exist("mnt/spiflash")) {
        mount("spiflash", "mnt/spiflash", "sdfile", 0, NULL);
    }

#ifdef CONFIG_UVC_SLAVE_ENABLE
    //uvc 从机模式方案

    printf("\n dev_online_usb0===================%d\n ", dev_online("usb0"));
    while (!dev_online("usb0")) {
        putchar('u');
        os_time_dly(100);
    }
    it.name = "usb_app";
    it.action = ACTION_USB_SLAVE_MAIN;
    start_app(&it);
    it.name = "usb_app";
    it.action = ACTION_USB_SLAVE_SET_CONFIG;
    it.data = "usb:uvc";
    start_app_async(&it, NULL, NULL);
    return;
#endif


    if (lcd_disp_busy_state()) {
        mount_sd_to_fs(SDX_DEV); //屏显示状态允许提前挂卡
        err = upgrade_detect(SDX_DEV); // TF卡升级
        if (!err) {
            return;
        }
    }

#ifdef RTOS_STACK_CHECK_ENABLE
    sys_timer_add(NULL, rtos_stack_check_func, 10 * 1000);
#endif

#if 0 // wifi暂不用
    /*生成文件列表*/
    if (dev_online(SDX_DEV)) {
        char buf[64];
#if defined CONFIG_ENABLE_VLIST
        FILE_LIST_IN_MEM(1);
#endif
        strcpy(buf, "online:1");
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "SD_STATUS", "NOTIFY", buf);

    }
#endif
    sys_power_init();

    /*
     * 播放开机动画
     */
#ifdef CONFIG_UI_ENABLE
    if (get_menu_state() && (lcd_disp_busy_state())) {

        printf("\n ui power on\n");
        struct ui_style style;
        style.file = "mnt/spiflash/audlogo/ani.sty";
        style.version = ANI_UI_VERSION;

        struct server *ui = server_open("ui_server", &style);
        if (ui) {
            union uireq req;

            req.show.id = ANI_ID_PAGE_POWER_ON;
            server_request_async(ui, UI_REQ_SHOW_SYNC | REQ_COMPLETE_CALLBACK, &req,
                                 animation_play_end, ui);
        }
    }
#else
    sys_key_event_enable();
    sys_touch_event_enable();
#endif




    sys_power_auto_shutdown_start(db_select("aff") * 60);	//自动关机时间
    sys_power_low_voltage_shutdown(925 - 150, 3);					//低电关机电压/时间
    //sys_power_charger_off_shutdown(3, 1);					//拔电关机时间

    init_intent(&it);
    it.name = "video_system";
    it.action = ACTION_SYSTEM_MAIN;
    start_app(&it);
    it.action = ACTION_BACK;
    start_app(&it);



    //屏显模式下， 接电脑usb 检测
    if (lcd_disp_busy_state()) {
#ifdef CONFIG_UI_ENABLE
        if (dev_online("usb0")) {
            it.name	= "usb_app";
            it.action = ACTION_USB_SLAVE_MAIN;
#ifdef CONFIG_GSENSOR_ENABLE
            usb_online_clear_park_wkup();
#endif
            start_app(&it);
            return;
        }
#endif
    }


    //  vTaskDeleteRaw(NULL);

//  如下上电进入哪个工作模式
    printf("\n delay_pv:%d,mode===========%d\n ", db_select("delay_pv"), db_select("mode"));


#if 1
//  此action  是工作在延时拍照功能下   第一次创建1s 的封装5fps 的视频文件
    if (db_select("delay_pv") && switch_work_on_state() && db_select("key_state")) { // 条件: 延时拍照+工作档位+启动封装1s视频标志

        it.name	= "video_rec";
        it.action = ACTION_PIR_VIDEO_REC;
        start_app(&it);


        return ; //  如果是延时拍照 需要事先录制一个avi 一帧文件
    }
#endif

    if (db_select("mode") == 0 || db_select("mode") == 2 || db_select("delay_pv")) {

        printf("\n  get_menu_state()================%d,lcd_disp_busy_state():%d\n", get_menu_state(), lcd_disp_busy_state());
        if (get_menu_state() && switch_work_on_state()) {



            it.name	= "video_photo";
            it.action = ACTION_PHOTO_TAKE_MAIN;//WORK档位 拍照模式


        } else {


            db_update("key_state", 1); // 更新延时拍照另创建文件标志
            // db_update("key_state",0); // 更新延时拍照另创建文件标志
            db_flush();

            it.name	= "video_photo";
            it.action = ACTION_PHOTO_DISP_MAIN;//MENU档位 拍照模式显示

            // it.action = ACTION_PHOTO_TAKE_MAIN;
        }

    } else {



        if (!lcd_disp_busy_state()) {

            it.name	= "video_rec";
            it.action = ACTION_VIDEO_REC_MAIN;//WORK 档位 录像模式
        } else {

            it.name	= "video_rec";
            it.action = ACTION_AUTO_VIDEO_REC_DISP;//MENU档位，录像模式显示


        }

    }


    start_app(&it);




}





