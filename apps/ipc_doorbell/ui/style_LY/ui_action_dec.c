#include "ui/includes.h"
#include "ui/ui_battery.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "video_system.h"


#define _STDIO_H_
#include "xciot_api.h"
#include "xciot_cmd_ipc.h"
#include "doorbell_event.h"


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

void rep_exit_menu_post(void);

#ifdef CONFIG_IPC_UI_ENABLE
void hide_layout_dec_file_pic_txt();
void set_layout_dec_file_txt();

extern u32 longtime_idle_return_timer;              //长时间无操作返回主界面定时器（部分界面）
void longtime_idle_return_time_handler();           //长时间无操作返回主界面服务函数（部分界面）
void update_longtime_idle_return_timer(u8 state);   //更新返回长时间无操作返回定时器
int notify_msg_to_video_rec_async(int action,const char *data,u32 exdata);

void return_ui_to_layer_dec_select();
extern void return_app_to_rec();
struct current_page_info{
    u8 current_page_file_num;       //当前页的文件数
    u8 current_page_now_show_num;   //当前页应该显示的图片
    u8 current_page_last_show_num;  //当前页上一次显示的文件数，用于回退后隐藏信息布局
    u8 current_page_num;            //当前所在页数
    u8 current_page_change_flag;    //页数变化标志位，置一需要做对应变化之后再清零
    u8 current_page_last_num;       //上一次显示所在页数
    u8 current_file_page;           //当前选择的页数，用于进入视频后回退还原选择
    u8 current_file_item;           //当前页选择的文件，用于进入视频后回退还原选择
    u8 current_page_file_name[6][MAX_FILE_NAME_LEN];    //当前页所有文件名
    u8 current_page_txt_info_change_flag;   //txt_info变化标志位，翻页只更新一次
};
struct current_page_info browser_info;
#endif //CONFIG_IPC_UI_ENABLE

#define STYLE_NAME  LY

//u8 select_msg_or_rec = 0;//区分录像和留言视频

struct replay_menu_info {

    u8 if_in_rep;           /* 是否正在播放 */
    s8 ff_fr_flag;          /* 是否正在快进快退, 0 否，1 快进， -1快退 */
    u8 no_file;
    u8 file_type;           /* 文件是JPG还是MOV,0 jpg, 1 mov */
    u8 file_type_index;     /* 文件分辨率的索引号，根据UI */
    u8 ppt;
    u8 is_lock;
    u8 err_file;
    const char *fname;      /* 文件名 */
    struct utime sum_time;  /* 视频的总时间 */
    struct utime cur_time;  /* 视频的当前时间 */

    u8 menu_status;
    u8 init;
    u8 battery_val;
    u8 battery_char;

    char path[128];			// 文件夹路径
    u8 if_in_bro;			// 是否在预览窗口
    u16 page;				// 文件预览页码
    u16 item;				// 页内位置
    FILE *cur_file;			// 当前文件

};

static struct replay_menu_info handler;

#define __this 	(&handler)

/************************************************************
						回放模式
************************************************************/


/*
 * replay的ppt播放间隔
 */
static const char *table_ppt_sec[] = {
    "ppt:2s",
    "ppt:5s",
    "ppt:8s",

};

/*
 *  SD卡状态
 */
enum machine_sd_state   dec_sd_state;

/*
 *  回放模式整体状态
 */
enum machine_dec_state  dec_state;


/*
 * (begin)提示框显示接口
 */
enum box_msg {
    BOX_MSG_NO_POWER = 1,
    BOX_MSG_NO_FILE,
    BOX_MSG_MEM_ERR,
    BOX_MSG_INSERT_SD,
    BOX_MSG_DEL_CUR,
    BOX_MSG_DEFAULT_SET,
    BOX_MSG_FORMATTING,
    BOX_MSG_DEL_ALL,
    BOX_MSG_ERR_FILE,
    BOX_MSG_LOCKED_FILE,
    BOX_MSG_SD_ERR,
    BOX_MSG_10S_SHUTDOWN,
};
static u8 msg_show_f = 0;
static enum box_msg msg_show_id = 0;
static void __dec_msg_hide(enum box_msg id)
{
    struct intent it;
    init_intent(&it);
    printf("\n /****** [%s] %d id = %d, msg_show_id = %d\n",__func__,__LINE__,id,msg_show_id);
    if (msg_show_id == id) {
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(LAYER_MSG_REP);
        }
    } else if (id == 0) {
        /*
         * 没有指定ID，强制隐藏
         */
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(LAYER_MSG_REP);
        }
    }
#ifdef CONFIG_IPC_UI_ENABLE
        if(id == BOX_MSG_NO_FILE ){
            printf("\n /*** BOX_MSG_NO_FILE\n");
            printf("\n /*** dec_sd_state = %d\n ",dec_sd_state);
            if(__this->no_file == 1){
                printf("\n \n /*********** return ui \n ");
                ui_show(LAYER_DEC_SELECT);
//                return_ui_to_layer_dec_select();
            }
//            if(dec_state == DEC_STA_SEL_FILE_FOLDER){
//                ui_hide();
//            }
            if(dec_sd_state == SD_STA_OFFLINE){
                printf("\n \n /*********** return app\n ");
                it.name = "video_dec";
                it.action = ACTION_VIDEO_DEC_CHANGE_REC;
                start_app_async(&it,NULL,NULL);
            }
    }else if(id == BOX_MSG_ERR_FILE){
            printf("\n /*** BOX_MSG_ERR_FILE\n");
            printf("\n /*** dec_sd_state = %d\n ",dec_sd_state);
            ui_show(LAYER_BROWSER);
            ui_show(LAYOUT_DEC_FILE);
            ui_show(LAYOUT_DEC_FILE_TXT);
            ui_show(LAYOUT_BROWSER2);
    }
#endif // CONFIG_IPC_UI_ENABLE
}
static void __dec_msg_timeout_func(void *priv)
{
    __dec_msg_hide((enum box_msg)priv);
}
static void __dec_msg_show(enum box_msg msg, u32 timeout_msec)
{
    printf("\n /***** [%s] msg =%d, msg_show_id =%d\n",__func__,msg,msg_show_id);
    static int t_id = 0;
    if (msg == 0) {
        ASSERT(0, "__dec_msg_show msg 0!\n");
        return;
    }

#ifdef CONFIG_IPC_UI_ENABLE
    if(msg == BOX_MSG_NO_FILE){
        printf("\n /****** msg == BOX_MSG_NO_FILE\n");
        ui_hide(LAYER_BROWSER);
        ui_hide(LAYER_DEC_SELECT);
//        ui_hide(LAYOUT_DEC_FILE);
//        ui_hide(LAYOUT_DEC_FILE_TXT);
//        ui_hide(LAYOUT_BROWSER2);
    }
#endif // CONFIG_IPC_UI_ENABLE

    if (msg == msg_show_id) {
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(LAYER_MSG_REP);
            ui_text_show_index_by_id(TEXT_MSG_REP, msg - 1);
            if (t_id) {
                sys_timeout_del(t_id);
                t_id = 0;
            }
            if (timeout_msec > 0) {
                t_id = sys_timeout_add((void *)msg, __dec_msg_timeout_func, timeout_msec);
            }
        }
    } else {
        msg_show_id = msg;
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(LAYER_MSG_REP);
        }
        ui_text_show_index_by_id(TEXT_MSG_REP, msg - 1);
        if (t_id) {
            sys_timeout_del(t_id);
            t_id = 0;
        }
        if (timeout_msec > 0) {
            t_id = sys_timeout_add((void *)msg, __dec_msg_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */

static int rep_file_name_handler(const char *type, u32 args)
{
    printf("\n /******* [%s]%d\n",__func__,__LINE__);
    const char *fname = (const char *)args;
    const char *str_p;

    /* puts("rep_file_name_handler\n"); */

    if (!strcmp(type, "ascii")) {
        str_p = (const char *)(args + strlen((char *)args) - 3);
        if (!strcmp(str_p, "JPG") || !strcmp(str_p, "jpg")) {
            if (__this->file_type != 0) {
                __this->file_type = 0;

                ui_hide(TIMER_SUM_REP);
                ui_hide(TIMER_RUN_REP);
#ifndef CONFIG_IPC_UI_ENABLE
                ui_hide(PIC_START_REP);
                ui_hide(PIC_SS_REP);
                ui_hide(PIC_PREV_REP);
                ui_hide(PIC_NEXT_REP);
                ui_pic_show_image_by_id(PIC_LOGO_REP, 0);
#endif // CONFIG_IPC_UI_ENABLE
            }
        } else {
            if (__this->file_type != 1) {
                __this->file_type = 1;
            } else {

            }
            ui_hide(TIMER_RUN_REP);
            /* ui_show(TIMER_SUM_REP); */
#ifndef CONFIG_IPC_UI_ENABLE
            ui_pic_show_image_by_id(PIC_START_REP, 1);
            ui_show(PIC_SS_REP);
            ui_show(PIC_PREV_REP);
            ui_show(PIC_NEXT_REP);
            ui_pic_show_image_by_id(PIC_LOGO_REP, 1);
            ui_pic_show_image_by_id(PIC_PREV_REP, 0);
            ui_pic_show_image_by_id(PIC_NEXT_REP, 0);
#endif
        }
        ui_hide(PIC_FF_NUM_REP);
        ui_hide(PIC_FR_NUM_REP);
        __this->ff_fr_flag = 0;
        __this->if_in_rep = 0;
        __this->no_file = 0;
        __this->err_file = 0;
        __this->fname = (const char *)args;
        printf("\n /******* [%s]%d\n",__func__,__LINE__);
//        ui_text_set_str_by_id(TEXT_FNAME_REP, "ascii", (const char *)args);
        ui_text_set_textu_by_id(TEXT_FNAME_REP,__this->fname,strlen(__this->fname), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
        ui_show(TEXT_FNAME_REP);
    } else if (!strcmp(type, "utf16")) {

    }
#ifdef CONFIG_IPC_UI_ENABLE
    else if (!strcmp(type, "utf8")) {
                str_p = (const char *)(args + strlen((char *)args) - 3);
        if (!strcmp(str_p, "JPG") || !strcmp(str_p, "jpg")) {
            if (__this->file_type != 0) {
                __this->file_type = 0;
                ui_hide(TIMER_SUM_REP);
                ui_hide(TIMER_RUN_REP);
            }
        } else {
            if (__this->file_type != 1) {
                __this->file_type = 1;
            } else {

            }
            ui_hide(TIMER_RUN_REP);
            /* ui_show(TIMER_SUM_REP); */
        }
        ui_hide(PIC_FF_NUM_REP);
        ui_hide(PIC_FR_NUM_REP);
        __this->ff_fr_flag = 0;
        __this->if_in_rep = 0;
        __this->no_file = 0;
        __this->err_file = 0;
        __this->fname = (const char *)args;
        printf("\n /******* [%s]%d\n",__func__,__LINE__);
//        ui_text_set_str_by_id(TEXT_FNAME_REP, "ascii", (const char *)args);
        ui_text_set_textu_by_id(TEXT_FNAME_REP,__this->fname,strlen(__this->fname), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
        ui_show(TEXT_FNAME_REP);
    }
#endif // CONFIG_IPC_UI_ENABLE

    __dec_msg_hide(0);

    return 0;
}

static int rep_file_res_handler(const char *type, u32 args)
{
#ifndef CONFIG_IPC_UI_ENABLE
    if (*type == 'w') {
        /* printf("rep_file_res_handler w: %d.\n", args); */
        if (__this->file_type == 1) {
            if (args >= 1920) {
                /*
                 * 1080p
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 0);
                __this->file_type_index = 0;
            } else if (args >= 1080) {
                /*
                 * 720p
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 1);
                __this->file_type_index = 1;
            } else {
                /*
                 * vga
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 2);
                __this->file_type_index = 2;
            }
        } else if (__this->file_type == 0) {
            if (args >= 4032) {
                /*
                 * 12M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 10);
                __this->file_type_index = 10;
            } else if (args >= 3648) {
                /*
                 * 10M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 9);
                __this->file_type_index = 9;
            } else if (args >= 3264) {
                /*
                 * 8M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 8);
                __this->file_type_index = 8;
            } else if (args >= 2592) {
                /*
                 * 5M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 7);
                __this->file_type_index = 7;
            } else if (args >= 2048) {
                /*
                 * 3M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 6);
                __this->file_type_index = 6;
            } else if (args >= 1920) {
                /*
                 * 2M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 5);
                __this->file_type_index = 5;
            } else if (args >= 1280) {
                /*
                 * 1.3M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 4);
                __this->file_type_index = 4;
            } else {
                /*
                 * vga
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 3);
                __this->file_type_index = 3;
            }
        }
    }
#endif // CONFIG_IPC_UI_ENABLE
    return 0;
}

static int rep_film_length_handler(const char *type, u32 args)
{
    int sec;
    struct utime t;

    if (*type == 's') {
        sec = args;
    } else {
        return 0;
    }

    if (__this->file_type == 1) {
        if (sec) {
            t.sec = sec % 60;
            t.min = sec / 60 % 60;
            t.hour = sec / 60 / 60;
            __this->sum_time.sec = t.sec;
            __this->sum_time.min = t.min;
            __this->sum_time.hour = t.hour;
#ifndef CONFIG_IPC_UI_ENABLE
            ui_time_update_by_id(TIMER_SUM_REP, &t);
            ui_show(TIMER_SUM_REP);
#endif // CONFIG_IPC_UI_ENABLE
        } else {
            t.sec = 1;
            t.min = 0;
            t.hour = 0;
            __this->sum_time.sec = 1;
            __this->sum_time.min = 0;
            __this->sum_time.hour = 0;
#ifndef CONFIG_IPC_UI_ENABLE
            ui_time_update_by_id(TIMER_SUM_REP, &t);
#endif // CONFIG_IPC_UI_ENABLE
        }
    }
    /* printf("rep_film_length_handler: %d.\n", args); */

    return 0;
}

static int rep_current_time_handler(const char *type, u32 args)
{
    int sec;
    struct utime t;

    if (*type == 's') {
        sec = args;
    } else {
        return 0;
    }

    t.sec = sec % 60;
    t.min = sec / 60 % 60;
    t.hour = sec / 60 / 60;

    __this->cur_time.sec = t.sec;
    __this->cur_time.min = t.min;
    __this->cur_time.hour = t.hour;
    ui_time_update_by_id(TIMER_RUN_REP, &t);
    /* printf("rep_curtime: %d.\n", args); */

    return 0;
}
static int rep_no_file_handler(const char *type, u32 args)
{
    struct intent it;

    if (__this->ppt) {
        /*
         * 退出幻灯片浏览模式
         */
        __this->ppt = 0;

        init_intent(&it);
        it.name	= "video_dec";
        it.data = "ppt:0s";
        it.action = ACTION_VIDEO_DEC_SET_CONFIG;
        start_app_async(&it, NULL, NULL);
    }

    __this->file_type = 0xff;
    __this->no_file = 1;
    if (__this->menu_status == 0) {
        ui_hide(PIC_LOCK_REP);
        ui_hide(TIMER_SUM_REP);
        ui_hide(TIMER_RUN_REP);
        ui_hide(TEXT_RES_REP);
        ui_hide(TEXT_FNAME_REP);
        ui_hide(PIC_START_REP);
        ui_hide(PIC_SS_REP);
        ui_hide(PIC_PREV_REP);
        ui_hide(PIC_NEXT_REP);
        ui_hide(PIC_FF_NUM_REP);
        ui_hide(PIC_FR_NUM_REP);
		printf("/***** [%s] %d __dec_msg_show",__func__,__LINE__);
        __dec_msg_show(BOX_MSG_NO_FILE, 0);
    } else {
        if (__this->menu_status == 1) {
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
        } else if (__this->menu_status == 2) {
            sys_exit_menu_post();
        }
        __this->menu_status = 0;
    }
    return 0;
}

static int rep_file_err_handler(const char *type, u32 args)
{
    /* puts("rep_file_err_handler \n"); */

    if (__this->menu_status == 0) {
        ui_hide(PIC_LOCK_REP);
        ui_hide(TIMER_SUM_REP);
        ui_hide(TIMER_RUN_REP);
        ui_hide(TEXT_RES_REP);
        ui_hide(TEXT_FNAME_REP);
        ui_hide(PIC_START_REP);
        ui_hide(PIC_SS_REP);
        ui_hide(PIC_PREV_REP);
        ui_hide(PIC_NEXT_REP);
        ui_hide(PIC_FF_NUM_REP);
        ui_hide(PIC_FR_NUM_REP);
		printf("/***** [%s] %d __dec_msg_show",__func__,__LINE__);
#ifdef CONFIG_IPC_UI_ENABLE
        __dec_msg_show(BOX_MSG_ERR_FILE, 1000);
#else
        __dec_msg_show(BOX_MSG_ERR_FILE, 0);
#endif // CONFIG_IPC_UI_ENABLE
    } else {

    }
    __this->err_file = 1;
    return 0;
}


static int rep_file_attribute_handler(const char *type, u32 read_only)
{
    /* puts("rep_file_attribute_handler \n"); */
    if (read_only) {
        __this->is_lock = 1;
#ifndef CONFIG_IPC_UI_ENABLE
        ui_show(PIC_LOCK_REP);
#endif // CONFIG_IPC_UI_ENABLE
    } else {
        __this->is_lock = 0;
        ui_hide(PIC_LOCK_REP);
    }

    return 0;
}

static int rep_play_handler(const char *type, u32 args)
{
    /* puts("rep_play!\n"); */
    __this->if_in_rep = 1;
    if (__this->ff_fr_flag != 0) {
#ifndef CONFIG_IPC_UI_ENABLE
        if (__this->ff_fr_flag >= 1) {

            ui_pic_show_image_by_id(PIC_NEXT_REP, 0);
            ui_hide(PIC_FF_NUM_REP);

        } else if (__this->ff_fr_flag <= -1) {

            ui_pic_show_image_by_id(PIC_PREV_REP, 0);
            ui_hide(PIC_FR_NUM_REP);
        }
#endif // CONFIG_IPC_UI_ENABLE
        __this->ff_fr_flag = 0;
    } else {
#ifndef CONFIG_IPC_UI_ENABLE
        ui_pic_show_image_by_id(PIC_START_REP, 0);
#endif // CONFIG_IPC_UI_ENABLE
        ui_hide(TIMER_SUM_REP);
        ui_time_update_by_id(TIMER_RUN_REP, &__this->cur_time);
        ui_show(TIMER_RUN_REP);
    }


    return 0;
}

static int rep_pause_handler(const char *type, u32 args)
{
    /* puts("rep_pause!\n"); */
    __this->if_in_rep = 0;
#ifndef CONFIG_IPC_UI_ENABLE
    ui_pic_show_image_by_id(PIC_START_REP, 1);
#endif // CONFIG_IPC_UI_ENABLE
    return 0;
}

static int rep_ff_handler(const char *type, u32 args)
{
    /* printf("rep_ff: %d\n", args); */
#ifndef CONFIG_IPC_UI_ENABLE
    ui_pic_show_image_by_id(PIC_NEXT_REP, 1);
    if (__this->ff_fr_flag <= -1) {
        ui_pic_show_image_by_id(PIC_PREV_REP, 0);
        ui_hide(PIC_FR_NUM_REP);
    }
    __this->ff_fr_flag = 1;
    switch (args) {
    case 2:
        __this->ff_fr_flag = 1;
        ui_pic_show_image_by_id(PIC_FF_NUM_REP, 0);
        break;
    case 4:
        __this->ff_fr_flag = 2;
        ui_pic_show_image_by_id(PIC_FF_NUM_REP, 1);
        break;
    case 8:
        __this->ff_fr_flag = 3;
        ui_pic_show_image_by_id(PIC_FF_NUM_REP, 2);
        break;
    }
#else
    __this->ff_fr_flag = 1;
    switch (args) {
    case 2:
        __this->ff_fr_flag = 1;
        break;
    case 4:
        __this->ff_fr_flag = 2;
        break;
    case 8:
        __this->ff_fr_flag = 3;
        break;
    }
#endif // CONFIG_IPC_UI_ENABLE
    return 0;
}

static int rep_fr_handler(const char *type, u32 args)
{
    /* printf("rep_fr: %d\n", args); */
#ifndef CONFIG_IPC_UI_ENABLE
    ui_pic_show_image_by_id(PIC_PREV_REP, 1);
    if (__this->ff_fr_flag >= 1) {
        ui_pic_show_image_by_id(PIC_NEXT_REP, 0);
        ui_hide(PIC_FF_NUM_REP);
    }
    __this->ff_fr_flag = -1;
    switch (args) {
    case 2:
        __this->ff_fr_flag = -1;
        ui_pic_show_image_by_id(PIC_FR_NUM_REP, 0);
        break;
    case 4:
        __this->ff_fr_flag = -2;
        ui_pic_show_image_by_id(PIC_FR_NUM_REP, 1);
        break;
    case 8:
        __this->ff_fr_flag = -3;
        ui_pic_show_image_by_id(PIC_FR_NUM_REP, 2);
        break;
    }
#else
    __this->ff_fr_flag = -1;
    switch (args) {
    case 2:
        __this->ff_fr_flag = -1;
        break;
    case 4:
        __this->ff_fr_flag = -2;
        break;
    case 8:
        __this->ff_fr_flag = -3;
        break;
    }
#endif // CONFIG_IPC_UI_ENABLE
    return 0;
}

#ifdef CONFIG_IPC_UI_ENABLE
static int rep_end_handler(const char *type, u32 args)
{
    struct intent it;
    puts("\n/****** rep_end!\n");
    __this->if_in_rep = 0;
    if (__this->err_file) {
        __dec_msg_hide(BOX_MSG_ERR_FILE);
        __this->err_file = 0;
    }
    __this->cur_time.hour = 0;
    __this->cur_time.min = 0;
    __this->cur_time.sec = 0;

    ui_hide(LAYER_UP_REP);
    init_intent(&it);
    it.name = "video_dec";
    it.action = ACTION_VIDEO_DEC_CUR_PAGE;
    start_app(&it);
    ui_show(LAYER_BROWSER);
    ui_show(LAYOUT_DEC_FILE);
    ui_show(LAYOUT_DEC_FILE_TXT);
    ui_show(LAYOUT_BROWSER2);

    return 0;
}
#endif
/*
 * 回放模式的APP状态响应回调
 */
static const struct uimsg_handl rep_msg_handler[] = {
    { "fname",     rep_file_name_handler     },
    { "res",       rep_file_res_handler      },
    { "filmLen",   rep_film_length_handler   },
    { "plyTime",   rep_current_time_handler  },
    { "fattr",     rep_file_attribute_handler},
    { "noFile",    rep_no_file_handler       },
    { "fileErr",   rep_file_err_handler      },
    { "play",      rep_play_handler          },
    { "pause",     rep_pause_handler         },
    { "ff",        rep_ff_handler            },
    { "fr",        rep_fr_handler            },
#ifdef CONFIG_IPC_UI_ENABLE
    { "end",        rep_end_handler          },
#endif
    { NULL, NULL},
};



static void rep_tell_app_exit_menu(void)
{
    struct intent it;

    init_intent(&it);

    it.name	= "video_dec";
    it.action = ACTION_VIDEO_DEC_CHANGE_STATUS;
    it.data = "exitMenu";
    start_app(&it);
}

static int rep_ask_app_open_menu(void)
{
    int err;
    struct intent it;

    init_intent(&it);
    it.name	= "video_dec";
    it.action = ACTION_VIDEO_DEC_CHANGE_STATUS;
    it.data = "openMenu";
    err = start_app(&it);
    if (err) {
        return -1;
    }
    if (!strcmp(it.data, "dis")) {
        return -1;
    }

    return 0;
}

void rep_exit_menu_post(void)
{
    ui_hide(LAYER_MENU_REP);
#ifdef CONFIG_FILE_PREVIEW_ENABLE
    if (__this->if_in_bro) {
        ui_show(LAYER_BROWSER);
    } else {
        ui_show(LAYER_UP_REP);
    }
#else
    ui_show(LAYER_UP_REP);
#endif
    rep_tell_app_exit_menu();
    __this->menu_status = 0;
}

static void video_dec_ui_event_handler(struct sys_event *event, void *priv)
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
    }else if (!ASCII_StrCmp(event->arg, "sd*", 4)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            printf("\n /********%s %d DEVICE_EVENT_IN\n",__func__,__LINE__);
            dec_sd_state = SD_STA_ONLINE;
            break;
        case DEVICE_EVENT_OUT:
            printf("\n /********%s %d DEVICE_EVENT_OUT\n",__func__,__LINE__);
#ifdef CONFIG_IPC_UI_ENABLE
            update_longtime_idle_return_timer(2);//0开启 1更新 2关闭
 #endif
            dec_sd_state = SD_STA_OFFLINE;
            __dec_msg_show(BOX_MSG_NO_FILE, 1500);
            break;
        }
    }
}

/*****************************回放模式页面回调 ************************************/
static int replay_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    static int id = 0;

    switch (e) {
    case ON_CHANGE_INIT:

        puts("\n***dec mode onchange init***\n");
        memset(__this, 0, sizeof(struct replay_menu_info));

        __this->file_type = 0xff;
        ui_register_msg_handler(ID_WINDOW_VIDEO_REP, rep_msg_handler);
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, video_dec_ui_event_handler);
        sys_cur_mod = 3;
        break;
    case ON_CHANGE_FIRST_SHOW:
#ifdef CONFIG_FILE_PREVIEW_ENABLE
#ifdef CONFIG_IPC_UI_ENABLE
//        strcpy(__this->path, CONFIG_DEC_ROOT_PATH);
        if (storage_device_ready() == 0) {
            /*
             * 需要判断一下,SD不在线
             */
            printf("/****** [%s] %d ON_CHANGE_FIRST_SHOW __dec_msg_show",__func__,__LINE__);
            dec_sd_state = SD_STA_OFFLINE;
            __dec_msg_show(BOX_MSG_NO_FILE, 1500);
        } else {
            /*
             * SD卡在线
             */
            __this->if_in_bro = 1;
            dec_sd_state = SD_STA_ONLINE;
            ui_show(LAYER_DEC_SELECT);
            extern int dec_pro_file(char *path,int flag);
            dec_pro_file(NULL,0);
        }

//        ui_show(LAYER_DEC_SELECT);
//        ui_show(LAYER_EVNET_DEC);

#endif // CONFIG_IPC_UI_ENABLE

#else
        __this->if_in_bro = 0;
        ui_show(LAYER_UP_REP);
#endif
        break;
    case ON_CHANGE_RELEASE:
        __dec_msg_hide(0);
        ui_hide(ID_WINDOW_VIDEO_SYS);
        /*
         * 要隐藏一下系统菜单页面，防止在系统菜单插入USB进入USB页面
         */
        sys_key_event_takeover(false, false);
        unregister_sys_event_handler(id);
        id = 0;
        break;

        break;
    default:
        return false;
    }
    return false;
}

static int replay_mode_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n/**** [%s] %d\n",__func__,__LINE__);
    struct intent it;

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            if (__this->ppt) {
                /*
                 * 在幻灯片浏览模式，按menu键退出
                 */
                __this->ppt = 0;

                init_intent(&it);
                it.name	= "video_dec";
                it.data = "ppt:0s";
                it.action = ACTION_VIDEO_DEC_SET_CONFIG;
                start_app_async(&it, NULL, NULL);

                ui_show(LAYER_UP_REP);
                sys_key_event_takeover(false, false);
            } else {
                if (rep_ask_app_open_menu()) {
                    break;
                }
                __dec_msg_hide(0);
#ifdef CONFIG_FILE_PREVIEW_ENABLE
                ui_hide(LAYER_BROWSER);
#else
                ui_hide(LAYER_UP_REP);
#endif
                ui_show(LAYER_MENU_REP);
                ui_show(LAYOUT_MN_REP_REP);
                printf("\n [%s] %d takeover true\n",__func__,__LINE__);
                sys_key_event_takeover(true, false);
            }

            __this->menu_status = 1;
            break;
        default:
            return false;
        }
        break;
    default:
        if (__this->menu_status) {
            if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
                rep_exit_menu_post();
                printf("\n [%s] %d takeover true\n",__func__,__LINE__);
                sys_key_event_takeover(false, true);
                /* puts("\n take off msg power rep\n"); */
                return false;
            }
        }
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_REP)
.onchange = replay_mode_onchange,
#ifndef CONFIG_FILE_PREVIEW_ENABLE
 .onkey = replay_mode_onkey,
#endif
};

#ifdef CONFIG_FILE_PREVIEW_ENABLE
/*********************************************************************************
 *  		     				预览框回调
 *********************************************************************************/
static int browser_open_file(int p)
{
    struct intent it;
    FILE *fp = (FILE *)p;
    if (fp) {
        init_intent(&it);
        it.name = "video_dec";
        it.action = ACTION_VIDEO_DEC_OPEN_FILE;
        it.data = (const char *)fp;
        it.exdata = (u32)__this->path;
        /* start_app_async(&it, NULL, NULL); */
        start_app(&it);
    }
    return 0;
}

static int browser_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_browser *bro = ctr;
    printf("/******* [%s] e=%d\n",__func__,e);

    switch (e) {
    case ON_CHANGE_INIT:
        printf("/******* [%s] init\n",__func__);
        bro->order = 1;
        bro->path = __this->path;
        /* __this->browser->path = cPATH[__this->dir]; */
#ifdef CONFIG_EMR_DIR_ENABLE
        if (strcmp(__this->path, CONFIG_DEC_PATH_1) == 0
            || strcmp(__this->path, CONFIG_DEC_PATH_2) == 0
            || strcmp(__this->path, CONFIG_DEC_PATH_3) == 0) {
            log_d("dec_path");
            bro->ftype = "-tMOVJPGAVI -st -r";
        } else {
            bro->ftype = "-tMOVJPGAVI -st -d";
        }
#else
//    printf("/***[%s] select_msg_or_rec = %d\n",__func__,select_msg_or_rec);
    bro->ftype = "-tMOVJPGAVI -st -d";
//    if(select_msg_or_rec){
//        bro->ftype = "-tMOVJPGAVI -st -d";
//    }else{
//        bro->ftype = "-tMOVJPGAVI -st -ar -d";
//    }

#endif
        bro->show_mode = 1;

        if (__this->cur_file) {
            fclose(__this->cur_file);
            __this->cur_file = 0;
        }
        if (__this->page) {
            ui_file_browser_set_page(bro, __this->page);
        }
        if (__this->item) {
            ui_file_browser_highlight_item(bro, 0, false);
            ui_file_browser_highlight_item(bro, __this->item, true);
        }
        break;
    case ON_CHANGE_FIRST_SHOW:
        __this->if_in_bro = 1;
        printf("\n [%s] %d takeover true\n",__func__,__LINE__);
        sys_key_event_takeover(true, false);
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        if (!ui_file_browser_page_num(bro)) {
            __this->no_file = 1;
            printf("/****** [%s] %d INIT __dec_msg_show\n",__func__,__LINE__);
            __dec_msg_show(BOX_MSG_NO_FILE, 0);
        } else {
            __this->no_file = 0;
            __dec_msg_hide(BOX_MSG_NO_FILE);
        }
        break;
    default:
        break;
    }
    return false;
}
static int browser_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n [%s] %d",__func__,__LINE__);
    struct ui_browser *_browser = ctr;
    struct ui_file_attrs attrs;
    int _file_num = 0;
    __this->page = ui_file_browser_cur_page(_browser, &_file_num);
    __this->item = ui_file_browser_cur_item(_browser);

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MODE:
            //返回上级目录
            if (strcmp(__this->path, CONFIG_DEC_ROOT_PATH)) {
                __this->if_in_bro = 1;
                int len = strlen(__this->path);
                len--;
                while (--len) {
                    if (__this->path[len] != '/') {
                        __this->path[len] = 0;
                    } else {
                        break;
                    }
                }
                printf("path = %s\n", __this->path);

//                ui_hide(LAYER_BROWSER);
//                ui_show(LAYER_DEC_SELECT);
                ui_text_set_str_by_id(TEXT_PATH_BROWSER, "ascii", __this->path + sizeof(CONFIG_ROOT_PATH) - 1);
                ui_file_browser_set_dir_by_id(BROWSER, __this->path, "-tMOVJPGAVI -st -d");
                return true;
            } else {
                log_d("root ptah");
                printf("\n [%s] %d takeover true\n",__func__,__LINE__);
                sys_key_event_takeover(false, true);
                sys_key_event_takeover(false, false);
                __this->if_in_bro = 0;
            }
            break;
        case KEY_OK:
            if (_file_num == 0) {
                log_d("no file\n");
                return true;
            }
            printf("item=%d \n", __this->item);
            ui_file_browser_get_file_attrs(_browser, __this->item, &attrs); //获取文件属性
            if (attrs.ftype == UI_FTYPE_DIR) {
                //进入文件夹
                if (__this->path == 0) {
                    log_e("DIR ERR\n");
                    return false;
                }
                printf("/****** [%s] %d\n",__func__,__LINE__);
                printf("/***** attrs.fname = %s\n",attrs.fname);
                printf("/***** __this->path = %s\n",__this->path);
                strcat(__this->path, attrs.fname);
                int len = strlen(__this->path);
                __this->path[len] = '/';
                __this->path[len + 1] = '\0';

                printf("path=%s\n", __this->path);
#ifdef CONFIG_EMR_DIR_ENABLE
                if (strcmp(__this->path, CONFIG_DEC_PATH_1) == 0
                    || strcmp(__this->path, CONFIG_DEC_PATH_2) == 0
                    || strcmp(__this->path, CONFIG_DEC_PATH_3) == 0) {
                    log_d("dec_path");
                    ui_file_browser_set_dir_by_id(BROWSER, __this->path, "-tMOVJPGAVI -st -r");
                } else {
                    ui_file_browser_set_dir_by_id(BROWSER, __this->path, "-tMOVJPGAVI -st -d");
                }
#else
//                printf("/********* select_msg_or_rec = %d",select_msg_or_rec);
                ui_file_browser_set_dir_by_id(BROWSER, __this->path, "-tMOVJPGAVI -st -d");
//                if(select_msg_or_rec){
//                    ui_file_browser_set_dir_by_id(BROWSER, __this->path, "-tMOVJPGAVI -st -d");
//                }else{
//                    ui_file_browser_set_dir_by_id(BROWSER, __this->path, "-tMOVJPGAVI -st -ar -d");
//                }

#endif

                ui_text_set_str_by_id(TEXT_PATH_BROWSER, "ascii", __this->path + sizeof(CONFIG_ROOT_PATH) - 1);
                __this->if_in_bro = 1;
                return true;
            } else if (attrs.ftype == UI_FTYPE_IMAGE || attrs.ftype == UI_FTYPE_VIDEO) {
                //播放文件
                printf("path=%s, num=%d\n", __this->path, attrs.file_num);
                if (attrs.attr.attr & F_ATTR_RO) {
                    __this->is_lock = 1;
                } else {
                    __this->is_lock = 0;
                }
                FILE *fp = ui_file_browser_open_file(_browser, __this->item);
                __this->sum_time.sec = 0;
                __this->sum_time.min = 0;
                __this->sum_time.hour = 0;
                ui_hide(LAYER_BROWSER);
                ui_show(LAYER_UP_REP);
                ui_set_call(browser_open_file, (int)fp);
                __this->if_in_bro = 0;
                return true;
            }
            break;
        case KEY_MENU:
            if (_file_num == 0) {
                log_d("no file\n");
                return false;
            }
            ui_file_browser_get_file_attrs(_browser, __this->item, &attrs); //获取文件属性
            if (attrs.ftype == UI_FTYPE_IMAGE || attrs.ftype == UI_FTYPE_VIDEO) {
                __dec_msg_hide(0);
                __this->cur_file = ui_file_browser_open_file(_browser, __this->item);
                /* ui_hide(LAYER_UP_REP); */
                ui_hide(LAYER_BROWSER);
                ui_show(LAYER_MENU_REP);
                ui_show(LAYOUT_MN_REP_REP);
                __this->menu_status = 1;
                return true;
            }
            break;
        }
    default:
        if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
            printf("\n [%s] %d takeover true\n",__func__,__LINE__);
            sys_key_event_takeover(false, true);
            /* puts("\n take off msg power rep\n"); */
            return false;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(BROWSER)
.onchange = browser_onchange,
 .onkey = browser_onkey,
};


/*********************************************************************************
 *  		     				回放回调
 *********************************************************************************/
static int replay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        __this->if_in_bro = 0;
        printf("\n [%s] %d takeover true\n",__func__,__LINE__);
        sys_key_event_takeover(true, false);
//        dec_state = DEC_STA_PLAYBACK;//播放文件状态
        break;
    default:
        break;
    }
    return false;

}
static int replay_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n [%s] %d\n",__func__,__LINE__);
    struct intent it;

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MODE:
            if (__this->err_file) {
                __dec_msg_hide(BOX_MSG_ERR_FILE);
                __this->err_file = 0;
            }
            printf("\n [%s] %d\n",__func__,__LINE__);
            __this->cur_time.hour = 0;
            __this->cur_time.min = 0;
            __this->cur_time.sec = 0;

            ui_hide(LAYER_UP_REP);
            printf("\n [%s] %d\n",__func__,__LINE__);
            init_intent(&it);
            it.name = "video_dec";
            it.action = ACTION_VIDEO_DEC_CUR_PAGE;
            start_app(&it);
            printf("\n [%s] %d\n",__func__,__LINE__);
            ui_show(LAYER_BROWSER);
            printf("\n [%s] %d\n",__func__,__LINE__);
#ifdef CONFIG_IPC_UI_ENABLE
            ui_show(LAYOUT_DEC_FILE);
            ui_show(LAYOUT_DEC_FILE_TXT);
            ui_show(LAYOUT_BROWSER2);
            printf("\n [%s] %d\n",__func__,__LINE__);
#endif
            return true;
        case KEY_OK:
            printf("\n [%s] %d\n",__func__,__LINE__);
            init_intent(&it);
            it.name = "video_dec";
            it.action = ACTION_VIDEO_DEC_CONTROL;
            start_app(&it);
            return true;
        case KEY_UP:
        case KEY_DOWN:
            if (__this->if_in_rep) {
                printf("\n [%s] %d takeover true\n",__func__,__LINE__);
                sys_key_event_takeover(false, true);
            }
            return true;
        case KEY_MENU:
            __dec_msg_hide(0);
            ui_hide(LAYER_UP_REP);
            /* ui_hide(LAYER_BROWSER); */
            ui_show(LAYER_MENU_REP);
            ui_show(LAYOUT_MN_REP_REP);
            __this->menu_status = 1;
            return true;
        default:
            return false;
        }
        break;
    default:
        if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
            printf("\n [%s] %d takeover true\n",__func__,__LINE__);
            sys_key_event_takeover(false, true);
            /* puts("\n take off msg power rep\n"); */
            return false;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYER_UP_REP)
.onchange = replay_onchange,
 .onkey = replay_onkey,
};


#endif

/*********************************************************************************
 *  		     				菜单动作
 *********************************************************************************/


static const int menu_rep_list_items[] = {
    LAYOUT_MN_DEL_REP,
    LAYOUT_MN_LOCK_REP,
    LAYOUT_MN_PPT_REP,
};
/***************************** 回放模式菜单1 ************************************/
static int menu_rep_rep_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n [%s] %d",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item = 0;
    /* puts("\n ===menu_rep_rep_onkey===\n"); */
    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);
            ASSERT(sel_item < (sizeof(menu_rep_list_items) / sizeof(int)));
            ui_show(menu_rep_list_items[sel_item]);
            break;
        case KEY_DOWN:
            return FALSE;
            break;
        case KEY_UP:
            return FALSE;
            break;
        case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
            break;
#endif
        case KEY_MENU:
            ui_hide(LAYER_MENU_REP);
            ui_show(ID_WINDOW_VIDEO_SYS);
            __this->menu_status = 2;
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_REP_REP)
.onkey = menu_rep_rep_onkey,
};


/***************************** 删除选择菜单 ************************************/
static const int menu_del_list_items[] = {
    LAYOUT_MN_DEL_CUR_REP,
    LAYOUT_MN_DEL_ALL_REP,
};

static int menu_del_rep_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n [%s] %d",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item = 0;

    switch (e->event) {
    case ELM_EVENT_KEY_CLICK:
        switch (e->value) {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);
            ASSERT(sel_item < (sizeof(menu_del_list_items) / sizeof(int)));
            ui_show(menu_del_list_items[sel_item]);
            break;
        case KEY_DOWN:
            return FALSE;
            break;
        case KEY_UP:
            return FALSE;
            break;
        case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
            break;
#endif
        case KEY_MENU:
            ui_hide(LAYOUT_MN_DEL_REP);
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_DEL_REP)
.onkey = menu_del_rep_onkey,
};


/***************************** 锁文件菜单 ************************************/
static int menu_lock_rep_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n [%s] %d",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item = 0;
    struct intent it;
    int attrs;
    int err;
    char move_path[128];

    init_intent(&it);

    switch (e->event) {
    case ELM_EVENT_KEY_CLICK:
        switch (e->value) {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);
            if (sel_item == 0) {
#ifdef CONFIG_FILE_PREVIEW_ENABLE
                if (__this->if_in_bro) {
                    fget_attr(__this->cur_file, &attrs);
                    attrs |= F_ATTR_RO;
                    fset_attr(__this->cur_file, attrs);
#ifdef CONFIG_EMR_DIR_ENABLE
                    strcpy(move_path, __this->path);
                    strcat(move_path, CONFIG_EMR_REC_DIR);
                    printf("move to %s\n", move_path + sizeof(CONFIG_ROOT_PATH) - 1);
                    err = fmove(__this->cur_file, move_path + sizeof(CONFIG_ROOT_PATH) - 1, NULL, 0);
                    if (err) {
                        log_e("move err");
                        fclose(__this->cur_file);
                    }
#else
                    fclose(__this->cur_file);
#endif
                    __this->cur_file = 0;
                    rep_exit_menu_post();
                    break;
                } else {
                    it.data = "lock:cur";
                }
#else
                it.data = "lock:cur";
#endif
            } else if (sel_item == 1) {
#ifdef CONFIG_FILE_PREVIEW_ENABLE
                if (__this->if_in_bro) {
                    fget_attr(__this->cur_file, &attrs);
                    attrs &= ~F_ATTR_RO;
                    fset_attr(__this->cur_file, attrs);
#ifdef CONFIG_EMR_DIR_ENABLE
                    printf("move to %s\n", __this->path + sizeof(CONFIG_ROOT_PATH) - 1);
                    err = fmove(__this->cur_file, __this->path + sizeof(CONFIG_ROOT_PATH) - 1, NULL, 0);
                    if (err) {
                        log_e("move err");
                        fclose(__this->cur_file);
                    }
#else
                    fclose(__this->cur_file);
#endif
                    __this->cur_file = 0;
                    rep_exit_menu_post();
                    break;
                } else {
                    it.data = "unlock:cur";
                }
#else
                it.data = "unlock:cur";
#endif
            } else if (sel_item == 2) {
                it.data = "lock:all";
#ifdef CONFIG_FILE_PREVIEW_ENABLE
                it.exdata = (int)__this->path;
#endif
                __this->is_lock = 1;
            } else if (sel_item == 3) {
                it.data = "unlock:all";
#ifdef CONFIG_FILE_PREVIEW_ENABLE
                it.exdata = (int)__this->path;
#endif
                __this->is_lock = 0;
            }
            it.name	= "video_dec";
            it.action = ACTION_VIDEO_DEC_SET_CONFIG;
            start_app(&it);
            rep_exit_menu_post();
            ui_hide(LAYOUT_MN_LOCK_REP);
            sys_key_event_takeover(false, false);
            break;
        case KEY_DOWN:
            return FALSE;
            break;
        case KEY_UP:
            return FALSE;
            break;
        case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
            break;
#endif
        case KEY_MENU:
            ui_hide(LAYOUT_MN_LOCK_REP);
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_LOCK_REP)
.onkey = menu_lock_rep_onkey,
};


/***************************** ppt菜单 ************************************/
static int menu_ppt_rep_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n [%s] %d",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item = 0;
    struct intent it;
    int err;

    switch (e->event) {
    case ELM_EVENT_KEY_CLICK:
        switch (e->value) {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);

            ASSERT(sel_item < ARRAY_SIZE(table_ppt_sec));

            init_intent(&it);
            it.name	= "video_dec";
            it.action = ACTION_VIDEO_DEC_SET_CONFIG;
            it.data = table_ppt_sec[sel_item];
            err = start_app(&it);
            if (err) {
                break;
            }
            ui_hide(LAYER_MENU_REP);
            ui_hide(LAYER_UP_REP);
            __this->ppt = true;
            break;
        case KEY_DOWN:
            return FALSE;
            break;
        case KEY_UP:
            return FALSE;
            break;
        case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
            break;
#endif
        case KEY_MENU:
            ui_hide(LAYOUT_MN_PPT_REP);
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_PPT_REP)
.onkey = menu_ppt_rep_onkey,
};


void del_file_callback(void *priv, int err)
{
    /* printf("del_file_callback: err=%d\n", err); */
    if (err == (int) - EPERM) {
        puts("del file -EPERM, locked file.\n");
        __dec_msg_show(BOX_MSG_LOCKED_FILE, 2000);
    } else {
#ifdef CONFIG_FILE_PREVIEW_ENABLE
        __this->if_in_rep = 0;
        __this->cur_time.hour = 0;
        __this->cur_time.min = 0;
        __this->cur_time.sec = 0;
        ui_hide(LAYER_MENU_REP);
        ui_show(LAYER_BROWSER);
        __this->menu_status = 0;
#else
        if (__this->no_file == 1) {
            __dec_msg_show(BOX_MSG_NO_FILE, 0);
        } else {
            __dec_msg_hide(BOX_MSG_LOCKED_FILE);
        }
#endif
    }
}
/***************************** 删除当前菜单 ************************************/
static int menu_delcur_rep_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n [%s] %d",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item = 0;
    struct intent it;
    int attrs;
    u8 _name[128];

    init_intent(&it);

    switch (e->event) {
    case ELM_EVENT_KEY_CLICK:
        switch (e->value) {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);
            if (sel_item == 1) {
#ifdef CONFIG_FILE_PREVIEW_ENABLE
                if (__this->if_in_bro) {
                    fget_attr(__this->cur_file, &attrs);
                    if (attrs & F_ATTR_RO) {
                        log_d("locked file");
                        __dec_msg_show(BOX_MSG_LOCKED_FILE, 2000);
                    } else {
                        fdelete(__this->cur_file);
                        __this->cur_file = 0;
                        rep_exit_menu_post();
                    }
                } else {
                    it.name	= "video_dec";
                    it.action = ACTION_VIDEO_DEC_SET_CONFIG;
                    it.data = "del:cur";
                    start_app_async(&it, del_file_callback, NULL);
                }
#else
                it.name	= "video_dec";
                it.action = ACTION_VIDEO_DEC_SET_CONFIG;
                it.data = "del:cur";
                start_app_async(&it, del_file_callback, NULL);

                __dec_msg_show(BOX_MSG_DEL_CUR, 0);
#endif
            }
            ui_hide(LAYOUT_MN_DEL_CUR_REP);
            ui_hide(LAYOUT_MN_DEL_REP);
            break;
        case KEY_DOWN:
            return FALSE;
            break;
        case KEY_UP:
            return FALSE;
            break;
        case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
            break;
#endif
        case KEY_MENU:
            ui_hide(LAYOUT_MN_DEL_CUR_REP);
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_DEL_CUR_REP)
.onkey = menu_delcur_rep_onkey,
};


static void del_all_file_ok(void *p, int err)
{
    sys_key_event_enable();
    if (err == 0) {
        puts("---del_file_ok\n");
        __dec_msg_hide(BOX_MSG_DEL_ALL);
#ifdef CONFIG_FILE_PREVIEW_ENABLE
        __this->if_in_rep = 0;
        __this->cur_time.hour = 0;
        __this->cur_time.min = 0;
        __this->cur_time.sec = 0;
        __this->page = 0;
        __this->item = 0;
        ui_hide(LAYER_MENU_REP);
        ui_show(LAYER_BROWSER);
        __this->menu_status = 0;
#endif
    } else {
        printf("---del_file_faild: %d\n", err);
    }
}



/***************************** 删除所有菜单 ************************************/
static int menu_delall_rep_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n [%s] %d",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item = 0;
    struct intent it;

    init_intent(&it);

    switch (e->event) {
    case ELM_EVENT_KEY_CLICK:
        switch (e->value) {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);
            if (sel_item == 1) {
                it.name	= "video_dec";
                it.action = ACTION_VIDEO_DEC_SET_CONFIG;
                it.data = "del:all";
#ifdef CONFIG_FILE_PREVIEW_ENABLE
                it.exdata = (int)__this->path;
#endif
                sys_key_event_disable();
                start_app_async(&it, del_all_file_ok, NULL);
                __dec_msg_show(BOX_MSG_DEL_ALL, 0);
            }
            ui_hide(LAYOUT_MN_DEL_ALL_REP);
            ui_hide(LAYOUT_MN_DEL_REP);
            break;
        case KEY_DOWN:
            return FALSE;
            break;
        case KEY_UP:
            return FALSE;
            break;
        case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
            break;
#endif
        case KEY_MENU:
            ui_hide(LAYOUT_MN_DEL_ALL_REP);
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_DEL_ALL_REP)
.onkey = menu_delall_rep_onkey,
};



/*****************************布局上部回调 ************************************/
static int rep_layout_up_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;
    int ret;

    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 在此获取默认隐藏的图标的状态并显示
         */

        puts("rep_layout_up_onchange first show\n");
        if (__this->no_file == 0) {
            if (__this->file_type != 0xff) {
            }
            if (__this->is_lock) {
#ifndef CONFIG_IPC_UI_ENABLE
                ui_show(PIC_LOCK_REP);
#endif // CONFIG_IPC_UI_ENABLE
            }
            if (__this->fname) {
                printf("\n /******* [%s]%d\n",__func__,__LINE__);
//                ui_text_set_str_by_id(TEXT_FNAME_REP, "ascii", __this->fname);
                ui_text_set_textu_by_id(TEXT_FNAME_REP,__this->fname,strlen(__this->fname), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
            }
            if (__this->err_file) {
                __dec_msg_show(BOX_MSG_ERR_FILE, 0);
            }
            if (__this->file_type == 1) {
                if (__this->cur_time.hour || __this->cur_time.min || __this->cur_time.sec) {
                    ui_time_update_by_id(TIMER_RUN_REP, &__this->cur_time);
                    ui_show(TIMER_RUN_REP);
                } else {
#ifndef CONFIG_IPC_UI_ENABLE
                    ui_time_update_by_id(TIMER_SUM_REP, &__this->sum_time);
                    ui_show(TIMER_SUM_REP);
#endif // CONFIG_IPC_UI_ENABLE
                }

                /* ui_show(PIC_START_REP); */
                /* ui_show(PIC_SS_REP); */
                /* ui_show(PIC_PREV_REP); */
                /* ui_show(PIC_NEXT_REP); */
                /* printf("__this->ff_fr_flag = %d\n", __this->ff_fr_flag); */
                /* if(__this->ff_fr_flag > 0){ */
                /*     ui_show(PIC_FF_NUM_REP); */
                /*     ui_pic_show_image_by_id(PIC_FF_NUM_REP, __this->ff_fr_flag-1); */
                /*     ui_pic_show_image_by_id(PIC_NEXT_REP, 1); */
                /* } */
                /* else if(__this->ff_fr_flag < 0){ */
                /*     ui_show(PIC_FR_NUM_REP); */
                /*     ui_pic_show_image_by_id(PIC_FR_NUM_REP, __this->ff_fr_flag*(-1)-1); */
                /*     ui_pic_show_image_by_id(PIC_PREV_REP, 1); */
                /* } */
            }
        } else {
            if (__this->init == 1) {
                printf("/****** [%s] %d INIT __dec_msg_show",__func__,__LINE__);
                __dec_msg_show(BOX_MSG_NO_FILE, 0);
                ui_hide(TEXT_FNAME_REP);
                ui_hide(TEXT_RES_REP);
            }
        }
        __this->init = 1;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_UP_REP)
.onchange = rep_layout_up_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




/*****************************布局下部回调 ************************************/
static int rep_layout_down_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;
    int ret;

    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 在此获取默认隐藏的图标的状态并显示
         */

        puts("rep_layout_down_onchange first show\n");
        if (__this->no_file == 0) {

            /*
             * 视频文件
             */
            if (__this->file_type == 1) {
#ifndef CONFIG_IPC_UI_ENABLE
                ui_show(PIC_SS_REP);
#endif // CONFIG_IPC_UI_ENABLE
                printf("__this->ff_fr_flag = %d\n", __this->ff_fr_flag);

                /*
                 * 快进图标
                 */
                if (__this->ff_fr_flag > 0) {
                    ui_pic_show_image_by_id(PIC_FF_NUM_REP, __this->ff_fr_flag - 1);
                    ui_pic_show_image_by_id(PIC_NEXT_REP, 1);
                    ui_pic_show_image_by_id(PIC_PREV_REP, 0);
                } else if (__this->ff_fr_flag < 0) { /* 快退图标 */
                    ui_pic_show_image_by_id(PIC_FR_NUM_REP, __this->ff_fr_flag * (-1) - 1);
                    ui_pic_show_image_by_id(PIC_PREV_REP, 1);
                    ui_pic_show_image_by_id(PIC_NEXT_REP, 0);
                } else {
                    ui_pic_show_image_by_id(PIC_NEXT_REP, 0);
                    ui_pic_show_image_by_id(PIC_PREV_REP, 0);
                }

                /*
                 * 正在播放
                 */
#ifndef CONFIG_IPC_UI_ENABLE
                if (__this->if_in_rep == 1) {
                    ui_pic_show_image_by_id(PIC_START_REP, 0);
                } else {
                    ui_pic_show_image_by_id(PIC_START_REP, 1);
                }
#endif // CONFIG_IPC_UI_ENABLE
            }
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_DOWN_REP)
.onchange = rep_layout_down_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static void no_power_msg_box_timer(void *priv)
{
    static u8 cnt = 0;
    if (__this->battery_val <= 20 && __this->menu_status == 0 && __this->battery_char == 0) {
        cnt++;
        if ((cnt % 2) == 0) {
            __dec_msg_show(BOX_MSG_NO_POWER, 0);
        } else {
            __dec_msg_hide(BOX_MSG_NO_POWER);
        }
    } else {
        __dec_msg_hide(BOX_MSG_NO_POWER);
        cnt = 0;
        if (__this->no_file == 1 && __this->err_file == 0) {
            printf("/****** [%s] %d INIT __dec_msg_show",__func__,__LINE__);
            __dec_msg_show(BOX_MSG_NO_FILE, 0);
        } else if (__this->err_file == 1) {
            /* __dec_msg_show(BOX_MSG_ERR_FILE, 2000); */
        }
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
            __dec_msg_hide(0);
            return;
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
                __dec_msg_hide(BOX_MSG_10S_SHUTDOWN);
                ui_battery_level_change(100, 1);
                __this->battery_char = 1;
            } else if (event->u.dev.event == DEVICE_EVENT_POWER_CHARGER_OUT) {
                ten_sec_off = 1;
                __dec_msg_show(BOX_MSG_10S_SHUTDOWN, 0);
                ui_battery_level_change(__this->battery_val, 0);
                __this->battery_char = 0;
            }
        }
    }
}
/***************************** 电池控件动作 ************************************/
static int battery_rep_onchange(void *ctr, enum element_change_event e, void *arg)
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
        id = 0;
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
REGISTER_UI_EVENT_HANDLER(BATTERY_REP)
.onchange = battery_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#ifdef CONFIG_FILE_PREVIEW_ENABLE
REGISTER_UI_EVENT_HANDLER(BATTERY_BROWSER)
.onchange = battery_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif


/*
 * sd卡事件处理函数
 */
static void sd_event_handler(struct sys_event *event, void *priv)
{
    if (!strcmp(event->arg, "sd0") || !strcmp(event->arg, "sd1") || !strcmp(event->arg, "sd2")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
#ifdef CONFIG_FILE_PREVIEW_ENABLE
            if (__this->if_in_bro) {
                ui_pic_show_image_by_id(PIC_SD_BROWSER, 1);
                ui_file_browser_set_dir_by_id(BROWSER, __this->path, "-tMOVJPGAVI -st -d");
                /* ui_show(BROWSER); */
            } else {
#ifndef CONFIG_IPC_UI_ENABLE
                ui_pic_show_image_by_id(PIC_SD_REP, 1);
#endif
            }
#else
            ui_pic_show_image_by_id(PIC_SD_REP, 1);
#endif
            __dec_msg_hide(BOX_MSG_NO_FILE);
            break;
        case DEVICE_EVENT_OUT:
        case DEVICE_EVENT_OFFLINE:
#ifdef CONFIG_FILE_PREVIEW_ENABLE
            if (__this->menu_status) {
                ui_hide(LAYER_MENU_REP);
                ui_show(LAYER_BROWSER);
                __this->menu_status = 0;
            }
            if (__this->if_in_bro) {
                ui_pic_show_image_by_id(PIC_SD_BROWSER, 0);
                strcpy(__this->path, CONFIG_DEC_ROOT_PATH);
                ui_file_browser_set_dir_by_id(BROWSER, __this->path, "-tMOVJPGAVI -st -d");
                /* __this->no_file = 1; */
                /* ui_hide(BROWSER); */
            } else {
                __this->cur_time.hour = 0;
                __this->cur_time.min = 0;
                __this->cur_time.sec = 0;
                ui_hide(LAYER_UP_REP);
                ui_show(LAYER_BROWSER);
            }
#else
            ui_pic_show_image_by_id(PIC_SD_REP, 0);
#endif
            printf("/****** [%s] %d INIT __dec_msg_show",__func__,__LINE__);
            dec_sd_state = SD_STA_OFFLINE;
            __dec_msg_show(BOX_MSG_NO_FILE, 1500);

            break;
        default:
            break;
        }
    }
}

extern int storage_device_ready();
/***************************** SD 卡图标动作 ************************************/
int pic_sd_rep_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u16 id = 0;
    switch (e) {
    case ON_CHANGE_INIT:
        if (storage_device_ready() == 0) {
            /*
             * 第一次显示需要判断一下,SD不在线
             */
            ui_pic_set_image_index(pic, 0);
            printf("/****** [%s] %d INIT __dec_msg_show",__func__,__LINE__);
            __dec_msg_show(BOX_MSG_NO_FILE, 0);
        } else {
            /*
             * SD卡在线
             */
            ui_pic_set_image_index(pic, 1);
        }
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, sd_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PIC_SD_REP)
.onchange = pic_sd_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#ifdef CONFIG_FILE_PREVIEW_ENABLE
REGISTER_UI_EVENT_HANDLER(PIC_SD_BROWSER)
.onchange = pic_sd_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif
int menu_sd_rep_onchange(void *ctr, enum element_change_event e, void *arg)
{
    static u16 id = 0;
    switch (e) {
    case ON_CHANGE_INIT:
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, sd_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_MN_REP_REP)
.onchange = menu_sd_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

char *path = "B:/VIDEO/video.avi\0";

/***************************** 文件路径文本动作 ************************************/
int text_path_rep_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *txt = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        /*txt->str = path;*/
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TEXT_FNAME_REP)
.onchange = text_path_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/***************************** 文件路径文本动作 ************************************/
int text_path_browser_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *txt = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        txt->str = __this->path + sizeof(CONFIG_ROOT_PATH) - 1;
        return true;
#ifdef CONFIG_IPC_UI_ENABLE
    case ON_CHANGE_FIRST_SHOW:
        ui_text_set_textu_by_id(TEXT_PATH_BROWSER,__this->path + sizeof(CONFIG_ROOT_PATH) - 1,strlen(__this->path + sizeof(CONFIG_ROOT_PATH) - 1), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
        return true;
#endif // CONFIG_IPC_UI_ENABLE
    default:
        return false;
    }
    return false;
}

#ifndef CONFIG_IPC_UI_ENABLE
REGISTER_UI_EVENT_HANDLER(TEXT_PATH_BROWSER)
.onchange = text_path_browser_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif // CONFIG_IPC_UI_ENABLE

/***************************** 文件格式文本动作 ************************************/
int text_res_rep_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *txt = (struct ui_text *)ctr;
    int index = 0;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(txt, __this->file_type_index);
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TEXT_RES_REP)
.onchange = text_res_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/***************************** MSG BOX文本动作 ************************************/
int text_msg_box_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *txt = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        return TRUE;
    case ON_CHANGE_SHOW_PROBE:
        ui_text_set_index(txt, msg_show_id - 1);
        return TRUE;
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(TEXT_MSG_REP)
.onchange = text_msg_box_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/***************************** 回放模式图标动作 ************************************/
int pic_logo_rep_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, __this->file_type);
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PIC_LOGO_REP)
.onchange = pic_logo_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/***************************** 播放图标动作 ************************************/
int pic_play_rep_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    int index = 1;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, index);
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PIC_START_REP)
.onchange = pic_play_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




/***************** 以下为可视对讲项目新增内容**************************/
#ifdef CONFIG_IPC_UI_ENABLE

enum video_select_dec_enum {
    SELECT_MSG = 0,
    SELECT_VIDEO,
    SELECT_EVENT,
    SELECT_APP_MSG,
};

enum video_select_dec_enum dec_sel_item;

static int video_select_dec_table[] = {
    LAYOUT_SELECT_MSG,
    LAYOUT_SELECT_VIDEO,
    LAYOUT_SELECT_EVENT,
    LAYOUT_SELECT_APP_MSG,
};
//static int video_file_dec_table[] = {
//    LAYOUT_DEC_FILE11,
//    LAYOUT_DEC_FILE21,
//    LAYOUT_DEC_FILE31,
//    LAYOUT_DEC_FILE41,
//    LAYOUT_DEC_FILE51,
//    LAYOUT_DEC_FILE61,
//};


static int PIC_DEC_FILE[]={
    PIC_DEC_FILE1,
    PIC_DEC_FILE2,
    PIC_DEC_FILE3,
    PIC_DEC_FILE4,
    PIC_DEC_FILE5,
    PIC_DEC_FILE6,
};
static int TXT_DEC_FILE[]={
    TXT_DEC_FILE1,
    TXT_DEC_FILE2,
    TXT_DEC_FILE3,
    TXT_DEC_FILE4,
    TXT_DEC_FILE5,
    TXT_DEC_FILE6,
};
static int TXT_DEC_FILE_INFO[]={
    TXT_DEC_FILE1_INFO,
    TXT_DEC_FILE2_INFO,
    TXT_DEC_FILE3_INFO,
    TXT_DEC_FILE4_INFO,
    TXT_DEC_FILE5_INFO,
    TXT_DEC_FILE6_INFO,
};
static int PIC_UNREAD_MSG[]={
    PIC_UNREAD_MSG1,
    PIC_UNREAD_MSG2,
    PIC_UNREAD_MSG3,
    PIC_UNREAD_MSG4,
    PIC_UNREAD_MSG5,
    PIC_UNREAD_MSG6,
};
static int TXT_UNREAD_MSG[]={
    TXT_UNREAD_MSG1,
    TXT_UNREAD_MSG2,
    TXT_UNREAD_MSG3,
    TXT_UNREAD_MSG4,
    TXT_UNREAD_MSG5,
    TXT_UNREAD_MSG6,
};

static int vlist_select_dec_onchange(void *ctr, enum element_change_event e, void *arg)
{
//    printf("\n /***** [%s] e=%d\n",__func__,e);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n/****** [%s] INIT\n",__func__);
        break;
    case ON_CHANGE_FIRST_SHOW:
        if(dec_sel_item){
            printf("\n/****** [%s] FIRST_SHOW\n",__func__);
            if(dec_sel_item){
                ui_grid_highlight_item(grid, 0, false);
                ui_grid_highlight_item(grid, dec_sel_item, true);
            }
        }

        printf("\n /***** [%s] first show\n",__func__);
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        printf("\n /***** [%s] show completed\n",__func__);
        break;
    default:
        return false;
    }
    return false;
}

static int vlist_select_dec_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n [%s] %d\n",__func__,__LINE__);
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct vfscan *grid_vfscan = NULL;
    struct intent it;
    static int sel_item = 0;
    switch(e->event){
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_OK:
            update_longtime_idle_return_timer(2);//0开启 1更新 2关闭

            ui_hide(LAYER_DEC_SELECT);
            sel_item = ui_grid_cur_item(grid);
            printf("\n/***** sel_item = %d\n",sel_item);
//            select_msg_or_rec = sel_item;
            dec_sel_item = sel_item;

            switch(sel_item){
            case SELECT_MSG:
                strcpy(__this->path, CONFIG_VIDEO_TALK_PATH_2);
                break;

            case SELECT_VIDEO:
                strcpy(__this->path, CONFIG_VIDEO_TALK_PATH_1);
                break;

            case SELECT_EVENT:
                printf("\n/***** SELECT_EVENT \n");
                strcpy(__this->path, CONFIG_VIDEO_TALK_PATH_3);
                break;

            case SELECT_APP_MSG:
                printf("\n/***** SELECT_APP_MSG \n");
                strcpy(__this->path, CONFIG_VIDEO_TALK_PATH_4);
//                grid_vfscan = fscan(__this->path,"-tMOVJPGAVI -st -r");
//                if(grid_vfscan->file_number != 0){
//                    ui_show(LAYOUT_DEC_FILE);
//                    ui_show(LAYOUT_DEC_FILE_TXT);
//                    ui_show(LAYOUT_BROWSER2);
//                }
                break;
            }
            grid_vfscan = fscan(__this->path,"-tMOVJPGAVI -st -r");
            if(grid_vfscan->file_number != 0){
                ui_show(LAYER_BROWSER);
                ui_show(LAYOUT_DEC_FILE);
                ui_show(LAYOUT_DEC_FILE_TXT);
                ui_show(LAYOUT_BROWSER2);
            }else{
                __this->no_file = 1;
                __dec_msg_show(BOX_MSG_NO_FILE, 1500); //显示无文件
            }

            break;
        case KEY_DOWN:
            update_longtime_idle_return_timer(1);//0开启 1更新 2关闭
            return false;
            /*
             * 向后分发消息
             */

            break;
        case KEY_UP:
            update_longtime_idle_return_timer(1);//0开启 1更新 2关闭
            return false;
            /*
             * 向后分发消息
             */

            break;
        case KEY_MODE:
            printf("\n /***** [%s] key_mode ",__func__);
            return false;

            break;
        case KEY_MENU:

            break;

        default:
            return false;
            break;
        }
    }


    return true;
    /*
     * 不向后分发消息
     */
}

REGISTER_UI_EVENT_HANDLER(VLIST_SELECT_DEC)
.onchange = vlist_select_dec_onchange,
.onkey = vlist_select_dec_onkey,
 .ontouch = NULL,
};


static int layer_dec_select_onchange(void *ctr, enum element_change_event e, void *arg)
{
//    printf("\n /***** [%s] e=%d\n",__func__,e);
    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n/****** [%s] INIT\n",__func__);

        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("\n /***** [%s] first show\n",__func__);
        dec_state = DEC_STA_SEL_FILE_FOLDER;
        update_longtime_idle_return_timer(0);//0开启 1更新 2关闭

        if(dec_sel_item){
            printf("\n/****** [%s] dec_sel_item\n",__func__);
//            ui_grid_highlight_item_by_id(VLIST_SELECT_DEC, 1, false);
//            ui_grid_highlight_item_by_id(VLIST_SELECT_DEC, dec_sel_item, true);

//            ui_no_highlight_element_by_id(video_select_dec_table[0]);
//            ui_highlight_element_by_id(video_select_dec_table[dec_sel_item]);
        }
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        printf("\n /***** [%s] show completed\n",__func__);
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(LAYER_DEC_SELECT)
.onchange = layer_dec_select_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/*********************************************************************************
 *  		     				预览框回调
 *********************************************************************************/

static int browser2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("\n/******* browser2_onchange e= %d\n",e);
    struct ui_browser *bro = ctr;
    struct ui_file_attrs attrs_brower2;

    int _file_num = 0;
//    __this->page = ui_file_browser_cur_page(bro, &_file_num);
    ui_file_browser_cur_page(bro, &_file_num);
    browser_info.current_page_file_num = _file_num;
    browser_info.current_page_num = __this->page;


    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n/******browser2 ON_CHANGE_INIT\n");
        printf("\n/******* file_num =%d\n",browser_info.current_page_file_num);
        dec_state = DEC_STA_SEL_VIDEO;
        update_longtime_idle_return_timer(0);//0开启 1更新 2关闭

        bro->order = 0;
        bro->path = __this->path;
        /* __this->browser->path = cPATH[__this->dir]; */
#ifdef CONFIG_EMR_DIR_ENABLE
        if (strcmp(__this->path, CONFIG_DEC_PATH_1) == 0
            || strcmp(__this->path, CONFIG_DEC_PATH_2) == 0
            || strcmp(__this->path, CONFIG_DEC_PATH_3) == 0) {
            log_d("dec_path");
            bro->ftype = "-tMOVJPGAVI -st -r";
        } else {
            bro->ftype = "-tMOVJPGAVI -st -d";
        }
#else
        bro->ftype = "-tMOVJPGAVI -st -d";
#endif
        bro->show_mode = 1;

        if (__this->cur_file) {
            fclose(__this->cur_file);
            __this->cur_file = 0;
        }
        /**********公版************/
        if (__this->page) {
            printf("\n /***[%s] INIT __this->page = %d\n",__func__,__this->page);
            ui_file_browser_set_page(bro, __this->page);
        }
//        if (__this->item) {
//            ui_file_browser_highlight_item(bro, 0, false);
//            ui_file_browser_highlight_item(bro, __this->item, true);
//        }
        /*****************************/
        if(browser_info.current_file_page){
            bro->cur_number = browser_info.current_file_page*6;
            ui_file_browser_set_page(bro,browser_info.current_file_page);
            browser_info.current_file_page = 0;
        }
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("\n/***** browser2 ON_CHANGE_FIRST_SHOW takeover\n ");
        sys_key_event_takeover(true, false);


        printf("\n/***** file_num =%d\n",browser_info.current_page_file_num);
//        dec_state = DEC_STA_SEL_VIDEO;  //选择视频状态
        //初始化显示时，先根据文件数量隐藏文件框和文件
        hide_layout_dec_file_pic_txt();
        //初始化显示时，先根据文件数量设置文件类型名称
        set_layout_dec_file_txt();
        __this->if_in_bro = 1;

        break;
    case ON_CHANGE_SHOW_PROBE:
        update_longtime_idle_return_timer(1);//0开启 1更新 2关闭
        //实测browser2换页时的加载时间较长，这里需要先更新一次，按键后比起onkey会先跑这里，再在按键中更新
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        printf("\n/****** [%s] %d page_num = %d\n",__func__,__LINE__,ui_file_browser_page_num(bro));
        printf("\n/****** file_num =%d\n",browser_info.current_page_file_num);
        printf("\n ui_file_browser_page_num(bro) = %d\n",ui_file_browser_page_num(bro));
        if (!ui_file_browser_page_num(bro)) {
            __this->no_file = 1;
            __dec_msg_show(BOX_MSG_NO_FILE, 0); //显示无文件
        } else {
            __this->no_file = 0;
            __dec_msg_hide(BOX_MSG_NO_FILE);
        }

         printf("\n /*** [%s]page = %d, item = %d\n",__func__,browser_info.current_file_page,browser_info.current_file_item);
        //返回记忆的当前页
//        if(browser_info.current_file_page){
//            ui_file_browser_set_page(bro,browser_info.current_file_page);
//            __this->page = browser_info.current_file_page;
//            browser_info.current_file_page = 0;
//        }
        //返回记忆的当前页选择文件处
        if(browser_info.current_file_item){
            ui_file_browser_highlight_item(bro,0,false);
            ui_file_browser_highlight_item(bro,browser_info.current_file_item,true);
            __this->item = browser_info.current_file_item;
            browser_info.current_file_item = 0;
        }

        printf("\n /****** file_num =%d\n",browser_info.current_page_file_num);
        // 翻页时根据文件数量隐藏文件框和文件
        hide_layout_dec_file_pic_txt();
        // 翻页时根据文件数量显示文件类型，该处若是事件视频，则还需要判断什么事件
        set_layout_dec_file_txt();

        // 翻页时记录更新的文件名和未读状态
        for(int i = 0;i<browser_info.current_page_file_num;i++){
            ui_file_browser_get_file_attrs(bro, i, &attrs_brower2);

            //加锁文件（只读文件）识别为未读文件，已读后解锁（取消只读）
//            printf("\n /**** attr = %d\n",attrs_brower2.attr.attr);
            if (attrs_brower2.attr.attr & F_ATTR_RO) {
                ui_show(PIC_UNREAD_MSG[i]);
                ui_show(TXT_UNREAD_MSG[i]);
            }else{
                ui_hide(PIC_UNREAD_MSG[i]);
                ui_hide(TXT_UNREAD_MSG[i]);
            }

            //文件名相关
            if(attrs_brower2.fname){
//                printf("\n /***** this file name = %s\n",attrs_brower2.fname);
            }else{
                printf("\n /***** this file name = NULL\n");
            }
            strcpy(browser_info.current_page_file_name[i],attrs_brower2.fname);
//            ui_text_set_str_by_id(TXT_DEC_FILE_INFO[i], "ascii",attrs_brower2.fname);
            ui_text_set_str_by_id(TXT_DEC_FILE_INFO[i], "ascii",browser_info.current_page_file_name[i]);
//            ui_text_set_textu_by_id(TXT_DEC_FILE_INFO[i],browser_info.current_page_file_name[i],strlen(browser_info.current_page_file_name[i]), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);

        }

        if(browser_info.current_page_change_flag){
            browser_info.current_page_change_flag = 0;
//            browser_info.current_page_txt_info_change_flag = 0;
        }
        printf("/***[%s] %d\n ",__func__,__LINE__);
        break;

    case ON_CHANGE_RELEASE:
        printf("/***[%s] ON_CHANGE_RELEASE\n ",__func__);
        sys_key_event_takeover(false, false);
    default:
        break;
    }
    return false;
}

static int browser2_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n [%s] %d",__func__,__LINE__);
    struct ui_browser *_browser = ctr;
    struct ui_file_attrs attrs;
    int _file_num = 0;
    __this->page = ui_file_browser_cur_page(_browser, &_file_num);
    browser_info.current_page_file_num = _file_num;
    printf("\n /********** [%s] now file_num = %d\n",__func__,_file_num);
    __this->item = ui_file_browser_cur_item(_browser);

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MODE:
            //返回上级目录
            if (strcmp(__this->path, CONFIG_DEC_ROOT_PATH)) {
                update_longtime_idle_return_timer(2);//0开启 1更新 2关闭

                printf("/****** [%s] %d",__func__,__LINE__);
                __this->if_in_bro = 1;
                int len = strlen(__this->path);
                len--;
                while (--len) {
                    if (__this->path[len] != '/') {
                        __this->path[len] = 0;
                    } else {
                        break;
                    }
                }
                printf("\n path = %s\n", __this->path);
                ui_hide(LAYER_BROWSER);
                ui_hide(LAYER_MSG_REP);
                ui_hide(LAYER_UP_REP);
                ui_hide(LAYER_MENU_REP);
                ui_hide(LAYER_BROWSER2_INFO);
                ui_show(LAYER_DEC_SELECT);
//                ui_text_set_str_by_id(TEXT_PATH_BROWSER, "ascii", __this->path + sizeof(CONFIG_ROOT_PATH) - 1);
//                ui_file_browser_set_dir_by_id(BROWSER2, __this->path, "-tMOVJPGAVI -st -d");
                return true;
            } else {
                printf("/****** [%s] %d takeover",__func__,__LINE__);
                log_d("root ptah");
//                sys_key_event_takeover(false, true);
                sys_key_event_takeover(false, false);
                __this->if_in_bro = 0;
            }
            break;
        case KEY_OK:
            if (_file_num == 0) {
                log_d("no file\n");
                return true;
            }
            dec_state = DEC_STA_PLAYBACK;
            update_longtime_idle_return_timer(2);//0开启 1更新 2关闭

//            printf("item=%d \n", __this->item);
            printf("\n /*** [%s]page = %d, item = %d\n",__func__,__this->page,__this->item);
            browser_info.current_file_page = __this->page;
            browser_info.current_file_item = __this->item;
            ui_file_browser_get_file_attrs(_browser, __this->item, &attrs); //获取文件属性
            if (attrs.ftype == UI_FTYPE_DIR) {
                //进入文件夹
                if (__this->path == 0) {
                    log_e("DIR ERR\n");
                    return false;
                }
                printf("/****** [%s] %d",__func__,__LINE__);
                printf("/***** attrs.fname = %s",attrs.fname);
                printf("/***** __this->path = %s",__this->path);
                strcat(__this->path, attrs.fname);
                int len = strlen(__this->path);
                __this->path[len] = '/';
                __this->path[len + 1] = '\0';

                printf("path=%s\n", __this->path);
#ifdef CONFIG_EMR_DIR_ENABLE
                if (strcmp(__this->path, CONFIG_DEC_PATH_1) == 0
                    || strcmp(__this->path, CONFIG_DEC_PATH_2) == 0
                    || strcmp(__this->path, CONFIG_DEC_PATH_3) == 0) {
                    log_d("dec_path");
                    ui_file_browser_set_dir_by_id(BROWSER2, __this->path, "-tMOVJPGAVI -st -r");
                } else {
                    ui_file_browser_set_dir_by_id(BROWSER2, __this->path, "-tMOVJPGAVI -st -d");
                }
#else
//                printf("/********* select_msg_or_rec = %d",select_msg_or_rec);
                ui_file_browser_set_dir_by_id(BROWSER2, __this->path, "-tMOVJPGAVI -st -d");
//                if(select_msg_or_rec){
//                    ui_file_browser_set_dir_by_id(BROWSER, __this->path, "-tMOVJPGAVI -st -d");
//                }else{
//                    ui_file_browser_set_dir_by_id(BROWSER, __this->path, "-tMOVJPGAVI -st -ar -d");
//                }
#endif
//                ui_text_set_str_by_id(TEXT_PATH_BROWSER, "ascii", __this->path + sizeof(CONFIG_ROOT_PATH) - 1);
                    //使用较大字体
//                ui_text_set_textu_by_id(TEXT_PATH_BROWSER,__this->path + sizeof(CONFIG_ROOT_PATH) - 1,strlen(__this->path + sizeof(CONFIG_ROOT_PATH) - 1), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
                __this->if_in_bro = 1;
                return true;
            } else if (attrs.ftype == UI_FTYPE_IMAGE || attrs.ftype == UI_FTYPE_VIDEO) {
                //播放文件
                printf("path=%s, num=%d\n", __this->path, attrs.file_num);
                if (attrs.attr.attr & F_ATTR_RO) {
                    __this->is_lock = 1;
                } else {
                    __this->is_lock = 0;
                }
                FILE *fp = ui_file_browser_open_file(_browser, __this->item);
                __this->sum_time.sec = 0;
                __this->sum_time.min = 0;
                __this->sum_time.hour = 0;
                ui_hide(LAYER_BROWSER);
                ui_show(LAYER_UP_REP);
                ui_set_call(browser_open_file, (int)fp);
                __this->if_in_bro = 0;
                return true;
            }
            break;
        case KEY_MENU:
            if (_file_num == 0) {
                log_d("no file\n");
                return false;
            }
            ui_file_browser_get_file_attrs(_browser, __this->item, &attrs); //获取文件属性
            if (attrs.ftype == UI_FTYPE_IMAGE || attrs.ftype == UI_FTYPE_VIDEO) {
                __dec_msg_hide(0);
                __this->cur_file = ui_file_browser_open_file(_browser, __this->item);
                /* ui_hide(LAYER_UP_REP); */
                ui_hide(LAYER_BROWSER);
                ui_show(LAYER_MENU_REP);
                ui_show(LAYOUT_MN_REP_REP);
                __this->menu_status = 1;
                return true;
            }
            break;
        case KEY_UP:
            update_longtime_idle_return_timer(1);//0开启 1更新 2关闭
            //实测browser2换页时的加载时间较长，临界会有问题，在browser2_onchange中先更新一次，在此处再一次
            break;
        case KEY_DOWN:
            update_longtime_idle_return_timer(1);//0开启 1更新 2关闭
            //实测browser2换页时的加载时间较长，临界会有问题，在browser2_onchange中先更新一次，在此处再一次
            break;
        }
    default:
        if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
            printf("\n [%s] %d takeover true\n",__func__,__LINE__);
            sys_key_event_takeover(false, true);
            /* puts("\n take off msg power rep\n"); */
            return false;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(BROWSER2)
.onchange = browser2_onchange,
 .onkey = browser2_onkey,
};

static int layout_browser2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;
    int ret;

    switch (e) {
    case ON_CHANGE_INIT:

        printf("\n/****** [%s] %d INIT \n",__func__,__LINE__);

        break;
    case ON_CHANGE_FIRST_SHOW:
        if(browser_info.current_file_page){
            ui_file_browser_set_page_by_id(BROWSER2,browser_info.current_file_page-1);
            browser_info.current_file_page = 0;
        }
        puts("\n layout_browser2_onchange first show\n");


        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_BROWSER2)
.onchange = layout_browser2_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/*****************************回放信息布局界面 ************************************/

void hide_layout_dec_file_pic_txt()
{
    printf("\n /**** [%s] %d\n",__func__,__LINE__);
    int i = 0;

    if (__this->no_file == 0) {
        printf("\n /*** file_num = %d, now_show_num = %d, last_show_num = %d\n" ,browser_info.current_page_file_num\
                                                                                ,browser_info.current_page_now_show_num\
                                                                                ,browser_info.current_page_last_show_num);
        //当前显示小于实际文件数量时
        if((browser_info.current_page_file_num > browser_info.current_page_now_show_num)\
           && (browser_info.current_page_last_show_num != 0)){
            printf("\n /**** show_num!=page_file_num\n");
            for(i=browser_info.current_page_now_show_num;i<browser_info.current_page_file_num;i++){
//                printf("\n /***** show TXT_DEC_FILE AND INFO%d\n ",i);
//                ui_show(PIC_DEC_FILE[i]);
                ui_show(TXT_DEC_FILE[i]);
                ui_show(TXT_DEC_FILE_INFO[i]);
            }
        }
        //当前显示大于实际文件数量时
        if(browser_info.current_page_file_num < browser_info.current_page_now_show_num){
            if(browser_info.current_page_file_num < 6){
                for(i=5;i>=browser_info.current_page_file_num;i--){
//                    printf("\n /***** hide TXT_DEC_FILE AND INFO%d\n ",i);
                    ui_hide(PIC_UNREAD_MSG[i]);
                    ui_hide(TXT_UNREAD_MSG[i]);
                    ui_hide(TXT_DEC_FILE[i]);
                    ui_hide(TXT_DEC_FILE_INFO[i]);
                }
            }else{

            }
        }

        browser_info.current_page_last_show_num = browser_info.current_page_now_show_num;
        browser_info.current_page_now_show_num = browser_info.current_page_file_num;
//        if(browser_info.current_page_file_num != browser_info.current_page_now_show_num){
//            printf("\n /**** show_num!=page_file_num\n");
//
//        }else {
//            return;
//        }
    } else {
        if (__this->init == 1) {
            printf("/****** [%s] ON_CHANGE_FIRST_SHOW __dec_msg_show\n",__func__);
            __dec_msg_show(BOX_MSG_NO_FILE, 0);
            ui_hide(TEXT_FNAME_REP);
            ui_hide(TEXT_RES_REP);
            ui_hide(LAYOUT_DEC_FILE);
            ui_hide(LAYOUT_DEC_FILE_TXT);
        }
    }
}

//void hide_txt_in_layout_dec_file_txt()
//{
//    int i = 0;
//
//    if (__this->no_file == 0) {
//        printf("\n /*** file_num = %d, now_show_num = %d, last_show_num = %d\n" ,browser_info.current_page_file_num\
//                                                                                ,browser_info.current_page_now_show_num\
//                                                                                ,browser_info.current_page_last_show_num);
//        //当前显示小于实际文件数量时
//        if((browser_info.current_page_file_num > browser_info.current_page_now_show_num)\
//           && (browser_info.current_page_last_show_num != 0)){
//            printf("\n /**** show_num!=page_file_num\n");
//            for(i=browser_info.current_page_now_show_num;i<browser_info.current_page_file_num;i++){
//                printf("\n /***** show PIC_DEC_FILE\n ");
//                ui_show(TXT_DEC_FILE[i]);
//                ui_show(TXT_DEC_FILE_INFO[i]);
//            }
//        }
//        //当前显示大于实际文件数量时
//        if(browser_info.current_page_file_num < browser_info.current_page_now_show_num){
//            if(browser_info.current_page_file_num < 6){
//                for(i=5;i>=browser_info.current_page_file_num;i--){
//                    printf("\n /***** hide PIC_DEC_FILE\n ");
//                    ui_hide(TXT_DEC_FILE[i]);
//                    ui_hide(TXT_DEC_FILE_INFO[i]);
//                }
//            }else{
//
//            }
//        }
//        browser_info.current_page_last_show_num = browser_info.current_page_now_show_num;
//        browser_info.current_page_now_show_num = browser_info.current_page_file_num;
//    }
//}

void set_layout_dec_file_txt()
{
    int i = 0;
    if (__this->no_file == 0) {
        printf("\n /******* dec_sel_item = %d\n",dec_sel_item);
        if(browser_info.current_page_file_num < 6){
            for(i = 0;i<browser_info.current_page_file_num;i++){
                ui_text_show_index_by_id(TXT_DEC_FILE[i],dec_sel_item);
            }
        }else{
            for(int i = 0;i<6;i++){
                ui_text_show_index_by_id(TXT_DEC_FILE[i],dec_sel_item);
            }
        }
    }
    return;
}

//void set_txt_in_layout_dec_file_txt()
//{
//    int i = 0;
//    if (__this->no_file == 0) {
//        printf("\n /******* dec_sel_item = %d\n",dec_sel_item);
//        if(browser_info.current_page_file_num < 6){
//            for(i = 0;i<browser_info.current_page_file_num;i++){
//                ui_text_show_index_by_id(TXT_DEC_FILE[i],dec_sel_item);
//            }
//        }else{
//            for(int i = 0;i<6;i++){
//                ui_text_show_index_by_id(TXT_DEC_FILE[i],dec_sel_item);
//            }//该处show会触发TXT_DEC_FILE_INFO 2~6 的onchange，待查明原因
////            ui_text_show_index_by_id(TXT_DEC_FILE[0],dec_sel_item);
////            ui_text_show_index_by_id(TXT_DEC_FILE[1],dec_sel_item);
////            ui_text_show_index_by_id(TXT_DEC_FILE[2],dec_sel_item);
////            ui_text_show_index_by_id(TXT_DEC_FILE[3],dec_sel_item);
////            ui_text_show_index_by_id(TXT_DEC_FILE[4],dec_sel_item);
////            ui_text_show_index_by_id(TXT_DEC_FILE[5],dec_sel_item);

//        }
//    }
//    return;
//}

static int layout_dec_file_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;
    int ret;

    switch (e) {
    case ON_CHANGE_INIT:
        /*
         * 需要根据当前页面的文件数量先隐藏文件数量
         */
        printf("\n/****** [%s] %d INIT current_page_file_num = %d\n",__func__,__LINE__,browser_info.current_page_file_num);
//        hide_layout_dec_file_pic_txt();
        break;
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 需要根据当前页面的文件数量来显示文件名
         */
        puts("\n layout_dec_file_onchange first show\n");
        browser_info.current_page_now_show_num = 6;     //初始化显示6个文件，之后根据实际数量隐藏
        browser_info.current_page_last_show_num = 0;
//        set_layout_dec_file_txt();

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_DEC_FILE)
.onchange = layout_dec_file_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int layout_dec_file_txt_onchange(void *ctr, enum element_change_event e, void *arg)
{
//    printf("\n /***** layout_dec_file_txt_onchange e=%d\n",e);
    int item, id;
    const char *str = NULL;
    struct intent it;
    int ret;

    switch (e) {
    case ON_CHANGE_INIT:
        /*
         * 需要根据当前页面的文件数量先隐藏文件数量
         */
//        printf("\n/****** [%s] %d INIT current_page_file_num = %d\n",__func__,__LINE__,browser_info.current_page_file_num);
//        hide_txt_in_layout_dec_file_txt();

        break;
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 需要根据当前页面的文件数量来显示文件名
         */
        printf("\n /***** [%s] first show\n",__func__);
//        set_txt_in_layout_dec_file_txt();
        for(int i = 0;i<6;i++){
            ui_text_show_index_by_id(TXT_DEC_FILE[i],dec_sel_item);
        }
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        /*
         * 需要根据当前页面的文件数量来显示文件名
         */
        printf("\n /***** [%s] ON_CHANGE_SHOW_COMPLETED\n",__func__);
//        set_txt_in_layout_dec_file_txt();
        //初始化显示文件类型
//        for(int i = 0;i<6;i++){
//            ui_text_show_index_by_id(TXT_DEC_FILE[i],dec_sel_item);
//        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_DEC_FILE_TXT)
.onchange = layout_dec_file_txt_onchange,
};

static int txt_dec_file1_info_onchange(void *ctr, enum element_change_event e, void *arg)
{
//    printf("\n /***** [%s] e=%d\n",__func__,e);

    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n/****** [%s] %d INIT current_page_file_num = %d\n",__func__,__LINE__,browser_info.current_page_file_num);
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("\n /***** [%s] first show\n",__func__);
//        ui_text_set_str_by_id(TXT_DEC_FILE_INFO[0], "ascii",browser_info.current_page_file_name[0]);
        break;
    case ON_CHANGE_SHOW_COMPLETED:
//        printf("\n /***** [%s] show completed\n",__func__);
//        if(!(browser_info.current_page_txt_info_change_flag & BIT(0))){
//            browser_info.current_page_txt_info_change_flag |= BIT(0);
//            ui_text_set_str_by_id(TXT_DEC_FILE_INFO[0], "ascii",browser_info.current_page_file_name[0]);
////        ui_text_set_textu_by_id(TXT_DEC_FILE_INFO[i],attrs_brower2.fname,strlen(attrs_brower2.fname), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
//       }
        break;
    default:
        return false;
    }
    return false;
}
//REGISTER_UI_EVENT_HANDLER(TXT_DEC_FILE1_INFO)
//.onchange = txt_dec_file1_info_onchange,
//};

static int txt_dec_file2_info_onchange(void *ctr, enum element_change_event e, void *arg)
{
//    printf("\n /***** [%s] e=%d\n",__func__,e);

    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n/****** [%s] %d INIT current_page_file_num = %d\n",__func__,__LINE__,browser_info.current_page_file_num);
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("\n /***** [%s] first show\n",__func__);
//        ui_text_set_str_by_id(TXT_DEC_FILE_INFO[1], "ascii",browser_info.current_page_file_name[1]);
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        printf("\n /***** [%s] show completed\n",__func__);
//        if(!(browser_info.current_page_txt_info_change_flag & BIT(1))){
//            browser_info.current_page_txt_info_change_flag |= BIT(1);
//            ui_text_set_str_by_id(TXT_DEC_FILE_INFO[1], "ascii",browser_info.current_page_file_name[1]);
////        ui_text_set_textu_by_id(TXT_DEC_FILE_INFO[i],attrs_brower2.fname,strlen(attrs_brower2.fname), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
//        }
        break;
    default:
        return false;
    }
    return false;
}
//REGISTER_UI_EVENT_HANDLER(TXT_DEC_FILE2_INFO)
//.onchange = txt_dec_file2_info_onchange,
//};

static int txt_dec_file3_info_onchange(void *ctr, enum element_change_event e, void *arg)
{
//    printf("\n /***** [%s] e=%d\n",__func__,e);

    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n/****** [%s] %d INIT current_page_file_num = %d\n",__func__,__LINE__,browser_info.current_page_file_num);
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("\n /***** [%s] first show\n",__func__);
//        ui_text_set_str_by_id(TXT_DEC_FILE_INFO[2], "ascii",browser_info.current_page_file_name[2]);
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        printf("\n /***** [%s] show completed\n",__func__);
//        if(!(browser_info.current_page_txt_info_change_flag & BIT(2))){
//            browser_info.current_page_txt_info_change_flag |= BIT(2);
//            ui_text_set_str_by_id(TXT_DEC_FILE_INFO[2], "ascii",browser_info.current_page_file_name[2]);
////        ui_text_set_textu_by_id(TXT_DEC_FILE_INFO[i],attrs_brower2.fname,strlen(attrs_brower2.fname), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
//        }
        break;
    default:
        return false;
    }
    return false;
}
//REGISTER_UI_EVENT_HANDLER(TXT_DEC_FILE3_INFO)
//.onchange = txt_dec_file3_info_onchange,
//};

static int txt_dec_file4_info_onchange(void *ctr, enum element_change_event e, void *arg)
{
//    printf("\n /***** [%s] e=%d\n",__func__,e);

    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n/****** [%s] %d INIT current_page_file_num = %d\n",__func__,__LINE__,browser_info.current_page_file_num);
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("\n /***** [%s] first show\n",__func__);
//         ui_text_set_str_by_id(TXT_DEC_FILE_INFO[3], "ascii",browser_info.current_page_file_name[3]);
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        printf("\n /***** [%s] show completed\n",__func__);
//        if(!(browser_info.current_page_txt_info_change_flag & BIT(3))){
//            browser_info.current_page_txt_info_change_flag |= BIT(3);
//            ui_text_set_str_by_id(TXT_DEC_FILE_INFO[3], "ascii",browser_info.current_page_file_name[3]);
////        ui_text_set_textu_by_id(TXT_DEC_FILE_INFO[i],attrs_brower2.fname,strlen(attrs_brower2.fname), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
//        }
        break;
    default:
        return false;
    }
    return false;
}
//REGISTER_UI_EVENT_HANDLER(TXT_DEC_FILE4_INFO)
//.onchange = txt_dec_file4_info_onchange,
//};

static int txt_dec_file5_info_onchange(void *ctr, enum element_change_event e, void *arg)
{
//    printf("\n /***** [%s] e=%d\n",__func__,e);

    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n/****** [%s] %d INIT current_page_file_num = %d\n",__func__,__LINE__,browser_info.current_page_file_num);
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("\n /***** [%s] first show\n",__func__);
//        ui_text_set_str_by_id(TXT_DEC_FILE_INFO[4], "ascii",browser_info.current_page_file_name[4]);
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        printf("\n /***** [%s] show completed\n",__func__);
//        if(!(browser_info.current_page_txt_info_change_flag & BIT(4))){
//            browser_info.current_page_txt_info_change_flag |= BIT(4);
//            ui_text_set_str_by_id(TXT_DEC_FILE_INFO[4], "ascii",browser_info.current_page_file_name[4]);
////        ui_text_set_textu_by_id(TXT_DEC_FILE_INFO[i],attrs_brower2.fname,strlen(attrs_brower2.fname), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
//        }
        break;
    default:
        return false;
    }
    return false;
}
//REGISTER_UI_EVENT_HANDLER(TXT_DEC_FILE5_INFO)
//.onchange = txt_dec_file5_info_onchange,
//};

static int txt_dec_file6_info_onchange(void *ctr, enum element_change_event e, void *arg)
{
//    printf("\n /***** [%s] e=%d\n",__func__,e);

    switch (e) {
    case ON_CHANGE_INIT:
        printf("\n/****** [%s] %d INIT current_page_file_num = %d\n",__func__,__LINE__,browser_info.current_page_file_num);
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("\n /***** [%s] first show\n",__func__);
//        ui_text_set_str_by_id(TXT_DEC_FILE_INFO[5], "ascii",browser_info.current_page_file_name[5]);
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        printf("\n /***** [%s] show completed\n",__func__);
//        if(!(browser_info.current_page_txt_info_change_flag & BIT(5))){
//            browser_info.current_page_txt_info_change_flag |= BIT(5);
//            ui_text_set_str_by_id(TXT_DEC_FILE_INFO[5], "ascii",browser_info.current_page_file_name[5]);
////        ui_text_set_textu_by_id(TXT_DEC_FILE_INFO[i],attrs_brower2.fname,strlen(attrs_brower2.fname), FONT_DEFAULT|FONT_SHOW_MULTI_LINE);
//        }
        break;
    default:
        return false;
    }
    return false;
}
//REGISTER_UI_EVENT_HANDLER(TXT_DEC_FILE6_INFO)
//.onchange = txt_dec_file6_info_onchange,
//};

/*****************************回放模式页面回调 ************************************/
static int layer_msg_rep_onchange(void *ctr, enum element_change_event e, void *arg)
{
    printf("\n /***** [%s] e=%d\n",__func__,e);
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("\n***[%s] first show takeover true***\n",__func__);
        sys_key_event_takeover(true, true);
        break;
    case ON_CHANGE_HIDE:
        printf("\n***[%s] hide takeover false***\n",__func__);
        sys_key_event_takeover(false, false);
    case ON_CHANGE_RELEASE:
        break;
    default:
        return false;
    }
    return false;
}

static int layer_msg_rep_onkey(void *ctr, struct element_key_event *e)
{
    printf("\n/**** [%s] %d\n",__func__,__LINE__);
    struct intent it;

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            break;
        default:
            return true;
        }
        break;
    default:
        break;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYER_MSG_REP)
.onchange = layer_msg_rep_onchange,
 .onkey = layer_msg_rep_onkey,
};

void return_ui_to_layer_dec_select()
{
    ui_show(LAYER_DEC_SELECT);
}

enum machine_dec_state get_dec_state()
{
    return dec_state;
}

#endif // CONFIG_IPC_UI_ENABLE


#endif
