#include "system/includes.h"
#include "server/ui_server.h"
#include "video_system.h"
#include "ui/res/resfile.h"
#include "res.h"

#include "action.h"
#include "style.h"
#include "app_config.h"
#include "storage_device.h"
#include "key_voice.h"
#include "app_database.h"
#include "device/lcd_driver.h"
#include "app_ui_index.h"

extern struct video_system_hdl sys_handler;
extern int sys_cur_mod;  /* 1:rec, 2:tph, 3:dec, 4:audio, 5:music */
#define __this 	(&sys_handler)




int kvo_set_function(u32 parm)
{
    return 0;
}
int hlight_set_function(u32 parm)
{
    return 0;
}

int fre_set_function(u32 parm)
{
    return 0;
}

int aff_set_function(u32 parm)
{
    sys_power_auto_shutdown_stop();
    sys_power_auto_shutdown_start(parm * 60);

    printf("\n aff_set_function_parm===================%d\n ", parm);
    return 0;
}

extern void ui_lcd_light_time_set(int sec);

int pro_set_function(u32 parm)
{
#ifdef CONFIG_DISPLAY_ENABLE
    if (get_menu_state() && (lcd_disp_busy_state())) {
        ui_lcd_light_time_set(parm);
    }
#endif
    return 0;
}

int lag_set_function(u32 parm)
{
    ui_language_set(parm);

    return 0;
}

int tvm_set_function(u32 parm)
{
    return 0;
}

int frm_set_function(u32 parm)
{
    int err;

    sys_key_event_disable();
    sys_touch_event_disable();
    printf("\n  ready format >>>>>>>>>>>>>>>\n");
    err = storage_device_format();

    sys_key_event_enable();
    sys_touch_event_enable();

    return err;
}

u8 get_default_setting_st()
{
    return	(__this->default_set);
}

void clear_default_setting_st()
{
    __this->default_set = 0;
}
extern void def_work_timer_();
static int def_set_function(u32 parm)
{
    puts("def_set_function\n");
    __this->default_set = 1;
    db_reset();
    /* os_time_dly(200); */
    /* cpu_reset(); */
    def_work_timer_();//恢复默认时间

    sys_fun_restore();

    return 0;
}

static int lane_det_set_function(u32 parm)
{


    return 0;
}

#if 1
static int mode_set_function(u32 parm)
{


    return 0;
}
extern int set_camera0_reso(u8 reso);
static  int res_set_function_camera0(u32 parm)
{

#if  1
    printf("\n res_set_function_camera0 >>>>>>>>>>>>>>>:%d\n", parm);
    //   parm=get_table_photo_res();
    switch (parm) {


    case  2:

        parm = 0;

        break ;

    case  5:

        parm = 1;

        break ;

    case  8:
        parm = 2;
        break ;


    case  16:
        parm = 3;
        break ;

    case  24:
        parm = 4;
        break ;

    case  36:
        parm = 5;
        break ;

    default:

        parm = 0;
        break ;

    }
#endif

    set_camera0_reso(parm);



    return 0;
}
static  int cyt_set_function_camera0(u32 parm)
{

    printf("\n cyt_set_function_camera0 >>>>>> ");

    return 0;
}

static  int senity_set_function_camera0(u32 parm)
{

    printf("\n senity_set_function_camera0 >>>>>> ");

    return 0;
}

static  int cyc_set_function_camera0(u32 parm)
{

    printf("\n cyc_set_function_camera0 >>>>>> ");

    return 0;
}

static  int mic_set_function(u32 parm)
{

    printf("\n mic_set_function_camera0 >>>>>> ");

    return 0;
}


static  int lable_set_function(u32 parm)
{

    printf("\n lable_set_function_camera0 >>>>>> ");

    return 0;
}
#endif
static int backlight_set_function(u32 parm)
{
#ifdef CONFIG_PWM_BACKLIGHT_ENABLE
    if (parm < 20 || parm > 100) {
        parm = 100;
    }
    pwm_ch0_backlight_set_duty(parm);
#endif
    return 0;
}

static  int allow_onoff_set_function_camera0(u32 parm)
{

    printf("\n allow_onoff_set_function_camera0 >>>>>>%d\n ", parm);

    return 0;
}
static  int everytimer_onoff_set_function_camera0(u32 parm)
{

    printf("\n everytimer_onoff_set_function_camera0 >>>>>>%d\n ", parm);

    return 0;
}

static  int video_set_timer_camera0(u32 parm)
{

    printf("\n video_set_timer_camera0 >>>>>>%d\n ", parm);

    return 0;
}
static  int video_set_audio_camera0(u32 parm)
{

    printf("\n video_set_audio_camera0 >>>>>>%d\n ", parm);

    return 0;
}

/*
 * 在此处添加所需配置即可
 */
static struct app_cfg cfg_table[] = {
    {"kvo", kvo_set_function},
    {"fre", fre_set_function},
    {"aff", aff_set_function},
    {"pro", pro_set_function},
    {"lag", lag_set_function},
    {"tvm", tvm_set_function},
    {"frm", frm_set_function},
    {"def", def_set_function},
    {"hlw", hlight_set_function},
    {"lan", lane_det_set_function },
    {"blk", backlight_set_function },
    {"mode", mode_set_function },
    {"pres", res_set_function_camera0},
    {"cyt",  cyt_set_function_camera0},
    {"Senity",  senity_set_function_camera0},
    {"res",  cyc_set_function_camera0},
    {"mic", mic_set_function },

    {"pdat", lable_set_function },
    {"tallow", allow_onoff_set_function_camera0 },
    {"sentimer", everytimer_onoff_set_function_camera0 },

    {"vtimer",  video_set_timer_camera0},
    {"aud_vol",  video_set_audio_camera0},

};

//无用
#if 1

int def_rtc_sys_date_set()
{
    struct sys_time t, rtc_time;

//   printf("menu_sys_date_set : %d-%d-%d,%d:%d:%d\n", t.year, t.month, t.day, t.hour, t.min, t.sec);




    rtc_time.year = db_select("rtc_y");
    rtc_time.month = db_select("rtc_m");
    rtc_time.day = db_select("rtc_d");
    rtc_time.hour = db_select("rtc_h");
    rtc_time.min = db_select("rtc_mn");
    rtc_time.sec = db_select("rtc_s");


    printf("menu_sys_date_set : %d-%d-%d,%d:%d:%d\n", rtc_time.year, rtc_time.month, rtc_time.day, rtc_time.hour, rtc_time.min, rtc_time.sec);
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        return -EFAULT;
    }
    dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)&rtc_time);
    dev_close(fd);

    return 0;
}

void def_work_timer_()
{




    struct sys_time s_time, e_time, gap_time, ws_time;
// 定时时间范围
    s_time.hour = db_select("sran_h");
    s_time.min = db_select("sran_m");
    s_time.sec = db_select("sran_s");


    printf("\n defStart_timer---------%d:%d:%d------------\n", s_time.hour, s_time.min, s_time.sec);

    db_update_buffer(ALLOW_START_TIMER_INFO, &s_time, sizeof(struct sys_time));

    e_time.hour = db_select("eran_h");
    e_time.min = db_select("eran_m");
    e_time.sec = db_select("eran_s");

    printf("\n defEtart_timer---------%d:%d:%d------------\n", e_time.hour, e_time.min, e_time.sec);
    db_update_buffer(ALLOW_END_TIMER_INFO, &e_time, sizeof(struct sys_time));

//pir间隔时间


    gap_time.min = db_select("pir_m");
    gap_time.sec = db_select("pir_s");

    printf("\n pir_gip_timer---------%d:%d------------\n", gap_time.min, gap_time.sec);
    db_update_buffer(GAP_TIMER_INFO, &gap_time, sizeof(struct sys_time));


//定时器
    ws_time.hour = db_select("time_h");
    ws_time.min = db_select("time_m");
    ws_time.sec = db_select("time_s");

    printf("\n ws_timer---------%d:%d:%d------------\n", ws_time.hour, ws_time.min, ws_time.sec);

    db_update_buffer(WORK_TIMER_INFO, &ws_time, sizeof(struct sys_time));




    def_rtc_sys_date_set();







}

#endif

void sys_fun_restore()
{
    aff_set_function(db_select("aff"));
    pro_set_function(db_select("pro"));
    lag_set_function(db_select("lag"));
    backlight_set_function(db_select("bkl"));



}


/*
 * 被请求设置参数
 */
int video_sys_set_config(struct intent *it)
{

    ASSERT(it != NULL);
    ASSERT(it->data != NULL);

    return app_set_config(it, cfg_table, ARRAY_SIZE(cfg_table));
}

