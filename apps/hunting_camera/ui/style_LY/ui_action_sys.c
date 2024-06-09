#include "ui/includes.h"
#include "system/includes.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "storage_device.h"
#include "res.h"
#include "app_database.h"
#include "key_commucation.h"

#include "app_ui_index.h"

#ifdef CONFIG_UI_STYLE_LY_ENABLE


#define STYLE_NAME  LY

REGISTER_UI_STYLE(STYLE_NAME)



int sys_cur_mod = 0;  /* 1:rec, 2:tph, 3:dec, 4:audio, 5:music */
extern void rec_exit_menu_post(void);
extern void tph_exit_menu_post(void);
extern void rep_exit_menu_post(void);




/************************************************************
						系统设置
************************************************************/

/*
 * system光源频率设置
 */
static const u8 table_system_led_fre[] = {
    50,
    60,
};

/*
 * system自动关机设置
 */
static const u8 table_system_auto_close[] = {
    0,
    1,
    3,
    5,
};

/*
 * 录影时长设置
 */
static const u16 table_video_timer[] = {
    5,
    10,
    20,
    30,
    1 * 60,
    3 * 60,
    5 * 60,
    10 * 60,
};


/*
 * system屏幕保护设置
 */
static const u16 table_system_lcd_protect[] = {
    0,
    30,
    60,
    120,
};

/*
 * system语言设置
 */
static const u8 table_system_language[] = {
    Chinese_Simplified,  /* 简体中文 */
    Chinese_Traditional, /* 繁体中文 */
    Japanese,            /* 日文 */
    English,             /* 英文 */
    Korean,              /* 韩文 */
    French,              /* 法文 */
};

/*
 * system 声音开关设置
 */
static const u8 table_system_audio_mod[] = {
    0,
    1,
};

/*
 * 水印开关设置
 */
static const u8 table_osd_lable[] = {
    0,
    1,
};
/*
允许sensor 定时感应 工作时间段开关

*/
static const u8 table_tallow_lable[] = {
    0,
    1,
};
/*
定时起来工作时间开关，不管是否感应

*/
static const u8 table_everytime_lable[] = {
    0,
    1,
};


/************************************************************
					  工作模式设置
************************************************************/


static const u8 table_work_mode_camera0[] = {
    0, // 拍照
    1, //录像
    2, //拍照+录像

};
/*
 * 灵敏度设置
 */
static const u8 table_sensor_activity_camera0[] = {
    SEN_LO,
    SEN_MD,
    SEN_HI,
};
/*
 * 音量设置
 */
static const u8 table_audio_set_camera0[] = {
// 0-32
    AUDIO_HI,
    AUDIO_MD,
    AUDIO_LO,
    AUDIO_OFF,

};
/****************************  	录像模式设置  ********************************/
/*
 * rec分辨率设置
 */
static const u8 table_video_resolution[] = {

    VIDEO_RES_VGA,
    VIDEO_RES_720P,
    VIDEO_RES_1080P,
    VIDEO_RES_1296P,
    // VIDEO_RES_1440P,
};

/*
 * photo拍摄分辨率设置
 */
static const u8 table_photo_res_camera0[] = {
    PHOTO_RES_2M,
    PHOTO_RES_5M, 		/* 2592*1944 */
    PHOTO_RES_8M, 		/* 3264*2448 */
    PHOTO_RES_16M, 		/* 3648*2736 */
    PHOTO_RES_24M, 		/* 4032*3024 */
    PHOTO_RES_36M, 		/* 4032*3024 */
};


/*
 * (begin)提示框显示接口
 */
enum box_msg {
    BOX_MSG_INSERT_SD,
    BOX_MSG_DELETING_FILE,
    BOX_MSG_DEFAULTING,
    BOX_MSG_FORMATTING,
    BOX_MSG_DELETING_ALL_FILE,
    BOX_MSG_FORMAT_ERR,
    BOX_MSG_SD_ERR,
};


static u8 msg_show_f = 0;
static enum box_msg g_msg_id = 0;
static void __sys_msg_hide(void)
{
    if (msg_show_f) {
        msg_show_f = 0;
        ui_hide(LAYOUT_SYS_MSG);
    }
}
static void __sys_msg_timeout_func(void *priv)
{
    if (g_msg_id == BOX_MSG_DEFAULTING) {
        ui_hide(VLIST_MENU_SYS);
        ui_show(VLIST_MENU_SYS);
    }
    __sys_msg_hide();
}
static void __sys_msg_show(enum box_msg msg, u32 timeout_msec)
{

    if (msg_show_f == 0) {
        ui_show(LAYOUT_SYS_MSG);
        ui_text_show_index_by_id(TEXT_SYS_MSG, msg);
        g_msg_id = msg;
        msg_show_f = 1;
        if (timeout_msec > 0) {
            sys_timeout_add(NULL, __sys_msg_timeout_func, timeout_msec);
        }
    } else {
        ui_text_show_index_by_id(TEXT_SYS_MSG, msg);
        g_msg_id = msg;
        if (timeout_msec > 0) {
            sys_timeout_add(NULL, __sys_msg_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */

u8   get_table_photo_res()
{

    u8 res = index_of_table8(db_select("pres"), TABLE(table_photo_res_camera0));
    printf("\n res=========================%d,db_select(pres):%d\n ", res, db_select("pres"));
    return res ;
}



/*
 * (begin)系统设置各项操作请求
 */
static int sys_app_back(void)
{

    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_BACK;
    it.data = NULL;
    err = start_app(&it);
    if (err) {
        printf("sys app back err! %d\n", err);
        /* ASSERT(err == 0, "sys back app fail.\n"); */
        return -1;
    }
    return 0;
}


int menu_sys_lcd_pro_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "pro";
    it.exdata = table_system_lcd_protect[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":lcd_protect set fail!\n");
        return -1;
    }
    sys_app_back();
    return 0;
}
int menu_sys_auto_off_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "aff";
    it.exdata = table_system_auto_close[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":auto_off set fail!\n");
        return -1;
    }
    sys_app_back();
    return 0;
}


int menu_video_timer_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "vtimer";
    it.exdata = table_video_timer[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":video_timer set fail!\n");
        return -1;
    }
    sys_app_back();
    return 0;
}


int menu_sys_led_fre_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "fre";
    it.exdata = table_system_led_fre[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":led_fre set fail!\n");
        return -1;
    }
    sys_app_back();
    return 0;
}
int menu_sys_key_voice_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "kvo";
    it.exdata = sel_item;
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":key_voice set fail!\n");
        return -1;
    }
    sys_app_back();
    return 0;
}
int menu_sys_language_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "lag";
    it.exdata = table_system_language[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":language set fail!\n");
        return -1;
    }
    sys_app_back();
    return 0;
}

static int __time_get(struct sys_time *t);

int menu_sys_date_set(u8 sel_item)
{
    struct sys_time t;

    __time_get(&t);

    printf("menu_sys_date_set : %d-%d-%d,%d:%d:%d\n", t.year, t.month, t.day, t.hour, t.min, t.sec);


    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        return -EFAULT;
    }
    dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)&t);
    dev_close(fd);

    return 0;
}

#if 0
int menu_sys_tv_mod_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "tvm";
    it.exdata = table_system_tv_mod[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":tv_mod set fail!\n");
        return -1;
    }
    sys_app_back();
    return 0;
}

#endif
void format_callback(void *priv, int err)
{

    printf("\n format_aaaaaaaaaaaa\n");
    sys_app_back();
    printf("\n format_bbbbbbbbbbb\n");
    __sys_msg_hide();
    if (err) {
        /*
         * 格式化出错
         */
        printf("format_callback err!!\n");
        __sys_msg_show(BOX_MSG_FORMAT_ERR, 2000);
    }
}
int menu_sys_format_set(u8 sel_item)
{
    struct intent it;
    struct application *app;


    if (sel_item == 1) {
        if (fget_err_code(CONFIG_ROOT_PATH) == -EIO) {
            __sys_msg_show(BOX_MSG_SD_ERR, 2000);
        } else {
            init_intent(&it);

            app = get_current_app();

            if (!strcmp(app->name, "video_dec")) {
                it.name = app->name;
                it.action = ACTION_VIDEO_DEC_CUR_PAGE;
                start_app(&it);
            }


            it.name	= "video_system";
            it.action = ACTION_SYSTEM_SET_CONFIG;
            it.data = "frm";
            printf("\n start_app_async>>>>>>>>>>\n ");
            start_app_async(&it, format_callback, NULL);
            __sys_msg_show(BOX_MSG_FORMATTING, 0);
        }
    }
    return 0;
}

int menu_sys_version_set(u8 sel_item)
{

    sys_app_back();
    return 0;
}
/*
 * (end)
 */


/***************************** 菜单模式图标动作 ************************************/
static int pic_sys_mn_mod_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        if (sys_cur_mod) {
            ui_pic_set_image_index(pic, sys_cur_mod - 1);
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PIC_SYS_MN_MOD_LOGO)
.onchange = pic_sys_mn_mod_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




void sys_exit_menu_post(void)
{
    ui_hide(ID_WINDOW_VIDEO_SYS);

    switch (sys_cur_mod) {
    case 1:
        rec_exit_menu_post();
        break;
    case 2:
        tph_exit_menu_post();
        break;
    case 3:
        rep_exit_menu_post();
        break;
    default:
        break;
    }
    sys_key_event_takeover(false, false);
}

static int sys_page_onkey(void *ctr, struct element_key_event *e)
{

    switch (e->event) {
    case KEY_EVENT_LONG:
        if (e->value == KEY_POWER) {
            sys_exit_menu_post();
            sys_key_event_takeover(false, true);
        }
        break;
    }
    return false;
}

static void video_sys_ui_event_handler(struct sys_event *event, void *priv)
{
    if (!strcmp(event->arg, "parking")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            break;
        case DEVICE_EVENT_OUT:
        case DEVICE_EVENT_OFFLINE:
            /* if(__this->menu_status) { */
            sys_key_event_takeover(false, false);
            /* } */
            break;
        default:
            break;
        }
    }
}
static int sys_returnmenu_handler(const char *type, u32 arg)
{

    printf("\n  sys_returnmenu_handler>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    sys_exit_menu_post();

    return 0;
}
/*
 * 系统模式的APP状态响应回调
 */
static const struct uimsg_handl sys_msg_handler[] = {

    { "re_sys",     sys_returnmenu_handler   },
    { NULL, NULL},      /* 必须以此结尾！ */
};

/***************************** 系统菜单页面回调 ************************************/


static int sys_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    static int id = 0;


    printf("\n sys_page_onchange>>>>>>>>>>>>>>>>>>>\n ");
    switch (e) {
    case ON_CHANGE_INIT:

        ui_register_msg_handler(ID_WINDOW_VIDEO_SYS, sys_msg_handler);
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, video_sys_ui_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_SYS)
.onchange = sys_page_onchange,
 .onkey = sys_page_onkey,
  .ontouch = NULL,
};

/*********************************************************************************
 *								系统设置菜单
*********************************************************************************/

#if  1

static const int menu_sys_items[] = {
#if 1
    LAYOUT_MN_WORK_MODE,    //LAYOUT_MN_DELAY_TPH,  //工作模式  LAYOUT_MN_WORK_MODE
    LAYOUT_SET_RES_TPH,  // 照片分辨率
    LAYOUT_SET_REPEAT_TPH, // 连拍

    LAYOUT_SET_LABEL_TPH,
    LAYOUT_MN_SENSOR_ACTIVITY,//LAYOUT_MN_QUA_TPH,      // 灵敏度   LAYOUT_MN_SENSOR_TPH
    LAYOUT_MN_VIDEO_RES,//LAYOUT_MN_ACU_TPH,      // 录像分辨率




    LAYOUT_MN_SENSOR_GAP, //人体感应间隔  LAYOUT_MN_LCD_SYS,    打开后多少时间后才能触发


    LAYOUT_MN_AUTOF_SYS,    //  自动关机
#endif
    LAYOUT_ALLOWWORK_ONOFF,  //  定时器范围 是否允许设备工作
    //LAYOUT_MN_ALLOW_TIMER_WORK, // //  定时器设置  有感应才工作  LAYOUT_MN_HZ_SYS,  LAYOUT_MN_TIMER_SET
    // LAYOUT_MN_TIMER_WORK,  // 定时工作  ， 不管是否触发 ，定时起来工作 LAYOUT_MN_KEYV_SYS,
    LAYOUT_EVERYTIME_WORK_ONOFF,   //  定时起来工作，打开 无需PIR参与和设置
    LAYOUT_MN_LANG_SYS, //  语言设置
    LAYOUT_MN_TIME_SYS,//  时间日期设置
    LAYOUT_MN_AUDIO_ON_OFF,// 声音开关  LAYOUT_MN_TV_SYS,
    LAYOUT_MN_AUDIO_SET,// 音量大小

    LAYOUT_MN_VIDEO_TIMER,  //录影时长

    LAYOUT_MN_FORMAT_SYS,// 格式化
    LAYOUT_MN_DEFAULT_SYS,// 默认设置
    LAYOUT_MN_VERSION_SYS,// 版本号
};
#endif
static int menu_sys_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
    struct ui_grid *grid = (struct ui_grid *)_grid;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        ASSERT(sel_item < (sizeof(menu_sys_items) / sizeof(int)));
        if (menu_sys_items[sel_item] == LAYOUT_MN_FORMAT_SYS) {
            if (dev_online(SDX_DEV) == false && storage_device_ready() == 0) {
                printf("format sd offline !\n");
                __sys_msg_show(BOX_MSG_INSERT_SD, 2000);

                return TRUE;
            }
        }
        ui_show(menu_sys_items[sel_item]);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MENU:
        sys_exit_menu_post();
        break;
    case KEY_MODE:
        sys_exit_menu_post();

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_MENU_SYS)
.onkey = menu_sys_onkey,
 .ontouch = NULL,
};



/*****************************  工作模式选择 ************************************/
static int menu_work_mode__onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("mode"), TABLE(table_work_mode_camera0));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_work_mode_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_system";
        it.action = ACTION_SYSTEM_SET_CONFIG;
        it.data = "mode";
        it.exdata = table_work_mode_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_WORK_MODE);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_WORK_MODE);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_WORK_MODE)
.onchange = menu_work_mode__onchange,
 .onkey = menu_work_mode_onkey,
  .ontouch = NULL,
};


/***************************** 分辨率设置 ************************************/

static int menu_photo_res_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("pres"), TABLE(table_photo_res_camera0));


        printf("\n pres====================pres:%d,=%d,%d\n ", db_select("pres"), index, sizeof(table_photo_res_camera0) / sizeof(table_photo_res_camera0[0]));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_photo_res_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_system";
        it.action = ACTION_SYSTEM_SET_CONFIG;
        it.data = "pres";
        it.exdata = table_photo_res_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_SET_RES_TPH);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_SET_RES_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_SET_RES_TPH)
.onchange = menu_photo_res_onchange,
 .onkey = menu_photo_res_onkey,
  .ontouch = NULL,
};






/*
 * 连拍张数设置
 */
static const u8 table_photo_repeat_camera0[] = {
    1,
    2,
    3,
    4,
    5,

};

/***************************** 连拍设置 ************************************/

static int menu_repeat_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:

        index = index_of_table8(db_select("cyt"), TABLE(table_photo_repeat_camera0));

        ui_grid_set_item(grid, index);
        printf("\n cyt=====================%d\n ", db_select("cyt"));


        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_repeat_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_system";
        it.action = ACTION_SYSTEM_SET_CONFIG;
        it.data = "cyt";
        it.exdata = table_photo_repeat_camera0[sel_item];;
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_SET_REPEAT_TPH);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_SET_REPEAT_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_SET_REPEAT_TPH)
.onchange = menu_repeat_onchange,
 .onkey = menu_repeat_onkey,
  .ontouch = NULL,
};


/***************************** 系统声音设置 ************************************/

static int menu_lable_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("pdat"), TABLE(table_osd_lable));
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_lable_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item = 0;
    struct intent it;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        // menu_sys_tv_mod_set(sel_item);

        printf("\n audio_sel_item:%d\n", sel_item);
        init_intent(&it);
        it.name	= "video_system";
        it.action = ACTION_SYSTEM_SET_CONFIG;
        it.data = "pdat";
        it.exdata = sel_item;
        int err = start_app(&it);
        if (err) {
            printf("audio onkey err! %d\n", err);
            break;
        }


        ui_hide(LAYOUT_SET_LABEL_TPH);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_SET_LABEL_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_SET_LABEL_TPH)
.onchange = menu_lable_onchange,
 .onkey = menu_lable_onkey,
  .ontouch = NULL,
};


/***************************** 灵敏度设置 ************************************/
static int menu_sensor_activity_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("Senity"), TABLE(table_sensor_activity_camera0));



        printf("\n sensor_activity=================%d\n ", index);
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_sensor_activity_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_system";
        it.action = ACTION_SYSTEM_SET_CONFIG;
        it.data = "Senity";
        it.exdata = table_sensor_activity_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_SENSOR_ACTIVITY);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_SENSOR_ACTIVITY);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_SENSSOR_ACTIVITY)
.onchange = menu_sensor_activity_onchange,
 .onkey = menu_sensor_activity_onkey,
  .ontouch = NULL,
};



/***************************** 录像分辨率设置 ************************************/

static int menu_video_res_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("res"), TABLE(table_video_resolution));

        printf("\n ideo_res_index========================%d,%d,%d\n ", db_select("res"), index, (sizeof(table_video_resolution) / sizeof(table_video_resolution[0])));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_ideo_res_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        printf("\n  ideo_res_sel_item======================%d\n", sel_item);
        init_intent(&it);
        it.name	= "video_system";
        it.action = ACTION_SYSTEM_SET_CONFIG;
        it.data = "res";
        it.exdata = table_video_resolution[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_VIDEO_RES);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_VIDEO_RES);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_VIDEO_RES)
.onchange = menu_video_res_onchange,
 .onkey = menu_ideo_res_onkey,
  .ontouch = NULL,
};

#if 0
/*****************************  ************************************/

static int menu_lcd_pro_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table16(db_select("pro"), TABLE(table_system_lcd_protect));
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_lcd_pro_onkey(void *_grid, struct element_key_event *e)
{
    int sel_item = 0;
    struct ui_grid *grid = (struct ui_grid *)_grid;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        menu_sys_lcd_pro_set(sel_item);

        ui_hide(LAYOUT_MN_SENSOR_GAP);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_SENSOR_GAP);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_SENSOR_GAP_SYS)  // VLIST_SENSOR_GAP_SYS

.onchange = menu_lcd_pro_onchange,
 .onkey = menu_lcd_pro_onkey,
  .ontouch = NULL,
};

#endif



/***************************** 自动关机设置 ************************************/

static int menu_auto_off_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("aff"), TABLE(table_system_auto_close));
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_auto_off_onkey(void *ctr, struct element_key_event *e)
{
    int sel_item = 0;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        menu_sys_auto_off_set(sel_item);

        ui_hide(LAYOUT_MN_AUTOF_SYS);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_AUTOF_SYS);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_AUTOF_SYS)
.onchange = menu_auto_off_onchange,
 .onkey = menu_auto_off_onkey,
  .ontouch = NULL,
};



/***************************** 录影时长设置 ************************************/

static int menu_video_timer_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table16(db_select("vtimer"), TABLE(table_video_timer));

        printf("\n  video_timer_item====================%d\n", item);
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_video_timer_onkey(void *ctr, struct element_key_event *e)
{
    int sel_item = 0;
    struct ui_grid *grid = (struct ui_grid *)ctr;



    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        printf("\n menu_video_timer_onkey=================%d\n", sel_item);

        menu_video_timer_set(sel_item);

        ui_hide(LAYOUT_MN_VIDEO_TIMER);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_VIDEO_TIMER);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_VIDEO_TIMER)
.onchange = menu_video_timer_onchange,
 .onkey = menu_video_timer_onkey,
  .ontouch = NULL,
};





/***************************** 前照灯提醒设置 ************************************/
static int menu_headlight_onchange(void *ctr, enum element_change_event e, void *arg)
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



/***************************** 语言设置设置 ************************************/

static int menu_language_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("lag"), TABLE(table_system_language));
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_language_onkey(void *ctr, struct element_key_event *e)
{
    int sel_item = 0;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        menu_sys_language_set(sel_item);

        ui_hide(LAYOUT_MN_LANG_SYS);
        ui_show(VLIST_MENU_SYS);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_LANG_SYS);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_LANG_SYS)
.onchange = menu_language_onchange,
 .onkey = menu_language_onkey,
  .ontouch = NULL,
};





/***************************** 日期时间设置 ************************************/
struct time_setting {
    const char *mark;
    u32 time_id;
    u16 time_value;
};
struct time_setting time_set_table[] = {
    {"year",  TIMER_YEAR_SYS,   0},
    {"month", TIMER_MONTH_SYS,  0},
    {"day",   TIMER_DAY_SYS,    0},
    {"hour",  TIMER_HOUR_SYS,   0},
    {"min",   TIMER_MINUTE_SYS, 0},
    {"sec",   TIMER_SECOND_SYS, 0},
};
enum sw_dir { /* 切换方向 */
    DIR_NEXT = 1,
    DIR_PREV,
    DIR_SET,
};
enum set_mod { /* 加减时间方向 */
    MOD_ADD = 1,
    MOD_DEC,
    MOD_SET,
};
static void get_sys_time(struct sys_time *time)
{

#if  0
    // struct sys_time rsys_time;

    db_select_buffer(SYS_TIMER_INFO, time, sizeof(struct sys_time));


    /* printf("get_sys_time : %d-%d-%d,%d:%d:%d\n", time->year, time->month, time->day, time->hour, time->min, time->sec); */
#else

    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        memset(time, 0, sizeof(*time));
        return;
    }
    dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)time);
    printf("get_sys_time : %d-%d-%d,%d:%d:%d\n", time->year, time->month, time->day, time->hour, time->min, time->sec);
    dev_close(fd);

#endif


}
static u16 time_set_p = 0xff; /* time_set_table的当前设置指针 */
static u8 __time_search_by_mark(const char *mark)
{
    u16 p = 0;
    u16 table_sum = sizeof(time_set_table) / sizeof(struct time_setting);
    while (p < table_sum) {
        if (!strcmp(mark, time_set_table[p].mark)) {

            printf("\n  p==========%d\n", p);
            return p;
        }
        p++;
    }
    return -1;
}
static u8 __time_set_switch(enum sw_dir dir, const char *mark)
{
    u16 table_sum;
    u16 prev_set_p = time_set_p;
    u8 p;
    printf("__time_set_switch dir: %d\n", dir);
    table_sum = sizeof(time_set_table) / sizeof(struct time_setting);
    ASSERT(dir == DIR_NEXT || dir == DIR_PREV || dir == DIR_SET);
    switch (dir) {
    case DIR_NEXT:
        if (time_set_p >= (table_sum - 1)) {
            time_set_p = 0;
        } else {
            time_set_p++;
        }
        break;
    case DIR_PREV:
        if (time_set_p == 0) {
            time_set_p = (table_sum - 1);
        } else {
            time_set_p--;
        }
        break;
    case DIR_SET:
        p = __time_search_by_mark(mark);
        if (p == (u8) - 1) {
            return -1;
        }
        time_set_p = p;
        printf("\n time_set_p===============%d\n", time_set_p);
        break;
    }
    if (prev_set_p != 0xff) {
        printf("\n  ui_no_highlight_prev_set_p:%d,%d", prev_set_p, time_set_table[prev_set_p].time_id);
        ui_no_highlight_element_by_id(time_set_table[prev_set_p].time_id);
    }
    printf("\n  ui_highlight_prev_set_p:%d,%d", time_set_p, time_set_table[time_set_p].time_id);
    ui_highlight_element_by_id(time_set_table[time_set_p].time_id);
    printf("__time_set_switch ok.\n");
    return 0;



}
static u8 __time_set_reset(void)
{
    struct sys_time time;

    time_set_p = 0xff;
    /*
     * 此处应该读取系统RTC时间
     */
    get_sys_time(&time);

    time_set_table[0].time_value = time.year;
    time_set_table[1].time_value = time.month;
    time_set_table[2].time_value = time.day;
    time_set_table[3].time_value = time.hour;
    time_set_table[4].time_value = time.min;
    time_set_table[5].time_value = time.sec;



    printf("\n get_sys_time_time.year:%d, rsys_time.month:%d, rsys_time.day:%d, sys_time.hour:%d,rsys_time.min:%d,rsys_time.sec:%d,%d\n", \
           time_set_table[0].time_value, time_set_table[1].time_value, time_set_table[2].time_value, time_set_table[3].time_value, time_set_table[4].time_value, time_set_table[5].time_value, sizeof(struct sys_time));

    printf("\n zzzzzzzzzzzzzzzzzzzzz \n");

    return 0;
}

static u8 send_rtc_time_tomcu()
{
    struct sys_time time;


    /*
     * 此处应该读取系统RTC时间
     */
    get_sys_time(&time);

    time_set_table[0].time_value = time.year;
    time_set_table[1].time_value = time.month;
    time_set_table[2].time_value = time.day;
    time_set_table[3].time_value = time.hour;
    time_set_table[4].time_value = time.min;
    time_set_table[5].time_value = time.sec;



    // time_set_table[3].time_value,time_set_table[4].time_value,time_set_table[5].time_value);

    printf("\n get_sys_time_time.year:%d, rsys_time.month:%d, rsys_time.day:%d, sys_time.hour:%d,rsys_time.min:%d,rsys_time.sec:%d,%d\n", \
           time_set_table[0].time_value, time_set_table[1].time_value, time_set_table[2].time_value, time_set_table[3].time_value, time_set_table[4].time_value, time_set_table[5].time_value, sizeof(struct sys_time));

    printf("\n send rtc time>>>>>>>>>>>>>>>>>>>> \n");

    return 0;
}



u16 leap_month_table[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static u8 __is_leap_year(u16 year)
{
    return (((year % 4) == 0 && (year % 100) != 0) || ((year % 400) == 0));
}
static u8 __time_update_show(u8 pos) /* 更新单个时间控件的时间 */
{
    struct utime t;

    t.year  = time_set_table[0].time_value;
    t.month = time_set_table[1].time_value;
    t.day   = time_set_table[2].time_value;
    t.hour  = time_set_table[3].time_value;
    t.min   = time_set_table[4].time_value;
    t.sec   = time_set_table[5].time_value;
    printf("\n __time_update_show=====================%d,value:%d\n ", time_set_table[pos].time_value);
    ui_time_update_by_id(time_set_table[pos].time_id, &t);
    return 0;
}

//  保存
void   sys_timer_set_on(enum sw_dir dir, const char *mark)
{


    struct sys_time rsys_time;

    rsys_time.year  = time_set_table[0].time_value;
    rsys_time.month  = time_set_table[1].time_value;
    rsys_time.day   = time_set_table[2].time_value;

    rsys_time.hour  = time_set_table[3].time_value;
    rsys_time.min   = time_set_table[4].time_value;
    rsys_time.sec   = time_set_table[5].time_value;


    printf("\n uddate_rrsys_time.year:%d, rsys_time.month:%d, rsys_time.day:%d, sys_time.hour:%d,rsys_time.min:%d,rsys_time.sec:%d,%d\n", \
           rsys_time.year, rsys_time.month, rsys_time.day, rsys_time.hour, rsys_time.min, rsys_time.sec, sizeof(struct sys_time));

    db_update_buffer(SYS_TIMER_INFO, &rsys_time, sizeof(struct sys_time));




    db_select_buffer(SYS_TIMER_INFO, &rsys_time, sizeof(struct sys_time));

    printf("\n read_rrsys_time.year:%d, rsys_time.month:%d, rsys_time.day:%d, sys_time.hour:%d,rsys_time.min:%d,rsys_time.sec:%d,%d\n", \
           rsys_time.year, rsys_time.month, rsys_time.day, rsys_time.hour, rsys_time.min, rsys_time.sec, sizeof(struct sys_time));



}



static u8 __time_set_value(enum set_mod mod, u16 value)
{

    enum set_mod need_check = 0;
    u16 year, month, day;
    ASSERT(mod == MOD_ADD || mod == MOD_DEC || mod == MOD_SET);
    printf("\n mod===================%d,time_set_p:%d\n", mod, time_set_p);
    switch (mod) {
    case MOD_ADD:
        switch (time_set_p) {
        case 0: /* year */
            value = time_set_table[time_set_p].time_value + 1;
            if (value >= 2100) {
                value = 2099;
            }
            need_check  = MOD_ADD;
            break;
        case 1: /* month */
            value = time_set_table[time_set_p].time_value + 1;
            if (value > 12) {
                value = 1;
            }
            need_check  = MOD_ADD;
            break;
        case 2: /* day */
            month = time_set_table[1].time_value;
            ASSERT(month >= 1 && month <= 12);
            value = time_set_table[time_set_p].time_value + 1;
            if (value > leap_month_table[month - 1]) {
                value = 1;
            }
            need_check  = MOD_ADD;
            break;
        case 3: /* hour */
            value = time_set_table[time_set_p].time_value + 1;
            if (value > 23) {
                value = 0;
            }
            break;
        case 4: /* min */
            value = time_set_table[time_set_p].time_value + 1;
            if (value > 59) {
                value = 0;
            }
            break;
        case 5: /* sec */
            value = time_set_table[time_set_p].time_value + 1;
            if (value > 59) {
                value = 0;
            }
            break;
        default:
            ASSERT(0, "mod_add time_set_p:%d err!", time_set_p);
            break;
        }
        break;
    case MOD_DEC:
        switch (time_set_p) {
        case 0: /* year */
            value = time_set_table[time_set_p].time_value - 1;
            if (value <= 2000) {
                value = 2001;
            }
            need_check  = MOD_DEC;
            break;
        case 1: /* month */
            value = time_set_table[time_set_p].time_value;
            if (value == 1) {
                value = 12;
            } else {
                value--;
            }
            need_check  = MOD_DEC;
            break;
        case 2: /* day */
            value = time_set_table[time_set_p].time_value;
            if (value == 1) {
                month = time_set_table[1].time_value;
                ASSERT(month >= 1 && month <= 12);
                value = leap_month_table[month - 1];
            } else {
                value--;
            }
            need_check = MOD_DEC;
            break;
        case 3: /* hour */
            value = time_set_table[time_set_p].time_value;
            if (value == 0) {
                value = 23;
            } else {
                value--;
            }
            break;
        case 4: /* min */
            value = time_set_table[time_set_p].time_value;
            if (value == 0) {
                value = 59;
            } else {
                value--;
            }
            break;
        case 5: /* sec */
            value = time_set_table[time_set_p].time_value;
            if (value == 0) {
                value = 59;
            } else {
                value--;
            }
            break;
        default:
            ASSERT(0, "mod_dec time_set_p:%d err!", time_set_p);
            break;
        }
        break;
    case MOD_SET:
        switch (time_set_p) {
        case 0: /* year */
            need_check = MOD_SET;
            break;
        case 1: /* month */
            ASSERT(value >= 1 && value <= 12);
            need_check = MOD_SET;
            break;
        case 2: /* day */
            need_check = MOD_SET;
            break;
        case 3: /* hour */
            ASSERT(value >= 0 && value <= 23);
            break;
        case 4: /* min */
            ASSERT(value >= 0 && value <= 59);
            break;
        case 5: /* sec */
            ASSERT(value >= 0 && value <= 59);
            break;
        default:
            ASSERT(0, "mod_set time_set_p:%d err!", time_set_p);
            break;
        }
        break;
    }
    time_set_table[time_set_p].time_value = value;

    printf("\n need_check==============%d\n", need_check);

    if (need_check) {
        year = time_set_table[0].time_value;
        month = time_set_table[1].time_value;
        day = time_set_table[2].time_value;
        if (month == 2 && !__is_leap_year(year)) {
            if (day >= 29) {
                if (need_check == MOD_ADD) {
                    day = 1;
                } else if (need_check == MOD_DEC) {
                    day = 28;
                } else {
                    day = 28;
                }
                time_set_table[2].time_value = day;
                __time_update_show(2); /* 调整day数值显示 */
            }

        } else {
            if (day > leap_month_table[month - 1]) {
                day = leap_month_table[month - 1];
                time_set_table[2].time_value = day;
                __time_update_show(2); /* 调整day数值显示 */
            }
        }
    }
    __time_update_show(time_set_p); /* 更新当前位数值显示 */

    return 0;
}
static int __time_get(struct sys_time *t)
{
    t->year  = time_set_table[0].time_value;
    t->month = time_set_table[1].time_value;
    t->day   = time_set_table[2].time_value;
    t->hour  = time_set_table[3].time_value;
    t->min   = time_set_table[4].time_value;
    t->sec   = time_set_table[5].time_value;
    return 0;
}
static int time_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("time_layout_onchange onfocus.\n");
        layout_on_focus(layout);
        __time_set_reset();
        sys_timer_set_on(SHOT, 0);
        break;
    case ON_CHANGE_RELEASE:
        puts("time_layout_onchange losefocus.\n");
        layout_lose_focus(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        puts("time_layout_onchange ON_CHANGE_FIRST_SHOW.\n");
        __time_set_switch(DIR_SET, "year");

        break;
    default:
        break;
    }
    return FALSE;
}




static int time_layout_onkey(void *ctr, struct element_key_event *e)
{

    switch (e->value) {
    case KEY_OK:
        puts("time_layout_onkey KEY_OK.\n");


        int table_sum = sizeof(time_set_table) / sizeof(struct time_setting);
        if (time_set_p >= (table_sum - 1)) {

            sys_timer_set_on(SHOT, 0);

            menu_sys_date_set(0);
            ui_hide(LAYOUT_MN_TIME_SYS);
        } else {

            __time_set_switch(DIR_NEXT, NULL);

        }




        break;
    case  SHOT:

        // sys_timer_set_on(SHOT,0 );

        //  menu_sys_date_set(0);
        //  send_rtc_time_tomcu();

        //  ui_hide(LAYOUT_MN_TIME_SYS);

        break ;


    case KEY_DOWN:
        __time_set_value(MOD_ADD, 0);

        break;
    case KEY_UP:
        __time_set_value(MOD_DEC, 0);

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        menu_sys_date_set(0);
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_TIME_SYS);
        //    menu_sys_date_set(0);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(LAYOUT_MN_TIME_SYS)
.onchange = time_layout_onchange,
 .onkey = time_layout_onkey,
  .ontouch = NULL,
};

/***************************** sensor 感应时间间隔 ************************************/

#if 0
struct time_setting {
    const char *mark;
    u32 time_id;
    u16 time_value;
};

#endif

/***************************** sensor 间隔时间设置 ************************************/
struct time_setting sensor_set_table[] = {
    //  {"hour",  SENSOR_GAP_HOUR_SYS,   0},
    {"min",   SENSOR_GAP_MINUTE_SYS, 0},
    {"sec",   SENSOR_GAP_SECOND_SYS, 0},
};

static int gap_timer_sys_min_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:

        //   time->hour = sensor_set_table[0].time_value;
        time->min = sensor_set_table[0].time_value;
        time->sec = sensor_set_table[1].time_value;
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(SENSOR_GAP_MINUTE_SYS)
.onchange = gap_timer_sys_min_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int gap_timer_sys_sec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:

        //  time->hour = sensor_set_table[0].time_value;
        time->min = sensor_set_table[0].time_value;
        time->sec = sensor_set_table[1].time_value;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SENSOR_GAP_SECOND_SYS)
.onchange = gap_timer_sys_sec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

extern void  gap_work_timer_set_on(enum sw_dir dir, const char *mark);
static u8 gap__worktime_set_reset(void)
{
    struct sys_time s_time, e_time;

    time_set_p = 0xff;
    /*
     * 此处应该读取系统RTC时间
     */




    db_select_buffer(GAP_TIMER_INFO, &s_time, sizeof(struct sys_time));

    printf("\n gap__worktime_set_reset_.min:%d, s_time.sec:%d,%d\n", s_time.min,  s_time.sec,  sizeof(struct sys_time));

    //  db_select_buffer(ALLOW_END_TIMER_INFO,&e_time,sizeof(struct sys_time));

    // printf("\ngap___time.min:%d, e_time.sec:%d\n",e_time.min,e_time.sec);

    if (s_time.min > 59 || s_time.sec > 59) { // 非法时间 设置默认15s Pir间隔
        s_time.min = 0;
        s_time.sec = 15;
        sensor_set_table[0].time_value = s_time.min;
        sensor_set_table[1].time_value = s_time.sec;
        gap_work_timer_set_on(SHOT, 0);
    }

    sensor_set_table[0].time_value = s_time.min;
    sensor_set_table[1].time_value = s_time.sec;






    return 0;
}


static u16 sensor_gap_set_p = 0xff; /* sensor_set_table 的当前设置指针 */
static u8 sensor_gap__time_update_show(u8 pos) /* 更新单个时间控件的时间 */
{
    struct utime t;

//   t.year  = sensor_set_table[0].time_value;
//   t.month = sensor_set_table[1].time_value;
//   t.day   = sensor_set_table[2].time_value;
    //  t.hour  = sensor_set_table[3].time_value;
    t.min   = sensor_set_table[0].time_value;
    t.sec   = sensor_set_table[1].time_value;
    printf("\n sensor_gap__time_update_show=====================%d,value:%d\n ", sensor_set_table[pos].time_value);
    ui_time_update_by_id(sensor_set_table[pos].time_id, &t);
    return 0;
}

static u8 __sensor_gap_set_value(enum set_mod mod, u16 value)
{

    enum set_mod need_check = 0;
    u16 year, month, day;
    ASSERT(mod == MOD_ADD || mod == MOD_DEC || mod == MOD_SET);
    printf("\n gap_mod===================%d,sensor_gap_set_p:%d\n", mod, sensor_gap_set_p);
    switch (mod) {
    case MOD_ADD:
        switch (sensor_gap_set_p) {


        case 0: /* min */
            value = sensor_set_table[sensor_gap_set_p].time_value + 1;
            if (value > 59) {
                value = 0;
            }
            break;
        case 1: /* sec */
            value = sensor_set_table[sensor_gap_set_p].time_value + 1;
            if (value > 59) {
                value = 0;
            }
            break;
        default:
            ASSERT(0, "mod_add sensor_gap_set_p:%d err!", sensor_gap_set_p);
            break;
        }
        break;
    case MOD_DEC:

        switch (sensor_gap_set_p) {

        case 0: /* hour */
            value = sensor_set_table[sensor_gap_set_p].time_value;
            if (value == 0) {
                value = 23;
            } else {
                value--;
            }
            break;
        case 1: /* min */
            value = sensor_set_table[sensor_gap_set_p].time_value;
            if (value == 0) {
                value = 59;
            } else {
                value--;
            }
            break;
        case 2: /* sec */
            value = sensor_set_table[sensor_gap_set_p].time_value;
            if (value == 0) {
                value = 59;
            } else {
                value--;
            }
            break;
        default:
            ASSERT(0, "mod_dec sensor_gap_set_p:%d err!", sensor_gap_set_p);
            break;
        }
        break;
    case MOD_SET:
        switch (sensor_gap_set_p) {

        case 0: /* min */
            ASSERT(value >= 0 && value <= 59);
            break;
        case 1: /* sec */
            ASSERT(value >= 0 && value <= 59);
            break;
        default:
            ASSERT(0, "mod_set sensor_gap_set_p:%d err!", sensor_gap_set_p);
            break;
        }
        break;
    }

    sensor_set_table[sensor_gap_set_p].time_value = value;

    printf("\n need_check==============%d,value:%d,sensor_set_table[sensor_gap_set_p].time_value:%d\n", need_check, value, sensor_set_table[sensor_gap_set_p].time_value);

    sensor_gap__time_update_show(sensor_gap_set_p); /* 更新当前位数值显示 */

    return 0;
}

static u8 __sensor_gap_search_by_mark(const char *mark)
{
    u16 p = 0;
    u16 table_sum = sizeof(sensor_set_table) / sizeof(struct time_setting);
    while (p < table_sum) {
        if (!strcmp(mark, sensor_set_table[p].mark)) {

            printf("\n  p==========%d\n", p);
            return p;
        }
        p++;
    }
    return -1;
}
static u8 __sensor_gap_set_switch(enum sw_dir dir, const char *mark)
{
    u16 table_sum;
    u16 prev_set_p = sensor_gap_set_p;
    u8 p;
    printf("__time_set_switch dir: %d\n", dir);
    table_sum = sizeof(sensor_set_table) / sizeof(struct time_setting);
    ASSERT(dir == DIR_NEXT || dir == DIR_PREV || dir == DIR_SET);
    switch (dir) {
    case DIR_NEXT:
        if (sensor_gap_set_p >= (table_sum - 1)) {
            sensor_gap_set_p = 0;
        } else {
            sensor_gap_set_p++;
        }
        break;
    case DIR_PREV:
        if (sensor_gap_set_p == 0) {
            sensor_gap_set_p = (table_sum - 1);
        } else {
            sensor_gap_set_p--;
        }
        break;
    case DIR_SET:
        p = __sensor_gap_search_by_mark(mark);
        if (p == (u8) - 1) {
            return -1;
        }
        sensor_gap_set_p = p;
        printf("\n sensor_gap_set_p===============%d\n", sensor_gap_set_p);
        break;
    }
    if (prev_set_p != 0xff) {
        printf("\n  gap_ui_no_highlight_prev_set_p:%d,%d", prev_set_p, sensor_set_table[prev_set_p].time_id);
        ui_no_highlight_element_by_id(sensor_set_table[prev_set_p].time_id);
    }
    printf("\n  gap_ui_highlight_prev_set_p:%d,id:%d", sensor_gap_set_p, sensor_set_table[sensor_gap_set_p].time_id);


    ui_highlight_element_by_id(sensor_set_table[sensor_gap_set_p].time_id);
    // ui_highlight_element_by_id(sensor_set_table[sensor_gap_set_p].time_id);
    printf("__sensor_gap_set_switch ok.\n");
    return 0;



}



void  gap_work_timer_set_on(enum sw_dir dir, const char *mark)
{


    struct sys_time s_time, e_time;


    s_time.min   = sensor_set_table[0].time_value;
    s_time.sec   = sensor_set_table[1].time_value;

    printf("\n gap__time.min:%d,s_time.sec:%d,%d\n", s_time.min, s_time.sec, sizeof(struct sys_time));

    db_update_buffer(GAP_TIMER_INFO, &s_time, sizeof(struct sys_time));





}


void  gap_work_timer_send_tomcu(enum sw_dir dir, const char *mark)
{


    struct sys_time s_time;


    //  s_time.min
    //  s_time.sec



    db_select_buffer(GAP_TIMER_INFO, &s_time, sizeof(struct sys_time));

    printf("\n send_gap_time.min:%d,s_time.sec:%d,%d\n", s_time.min, s_time.sec, sizeof(struct sys_time));

    //  MCU_Camera_Interval(1, s_time.min*60+s_time.sec);


}



static int sensor_gap_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("sensor_gap__layout_onchange onfocus.\n");
        layout_on_focus(layout);
        gap__worktime_set_reset();
        break;
    case ON_CHANGE_RELEASE:
        puts("sensor_gap_layout_onchange losefocus.\n");
        layout_lose_focus(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        puts("sensor_gap_layout_onchange ON_CHANGE_FIRST_SHOW.\n");
        __sensor_gap_set_switch(DIR_SET, "min");

        break;
    default:
        break;
    }
    return FALSE;
}

void save_gap_timer()
{

    struct sys_time s_time;
    gap_work_timer_set_on(SHOT, 0);


    db_select_buffer(GAP_TIMER_INFO, &s_time, sizeof(struct sys_time));

    printf("\n send_gap_time.min:%d,s_time.sec:%d,%d\n", s_time.min, s_time.sec, sizeof(struct sys_time));
    ui_hide(LAYOUT_MN_SENSOR_GAP);





}


static int sensor_gap_layout_onkey(void *ctr, struct element_key_event *e)
{

    switch (e->value) {
    case KEY_OK:


        puts(" sensor_gap_layout_onkey KEY_OK.\n");
        int table_sum = sizeof(sensor_set_table) / sizeof(struct time_setting);
        if (sensor_gap_set_p >= (table_sum - 1)) {

            save_gap_timer();
        } else {
            __sensor_gap_set_switch(DIR_NEXT, NULL);
        }








        break;
    case  SHOT:

        gap_work_timer_set_on(SHOT, 0);


        gap_work_timer_send_tomcu(1, 0);
        ui_hide(LAYOUT_MN_SENSOR_GAP);

        break ;

    case KEY_DOWN:
        __sensor_gap_set_value(MOD_ADD, 0);

        break;
    case KEY_UP:
        __sensor_gap_set_value(MOD_DEC, 0);

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        // menu_sys_date_set(0);
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_SENSOR_GAP);
        // menu_sys_date_set(0);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(LAYOUT_MN_SENSOR_GAP)
.onchange = sensor_gap_layout_onchange,
 .onkey = sensor_gap_layout_onkey,
  .ontouch = NULL,
};




/*****************************工作时间设定，只在该时间内 有感应才工作************************************/



#if 0
struct time_setting {
    const char *mark;
    u32 time_id;
    u16 time_value;
};

#endif

/***************************** 工作时间设定，只在该时间内 有感应才工作 ************************************/
struct time_setting allow_worktimer_set_table[] = {
    {"hour",  START_ALLOW_WORK_HOUR_SYS,   0},
    {"min",   START_ALLOW_WORK_MINUTE_SYS, 0},
    {"sec",   START_ALLOW_WORK_SECOND_SYS, 0},

    {"ehour",  END_ALLOW_WORK_HOUR_SYS,   0},
    {"emin",   END_ALLOW_WORK_MINUTE_SYS, 0},
    {"esec",   END_ALLOW_WORK_SECOND_SYS, 0},
};

static int start_timer_sys_hour_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:

        time->hour = allow_worktimer_set_table[0].time_value;
        // time->min = allow_worktimer_set_table[1].time_value;
        // time->sec = allow_worktimer_set_table[2].time_value;

        printf("\n starthour_time->hour:%d,time->min:%d,time->sec:%d\n ", time->hour, time->min, time->sec);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(START_ALLOW_WORK_HOUR_SYS)
.onchange = start_timer_sys_hour_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int start_timer_sys_min_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        //  time->year = allow_worktimer_set_table[0].time_value;
        //  time->month = allow_worktimer_set_table[1].time_value;
        //  time->day = allow_worktimer_set_table[2].time_value;


        //   time->hour = allow_worktimer_set_table[0].time_value;
        time->min = allow_worktimer_set_table[1].time_value;
        //   time->sec = allow_worktimer_set_table[2].time_value;

        printf("\n startmin_time->hour:%d,time->min:%d,time->sec:%d\n ", time->hour, time->min, time->sec);


        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(START_ALLOW_WORK_MINUTE_SYS)
.onchange = start_timer_sys_min_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int start_timer_sys_sec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:

        //   time->hour = allow_worktimer_set_table[0].time_value;
        //  time->min = allow_worktimer_set_table[1].time_value;
        time->sec = allow_worktimer_set_table[2].time_value;

        printf("\n startsec_time->hour:%d,time->min:%d,time->sec:%d\n ", time->hour, time->min, time->sec);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(START_ALLOW_WORK_SECOND_SYS)
.onchange = start_timer_sys_sec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int end_timer_sys_hour_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:

        time->hour = allow_worktimer_set_table[3].time_value;
        //  time->min = allow_worktimer_set_table[4].time_value;
        //  time->sec = allow_worktimer_set_table[5].time_value;
        printf("\n endhour_time->hour:%d,time->min:%d,time->sec:%d\n ", time->hour, time->min, time->sec);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(END_ALLOW_WORK_HOUR_SYS)
.onchange = end_timer_sys_hour_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int end_timer_sys_min_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        //  time->year = allow_worktimer_set_table[0].time_value;
        //  time->month = allow_worktimer_set_table[1].time_value;
        //  time->day = allow_worktimer_set_table[2].time_value;
        //  time->hour = allow_worktimer_set_table[0].time_value;
        time->min = allow_worktimer_set_table[4].time_value;
        //  time->sec = allow_worktimer_set_table[2].time_value;
        printf("\n endmin_time->hour:%d,time->min:%d,time->sec:%d\n ", time->hour, time->min, time->sec);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(END_ALLOW_WORK_MINUTE_SYS)
.onchange = end_timer_sys_min_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int end_timer_sys_sec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        //   time->year = allow_worktimer_set_table[0].time_value;
        //   time->month = allow_worktimer_set_table[1].time_value;
        //   time->day = allow_worktimer_set_table[2].time_value;
        //   time->hour = allow_worktimer_set_table[0].time_value;
        //   time->min = allow_worktimer_set_table[1].time_value;
        time->sec = allow_worktimer_set_table[5].time_value;

        printf("\n endsec_time->hour:%d,time->min:%d,time->sec:%d\n ", time->hour, time->min, time->sec);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(END_ALLOW_WORK_SECOND_SYS)
.onchange = end_timer_sys_sec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static u16 allow_worktimer_set_p = 0xff; /* worktimer_set_p 的当前设置指针 */  //sensor_gap_set_p
static u8 allow_work__time_update_show(u8 pos) /* 更新单个时间控件的时间 */ //work__time_update_show
{
    struct utime t;

// t.year  = allow_worktimer_set_table[0].time_value;
// t.month = allow_worktimer_set_table[1].time_value;
// t.day   = allow_worktimer_set_table[2].time_value;

    switch (pos) {

    case 0:
    case 1:
    case 2:
        t.hour  = allow_worktimer_set_table[0].time_value;
        t.min   = allow_worktimer_set_table[1].time_value;
        t.sec   = allow_worktimer_set_table[2].time_value;
        printf("\n start_pos=====================%d,value:%d\n ", pos, allow_worktimer_set_table[pos].time_value);
        ui_time_update_by_id(allow_worktimer_set_table[pos].time_id, &t);

        break ;




    case 3:
    case 4:
    case 5:
        t.hour  = allow_worktimer_set_table[3].time_value;
        t.min   = allow_worktimer_set_table[4].time_value;
        t.sec   = allow_worktimer_set_table[5].time_value;
        printf("\n end_pos=====================%d,value:%d\n ", pos, allow_worktimer_set_table[pos].time_value);
        ui_time_update_by_id(allow_worktimer_set_table[pos].time_id, &t);


        break ;

    default:
        break ;



    }




    return 0;
}

static u8 allow__worktimer_set_value(enum set_mod mod, u16 value)  //__worktimer_set_value
{

    enum set_mod need_check = 0;
    u16 year, month, day;
    ASSERT(mod == MOD_ADD || mod == MOD_DEC || mod == MOD_SET);
    printf("\n allow_gap_mod===================%d,allow_worktimer_set_p:%d\n", mod, allow_worktimer_set_p);
    switch (mod) {
    case MOD_ADD:
        switch (allow_worktimer_set_p) {

#if  0
        case 0: /* year */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value + 1;
            if (value >= 2100) {
                value = 2099;
            }
            need_check  = MOD_ADD;
            break;
        case 1: /* month */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value + 1;


            if (value > 12) {
                value = 1;
            }
            need_check  = MOD_ADD;
            break;
        case 2: /* day */
            month = allow_worktimer_set_table[1].time_value;
            ASSERT(month >= 1 && month <= 12);
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value + 1;
            if (value > leap_month_table[month - 1]) {
                value = 1;
            }
            need_check  = MOD_ADD;
            break;
#endif
        case 0: /*start hour */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value + 1;
            if (value > 23) {
                value = 0;
            }
            break;

        case 1: /*start min */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value + 1;
            if (value > 59) {
                value = 0;
            }
            break;
        case 2: /* start sec */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value + 1;
            if (value > 59) {
                value = 0;
            }
            break;
        default:


        case 3: /*end hour */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value + 1;
            if (value > 23) {
                value = 0;
            }
            break;

        case 4: /*end min */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value + 1;
            if (value > 59) {
                value = 0;
            }
            break;
        case 5: /*end sec */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value + 1;
            if (value > 59) {
                value = 0;
            }
            break;


            ASSERT(0, "mod_add allow_worktimer_set_p:%d err!", allow_worktimer_set_p);
            break;
        }
        break;
    case MOD_DEC:
        switch (allow_worktimer_set_p) {

        case 0: /*start hour */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value;
            if (value == 0) {
                value = 23;
            } else {
                value--;
            }
            break;
        case 1: /*start min */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value;
            if (value == 0) {
                value = 59;
            } else {
                value--;
            }
            break;
        case 2: /*start sec */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value;
            if (value == 0) {
                value = 59;
            } else {
                value--;
            }
            break;
        case 3: /*end hour */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value;
            if (value == 0) {
                value = 23;
            } else {
                value--;
            }
            break;
        case 4: /*end min */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value;
            if (value == 0) {
                value = 59;
            } else {
                value--;
            }
            break;
        case 5: /*end sec */
            value = allow_worktimer_set_table[allow_worktimer_set_p].time_value;
            if (value == 0) {
                value = 59;
            } else {
                value--;
            }
            break;

        default:
            ASSERT(0, "mod_dec allow_worktimer_set_p:%d err!", allow_worktimer_set_p);
            break;
        }
        break;
    case MOD_SET:
        switch (allow_worktimer_set_p) {

#if 0
        case 0: /* year */
            need_check = MOD_SET;
            break;
        case 1: /* month */
            ASSERT(value >= 1 && value <= 12);
            need_check = MOD_SET;
            break;
        case 2: /* day */
            need_check = MOD_SET;
            break;
#endif
        case 0: /* hour */
            ASSERT(value >= 0 && value <= 23);
            break;
        case 1: /* min */
            ASSERT(value >= 0 && value <= 59);
            break;
        case 2: /* sec */
            ASSERT(value >= 0 && value <= 59);
            break;

        case 3: /* end hour */
            ASSERT(value >= 0 && value <= 23);
            break;
        case 4: /* end min */
            ASSERT(value >= 0 && value <= 59);
            break;
        case 5: /* end sec */
            ASSERT(value >= 0 && value <= 59);
            break;


        default:
            ASSERT(0, "mod_set allow_worktimer_set_p:%d err!", allow_worktimer_set_p);
            break;
        }
        break;
    }

    allow_worktimer_set_table[allow_worktimer_set_p].time_value = value;

    printf("\n workneed_check==============%d,value:%d,allow_worktimer_set_table[allow_worktimer_set_p].time_value:%d\n", need_check, value, allow_worktimer_set_table[allow_worktimer_set_p].time_value);
#if  0
    if (need_check) {
        year = allow_worktimer_set_table[0].time_value;
        month = allow_worktimer_set_table[1].time_value;
        day = allow_worktimer_set_table[2].time_value;
        if (month == 2 && !__is_leap_year(year)) {
            if (day >= 29) {
                if (need_check == MOD_ADD) {
                    day = 1;
                } else if (need_check == MOD_DEC) {
                    day = 28;
                } else {
                    day = 28;
                }
                allow_worktimer_set_table[2].time_value = day;
                __time_update_show(2); /* 调整day数值显示 */
            }

        } else {
            if (day > leap_month_table[month - 1]) {
                day = leap_month_table[month - 1];
                allow_worktimer_set_table[2].time_value = day;
                __time_update_show(2); /* 调整day数值显示 */
            }
        }
    }

#endif
    allow_work__time_update_show(allow_worktimer_set_p); /* 更新当前位数值显示 */

    return 0;
}

static u8 allow__worktimer_search_by_mark(const char *mark) //__sensor_gap_search_by_mark
{
    u16 p = 0;
    u16 table_sum = sizeof(allow_worktimer_set_table) / sizeof(struct time_setting);
    while (p < table_sum) {
        if (!strcmp(mark, allow_worktimer_set_table[p].mark)) {

            printf("\n  p==========%d\n", p);
            return p;
        }
        p++;
    }
    return -1;
}
static u8 allow__work_timer_set_switch(enum sw_dir dir, const char *mark) // __work_timer_set_switch
{
    u16 table_sum;
    u16 prev_set_p = allow_worktimer_set_p;
    u8 p;
    printf("__time_set_switch dir: %d\n", dir);
    table_sum = sizeof(allow_worktimer_set_table) / sizeof(struct time_setting);
    ASSERT(dir == DIR_NEXT || dir == DIR_PREV || dir == DIR_SET);
    switch (dir) {
    case DIR_NEXT:
        if (allow_worktimer_set_p >= (table_sum - 1)) {
            allow_worktimer_set_p = 0;
        } else {
            allow_worktimer_set_p++;
        }
        break;
    case DIR_PREV:
        if (allow_worktimer_set_p == 0) {
            allow_worktimer_set_p = (table_sum - 1);
        } else {
            allow_worktimer_set_p--;
        }
        break;
    case DIR_SET:
        p = allow__worktimer_search_by_mark(mark);
        if (p == (u8) - 1) {
            return -1;
        }
        allow_worktimer_set_p = p;
        printf("\n allow_worktimer_set_p===============%d\n", allow_worktimer_set_p);
        break;
    }
    if (prev_set_p != 0xff) {
        printf("\n  worktimer_ui_no_highlight_prev_set_p:%d,%d", prev_set_p, allow_worktimer_set_table[prev_set_p].time_id);
        ui_no_highlight_element_by_id(allow_worktimer_set_table[prev_set_p].time_id);
    }
    printf("\n allow_worktimer_ui_highlight_prev_set_p:%d,id:%d", allow_worktimer_set_p, allow_worktimer_set_table[allow_worktimer_set_p].time_id);


    ui_highlight_element_by_id(allow_worktimer_set_table[allow_worktimer_set_p].time_id);
    // ui_highlight_element_by_id(allow_worktimer_set_table[allow_worktimer_set_p].time_id);
    printf("allow_worktimer_timer_set_switch ok.\n");
    return 0;



}

extern int db_select_buffer(u8 index, char *buffer, int len);
extern int db_update_buffer(u8 index, char *buffer, int len);
static u8 allow__worktime_set_reset(void)
{
    struct sys_time s_time, e_time;

    time_set_p = 0xff;
    /*
     * 此处应该读取系统RTC时间
     */


    //  get_sys_time(&time);

    //char s_timer[50]={0};

    db_select_buffer(ALLOW_START_TIMER_INFO, &s_time, sizeof(struct sys_time));

    printf("\nallow__worktime_start.hour:%d,s_time.min:%d, s_time.sec:%d,%d\n", s_time.hour, s_time.min, s_time.sec, sizeof(struct sys_time));

    db_select_buffer(ALLOW_END_TIMER_INFO, &e_time, sizeof(struct sys_time));

    printf("\n allow__worktime_end_time.hour:%d,e_time.min:%d, e_time.sec:%d\n", e_time.hour, e_time.min, e_time.sec);
    allow_worktimer_set_table[0].time_value = s_time.hour;
    allow_worktimer_set_table[1].time_value = s_time.min;
    allow_worktimer_set_table[2].time_value = s_time.sec;




    allow_worktimer_set_table[3].time_value = e_time.hour;
    allow_worktimer_set_table[4].time_value = e_time.min;
    allow_worktimer_set_table[5].time_value = e_time.sec;

    return 0;
}


void  allow__work_timer_set_on(enum sw_dir dir, const char *mark)
{


    struct sys_time s_time, e_time;

    s_time.hour  = allow_worktimer_set_table[0].time_value;
    s_time.min   = allow_worktimer_set_table[1].time_value;
    s_time.sec   = allow_worktimer_set_table[2].time_value;

    printf("\n uddate_s_time.hour:%d,s_time.min:%d,s_time.sec:%d,%d\n", s_time.hour, s_time.min, s_time.sec, sizeof(struct sys_time));

    db_update_buffer(ALLOW_START_TIMER_INFO, &s_time, sizeof(struct sys_time));


    e_time.hour  = allow_worktimer_set_table[3].time_value;
    e_time.min   = allow_worktimer_set_table[4].time_value;
    e_time.sec   = allow_worktimer_set_table[5].time_value;

    db_update_buffer(ALLOW_END_TIMER_INFO, &e_time, sizeof(struct sys_time));

    printf("\n uddate_e_time.hour:%d,s_time.min:%d,s_time.sec:%d,%d\n", e_time.hour, e_time.min, e_time.sec, sizeof(struct sys_time));



}



/****************************仅仅在设定的时间内 有感应时才工作******************************/
static int allow_worktimer_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("allow_worktimer__layout_onchange onfocus.\n");
        layout_on_focus(layout);
        allow__worktime_set_reset();
        break;
    case ON_CHANGE_RELEASE:
        puts("allow_worktimer_layout_onchange losefocus.\n");
        layout_lose_focus(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        puts("allow_worktimer_layout_onchange ON_CHANGE_FIRST_SHOW.\n");
        // allow__work_timer_set_switch(DIR_SET, "sec");
        allow__work_timer_set_switch(DIR_SET, "hour");
        break;
    default:
        break;
    }
    return FALSE;
}
static int allow_worktimer_layout_onkey(void *ctr, struct element_key_event *e)
{

    switch (e->value) {
    case KEY_OK:



        puts(" allow_worktimer_layout_onkey KEY_OK.\n");
        int table_sum = sizeof(allow_worktimer_set_table) / sizeof(struct time_setting);
        if (allow_worktimer_set_p >= (table_sum - 1)) {

            allow__work_timer_set_on(SHOT, NULL);
            ui_hide(LAYOUT_MN_ALLOW_TIMER_WORK);


        } else {

            allow__work_timer_set_switch(DIR_NEXT, NULL);
        }


        break;

    case SHOT:  //需要确认键确认保存时间



        // 保存时间并设置倒计时给关机
        printf("\n  <<<<<<<<<<<<<<<<<<<<<<<<<<set_allowtime>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        allow__work_timer_set_on(SHOT, NULL);
        ui_hide(LAYOUT_MN_ALLOW_TIMER_WORK);

        extern  void MCU_Timer_OnOff(uint8_t status, uint8_t start_hour, uint8_t start_minute, uint8_t end_hour, uint8_t end_minute);
        struct sys_time gs_time, ge_time;
        db_select_buffer(ALLOW_START_TIMER_INFO, &gs_time, sizeof(struct sys_time));


        db_select_buffer(ALLOW_END_TIMER_INFO, &ge_time, sizeof(struct sys_time));

        printf("\ngsallow__worktime_set_gs_time.hour:%d,gs_time.min:%d, gs_time.sec:%d,%d\n", gs_time.hour, gs_time.min, gs_time.sec, sizeof(struct sys_time));
        printf("\ngeallow__worktime_set_ge_time.hour:%d,ge_time.min:%d, ge_time.sec:%d,%d\n", ge_time.hour, ge_time.min, ge_time.sec, sizeof(struct sys_time));
        //  MCU_Timer_OnOff(1, gs_time.hour,gs_time.min, ge_time.hour, ge_time.sec);
        break;
    case KEY_DOWN:
        allow__worktimer_set_value(MOD_ADD, 0);

        break;
    case KEY_UP:
        allow__worktimer_set_value(MOD_DEC, 0);

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        // menu_sys_date_set(0);
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_ALLOW_TIMER_WORK);
        // menu_sys_date_set(0);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(LAYOUT_MN_ALLOW_TIMER_WORK)
.onchange = allow_worktimer_layout_onchange,
 .onkey = allow_worktimer_layout_onkey,
  .ontouch = NULL,
};





/****************************允许定时工作是否开启******************************/



static int allow_work_onoff_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("tallow"), TABLE(table_tallow_lable));


        printf("\n item======================ALLOW:%d,%d\n ", db_select("tallow"), item);
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;

}
static int allow_work_onoff_layout_onkey(void *ctr, struct element_key_event *e)
{

    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item = 0;
    struct intent it;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        // menu_sys_tv_mod_set(sel_item);

        printf("\n allow_work_onoff_layout_onkey:%d\n", sel_item);
        init_intent(&it);
        it.name	= "video_system";
        it.action = ACTION_SYSTEM_SET_CONFIG;
        it.data = "tallow";
        it.exdata = sel_item;
        int err = start_app(&it);
        if (err) {
            printf(" allow work onkey err! %d\n", err);
            break;
        }

        ui_hide(LAYOUT_ALLOWWORK_ONOFF);
        if (sel_item) {

            ui_show(LAYOUT_MN_ALLOW_TIMER_WORK);
        } else {
            printf("\n  <<<<<<<<<<<<<<<<<<<<<<<<close  allow_timer>>>>>>>>>>>>>>>>>> ");
            //extern void MCU_Timer_OnOff(uint8_t status, uint8_t start_hour, uint8_t start_minute, uint8_t end_hour, uint8_t end_minute);
            // MCU_Timer_OnOff(0, 0, 0,0, 0);


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
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_ALLOWWORK_ONOFF);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_ALLOWWORK_ONOFF)
.onchange = allow_work_onoff_layout_onchange,
 .onkey = allow_work_onoff_layout_onkey,
  .ontouch = NULL,
};





/****************************定时器工作，不管有没有触发 都在定时起来工作***************************/



/***************************** 定时器工作，不管有没有触发 都在定时起来工作 ************************************/
struct time_setting worktimer_set_table[] = {
    {"hour",  START_TIMER_WORK_HOUR_SYS,   0},
    {"min",   START_TIMER_WORK_MINUTE_SYS, 0},
    {"sec",   START_TIMER_WORK_SECOND_SYS, 0},
};

#if 0
struct time_setting {
    const char *mark;
    u32 time_id;
    u16 time_value;
};

#endif
static int work_timer_sys_hour_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        //  time->year = allow_worktimer_set_table[0].time_value;
        //  time->month = allow_worktimer_set_table[1].time_value;
        //  time->day = allow_worktimer_set_table[2].time_value;
        time->hour = worktimer_set_table[0].time_value;
        time->min = worktimer_set_table[1].time_value;
        time->sec = worktimer_set_table[2].time_value;

        printf("\n work_timer_sys_hour_onchange hour:%d,time.min:%d, time.sec:%d,%d\n", time->hour, time->min, time->sec);

        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(START_TIMER_WORK_HOUR_SYS)
.onchange = work_timer_sys_hour_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int work_timer_sys_min_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:

        time->hour = worktimer_set_table[0].time_value;
        time->min = worktimer_set_table[1].time_value;
        time->sec = worktimer_set_table[2].time_value;
        printf("\n work_timer_sys_min_onchange hour:%d,time.min:%d, time.sec:%d,%d\n", time->hour, time->min, time->sec);

        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(START_TIMER_WORK_MINUTE_SYS)
.onchange = work_timer_sys_min_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int work_timer_sys_sec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        //   time->year = allow_worktimer_set_table[0].time_value;
        //   time->month = allow_worktimer_set_table[1].time_value;
        //   time->day = allow_worktimer_set_table[2].time_value;
        time->hour = worktimer_set_table[0].time_value;
        time->min = worktimer_set_table[1].time_value;
        time->sec = worktimer_set_table[2].time_value;
        printf("\n work_timer_sys_sec_onchange hour:%d,time.min:%d, time.sec:%d,%d\n", time->hour, time->min, time->sec);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(START_TIMER_WORK_SECOND_SYS)
.onchange = work_timer_sys_sec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static u8 work__worktime_set_reset(void)
{
    struct sys_time s_time, e_time;

    time_set_p = 0xff;
    /*
     * 此处应该读取系统RTC时间
     */


    //  get_sys_time(&time);

    //char s_timer[50]={0};

    db_select_buffer(WORK_TIMER_INFO, &s_time, sizeof(struct sys_time));

    printf("\n work__worktime_set_reset.hour:%d,s_time.min:%d, s_time.sec:%d,%d\n", s_time.hour, s_time.min, s_time.sec, sizeof(struct sys_time));

    //  db_select_buffer(ALLOW_END_TIMER_INFO,&e_time,sizeof(struct sys_time));


    worktimer_set_table[0].time_value = s_time.hour;
    worktimer_set_table[1].time_value = s_time.min;
    worktimer_set_table[2].time_value = s_time.sec;




    // worktimer_set_table[3].time_value = e_time.hour;
    // worktimer_set_table[4].time_value = e_time.min;
    // worktimer_set_table[5].time_value = e_time.sec;

    return 0;
}

static u16 worktimer_set_p = 0xff; /* worktimer_set_table 的当前设置指针 */  //sensor_gap_set_p
static u8 work__time_update_show(u8 pos) /* 更新单个时间控件的时间 */ //sensor_gap__time_update_show
{
    struct utime t;

//   t.year  = worktimer_set_table[0].time_value;
//   t.month = worktimer_set_table[1].time_value;
//   t.day   = worktimer_set_table[2].time_value;
    t.hour  = worktimer_set_table[0].time_value;
    t.min   = worktimer_set_table[1].time_value;
    t.sec   = worktimer_set_table[2].time_value;
    printf("\n work__time_update_show_t.hour=====================%d,t.min:%d,t.sec:%d,pos:%d\n ", t.hour, t.min, t.sec, pos);
    ui_time_update_by_id(worktimer_set_table[pos].time_id, &t);
    return 0;
}

static u8 __worktimer_set_value(enum set_mod mod, u16 value)  //__sensor_gap_set_value
{

    enum set_mod need_check = 0;
    u16 year, month, day;
    ASSERT(mod == MOD_ADD || mod == MOD_DEC || mod == MOD_SET);
    printf("\n timer_point_mod===================%d,worktimer_set_p:%d\n", mod, worktimer_set_p);
    switch (mod) {
    case MOD_ADD:
        switch (worktimer_set_p) {

        case 0: /* hour */
            value = worktimer_set_table[worktimer_set_p].time_value + 1;
            if (value > 23) {
                value = 0;
            }
            break;

        case 1: /* min */
            value = worktimer_set_table[worktimer_set_p].time_value + 1;
            if (value > 59) {
                value = 0;
            }
            break;
        case 2: /* sec */
            value = worktimer_set_table[worktimer_set_p].time_value + 1;
            if (value > 59) {
                value = 0;
            }
            break;
        default:
            ASSERT(0, "mod_add worktimer_set_p:%d err!", worktimer_set_p);
            break;
        }
        break;
    case MOD_DEC:
        switch (worktimer_set_p) {


        case 0: /* hour */
            value = worktimer_set_table[worktimer_set_p].time_value;
            if (value == 0) {
                value = 23;
            } else {
                value--;
            }
            break;
        case 1: /* min */
            value = worktimer_set_table[worktimer_set_p].time_value;
            if (value == 0) {
                value = 59;
            } else {
                value--;
            }
            break;
        case 2: /* sec */
            value = worktimer_set_table[worktimer_set_p].time_value;
            if (value == 0) {
                value = 59;
            } else {
                value--;
            }
            break;
        default:
            ASSERT(0, "mod_dec worktimer_set_p:%d err!", worktimer_set_p);
            break;
        }
        break;
    case MOD_SET:
        switch (worktimer_set_p) {


        case 0: /* hour */
            ASSERT(value >= 0 && value <= 23);
            break;
        case 1: /* min */
            ASSERT(value >= 0 && value <= 59);
            break;
        case 2: /* sec */
            ASSERT(value >= 0 && value <= 59);
            break;
        default:
            ASSERT(0, "mod_set worktimer_set_p:%d err!", worktimer_set_p);
            break;
        }
        break;
    }

    worktimer_set_table[worktimer_set_p].time_value = value;

    printf("\n workneed_check==============%d,value:%d,worktimer_set_table[worktimer_set_p].time_value:%d\n", need_check, value, worktimer_set_table[worktimer_set_p].time_value);

    work__time_update_show(worktimer_set_p); /* 更新当前位数值显示 */

    return 0;
}

static u8 __worktimer_search_by_mark(const char *mark) //__sensor_gap_search_by_mark
{
    u16 p = 0;
    u16 table_sum = sizeof(worktimer_set_table) / sizeof(struct time_setting);
    while (p < table_sum) {
        if (!strcmp(mark, worktimer_set_table[p].mark)) {

            printf("\n  work_timer_p==========%d\n", p);
            return p;
        }
        p++;
    }
    return -1;
}
static u8 __work_timer_set_switch(enum sw_dir dir, const char *mark)
{
    u16 table_sum;
    u16 prev_set_p = worktimer_set_p;
    u8 p;
    printf("__time_set_switch dir: %d\n", dir);
    table_sum = sizeof(worktimer_set_table) / sizeof(struct time_setting);
    ASSERT(dir == DIR_NEXT || dir == DIR_PREV || dir == DIR_SET);
    switch (dir) {
    case DIR_NEXT:
        if (worktimer_set_p >= (table_sum - 1)) {
            worktimer_set_p = 0;
        } else {
            worktimer_set_p++;
        }
        break;
    case DIR_PREV:
        if (worktimer_set_p == 0) {
            worktimer_set_p = (table_sum - 1);
        } else {
            worktimer_set_p--;
        }
        break;
    case DIR_SET:
        p = __worktimer_search_by_mark(mark);
        if (p == (u8) - 1) {
            return -1;
        }
        worktimer_set_p = p;
        printf("\n worktimer_set_p===============%d\n", worktimer_set_p);
        break;
    }
    if (prev_set_p != 0xff) {
        printf("\n  worktimer_ui_no_highlight_prev_set_p:%d,%d", prev_set_p, worktimer_set_table[prev_set_p].time_id);
        ui_no_highlight_element_by_id(worktimer_set_table[prev_set_p].time_id);
    }
    printf("\n worktimer_ui_highlight_prev_set_p:%d,ui_id:%d", worktimer_set_p, worktimer_set_table[worktimer_set_p].time_id);


    ui_highlight_element_by_id(worktimer_set_table[worktimer_set_p].time_id);
    // ui_highlight_element_by_id(worktimer_set_table[worktimer_set_p].time_id);
    printf("__work_timer_set_switch ok.\n");
    return 0;



}

void  set__work_timer_set_on(enum sw_dir dir, const char *mark)
{


    struct sys_time s_time, e_time;

    s_time.hour  = worktimer_set_table[0].time_value;
    s_time.min   = worktimer_set_table[1].time_value;
    s_time.sec   = worktimer_set_table[2].time_value;


    db_update_buffer(WORK_TIMER_INFO, &s_time, sizeof(struct sys_time));

    printf("\n set__work_timer_set_on_time.hour:%d,s_time.min:%d,s_time.sec:%d,%d\n", s_time.hour, s_time.min, s_time.sec, sizeof(struct sys_time));



}


/****************************定时设置  不管有没有触发都会定时器起来工作******************************/

void save_every_timer()
{


    set__work_timer_set_on(SHOT, NULL);

    ui_hide(LAYOUT_MN_TIMER_WORK);



    struct sys_time ws_time;
    db_select_buffer(WORK_TIMER_INFO, &ws_time, sizeof(struct sys_time));


    db_update("alarm", 1); // 更新菜单参数

    db_flush();

    printf("\n every_timer_layout_onkey_set_ws_time.hour:%d,ws_time.min:%d, ws_time.sec:%d,%d\n", ws_time.hour, ws_time.min, ws_time.sec, sizeof(struct sys_time));


}

extern void MCU_Timer_Camera_OnOff(uint8_t status, uint8_t hour, uint8_t minute, uint8_t second);
static int every_timer_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("every__layout_onchange onfocus.\n");
        layout_on_focus(layout);
        work__worktime_set_reset();
        break;
    case ON_CHANGE_RELEASE:
        puts("every_layout_onchange losefocus.\n");
        layout_lose_focus(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        puts("every_layout_onchange ON_CHANGE_FIRST_SHOW.\n");
        __work_timer_set_switch(DIR_SET, "hour");

        break;
    default:
        break;
    }
    return FALSE;
}




static int every_timer_layout_onkey(void *ctr, struct element_key_event *e)
{

    switch (e->value) {
    case KEY_OK:
        puts(" everytimer_layout_onkey KEY_OK.\n");

        int table_sum = sizeof(worktimer_set_table) / sizeof(struct time_setting);
        if (worktimer_set_p >= (table_sum - 1)) {

            save_every_timer();

            db_update("delay_pv", 1); // 更新定时拍照标志
            db_flush();
        } else {
            __work_timer_set_switch(DIR_NEXT, NULL);
        }
        break;
    case SHOT:
        set__work_timer_set_on(SHOT, NULL);

        ui_hide(LAYOUT_MN_TIMER_WORK);



        struct sys_time ws_time;
        db_select_buffer(WORK_TIMER_INFO, &ws_time, sizeof(struct sys_time));


        db_update("alarm", 1); // 更新闹钟参数
        db_flush();


        printf("\n every_timer_layout_onkey_set_ws_time.hour:%d,ws_time.min:%d, ws_time.sec:%d,%d\n", ws_time.hour, ws_time.min, ws_time.sec, sizeof(struct sys_time));



        // MCU_Timer_Camera_OnOff(1, ws_time.hour,ws_time.min,  ws_time.sec);


        break;
    case KEY_DOWN:
        __worktimer_set_value(MOD_ADD, 0);

        break;
    case KEY_UP:
        __worktimer_set_value(MOD_DEC, 0);

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        // menu_sys_date_set(0);
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_TIMER_WORK);
        // menu_sys_date_set(0);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(LAYOUT_MN_TIMER_WORK)
.onchange = every_timer_layout_onchange,
 .onkey = every_timer_layout_onkey,
  .ontouch = NULL,
};




static int every_time_onoff_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("sentimer"), TABLE(table_everytime_lable));


        printf("\n item======================sentimer:%d,%d\n ", db_select("sentimer"), item);
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;

}
static int every_time_onoff_layout_onkey(void *ctr, struct element_key_event *e)
{

    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item = 0;
    struct intent it;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        // menu_sys_tv_mod_set(sel_item);

        printf("\n every_time_onoff_layout_onkey:%d\n", sel_item);
        init_intent(&it);
        it.name	= "video_system";
        it.action = ACTION_SYSTEM_SET_CONFIG;
        it.data = "sentimer";
        it.exdata = sel_item;
        int err = start_app(&it);
        if (err) {
            printf(" every timer work onkey err! %d\n", err);
            break;
        }

        ui_hide(LAYOUT_EVERYTIME_WORK_ONOFF);
        if (sel_item) {

            ui_show(LAYOUT_MN_TIMER_WORK);
        } else {
            //  关闭定时起来工作
            // db_update("alarm",0); //关闭定时工作
            db_update("delay_pv", 0); //关闭定时工作
            db_flush();

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
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_EVERYTIME_WORK_ONOFF);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_EVERYTIMER_WWORK_ONOFF)
.onchange = every_time_onoff_layout_onchange,
 .onkey = every_time_onoff_layout_onkey,
  .ontouch = NULL,
};




/***************************** 系统录像声音开关 ************************************/

static int menu_audio_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("mic"), TABLE(table_system_audio_mod));
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_audio_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item = 0;
    struct intent it;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        // menu_sys_tv_mod_set(sel_item);

        printf("\n audio_sel_item:%d\n", sel_item);
        init_intent(&it);
        it.name	= "video_system";
        it.action = ACTION_SYSTEM_SET_CONFIG;
        it.data = "mic";
        it.exdata = sel_item;
        int err = start_app(&it);
        if (err) {
            printf("audio onkey err! %d\n", err);
            break;
        }


        ui_hide(LAYOUT_MN_AUDIO_ON_OFF);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_AUDIO_ON_OFF);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_AUDIO_STATE)
.onchange = menu_audio_onchange,
 .onkey = menu_audio_onkey,
  .ontouch = NULL,
};


/***************************** 音量大小设置 ************************************/
static int menu_audio_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("aud_vol"), TABLE(table_audio_set_camera0));



        printf("\n audio_set=================%d\n ", index);
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_audio_set_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_system";
        it.action = ACTION_SYSTEM_SET_CONFIG;
        it.data = "aud_vol";
        it.exdata = table_audio_set_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_AUDIO_SET);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_AUDIO_SET);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_AUDIO_SET)
.onchange = menu_audio_set_onchange,
 .onkey = menu_audio_set_onkey,
  .ontouch = NULL,
};



/***************************** 格式化设置 ************************************/

static int menu_format_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item = 0;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);


        menu_sys_format_set(sel_item);

        ui_hide(LAYOUT_MN_FORMAT_SYS);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_FORMAT_SYS);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_FORMAT_SYS)
.onkey = menu_format_onkey,
 .ontouch = NULL,
};



/***************************** 默认设置设置 ************************************/

static int menu_default_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        if (sel_item) {
            it.name	= "video_system";
            it.action = ACTION_SYSTEM_SET_CONFIG;
            it.data = "def";
            __sys_msg_show(BOX_MSG_DEFAULTING, 1000);
            start_app_async(&it, NULL, NULL);
        }

        ui_hide(LAYOUT_MN_DEFAULT_SYS);


        int on = db_select("tallow");
        if (!on) {

            // MCU_Timer_OnOff(0, 0, 0,0, 0);
        }

        printf("\n  on===============%d\n", on);

        int sen_on = db_select("sentimer");

        if (!sen_on) {


        }
        printf("\n  sen_on===============%d\n", sen_on);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_DEFAULT_SYS);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_DEFAULT_SYS)
.onkey = menu_default_onkey,
 .ontouch = NULL,
};



/***************************** 版本 ************************************/

static int menu_version_onchange(void *ctr, enum element_change_event e, void *arg)
{

    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        layout_on_focus(layout);
        /*
         * 布局key消息接管
         */
        break;
    case ON_CHANGE_RELEASE:
        layout_lose_focus(layout);
        /*
         * 布局key消息取消接管
         */
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_version_onkey(void *ctr, struct element_key_event *e)
{
    switch (e->value) {
    case KEY_OK:
        ui_hide(LAYOUT_MN_VERSION_SYS);
        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_VERSION_SYS);
        break;
    default:
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(LAYOUT_MN_VERSION_SYS)
.onchange = menu_version_onchange,
 .onkey = menu_version_onkey,
  .ontouch = NULL,
};


/***************************** 版本字符串 ************************************/
static int text_sys_version_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        text->str = "Version:JLV1.0";
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TEXT_VERSION_SYS)
.onchange = text_sys_version_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/***************************** 系统时间各个数字动作 ************************************/
static int timer_sys_year_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("timer_sys_year_onchange .\n");
        time->year = time_set_table[0].time_value;
        time->month = time_set_table[1].time_value;
        time->day = time_set_table[2].time_value;
        time->hour = time_set_table[3].time_value;
        time->min = time_set_table[4].time_value;
        time->sec = time_set_table[5].time_value;
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TIMER_YEAR_SYS)
.onchange = timer_sys_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int timer_sys_month_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        time->year = time_set_table[0].time_value;
        time->month = time_set_table[1].time_value;
        time->day = time_set_table[2].time_value;
        time->hour = time_set_table[3].time_value;
        time->min = time_set_table[4].time_value;
        time->sec = time_set_table[5].time_value;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_MONTH_SYS)
.onchange = timer_sys_month_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int timer_sys_day_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        time->year = time_set_table[0].time_value;
        time->month = time_set_table[1].time_value;
        time->day = time_set_table[2].time_value;
        time->hour = time_set_table[3].time_value;
        time->min = time_set_table[4].time_value;
        time->sec = time_set_table[5].time_value;
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TIMER_DAY_SYS)
.onchange = timer_sys_day_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int timer_sys_hour_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        time->year = time_set_table[0].time_value;
        time->month = time_set_table[1].time_value;
        time->day = time_set_table[2].time_value;
        time->hour = time_set_table[3].time_value;
        time->min = time_set_table[4].time_value;
        time->sec = time_set_table[5].time_value;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_HOUR_SYS)
.onchange = timer_sys_hour_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int timer_sys_min_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        time->year = time_set_table[0].time_value;
        time->month = time_set_table[1].time_value;
        time->day = time_set_table[2].time_value;
        time->hour = time_set_table[3].time_value;
        time->min = time_set_table[4].time_value;
        time->sec = time_set_table[5].time_value;
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TIMER_MINUTE_SYS)
.onchange = timer_sys_min_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int timer_sys_sec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        time->year = time_set_table[0].time_value;
        time->month = time_set_table[1].time_value;
        time->day = time_set_table[2].time_value;
        time->hour = time_set_table[3].time_value;
        time->min = time_set_table[4].time_value;
        time->sec = time_set_table[5].time_value;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_SECOND_SYS)
.onchange = timer_sys_sec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_sys_msg_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, g_msg_id);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TEXT_SYS_MSG)
.onchange = text_sys_msg_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif





