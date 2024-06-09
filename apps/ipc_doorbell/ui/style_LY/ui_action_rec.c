#include "ui/includes.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "app_database.h"
#include "server/audio_server.h"
#include "server/video_dec_server.h"
#include "asm/lcd_config.h"

#define _STDIO_H_
#include "time.h"
#include "xciot_api.h"
#include "xciot_cmd_ipc.h"
#include "doorbell_event.h"
#include "video_rec.h"

enum machine_call_work_state    video_call_state;
enum machine_call_work_state    video_last_call_state;
enum machine_msg_work_state     video_msg_state;
enum machine_rec_state          video_rec_in_state;

int get_video_call_state()
{
    return video_call_state;
}

int get_video_msg_state()
{
    return video_msg_state;
}

int get_video_rec_in_state()
{
    return video_rec_in_state;
}

#ifdef CONFIG_UI_STYLE_LY_ENABLE


extern int sys_cur_mod;  /* 1:rec, 2:tph, 3:dec, 4:audio, 5:music */

extern int menu_sys_lcd_pro_set(u8 sel_item);
extern int menu_sys_auto_off_set(u8 sel_item);
extern int menu_sys_led_fre_set(u8 sel_item);
extern int menu_sys_key_voice_set(u8 sel_item);
extern int menu_sys_language_set(u8 sel_item);
extern int menu_sys_date_set(u8 sel_item);
extern int menu_sys_tv_mod_set(u8 sel_item);
extern int menu_sys_format_set(u8 sel_item);
extern int menu_sys_default_set(u8 sel_item);

static u8 __car_num_set_by_ascii(const char *str);

#define STYLE_NAME  LY

extern u8 sel_contact_flag;


struct video_menu_info {
    volatile char if_in_rec;    /* 是否正在录像 */
    u8 lock_file_flag;          /* 是否当前文件被锁 */
    u8 sd_write_err;			/* 写卡错误 */

    u8 menu_status;             /*0 menu off, 1 menu on*/
    u8 battery_val;
    u8 battery_char;
    u8 hlight_show_status;  /* 前照灯显示状态 */

    int vanish_line;
    int car_head_line;

    int remain_time;
};

static struct video_menu_info handler = {0};
#define __this 	(&handler)
#define sizeof_this     (sizeof(struct video_menu_info))

static struct server *audio = NULL;
static FILE *file;
static int rec_time = 0;
/************************************************************
				    	录像模式设置
************************************************************/
/*
 * rec分辨率设置
 */
static const u8 table_video_resolution[] = {
    VIDEO_RES_1080P,
    VIDEO_RES_720P,
    VIDEO_RES_VGA,
};


/*
 * rec循环录像设置
 */
static const u8 table_video_cycle[] = {
    0,
    3,
    5,
    10,
};


/*
 * rec曝光补偿设置
 */
static const u8 table_video_exposure[] = {
    3,
    2,
    1,
    0,
    (u8) - 1,
    (u8) - 2,
    (u8) - 3,
};


/*
 * rec重力感应设置
 */
static const u8 table_video_gravity[] = {
    GRA_SEN_OFF,
    GRA_SEN_LO,
    GRA_SEN_MD,
    GRA_SEN_HI,
};



/*
 * rec间隔录影设置, ms
 */
static const u16 table_video_gap[] = {
    0,
    100,
    200,
    500,
};

static const u16 province_gb2312[] = {
    0xA9BE, 0xFEC4, 0xA8B4, 0xA6BB, 0xF2BD, //京，宁，川，沪，津
    0xE3D5, 0xE5D3, 0xE6CF, 0xC1D4, 0xA5D4, //浙，渝，湘，粤，豫
    0xF3B9, 0xD3B8, 0xC9C1, 0xB3C2, 0xDABA, //贵，赣，辽，鲁，黑
    0xC2D0, 0xD5CB, 0xD8B2, 0xF6C3, 0xFABD, //新，苏，藏，闽，晋
    0xEDC7, 0xBDBC, 0xAABC, 0xF0B9, 0xCAB8, //琼，冀，吉，桂，甘，
    0xEECD, 0xC9C3, 0xF5B6, 0xC2C9, 0xE0C7, //皖，蒙，鄂，陕，青，
    0xC6D4                                  //云
};

static const u8 num_table[] = {
    'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y',
    'Z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8',
    '9'
};


struct car_num {
    const char *mark;
    u32 text_id;
    u32 text_index;
};

struct car_num_str {
    u8 province;
    u8 town;
    u8 a;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
};

struct car_num text_car_num_table[] = {
    {"province", TEXT_MN_CN_PROVINCE, 0}, /* 京 */
    {"town",     TEXT_MN_CN_TOWN,     0}, /* A */
    {"a",        TEXT_MN_CN_A,        0}, /* 1 */
    {"b",        TEXT_MN_CN_B,        0}, /* 2 */
    {"c",        TEXT_MN_CN_C,        0}, /* 3 */
    {"d",        TEXT_MN_CN_D,        0}, /* 4 */
    {"e",        TEXT_MN_CN_E,        0}, /* 5 */
};

enum sw_dir {
    /*
     * 切换方向
     */
    DIR_NEXT = 1,
    DIR_PREV,
    DIR_SET,
};
enum set_mod {
    /*
     * 加减方向
     */
    MOD_ADD = 1,
    MOD_DEC,
    MOD_SET,
};
static u16 car_num_set_p = 0xff;
/*
 * text_car_num_table的当前设置指针
 */

/*
 * (begin)提示框显示接口
 */
enum box_msg {
    BOX_MSG_NO_POWER = 1,
    BOX_MSG_MEM_ERR,
    BOX_MSG_NO_MEM,
    BOX_MSG_NEED_FORMAT,
    BOX_MSG_INSERT_SD,
    BOX_MSG_DEFAULT_SET,
    BOX_MSG_FORMATTING,
    BOX_MSG_10S_SHUTDOWN,
    BOX_MSG_SD_ERR,
    BOX_MSG_SD_WRITE_ERR,
};
static u8 msg_show_f = 0;
static enum box_msg msg_show_id = 0;
static void __rec_msg_hide(enum box_msg id)
{
    printf("\n /****** [%s] %d id = %d\n",__func__,__LINE__,id);
    if (msg_show_id == id) {
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(LAYER_MSG_REC);
        }
    } else if (id == 0) {
        /*
         * 没有指定ID，强制隐藏
         */
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(LAYER_MSG_REC);
        }
    }
}
static void __rec_msg_timeout_func(void *priv)
{
    __rec_msg_hide((enum box_msg)priv);
}
static void __rec_msg_show(enum box_msg msg, u32 timeout_msec)
{
    printf("\n /****** [%s] %d msg = %d, msg_show_id\n",__func__,__LINE__,msg,msg_show_id);
    static int t_id = 0;
    if (msg == 0) {
        ASSERT(0, "__rec_msg_show msg 0!\n");
        return;
    }

    if (msg == msg_show_id) {
        printf("\n /****** %s %d\n",__func__,__LINE__);
        if (msg_show_f == 0) {
            printf("\n /****** %s %d\n",__func__,__LINE__);
            msg_show_f = 1;
            ui_show(LAYER_MSG_REC);
            ui_text_show_index_by_id(TEXT_MSG_REC, msg - 1);
            if (t_id) {
                sys_timeout_del(t_id);
                t_id = 0;
            }
            if (timeout_msec > 0) {
                t_id = sys_timeout_add((void *)msg, __rec_msg_timeout_func, timeout_msec);
            }
        }
    } else {
        printf("\n /****** %s %d\n",__func__,__LINE__);
        msg_show_id = msg;
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(LAYER_MSG_REC);
        }
        ui_text_show_index_by_id(TEXT_MSG_REC, msg - 1);
        if (t_id) {
            sys_timeout_del(t_id);
            t_id = 0;
        }
        if (timeout_msec > 0) {
            t_id = sys_timeout_add((void *)msg, __rec_msg_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */

/*****************************布局上部回调 ************************************/
static int video_layout_up_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;

    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 在此获取默认隐藏的图标的状态并显示
         */


        if (__this->lock_file_flag == 1) {
            ui_show(PIC_LOCK_REC);
        }

        if (db_select("mot")) {
            ui_show(PIC_MOTDET_REC);
        } else {
            ui_hide(PIC_MOTDET_REC);
        }

        index = db_select("gra");
        if (index != 0) {
            ui_pic_show_image_by_id(PIC_GRAVITY_REC, index - 1);
        } else {
            ui_hide(PIC_GRAVITY_REC);
        }

        if (db_select("par")) {
            ui_show(PIC_PARK_REC);
        } else {
            ui_hide(PIC_PARK_REC);
        }

        if (db_select("wdr")) {
            ui_show(PIC_HDR_REC);
        } else {
            ui_hide(PIC_HDR_REC);
        }

        index = index_of_table8(db_select("cyc"), TABLE(table_video_cycle));

        if (index != 0) {
            ui_pic_show_image_by_id(PIC_CYC_REC, index - 1);
        } else {
            ui_hide(PIC_CYC_REC);
        }

        index = index_of_table16(db_select("gap"), TABLE(table_video_gap));
        if (index) {
            ui_show(PIC_GAP_REC);
        } else {
            ui_hide(PIC_GAP_REC);
        }

        if (__this->if_in_rec) {
            puts("show run rec timer.\n");
            ui_show(TIMER_RUN_REC);
        } else {
            ui_show(TIMER_REMAI_REC);
        }
        index = index_of_table8(db_select("exp"), TABLE(table_video_exposure));
        ui_pic_show_image_by_id(PIC_EXP_REC, index);

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_UP_REC)
.onchange = video_layout_up_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
/*****************************布局下部回调 ************************************/
static int video_layout_down_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u32 a, b;

    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 在此获取默认隐藏的图标的状态并显示
         */
        a = db_select("cna");
        b = db_select("cnb");
        text_car_num_table[0].text_index = index_of_table16(a >> 16, TABLE(province_gb2312));
        text_car_num_table[1].text_index  = index_of_table8((a >> 8) & 0xff, TABLE(num_table));
        text_car_num_table[2].text_index  = index_of_table8((a >> 0) & 0xff, TABLE(num_table));
        text_car_num_table[3].text_index  = index_of_table8((b >> 24) & 0xff, TABLE(num_table));
        text_car_num_table[4].text_index  = index_of_table8((b >> 16) & 0xff, TABLE(num_table));
        text_car_num_table[5].text_index  = index_of_table8((b >> 8) & 0xff, TABLE(num_table));
        text_car_num_table[6].text_index  = index_of_table8((b >> 0) & 0xff, TABLE(num_table));

        if (db_select("num")) {
            ui_text_show_index_by_id(TEXT_CAR_PROVINCE, text_car_num_table[0].text_index);
            ui_text_show_index_by_id(TEXT_CAR_TOWN, text_car_num_table[1].text_index);
            ui_text_show_index_by_id(TEXT_CAR_A,    text_car_num_table[2].text_index);
            ui_text_show_index_by_id(TEXT_CAR_B,    text_car_num_table[3].text_index);
            ui_text_show_index_by_id(TEXT_CAR_C,    text_car_num_table[4].text_index);
            ui_text_show_index_by_id(TEXT_CAR_D,    text_car_num_table[5].text_index);
            ui_text_show_index_by_id(TEXT_CAR_E,    text_car_num_table[6].text_index);
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_DOWN_REC)
.onchange = video_layout_down_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static void get_sys_time(struct sys_time *time)
{
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        memset(time, 0, sizeof(*time));
        return;
    }
    dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)time);
    dev_close(fd);
}

/***************************** 系统时间控件动作 ************************************/
static int timer_sys_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    static struct sys_time sys_time = {0};

    switch (e) {
    case ON_CHANGE_INIT:
        get_sys_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    case ON_CHANGE_SHOW_PROBE:
        get_sys_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_HOUR_REC)
.onchange = timer_sys_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
/* [>**************************** 系统日期控件动作 ***********************************<] */
/* static int timer_sys_date_rec_onchange(void *ctr, enum element_change_event e, void *arg) */
/* { */
/* struct ui_time *time = (struct ui_time *)ctr; */
/* struct sys_time sys_time; */

/* switch (e) { */
/* case ON_CHANGE_INIT: */
/* get_sys_time(&sys_time); */
/* time->year = sys_time.year; */
/* time->month = sys_time.month; */
/* time->day = sys_time.day; */
/* time->hour = sys_time.hour; */
/* time->min = sys_time.min; */
/* time->sec = sys_time.sec; */
/* break; */
/* case ON_CHANGE_SHOW_PROBE: */
/* get_sys_time(&sys_time); */
/* time->year = sys_time.year; */
/* time->month = sys_time.month; */
/* time->day = sys_time.day; */
/* time->hour = sys_time.hour; */
/* time->min = sys_time.min; */
/* time->sec = sys_time.sec; */
/* break; */
/* default: */
/* return false; */
/* } */
/* return false; */
/* } */
/* REGISTER_UI_EVENT_HANDLER(TIMER_YEAR_REC) */
/* .onchange = timer_sys_date_rec_onchange, */
/* .onkey = NULL, */
/* .ontouch = NULL, */
/* }; */

/*
 * 录像计时的定时器,1s
 */
static void rec_cnt_handler(int rec_cnt_sec)
{
    struct sys_time sys_time;
    struct utime time_r;

    get_sys_time(&sys_time);

    rec_time = sys_time.hour * 3600 + sys_time.min * 60 + sys_time.sec - rec_cnt_sec;

    time_r.hour = rec_cnt_sec / 60 / 60;
    time_r.min = rec_cnt_sec / 60 % 60;
    time_r.sec = rec_cnt_sec % 60;
    ui_time_update_by_id(TIMER_RUN_REC, &time_r);
}

/*
 * sd卡事件处理函数
 */
static void sd_event_handler(struct sys_event *event, void *priv)
{
    if (!strcmp(event->arg, "sd0") || !strcmp(event->arg, "sd1") || !strcmp(event->arg, "sd2")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            ui_pic_show_image_by_id(PIC_SD_REC, 1);
            ui_show(TIMER_REMAI_REC);
            break;
        case DEVICE_EVENT_OUT:
        case DEVICE_EVENT_OFFLINE:
            if (__this->sd_write_err) {
                __rec_msg_hide(BOX_MSG_SD_WRITE_ERR);
                __this->sd_write_err = 0;
            }
            __this->remain_time = 0;
            ui_pic_show_image_by_id(PIC_SD_REC, 0);
            ui_show(TIMER_REMAI_REC);
            break;
        default:
            break;
        }
    } else if (!strcmp(event->arg, "video_rec_time")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_CHANGE:
            rec_cnt_handler(event->u.dev.value);
            break;
        default:
            break;
        }
    } else if (!strncmp((char *)event->arg, "rec", 3)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_OFFLINE:
            log_e("video rec write error");
            if (!__this->sd_write_err) {
                __rec_msg_show(BOX_MSG_SD_WRITE_ERR, 0);
                __this->sd_write_err = 1;
            }
            break;
        }
    }

}

extern int storage_device_ready();
/***************************** SD 卡图标动作 ************************************/
static int pic_sd_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u16 id = 0;

    switch (e) {
    case ON_CHANGE_INIT:
        if (storage_device_ready() == 0) {
            ui_pic_set_image_index(pic, 0);
        } else {
            ui_pic_set_image_index(pic, 1);
        }
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 250, 0, sd_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PIC_SD_REC)
.onchange = pic_sd_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static void no_power_msg_box_timer(void *priv)
{
    static u8 cnt = 0;
    if (__this->battery_val <= 20 && __this->menu_status == 0 && __this->battery_char == 0) {
        cnt++;
        if ((cnt % 2) == 0) {
            __rec_msg_show(BOX_MSG_NO_POWER, 0);
        } else {
            __rec_msg_hide(BOX_MSG_NO_POWER);
        }
    } else {
        __rec_msg_hide(BOX_MSG_NO_POWER);
        cnt = 0;
    }
}


/*
 * battery事件处理函数
 */
static void battery_event_handler(struct sys_event *event, void *priv)
{

    static u8 ten_sec_off = 0;
    if (ten_sec_off) {
        if (event->type == SYS_KEY_EVENT || event->type == SYS_TOUCH_EVENT) {
            ten_sec_off = 0;
            __rec_msg_hide(0);
            return;
        }
    }

    if (__this->sd_write_err) {
        if (event->type == SYS_KEY_EVENT || event->type == SYS_TOUCH_EVENT) {
            __rec_msg_hide(BOX_MSG_SD_WRITE_ERR);
            __this->sd_write_err = 0;
        }
    }

    if (event->type == SYS_DEVICE_EVENT) {
        if (!ASCII_StrCmp(event->arg, "sys_power", 9)) {
            if (event->u.dev.event == DEVICE_EVENT_POWER_PERCENT) {
                __this->battery_val = event->u.dev.value;
                if (__this->battery_val > 100) {
                    __this->battery_val = 100;
                }
                if (__this->battery_char == 0) {
                    ui_battery_level_change(__this->battery_val, 0);
                }
            } else if (event->u.dev.event == DEVICE_EVENT_POWER_CHARGER_IN) {
                ten_sec_off = 0;
                __rec_msg_hide(BOX_MSG_10S_SHUTDOWN);
                ui_battery_level_change(100, 1);
                __this->battery_char = 1;
            } else if (event->u.dev.event == DEVICE_EVENT_POWER_CHARGER_OUT) {
                ten_sec_off = 1;
                __rec_msg_show(BOX_MSG_10S_SHUTDOWN, 0);
                ui_battery_level_change(__this->battery_val, 0);
                __this->battery_char = 0;
            }
        }
    }
}
/***************************** 电池控件动作 ************************************/
static int battery_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_battery *battery = (struct ui_battery *)ctr;
    static u16 id = 0;
    static u32 timer_handle = 0;
    static void *fd = NULL;
    switch (e) {
    case ON_CHANGE_INIT:
        /* id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, fd, battery_event_handler); */
        id = register_sys_event_handler(SYS_DEVICE_EVENT | SYS_KEY_EVENT | SYS_TOUCH_EVENT, 200, 0, battery_event_handler);
        break;
    case ON_CHANGE_FIRST_SHOW:
        /* fd = dev_open("sys_power", NULL); */
        /* if (fd) { */
        /* u32 power_level = 0; */
        /* dev_ioctl(fd, SYS_POWER_IOC_GET_PERCENT, (u32)&power_level); */
        /* __this->battery_val = power_level; */
        /* if (__this->battery_val > 100) { */
        /* __this->battery_val = 100; */
        /* } */
        /* dev_close(fd); */
        /* } */
        __this->battery_val = sys_power_get_battery_persent();
        if (__this->battery_val > 100) {
            __this->battery_val = 100;
        }
        __this->battery_char = (usb_is_charging() ? 1 : 0);
        ui_battery_level_change(__this->battery_val, __this->battery_char);
        timer_handle = sys_timer_add(NULL, no_power_msg_box_timer, 1000);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        if (timer_handle) {
            sys_timer_del(timer_handle);
            timer_handle = 0;
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(BATTERY_REC)
.onchange = battery_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/***************************** 录像剩余时间控件动作 ************************************/
static int timer_rec_remain_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct intent it;
    int err, i, j;
    u16 timebuf[3] = {0};

    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW_PROBE:
        if (storage_device_ready() == 0) {
            /*
             * 第一次显示需要判断一下SD卡是否在线
             */
            time->hour = 0;
            time->min = 0;
            time->sec = 0;
            break;
        }

        time->hour = __this->remain_time / 3600;
        time->min = __this->remain_time % 3600 / 60;
        time->sec = __this->remain_time % 60;
        printf("reTIME hour:%02d, min:%02d, sec:%02d\n", time->hour, time->min, time->sec);

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_REMAI_REC)
.onchange = timer_rec_remain_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/***************************** 录像时间控件动作 ************************************/
static int timer_rec_red_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        if (rec_time) {
            struct sys_time sys_time;
            get_sys_time(&sys_time);
            int rtime = sys_time.hour * 3600 + sys_time.min * 60 + sys_time.sec - rec_time;
            time->hour = rtime / 60 / 60;
            time->min = rtime / 60 % 60;
            time->sec = rtime % 60;
        } else {
            time->hour = 0;
            time->min = 0;
            time->sec = 0;
        }
        break;
    case ON_CHANGE_HIDE:
        time->hour = 0;
        time->min = 0;
        time->sec = 0;
        break;
    case ON_CHANGE_SHOW_PROBE:
        if ((time->sec % 2) == 0) {
            ui_show(PIC_RED_DOT_REC);
        } else {
            ui_hide(PIC_RED_DOT_REC);
        }
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_RUN_REC)
.onchange = timer_rec_red_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




/***************************** MIC 图标动作 ************************************/
static int pic_mic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, db_select("mic"));
        /*
         * 禁止录音
         */
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(PIC_MIC_REC)
.onchange = pic_mic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/***************************** 分辨率文字动作 ************************************/
static int text_res_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
#ifndef CONFIG_VIDEO4_ENABLE
        index = index_of_table8(db_select("res"), TABLE(table_video_resolution));
#else
        index = VIDEO_RES_720P;
#endif
        ui_text_set_index(text, index);
        return TRUE;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TEXT_RES_REC)
.onchange = text_res_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




/*
 * (begin)APP状态变更，UI响应回调
 */
static int rec_on_handler(const char *type, u32 arg)
{
    struct sys_time sys_time;
    struct utime time_r;
    time_r.hour = 0;
    time_r.min = 0;
    time_r.sec = 0;

    get_sys_time(&sys_time);
    rec_time = sys_time.hour * 3600 + sys_time.min * 60 + sys_time.sec;

    ui_time_update_by_id(TIMER_RUN_REC, &time_r);
    __this->if_in_rec = TRUE;
    ui_hide(TIMER_REMAI_REC);
    ui_show(TIMER_RUN_REC);
    return 0;
}

static int rec_off_handler(const char *type, u32 arg)
{
    __this->if_in_rec = FALSE;
    rec_time = 0;
    if (__this->lock_file_flag) {
        __this->lock_file_flag = 0;
        ui_hide(PIC_LOCK_REC);
    }
    ui_hide(TIMER_RUN_REC);
    ui_hide(PIC_RED_DOT_REC);
    ui_show(TIMER_REMAI_REC);
    return 0;
}

static int rec_save_handler(const char *type, u32 arg)
{
    /* struct utime time_r; */
    /* time_r.hour = 0; */
    /* time_r.min = 0; */
    /* time_r.sec = 0; */
    /* ui_time_update_by_id(TIMER_RUN_REC, &time_r); */

    if (__this->lock_file_flag) {
        __this->lock_file_flag = 0;
        ui_hide(PIC_LOCK_REC);
    }

    return 0;
}
static int rec_no_card_handler(const char *type, u32 arg)
{
    __rec_msg_show(BOX_MSG_INSERT_SD, 3000);
    return 0;
}
static int rec_fs_err_handler(const char *type, u32 arg)
{
    ui_hide(TIMER_RUN_REC);
    ui_show(TIMER_REMAI_REC);
    if (fget_err_code(CONFIG_ROOT_PATH) == -EIO) {
        __rec_msg_show(BOX_MSG_SD_ERR, 3000);
    } else {
        __rec_msg_show(BOX_MSG_NEED_FORMAT, 3000);
    }
    return 0;
}

static int rec_on_mic_handler(const char *type, u32 arg)
{
    ui_pic_show_image_by_id(PIC_MIC_REC, 1);

    return 0;
}
static int rec_off_mic_handler(const char *type, u32 arg)
{
    ui_pic_show_image_by_id(PIC_MIC_REC, 0);
    return 0;
}
static int rec_lock_handler(const char *type, u32 arg)
{
    __this->lock_file_flag = 1;
    ui_show(PIC_LOCK_REC);
    return 0;
}
static int rec_unlock_handler(const char *type, u32 arg)
{
    __this->lock_file_flag = 0;
    ui_hide(PIC_LOCK_REC);
    return 0;
}

extern void play_voice_file(const char *file_name);
static int rec_headlight_on_handler(const char *type, u32 arg)
{
    if (__this->menu_status == 0) {
        puts("rec_headlight_on_handler\n");
        if (__this->hlight_show_status == 0) {
            __this->hlight_show_status = 1;
            ui_show(ANI_FLIG_REC);//show head light
            play_voice_file("mnt/spiflash/audlogo/olight.adp");
        }
    } else {
        __this->hlight_show_status = 0;
    }

    return 0;
}
static int rec_headlight_off_handler(const char *type, u32 arg)
{
    if (__this->menu_status == 0) {
        puts("rec_headlight_off_handler\n");
        ui_hide(ANI_FLIG_REC);//hide head light
        __this->hlight_show_status = 0;
    }
    return 0;
}

static int rec_remain_handler(const char *type, u32 arg)
{
    printf("remain= %s %d\n", type, arg);
    if (type[0] == 's') {
        __this->remain_time = arg;
        if (__this->if_in_rec == 0) {
            ui_hide(TIMER_REMAI_REC);
            ui_show(TIMER_REMAI_REC);
        }
    }
    return 0;
}
/*
 * 录像模式的APP状态响应回调
 */
static const struct uimsg_handl rec_msg_handler[] = {
//    { "onREC",          rec_on_handler       }, /* 开始录像 */
//    { "offREC",         rec_off_handler      }, /* 停止录像 */
//    { "saveREC",        rec_save_handler     }, /* 保存录像 */
//    { "noCard",         rec_no_card_handler  }, /* 没有SD卡 */
//    { "fsErr",          rec_fs_err_handler   }, /* 需要格式化 */
//    { "onMIC",          rec_on_mic_handler   }, /* 打开录音 */
//    { "offMIC",         rec_off_mic_handler  }, /* 关闭录音 */
//    { "lockREC",        rec_lock_handler     }, /* 锁文件 */
//    { "unlockREC",      rec_unlock_handler   }, /* 解锁文件 */
//    { "HlightOn",      rec_headlight_on_handler   },
//    { "HlightOff",     rec_headlight_off_handler  },
//    { "Remain",         rec_remain_handler  },
    { NULL, NULL},      /* 必须以此结尾！ */
};
/*
 * (end)
 */



/*
 * (begin)UI状态变更主动请求APP函数
 */
static void rec_tell_app_exit_menu(void)
{

    int err;
    struct intent it;
    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_CHANGE_STATUS;
    it.data = "exitMENU";
    err = start_app(&it);
    if (err) {
        printf("res exit menu err! %d\n", err);
        /* ASSERT(err == 0, ":rec exitMENU\n"); */
    }
}

static int rec_ask_app_open_menu(void)
{

    int err;
    struct intent it;

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_CHANGE_STATUS;
    it.data = "opMENU:";
    err = start_app(&it);
    if (err) {
        printf("res ask menu err! %d\n", err);
        return -1;
        /* ASSERT(err == 0, ":rec opMENU fail! %d\n", err); */
    }
    if (!strcmp(it.data, "opMENU:dis")) {
        return -1;
    } else if (!strcmp(it.data, "opMENU:en")) {
    } else {
        ASSERT(0, "opMENU err\n");
    }
    return 0;
    /*
     * 返回0则打开菜单
     */
}



static void menu_lane_det_set(int value)
{
    struct intent it;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "lan";
    it.exdata = value;
    start_app(&it);
}
static void open_set_lane_page(void *p, int err)
{
    struct intent it;
    puts("\n =============car lane set =========\n");
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    it.data   = "lan_setting";
    start_app(&it);
}
static void close_set_lane_page(void *p)
{
    struct intent it;
    ui_hide(ui_get_current_window_id());
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_BACK;
    start_app(&it);
    puts("\n =============car lane set exit=========\n");
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    start_app(&it);
}
static char lane_set_arg[128];
static void send_lane_det_setting_msg(const char *arg, u32 lane)
{
    struct sys_event eve;
    memset(lane_set_arg, 0, sizeof(lane_set_arg));
    sprintf(lane_set_arg, "%s:%d", arg, lane);
    puts(lane_set_arg);
    eve.arg = lane_set_arg;
    eve.type = SYS_DEVICE_EVENT;
    eve.u.dev.event = DEVICE_EVENT_CHANGE;
    sys_event_notify(&eve);
}
/*
 * (end)
 */


void rec_exit_menu_post(void)
{
    /*
     * 退出菜单，回到APP
     */
    ui_hide(LAYER_MENU_REC);
    ui_show(LAYER_UP_REC);
    rec_tell_app_exit_menu();

    __this->menu_status = 0;

}

static void video_rec_ui_event_handler(struct sys_event *event, void *priv)
{
    if (!strcmp(event->arg, "parking")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            break;
        case DEVICE_EVENT_OUT:
        case DEVICE_EVENT_OFFLINE:
            if (__this->menu_status) {
                sys_key_event_takeover(false, false);
            }
            break;
        default:
            break;
        }
    }
}

/*****************************录像模式页面回调 ************************************/
static int video_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    int err, item;
    static int id = 0;
    const char *str = NULL;
    struct intent it;
    int ret;
    u8 timeout;

    switch (e) {
    case ON_CHANGE_INIT:
        puts("\n***rec mode onchange init***\n");
        ui_register_msg_handler(ID_WINDOW_VIDEO_REC, rec_msg_handler);
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, video_rec_ui_event_handler);
        /*
         * 注册APP消息响应
         */
        sys_cur_mod = 1;
        __this->hlight_show_status = 0;
        break;
    case ON_CHANGE_RELEASE:
        ui_hide(ID_WINDOW_VIDEO_SYS);
        /*
         * 要隐藏一下系统菜单页面，防止在系统菜单插入USB进入USB页面
         */
        unregister_sys_event_handler(id);
        break;
    default:
        return false;
    }
    return false;
}

static int video_mode_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct window *win = (struct window *)ctr;
    static char flag = 0;
    int err;
    struct intent it;

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            if (__this->if_in_rec == TRUE) {
                break;
            }
            if (rec_ask_app_open_menu() == (int) - 1) {
                break;
            }
            __rec_msg_hide(0);
            ui_hide(LAYER_UP_REC);
            ui_show(LAYER_MENU_REC);
            ui_show(LAYOUT_MN_REC_REC);
            /*
             * 显示菜单1
             */
            sys_key_event_takeover(true, false);
            printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);
            __this->menu_status = 1;
            break;
        default:
            return false;
        }
        break;
    default:
        if (__this->menu_status) {
            if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
                rec_exit_menu_post();
                sys_key_event_takeover(false, true);
            }
        }
        return true;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_REC)
.onchange = video_mode_onchange,
 .onkey = video_mode_onkey,
  .ontouch = NULL,
};

static int parking_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        ui_register_msg_handler(ID_WINDOW_PARKING, rec_msg_handler); /* 注册APP消息响应 */
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_PARKING)
.onchange = parking_page_onchange,
 .ontouch = NULL,
};
/*********************************************************************************
 *  		     				菜单动作
 *********************************************************************************/
enum video_menu_rec_enum {
    MENU_RES_REC = 0,
    MENU_TWO_REC,
    MENU_CYC_REC,
    MENU_GAP_REC,
    MENU_HDR_REC,
    MENU_EXP_REC,
    MENU_MOTD_REC,
    MENU_MIC_REC,
    MENU_LABEL_REC,
    MENU_GRAV_REC,
    MENU_PARK_REC,
    MENU_NUM_REC,
    /* MENU_MN_LANE_REC, */
    MENU_FLIG_REC,
};

static int video_menu_rec_table[] = {
    LAYOUT_MN_RES_REC,
    LAYOUT_MN_TWO_REC,
    LAYOUT_MN_CYC_REC,
    LAYOUT_MN_GAP_REC,
    LAYOUT_MN_HDR_REC,
    LAYOUT_MN_EXP_REC,
    LAYOUT_MN_MOTD_REC,
    LAYOUT_MN_MIC_REC,
    LAYOUT_MN_LABEL_REC,
    LAYOUT_MN_GRAV_REC,
    LAYOUT_MN_PARK_REC,
    LAYOUT_MN_NUM_REC,
    /* LAYOUT_MN_LANE_REC, */
    LAYOUT_MN_FLIG_REC,
};

static int video_menu_rec_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    static int sel_item = 0;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        ASSERT(sel_item < (sizeof(video_menu_rec_table) / sizeof(int)));
#ifdef CONFIG_VIDEO4_ENABLE
        if (sel_item != MENU_RES_REC &&
            sel_item != MENU_TWO_REC &&
            sel_item != MENU_GAP_REC &&
            sel_item != MENU_HDR_REC &&
            sel_item != MENU_EXP_REC &&
            sel_item != MENU_MOTD_REC &&
            sel_item != MENU_NUM_REC &&
            sel_item != MENU_FLIG_REC)
#endif
            ui_show(video_menu_rec_table[sel_item]);

        break;
    case KEY_DOWN:
        return FALSE;
        /*
         * 向后分发消息
         */

        break;
    case KEY_UP:
        return FALSE;
        /*
         * 向后分发消息
         */

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYER_MENU_REC);
        ui_show(ID_WINDOW_VIDEO_SYS);
        break;

    default:
        return false;
        break;
    }

    return true;
    /*
     * 不向后分发消息
     */
}


REGISTER_UI_EVENT_HANDLER(VLIST_REC_REC)
.onkey = video_menu_rec_onkey,
 .ontouch = NULL,
};



/***************************** 分辨率设置 ************************************/

static int menu_res_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("res"), TABLE(table_video_resolution));
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_res_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "res";
        it.exdata = table_video_resolution[sel_item];
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_RES_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_RES_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_RES_REC)
.onchange = menu_res_onchange,
 .onkey = menu_res_onkey,
  .ontouch = NULL,
};


/***************************** 双路录像设置 ************************************/
static int menu_double_route_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("two"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_double_route_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "two";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("two onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_TWO_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_TWO_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_TWO_REC)
.onchange = menu_double_route_onchange,
 .onkey = menu_double_route_onkey,
  .ontouch = NULL,
};


/***************************** 循环录影设置 ************************************/
static int menu_cyc_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("cyc"), TABLE(table_video_cycle));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_cyc_rec_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "cyc";
        it.exdata = table_video_cycle[sel_item];
        err = start_app(&it);
        if (err) {
            printf("cyc onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_CYC_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_CYC_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_CYC_REC)
.onchange = menu_cyc_rec_onchange,
 .onkey = menu_cyc_rec_onkey,
  .ontouch = NULL,
};



/***************************** 间隔录像设置 ************************************/
static int menu_gap_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table16(db_select("gap"), TABLE(table_video_gap));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_gap_rec_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "gap";
        it.exdata = table_video_gap[sel_item];
        err = start_app(&it);
        if (err) {
            printf("gap onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_GAP_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_GAP_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_GAP_REC)
.onchange = menu_gap_rec_onchange,
 .onkey = menu_gap_rec_onkey,
  .ontouch = NULL,
};



/***************************** 动态范围设置 ************************************/
static int menu_wdr_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("wdr"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_wdr_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "wdr";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("wdr onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_HDR_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_HDR_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_HDR_REC)
.onchange = menu_wdr_onchange,
 .onkey = menu_wdr_onkey,
  .ontouch = NULL,
};



/***************************** 曝光补偿设置 ************************************/

static int menu_exposure_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("exp"), TABLE(table_video_exposure));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_exposure_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "exp";
        it.exdata = table_video_exposure[sel_item];
        err = start_app(&it);
        if (err) {
            printf("exp onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_EXP_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_EXP_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_EXP_REC)
.onchange = menu_exposure_onchange,
 .onkey = menu_exposure_onkey,
  .ontouch = NULL,
};



/***************************** 运动检测设置 ************************************/

static int menu_motdet_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("mot"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_motdet_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "mot";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("mot onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_MOTD_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_MOTD_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_MOTD_REC)
.onchange = menu_motdet_onchange,
 .onkey = menu_motdet_onkey,
  .ontouch = NULL,
};



/***************************** 录音设置 ************************************/

static int menu_audio_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("mic"));
        break;
    default:
        return FALSE;
    }

    return FALSE;
}

static int menu_audio_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "mic";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("audio onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_MIC_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_MIC_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_MIC_REC)
.onchange = menu_audio_onchange,
 .onkey = menu_audio_onkey,
  .ontouch = NULL,
};


/***************************** 日期标签设置 ************************************/

static int menu_date_label_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("dat"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_date_label_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "dat";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("date label onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_LABEL_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_LABEL_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_LABEL_REC)
.onchange = menu_date_label_onchange,
 .onkey = menu_date_label_onkey,
  .ontouch = NULL,
};


/***************************** 重力感应设置 ************************************/

static int menu_gravity_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("gra"), TABLE(table_video_gravity));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_gravity_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "gra";
        it.exdata = table_video_gravity[sel_item];
        err = start_app(&it);
        if (err) {
            printf("gravity onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_GRAV_REC);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_GRAV_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_GRAV_REC)
.onchange = menu_gravity_onchange,
 .onkey = menu_gravity_onkey,
  .ontouch = NULL,
};



/***************************** 停车守卫设置 ************************************/
static int menu_parking_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("par"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_parking_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "par";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("park onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_PARK_REC);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_PARK_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_PARK_REC)
.onchange = menu_parking_onchange,
 .onkey = menu_parking_onkey,
  .ontouch = NULL,
};


/***************************** 车牌号码开关设置 ************************************/
static int menu_car_num_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("num"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_car_num_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "num";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("res car num err! %d\n", err);
            break;
        }

        ui_hide(LAYOUT_MN_NUM_REC);
        if (sel_item) {
            ui_show(LAYOUT_MN_CARNUM_SET_REC);
        }

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_NUM_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_NUM_REC)
.onchange = menu_car_num_onchange,
 .onkey = menu_car_num_onkey,
  .ontouch = NULL,
};
/***************************** 车道偏移开关设置 ************************************/
static int menu_lane_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, !!db_select("lan"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_lane_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        if (sel_item == 0) {
            menu_lane_det_set(0);
            ui_hide(LAYOUT_MN_LANE_REC);
        } else {
            struct intent it;
            ui_hide(ui_get_current_window_id());
            rec_exit_menu_post();
            init_intent(&it);
            it.name = "video_rec";
            it.action = ACTION_BACK;
            start_app_async(&it, open_set_lane_page, NULL);
            break;
        }

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_LANE_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_LANE_REC)
.onchange = menu_lane_onchange,
 .onkey = menu_lane_onkey,
  .ontouch = NULL,
};
/***************************** 前照灯开关设置 ************************************/
static int menu_flig_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("hlw"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_flig_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "hlw";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("head light warning err! %d\n", err);
            break;
        }

        ui_hide(LAYOUT_MN_FLIG_REC);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_FLIG_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_FLIG_REC)
.onchange = menu_flig_onchange,
 .onkey = menu_flig_onkey,
  .ontouch = NULL,
};
/***************************** 车牌号码设置 ************************************/

static u16 __car_num_search_by_mark(const char *mark)
{
    u16 p = 0;
    u16 table_sum = sizeof(text_car_num_table) / sizeof(struct car_num);
    while (p < table_sum) {
        if (!strcmp(mark, text_car_num_table[p].mark)) {
            return p;
        }
        p++;
    }
    return -1;
}
static u8 __car_num_reset(void)
{
    car_num_set_p = 0xff;
    return 0;
}
static u8 __car_num_switch(enum sw_dir dir, const char *mark)
{
    u16 table_sum;
    u16 prev_set_p = car_num_set_p;
    u16 p;

    table_sum = sizeof(text_car_num_table) / sizeof(struct car_num);

    ASSERT(dir == DIR_NEXT || dir == DIR_PREV || dir == DIR_SET);
    switch (dir) {
    case DIR_NEXT:
        if (car_num_set_p >= (table_sum - 1)) {
            car_num_set_p = 0;
        } else {
            car_num_set_p++;
        }
        break;
    case DIR_PREV:
        if (car_num_set_p == 0) {
            car_num_set_p = (table_sum - 1);
        } else {
            car_num_set_p--;
        }
        break;
    case DIR_SET:
        p = __car_num_search_by_mark(mark);
        if (p == (u16) - 1) {
            return -1;
        }
        car_num_set_p = p;

        break;
    }
    if (prev_set_p != 0xff) {
        ui_no_highlight_element_by_id(text_car_num_table[prev_set_p].text_id);
    }
    ui_highlight_element_by_id(text_car_num_table[car_num_set_p].text_id);

    return 0;

}
static u8 __car_num_update_show(u8 pos) /* 更新单个时间控件的时间 */
{
    ui_text_show_index_by_id(text_car_num_table[pos].text_id, text_car_num_table[pos].text_index);
    return 0;
}
static u8 __car_num_set_value(enum set_mod mod, u16 value)
{

    u8 p;
    ASSERT(mod == MOD_ADD || mod == MOD_DEC || mod == MOD_SET);
    switch (mod) {
    case MOD_ADD:
        switch (car_num_set_p) {
        case 0: /* province */
            value = text_car_num_table[car_num_set_p].text_index + 1;
            if (value >= 31) {
                value = 0;
            }
            break;
        case 1: /* town */
        case 2: /* a */
        case 3: /* b */
        case 4: /* c */
        case 5: /* d */
        case 6: /* e */
            value = text_car_num_table[car_num_set_p].text_index + 1;
            if (value >= (('Z' - 'A' + 1) + ('9' - '0' + 1))) {
                value = 0;
            }
            break;
        default:
            ASSERT(0, "mod_add car_num_set_p:%d err!", car_num_set_p);
            break;
        }
        break;
    case MOD_DEC:
        switch (car_num_set_p) {
        case 0: /* province */
            value = text_car_num_table[car_num_set_p].text_index;
            if (value == 0) {
                value = 30;
            } else {
                value--;
            }
            break;
        case 1: /* town */
        case 2: /* a */
        case 3: /* b */
        case 4: /* c */
        case 5: /* d */
        case 6: /* e */
            value = text_car_num_table[car_num_set_p].text_index;
            if (value == 0) {
                value = ('Z' - 'A' + 1) + ('9' - '0' + 1) - 1;
            } else {
                value--;
            }
            break;
        default:
            ASSERT(0, "mod_dec car_num_set_p:%d err!", car_num_set_p);
            break;
        }
        break;
    case MOD_SET:
        switch (car_num_set_p) {
        case 0: /* province */
            ASSERT(value >= 0 && value <= 30, "car num set value err!\n");
            break;
        case 1: /* town */
        case 2: /* a */
        case 3: /* b */
        case 4: /* c */
        case 5: /* d */
        case 6: /* e */
            ASSERT(value >= 0 && value <= (('Z' - 'A' + 1) + ('9' - '0' + 1) - 1));
            break;
        default:
            ASSERT(0, "mod_set car_num_set_p:%d err!", car_num_set_p);
            break;
        }
        break;
    }
    text_car_num_table[car_num_set_p].text_index = value;

    printf("car_num_set p:%d, value:%d\n", car_num_set_p, value);
    __car_num_update_show(car_num_set_p); /* 更新当前位显示 */

    return 0;
}

static u8 __car_num_get(struct car_num_str *num)
{
    num->province = text_car_num_table[0].text_index;
    num->town = text_car_num_table[1].text_index;
    num->a = text_car_num_table[2].text_index;
    num->b = text_car_num_table[3].text_index;
    num->c = text_car_num_table[4].text_index;
    num->d = text_car_num_table[5].text_index;
    num->e = text_car_num_table[6].text_index;
    return 0;
}

static u8 __car_num_set_by_ascii(const char *str)
{
    u16 i;
    u16 province;
    /* ASSERT(strlen(str) == 8, "car num err!"); */

    memcpy((char *)&province, str, 2);
    text_car_num_table[0].text_index = 0;
    for (i = 0; i < (sizeof(province_gb2312) / sizeof(u16)); i++) {
        if (province_gb2312[i] == province) {
            text_car_num_table[0].text_index = i;
            break;
        }
    }
    text_car_num_table[1].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[2]) {
            text_car_num_table[1].text_index = i;
            break;
        }
    }
    text_car_num_table[2].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[3]) {
            text_car_num_table[2].text_index = i;
            break;
        }
    }
    text_car_num_table[3].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[4]) {
            text_car_num_table[3].text_index = i;
            break;
        }
    }
    text_car_num_table[4].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[5]) {
            text_car_num_table[4].text_index = i;
            break;
        }
    }
    text_car_num_table[5].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[6]) {
            text_car_num_table[5].text_index = i;
            break;
        }
    }
    text_car_num_table[6].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[7]) {
            text_car_num_table[6].text_index = i;
            break;
        }
    }
    return 0;

}
int menu_rec_car_num_set(void)
{
    struct intent it;
    int err;
    struct car_num_str num;

    __car_num_get(&num);
    u32 part_a = (province_gb2312[num.province] << 16) |
                 (num_table[num.town] << 8) | num_table[num.a];
    u32 part_b = (num_table[num.b] << 24) | (num_table[num.c] << 16) |
                 (num_table[num.d] << 8)  |  num_table[num.e];

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "cna";
    it.exdata = part_a;
    err = start_app(&it);
    if (err) {
        printf("num set onkey err! %d\n", err);
        return -1;
    }
    it.data = "cnb";
    it.exdata = part_b;
    err = start_app(&it);

    return 0;
}

static int menu_layout_car_num_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        layout_on_focus(layout);
        __car_num_reset();
        break;
    case ON_CHANGE_RELEASE:
        layout_lose_focus(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        __car_num_switch(DIR_SET, "province");
        break;
    default:
        return false;
    }
    return false;
}
static int menu_layout_car_num_set_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        __car_num_switch(DIR_NEXT, NULL);

        break;
    case KEY_DOWN:
        __car_num_set_value(MOD_ADD, 0);
        return FALSE;

        break;
    case KEY_UP:
        __car_num_set_value(MOD_DEC, 0);
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_CARNUM_SET_REC);
        menu_rec_car_num_set();

        break;
    default:
        return false;
        break;
    }

    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYOUT_MN_CARNUM_SET_REC)
.onchange = menu_layout_car_num_set_onchange,
 .onkey = menu_layout_car_num_set_onkey,
  .ontouch = NULL,
};




/*
 * (begin)菜单中车牌号码的显示初始化动作 ********************************************
 */
static int text_menu_car_num_province_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[0].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_PROVINCE)
.onchange = text_menu_car_num_province_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_town_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[1].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_TOWN)
.onchange = text_menu_car_num_town_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_a_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[2].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_A)
.onchange = text_menu_car_num_a_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_b_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[3].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_B)
.onchange = text_menu_car_num_b_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_c_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[4].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_C)
.onchange = text_menu_car_num_c_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int text_menu_car_num_d_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[5].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_D)
.onchange = text_menu_car_num_d_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int text_menu_car_num_e_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[6].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_E)
.onchange = text_menu_car_num_e_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
/*
 * (end)---------------------------------------------------------------
 */




static u8 lane_set_flag = 0;
static int menu_lane_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        layout_on_focus(layout);
        __car_num_reset();
        break;
    case ON_CHANGE_RELEASE:
        layout_lose_focus(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        __this->vanish_line    = ((db_select("lan") >> 16) & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        __this->car_head_line = (db_select("lan") & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        if (__this->vanish_line < 30) {
            __this->vanish_line = 30;
        }
        if (__this->car_head_line > LCD_DEV_HIGHT - 30 || __this->car_head_line < __this->vanish_line + 30) {
            __this->car_head_line = LCD_DEV_HIGHT - 30 ;
        }
        lane_set_flag = 0;
        break;
    default:
        return false;
    }
    return false;
}

#define LANE_LINSE_STEP_SMALL  3
#define LANE_LINSE_STEP_BIG    30
static int menu_lane_set_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    switch (e->value) {
    case KEY_DOWN:
        if (!(lane_set_flag & 0x02)) {
            lane_set_flag |= 0x02;
            send_lane_det_setting_msg("lane_set_close", 0);
        }
        if ((lane_set_flag & 0x01) == 0) {
            if (e->event == KEY_EVENT_CLICK) {
                __this->vanish_line += LANE_LINSE_STEP_SMALL;
            } else if (e->event == KEY_EVENT_HOLD) {
                __this->vanish_line += LANE_LINSE_STEP_BIG;
            }
            __this->vanish_line = __this->vanish_line > __this->car_head_line - 30 ? __this->car_head_line - 30 : __this->vanish_line;
            ui_hide(TEXT_LANE_TOP);
            ui_show(TEXT_LANE_TOP);
        } else if ((lane_set_flag & 0x01) == 1) {
            if (e->event == KEY_EVENT_CLICK) {
                __this->car_head_line += LANE_LINSE_STEP_SMALL;
            } else if (e->event == KEY_EVENT_HOLD) {
                __this->car_head_line += LANE_LINSE_STEP_BIG;
            }
            __this->car_head_line = __this->car_head_line > LCD_DEV_HIGHT - 30 ? LCD_DEV_HIGHT - 30 : __this->car_head_line;
            ui_hide(TEXT_LANE_BOTTOM);
            ui_show(TEXT_LANE_BOTTOM);
        }
        break;
    case KEY_UP:
        if (!(lane_set_flag & 0x02)) {
            lane_set_flag |= 0x02;
            send_lane_det_setting_msg("lane_set_close", 0);
        }
        if ((lane_set_flag & 0x01) == 0) {
            if (e->event == KEY_EVENT_CLICK) {
                __this->vanish_line -= LANE_LINSE_STEP_SMALL;
            } else if (e->event == KEY_EVENT_HOLD) {
                __this->vanish_line -= LANE_LINSE_STEP_BIG;
            }
            __this->vanish_line = __this->vanish_line < 30 ? 30 : __this->vanish_line;
            ui_hide(TEXT_LANE_TOP);
            ui_show(TEXT_LANE_TOP);
        } else if ((lane_set_flag & 0x01) == 1) {
            if (e->event == KEY_EVENT_CLICK) {
                __this->car_head_line -= LANE_LINSE_STEP_SMALL;
            } else if (e->event == KEY_EVENT_HOLD) {
                __this->car_head_line -= LANE_LINSE_STEP_BIG;
            }
            __this->car_head_line = __this->car_head_line < __this->vanish_line + 30 ? __this->vanish_line + 30 : __this->car_head_line;
            ui_hide(TEXT_LANE_BOTTOM);
            ui_show(TEXT_LANE_BOTTOM);
        }
        break;
    case KEY_OK:
        if (lane_set_flag & 0x02) {
            lane_set_flag &= 0x01;
            int top = __this->vanish_line * 352 / LCD_DEV_HIGHT;
            int bottom = __this->car_head_line * 352 / LCD_DEV_HIGHT;
            menu_lane_det_set(top << 16 | bottom);
            printf("lane set top=%d , bottom=%d\n", top, bottom);

            send_lane_det_setting_msg("lane_set_open", top << 16 | bottom);
        }
        lane_set_flag = !lane_set_flag;
        break;
    case KEY_MODE:
        if (lane_set_flag & 0x02 || lane_set_flag == 0) {
            int top = __this->vanish_line * 352 / LCD_DEV_HIGHT;
            int bottom = __this->car_head_line * 352 / LCD_DEV_HIGHT;
            menu_lane_det_set(top << 16 | bottom);
            printf("lane set top=%d , bottom=%d\n", top, bottom);
        }
        sys_key_event_takeover(false, false);
        close_set_lane_page(NULL);
        break;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYOUT_LANE_REC)
.onchange = menu_lane_set_onchange,
 .onkey = menu_lane_set_onkey,
  .ontouch = NULL,
};


static int lane_set_txt_top_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct draw_context *dc = (struct draw_context *)arg;

    switch (e) {
    case ON_CHANGE_SHOW:
        dc->rect.top = __this->vanish_line;
        dc->draw.top = dc->rect.top;
        break;
    default:
        return false;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_LANE_TOP)
.onchange = lane_set_txt_top_onchange,
};
static int lane_set_txt_bottom_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct draw_context *dc = (struct draw_context *)arg;

    switch (e) {
    case ON_CHANGE_SHOW:
        dc->rect.top = __this->car_head_line - 30;
        dc->draw.top = dc->rect.top;
        break;
    default:
        return false;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_LANE_BOTTOM)
.onchange = lane_set_txt_bottom_onchange,
};


/******************************************************************************************************/
/****** 以下为对讲工程的UI响应部分 ****************/
#ifdef  CONFIG_IPC_UI_ENABLE
void rec_call_prompt_close_time_handler();   //定时显示通话挂断提示，6s之后才可挂断

u32 update_net_connect_pic_timer = 0;
u32 update_net_connect_pic_num = 0;
static u32 rec_call_prompt_close_timer = 0;     //定时显示通话挂断提示，6s之后才可挂断
u32 longtime_idle_return_timer = 0;             //长时间无操作返回主界面定时器（部分界面）
void longtime_idle_return_time_handler();       //长时间无操作返回主界面服务函数（部分界面）
void update_longtime_idle_return_timer(u8 state);   //更新返回长时间无操作返回定时器
int notify_msg_to_video_rec_async(int action,const char *data,u32 exdata);
enum machine_dec_state get_dec_state();

extern u8 msging_countdown;

/*
 * (begin)提示框显示接口
 */
enum box_tip {
    BOX_TIP_CALLING_CLOSE=1,
    BOX_TIP_CALLING_BUSY,
    BOX_TIP_CALLING_TIMEOUT,
    BOX_TIP_INSERT_SD,

//    BOX_MSG_NO_POWER = 1,
//    BOX_MSG_MEM_ERR,
//    BOX_MSG_NO_MEM,
//    BOX_MSG_NEED_FORMAT,
//    BOX_MSG_INSERT_SD,
//    BOX_MSG_DEFAULT_SET,
//    BOX_MSG_FORMATTING,
//    BOX_MSG_10S_SHUTDOWN,
//    BOX_MSG_SD_ERR,
//    BOX_MSG_SD_WRITE_ERR,
};
static u8 tip_show_f = 0;
static enum box_tip tip_show_id = 0;
static void __rec_tip_hide(enum box_tip id)
{
    printf("\n /****** [%s] %d id = %d\n",__func__,__LINE__,id);
    if (tip_show_id == id) {
        if (tip_show_f) {
            tip_show_f = 0;
            ui_hide(LAYER_CALLING_EVENT);
        }
#ifdef CONFIG_IPC_UI_ENABLE
//        if(video_rec_in_state == REC_STA_IN_MSG){
//            ui_hide(LAYER_MSG_MODE);
//            ui_show(LAYER_STANDBY);
//        }
#endif // CONFIG_IPC_UI_ENABLE
    } else if (id == 0) {
        /*
         * 没有指定ID，强制隐藏
         */
        if (tip_show_f) {
            tip_show_f = 0;
            ui_hide(LAYER_CALLING_EVENT);
        }
    }
}
static void __rec_tip_timeout_func(void *priv)
{
    __rec_tip_hide((enum box_tip)priv);
}
static void __rec_tip_show(enum box_tip tip, u32 timeout_msec)
{
    printf("\n /****** [%s] %d tip = %d, tip_show_id\n",__func__,__LINE__,tip,msg_show_id);
    static int t_id = 0;
    if (tip == 0) {
        ASSERT(0, "__rec_msg_show tip 0!\n");
        return;
    }

    if (tip == tip_show_id) {
        printf("\n /****** %s %d\n",__func__,__LINE__);
        if (msg_show_f == 0) {
            printf("\n /****** %s %d\n",__func__,__LINE__);
            msg_show_f = 1;

//            ui_show(LAYER_MSG_REC);
//            ui_text_show_index_by_id(TEXT_MSG_REC, tip - 1);
            ui_show(LAYER_CALLING_EVENT);
            if(tip == BOX_TIP_INSERT_SD){
               ui_show(LAYOUT_MSG_NO_CARD);
            }

            if (t_id) {
                sys_timeout_del(t_id);
                t_id = 0;
            }
            if (timeout_msec > 0) {
                t_id = sys_timeout_add((void *)tip, __rec_tip_timeout_func, timeout_msec);
            }
        }
    } else {
        printf("\n /****** %s %d\n",__func__,__LINE__);
        msg_show_id = tip;
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(LAYER_CALLING_EVENT);
            if(tip == BOX_TIP_INSERT_SD){
               ui_show(LAYOUT_MSG_NO_CARD);
            }
        }
//        ui_text_show_index_by_id(TEXT_MSG_REC, tip - 1);
        if (t_id) {
            sys_timeout_del(t_id);
            t_id = 0;
        }
        if (timeout_msec > 0) {
            t_id = sys_timeout_add((void *)tip, __rec_tip_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */

/******************  post msg响应处   **************************/
#define STANDBY_PIC_NUM 8
uint64_t get_utc_ms(void);
void covUnixTimeStp2Beijing(uint64_t unixTime,  struct tm *p);

enum weather_ui_pic{
    weather_sun = 0,
    weather_cloudy_morni,
    weather_cloudy_night,
    weather_cloudy_morni_less,
    weather_cloudy,
    weather_rain_morni,
    weather_rain_night,
    weather_rain,
    weather_snowy_morni,
    weather_snowy_night,
    weather_snowy,
    weather_fog,
    weather_thunder_morni,
    weather_thunder_night,
    weather_thunder,
    weather_rain_big,
    weather_frost,
};
enum weather_ui_pic app_weather_base[]={
    weather_sun,    weather_sun,    weather_cloudy_morni,    weather_cloudy_morni_less,    weather_cloudy,    //1~5
    weather_rain,    weather_rain,    weather_rain,    weather_rain,    weather_snowy,    //6~10
    weather_snowy,    weather_fog,    weather_fog,    weather_fog,    weather_fog,    //11~15
    weather_sun,    weather_sun,    weather_sun,    weather_sun,    weather_sun,    //16~20
    weather_thunder,    weather_thunder,    weather_thunder,    weather_sun,    weather_sun,    //21~25
    weather_snowy,    weather_snowy,    weather_rain,    weather_rain,    weather_rain,    //26~30
    weather_rain,    weather_rain,    weather_rain,    weather_rain,    weather_snowy,    //31~35
    weather_snowy,    weather_snowy,    weather_snowy,    weather_snowy,    weather_rain,    //36~40
    weather_rain,    weather_rain,    weather_rain,    weather_snowy,    weather_sun,    //41~45
};
#ifndef IPC_DEVELOP_BOARD_DEMO
static void update_unread_msg_show()
{
    int get_msg_ro_attr_num(void);
    u8 unread_msg_num = get_msg_ro_attr_num();
//        u8 unread_msg_num_to_show[2] = {0};
    if(unread_msg_num != 0){
//        unread_msg_num_to_show[1] = '0' + unread_msg_num/10;
//        unread_msg_num_to_show[0] = '0' + unread_msg_num%10;
        printf("\n /**** unread msg num =%d\n ",unread_msg_num);
        ui_show(TXT_NEW_MSG_TIP_NUM);
        ui_text_show_index_by_id(TXT_NEW_MSG_TIP, 0);
        if(unread_msg_num < 10){
            printf("\n /*** unread_msg_num < 10\n");
            ui_text_show_index_by_id(TXT_NEW_MSG_TIP_NUM,unread_msg_num);
//            ui_text_set_textu_by_id(TXT_NEW_MSG_TIP_NUM,'0',1, FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
        }else if(unread_msg_num < 100){
            printf("\n /*** unread_msg_num < 100\n");
            ui_text_show_index_by_id(TXT_NEW_MSG_TIP_NUM,unread_msg_num);
//            ui_text_set_textu_by_id(TXT_NEW_MSG_TIP_NUM,unread_msg_num_to_show,2, FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
        }else {
            printf("\n /*** unread_msg_num > 100\n");
            ui_text_show_index_by_id(TXT_NEW_MSG_TIP_NUM,100);
//            ui_text_set_textu_by_id(TXT_NEW_MSG_TIP_NUM,unread_msg_num_to_show,3, FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
        }
//        ui_text_show_index_by_id(TXT_NEW_MSG_TIP_NUM,100);
        ui_highlight_element_by_id(STANDBY_BTN_DEC);
    }else{
        ui_hide(TXT_NEW_MSG_TIP_NUM);
        ui_text_show_index_by_id(TXT_NEW_MSG_TIP, 1);
        ui_no_highlight_element_by_id(STANDBY_BTN_DEC);
    }
        return ;
}
#endif
struct tm standby_time= {0};
static int standby_update_time_handler(const char *type, u32 arg)
{
   // printf("[%s] %d\n",__func__,__LINE__);

    covUnixTimeStp2Beijing(get_utc_ms() / 1000,&standby_time);

//    printf("/********* standby_time y = %d, mon = %d, day = %d, hour = %d, min= %d",standby_time->tm_year,standby_time->tm_mon,\
//                                                                standby_time->tm_mday,standby_time->tm_hour,standby_time->tm_min);
//    printf("/********* standby_time y = %d, mon = %d, day = %d, hour = %d, min= %d\n",standby_time.tm_year,standby_time.tm_mon,\
//                                                                standby_time.tm_mday,standby_time.tm_hour,standby_time.tm_min);
//    ui_pic_show_image_by_id(PIC_YEAR1, 2);
//    ui_pic_show_image_by_id(PIC_YEAR2, 0);
//    ui_pic_show_image_by_id(PIC_YEAR3, (u16)standby_time.tm_year%100/10);
//    ui_pic_show_image_by_id(PIC_YEAR4, (u16)standby_time.tm_year%10);

        ui_pic_show_image_by_id(PIC_MON1, (u8)standby_time.tm_mon/10);
        ui_pic_show_image_by_id(PIC_MON2, (u8)standby_time.tm_mon%10);
        ui_pic_show_image_by_id(PIC_DAY1, (u8)standby_time.tm_mday/10);
        ui_pic_show_image_by_id(PIC_DAY2, (u8)standby_time.tm_mday%10);
        ui_pic_show_image_by_id(PIC_HOUR1, (u8)standby_time.tm_hour/10);
        ui_pic_show_image_by_id(PIC_HOUR2, (u8)standby_time.tm_hour%10);
        ui_pic_show_image_by_id(PIC_MIN1, (u8)standby_time.tm_min/10);
        ui_pic_show_image_by_id(PIC_MIN2, (u8)standby_time.tm_min%10);
        ui_text_show_index_by_id(TXT_WEEKDAY, (u8)standby_time.tm_wday-1);

       // update_unread_msg_show();
       #ifndef LONG_POWER_IPC //rtl8189 获取信号强度
        ui_pic_show_image_by_id(PIC_WIFI_SIGNAL_STRENGTH,system_get_network_info_qos_for_ui()-3);  //WIFI信号强度图标
       #else
        ui_pic_show_image_by_id(PIC_WIFI_SIGNAL_STRENGTH,system_get_network_info_qos_for_ui());  //WIFI信号强度图标
       #endif
        return 0;
}


 #ifndef IPC_DEVELOP_BOARD_DEMO
iot_weatherInfo_t weather_p = {0};
static int standby_update_weather_handler(const char *type, u32 arg)
{
//    printf("[%s] %d\n",__func__,__LINE__);
    void get_weather(iot_weatherInfo_t *p);
    get_weather(&weather_p);
    ui_pic_show_image_by_id(PIC_TEM1, weather_p.info.temp/10);
    ui_pic_show_image_by_id(PIC_TEM2, weather_p.info.temp%10);

//    if(weather_p.city_name){
//        printf("\n/***** %s\n",weather_p.city_name);
//    }

    printf("\n /***** weather condition = %d\n",weather_p.info.condition);
    printf("\n /***** weather num = %d\n",app_weather_base[weather_p.info.condition-1]);
    put_buf(weather_p.city_name,sizeof(weather_p.city_name));
    ui_text_set_textu_by_id(TXT_STANDBY_CITY,weather_p.city_name,calculate_utf8_length(weather_p.city_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
    ui_text_show_index_by_id(STANDBY_TXT_WEATHER,weather_p.info.condition-1);
    ui_pic_show_image_by_id(STANDBY_PIC_WEATHER, (int)app_weather_base[weather_p.info.condition-1]);




    return 0;
}
#endif // IPC_DEVELOP_BOARD_DEMO
static void rec_call_prompt_close_time_handler()
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_show(LAYOUT_CALLING_REC);

    if(rec_call_prompt_close_timer){
        sys_timeout_del(rec_call_prompt_close_timer);
        rec_call_prompt_close_timer = 0;
    }
}
static int calling_request_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    printf("sel_contact_flag = %d\n",sel_contact_flag);

    if(sel_contact_flag == 0){
        ui_hide(LAYER_STANDBY);
        ui_show(LAYER_CALL_REC);
        ui_show(LAYOUT_CALLING);
    }
    else if(sel_contact_flag == 1){     //选联系人流程
        ui_hide(LAYOUT_CONTACT_SEL);
//        os_time_dly(1);
        ui_show(LAYOUT_CALLING);

        sel_contact_flag = 0;
    }
    rec_call_prompt_close_timer = sys_timeout_add(NULL,rec_call_prompt_close_time_handler,6*1000);
    video_last_call_state = video_call_state;
    video_call_state = CALL_STA_STARTING;       //拨号中
    return 0;
}


static int LIST_CONTACT_TXT_TABLE[]={
    TXT_CONTACT1,
    TXT_CONTACT2,
    TXT_CONTACT3,
};

//const static u8 utf8_contact1_name[] = {
//    0xE6,0xB7,0xB1,0xE5,0x9C,0xB3       //深圳
//};

const static u8 utf8_test_name[] = {
	0xE6, 0x96, 0x87, 0xE5, 0xAD, 0x97, 0xE6, 0x8E, 0xA7, 0xE4, 0xBB, 0xB6, 0xE5, 0xAF, 0xB9, 0xE9,
	0xBD, 0x90, 0xE6, 0xB5, 0x8B, 0xE8, 0xAF, 0x95
};
user_list_t *contact_p;
static int sel_contact_handler(const char *type, u32 arg)
{
    video_call_state = CALL_STA_GET_CONTACT;//选择联系人中
    video_rec_in_state = REC_STA_IN_CALL;//选择联系人中
    printf("\n >>>>>>>>>[%s] %d arg = %x\n",__func__,__LINE__,arg);
    ui_hide(LAYER_STANDBY);

    ui_show(LAYER_CALL_REC);
    update_longtime_idle_return_timer(0);//0开启 1更新 2关闭

    return 0;
}

static int set_contact_finish_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYOUT_CONTACT_SEL);
    ui_show(LAYOUT_CALLING);
    return 0;
}

static int app_accept_call_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYOUT_CALLING);

//    it.name	= "video_rec";
//    it.action = ACTION_VIDEO_REC_SET_CONFIG;
//    it.data = "";
//    int err = start_app_async(&it, NULL, NULL);
//    if (err) {
//        printf("msg change err! %d\n", err);
//    }
    video_last_call_state = video_call_state;
    video_call_state = CALL_STA_TALKING;        //通话中
    return 0;
}

static int accept_call_without_video_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
//    ui_hide(LAYOUT_CALLING);
    ui_text_show_index_by_id(TXT_CALLING,1);
    video_last_call_state = video_call_state;
    video_call_state = CALL_STA_TALKING_WITHOUT_VIDEO;        //语音通话
    return 0;
}

static int app_launch_call_handler(const char *type, u32 arg)   //app主动通话
{
    update_longtime_idle_return_timer(2);//0开启 1更新 2关闭
    printf("[%s] %d last_call_state = %d\n",__func__,__LINE__,video_last_call_state);
    printf("[%s] %d video_call_state = %d\n",__func__,__LINE__,video_call_state);
    if(video_call_state == CALL_STA_IDLE){
        ui_hide(LAYER_STANDBY);
        ui_show(LAYER_CALL_REC);
        ui_show(LAYOUT_CALLING_REC);
    }else if(video_call_state == CALL_STA_GET_CONTACT){
        ui_hide(LAYOUT_CONTACT_SEL);
        ui_show(LAYOUT_CALLING_REC);
    }
    video_call_state = CALL_STA_TALKING;        //通话中
    return 0;
}
static hund_upflag=0;// 用来分区APP 挂断 还是  设备挂断
static int app_close_call_handler(const char *type, u32 arg)
{
    printf("\n[%s] %d,%s,%s\n",__func__,__LINE__,arg,type);

    if(!strcmp(arg,"machine")){
    hund_upflag=1;
    printf("\n  machine hund up\n");

    }else if(!strcmp(arg,"app")){

    hund_upflag=0;
    printf("\n  app hund up\n");
    }
    ui_hide(LAYER_CALL_REC);
    ui_show(LAYER_CALLING_EVENT);
    os_time_dly(1);
    ui_show(LAYOUT_CALLING_CLOSE);

    ui_show(LAYOUT_CALL_TIME);//时间布局
    os_time_dly(1);
    ui_show(CALL_TIME_CTROL);//时间控件


//    ui_show(LAYOUT_CALLING_CLOSE);
//    ui_show(TXT_CALLING_CLOSE);
    if(rec_call_prompt_close_timer){
        printf("\n /*** del close_timer\n");
        sys_timeout_del(rec_call_prompt_close_timer);
        rec_call_prompt_close_timer = 0;
    }
    video_last_call_state = video_call_state;
    video_call_state = CALL_STA_IDLE;           //挂断后，为空闲状态
    return 0;
}

static int machine_close_calling_handler(const char *type, u32 arg)
{
  //  printf("[%s] %d\n",__func__,__LINE__);

    printf("\n[%s] %d,%s,%s\n",__func__,__LINE__,arg,type);
        ui_hide(LAYER_CALL_REC);
        ui_show(LAYER_STANDBY);
//    ui_hide(LAYER_CALL_REC);
//    ui_show(LAYER_CALLING_EVENT);
//    os_time_dly(5);
//    ui_show(LAYOUT_CALLING_CLOSE);
//    ui_show(TXT_CALLING_CLOSE);
    video_last_call_state = video_call_state;
    video_call_state = CALL_STA_IDLE;                   //挂断后，为空闲状态
    return 0;
}

static int app_audio_talk_busy_handler(const char *type, u32 arg)//app_audio_talk_forbid_close
{
    printf("\n[%s] %d\n",__func__,__LINE__);
    ui_show(LAYER_CALLING_EVENT);
    ui_show(LAYOUT_CALLING_BUSY);

    video_last_call_state = video_call_state;
    return 0;
}
static int app_audio_talk_hide_busy_handler(const char *type, u32 arg)//app_audio_talk_forbid_close
{
    printf("\n[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYER_CALLING_EVENT);
    return 0;
}

static int app_refuse_calling_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYER_CALL_REC);
    ui_show(LAYER_CALLING_EVENT);
    os_time_dly(1);
    ui_show(LAYOUT_CALLING_BUSY);
//    ui_show(TXT_CALLING_REFUSED);


    if(rec_call_prompt_close_timer){
        printf("\n /*** del close_timer\n");
        sys_timeout_del(rec_call_prompt_close_timer);
        rec_call_prompt_close_timer = 0;
    }
    return 0;
}

static int calling_timeout_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYER_CALL_REC);
    ui_show(LAYER_CALLING_EVENT);
    os_time_dly(1);
    ui_show(LAYOUT_CALLING_TIMEOUT);
//    ui_show(TXT_CALLING_TIMEOUT);
    video_last_call_state = video_call_state;
    video_call_state = CALL_STA_IDLE;               //超时后，为空闲状态



    usb_speak_switch("off");

    return 0;
}

static int calling_net_connectfail_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYER_CALL_REC);
    ui_show(LAYER_CALLING_EVENT);
    os_time_dly(1);
    ui_show(LAYOUT_CALLING_NET_CONNECTFAIL);

    if(rec_call_prompt_close_timer){
        sys_timeout_del(rec_call_prompt_close_timer);
        rec_call_prompt_close_timer = NULL;
    }
    video_last_call_state = video_call_state;
    video_call_state = CALL_STA_IDLE;         //断网拨号失败，拨号空闲态
    return 0;
}

static int calling_fail_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYER_CALL_REC);
    ui_show(LAYER_CALLING_EVENT);
    os_time_dly(1);
    ui_show(LAYOUT_CALLING_BUSY);

    if(rec_call_prompt_close_timer){
        sys_timeout_del(rec_call_prompt_close_timer);
        rec_call_prompt_close_timer = NULL;
    }
    video_last_call_state = video_call_state;
    video_call_state = CALL_STA_IDLE; //拨号失败,状态为空闲
    return 0;
}


static int network_connect_start_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    os_time_dly(2);
    ui_hide(LAYOUT_LINK_FAIL);
    ui_hide(LAYER_STANDBY);
    os_time_dly(1);
    ui_hide(LAYER_STANDBY);
    ui_show(LAYER_DIST_NET);

    ui_show(LAYOUT_SCAN_CODE);
    return 0;
}

static int msg_upgrade_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    os_time_dly(2);
    ui_hide(LAYOUT_LINK_FAIL);
    ui_hide(LAYER_STANDBY);
    os_time_dly(1);
    ui_hide(LAYER_STANDBY);
    ui_show(UPGRADE_LAYOUT);



    return 0;
}
void update_net_connect_pic()
{
    //printf("[%s] num=%d\n",__func__,update_net_connect_pic_num);
    ui_pic_show_image_by_id(PIC_LINKING_NET, update_net_connect_pic_num++);
    if(update_net_connect_pic_num>4){
        update_net_connect_pic_num=0;
    }
}
static int network_connecting_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYOUT_SCAN_CODE);
    ui_show(LAYOUT_NET_CONNECTING);
    update_net_connect_pic_timer = sys_timer_add(NULL,update_net_connect_pic,50);
    printf("[%s] %d\n",__func__,__LINE__);
    return 0;
}

static int connect_fail_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    if(update_net_connect_pic_timer){
        sys_timer_del(update_net_connect_pic_timer);
    }
    ui_hide(LAYOUT_NET_CONNECTING);
    ui_show(LAYOUT_LINK_FAIL);

    return 0;
}

static int connect_succ_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYOUT_NET_CONNECTING);
    ui_show(LAYOUT_LINK_SUCC);
    if(update_net_connect_pic_timer){
        sys_timer_del(update_net_connect_pic_timer);
    }

    return 0;
}
static int return_ui_to_main_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYER_DIST_NET);
    ui_show(LAYER_STANDBY);

    return 0;
}
static int return_ui_to_connect_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYOUT_LINK_FAIL);
    ui_show(LAYOUT_NET_CONNECTING);
    update_net_connect_pic_timer = sys_timer_add(NULL,update_net_connect_pic,1000);

    return 0;
}

static void distribute_net_ui_event_handler(struct sys_event *event, void *priv)
{
    printf("[%s] %d\n",__func__,__LINE__);
//    if (!strcmp(event->arg, "parking")) {
//        switch (event->u.dev.event) {
//        case DEVICE_EVENT_IN:
//        case DEVICE_EVENT_ONLINE:
//            break;
//        case DEVICE_EVENT_OUT:
//        case DEVICE_EVENT_OFFLINE:
////            if (__this->menu_status) {
////                sys_key_event_takeover(false, false);
////            }
//            break;
//        default:
//            break;
//        }
//    }
}

static int msg_mode_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYER_STANDBY);
    ui_show(LAYER_MSG_MODE);
    update_longtime_idle_return_timer(0);
    return 0;
}

static int msg_finish_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(PIC_MSG_STARTING);
    ui_hide(TXT_MSG_COUNT);
    ui_show(PIC_MSG_NO);
    ui_show(PIC_MSG_YES);
    msging_countdown = 0;

    video_msg_state = MSG_STA_FINISH;
    update_longtime_idle_return_timer(0);//0开启 1更新 2关闭
    return 0;
}

static int msg_update_time_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_text_show_index_by_id(TXT_MSG_COUNT,59-msging_countdown);
    return 0;
}

static int msg_del_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYER_MSG_MODE);
    ui_show(LAYER_STANDBY);
    video_msg_state = MSG_STA_IDLE;
    return 0;
}

static int msg_save_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYER_MSG_MODE);
    msleep(30);
    ui_show(LAYER_STANDBY);
    video_msg_state = MSG_STA_IDLE;
    return 0;
}

static int msg_exit_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_hide(LAYER_MSG_MODE);
    ui_show(LAYER_STANDBY);
    video_msg_state = MSG_STA_IDLE;
    return 0;
}

static int msg_fail_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);

    ui_hide(LAYER_MSG_MODE);
    ui_show(LAYER_CALLING_EVENT);
    os_time_dly(1);
    ui_show(LAYOUT_MSG_FAIL);

    video_msg_state = MSG_STA_IDLE;
    return 0;
}
#ifndef IPC_DEVELOP_BOARD_DEMO
static int new_unread_msg_handler(const char *type, u32 arg)
{
    printf("[%s] %d\n",__func__,__LINE__);
    ui_highlight_element_by_id(STANDBY_BTN_DEC);
    ui_text_show_index_by_id(TXT_NEW_MSG_TIP,0);

//    ui_hide(LAYER_MSG_MODE);
//    ui_show(LAYER_STANDBY);
//    video_msg_state = MSG_STA_IDLE;
    return 0;
}

#endif
static void standby_ui_event_handler(struct sys_event *event, void *priv)
{
    printf("[%s] %d event = %d\n",__func__,__LINE__,event->u.dev.event);
    if (!strcmp(event->arg, "parking")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            break;
        case DEVICE_EVENT_ONLINE:
            break;
        case DEVICE_EVENT_OUT:
            break;
        case DEVICE_EVENT_OFFLINE:
            if (__this->menu_status) {
                printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
                sys_key_event_takeover(false, false);
            }
            break;
        default:
            break;
        }
    }
}


static const struct uimsg_handl standby_msg_handler[] = {
    { "updateTIME",                     standby_update_time_handler  },
   // { "updateWEATHER",                  standby_update_weather_handler  },
    { "calling_request",                calling_request_handler},
    { "machine_close_calling",          machine_close_calling_handler},
    { "sel_contact",                    sel_contact_handler},
    { "set_contact_finish",             set_contact_finish_handler},
    { "app_accept_call",                app_accept_call_handler},
    { "accept_call_without_video",      accept_call_without_video_handler},
    { "app_launch_call",                app_launch_call_handler},
    { "app_close_call",                 app_close_call_handler},
    { "machine_close_call",             app_close_call_handler},
    { "app_refuse_calling",             app_refuse_calling_handler},
    { "app_audio_talk_busy",            app_audio_talk_busy_handler},
    { "app_audio_talk_hide_busy",       app_audio_talk_hide_busy_handler},
    { "calling_timeout",                calling_timeout_handler},
    { "calling_net_connectfail",        calling_net_connectfail_handler},
    { "calling_fail",                   calling_fail_handler},

    { "network_connect_start",      network_connect_start_handler   },
    { "network_connecting",         network_connecting_handler      },
    { "connect_fail",               connect_fail_handler            },
    { "connect_succ",               connect_succ_handler            },
    { "return_ui_to_main",          return_ui_to_main_handler       },
    { "return_ui_to_connect",       return_ui_to_connect_handler    },

    { "msg_finish",                 msg_finish_handler},
    { "msg_update_time",            msg_update_time_handler},
    { "msg_mode",                   msg_mode_handler},
    { "msg_del",                    msg_del_handler},
    { "msg_save",                   msg_save_handler},
    { "msg_exit",                   msg_exit_handler},
    { "msg_fail",                   msg_fail_handler},
    { "upgrade_show",                msg_upgrade_handler},
  // { "new_unread_msg",             new_unread_msg_handler},


    { NULL, NULL},      /* 必须以此结尾！ */
};
/*****************************************************************************/


static u32 standby_update_timer = 0;   //更新主界面时间



static int main_and_rec_onkey(void *ctr, struct element_key_event *e)
{
    struct intent it;
    struct application *app = get_current_app();
    if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {

      //  return true;
    }

    printf("/****** [%s] %d \n",__func__,__LINE__);
    switch (e->value) {
    case KEY_UP:

        break;
    case KEY_DOWN:

        break;
    case KEY_OK:

        break;
    case KEY_MODE:
        return true;
        break;
    default:
        return false;
    }

    return true;
}

/*****************************待机页面回调 ************************************/
static int window_main_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
    struct window *window = (struct window *)ctr;
    int err, item;
    static int id = 0;
    const char *str = NULL;
    int ret;
    u8 timeout;

    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n***[%s] init***\n",__func__);
        /*
         * 注册APP消息响应
         */
        ui_register_msg_handler(ID_WINDOW_MAIN_AND_REC, standby_msg_handler);
//        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, standby_ui_event_handler);
        break;
//        sys_cur_mod = 1;
//        __this->hlight_show_status = 0;
//        break;

    case ON_CHANGE_FIRST_SHOW:
        printf("/************* [%s] FIRST_SHOW\n",__func__);
        break;
    case ON_CHANGE_RELEASE:
        video_rec_in_state = REC_STA_IN_BREAK;
        printf("/****** [%s] %d\n",__func__,__LINE__);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_MAIN_AND_REC)
.onchange = window_main_mode_onchange,
 .onkey = main_and_rec_onkey,
  .ontouch = NULL,
};


static s8 standby_onkey_sel = 0;

#ifndef IPC_DEVELOP_BOARD_DEMO
static int standby_onkey_sel_item[3] = {
    STANDBY_BTN_CALL,       //拨号
    STANDBY_BTN_MSG,           //留言
    STANDBY_BTN_DEC,       //查看视频（回放留言视频+录像视频）长按进设置
};
#endif

//static int standby_onkey_sel_item1[3] = {
//    STANDBY_TXT_SETTING,
//    STANDBY_TXT_DEC,
//    STANDBY_TXT_MESSAGE,
//};
static const char *table_standby_menu[] = {
    "rec:cal",
    "rec:msg",
    "rec:dec",
    "\0"
};




static int standby_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
   // printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);

    int err, item;
    static int id = 0;
    const char *str = NULL;
    int ret;
    u8 timeout;

    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n***[%s] init***\n",__func__);

        break;


    case ON_CHANGE_FIRST_SHOW:
        printf("/************* [%s] FIRST_SHOW\n",__func__);
      //  printf("/************* [%s] FIRST_SHOW\n",__func__);
      //  printf("/************* [%s] FIRST_SHOW\n",__func__);

        video_last_call_state = video_call_state;
        video_call_state = CALL_STA_IDLE;               //被拒接后，为空闲状态
        video_rec_in_state = REC_STA_IN_IDLE;           //在主界面，模式状态为空闲状态
        update_longtime_idle_return_timer(2);//0开启 1更新 2关闭

   //   update_unread_msg_show();

        covUnixTimeStp2Beijing(get_utc_ms() / 1000,&standby_time);
        ui_pic_show_image_by_id(PIC_MON1, (u8)standby_time.tm_mon/10);
        ui_pic_show_image_by_id(PIC_MON2, (u8)standby_time.tm_mon%10);
        ui_pic_show_image_by_id(PIC_DAY1, (u8)standby_time.tm_mday/10);
        ui_pic_show_image_by_id(PIC_DAY2, (u8)standby_time.tm_mday%10);
        ui_pic_show_image_by_id(PIC_HOUR1, (u8)standby_time.tm_hour/10);
        ui_pic_show_image_by_id(PIC_HOUR2, (u8)standby_time.tm_hour%10);
        ui_pic_show_image_by_id(PIC_MIN1, (u8)standby_time.tm_min/10);
        ui_pic_show_image_by_id(PIC_MIN2, (u8)standby_time.tm_min%10);
     // ui_pic_show_image_by_id(PIC_TEM1, weather_p.info.temp/10);
     // ui_pic_show_image_by_id(PIC_TEM2, weather_p.info.temp%10);
        ui_text_show_index_by_id(TXT_WEEKDAY, (u8)standby_time.tm_wday-1);
        #ifndef LONG_POWER_IPC //rtl8189 获取信号强度
        ui_pic_show_image_by_id(PIC_WIFI_SIGNAL_STRENGTH,system_get_network_info_qos_for_ui()-3);  //WIFI信号强度图标
        #else
        ui_pic_show_image_by_id(PIC_WIFI_SIGNAL_STRENGTH,system_get_network_info_qos_for_ui());  //WIFI信号强度图标

        #endif
      //printf("\n /***** weather condition = %d\n",weather_p.info.condition);
    //  printf("\n /***** weather num = %d\n",app_weather_base[weather_p.info.condition]);

      //  put_buf(weather_p.city_name,sizeof(weather_p.city_name));
       // ui_text_set_textu_by_id(TXT_STANDBY_CITY,weather_p.city_name,calculate_utf8_length(weather_p.city_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
       // ui_text_show_index_by_id(STANDBY_TXT_WEATHER,weather_p.info.condition-1);
      //  ui_pic_show_image_by_id(STANDBY_PIC_WEATHER, (int)app_weather_base[weather_p.info.condition]);

        return false;
        break;
    case ON_CHANGE_RELEASE:
        printf("/****** [%s] %d\n",__func__,__LINE__);

        break;
    case ON_CHANGE_HIDE:
        printf("/****** [%s] %d hide\n",__func__,__LINE__);
        break;

    case ON_CHANGE_SHOW_COMPLETED:

         printf("/****** [%s] %d hide\n",__func__,__LINE__);
        break ;


    default:
        return false;
    }
    return false;
}

static int standby_mode_onkey(void *ctr, struct element_key_event *e)
{
    struct window *win = (struct window *)ctr;
    static char flag = 0;
    int err;
    printf("/****** [%s] %d \n",__func__,__LINE__);

//    sys_key_event_takeover(true, true); //拨号按键不属于UI控件动作,UI无需接管

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            printf("[%s] takeover\n",__func__);
            break;
        case KEY_MODE:
            printf("[%s] takeover\n",__func__);

            break;
        case KEY_UP:
            printf("[%s] KEY_UP\n",__func__);
#if 0
            /*
             * 留言模式
             */
            ui_hide(LAYER_STANDBY);
            ui_show(LAYER_MSG_MODE);

            init_intent(&it);
            it.name	= "video_rec";
            it.action = ACTION_VIDEO_REC_SET_CONFIG;
//            it.data = table_standby_menu[1];
            it.data = "msg_start";
            err = start_app_async(&it, NULL, NULL);
            if (err) {
                printf("msg change err! %d\n", err);
            }
#endif
            break;

//            ui_hide(LAYER_STANDBY);
//            ui_show(LAYER_MSG_SEL);
//            break;

//            if (standby_onkey_sel) {
//                ui_no_highlight_element_by_id(standby_onkey_sel_item[standby_onkey_sel - 1]);
//
//            }
//            standby_onkey_sel --;
//            if (standby_onkey_sel < 1) {
//                standby_onkey_sel = 3;
//            }
//            ui_highlight_element_by_id(standby_onkey_sel_item[standby_onkey_sel - 1]);
//
//            break;
        case KEY_DOWN:
            printf("[%s] KEY_DOWN\n",__func__);
            break;
//            if (standby_onkey_sel) {
//                ui_no_highlight_element_by_id(standby_onkey_sel_item[standby_onkey_sel - 1]);
//
//            }
//            standby_onkey_sel ++;
//            if (standby_onkey_sel > 3) {
//                standby_onkey_sel = 1;
//            }
//            ui_highlight_element_by_id(standby_onkey_sel_item[standby_onkey_sel - 1]);
//
//            break;

            case KEY_OK:
#if 0
                printf("/****** [%s] %d ,sel= %d\n",__func__,__LINE__,standby_onkey_sel);
                /*
                 * 查看视频模式
                 */
                init_intent(&it);
                it.name	= "video_rec";
                it.action = ACTION_VIDEO_REC_SET_CONFIG;
                it.data = table_standby_menu[2];
                err = start_app_async(&it, NULL, NULL);
                if (err) {
                        printf("res STANDBY change err! %d\n", err);
                }
#endif // 0
                break;
//                if (standby_onkey_sel) {
//                    struct intent it;
//                    switch (standby_onkey_sel) {
//                    case 1:
////                        /*
////                         * 拨号模式
////                         */
////                        ui_hide(LAYER_STANDBY);
////                        ui_show(LAYER_CALL_SEL);
//
//                        break;
//                    case 2:
//                        /*
//                         * 留言模式
//                         */
//                        ui_hide(LAYER_STANDBY);
//                        ui_show(LAYER_MSG_SEL);
//
//                        break;
//                    case 3:
//                        /*
//                         * 查看视频模式
//                         */
//                        init_intent(&it);
//                        it.name	= "video_rec";
//                        it.action = ACTION_VIDEO_REC_SET_CONFIG;
//                        it.data = table_standby_menu[2];
//                        err = start_app_async(&it, NULL, NULL);
//                        if (err) {
//                                printf("res STANDBY change err! %d\n", err);
//                        }
//                        break;
//                    }
//                    return true;
//                }
//                break;
        default:
            printf("/****** [%s] %d",__func__,__LINE__);
            return true;
        }
        break ;
#ifdef LONG_POWER_IPC
     case   KEY_EVENT_LONG:

        printf("\n aaaaaaaaaaaaaaa \n");
          sys_key_event_takeover(false, false);

          return false;
#endif
    default:
//        if (__this->menu_status) {
//            if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
//                rec_exit_menu_post();
//                sys_key_event_takeover(false, true);
//            }
//        }
        return true;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(LAYER_STANDBY)
  .onchange = standby_mode_onchange,
 //.onkey = standby_mode_onkey,
  .onkey = NULL,
  .ontouch = NULL,
};

#ifndef IPC_DEVELOP_BOARD_DEMO
const static u8 utf8_contact_name[] = {
    0xE6,0xB7,0xB1,0xE5,0x9C,0xB3
};
const static u8 unicode_contact_name[] = {
    0x68, 0x67, 0x42, 0x5E, 0x2D, 0x00, 0x31, 0x72, 0x84,
    0x76, 0x9B, 0x4F, 0x7B, 0x51, 0x2E, 0x00, 0x6D, 0x00,
    0x70, 0x00, 0x33, 0x00, 0x00, 0x00
};



static int text_txt_test_onchange(void *ctr, enum element_change_event e, void *arg)
{
//    printf("/******* [%s] e =%d\n",__func__,e);
    struct draw_context *dc = (struct draw_context *)arg;
    struct ui_text *text = (struct ui_text *)ctr;
    const char *ascii = "0123abcd\n";
    /*注:此处字符串 ascii 不能直接输入中文,中文的编码由本文件的编码格式决定,如需
    显示中文,请使用数组的形式.
    */
    switch (e) {
    case ON_CHANGE_INIT:
        printf("[%s] ON_CHANGE_INIT\n",__func__);

        /*此处添加控件默认显示文字,若无显示默认文字的需求可不加*/
//        ui_text_set_text_attrs(text,ascii,strlen(ascii),FONT_ENCODE_UTF8,0,FONT_DEFAULT);
//        ui_text_set_textu_by_id(TXT_STANDBY_CITY,utf8_contact_name,sizeof(utf8_contact_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);

        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("[%s] ON_CHANGE_FIRST_SHOW",__func__);
        if(calculate_utf8_length(weather_p.city_name) == 0 ){
            void get_weather(iot_weatherInfo_t *p);
            wait_completion(get_avsdk_connect_flag,get_weather,&weather_p);
            printf("\n /**** get_avsdk_connect_flag = %d\n",get_avsdk_connect_flag());
//            get_weather(&weather_p);
        }
        else{
            printf("\n /** city_name = %d\n",calculate_utf8_length(weather_p.city_name));
        }
            put_buf(weather_p.city_name,sizeof(weather_p.city_name));
            ui_text_set_textu_by_id(TXT_STANDBY_CITY,weather_p.city_name,calculate_utf8_length(weather_p.city_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TXT_STANDBY_CITY)
.onchange = text_txt_test_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif


#ifndef IPC_DEVELOP_BOARD_DEMO

static int layout_weather_onchange(void *ctr, enum element_change_event e, void *arg)
{
//    printf("/******* [%s] e =%d\n",__func__,e);

    switch (e) {
    case ON_CHANGE_INIT:
        printf("[%s] ON_CHANGE_INIT\n",__func__);

        break;

    case ON_CHANGE_FIRST_SHOW:
        printf("[%s] ON_CHANGE_FIRST_SHOW",__func__);

        if(calculate_utf8_length(weather_p.city_name) == 0 ){
            void get_weather(iot_weatherInfo_t *p);
            wait_completion(get_avsdk_connect_flag,get_weather,&weather_p);
//            get_weather(&weather_p);
        }
        ui_pic_show_image_by_id(PIC_TEM1, weather_p.info.temp/10);
        ui_pic_show_image_by_id(PIC_TEM2, weather_p.info.temp%10);

        printf("\n /***** weather condition = %d\n",weather_p.info.condition);
        printf("\n /***** weather num = %d\n",app_weather_base[weather_p.info.condition-1]);

        ui_text_show_index_by_id(STANDBY_TXT_WEATHER,weather_p.info.condition-1);
        ui_pic_show_image_by_id(STANDBY_PIC_WEATHER, (int)app_weather_base[weather_p.info.condition-1]);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_WEATHER)
.onchange = layout_weather_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif
///************************拨号主界面**********************/
//static int call_mode_onchange(void *ctr, enum element_change_event e, void *arg)
//{
//    struct layout *layout = (struct layout *)ctr;
//    int err, item;
//    static int id = 0;
//    const char *str = NULL;
//    struct intent it;
//    int ret;
//    u8 timeout;
//
//    switch (e) {
//    case ON_CHANGE_INIT:
//        printf("\n***[%s] init***\n",__func__);
//        layout_on_focus(layout);
//        /*
//         * 注册APP消息响应
//         */
////        ui_register_msg_handler(LAYER_CALL_SEL, call_or_msg_handler);
////        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, call_or_msg_ui_event_handler);
//        break;
////        sys_cur_mod = 1;
////        __this->hlight_show_status = 0;
//        break;
//    case ON_CHANGE_RELEASE:
//        printf("/****** [%s] %d",__func__,__LINE__);

//        break;
//    default:
//        return false;
//    }
//    return false;
//}
//
//static int call_mode_onkey(void *ctr, struct element_key_event *e)
//{
//    struct layout *layout = (struct layout *)ctr;
//    static char flag = 0;
//    int err;
//    struct intent it;
//    printf("/****** [%s] %d",__func__,__LINE__);
//    sys_key_event_takeover(false, true);
//
//    switch (e->event) {
//    case KEY_EVENT_CLICK:
//        switch (e->value) {
//        case KEY_MENU:
//            printf("[%s] takeover\n",__func__);
//            return true;
//            break;
//        case KEY_UP:
//            printf("[%s] takeover\n",__func__);
//            return true;
//            break;
//        case KEY_DOWN:
//            printf("[%s] takeover\n",__func__);
//            return true;
//            break;
//        case KEY_OK:
//            printf("/****** [%s] %d",__func__,__LINE__);
//            /*
//             * 拨号模式
//             */
//            ui_hide(LAYER_CALL_SEL);
//
//            init_intent(&it);
//            it.name	= "video_rec";
//            it.action = ACTION_VIDEO_REC_SET_CONFIG;
//            it.data = table_standby_menu[0];
//            err = start_app_async(&it, NULL, NULL);
//            if (err) {
//                printf("call change err! %d\n", err);
//            }
//            break;
//        default:
//            printf("/****** [%s] %d",__func__,__LINE__);
//            return true;
//        }
//        break;
//    default:
//        if (__this->menu_status) {
//            if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
//                rec_exit_menu_post();
//                sys_key_event_takeover(false, true);
//            }
//        }
//        return true;
//    }
//    printf("/****** [%s] %d",__func__,__LINE__);
//    return true;
//}

//REGISTER_UI_EVENT_HANDLER(LAYOUT_CALL_SEL)
//.onchange = call_mode_onchange,
// .onkey = call_mode_onkey,
//  .ontouch = NULL,
//};


/******************** 联系人名文字控件*******************/
#if 0
const static u8 utf8_contact1_name[] = {
    0xE6,0xB7,0xB1,0xE5,0x9C,0xB3       //深圳
};
//const static u8 unicode_contact_name[] = {
//    0x68, 0x67, 0x42, 0x5E, 0x2D, 0x00, 0x31, 0x72, 0x84,
//    0x76, 0x9B, 0x4F, 0x7B, 0x51, 0x2E, 0x00, 0x6D, 0x00,
//    0x70, 0x00, 0x33, 0x00, 0x00, 0x00
//};

static int text_txt_contact1_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("/******* [%s] e =%d\n",__func__,e);
    struct draw_context *dc = (struct draw_context *)arg;
    struct ui_text *text = (struct ui_text *)ctr;
    const char *ascii = "0123abcd\n";
    /*注:此处字符串 ascii 不能直接输入中文,中文的编码由本文件的编码格式决定,如需
    显示中文,请使用数组的形式.
    */
    switch (e) {
    case ON_CHANGE_INIT:
        printf("[%s] ON_CHANGE_INIT\n",__func__);

        /*此处添加控件默认显示文字,若无显示默认文字的需求可不加*/
//        ui_text_set_text_attrs(text,ascii,strlen(ascii),FONT_ENCODE_UTF8,0,FONT_DEFAULT);
//        ui_text_set_textu_by_id(LAYOUT_CONTACT1,utf8_contact_name,sizeof(utf8_contact_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);

        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("[%s] ON_CHANGE_FIRST_SHOW",__func__);
        ui_text_set_textu_by_id(LAYOUT_CONTACT1,utf8_contact_name,sizeof(utf8_contact_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(LAYOUT_CONTACT1)
.onchange = text_txt_contact1_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

const static u8 utf8_contact2_name[] = {
    0xE6,0xB7,0xB1,0xE5,0x9C,0xB3       //深圳
};
static int text_txt_contact2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("/******* [%s] e =%d\n",__func__,e);
    struct draw_context *dc = (struct draw_context *)arg;
    struct ui_text *text = (struct ui_text *)ctr;
    const char *ascii = "0123abcd\n";
    /*注:此处字符串 ascii 不能直接输入中文,中文的编码由本文件的编码格式决定,如需
    显示中文,请使用数组的形式.
    */
    switch (e) {
    case ON_CHANGE_INIT:
        printf("[%s] ON_CHANGE_INIT\n",__func__);

        /*此处添加控件默认显示文字,若无显示默认文字的需求可不加*/
//        ui_text_set_text_attrs(text,ascii,strlen(ascii),FONT_ENCODE_UTF8,0,FONT_DEFAULT);
//        ui_text_set_textu_by_id(TXT_CONTACT2,utf8_contact_name,sizeof(utf8_contact_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);

        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("[%s] ON_CHANGE_FIRST_SHOW",__func__);
        ui_text_set_textu_by_id(TXT_CONTACT2,utf8_contact_name,sizeof(utf8_contact_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TXT_CONTACT2)
.onchange = text_txt_contact2_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


const static u8 utf8_contact3_name[] = {
    0xE6,0xB7,0xB1,0xE5,0x9C,0xB3       //深圳
};
static int text_txt_contact3_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("/******* [%s] e =%d\n",__func__,e);
    struct draw_context *dc = (struct draw_context *)arg;
    struct ui_text *text = (struct ui_text *)ctr;
    const char *ascii = "0123abcd\n";
    /*注:此处字符串 ascii 不能直接输入中文,中文的编码由本文件的编码格式决定,如需
    显示中文,请使用数组的形式.
    */
    switch (e) {
    case ON_CHANGE_INIT:
        printf("[%s] ON_CHANGE_INIT\n",__func__);

        /*此处添加控件默认显示文字,若无显示默认文字的需求可不加*/
//        ui_text_set_text_attrs(text,ascii,strlen(ascii),FONT_ENCODE_UTF8,0,FONT_DEFAULT);
//        ui_text_set_textu_by_id(TXT_CONTACT3,utf8_contact_name,sizeof(utf8_contact_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);

        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("[%s] ON_CHANGE_FIRST_SHOW",__func__);
//        ui_text_set_textu_by_id(TXT_CONTACT3,utf8_contact_name,sizeof(utf8_contact_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TXT_CONTACT3)
.onchange = text_txt_contact3_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif

const static u8 utf8_contact3_name[] = {
    0xE6,0xB7,0xB1,0xE5,0x9C,0xB3       //深圳
};
static int layout_contact_sel_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u8 utf8_name_len = 0;
    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
    switch (e) {
	case ON_CHANGE_FIRST_SHOW:
#if 1   //获取联系人
      //  int32_t user_type = get_user_type();
      //  user_list_t *p = get_user_list_t_info();
        int32_t user_type = get_user_type();
        user_list_t *p = get_user_list_t_info();

        printf("\n\n p.users_count = %d\n\n",p->users_count );
        for(int i = 0;i< p->users_count;i++){
            printf("\n\n p.users[%d].name = %s\n\n",i,p->users[i].name );
        }
        if(p->users_count > 2){
            for(int i = 0;i< 3;i++){
                utf8_name_len = calculate_utf8_length(p->users[i].name);
                printf("\n/***** utf8_length = %d\n",utf8_name_len);

                ui_text_set_textu_by_id(LIST_CONTACT_TXT_TABLE[i],p->users[i].name,utf8_name_len, FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
            }
        }
        else{
            for(int i = 0;i< p->users_count;i++){
                utf8_name_len = calculate_utf8_length(p->users[i].name);
//                printf("\n /**********set contact show now\n ");
                printf("\n/***** utf8_length = %d\n",utf8_name_len);
                ui_text_set_textu_by_id(LIST_CONTACT_TXT_TABLE[i],p->users[i].name,utf8_name_len, FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
//                ui_text_set_str_by_id(LIST_CONTACT_TXT_TABLE[i], "ascii", P->users[i].name);
            }
//            for(int i = 0;i< p->users_count;i++){
//                printf("\n /**********set contact show now\n ");
//                 printf("\n/***** sizeof(name) = %d\n",sizeof(utf8_test_name));
//                ui_text_set_textu_by_id(LIST_CONTACT_TXT_TABLE[i],utf8_test_name,calculate_utf8_length(utf8_test_name), FONT_DEFAULT);
//            }
            for(int i = p->users_count;i<3;i++){
                printf("\n /**********set contact show NULL\n ");
                ui_text_set_textu_by_id(LIST_CONTACT_TXT_TABLE[i],"NULL",sizeof(NULL), FONT_DEFAULT);
//                ui_text_set_textu_by_id(LIST_CONTACT_TXT_TABLE[i],utf8_test_name,sizeof(utf8_test_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
//                ui_text_set_textu_by_id(LIST_CONTACT_TXT_TABLE[i],utf8_contact3_name,sizeof(utf8_contact3_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
            }
        }
#endif
	    break;
    default:
        return false;
    }
    return false;
}

static int layout_contact_sel_onkey(void *ctr, struct element_key_event *e)
{
    struct layout *layout = (struct layout *)ctr;
    static char flag = 0;
    int err;
    printf("/****** [%s] %d\n",__func__,__LINE__);

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_MODE:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_UP:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_DOWN:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_OK:
            printf("/****** [%s] %d\n",__func__,__LINE__);
            /*
             * 拨号中按OK 通知APP层停止拨号并返回待机界面
             */

            /*********通知门铃任务主动挂断逻辑********/
            if(video_call_state == CALL_STA_STARTING || video_call_state == CALL_STA_TALKING){
                post_msg_doorbell_task("video_dec_task", 1, DEVICE_HANGUP_STATE);
                printf("/****** [%s] %d DEVICE_HANGUP_STATE\n",__func__,__LINE__);
                return true;
            }else{
                return false;
            }
            break;

        default:
            printf("/****** [%s] %d\n",__func__,__LINE__);
            return true;
        }
        break;
    default:
//        if (__this->menu_status) {
//            if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
//                rec_exit_menu_post();
//                sys_key_event_takeover(false, true);
//            }
//        }
        return true;
    }
    printf("/****** [%s] %d\n",__func__,__LINE__);
    return true;
}


REGISTER_UI_EVENT_HANDLER(LAYOUT_CONTACT_SEL)
.onchange = layout_contact_sel_onchange,
 .onkey = layout_contact_sel_onkey,
  .ontouch = NULL,
};


static int layout_calling_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
	case ON_CHANGE_FIRST_SHOW:
        sys_key_event_takeover(true, true);
        layout_on_focus(layout);
        printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);
	    break;
    case ON_CHANGE_RELEASE:
        sys_key_event_takeover(false, false);
        layout_lose_focus(layout);
        printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
        break;
    default:
        return false;
    }
    return false;
}
void call_timer_handler(){


    printf("\n timer is arrived\n");
    ui_hide(LAYER_CALLING_EVENT);
     post_msg_doorbell_task("video_dec_task", 1, DEVICE_HANGUP_STATE);



}
static int layout_calling_rec_onkey(void *ctr, struct element_key_event *e)
{
    struct layout *layout = (struct layout *)ctr;
    static char flag = 0;
    int err;
    printf("/****** [%s] %d\n",__func__,__LINE__);

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_MODE:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_UP:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_DOWN:
         #if  0
            printf("[%s] takeover\n",__func__);
            return true;

            break;
        #endif
        case KEY_OK:
            if(video_call_state == CALL_STA_STARTING || \
                video_call_state == CALL_STA_TALKING || \
                video_call_state == CALL_STA_TALKING_WITHOUT_VIDEO){
               printf("/****** [%s] %d DEVICE_HANGUP_STATE\n",__func__,__LINE__);
                /*
                 * 拨号中按ENC按键 通知APP层停止拨号并返回待机界面
                 */
            #if  0
//                ui_show(LAYER_CALLING_EVENT);
//                os_time_dly(1);
//                ui_show(LAYOUT_CALL_TIME);
//                os_time_dly(1);
//                ui_show(CALL_TIME_CTROL);
//                sys_timeout_add(NULL,call_timer_handler,2000);


                  //  rec_call_dispose_mode = 1;
                  extern void rec_dispose_timer_handler(u8 mode);
                  video_standby_post_msg("app_close_call");
                  //rec_call_dispose_timer = sys_timeout_add(1,rec_dispose_timer_handler,2000);
                  sys_timeout_add(1,rec_dispose_timer_handler,2000);


            #else
                post_msg_doorbell_task("video_dec_task", 1, DEVICE_HANGUP_STATE);
            #endif
                return true;
            }else{
                printf("/****** [%s] %d\n",__func__,__LINE__);
                return false;
            }

            break;
        default:
            printf("/****** [%s] %d\n",__func__,__LINE__);
            return true;
        }
        break;
    default:
//        if (__this->menu_status) {
//            if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
//                rec_exit_menu_post();
//                sys_key_event_takeover(false, true);
//            }
//        }
        return true;
    }
    printf("/****** [%s] %d\n",__func__,__LINE__);
    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYOUT_CALLING_REC)
    .onchange = layout_calling_rec_onchange,
    .onkey = layout_calling_rec_onkey,
    .ontouch = NULL,
};

/*
static int lay_calling_timer_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
    switch (e) {
	case ON_CHANGE_INIT:
        //sys_key_event_takeover(true, true);

        printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);
	    break;
	case ON_CHANGE_FIRST_SHOW:


         ui_text_set_str_by_id(TXT_CALL_TIME, "ascii", "00:15:00");
        break;

    case ON_CHANGE_RELEASE:
       // sys_key_event_takeover(false, false);
        printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
        break;
    default:
        return false;
    }
    return false;
}
static int layout_calling_timer_onkey(void *ctr, struct element_key_event *e)
{
    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
    return false;
}

REGISTER_UI_EVENT_HANDLER(LAYOUT_CALL_TIME)
    .onchange = lay_calling_timer_onchange,
    .onkey = NULL,//layout_calling_timer_onkey,

    .ontouch = NULL,
};
*/
/***************************** 通话时间控件动作 ************************************/
extern u32 call_start_timer;
static int timer_call_timer_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct intent it;
    int err, i, j;
    u16 timebuf[3] = {0};

    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_FIRST_SHOW:
    case ON_CHANGE_SHOW_PROBE:
    case ON_CHANGE_SHOW_COMPLETED:


        u32 total_timer=(timer_get_ms()-call_start_timer);
        if(!call_start_timer)
        {
         total_timer=0;
        }

        printf("\n  call_timer :%d S\n",(timer_get_ms()-call_start_timer)/1000);



        time->hour = total_timer / 1000/3600;
        time->min = total_timer/1000 % 3600 / 60;
        time->sec = total_timer/1000 % 60;
        printf("reTIME hour:%02d, min:%02d, sec:%02d\n", time->hour, time->min, time->sec);

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(CALL_TIME_CTROL)
.onchange = timer_call_timer_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




static int layout_calling_close_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
    switch (e) {
	case ON_CHANGE_FIRST_SHOW:
	case ON_CHANGE_SHOW_COMPLETED:
      //  sys_key_event_takeover(true, true);
        printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);

        if(hund_upflag){

        ui_text_show_index_by_id(TXT_CALLING_CLOSE, 1);
        }else{

          ui_text_show_index_by_id(TXT_CALLING_CLOSE, 0);
        }
	    break;
    case ON_CHANGE_RELEASE:
       // sys_key_event_takeover(false, false);
        printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
        break;
    default:
        return false;
    }
    return false;
}
//static int layout_calling_close_onkey(void *ctr, struct element_key_event *e)
//{
//    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
//    return false;
//}

REGISTER_UI_EVENT_HANDLER(LAYOUT_CALLING_CLOSE)
    .onchange = layout_calling_close_onchange,
  // .onkey = layout_calling_close_onkey,
////    .onkey = NULL,
//    .ontouch = NULL,
};

//static int layout_calling_busy_onchange(void *ctr, enum element_change_event e, void *arg)
//{
//    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
//    switch (e) {
//	case ON_CHANGE_FIRST_SHOW:
//        sys_key_event_takeover(true, true);
//        printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);
//	    break;
//    case ON_CHANGE_RELEASE:
//        sys_key_event_takeover(false, false);
//        printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
//        break;
//    default:
//        return false;
//    }
//    return false;
//}
//static int layout_calling_busy_onkey(void *ctr, struct element_key_event *e)
//{
//    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
//    return true;
//}

//REGISTER_UI_EVENT_HANDLER(LAYOUT_CALLING_BUSY)
//    .onchange = layout_calling_busy_onchange,
//    .onkey = layout_calling_busy_onkey,
//    .ontouch = NULL,
//};

//static int layout_calling_timeout_onkey(void *ctr, struct element_key_event *e)
//{
//    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
//    return true;
//}
//
//static int layout_calling_timeout_onchange(void *ctr, enum element_change_event e, void *arg)
//{
//    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
//    switch (e) {
//	case ON_CHANGE_FIRST_SHOW:
//        sys_key_event_takeover(true, true);
//        printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);
//	    break;
//    case ON_CHANGE_RELEASE:
//        sys_key_event_takeover(false, false);
//        printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
//        break;
//    default:
//        return false;
//    }
//    return false;
//}

//REGISTER_UI_EVENT_HANDLER(LAYOUT_CALLING_TIMEOUT)
//    .onchange = layout_calling_timeout_onchange,
//    .onkey = layout_calling_timeout_onkey,
//    .ontouch = NULL,
//};

//static int layout_calling_net_connectfail_onkey(void *ctr, struct element_key_event *e)
//{
//    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
//    return true;
//}
//
//static int layout_calling_net_connectfail_onchange(void *ctr, enum element_change_event e, void *arg)
//{
//    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
//    switch (e) {
//	case ON_CHANGE_FIRST_SHOW:
//        sys_key_event_takeover(true, true);
//        printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);
//	    break;
//    case ON_CHANGE_RELEASE:
//        sys_key_event_takeover(false, false);
//        printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
//        break;
//    default:
//        return false;
//    }
//    return false;
//}

//REGISTER_UI_EVENT_HANDLER(LAYOUT_CALLING_NET_CONNECTFAIL)
//.onchange = layout_calling_net_connectfail_onchange,
// .onkey = layout_calling_net_connectfail_onkey,
//  .ontouch = NULL,
//};

//static int layout_msg_no_card_onchange(void *ctr, enum element_change_event e, void *arg)
//{
//    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
//    switch (e) {
//	case ON_CHANGE_FIRST_SHOW:
//        sys_key_event_takeover(true, true);
//        printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);
//	    break;
//    case ON_CHANGE_RELEASE:
//        sys_key_event_takeover(false, false);
//        printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
//        break;
//    default:
//        return false;
//    }
//    return false;
//}
//
//static int layout_msg_no_card_onkey(void *ctr, struct element_key_event *e)
//{
//    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
//    return true;
//}
//
//REGISTER_UI_EVENT_HANDLER(LAYOUT_MSG_NO_CARD)
//    .onchange = layout_msg_no_card_onchange,
//    .onkey = layout_msg_no_card_onkey,
//    .ontouch = NULL,
//};


//static int layout_msg_no_card_onchange(void *ctr, enum element_change_event e, void *arg)
//{
//    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
//    switch (e) {
//	case ON_CHANGE_FIRST_SHOW:
//        sys_key_event_takeover(true, true);
//        printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);
//	    break;
//    case ON_CHANGE_RELEASE:
//        sys_key_event_takeover(false, false);
//        printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
//        break;
//    default:
//        return false;
//    }
//    return false;
//}
//
//static int layout_msg_no_card_onkey(void *ctr, struct element_key_event *e)
//{
//    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
//    return true;
//}
//
//REGISTER_UI_EVENT_HANDLER(LAYOUT_MSG_NO_CARD)
//    .onchange = layout_msg_no_card_onchange,
//    .onkey = layout_msg_no_card_onkey,
//    .ontouch = NULL,
//};


/*************************事件提示框(挂断、未接、超时等)***************************/
static int layer_calling_event_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
    struct layout *layout = (struct layout *)ctr;
    int err, item;
    static int id = 0;
    const char *str = NULL;
    int ret;
    u8 timeout;

    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n***[%s] init***\n",__func__);
        break;

    case ON_CHANGE_FIRST_SHOW:
        sys_key_event_takeover(true, true);
        printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);
        break;

    case ON_CHANGE_HIDE:
        sys_key_event_takeover(false, false);
        printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
        break;
    default:
        return false;
    }
    return false;
}

static int layer_calling_event_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    static int sel_item = 0;
    printf("/****** [%s] %d\n",__func__,__LINE__);

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            printf("[%s] KEY_MENU\n",__func__);
            return true;
            break;

        case KEY_MODE:
            printf("[%s] KEY_MODE \n",__func__);
            return true;
            break;

        case KEY_UP:
            printf("[%s] KEY_UP\n",__func__);
            return false;
            break;

        case KEY_DOWN:
            printf("[%s] KEY_DOWN\n",__func__);
            return false;
            break;

        case KEY_OK:
            printf("[%s] KEY_OK\n",__func__);
            return true;
            break;

        default:
            printf("/****** [%s] %d\n",__func__,__LINE__);
            return true;
        }
        break;
    default:
        return true;
    }
    printf("/****** [%s] %d\n",__func__,__LINE__);
    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYER_CALLING_EVENT)
.onchange = layer_calling_event_onchange,
 .onkey = layer_calling_event_onkey,
  .ontouch = NULL,
};

/*********************************************************/

/************************拨号中界面**********************/
static int calling_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);

    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n /***** [%s] %d arg = %x\n",__func__,__LINE__,arg);
        break;

	case ON_CHANGE_FIRST_SHOW:
        printf("\n os_current_task() = %s\n",os_current_task());
        printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);
        sys_key_event_takeover(true, true);


        printf("\n >>>>>>>>>[%s] %d arg = %x\n",__func__,__LINE__,arg);

        if(video_call_state == CALL_STA_STARTING){    //拨号中状态
//            ui_show(LAYOUT_CALLING_REC);
            ui_show(LAYOUT_CALLING);
        }else if(video_call_state == CALL_STA_GET_CONTACT){    //选择联系人状态
            ui_show(LAYOUT_CONTACT_SEL);
        }
        video_last_call_state = video_call_state;
        break;
    case ON_CHANGE_HIDE:
        sys_key_event_takeover(false, false);
        printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
        break;
    case ON_CHANGE_RELEASE:
//        sys_key_event_takeover(false, false);

        printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);

        break;
    default:
        return false;
    }
    return false;
}

static int calling_mode_onkey(void *ctr, struct element_key_event *e)
{
    struct layout *layout = (struct layout *)ctr;
    static char flag = 0;
    int err;
    printf("/****** [%s] %d\n",__func__,__LINE__);

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_MODE:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_UP:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_DOWN:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_OK:
            printf("/****** [%s] %d\n",__func__,__LINE__);
            /*
             * 拨号中按OK 通知APP层停止拨号并返回待机界面
             */

//            /*********通知门铃任务主动挂断逻辑********/
//            if(video_call_state == CALL_STA_STARTING || video_call_state == CALL_STA_TALKING){
//                post_msg_doorbell_task("video_dec_task", 1, DEVICE_HANGUP_STATE);
//                printf("/****** [%s] %d\n",__func__,__LINE__);
//                return true;
//            }else{
//                return false;
//            }
            break;
        case KEY_ENC:
//            if(video_call_state == CALL_STA_STARTING || video_call_state == CALL_STA_TALKING){
//               printf("/****** [%s] %d\n",__func__,__LINE__);
//                /*
//                 * 拨号中按ENC按键 通知APP层停止拨号并返回待机界面
//                 */
//                post_msg_doorbell_task("video_dec_task", 1, DEVICE_HANGUP_STATE);
//                return true;
//            }else{
//                printf("/****** [%s] %d\n",__func__,__LINE__);
//                return false;
//            }

            break;
        default:
            printf("/****** [%s] %d\n",__func__,__LINE__);
            return true;
        }
        break;
    default:
//        if (__this->menu_status) {
//            if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
//                rec_exit_menu_post();
//                sys_key_event_takeover(false, true);
//            }
//        }
        return true;
    }
    printf("/****** [%s] %d\n",__func__,__LINE__);
    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYER_CALL_REC)
    .onchange = calling_mode_onchange,
    .onkey =  NULL,
    .ontouch = NULL,
};


/*************************选择联系人列表***************************/

static int sel_contact_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);
    struct layout *layout = (struct layout *)ctr;
    int err, item;
    static int id = 0;
    const char *str = NULL;
    int ret;
    u8 timeout;

    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n***[%s] init***\n",__func__);


        break;

    case ON_CHANGE_FIRST_SHOW:
//        sys_key_event_takeover(true, true);
        printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);

        break;

    case ON_CHANGE_RELEASE:
//        sys_key_event_takeover(false, false);

        printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
        break;
    default:
        return false;
    }
    return false;
}

static int sel_contact_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    static int sel_item = 0;
    printf("/****** [%s] %d\n",__func__,__LINE__);


    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_MODE:
            update_longtime_idle_return_timer(2);//0开启 1更新 2关闭
            printf("[%s] KEY_MODE DEVICE_HANGUP_STATE\n",__func__);
            post_msg_doorbell_task("video_dec_task", 1, DEVICE_HANGUP_STATE);
            return true;
            break;

        case KEY_UP:
            update_longtime_idle_return_timer(1);//0开启 1更新 2关闭
            printf("[%s] takeover\n",__func__);
            return false;
            break;
        case KEY_DOWN:
            update_longtime_idle_return_timer(1);//0开启 1更新 2关闭
            printf("[%s] takeover\n",__func__);
            return false;
            break;
        case KEY_OK:       //选择联系人时，按此按键挂断
             /*
             * 选择完联系人，拨号中
             */
            update_longtime_idle_return_timer(2);//0开启 1更新 2关闭
            sel_item = ui_grid_cur_item(grid);
            printf("/****** [%s] %d sel=%d\n",__func__,__LINE__,sel_item);

            sel_contact_flag = 1;

            //实际应该走这个流程设置联系人
            notify_msg_to_video_rec_async(ACTION_VIDEO_CALLING_SET_CONTACT,"sel_contact",sel_item);

//            init_intent(&it);
//            it.name	= "video_rec";
//            it.action = ACTION_VIDEO_CALLING_SET_CONTACT;
//            it.data = "sel_contact";
//            it.exdata = sel_item;
//            int err = start_app_async(&it,NULL,NULL);
//            if (err) {
//                printf("sel_contact onkey err! %d\n", err);
//                break;
//            }


//            ui_show(LAYOUT_CALLING);

            return true;
            break;
        default:
            printf("/****** [%s] %d\n",__func__,__LINE__);
            return true;
        }
        break;
    default:
        return true;
    }
    printf("/****** [%s] %d\n",__func__,__LINE__);
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_SEL_CONTACT)
.onchange = sel_contact_onchange,
 .onkey = sel_contact_onkey,
  .ontouch = NULL,
};


/************************留言主界面**********************/
static int layout_msg_sel_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    int err, item;
    const char *str = NULL;
    int ret;
    u8 timeout;

    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n***[%s] init***\n",__func__);
        break;
    case ON_CHANGE_RELEASE:

        printf("/****** [%s] %d",__func__,__LINE__);

        break;
    default:
        return false;
    }
    return false;
}

//static int layout_msg_sel_onkey(void *ctr, struct element_key_event *e)
//{
//    printf("\n /****** [%s] %d\n",__func__,__LINE__);
////    struct window *win = (struct window *)ctr;
//    struct layout *layout = (struct layout *)ctr;
//    static char flag = 0;
//    int err;
//    struct intent it;
//    printf("/****** [%s] %d",__func__,__LINE__);
//    //sys_key_event_takeover(false, true);
//
//    switch (e->event) {
//    case KEY_EVENT_CLICK:
//        switch (e->value) {
//        case KEY_MENU:
//            printf("[%s] takeover\n",__func__);
//            return true;
//            break;
//        case KEY_MODE:
//            printf("[%s] takeover\n",__func__);
//            return true;
//            break;
//        case KEY_UP:
//            printf("[%s] takeover\n",__func__);
//            return true;
//            break;
//        case KEY_DOWN:
//            printf("[%s] takeover\n",__func__);
//            return true;
//            break;
//        case KEY_OK:
//            printf("/****** [%s] %d",__func__,__LINE__);
//            /*
//             * 留言模式
//             */
//            ui_hide(LAYER_MSG_SEL);
//            ui_show(LAYER_MSG_MODE);
//
//            init_intent(&it);
//            it.name	= "video_rec";
//            it.action = ACTION_VIDEO_REC_SET_CONFIG;
//            it.data = table_standby_menu[1];
//            err = start_app_async(&it, NULL, NULL);
//            if (err) {
//                printf("msg change err! %d\n", err);
//            }
//
//            break;
//        default:
//            printf("/****** [%s] %d",__func__,__LINE__);
//            return true;
//        }
//        break;
//    default:
//        if (__this->menu_status) {
//            if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
//                rec_exit_menu_post();
//                printf("\n/***** [%s] %d takeover false\n",__func__,__LINE__);
//                sys_key_event_takeover(false, true);
//            }
//        }
//        return true;
//    }
//    printf("/****** [%s] %d",__func__,__LINE__);
//    return true;
//}

//REGISTER_UI_EVENT_HANDLER(LAYOUT_MSG_SEL)
//.onchange = layout_msg_sel_onchange,
// .onkey = layout_msg_sel_onkey,
//  .ontouch = NULL,
//};

/************************留言中界面**********************/
static int layout_msg_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    printf("\n/***[%s] e= %d\n",__func__,e);
    int err;
    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n***[%s] init***\n",__func__);
        break;

    case ON_CHANGE_FIRST_SHOW:
        printf("\n/***** [%s] %d ON_CHANGE_FIRST_SHOW takeover true\n",__func__,__LINE__);
        layout_on_focus(layout);
        video_rec_in_state = REC_STA_IN_MSG;   //模式状态位留言状态
        sys_key_event_takeover(true,true);
        break;

    case ON_CHANGE_HIDE:
        printf("\n/***** [%s] %d ON_CHANGE_HIDE takeover false\n",__func__,__LINE__);
        layout_lose_focus(layout);
        sys_key_event_takeover(false,false);
        break;

    case ON_CHANGE_RELEASE:
        printf("\n/****** [%s] %d ON_CHANGE_RELEASE\n",__func__,__LINE__);
        layout_lose_focus(layout);
        sys_key_event_takeover(false,false);
        break;

    default:
        return false;
    }
    return false;
}

static int layout_msg_mode_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d,e->event:%d,e->value:%d\n",__func__,__LINE__,e->event,e->value);
    int err;

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MODE:
            printf("[%s] takeover\n",__func__);
            if(video_msg_state == MSG_STA_IDLE){
                update_longtime_idle_return_timer(2);//0开启 1更新 2关闭
                notify_msg_to_video_rec_async(ACTION_VIDEO_REC_MSG_WORK,"msg_exit",0);
//                init_intent(&it);
//                it.name	= "video_rec";
//                it.action = ACTION_VIDEO_REC_MSG_WORK;
//                it.data = "msg_exit";
//                err = start_app_async(&it, NULL, NULL);
//                if (err) {
//                        printf("\n msg change err! %d\n", err);
//                    }
            }else{
                update_longtime_idle_return_timer(1);//0开启 1更新 2关闭
            }
            return true;
            break;
        case KEY_UP:
            printf("[%s] KEY_UP\n",__func__);
            if(video_msg_state == MSG_STA_FINISH){
                update_longtime_idle_return_timer(2);//0开启 1更新 2关闭
                notify_msg_to_video_rec_async(ACTION_VIDEO_REC_MSG_WORK,"msg_del",0);
//                init_intent(&it);
//                it.name	= "video_rec";
//                it.action = ACTION_VIDEO_REC_MSG_WORK;
//                it.data = "msg_del";
//                err = start_app_async(&it, NULL, NULL);
//                if (err) {
//                    printf("\n msg change err! %d\n", err);
//                }
            }else{
                update_longtime_idle_return_timer(1);//0开启 1更新 2关闭
                printf("\n/***** video_msg_state!=MSG_STA_FINISH\n");
            }
            return true;
            break;
        case KEY_DOWN:
            printf("[%s] KEY_DOWN\n",__func__);
            printf("\n video_msg_state = %d\n",video_msg_state);
            if(video_msg_state == MSG_STA_FINISH){
                update_longtime_idle_return_timer(2);//0开启 1更新 2关闭
                notify_msg_to_video_rec_async(ACTION_VIDEO_REC_MSG_WORK,"msg_save",0);
//                init_intent(&it);
//                it.name	= "video_rec";
//                it.action = ACTION_VIDEO_REC_MSG_WORK;
//                it.data = "msg_save";
//                err = start_app_async(&it, NULL, NULL);
//
//                if (err) {
//                    printf("\n msg change err! %d\n", err);
//                }
            }else{
                update_longtime_idle_return_timer(1);//0开启 1更新 2关闭
                printf("\n/***** video_msg_state!=MSG_STA_FINISH\n");
            }
            return true;
            break;
        case KEY_OK:
            printf("\n/****** [%s] %d KEY_OK\n",__func__,__LINE__);
            /*
             * 留言模式中按OK键开始留言
             */
            if (storage_device_ready() == 0) {
                update_longtime_idle_return_timer(1);//0开启 1更新 2关闭
            /*
             * 需要判断一下,SD不在线
             */
//                printf("/****** [%s] %d KEY_OK __dec_msg_show",__func__,__LINE__);
////                dec_sd_state = SD_STA_OFFLINE;
//
//                __rec_msg_show(BOX_MSG_INSERT_SD, 1500);


            }else if(video_msg_state == MSG_STA_IDLE){
                update_longtime_idle_return_timer(2);//0开启 1更新 2关闭
                ui_hide(PIC_MSG_START);
                ui_show(PIC_MSG_STARTING);
                ui_show(TXT_MSG_COUNT);

                notify_msg_to_video_rec_async(ACTION_VIDEO_REC_MSG_WORK,"msg_start",0);
//                init_intent(&it);
//                it.name	= "video_rec";
//                it.action = ACTION_VIDEO_REC_MSG_WORK;
//                it.data = "msg_start";
//                err = start_app_async(&it, NULL, NULL);
//                if (err) {
//                    printf("\n msg change err! %d\n", err);
//                }
                video_msg_state = MSG_STA_START;
            }else if(video_msg_state == MSG_STA_START) {
                update_longtime_idle_return_timer(2);//0开启 1更新 2关闭
                notify_msg_to_video_rec_async(ACTION_VIDEO_REC_MSG_WORK,"msg_stop",0);
//                init_intent(&it);
//                it.name	= "video_rec";
//                it.action = ACTION_VIDEO_REC_MSG_WORK;
//                it.data = "msg_stop";
//                err = start_app_async(&it, NULL, NULL);
//                if (err) {
//                    printf("\n msg change err! %d\n", err);
//                }
            }else if(video_msg_state == MSG_STA_FINISH){    //录完留言保存
                update_longtime_idle_return_timer(2);//0开启 1更新 2关闭
                notify_msg_to_video_rec_async(ACTION_VIDEO_REC_MSG_WORK,"msg_save",0);
//                init_intent(&it);
//                it.name	= "video_rec";
//                it.action = ACTION_VIDEO_REC_MSG_WORK;
//                it.data = "msg_save";
//                err = start_app_async(&it, NULL, NULL);
//                if (err) {
//                    printf("\n msg change err! %d\n", err);
//                }
            }else{
                printf("\n/***** video_msg_state= %d !=MSG_STA_IDLE\n",video_msg_state);
            }
            break;
        default:
            printf("\n/****** [%s] %d\n",__func__,__LINE__);
            return true;
        }
        break;
    default:
        return true;
    }
    printf("\n/****** [%s] %d\n",__func__,__LINE__);
    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYOUT_MSG_MODE)
.onchange = layout_msg_mode_onchange,
 .onkey = layout_msg_mode_onkey,
  .ontouch = NULL,
};


/*************** 留言倒计时 *************/
static int txt_msg_count_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *ui_text = (struct ui_text *)ctr;
    printf("\n/***[%s] e= %d\n",__func__,e);
    int err;
    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n***[%s] init***\n",__func__);
        break;

    case ON_CHANGE_FIRST_SHOW:
        printf("\n/***** [%s] %d ON_CHANGE_FIRST_SHOW takeover true\n",__func__,__LINE__);
        ui_text_show_index_by_id(TXT_MSG_COUNT,59-msging_countdown);
//        ui_text_set_textu_by_id(TXT_MSG_COUNT,nmsging_countdown,calculate_utf8_length(weather_p.city_name), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
        break;

    case ON_CHANGE_HIDE:
        printf("\n/***** [%s] %d ON_CHANGE_HIDE takeover false\n",__func__,__LINE__);
        break;

    case ON_CHANGE_RELEASE:
        printf("\n/****** [%s] %d ON_CHANGE_RELEASE\n",__func__,__LINE__);
        break;

    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TXT_MSG_COUNT)
.onchange = txt_msg_count_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************留言界面开始按键**********************/
static int pic_msg_start_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("\n/***[%s] e= %d\n",__func__,e);
    struct ui_pic *ui_pic = (struct ui_pic *)ctr;
    int err;

    switch (e) {
    case ON_CHANGE_INIT:
//        sys_key_event_takeover(true,true);
        printf("\n***[%s] init***\n",__func__);
        break;
    case ON_CHANGE_HIDE:
//        sys_key_event_takeover(false,false);
        printf("/****** [%s] %d",__func__,__LINE__);
        break;
    case ON_CHANGE_RELEASE:

        printf("/****** [%s] %d",__func__,__LINE__);

        break;
    default:
        return false;
    }
    return false;
}

static int pic_msg_start_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n /****** [%s] %d\n",__func__,__LINE__);
    static char flag = 0;
    int err;
    printf("/****** [%s] %d\n",__func__,__LINE__);

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MODE:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_UP:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_DOWN:
            printf("[%s] takeover\n",__func__);
            return true;
            break;
        case KEY_OK:
            printf("/****** [%s] %d",__func__,__LINE__);
            /*
             * 留言模式中按OK键开始留言
             */

            break;
        default:
            printf("/****** [%s] %d",__func__,__LINE__);
            return true;
        }
        break;
    default:
        return true;
    }
    printf("/****** [%s] %d",__func__,__LINE__);
    return true;
}
//REGISTER_UI_EVENT_HANDLER(PIC_MSG_START)
//.onchange = pic_msg_start_onchange,
//// .onkey = pic_msg_start_onkey,
// .onkey = NULL,
//  .ontouch = NULL,
//};

/************************配网界面**********************/
static int layer_dist_net_onchange(void *ctr, enum element_change_event e, void *arg)
{
    //printf("/****** [%s] %d e=%d\n",__func__,__LINE__,e);

    switch (e) {
	case ON_CHANGE_FIRST_SHOW:
        printf("\n os_current_task() = %s\n",os_current_task());
        printf("\n/***** [%s] %d takeover true\n",__func__,__LINE__);

     //   sys_key_event_enable();
        sys_key_event_takeover(true, true);
        break;
    case ON_CHANGE_TRY_OPEN_DC: //layer初始化
        printf("\n***[%s] init***\n",__func__);
        break;
    case ON_CHANGE_HIDE:
        sys_key_event_takeover(false, false);
        printf("\n /******* ON_CHANGE_HIDE takeover false");
        break;
    case ON_CHANGE_SHOW_COMPLETED:


        break;
    case ON_CHANGE_RELEASE:
        printf("/****** [%s] %d",__func__,__LINE__);

        break;
    default:
        return false;
    }
    return false;
}
typedef __uint8_t uint8 ;
extern   void step_motor_drive(uint8 dire,uint8 speed,int angle,u8 point_step);
static int layer_dist_net_onkey(void *ctr, struct element_key_event *e)
{
    printf("/****** [%s] %d,%d,%d\n",__func__,__LINE__,e->event,e->value);

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            printf("[%s] takeover\n",__func__);
            break;
        case KEY_MODE:
            printf("[%s] takeover\n",__func__);
            break;
        case KEY_UP:
            printf("[%s] takeover\n",__func__);
            break;
        case KEY_DOWN:

         // step_motor_drive(1,2,10,0);

        //  step_motor_stop();



            printf("[%s] takeover\n",__func__);
            break;
        case KEY_OK:

         // step_motor_drive(2,2,10,0);

         // step_motor_stop();


            printf("/****** [%s] %d\n",__func__,__LINE__);
            break;
        case KEY_ENC:
            printf("/****** [%s] %d\n",__func__,__LINE__);
            break;



        default:
            printf("/****** [%s] %d\n",__func__,__LINE__);
            break;
        }
        break;
    default:
        return true;


     #ifdef LONG_POWER_IPC



       case   KEY_EVENT_HOLD:
      //case   KEY_EVENT_LONG:

        static int count_net=0;
        switch (e->value) {

        case KEY_POWER:
        case KEY_OK:
        case  KEY_NET:


        printf("\n ENTER_RESET_CPU_MODE\n ");
         post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_ENTER_RESET_CPU);

            break;

        default:

            break;
        }
     #endif



    }




    printf("/****** [%s] %d\n",__func__,__LINE__);
    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYER_DIST_NET)
.onchange = layer_dist_net_onchange,
 .onkey = layer_dist_net_onkey,
  .ontouch = NULL,
};

void update_longtime_idle_return_timer(u8 state)
{
    printf("\n /*** [%s] %d\n",__func__,__LINE__);
    switch(state){
        case 0:     //初次开启定时器(用于初次进入需要返回的界面)
        if(longtime_idle_return_timer){
            printf("\n /*** [warning] already have timer\n");
            sys_timeout_del(longtime_idle_return_timer);
        }
        longtime_idle_return_timer = sys_timeout_add_to_task("sys_timer", NULL, longtime_idle_return_time_handler, CONFIG_UI_WAIT_TIMEOUT);
        break;

        case 1:     //开启后更新定时器(有相应操作后，更新定时器)
        if(longtime_idle_return_timer){
            printf("\n /*** [normal] update timer now\n");
            sys_timeout_del(longtime_idle_return_timer);
            longtime_idle_return_timer = sys_timeout_add_to_task("sys_timer", NULL, longtime_idle_return_time_handler, CONFIG_UI_WAIT_TIMEOUT);
        }else{
            printf("\n /*** [warning] no tiemr update fail\n");
        }
        break;

        case 2:     //关闭定时器(用于从需要返回的界面切换到不需要返回的界面)
        if(longtime_idle_return_timer){
            printf("\n /*** [normal] update timer now\n");
            sys_timeout_del(longtime_idle_return_timer);
            longtime_idle_return_timer = NULL;
        }else{
            printf("\n /*** [warning] no tiemr close fail\n");
        }
        break;
    }

}

void longtime_idle_return_time_handler()
{
    printf("\n /*** [%s] %d\n",__func__,__LINE__);
    if(longtime_idle_return_timer){
        printf("\n /*** [%s] finish, del timer\n",__func__);
        sys_timeout_del(longtime_idle_return_timer);
        longtime_idle_return_timer = NULL;
    }
    printf("\n /*** video_rec_in_state = %d, video_call_state = %d, video_msg_state = %d, dec_state = %d\n",\
           video_rec_in_state,video_call_state,video_msg_state,get_dec_state());
    switch(video_rec_in_state){
        case REC_STA_IN_CALL:
            switch(video_call_state){
            case CALL_STA_GET_CONTACT:
                //选择联系人界面，长时间不操作通知video_dec_task主动挂断
                post_msg_doorbell_task("video_dec_task", 1, DEVICE_HANGUP_STATE);
            break;
            }
        break;

        case REC_STA_IN_MSG:
            switch(video_msg_state){
            case MSG_STA_IDLE:
                notify_msg_to_video_rec_async(ACTION_VIDEO_REC_MSG_WORK,"msg_exit",0);
            break;
            case MSG_STA_FINISH:
                notify_msg_to_video_rec_async(ACTION_VIDEO_REC_MSG_WORK,"msg_save",0);
            break;
            }
        break;

        case REC_STA_IN_BREAK:
            if(get_dec_state()!=DEC_STA_PLAYBACK){
                printf("\n /** stay longtime return to standby\n ");
                struct intent it;
                struct application *app;
                init_intent(&it);
                app = get_current_app();

                if (app) {
                    if (!strcmp(app->name, "video_dec")) {
                        it.action = ACTION_BACK;
                        start_app(&it);

                        it.name = "video_rec";
                        it.action = ACTION_VIDEO_REC_MAIN;
                        start_app(&it);
                    }

                }
            }
//            switch(get_dec_state()){
//            case DEC_STA_IDLE:
////                notify_msg_to_video_rec_async(ACTION_VIDEO_REC_MSG_WORK,"msg_exit",0);
//            break;
//            case DEC_STA_SEL_FILE_FOLDER:
////                notify_msg_to_video_rec_async(ACTION_VIDEO_REC_MSG_WORK,"msg_save",0);
//            break;
//            case DEC_STA_SEL_VIDEO:
////                notify_msg_to_video_rec_async(ACTION_VIDEO_REC_MSG_WORK,"msg_save",0);
//            break;
//            }
        break;
    }
//    video_call_state = CALL_STA_IDLE;    //拨号工作状态
//    video_msg_state = MSG_STA_IDLE;      //留言工作状态
//    video_rec_in_state = REC_STA_IN_IDLE;       //模式状态
//    ui_show(LAYER_STANDBY);
}



int notify_msg_to_video_rec_async(int action,const char *data,u32 exdata)
{
    int ret = 0;
    struct intent it;
    init_intent(&it);
    struct application *app = NULL;
    app = get_current_app();
    if(app && !strcmp(app->name,"video_rec")){
        it.name	= "video_rec";
        it.action = action;
        it.data = data;
        it.exdata = exdata;
        start_app_async(&it, NULL, NULL);
    }else{
        ret = -1;
    }
    return ret;
}

#endif // CONFIG_IPC_UI_ENABLE


#endif
