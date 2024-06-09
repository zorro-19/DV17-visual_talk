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

#define LOG_TAG "app_main"
#include "generic/log.h"


u32 spin_lock_cnt[2] = {0};

int upgrade_detect(const char *sdcard_name);


/*任务列表 */
#ifdef CONFIG_VIDEO4_ENABLE
const struct task_info task_info_table[] = {
    {"poweroff",            31,     1024,   1024  },
    {"video_fb",            13,     1024,   1024  },

    /* {"ui_server",           29,     1024 * 2,   1024  }, */
    /* {"ui_task",             30,     4096 * 2,   512   }, */

    {"ui_server",           24,     1024 * 4,   1024  },
    {"ui_task",             25,     4096 * 4,   512   },

    {"init",                30,     1024 * 2,   256   },
    {"app_core",            20,     2048 * 4,   4096 * 2},
    {"sys_event",           30,     1024 * 2,   0   },
    {"sys_timer",            20,     2048 * 4,   4096 * 2},
    {"video_server",        27,     2048,   256   },
    {"video_server_transfer",        28,     2048,   128},
    {"audio_server",        16,     1024,   256   },
    {"audio_decoder",       17,     1024,   0     },
    {"video_dec_server",    27,     1024,   1024  },
    {"video_preview_server", 27,     1024,   1024  },
    {"video0_rec0",         22,     2048,   512   },
    {"video0_rec1",         21,     2048,   512   },
    {"audio0_rec0",          26,     2048 * 2,   256 * 2   },
    {"audio0_rec1",          26,     2048 * 2,   256 * 2   },
    {"audio1_rec0",          26,     2048 * 2,   256 * 2   },
    {"audio1_rec1",          26,     2048 * 2,   256 * 2   },
    {"audio2_rec0",          26,     2048 * 2,   256 * 2   },
    {"audio2_rec1",          26,     2048 * 2,   256 * 2   },
    {"audio3_rec0",          26,     2048 * 2,   256 * 2   },
    {"audio3_rec1",          26,     2048 * 2,   256 * 2   },
    {"video1_rec0",         19,     2048,   512   },
    {"video2_rec0",         19,     2048,   512   },
    {"video3_rec0",         19,     2048,   512   },
    /* {"jlc_rec3",            18,     2048,   256   }, */
    /* {"isp_update",          27,     1024,   0     }, */
    {"vpkg_server",         28,     2048,   4096 * 2 },
    {"vunpkg_server",       23,     1024,   128   },
    {"avi0",                29,     2048,   64    },
    {"avi1",                29,     2048,   64    },
    {"avi2",                29,     2048,   64    },
    {"mov0",                29,     2048,   64    },
    {"mov1",                29,     2048,   64    },
    {"mov2",                29,     2048,   64    },
    {"mov3",                29,     2028,   64    },
    {"video_engine_server", 10,     1024,   1024  },
    {"video_engine_task",   10,     2048,   0     },
    {"usb_server",          10,     2048,   128   },
    {"khubd",               15,     1024,   512    },

    /* {"uvc_transceiver",     16,     2048,   32    }, */
    /* {"vimc_scale",          16,     2048,   32    }, */
    {"upgrade_ui",          16,     1024,   32    },
    {"upgrade_core",        20,     1024,   32    },
    {"upgrade_server",      21,     1024,   32    },

    {"dynamic_huffman0",    15,		1024,	32    },
    {"dynamic_huffman1",    15,		1024,	32    },

#ifdef CONFIG_AUTO_TEST
    {"auto_test",			15,		1024,	1024  },
#endif
    /* {"imc_test",			15,		1024,	1024  }, */
    /* {"pr1000_test",			15,		1024,	1024  }, */
    /* {"dma_test",			15,		1024,	1024  }, */

    {"edis0",               26,     4096,   32    },
    {"edis0_as",            19,     4096,   32    },
    {"edis1",               26,     4096,   32    },
    {"edis1_as",            19,     4096,   32    },
    {"edis2",               26,     4096,   32    },
    {"edis2_as",            19,     4096,   32    },
    {"edis3",               26,     4096,   32    },
    {"edis3_as",            19,     4096,   32    },


    {"video4_rec0",         27,     4096,   512   },
    {"video4_rec1",         27,     4096,   512   },
    {"video4_rec2",         27,     4096,   512   },
    {"video4_rec3",         27,     4096,   512   },
    {"audio4_rec0",          26,     2048 * 2,   256 * 2   },
    {"audio4_rec1",          26,     2048 * 2,   256 * 2   },
    {"audio4_rec2",          26,     2048 * 2,   256 * 2   },
    {"audio4_rec3",          26,     2048 * 2,   256 * 2   },

    {"powerdet_task",       15,     1024,   1024  },

    {"user_draw",           15,     1024,   1024  },
    {0, 0},
};
#else
const struct task_info task_info_table[] = {
    {"poweroff",            31,     1024,   1024  },
    {"video_fb",            30,     1024,   1024  },
    {"ui_server",           24,     1024 * 2,   1024  },
    {"ui_task",             25,     4096 * 2,   512   },
    {"init",                30,     1024 * 2,   256   },
    {"app_core",            20,     2048 * 2,   4096},
    {"sys_event",           30,     1024 * 2,  0 },
    {"sys_timer",            20,     2048 * 4,   4096 * 2},
    {"video_server",        27,     2048,   256   },
    {"video_server_transfer",        28,     2048,   128},
    {"audio_server",        16,     1024,   256   },
    {"audio_decoder",       17,     1024,   0     },
    {"video_dec_server",    27,     1024,   1024  },
    {"video_preview_server", 27,     1024,   1024  },
    {"video0_rec0",         22,     2048,   512   },
    {"video0_rec1",         21,     2048,   512   },
    {"video1_rec0",         19,     2048,   512   },

    {"audio0_rec0",          26,     2048 * 2,   256 * 2   },
    {"audio0_rec1",          26,     2048 * 2,   256 * 2   },
    {"audio1_rec0",          26,     2048 * 2,   256 * 2   },
    {"audio1_rec1",          26,     2048 * 2,   256 * 2   },
    {"audio2_rec0",          26,     2048 * 2,   256 * 2   },
    {"audio2_rec1",          26,     2048 * 2,   256 * 2   },
    {"audio3_rec0",          26,     2048 * 2,   256 * 2   },
    {"audio3_rec1",          26,     2048 * 2,   256 * 2   },


    /*{"video1_rec1",         25,     2048,   256   },*/
    {"video2_rec0",         19,     2048,   512   },
    {"video3_rec0",         19,     2048,   512   },
    {"video3_rec1",         18,     2048,   512   },
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
    {"ts0",                14,     2028,   64    },
    {"ts1",                14,     2028,   64    },
    {"aac0_encoder",                16,     2028,   64    },
    {"aac1_encoder",                16,     2028,   64    },

    {"video_engine_server", 14,     1024,   1024  },
    {"video_engine_task",   15,     2048,   0     },
    {"usb_server",          20,     2048,   128   },
    {"khubd",               25,     1024,   512    },

    {"uvc_transceiver",     26,     2048,   32    },
    {"uvc_transceiver1",    26,     2048,   32    },
    {"uvc_transceiver2",    26,     2048,   32    },
    {"uvc_transceiver3",    26,     2048,   32    },
    {"vimc_scale",          26,     2048,   32    },

    {"upgrade_core",        20,     1024,   32    },
    {"upgrade_server",      21,     1024,   32    },

    {"dynamic_huffman0",    15,		1024,	32    },
    {"dynamic_huffman1",    15,		1024,	32    },

    {"auto_test",			15,		1024,	1024  },
    {"imc_test",			15,		1024,	1024  },
    {"pr1000_test",			15,		1024,	1024  },
    {"dma_test",			15,		1024,	1024  },

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

    {"yuv_push_task",       15,     1024,   1024  },
    {"imr_rotate_task",     20,     2048,   32    },

    {"init_video_task",     30,     0x2E00,   32  },
#ifdef CONFIG_AUDIO_TEST
    {"audio_test",       15,     1024,   1024  },
#endif
    {0, 0},
};
#endif



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
    sys_power_poweroff(0);
}
#endif

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
        lcd_dev = dev_open("lcd", "avout");
    } else {
        lcd_dev = dev_open("lcd", "ili8961");
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



static int main_key_event_handler(struct key_event *key)
{
    struct intent it;
    struct application *app;

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
#ifdef MULTI_LCD_EN
        case KEY_F1:
            switch_lcd();/*双屏切换*/
            break;
#endif

        case KEY_MODE:
            init_intent(&it);
            app = get_current_app();

            if (app) {
                if (!strcmp(app->name, "usb_app")) {
                    break;
                }
                it.action = ACTION_BACK;
                start_app(&it);

                if (!strcmp(app->name, "video_rec")) {
                    it.name = "video_photo";
                    it.action = ACTION_PHOTO_TAKE_MAIN;
                } else if (!strcmp(app->name, "video_photo")) {
                    it.name = "video_dec";
                    it.action = ACTION_VIDEO_DEC_MAIN;
                } else if (!strcmp(app->name, "video_dec")) {
                    it.name = "video_rec";
                    it.action = ACTION_VIDEO_REC_MAIN;
                }
                start_app(&it);
            }
            break;
        default:
            return false;
        }
        break;
    case KEY_EVENT_LONG:
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
    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
#ifdef CONFIG_UI_ENABLE
        if (!ASCII_StrCmp(event->arg, "usb0", 4)) {
#ifdef CONFIG_USB_VIDEO_OUT
            if (usb_app_flag) {
                break;
            }
            usb_app_flag = 1;
#endif
            if (app && strcmp(app->name, "usb_app") && strcmp(app->name, "sdcard_upgrade")) {
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
        if (!strcmp(event->arg, "sys_power")) {
            u32 park_en;

            init_intent(&it);
            app = get_current_app();
            if (app) {
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
    }
    return 0;
}


/*
 * 默认的系统事件处理函数
 * 当所有活动的app的事件处理函数都返回false时此函数会被调用
 */
void app_default_event_handler(struct sys_event *event)
{

#ifdef CONFIG_AUDIO_TEST
    return;
#endif

    switch (event->type) {
    case SYS_KEY_EVENT:
        main_key_event_handler(&event->u.key);
        break;
    case SYS_TOUCH_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        main_dev_event_handler(event);
        break;
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
    printf(" \n\ntask_name         task_state priority stack task_num\n%s\n", pWriteBuffer);
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

#if defined CONFIG_WIFI_ENABLE && !defined CONFIG_MP_TX_TEST_ENABLE && !defined CONFIG_BT_TEST_ENABLE
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


/*
 * 应用程序主函数
 */
extern void imb_layer_disp();
/* #define DAC_TEST_ENABLE */
/* #define ADC_TEST_ENABLE */

void app_main()
{
#ifdef CONFIG_AUDIO_TEST
    void audio_test_init();
    audio_test_init();
    return;
#endif

    struct intent it;
    int err;

    /* gpio_direction_output(IO_PORTA_05, 1); */
    /* LDO_CON &=~(0xFF << 4); */
    /* LDO_CON |= BIT(4) | BIT(8) | (1 << 5) | (2 << 9); */
    /* delay(10000); */

    sys_power_low_voltage_shutdown(380, 0);	//检测开机电压

    printf("%s %s-%s\n", __FUNCTION__, __DATE__, __TIME__);
#ifdef MULTI_LCD_EN
    void *lcd_dev = dev_open("lcd", "ek79030_v2");
    /* void *lcd_dev = dev_open("lcd", "lcd_480x272_8bits"); */
    lcd_set_cur_hdl(lcd_dev);
#endif
#ifdef DISP_TEST
    /* lcd_disp_test(); */
    /* imb_layer_disp(); */
#endif

    if (!fdir_exist("mnt/spiflash")) {
        mount("spiflash", "mnt/spiflash", "sdfile", 0, NULL);
    }
#if (defined CONFIG_AEC_DNS_ENABLE || defined CONFIG_ONLY_DNS_ENABLE || defined CONFIG_ONLY_AEC_ENABLE)

#ifdef CONFIG_MPU_ENABLE
    extern void mpu_test_init();
    mpu_test_init();

#endif

    extern void audio_test_init();
    audio_test_init();
#endif


#ifdef CONFIG_UVC_SLAVE_ENABLE
    //uvc 从机模式方案
    while (!dev_online("usb0")) {
        putchar('u');
        os_time_dly(1);
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

#ifndef CONFIG_FAST_CAPTURE
    mount_sd_to_fs(SDX_DEV);
#endif


    /* dv17_dac_test(); */
#ifdef DAC_TEST_ENABLE
    extern void key_voice_start(int id);
    while (1) {
        key_voice_start(0);
        os_time_dly(100) ;
    }
#endif


#ifdef ADC_TEST_ENABLE
    extern void rec_wav_start(char *file_name, u8 audio_type, char *sorce);
    extern void rec_wav_stop();
    rec_wav_start(CONFIG_ROOT_PATH"abc.wav", AUDIO_TYPE_ENC, "mic");
    os_time_dly(2000);
    rec_wav_stop();
    while (1) {
        os_time_dly(100) ;
    }
#endif

#ifdef ISP_TEST_ENABLE
    extern int isp_test();
    isp_test();
    while (1) {
        os_time_dly(1) ;
    }
#endif

    /* task_create(pr1000_test, 0, "pr1000_test"); */
    /* task_create(dma_test, 0, "dma_test"); */
    /* gpio_direction_output(IO_PORTB_15, 0); */
#if 0

    /* imc_verify(640,480); */
    /* isp_utest_capture_yuv_0(6); */
    /* isp_test(); */
    /* imc_mutipul_test(); */

    while (1) {
        os_time_dly(1) ;
    }
#endif

#ifdef RTOS_STACK_CHECK_ENABLE
    sys_timer_add(NULL, rtos_stack_check_func, 10 * 1000);
#endif



#ifndef CONFIG_FAST_CAPTURE
    err = upgrade_detect(SDX_DEV);
    if (!err) {
        return;
    }
#endif

    sys_power_init();




#ifndef CONFIG_FAST_CAPTURE
    /*
     * 播放开机动画
     */
#ifdef CONFIG_UI_ENABLE
    extern int platform_set_text_info(int index, const char *format, u8 fontsize);
    platform_set_text_info(0, "text", 32);
    /* platform_set_text_info(1, "text2", 24); */

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
#else
    sys_key_event_enable();
    sys_touch_event_enable();
#endif

    sys_power_auto_shutdown_start(db_select("aff") * 60);	//自动关机时间
    sys_power_low_voltage_shutdown(370, 3);					//低电关机电压/时间
    sys_power_charger_off_shutdown(10, 1);					//拔电关机时间/能否取消

    init_intent(&it);
    it.name = "video_system";
    it.action = ACTION_SYSTEM_MAIN;
    start_app(&it);
    it.action = ACTION_BACK;
    start_app(&it);

#ifdef CONFIG_MPU_ENABLE
    void mpu_test_init();
    mpu_test_init();
#endif

#if 0
    void user_scr_draw_test(void *p);
    /* user_scr_draw_test(0); */
    task_create(user_scr_draw_test, 0, "user_draw");

    int user_audio_test(void *p);
    /* user_audio_test(0); */
    /* task_create(user_audio_test, 0, "user_audio"); */
    /* while (1) { */
    /* os_time_dly(10); */
    /* } */
#endif



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

#else
    gpio_direction_output(IO_PORTA_05, 0);
#endif

#ifdef CONFIG_FAST_CAPTURE
    it.name	= "video_photo";
    it.action = ACTION_PHOTO_TAKE_MAIN;
#else
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    /* it.name	= "video_dec"; */
    /* it.action = ACTION_VIDEO_DEC_MAIN; */
#endif


    start_app(&it);

    //extern void peripheral_verify_fun();
    //peripheral_verify_fun();
    /* dv17_dac_test(); */
}





