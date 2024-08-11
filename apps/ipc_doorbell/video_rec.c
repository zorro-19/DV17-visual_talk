#include "system/includes.h"
#include "server/ui_server.h"
#include "server/video_server.h"
#include "server/video_engine_server.h"
#include "server/usb_server.h"
#include "video_rec.h"
#include "video_system.h"
#include "gSensor_manage.h"
#include "user_isp_cfg.h"
#include "time.h"

#include "action.h"
#include "style.h"
#include "app_config.h"
#include "asm/debug.h"
#include "vrec_osd.h"
#include "vrec_icon_osd.h"
#include "app_database.h"
#include "storage_device.h"
#include "asm/lcd_config.h"
#include "doorbell_event.h"

#include "video_photo.h"
#include "server/video_dec_server.h"
//#include "xciot_cmd_ipc.h"
#define _STDIO_H_
#include "xciot_api.h"
#include "xciot_cmd_ipc.h"


#if (APP_CASE == __WIFI_CAR_CAMERA__)
#include "net_config.h"
#include "net_video_rec.h"
#endif

#define VIDEO_REC_NO_MALLOC //统一分配内存

#ifndef CONFIG_VIDEO0_ENABLE
#undef VREC0_FBUF_SIZE
#define VREC0_FBUF_SIZE   0
#endif
#ifndef CONFIG_VIDEO1_ENABLE
#undef VREC1_FBUF_SIZE
#define VREC1_FBUF_SIZE   0
#endif
#ifndef CONFIG_VIDEO2_ENABLE
#undef VREC2_FBUF_SIZE
#define VREC2_FBUF_SIZE   0
#endif
#ifndef CONFIG_VIDEO3_ENABLE
#undef VREC3_FBUF_SIZE
#define VREC3_FBUF_SIZE   0
#endif


#define SCREEN_W        LCD_DEV_WIDTH
#define SCREEN_H        LCD_DEV_HIGHT

#define MOTION_STOP_SEC     200
#define MOTION_START_SEC    2

#define LOCK_FILE_PERCENT	40    //0~100
#define NAME_FILE_BY_DATE   1

#ifdef PRODUCT_TEST_ENABLE
#include "datatype.h"
extern u8 product_rtc_default_wr(struct product_rtc_time *time, u8 is_write);
#endif

static int video_rec_start();
static int video_rec_stop(u8 close);
static int ve_mdet_start();
static int ve_mdet_stop();
static int ve_lane_det_start(u8 fun_sel);
static int ve_lane_det_stop(u8 fun_sel);
static int ve_face_det_start(u8 fun_sel);
static int ve_face_det_stop(u8 fun_sel);
static int video_rec_start_isp_scenes();
static int video_rec_stop_isp_scenes(u8 status, u8 restart);
static int video_rec_savefile(int dev_id);
extern int video_rec_set_config(struct intent *it);
static int video0_set_audio_callback(struct server *server, int (*callback)(u8 *buf, u32 len));
extern  void set_ip_value_add(int data);
extern  void set_ip_hue_value_add_low(int data);
extern  void set_ip_hue_value_add_high(int data);
extern  void test_audio_reg();
extern  void ui_lcd_light_on(void);
extern  void ui_lcd_light_off(void);
extern bool judge_plan_video_timer();
#define MSG_TIME 60
int dec_pro_file_close();
void hide_standby_ui();
int show_standby_ui();
int show_dist_net_ui();
void hide_dist_net_ui();
void video_standby_post_msg(const char *msg, ...);
void video_dist_net_post_msg(const char *msg, ...);
void rec_dispose_ui_calling(u8 mode);
void calling_timeout();
void return_ui_to_main();
void audio_talk_show_ui_busy();
extern const char *background_cfg_file[];

u8 simulate_state = 0;//模拟状态位
u8 sel_contact_flag = 0;//选择联系人标志位
u8 msging_countdown = 0;//留言倒计时

enum video_net_link_state {
    NET_STA_IDLE=0,
    NET_STA_START,
    NET_STA_CONNECTING,
    NET_STA_CONNECT_SUCC,
    NET_STA_CONNECT_FAIL,
    NET_STA_DISCONNECT,    //连接后又断开
};
enum video_net_link_state video_net_state;

u32 ui_page_now_id = 0;

//struct uimsg_pageid {
//    const char *msg;
//    u32 page_id;
//};

//static const struct uimsg_pageid msg_to_page_id[] = {
//    {"calling_request",             LAYER_CALL_REC          },
//    {"sel_contact",                 LAYOUT_CONTACT_SEL      },
//    {"machine_close_calling",       LAYER_STANDBY           },
//    {"app_launch_call",             LAYOUT_CALLING_REC      },
//    {"app_accept_call",             LAYOUT_CALLING_REC      },
//    {"app_close_call",              LAYOUT_CALLING_CLOSE    },
//    {"app_refuse_calling",          LAYOUT_CALLING_BUSY     },
//    {"calling_timeout",             LAYOUT_CALLING_TIMEOUT  },
//    {"network_connect_start",       LAYOUT_SCAN_CODE        },
//    {"network_connecting",          LAYOUT_NET_CONNECTING   },
//    {"connect_succ",                LAYOUT_LINK_SUCC        },
//    {"connect_fail",                LAYOUT_LINK_FAIL        },
//    {NULL,0},
//};

static u32 standby_update_timer     = 0;        //更新主界面时间
static u32 standby_update_weather   = 0;        //更新主界面天气
static u32 calling_timeout_timer    = 0;        //通话超时
static u32 rec_call_dispose_timer   = 0;        //定时显示通话ui处理
static u32 rec_msg_dispose_timer   = 0;        //定时显示留言ui处理
static u32 hide_ui_busy_timer       = 0;        //定时隐藏忙碌提示
static u32 msg_finish_timer         = 0;        //定时隐藏忙碌提示

static u8 rec_call_dispose_mode     = 0;        //定时显示通话ui处理参数
static u8 rec_msg_dispose_mode     = 0;        //定时显示留言ui处理参数

void msg_finish_timer_handler(u8 mode);
void rec_dispose_timer_handler(u8 mode);
void hide_ui_busy_timer_handler();

static int backup_wakeup_status;

void *get_user_list_t_info(void);

extern u8 now_msg_fname[MAX_FILE_NAME_LEN];

char video_rec_osd_buf[64] ALIGNE(64);
struct video_rec_hdl rec_handler;
#define __this 	(&rec_handler)


#ifdef __CPU_DV15__
static const u16 rec_pix_w[] = {1280, 640};
static const u16 rec_pix_h[] = {720,  480};
#else
#if 0
static const u16 rec_pix_w[] = {1440, 1280, 640};
static const u16 rec_pix_h[] = {1088, 720,  480};
#else

static const u16 rec_pix_w[] = {GET_VIDEO_WIDTH, 1280, 640};
static const u16 rec_pix_h[] = {GET_VIDEO_HEIGHT, 720,  480};
#endif
#endif

#ifdef CONFIG_VIDEO4_ENABLE
static struct video_window disp_window[DISP_MAX_WIN][4] = {0};

static void video_set_disp_window()
{
#define DISP_W (SCREEN_W - 80)
#define DISP_H (SCREEN_H)
#define DISP_W2 (DISP_W / 2 / 16 * 16)
#define DISP_H2 (DISP_H / 2 / 16 * 16)
#define DISP_H3 (DISP_H / 3 / 16 * 16)
#define DISP_W4 (DISP_W / 4 / 16 * 16)

#define BORDER2 ((720 * DISP_W2 / 1280 - DISP_H2) / 2)

    //DISP_MAIN_WIN
    disp_window[DISP_MAIN_WIN][0].left   = (DISP_W - DISP_W2 * 2) / 2;
    disp_window[DISP_MAIN_WIN][0].top    = 0;
    disp_window[DISP_MAIN_WIN][0].width  = DISP_W2;
    disp_window[DISP_MAIN_WIN][0].height = DISP_H2;
    disp_window[DISP_MAIN_WIN][0].border_top    = BORDER2 > 0 ? BORDER2 : 0;
    disp_window[DISP_MAIN_WIN][0].border_bottom = BORDER2 > 0 ? BORDER2 : 32;

    disp_window[DISP_MAIN_WIN][1].left   = DISP_W2 + (DISP_W - DISP_W2 * 2);
    disp_window[DISP_MAIN_WIN][1].top    = 0;
    disp_window[DISP_MAIN_WIN][1].width  = DISP_W2;
    disp_window[DISP_MAIN_WIN][1].height = DISP_H2;
    disp_window[DISP_MAIN_WIN][1].border_top    = BORDER2 > 0 ? BORDER2 : 0;
    disp_window[DISP_MAIN_WIN][1].border_bottom = BORDER2 > 0 ? BORDER2 : 32;

    disp_window[DISP_MAIN_WIN][2].left   = (DISP_W - DISP_W2 * 2) / 2;
    disp_window[DISP_MAIN_WIN][2].top    = DISP_H2;
    disp_window[DISP_MAIN_WIN][2].width  = DISP_W2;
    disp_window[DISP_MAIN_WIN][2].height = DISP_H2;
    disp_window[DISP_MAIN_WIN][2].border_top    = BORDER2 > 0 ? BORDER2 : 0;
    disp_window[DISP_MAIN_WIN][2].border_bottom = BORDER2 > 0 ? BORDER2 : 32;

    disp_window[DISP_MAIN_WIN][3].left   = DISP_W2 + (DISP_W - DISP_W2 * 2);
    disp_window[DISP_MAIN_WIN][3].top    = DISP_H2;
    disp_window[DISP_MAIN_WIN][3].width  = DISP_W2;
    disp_window[DISP_MAIN_WIN][3].height = DISP_H2;
    disp_window[DISP_MAIN_WIN][3].border_top    = BORDER2 > 0 ? BORDER2 : 0;
    disp_window[DISP_MAIN_WIN][3].border_bottom = BORDER2 > 0 ? BORDER2 : 32;

    //DISP_HALF_WIN
    disp_window[DISP_HALF_WIN][0].left   = 0;
    disp_window[DISP_HALF_WIN][0].top    = 0;
    disp_window[DISP_HALF_WIN][0].width  = DISP_W - DISP_W4;
    disp_window[DISP_HALF_WIN][0].height = DISP_H;
    disp_window[DISP_HALF_WIN][0].border_top    = 0;
    disp_window[DISP_HALF_WIN][0].border_bottom = 64;

    disp_window[DISP_HALF_WIN][1].left   = DISP_W - DISP_W4;
    disp_window[DISP_HALF_WIN][1].top    = 0;
    disp_window[DISP_HALF_WIN][1].width  = DISP_W4;
    disp_window[DISP_HALF_WIN][1].height = DISP_H3;
    disp_window[DISP_HALF_WIN][1].border_top    = 0;
    disp_window[DISP_HALF_WIN][1].border_bottom = 16;

    disp_window[DISP_HALF_WIN][2].left   = DISP_W - DISP_W4;
    disp_window[DISP_HALF_WIN][2].top    = DISP_H3;
    disp_window[DISP_HALF_WIN][2].width  = DISP_W4;
    disp_window[DISP_HALF_WIN][2].height = DISP_H3;
    disp_window[DISP_HALF_WIN][2].border_top    = 0;
    disp_window[DISP_HALF_WIN][2].border_bottom = 16;

    disp_window[DISP_HALF_WIN][3].left   = DISP_W - DISP_W4;
    disp_window[DISP_HALF_WIN][3].top    = DISP_H3 * 2;
    disp_window[DISP_HALF_WIN][3].width  = DISP_W4;
    disp_window[DISP_HALF_WIN][3].height = DISP_H3;
    disp_window[DISP_HALF_WIN][3].border_top    = 0;
    disp_window[DISP_HALF_WIN][3].border_bottom = 16;

    //DISP_VIDEO0
    disp_window[DISP_VIDEO0][0].width  = DISP_W;
    disp_window[DISP_VIDEO0][0].height = 0;

    disp_window[DISP_VIDEO0][1].width  = (u16) - 1;
    disp_window[DISP_VIDEO0][1].height = 0;

    disp_window[DISP_VIDEO0][2].width  = (u16) - 1;
    disp_window[DISP_VIDEO0][2].height = 0;

    disp_window[DISP_VIDEO0][3].width  = (u16) - 1;
    disp_window[DISP_VIDEO0][3].height = 0;

    //DISP_VIDEO1
    disp_window[DISP_VIDEO1][1].width  = DISP_W;
    disp_window[DISP_VIDEO1][1].height = 0;

    disp_window[DISP_VIDEO1][0].width  = (u16) - 1;
    disp_window[DISP_VIDEO1][0].height = 0;

    disp_window[DISP_VIDEO1][2].width  = (u16) - 1;
    disp_window[DISP_VIDEO1][2].height = 0;

    disp_window[DISP_VIDEO1][3].width  = (u16) - 1;
    disp_window[DISP_VIDEO1][3].height = 0;

    //DISP_VIDEO2
    disp_window[DISP_VIDEO2][2].width  = DISP_W;
    disp_window[DISP_VIDEO2][2].height = 0;

    disp_window[DISP_VIDEO2][0].width  = (u16) - 1;
    disp_window[DISP_VIDEO2][0].height = 0;

    disp_window[DISP_VIDEO2][1].width  = (u16) - 1;
    disp_window[DISP_VIDEO2][1].height = 0;

    disp_window[DISP_VIDEO2][3].width  = (u16) - 1;
    disp_window[DISP_VIDEO2][3].height = 0;

    //DISP_VIDEO3
    disp_window[DISP_VIDEO3][3].width  = DISP_W;
    disp_window[DISP_VIDEO3][3].height = 0;

    disp_window[DISP_VIDEO3][0].width  = (u16) - 1;
    disp_window[DISP_VIDEO3][0].height = 0;

    disp_window[DISP_VIDEO3][1].width  = (u16) - 1;
    disp_window[DISP_VIDEO3][1].height = 0;

    disp_window[DISP_VIDEO3][2].width  = (u16) - 1;
    disp_window[DISP_VIDEO3][2].height = 0;

    //DISP_PARK_WIN
    disp_window[DISP_PARK_WIN][1].width  = DISP_W;
    disp_window[DISP_PARK_WIN][1].height = 0;

    disp_window[DISP_PARK_WIN][0].width  = (u16) - 1;
    disp_window[DISP_PARK_WIN][0].height = 0;

    disp_window[DISP_PARK_WIN][2].width  = (u16) - 1;
    disp_window[DISP_PARK_WIN][2].height = 0;

    disp_window[DISP_PARK_WIN][3].width  = (u16) - 1;
    disp_window[DISP_PARK_WIN][3].height = 0;
}


#else
static struct video_window disp_window[DISP_MAX_WIN][4] = {0};

static void video_set_disp_window()
{
    u16 small_screen_w;
    u16 small_screen_h;

//    if (LCD_DEV_WIDTH > 1280) {
//        small_screen_w = 480; //16 aline
//        small_screen_h = 400; //16 aline
//    } else if (LCD_DEV_WIDTH > 480) {
//        small_screen_w = 800;//320; //16 aline
//        small_screen_h = 480;//240; //16 aline
//    } else {
//        small_screen_w = 192; //16 aline
//        small_screen_h = 160; //16 aline
//    }
    small_screen_w = SCREEN_W; //16 aline
    small_screen_h = SCREEN_H; //16 aline
    //DISP_MAIN_WIN
    disp_window[DISP_MAIN_WIN][0].width  = 320;
    disp_window[DISP_MAIN_WIN][0].height = 240;
    disp_window[DISP_MAIN_WIN][1].width  = small_screen_w;
    disp_window[DISP_MAIN_WIN][1].height = small_screen_h;
#ifdef THREE_WAY_ENABLE
    disp_window[DISP_MAIN_WIN][3].width  = small_screen_w;
    disp_window[DISP_MAIN_WIN][3].height = small_screen_h;
    disp_window[DISP_MAIN_WIN][3].left   = SCREEN_W - small_screen_w;
    disp_window[DISP_MAIN_WIN][3].top    = 0;
#endif

    //DISP_HALF_WIN
    disp_window[DISP_HALF_WIN][0].width  = SCREEN_W / 2;
    disp_window[DISP_HALF_WIN][0].height = SCREEN_H;
    disp_window[DISP_HALF_WIN][1].left   = SCREEN_W / 2;
    disp_window[DISP_HALF_WIN][1].width  = SCREEN_W / 2;
    disp_window[DISP_HALF_WIN][1].height = SCREEN_H;
#ifdef THREE_WAY_ENABLE
    disp_window[DISP_HALF_WIN][1].height = SCREEN_H / 2;
    disp_window[DISP_HALF_WIN][3].left   = SCREEN_W / 2;
    disp_window[DISP_HALF_WIN][3].top    = SCREEN_H / 2;
    disp_window[DISP_HALF_WIN][3].width  = SCREEN_W / 2;
    disp_window[DISP_HALF_WIN][3].height = SCREEN_H / 2;
#endif

    //DISP_FRONT_WIN
    disp_window[DISP_FRONT_WIN][0].width  = SCREEN_W;
    disp_window[DISP_FRONT_WIN][0].height = 0;
    disp_window[DISP_FRONT_WIN][1].width  = (u16) - 1;
    disp_window[DISP_FRONT_WIN][1].height = 0;
#ifdef THREE_WAY_ENABLE
    disp_window[DISP_FRONT_WIN][3].width  = (u16) - 1;
    disp_window[DISP_FRONT_WIN][3].height = 0;
#endif

    //DISP_BACK_WIN
    disp_window[DISP_BACK_WIN][0].width  = (u16) - 1;
    disp_window[DISP_BACK_WIN][0].height = 0;
    disp_window[DISP_BACK_WIN][1].width  = SCREEN_W;
    disp_window[DISP_BACK_WIN][1].height = SCREEN_H;
#ifdef THREE_WAY_ENABLE
    disp_window[DISP_BACK_WIN][3].width  = (u16) - 1;
    disp_window[DISP_BACK_WIN][3].height = 0;
#endif

    //DISP_PARK_WIN
    disp_window[DISP_PARK_WIN][0].width  = (u16) - 1;
    disp_window[DISP_PARK_WIN][0].height = 0;
    disp_window[DISP_PARK_WIN][1].width  = SCREEN_W;
    disp_window[DISP_PARK_WIN][1].height = SCREEN_H;
#ifdef THREE_WAY_ENABLE
    disp_window[DISP_PARK_WIN][3].width  = (u16) - 1;
    disp_window[DISP_PARK_WIN][3].height = 0;
#endif
}
#endif

static void video_set_disp_mirror(int id, u16 mirror)
{
    for (int i = 0; i < DISP_MAX_WIN; i++) {
        disp_window[i][id].mirror = mirror;
    }
}


static const char *rec_dir[][2] = {
#ifdef CONFIG_EMR_DIR_ENABLE
    {CONFIG_REC_DIR_0, CONFIG_EMR_REC_DIR_0},
    {CONFIG_REC_DIR_1, CONFIG_EMR_REC_DIR_1},
    {CONFIG_REC_DIR_2, CONFIG_EMR_REC_DIR_2},
    {CONFIG_REC_DIR_3, CONFIG_EMR_REC_DIR_3},
#else
    {CONFIG_REC_DIR_0, CONFIG_REC_DIR_0},
    {CONFIG_REC_DIR_1, CONFIG_REC_DIR_1},
    {CONFIG_REC_DIR_2, CONFIG_REC_DIR_2},
    {CONFIG_REC_DIR_3, CONFIG_REC_DIR_3},
#endif
};

static const char *rec_path[][2] = {
#ifdef CONFIG_EMR_DIR_ENABLE
    {CONFIG_REC_PATH_0, CONFIG_EMR_REC_PATH_0},
    {CONFIG_REC_PATH_1, CONFIG_EMR_REC_PATH_1},
    {CONFIG_REC_PATH_2, CONFIG_EMR_REC_PATH_2},
    {CONFIG_REC_PATH_3, CONFIG_EMR_REC_PATH_3},
#else
    {CONFIG_REC_PATH_0, CONFIG_REC_PATH_0},
    {CONFIG_REC_PATH_1, CONFIG_REC_PATH_1},
    {CONFIG_REC_PATH_2, CONFIG_REC_PATH_2},
    {CONFIG_REC_PATH_3, CONFIG_REC_PATH_3},
#endif
};


#if NAME_FILE_BY_DATE
static int __get_sys_time(struct sys_time *time)
{
    void *fd = dev_open("rtc", NULL);
    if (fd) {
        dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)time);
        dev_close(fd);
        return 0;
    }

    return -EINVAL;
}
#endif
static char file_name_prefix[12] = "KEY";

void set_rec_file_name_prefix(const char *str)
{
    if (str) {
        strcpy(file_name_prefix, str);
    }
}
struct vfscan *video_fscan[2][CONFIG_VIDEO_REC_NUM]={0};
//char video_name[128]={0};
void get_last_video_file(char *file_name){
    const char *str = "-tMOV -sn";
  //  struct vfscan *hd=  fscan(rec_dir[0][0],str );
    char video_name[128]={0};
    //file_name=&video_name[0];
    for (int i = 0; i < ARRAY_SIZE(rec_path); i++) {
         printf("\n ====page===%d,%s\n",__LINE__,__FUNCTION__);
        video_fscan[i][i] = fscan(rec_path[i][i], str);
         printf("\n ====page===%d,%s\n",__LINE__,__FUNCTION__);
        if (video_fscan[i][i] == NULL) {
        printf("\n no file\n");
            continue;
        }
        //int sel_mode = FSEL_FIRST_FILE;
        int sel_mode =FSEL_LAST_FILE;
        while (1) {
                 printf("\n ====page===%d,%s\n",__LINE__,__FUNCTION__);
            FILE *file = fselect(video_fscan[i][i], sel_mode, 0);
            if (!file) {
             printf("\n ====page===%d,%s\n",__LINE__,__FUNCTION__);

                break;
            }
        int file_len=flen(file);
        printf("\n file_len:%d MB\n",file_len/1024/1024);
        #if  1
        int len=fget_name(file,video_name,sizeof(video_name));
        strcpy(file_name,video_name);
        printf("\n video_name:::%s\n",video_name);
        #else
         int len=fget_name(file,file_name,sizeof(file_name));
        printf("\n file_name:::%s\n",file_name);

        #endif
        fclose(file);
        fscan_release(video_fscan[i][i]);
        video_fscan[i][i] = NULL;
        break ;
        }


     return ;
    }


}
//struct tm
//{
//  int	tm_sec;
//  int	tm_min;
//  int	tm_hour;
//  int	tm_mday;
//  int	tm_mon;
//  int	tm_year;
//  int	tm_wday;
//  int	tm_yday;
//  int	tm_isdst;
//#ifdef __TM_GMTOFF
//  long	__TM_GMTOFF;
//#endif
//#ifdef __TM_ZONE
//  const char *__TM_ZONE;
//#endif
//};




extern u8 get_avsdk_connect_flag(void);
static const char *rec_file_name(int format)
{
#if NAME_FILE_BY_DATE
    struct sys_time time;
    static char file_name[MAX_FILE_NAME_LEN];

    #ifdef ENABLE_VIDEO_FILE_AUTO_ADD
    static u8  very_timer=0;
    char last_file_name[128]={0};
     if(!get_avsdk_connect_flag()){
        if(!very_timer)
        {
         very_timer=1;
         get_last_video_file(&last_file_name[0]);
         struct tm p;
         get_utc_time_for_name(&p, &last_file_name[0], strlen(last_file_name));
         char day[64];
         snprintf(day, sizeof(day), "%d%02d%02d%02d%02d%02d", p.tm_year, p.tm_mon, p.tm_mday,p.tm_hour,p.tm_min,p.tm_sec);
         printf("\n last_file_name:%s, %s \n ",last_file_name,day);
         #ifdef PRODUCT_TEST_ENABLE
         struct product_rtc_time rtime = {0};
         rtime.year=p.tm_year;
         rtime.month=p.tm_mon;
         rtime.day=p.tm_mday;
         rtime.hour=p.tm_hour;
         rtime.min=p.tm_min+db_select("cyc") ;
         rtime.sec=p.tm_sec;
         printf("\n rtime.min=%d\n",rtime.min);
         product_rtc_default_wr(&rtime, 1);
         #endif
        }

     }
     #endif
    if (__get_sys_time(&time) == 0) {


        if (format == VIDEO_FMT_AVI) {
            sprintf(file_name, "%s_%d%02d%02d_%02d%02d%02d.AVI", file_name_prefix,
                    time.year, time.month, time.day, time.hour, time.min, time.sec);
        } else if (format == VIDEO_FMT_MOV) {
            sprintf(file_name, "%s_%d%02d%02d_%02d%02d%02d.MOV", file_name_prefix,
                    time.year, time.month, time.day, time.hour, time.min, time.sec);
        }
        return file_name;
    }
#endif

    if (format == VIDEO_FMT_AVI) {
        return "VID****.AVI";
    } else {
        return "VID****.MOV";
    }
}




u32 get_video_disp_state()
{
    return __this->disp_state;
}

static void video_home_post_msg(const char *msg, ...)
{
#ifdef CONFIG_UI_STYLE_JL02_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_MAIN_PAGE;
        req.msg.msg = msg;
        req.msg.exdata = argptr;

        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);

#endif
}

void video_parking_post_msg(const char *msg, ...)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_PARKING;
        req.msg.msg = msg;
        req.msg.exdata = argptr;

        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);
#endif
}

void video_rec_post_msg(const char *msg, ...)
{
    return ;
#ifdef CONFIG_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_VIDEO_REC;
        req.msg.msg = msg;
        req.msg.exdata = argptr;

        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);

#endif

}


static int video_rec_online_nums()
{
    u8 nums = 0;

    for (int i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
        if (__this->video_online[i]) {
            nums++;
        }
    }

    return nums;
}

static int video_disp_pause(int id)
{
    union video_req req = {0};
    req.display.state = VIDEO_STATE_PAUSE;

    return server_request(__this->video_display[id], VIDEO_REQ_DISPLAY, &req);
}

static int video_disp_start(int id, const struct video_window *win)
{
    int err = 0;
    union video_req req = {0};
    static char dev_name[20];
#ifdef CONFIG_DISPLAY_ENABLE

    log_d("video_disp_start: %d, %d x %d\n", id, win->width, win->height);

    if (win->width == (u16) - 1) {
        puts("video_disp_hide\n");
        return 0;
    }

#ifdef CONFIG_VIDEO0_ENABLE
#ifdef CONFIG_VIDEO0_INIT_ASYNC
    if(id == 0){
        extern int os_query_init_video_sem(void);
        if (os_query_init_video_sem()) {
            printf("\n os_query_init_video_sem err\n");
            hi3861l_reset();
        }
    }

#endif // CONFIG_VIDEO0_INIT_ASYNC
#endif // CONFIG_VIDEO0_ENABLE



#ifdef CONFIG_VIDEO4_ENABLE
    sprintf(dev_name, "video%d.%d.%d", 4, id, 0);
#else
    sprintf(dev_name, "video%d.%d", id, id < 3 ? 0 : __this->uvc_id);
#endif

    if (!__this->video_display[id]) {
        __this->video_display[id] = server_open("video_server", (void *)dev_name);
        if (!__this->video_display[id]) {
            log_e("open video_server: faild, id = %d\n", id);
            return -EFAULT;
        }
    }


    req.display.fb 		        = "fb1";
//    req.display.left  	        = win->left;
//    req.display.top 	        = win->top;
//    req.display.width 	        = win->width;
//    req.display.height 	        = win->height;
//    req.display.border_left     = win->border_left;
//    req.display.border_top      = win->border_top;
//    req.display.border_right    = win->border_right;
//    req.display.border_bottom   = win->border_bottom;


    req.display.left  	        = win->left;
    req.display.top 	        =0;// 128;
    req.display.width 	        =240;//360;// ;
    req.display.height 	        =320;//480;//
    req.display.border_left     =0;// 112;
    req.display.border_right    = 0;//112;
    req.display.border_top      = win->border_top;
    req.display.border_bottom   = win->border_bottom;


    req.display.mirror   		= win->mirror;
    req.display.jaggy			= 0;	// IMC 抗锯齿

    printf("\n /**** l = %d, t = %d, w = %d, h = %d\n",req.display.left,req.display.top,req.display.width,req.display.height);
    printf("\n /**** b_l = %d, b_t = %d, b_r = %d, b_b = %d\n",req.display.border_left,req.display.border_top,req.display.border_right,req.display.border_bottom);

    if (id == 0) {
        req.display.camera_config   = NULL;
        /* req.display.camera_config   = load_default_camera_config; */
        req.display.camera_type     = VIDEO_CAMERA_NORMAL;
    } else if ((id == 1) || (id == 2)) {
        /*if (req.display.width < 1280) {
            req.display.width 	+= 32;
            req.display.height 	+= 32;

            req.display.border_left   = 16;
            req.display.border_top    = 16;
            req.display.border_right  = 16;
            req.display.border_bottom = 16;
        }*/

        req.display.camera_config   = NULL;
        req.display.camera_type     = VIDEO_CAMERA_NORMAL;
    } else if (id == 3) {
        /* #ifdef THREE_WAY_ENABLE */
#if 0
        struct uvc_capability uvc_cap;

        req.display.three_way_type = VIDEO_THREE_WAY_JPEG;
        void *uvc_fd = dev_open("uvc", (void *)__this->uvc_id);
        if (!uvc_fd) {
            printf("uvc dev_open err!!!\n");
            return 0;
        }
        dev_ioctl(uvc_fd, UVCIOC_QUERYCAP, (unsigned int)&uvc_cap);
        int uvc_w = uvc_cap.reso[0].width;
        int uvc_h = uvc_cap.reso[0].height;
        dev_close(uvc_fd);

        if ((win->width > uvc_w * 2) || (win->height > uvc_h * 2)) {
            printf("uvc dev_open err b!!!\n");
            return 0;
        }

        req.display.width = (win->width > uvc_w) ? (uvc_w * 2) : uvc_w;
        req.display.border_left   = (req.display.width - win->width) / 2;
        req.display.border_right  = req.display.border_left;

        req.display.height = (win->height > uvc_h) ? (uvc_h * 2) : uvc_h;
        req.display.border_top = (req.display.height - win->height) / 2;
        req.display.border_bottom = req.display.border_top;
#else
        /*if (req.display.width < 1280) {
            req.display.width 	+= 32;
            req.display.height 	+= 32;

            req.display.border_left   = 16;
            req.display.border_top    = 16;
            req.display.border_right  = 16;
            req.display.border_bottom = 16;
        }*/
#endif
        req.display.uvc_id = __this->uvc_id;
        req.display.camera_config = NULL;
        req.display.camera_type = VIDEO_CAMERA_UVC;
        req.display.src_w = __this->src_width[3];
        req.display.src_h = __this->src_height[3];
        //旋转参数配置:
        //0:不旋转,不镜像 (原图)
        //1:逆时针旋转90度,不镜像
        //2:逆时针旋转270度,不镜像
        //3:逆时针旋转90度后,再垂直镜像
        //4:逆时针旋转90度后,再水平镜像
        req.display.rotate = 0; //usb后视频图像旋转显示
    }

#ifdef CONFIG_VIDEO4_ENABLE
    req.display.camera_config   = NULL;
    req.display.camera_type     = VIDEO_CAMERA_MUX;
#endif

    req.display.state 	        = VIDEO_STATE_START;
    req.display.pctl            = NULL;

    sys_key_event_disable();
    sys_touch_event_disable();
    err = server_request(__this->video_display[id], VIDEO_REQ_DISPLAY, &req);
    if (err) {
        printf("display req err = %d!!\n", err);
        server_close(__this->video_display[id]);
        __this->video_display[id] = NULL;
    }
#ifndef CONFIG_VIDEO4_ENABLE

    video_rec_start_isp_scenes();

    if (id == 0) {
        /*rec显示重设曝光补偿*/
        __this->exposure_set = 1;
        video_rec_set_exposure(db_select("exp"));
    }
#endif
#endif
    sys_key_event_enable();
    sys_touch_event_enable();

    return err;
}


static void video_disp_stop(int id)
{
#ifdef CONFIG_DISPLAY_ENABLE
    union video_req req = {0};
printf("\n %s %d\n", __func__, __LINE__);
    if (__this->video_display[id]) {
        if (id == 0) {
            video_rec_stop_isp_scenes(1, 0);
        }
printf("\n %s %d\n", __func__, __LINE__);
        req.display.state 	= VIDEO_STATE_STOP;
        server_request(__this->video_display[id], VIDEO_REQ_DISPLAY, &req);
printf("\n %s %d\n", __func__, __LINE__);
        server_close(__this->video_display[id]);
        __this->video_display[id] = NULL;
printf("\n %s %d\n", __func__, __LINE__);
        if (id == 0) {
            video_rec_start_isp_scenes();
        }
        printf("\n %s %d\n", __func__, __LINE__);
    }
#endif
}

static int video_disp_win_switch(int mode, int dev_id)
{
    int i;
    int err = 0;
    int next_win = 0;
    int curr_win = __this->disp_state;
    static u16 mirror[4] = {0};

#ifdef CONFIG_DISPLAY_ENABLE

#ifdef CONFIG_VIDEO0_INIT_ASYNC
    extern int os_query_init_video_sem(void);
    if (os_query_init_video_sem()) {
        printf("\n os_query_init_video_sem err\n");
        hi3861l_reset();
    }
#endif // CONFIG_VIDEO0_INIT_ASYNC

    switch (mode) {
    case DISP_WIN_SW_SHOW_PARKING:
        if (!__this->video_online[__this->disp_park_sel]) {
            return -ENODEV;
        }
        next_win        = DISP_PARK_WIN;
        break;
    case DISP_WIN_SW_HIDE_PARKING:
        next_win = curr_win;
        curr_win = DISP_PARK_WIN;
        break;
    case DISP_WIN_SW_SHOW_SMALL:
        curr_win        = DISP_MAIN_WIN;
        next_win        = DISP_MAIN_WIN;
        break;
    case DISP_WIN_SW_SHOW_NEXT:
        if (video_rec_online_nums() < 2) {
            return 0;
        }
        if (get_parking_status() == 1) {
            return 0;
        }
        next_win = curr_win;

        if (++next_win >= DISP_PARK_WIN) {
            next_win = DISP_MAIN_WIN;
        }
        break;
    case DISP_WIN_SW_DEV_IN:
        if (curr_win != DISP_MAIN_WIN) {
            return 0;
        }
        next_win = curr_win;
        break;
    case DISP_WIN_SW_DEV_OUT:
        next_win = DISP_MAIN_WIN;
        break;
    case DISP_WIN_SW_MIRROR:
        mirror[dev_id] = !mirror[dev_id];
        video_set_disp_mirror(dev_id, mirror[dev_id]);
        next_win = curr_win;
        break;
    default:
        return -EINVAL;
    }

    printf("disp_win_switch: %d, %d\n", curr_win, next_win);

    for (i = 1; i < CONFIG_VIDEO_REC_NUM; i++) {
        video_disp_stop(i);
    }

    if (curr_win != next_win || mode == DISP_WIN_SW_MIRROR) {
        video_disp_stop(0);
        err = video_disp_start(0, &disp_window[next_win][0]);
    }

    for (i = 1; i < CONFIG_VIDEO_REC_NUM; i++) {
        if (__this->video_online[i]) {
#ifdef CONFIG_VIDEO4_ENABLE
            err = video_disp_start(i, &disp_window[next_win][i]);
#elif defined THREE_WAY_ENABLE
            err = video_disp_start(i, &disp_window[next_win][i]);
#else
            err = video_disp_start(i, &disp_window[next_win][1]);
            if (err == 0) {
                break;
            }
#endif
        }
    }

    if (next_win != DISP_PARK_WIN) {
        __this->disp_state = next_win;
    }

#ifndef CONFIG_VIDEO4_ENABLE
    if (__this->disp_state == DISP_BACK_WIN) {
        /* 进入后视窗口前照灯关闭 */
        video_rec_post_msg("HlightOff");

        /* 后拉全屏显示（非录像状态），固定屏显场景1 */
        isp_scr_work_hdl(1);
    }
#endif

#endif

    return err;
}



static void rec_dev_server_event_handler(void *priv, int argc, int *argv)
{
    /*
     *该回调函数会在录像过程中，写卡出错被当前录像APP调用，例如录像过程中突然拔卡
     */
   // printf("\n argv[0]=======================%d\n",argv[0]);
    switch (argv[0]) {
    case VIDEO_SERVER_UVM_ERR:
        log_e("APP_UVM_DEAL_ERR\n");
        break;
    case VIDEO_SERVER_PKG_ERR:
        if (__this->state == VIDREC_STA_START) {
#if (APP_CASE == __WIFI_CAR_CAMERA__)
            video_rec_err_notify("VIDEO_REC_ERR");
#endif
            video_rec_stop(0);
        }
        break;
    case VIDEO_SERVER_PKG_END:
        static int  timer_count;
        //u32 t = timer_get_ms();
        int  t1= timer_get_ms()-timer_count;
        printf("\n t1:%d\n",t1);
        timer_count=timer_get_ms();


        /* break; */
        /* video_rec_savefile((int)priv); */
        /* break; */
       // if (db_select("cyc")) {
      //  printf("mot\n");

        if (!db_select("mot")) {
            putchar("A");
             #ifdef LONG_POWER_IPC
             //判断录像时间范围
               if(judge_plan_video_timer()){
                 putchar("B");
                 video_rec_savefile((int)priv);

               //  void start_cloud_video( int wakeup_status);
               //  start_cloud_video(NORMAL_WAKEUP);
                 break;
               }else{
                  putchar("C");
                  video_rec_stop(0);
                 break;
               }
            #endif
            putchar("D");
            video_rec_savefile((int)priv);
        } else {
            putchar("E");
            video_rec_stop(0);
        }
        break;
    default :
        log_e("unknow rec server cmd %x , %x!\n", argv[0], (int)priv);
        break;
    }
}



extern void play_voice_file(const char *file_name);
static void ve_server_event_handler(void *priv, int argc, int *argv)
{
    printf("\n ==============page=============:%d,%d,%s\n ",argv[0],__LINE__,__FUNCTION__);
    switch (argv[0]) {
    case VE_MSG_MOTION_DETECT_STILL:
        /*
         *录像时，移动侦测打开的情况下，画面基本静止20秒，则进入该分支
         */
        printf("**************==VE_MSG_MOTION_DETECT_STILL==**********\n");
      //  printf("\n mot:%d,menu:%d\n",db_select("mot"),__this->menu_inout);
       #if  0
        if (!db_select("mot") || (__this->menu_inout)) {
            return;
        }
        if (__this->state == VIDREC_STA_START && __this->user_rec == 0) {
            video_rec_stop(0);
        }
       #endif
        break;
    case VE_MSG_MOTION_DETECT_MOVING:
        /*
         *移动侦测打开，当检测到画面活动一段时间，则进入该分支去启动录像
         */
      printf("**************VE_MSG_MOTION_DETECT_MOVING**********\n");

      printf("\n db_select(mot)==========%d,%d\n",db_select("mot"),__this->menu_inout);
      #if  1
        if (!db_select("mot") || (__this->menu_inout)) {
            return;
        }
      #endif
        #ifdef LONG_POWER_IPC
       if(!judge_plan_video_timer()){

        break ;
       }
        #endif
        if ((__this->state == VIDREC_STA_STOP) || (__this->state == VIDREC_STA_IDLE)) {
           // video_rec_start();

         doorbell_start_rec();

        #if  1
         if (get_avsdk_connect_flag()) {

            extern   void start_cloud_video( int wakeup_status);
            start_cloud_video(MOVT_WAKEUP);
          }
        #endif
        }

        break;
    case VE_MSG_LANE_DETECT_WARNING:

        if (!__this->lan_det_setting) {
            if (!db_select("lan")) {
                return;
            }
        }

      //  play_voice_file("mnt/spiflash/audlogo/lane.adp");

        puts("==lane dete waring==\n");
        break;
    case VE_MSG_LANE_DETCET_LEFT:
        puts("==lane dete waring==l\n");
        break;
    case VE_MSG_LANE_DETCET_RIGHT:
        puts("==lane dete waring==r\n");
        break;
    case VE_MSG_VEHICLE_DETECT_WARNING:
        //printf("x = %d,y = %d,w = %d,hid = %d\n",argv[1],argv[2],argv[3],argv[4]);
        //位置
        video_rec_post_msg("carpos:p=%4", ((u32)(argv[1]) | (argv[2] << 16))); //x:x y:y
        //颜色
        if (argv[3] > 45) {
            video_rec_post_msg("carpos:w=%4", ((u32)(argv[3]) | (3 << 16)));    //w:width c:color,0:transparent, 1:green,2:yellow,3:red
        } else {
            video_rec_post_msg("carpos:w=%4", ((u32)(argv[3]) | (1 << 16)));
        }
        //隐藏
        if (argv[4] == 0) {
            video_rec_post_msg("carpos:w=%4", ((u32)(1) | (0 << 16)));
        }
        //刷新
        video_rec_post_msg("carpos:s=%4", 1);
        break;
    default :
        break;
    }
}


/*
 *智能引擎服务打开，它包括移动侦测等一些功能,在打开这些功能之前，必须要打开这个智能引擎服务
 */
static s32 ve_server_open(u8 fun_sel)
{

    #ifdef  ENABLE_VE_ENGINER
     return 0;

    #endif
    //printf("-----lane start\n");
    //printf("-----fun_sel = %d\n",fun_sel);
#ifdef CONFIG_VIDEO4_ENABLE
    return 0;
#endif
    if (!__this->video_engine) {
        __this->video_engine = server_open("video_engine_server", NULL);
        if (!__this->video_engine) {
            puts("video_engine_server:faild\n");
            return -1;
        }

        server_register_event_handler(__this->video_engine, NULL, ve_server_event_handler);

        struct video_engine_req ve_req;
        ve_req.module = 0;
        ve_req.md_mode = 0;
        ve_req.cmd = 0;
        /* ve_req.hint_info.hint = ((1 << VE_MODULE_MOTION_DETECT) | (1 << VE_MODULE_LANE_DETECT) | (1 << VE_MODULE_FACE_DETECT)); */
//        ve_req.hint_info.hint = ((1 << VE_MODULE_MOTION_DETECT) | (1 << VE_MODULE_LANE_DETECT));
        ve_req.hint_info.hint = 0;

        //  if (fun_sel) {
        //      ve_req.hint_info.hint = 0;
        //      ve_req.hint_info.hint = (1 << VE_MODULE_LANE_DETECT);
        //  }

#ifdef CONFIG_VE_MOTION_DETECT_MODE_ISP
        ve_req.hint_info.mode_hint0 = (VE_MOTION_DETECT_MODE_ISP
                                       << (VE_MODULE_MOTION_DETECT * 4));
#else
        ve_req.hint_info.mode_hint0 = (VE_MOTION_DETECT_MODE_NORMAL
                                       << (VE_MODULE_MOTION_DETECT * 4));
#endif

        ve_req.hint_info.mode_hint1 = 0;
        server_request(__this->video_engine, VE_REQ_SET_HINT, &ve_req);
    }



    if (fun_sel) {

        ve_face_det_start(1);

        //ve_mdet_start();//@kyj
        //ve_lane_det_start(1);

    } else {
        printf("--------------server open %d\n",fun_sel);
        __this->car_head_y = 351;//db_select("lan") & 0x0000ffff;
        __this->vanish_y   = 0;//(db_select("lan") >> 16) & 0x0000ffff;
        ve_mdet_start();
        #ifndef LONG_POWER_IPC
        ve_lane_det_start(0);
        #endif
    }

    return 0;
}

static s32 ve_server_close()
{
    #ifdef  ENABLE_VE_ENGINER
      return 0;
    #endif

    if (__this->video_engine) {

        if (!__this->lan_det_setting) {
            ve_mdet_stop();
        }
        #ifndef LONG_POWER_IPC
        ve_lane_det_stop(0);
        #endif
        server_close(__this->video_engine);

        __this->video_engine = NULL;
    }
    return 0;
}

static int ve_mdet_start()
{
    #ifdef  ENABLE_VE_ENGINER
     return 0;
    #endif

    struct video_engine_req ve_req;

#ifdef CONFIG_VIDEO4_ENABLE
    return -EINVAL;
#endif
    //@kyj
    // if ((__this->video_engine == NULL) || !db_select("mot")) {
    //     return -EINVAL;
    // }

    ve_req.module = VE_MODULE_MOTION_DETECT;
#ifdef CONFIG_VE_MOTION_DETECT_MODE_ISP
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
#else
    ve_req.md_mode = VE_MOTION_DETECT_MODE_NORMAL;
#endif
    ve_req.cmd = 0;

    server_request(__this->video_engine, VE_REQ_MODULE_OPEN, &ve_req);


    server_request(__this->video_engine, VE_REQ_MODULE_GET_PARAM, &ve_req);


    /*
    *移动侦测的检测启动时间和检测静止的时候
    **/
    ve_req.md_params.level = 2;
    ve_req.md_params.move_delay_ms = MOTION_START_SEC *10;// 1000;
    ve_req.md_params.still_delay_ms = MOTION_STOP_SEC *10;// 1000;
    server_request(__this->video_engine, VE_REQ_MODULE_SET_PARAM, &ve_req);

    server_request(__this->video_engine, VE_REQ_MODULE_START, &ve_req);

    return 0;
}


static int ve_mdet_stop()
{
   #ifdef  ENABLE_VE_ENGINER
    return 0;
    #endif
    struct video_engine_req ve_req;

    if ((__this->video_engine == NULL) || !db_select("mot")) {
        return -EINVAL;
    }

    ve_req.module = VE_MODULE_MOTION_DETECT;
#ifdef CONFIG_VE_MOTION_DETECT_MODE_ISP
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
#else
    ve_req.md_mode = VE_MOTION_DETECT_MODE_NORMAL;
#endif

    ve_req.cmd = 0;
    server_request(__this->video_engine, VE_REQ_MODULE_STOP, &ve_req);

    server_request(__this->video_engine, VE_REQ_MODULE_CLOSE, &ve_req);
    return 0;
}

static void ve_mdet_reset()
{

    #ifdef  ENABLE_VE_ENGINER
    return ;
    #endif
    ve_mdet_stop();
    ve_mdet_start();
}


static int ve_lane_det_start(u8 fun_sel)
{
    #ifdef LONG_POWER_IPC
     return 0;
    #endif
    struct video_engine_req ve_req;

#ifdef CONFIG_VIDEO4_ENABLE
    return -EINVAL;
#endif
    if (!fun_sel) {
        if ((__this->video_engine == NULL) || !db_select("lan")) {
            return -EINVAL;
        }
    }

    ve_req.module = VE_MODULE_LANE_DETECT;
#ifdef CONFIG_VE_MOTION_DETECT_MODE_ISP
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
#else
    ve_req.md_mode = VE_MOTION_DETECT_MODE_NORMAL;
#endif

    ve_req.cmd = 0;
    server_request(__this->video_engine, VE_REQ_MODULE_OPEN, &ve_req);

    server_request(__this->video_engine, VE_REQ_MODULE_GET_PARAM, &ve_req);

    /**
     *轨道偏移 配置车头 位置，视线结束为止，以及车道宽度
     * */
    /* ve_req.lane_detect_params.car_head_y = 230; */
    /* ve_req.lane_detect_params.vanish_y = 170; */
    /* ve_req.lane_detect_params.len_factor = 0; */
    ve_req.lane_detect_params.car_head_y = __this->car_head_y;
    ve_req.lane_detect_params.vanish_y =  __this->vanish_y;
    ve_req.lane_detect_params.len_factor = 0;

    server_request(__this->video_engine, VE_REQ_MODULE_SET_PARAM, &ve_req);

    server_request(__this->video_engine, VE_REQ_MODULE_START, &ve_req);

    return 0;
}


static int ve_lane_det_stop(u8 fun_sel)
{
    #ifdef LONG_POWER_IPC
     return 0;
    #endif
    struct video_engine_req ve_req;

    if (!fun_sel) {
        if ((__this->video_engine == NULL) || !db_select("lan")) {
            return -EINVAL;
        }
    }

    ve_req.module = VE_MODULE_LANE_DETECT;
#ifdef CONFIG_VE_MOTION_DETECT_MODE_ISP
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
#else
    ve_req.md_mode = VE_MOTION_DETECT_MODE_NORMAL;
#endif

    ve_req.cmd = 0;

    server_request(__this->video_engine, VE_REQ_MODULE_STOP, &ve_req);

    server_request(__this->video_engine, VE_REQ_MODULE_CLOSE, &ve_req);

    return 0;
}

void ve_lane_det_reset()
{

    #ifdef LONG_POWER_IPC
     return ;
    #endif

    ve_lane_det_stop(0);
    ve_lane_det_start(0);
    ve_face_det_start(0);
}



static int ve_face_det_start(u8 fun_sel)
{
    #ifdef LONG_POWER_IPC
     return 0 ;
    #endif
    struct video_engine_req ve_req;

    return 0;
    if (!fun_sel) {
        if (__this->video_engine == NULL) {
            return -EINVAL;
        }
    }


    ve_req.module = VE_MODULE_FACE_DETECT;
#ifdef CONFIG_VE_MOTION_DETECT_MODE_ISP
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
#else
    ve_req.md_mode = VE_MOTION_DETECT_MODE_NORMAL;
#endif



    ve_req.cmd = 0;
    server_request(__this -> video_engine, VE_REQ_MODULE_OPEN, &ve_req);
    server_request(__this -> video_engine, VE_REQ_MODULE_GET_PARAM, &ve_req);

    ve_req.face_detect_params.roi_top = 0;
    ve_req.face_detect_params.roi_bottom = 351;

    server_request(__this -> video_engine, VE_REQ_MODULE_SET_PARAM, &ve_req);
    server_request(__this -> video_engine, VE_REQ_MODULE_START, &ve_req);

    return 0;

}


static int ve_face_det_stop(u8 fun_sel)
{
     #ifdef LONG_POWER_IPC
     return 0;
    #endif

    struct video_engine_req ve_req;

    if (!fun_sel) {
        if ((__this -> video_engine == NULL) || !db_select("fac")) {
            return -EINVAL;
        }
    }


    ve_req.module = VE_MODULE_FACE_DETECT;
#ifdef CONFIG_VE_MOTION_DETECT_MODE_ISP
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
#else
    ve_req.md_mode = VE_MOTION_DETECT_MODE_NORMAL;
#endif

    ve_req.cmd = 0;

    server_request(__this -> video_engine, VE_REQ_MODULE_STOP, &ve_req);
    server_request(__this -> video_engine, VE_REQ_MODULE_CLOSE, &ve_req);


    return 0;

}


void ve_face_det_reset()
{

    #ifdef LONG_POWER_IPC
     return ;
    #endif
    ve_face_det_stop(0);
    ve_face_det_start(0);
}



void ve_server_reopen()
{
    #ifdef  ENABLE_VE_ENGINER
      return ;
    #endif

    ve_mdet_stop();

    #ifndef LONG_POWER_IPC
    ve_lane_det_stop(0);
    #endif


    ve_server_close();
    ve_server_open(0);
}

/*
 * 判断SD卡是否挂载成功和簇大小是否大于32K
 */
static int storage_device_available()
{
    struct vfs_partition *part;

    if (storage_device_ready() == 0) {
        if (!dev_online(SDX_DEV)) {
            video_rec_post_msg("noCard");
        } else {
            video_rec_post_msg("fsErr");
        }
        return false;
    } else {
        part = fget_partition(CONFIG_ROOT_PATH);
        printf("part_fs_attr: %x\n", part->fs_attr);
        if (part->clust_size < 32 || (part->fs_attr & F_ATTR_RO)) {
            video_rec_post_msg("fsErr");
            return false;
        }
        __this->total_size = part->total_size;
    }

    return true;
}

/*码率控制，根据具体分辨率设置*/
static int video_rec_get_abr(u32 width)
{
#if (APP_CASE ==  __CAR_CAMERA__)
    if (width <= 720) {
        return 1000;
    } else if (width <= 1280) {
        return 4000;
    } else if (width <= 1920) {
        return 10000;
    } else {
        return 10000;
    }
#endif


#if (APP_CASE ==  __WIFI_CAR_CAMERA__)
    if (width <= 720) {
        return 1000;
    } else if (width <= 1280) {
        return 2000;
    } else if (width <= 1920) {
        return 3000;
    } else {
        return 4000;
    }
#endif

#if  (APP_CASE == __WIFI_IPCAM__)
    if (width <= 720) {
        return 1000;
    } else if (width <= 1280) {
        return 1000;
    } else if (width <= 1920) {
        return 2000;
    } else {
        return 2000;
    }
#endif

#if  (APP_CASE == __WIFI_DOORBELL__)
    if (width <= 720) {
        return 1000;
    } else if (width <= 1280) {
        return 1000;
    } else if (width <= 1920) {
        return 1500;
    } else {
        return 2000;
    }
#endif


}

static void video_rec_get_remain_time(void)
{
    static char retime_buf[30];
    int err;
    u32 cur_space;
    u32 one_pic_size;
    u32 one_frame_size = 0;
    int second = 0;
    u32 gap_time = db_select("gap");
    if (!gap_time) {
        gap_time = 1000 / 30;
    }

    /*
     * 这里填入SD卡剩余录像时间
     */
    if (storage_device_available()) {
        log_d("calc_free_space...\n");
        err = fget_free_space(CONFIG_ROOT_PATH, &cur_space);
        if (err) {
            if (fget_err_code(CONFIG_ROOT_PATH) == -EIO) {
                video_rec_post_msg("fsErr");
            }
        } else {
            u32 res = db_select("res");
            one_pic_size = video_rec_get_abr(rec_pix_w[res]) / 240 + 1;
            /* if (res == VIDEO_RES_1080P) { */
            /* one_pic_size = (0x21000 + 0xa000) / 1024; */
            /* } else if (res == VIDEO_RES_720P) { */
            /* one_pic_size = (0x13000 + 0xa000) / 1024; */
            /* } else { */
            /* one_pic_size = (0xa000 + 0xa000) / 1024; */
            /* } */
            one_frame_size += one_pic_size;
#if (defined CONFIG_VIDEO1_ENABLE && !defined CONFIG_SINGLE_VIDEO_REC_ENABLE)
            if (__this->video_online[1] && db_select("two")) {
                one_frame_size += video_rec_get_abr(rec_pix_w[VIDEO_RES_1080P]) / 240 + 1;
            }
#endif
#ifdef CONFIG_VIDEO4_ENABLE
            for (int i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
                if (__this->video_online[i]) {
                    one_frame_size += one_pic_size;
                }
            }
#endif
            second = cur_space / one_frame_size * gap_time / 1000;

            printf("retime_buf %02d:%02d:%02d", second / 3600, second % 3600 / 60, second % 60);

            video_rec_post_msg("Remain:s=%4", ((u32)(second)));
        }
        log_d("calc_free_space_exit\n");
    }
}

/*
 *根据录像不同的时间和分辨率，设置不同的录像文件大小
 */
static u32 video_rec_get_fsize(u8 cycle_time, u16 vid_width, int format)
{
    u32 fsize;

#if 0
    if (cycle_time == 0) {
        cycle_time = 5;
    }
#endif

    fsize = (video_rec_get_abr(vid_width)) * cycle_time * 8250 ;

    if (format == VIDEO_FMT_AVI) {
        fsize = fsize + fsize / 4;
    }

    return fsize;
}

static int video_rec_cmp_fname(void *afile, void *bfile)
{
    int alen, blen;
    char afname[MAX_FILE_NAME_LEN];
    char bfname[MAX_FILE_NAME_LEN];

    if ((afile == NULL) || (bfile == NULL)) {
        return 0;
    }
    printf("video_rec_cmp_fname: %p, %p\n", afile, bfile);

    alen = fget_name(afile, (u8 *)afname, MAX_FILE_NAME_LEN);
    if (alen <= 0) {
        log_e("fget_name: afile=%x\n", afile);
        return 0;
    }
    ASCII_ToUpper(afname, alen);

    blen = fget_name(bfile, (u8 *)bfname, MAX_FILE_NAME_LEN);
    if (blen <= 0) {
        log_e("fget_name: bfile=%x\n", bfile);
        return 0;
    }
    ASCII_ToUpper(bfname, blen);

    printf("afname: %s, bfname: %s\n", afname, bfname);

    if (alen == blen && !strcmp(afname, bfname)) {
        return 1;
    }

    return 0;
}

static void video_rec_fscan_release(int lock_dir)
{
    printf("video_rec_fscan_release: %d\n", lock_dir);
    for (int i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
        if (__this->fscan[lock_dir][i]) {
            fscan_release(__this->fscan[lock_dir][i]);
            __this->fscan[lock_dir][i] = NULL;
        }
    }
}

static void video_rec_fscan_dir(int id, int lock_dir, const char *path)
{
    const char *str;
#ifdef CONFIG_EMR_DIR_ENABLE
    str = "-tMOVAVI -sn";
#else
    str = lock_dir ? "-tMOVAVI -sn -ar" : "-tMOVAVI -sn -a/r";
#endif
    if (__this->fscan[lock_dir][id]) {
        if (__this->old_file_number[lock_dir][id] == 0) {
            puts("--------delete_all_scan_file\n");
            fscan_release(__this->fscan[lock_dir][id]);
            __this->fscan[lock_dir][id] = NULL;
        }
    }

    if (!__this->fscan[lock_dir][id]) {
        __this->fscan[lock_dir][id] = fscan(path, str);
        if (!__this->fscan[lock_dir][id]) {
            __this->old_file_number[lock_dir][id] = 0;
        } else {
            __this->old_file_number[lock_dir][id] = __this->fscan[lock_dir][id]->file_number;
        }
        __this->file_number[lock_dir][id] = __this->old_file_number[lock_dir][id];
        printf("fscan_dir: %d, file_number = %d\n", id, __this->file_number[lock_dir][id]);
    }
}
int video_rec_delect_notify(FILE *fd, int id);
static FILE *video_rec_get_first_file(int id)
{
    int max_index = -1;
    int max_file_number = 0;
    int persent = __this->lock_fsize * 100 / __this->total_size;
    int lock_dir = !!(persent > LOCK_FILE_PERCENT);
    int i;

    log_d("lock_file_persent: %d, size: %dMB\n", persent, __this->lock_fsize / 1024);

#ifdef CONFIG_VIDEO0_ENABLE
    video_rec_fscan_dir(0, lock_dir, rec_path[0][lock_dir]);
#endif
#if (defined CONFIG_VIDEO1_ENABLE && !defined CONFIG_SINGLE_VIDEO_REC_ENABLE)
    video_rec_fscan_dir(1, lock_dir, rec_path[1][lock_dir]);
#endif
#ifdef CONFIG_VIDEO3_ENABLE
    video_rec_fscan_dir(3, lock_dir, rec_path[3][lock_dir]);
#endif

#ifdef CONFIG_VIDEO4_ENABLE
    for (i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
        video_rec_fscan_dir(i, lock_dir, rec_path[i][lock_dir]);
    }
#endif


    for (i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
        if (__this->fscan[lock_dir][i]) {
            if (max_file_number < __this->file_number[lock_dir][i]) {
                max_file_number = __this->file_number[lock_dir][i];
                max_index = i;
            }
        }
    }

    if (max_index < 0) {
        return NULL;
    }
    if (max_index != id && id >= 0) {
        /* 查看优先删除的文件夹是否满足删除条件 */
        if (__this->file_number[lock_dir][id] + 3 > __this->file_number[lock_dir][max_index]) {
            max_index = id;
        }
    }


    log_d("fselect file from dir %d, %d\n", lock_dir, max_index);


    if (__this->fscan[lock_dir][max_index]) {
        FILE *f = fselect(__this->fscan[lock_dir][max_index], FSEL_FIRST_FILE, 0);
        if (f) {

            if (lock_dir == 0) {
                if (video_rec_cmp_fname(__this->file[max_index], f)) {
                    fclose(f);
                    return NULL;
                }
            } else {
                __this->lock_fsize -= flen(f) / 1024;
                log_d("lock fsize - = %d\n", __this->lock_fsize);
            }

            __this->file_number[lock_dir][max_index]--;
            __this->old_file_number[lock_dir][max_index]--;
            if (__this->old_file_number[lock_dir][max_index] == 0) {
                video_rec_fscan_release(lock_dir);
            }
        }

        video_rec_delect_notify(f, -1);

        return f;
    } else {
        log_e("fscan[%d][%d] err", lock_dir, max_index);
        return NULL;
    }
    return NULL;
}


static void video_rec_rename_file(int id, FILE *file, int fsize, int format)
{
    char file_name[32];

    __this->new_file[id] = NULL;

    int err = fcheck(file);
    if (err) {
        puts("fcheck fail\n");

        video_rec_delect_notify(file, -1);

        fdelete(file);
        return;
    }

    int size = flen(file);
    int persent = (size / 1024) * 100 / (fsize / 1024);

    printf("rename file: persent=%d, %d,%d\n", persent, size >> 20, fsize >> 20);

    if (persent >= 90 && persent <= 110) {
        sprintf(file_name, "%s%s", rec_dir[id][0], rec_file_name(format));

        printf("fmove: %d, %d, %s\n", id, format, file_name);

        video_rec_delect_notify(file, -1);

        int err = fmove(file, file_name, &__this->new_file[id], 1);
        if (err == 0) {
            fseek(__this->new_file[id], fsize, SEEK_SET);
            fseek(__this->new_file[id], 0, SEEK_SET);
            return;
        }
        puts("fmove_file_faild\n");
    }


    video_rec_delect_notify(file, -1);


    fdelete(file);
}




static int video_rec_create_file(int id, u32 fsize, int format, const char *path)
{
    FILE *file;
    int try_cnt = 0;
    char file_path[64];

    sprintf(file_path, "%s%s", path, rec_file_name(format));

    printf("fopen: %s, %dMB\n", file_path, fsize >> 20);

    do {
        file = fopen(file_path, "w+");
        if (!file) {
            log_e("fopen faild\n");
            break;
        }
//不使用预创建文件的方式
#if 0
        if (fseek(file, fsize, SEEK_SET)) {
            goto __exit;
        }
        log_e("fseek faild\n");
        fdelete(file);
#else
        goto __exit;
#endif

    } while (++try_cnt < 2);

    return -EIO;

__exit:
    fseek(file, 0, SEEK_SET);
    __this->new_file[id] = file;

    return 0;
}



static int video_rec_del_old_file()
{

    int i;
    int online_num = 0;
    int err;
    FILE *file;
    u32 cur_space;
    u32 need_space = 0;
    u32 gap_time = db_select("gap");
    int cyc_time = 15;//db_select("cyc");
#ifdef THREE_WAY_ENABLE
    int fsize[4] = {0, 0, 0, 0};
    int format[4] = { VIDEO0_REC_FORMAT, VIDEO1_REC_FORMAT, VIDEO2_REC_FORMAT, VIDEO3_REC_FORMAT};
#else
    int fsize[4] = {0, 0, 0, 0};

#ifdef CONFIG_VIDEO4_ENABLE
    int format[4] = {VIDEO4_REC_FORMAT, VIDEO4_REC_FORMAT, VIDEO4_REC_FORMAT, VIDEO4_REC_FORMAT};
#else
    int format[4] = { VIDEO0_REC_FORMAT, VIDEO1_REC_FORMAT, VIDEO2_REC_FORMAT, VIDEO3_REC_FORMAT};
#endif
#endif


#ifdef CONFIG_VIDEO0_ENABLE
    if (!__this->new_file[0]) {
        int width;
        int qos = db_select("vqua");
        if (qos >= 16 && qos <= 20) {
            #if 0
            width  = 1440;
            #else
            width  =GET_VIDEO_WIDTH;
            #endif
        } else if (qos >= 6 && qos <= 10) {
            width  = 1280;
        } else if (qos >= 1 && qos <= 5) {
            width  = 640;
        }
        fsize[0] = video_rec_get_fsize(cyc_time, width, VIDEO0_REC_FORMAT);
        if (gap_time) {
            fsize[0] = fsize[0] / (30 * gap_time / 1000);
        }
        need_space += fsize[0];
    }
#endif

    if (db_select("two")) {
#if (defined CONFIG_VIDEO1_ENABLE && !defined CONFIG_SINGLE_VIDEO_REC_ENABLE)
        if (__this->video_online[1] && !__this->new_file[1]) {
            fsize[1] =  video_rec_get_fsize(cyc_time, AVIN_WIDTH, VIDEO1_REC_FORMAT);
            if (gap_time) {
                fsize[1] = fsize[1] / (30 * gap_time / 1000);
            }
            need_space += fsize[1];
        }
#endif
    }

#ifdef CONFIG_VIDEO3_ENABLE
    if (__this->video_online[3] && !__this->new_file[3]) {
        fsize[3] =  video_rec_get_fsize(cyc_time, UVC_ENC_WIDTH, VIDEO3_REC_FORMAT);
        if (gap_time) {
            fsize[3] = fsize[3] / (30 * gap_time / 1000);
        }
        need_space += fsize[3];
    }
#endif



#ifdef CONFIG_VIDEO4_ENABLE
    for (i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
        if (__this->video_online[i] && !__this->new_file[i]) {
            fsize[i] =  video_rec_get_fsize(cyc_time, rec_pix_w[VIDEO_RES_720P], VIDEO4_REC_FORMAT);
            if (gap_time) {
                fsize[i] = fsize[i] / (25 * gap_time / 1000);
            }
            need_space += fsize[i];
        }
    }

#endif


    err = fget_free_space(CONFIG_ROOT_PATH, &cur_space);
    if (err) {
        return err;
    }

    printf("space: %dMB, %dMB\n", cur_space / 1024, need_space / 1024 / 1024);

#ifdef CONFIG_VIDEO4_ENABLE
    online_num = ARRAY_SIZE(fsize);
#else
    online_num = video_rec_online_nums();
#endif
    if (cur_space >= (need_space / 1024) * 3) {
        for (i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
            if (fsize[i] != 0) {
                err = video_rec_create_file(i, fsize[i], format[i], rec_path[i][0]);
                if (err) {
                    return err;
                }
            }
        }
        return 0;
    }


    while (1) {
        if (cur_space >= (need_space / 1024) * 2) {
            break;
        }
        file = video_rec_get_first_file(-1);
        if (!file) {
            return -ENOMEM;
        }
        fdelete(file);
        fget_free_space(CONFIG_ROOT_PATH, &cur_space);
        printf("space: %dMB, %dMB\n", cur_space / 1024, need_space / 1024 / 1024);
    }

    for (i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
        if (fsize[i] != 0) {
            file = video_rec_get_first_file(i);
            if (file) {
                video_rec_rename_file(i, file, fsize[i], format[i]);
            }
            if (!__this->new_file[i]) {
                err = video_rec_create_file(i, fsize[i], format[i], rec_path[i][0]);
                if (err) {
                    return err;
                }
            }
        }
    }

    return 0;
}


static int video_rec_scan_lock_file()
{
#ifdef CONFIG_EMR_DIR_ENABLE
    const char *str = "-tMOVAVI -sn";
#else
    const char *str = "-tMOVAVI -sn -ar";
#endif

    __this->lock_fsize = 0;
    for (int i = 0; i < ARRAY_SIZE(rec_path); i++) {
        __this->fscan[1][i] = fscan(rec_path[i][1], str);
        if (__this->fscan[1][i] == NULL) {
            continue;
        }
        int sel_mode = FSEL_FIRST_FILE;
        while (1) {
            FILE *file = fselect(__this->fscan[1][i], sel_mode, 0);
            if (!file) {
                break;
            }
            __this->lock_fsize += (flen(file) / 1024);
            sel_mode = FSEL_NEXT_FILE;
            fclose(file);
        }
    }

    log_d("lock_file_size: %dMB\n", __this->lock_fsize / 1024);

    return 0;
}

/*
 *设置保护文件，必须要在关闭文件之前调用
 */
static int video_rec_lock_file(void *file, u8 lock)
{
    int attr;

    if (!file) {
        puts("lock file null\n");
        return -1;
    }

    fget_attr(file, &attr);

    if (lock) {
        if (attr & F_ATTR_RO) {
            return 0;
        }
        attr |= F_ATTR_RO;
    } else {
        if (!(attr & F_ATTR_RO)) {
            return 0;
        }
        attr &= ~F_ATTR_RO;
    }
    fset_attr(file, attr);

    return 0;
}

static void video_rec_close_file(int dev_id)
{
    if (!__this->file[dev_id]) {
        return;
    }

    char is_emf = 0;
    char *video_rec_finish_get_name(FILE * fd, int index, u8 is_emf) ; //index ：video0前视0，video1则1，video2则2 , is_emf 紧急文件
    char *path = video_rec_finish_get_name(__this->file[dev_id], dev_id, is_emf);
    log_d("video_rec_close_file:%s \n", path);


    log_i("close file in\n");

    if (__this->gsen_lock & BIT(dev_id)) {
        __this->gsen_lock &= ~BIT(dev_id);
        __this->lock_fsize += flen(__this->file[dev_id]) / 1024;
        log_d("lock fsize + = %d\n", __this->lock_fsize);
        video_rec_lock_file(__this->file[dev_id], 1);
#ifdef CONFIG_EMR_DIR_ENABLE


        is_emf = TRUE;
        path = video_rec_finish_get_name(__this->file[dev_id], dev_id, is_emf);


        log_d("move_file_to_dir: %s\n", rec_dir[dev_id][1]);
        int err = fmove(__this->file[dev_id], rec_dir[dev_id][1], NULL, 0);
        if (!err) {
            __this->file[dev_id] = NULL;

            if (path) { //必须关闭文件之后才能调用，否则在读取文件信息不全！！！
                video_rec_finish_notify(path);
            }

            return;
        }
#endif
    }

    fclose(__this->file[dev_id]);
    __this->file[dev_id] = NULL;


    if (path) { //必须关闭文件之后才能调用，否则在读取文件信息不全！！！
        extern int video_rec_finish_notify(char *path);
        video_rec_finish_notify(path);
    }
#if 0
    #include "time.h"
    uint64_t get_local_msg_start_time(void);
    uint64_t  msg_start_time = get_local_msg_start_time();
    struct tm p = {0};
    covUnixTimeStp2Beijing(msg_start_time / 1000, &p);
    printf("%llu convert is: %d/%02d/%02d-%02d:%02d:%02d weed = %d year_day = %d\n", msg_start_time,
           p.tm_year, p.tm_mon, p.tm_mday, \
           p.tm_hour, p.tm_min, p.tm_sec, p.tm_wday, p.tm_yday);


    void *fd = fopen(path,"r");

    char name[128];
    int len = fget_name(fd, name, sizeof(name));

    struct tm p1 = {0};
    get_utc_time_for_name(&p1, name, strlen(name));
    uint64_t  creation_time = covBeijing2UnixTimeStp(&p1);
    printf("%llu convert is: %d/%02d/%02d-%02d:%02d:%02d weed = %d year_day = %d\n", creation_time,
           p1.tm_year, p1.tm_mon, p1.tm_mday, \
           p1.tm_hour, p1.tm_min, p1.tm_sec, p1.tm_wday, p1.tm_yday);


    uint64_t  current_time = get_utc_ms();

    //录像启动时间
    int64_t get_local_msg_end_time(void);
    if(msg_start_time){
        if(current_time  < get_local_msg_end_time()){
            //需要继续留言

        }else{
            //留言结束
            set_local_msg_start_time(0);
        }
        name[0] = 'M';
        name[1] = 'S';
        name[2] = 'G';

        frename(fd,name);
    }
    fclose(fd);
#endif // 0

    log_i("close file finish\n");
}


static int video0_set_audio_callback(struct server *server, int (*callback)(u8 *buf, u32 len))
{
    union video_req req = {0};

    /* if (!__this->video_rec0) { */
    if (!server) {
        return -EINVAL;
    }

    req.rec.state 	= VIDEO_STATE_SET_AUDIO_CALLBACK;
    req.rec.audio_callback 	= callback;

    /* return server_request(__this->video_rec0, VIDEO_REQ_REC, &req); */
    return server_request(server, VIDEO_REQ_REC, &req);
}

#ifdef CONFIG_VIDEO0_ENABLE
/******* 不要单独调用这些子函数 ********/
static int video0_rec_start()
{
    puts("start_video_rec0\n");

    /* extern void h264_enc_manu_test(); */
    /* sys_timer_add(NULL, h264_enc_manu_test, 5000); */
    /* return 0; */

    /* extern avc_encode_test() ; */
    /* avc_encode_test() ; */
    /* return 0 ; */

    int err;
    union video_req req = {0};
    struct video_text_osd text_osd;
    struct video_graph_osd graph_osd;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;

    puts("start_video_rec0\n");
    if (!__this->video_rec0) {
        __this->video_rec0 = server_open("video_server", "video0.0");
        /* __this->video_rec0 = server_open("video_server", "video4.0"); */
        if (!__this->video_rec0) {
            return VREC_ERR_V0_SERVER_OPEN;
        }

        server_register_event_handler(__this->video_rec0, (void *)0, rec_dev_server_event_handler);
    }

    u32 res = db_select("res");
    printf("res=%d\n", res);
    /*
     *通道号，分辨率，封装格式，写卡的路径
     */
    req.rec.channel     = 0;
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;

    int qos = db_select("vqua");
    if (qos >= 16 && qos <= 20) {
       #if 0
        req.rec.width  = 1440;
        req.rec.height = 1088;
       #else

        req.rec.width  = GET_VIDEO_WIDTH;
        req.rec.height = GET_VIDEO_HEIGHT;
       #endif
    } else if (qos >= 6 && qos <= 10) {
        req.rec.width  = 1280;
        req.rec.height = 720;
    } else if (qos >= 1 && qos <= 5) {
        req.rec.width  = 640;
        req.rec.height = 480;
    }

    req.rec.format 	    = VIDEO0_REC_FORMAT;
    req.rec.state 	    = VIDEO_STATE_START;
    req.rec.file        = __this->file[0];

    /*
     *帧率为0表示使用摄像头的帧率
     */
    req.rec.quality     = VIDEO_MID_Q;
    req.rec.fps 	    = 0;
    req.rec.real_fps 	= 0;
#if (APP_CASE == __WIFI_IPCAM__)
    req.rec.fps 	    = 15;
    req.rec.real_fps 	= 15;
#endif

    /*
     *采样率，通道数，录像音量，音频使用的循环BUF,录不录声音
     */
    extern int net_video_rec_get_audio_rate();
    req.rec.audio.fmt_format = AUDIO_FMT_PCM;
    req.rec.audio.sample_rate = net_video_rec_get_audio_rate();
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = net_video_rec_get_adc_volume();
    req.rec.audio.buf = __this->audio_buf[0];
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    #ifdef CONFIG_USB_UVC_AND_UAC_ENABLE

    req.rec.audio.sample_source = "user";

    //printf("\n req.rec.audio.sample_source====================%s\n ",req.rec.audio.sample_source);

    #endif
    /*
     *码率，I帧和P帧比例，必须是偶数（当录MOV的时候才有效）,
     *roio_xy :值表示宏块坐标， [6:0]左边x坐标 ，[14:8]右边x坐标，[22:16]上边y坐标，[30:24]下边y坐标,写0表示1个宏块有效
     * roio_ratio : 区域比例系数
     */
    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);


    req.rec.qp_attr.static_min = 31;
    req.rec.qp_attr.static_max = 35;
    req.rec.qp_attr.dynamic_min = 32;
    req.rec.qp_attr.dynamic_max = 34;
    req.rec.qp_attr.enable_change_qp = 1;

    /* 编码参数配置，请仔细参阅说明文档后修改 */
    struct app_enc_info app_avc_cfg;

    app_avc_cfg.pre_pskip_limit_flag = 1; 				//SKIP块提前榖Ωِ制开关 */
    app_avc_cfg.pre_pskip_th = 0;						//SKIP块提剈Ŧ֭阈值 */
    app_avc_cfg.common_pskip_limit_flag = 0; 			//SKIP块常规榖Ωِ制开关 */
    app_avc_cfg.common_pskip_th = 0;					//SKIP块常览Ŧ֭阈值 */
    app_avc_cfg.dsw_size = 0;							//搜索窗口大小（5x7  1:3x7） */
    app_avc_cfg.fs_x_points = 36;						//水平秴ç¹数 */
    app_avc_cfg.fs_y_points = 32;						//垂直秴ç¹数 */
    app_avc_cfg.f_aq_strength = 0.8f;					//宏块QP臀åڔ强度 */
    app_avc_cfg.qp_offset_low = 0;						//宏块QP槧ܤ؋限 */
    app_avc_cfg.qp_offset_high = 7;					//宏块QP槧ܤ؊限 */
    app_avc_cfg.nr_reduction = 0;						//DCT降噼eų */
    app_avc_cfg.user_cfg_enable = 1;					//编码参敔ɦȷ配置使能 */
    req.rec.app_avc_cfg = &app_avc_cfg;
    //req.rec.app_avc_cfg = NULL;



#if defined __CPU_AC5401__
    req.rec.IP_interval = 0;
#elif defined __CPU_AC5601__
    if (req.rec.height > 720) {
        req.rec.IP_interval = 1;
    } else {
        req.rec.IP_interval = 0;
    }
#else
    req.rec.IP_interval = 32;//128;
    /* req.rec.IP_interval = 0;//96; */
#endif

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    /* req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16; */
    /* req.rec.roi.roi1_xy = (req.rec.height * 11 / 12 / 16) << 24 | (req.rec.height * 4 / 12 / 16) << 16 | (req.rec.width * 11 / 12 / 16) << 8 | (req.rec.width) * 1 / 12 / 16; */
    /* req.rec.roi.roi2_xy = 0; */
    /* req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0; */
    /* req.rec.roi.roio_ratio = 256 * 70 / 100 ; */
    /* req.rec.roi.roio_ratio1 = 256 * 90 / 100; */
    /* req.rec.roi.roio_ratio2 = 0; */
    /* req.rec.roi.roio_ratio3 = 256 * 80 / 100; */






    extern void video_rec_set_osd(struct video_text_osd * text_osd, int width, int height);
    video_rec_set_osd(&text_osd, req.rec.width, req.rec.height);
#if (defined __CPU_DV15__ || defined __CPU_DV17__ )

    text_osd.direction = 1;
    graph_osd.bit_mode = 16;//2bit的osd需要配置3个col|=    graph_osd.x = 0;
    graph_osd.x = 0;
    graph_osd.y = 0;
    graph_osd.width = 256;
    graph_osd.height = 256;
    graph_osd.icon = icon_16bit_data;//icon_osd_buf;
    graph_osd.icon_size = sizeof(icon_16bit_data);//sizeof(icon_osd_buf);
#else
    text_osd.direction = 0;
#endif

    if (db_select("dat")) {
        req.rec.text_osd = &text_osd;
#if (defined __CPU_DV15__ || defined __CPU_DV17__ || defined __CPU_AC571X__)
        req.rec.graph_osd = NULL;// &graph_osd;
#endif
    }

    /*
     *慢动作倍数(与延时摄影互斥,无音频); 延时录像的间隔ms(与慢动作互斥,无音频)
     */
    req.rec.slow_motion = 0;
    if (req.rec.camera_type != VIDEO_CAMERA_UVC) {
        req.rec.tlp_time = db_select("gap");
        if (req.rec.tlp_time) {
            req.rec.real_fps = 1000 / req.rec.tlp_time;
            req.rec.pkg_fps	 = 30;
        }
    } else {
        req.rec.tlp_time = 0;
    }

    if (req.rec.slow_motion || req.rec.tlp_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    }
    req.rec.buf = __this->video_buf[0];
    req.rec.buf_len = VREC0_FBUF_SIZE;
#if (defined THREE_WAY_ENABLE || defined CONFIG_USB_VIDEO_OUT)
    req.rec.rec_small_pic 	= 0;
#else
    req.rec.rec_small_pic 	= 1;
#endif

    /*
     *循环录像时间，文件大小
     */
    req.rec.cycle_time = 60;//db_select("cyc");
#if 0
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }

    req.rec.cycle_time = req.rec.cycle_time * 60;
#endif // 0
    /* if (req.rec.tlp_time) { */
    /* //此处可以使录像文件长度为设定值，需同步修改预创建大小 */
    /* req.rec.cycle_time = req.rec.cycle_time * req.rec.pkg_fps * req.rec.tlp_time / 1000; */
    /* } */


    get_dec_server_mutex();

   // printf("\n req.rec.audio.sample_source====================%s\n ",req.rec.audio.sample_source);
    err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    put_dec_server_mutex();

    if (err != 0) {
        puts("\n\n\nstart rec err\n\n\n");
        return VREC_ERR_V0_REQ_START;
    }


//    video_rec_start_isp_scenes();
    ADC_CON &= ~(~(0xffffffff << 4) <<  12);
    ADC_CON |= ((14 & (~(0xffffffff << 4))) << 12);
    printf("\n  ADC_CON = 0x%x \n", ADC_CON);
    /* extern int audio_callback(u8 *buf, u32 len); */
    /* video0_set_audio_callback(__this->video_rec0,audio_callback); */
    /* extern void h264_enc_manu_test(); */
    /* sys_timer_add(NULL, h264_enc_manu_test, 5000); */
    /* sys_timeout_add(NULL, h264_enc_manu_test, 5000); */
    /* extern  void stop_and_rec() ; */
    /* sys_timeout_add(NULL, stop_and_rec, 60 * 1000 * 1); */

    return 0;
}




static int video0_rec_len()
{
    union video_req req = {0};

    if (!__this->video_rec0) {
        return -EINVAL;
    }

    req.rec.state 	= VIDEO_STATE_PKG_LEN;
    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;

    return server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
}

static int video0_rec_aud_mute()
{
    union video_req req = {0};

    if (!__this->video_rec0) {
        return -EINVAL;
    }

    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_PKG_MUTE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    return server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
}


static int video0_rec_set_dr(u8 fps)
{
    union video_req req = {0};

    if (!__this->video_rec0) {
        return -EINVAL;
    }

    req.rec.real_fps = fps;
    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_SET_DR;


    return server_request(__this->video_rec0, VIDEO_REQ_REC, &req);

}

static int video_set_crop(u16 offset_x, u16 offset_y, u16 width, u16 height)
{
#define SOURCE_WIDTH 1920
#define SOURCE_HEIGHT 1080

    union video_req req = {0};
    static struct video_source_crop crop = {0};

    if (!__this->video_rec0) {
        /* if (!__this->video_display[0]) { */
        return -EINVAL;
    }

    if (offset_x + width > SOURCE_WIDTH || offset_y + height > SOURCE_HEIGHT) {
        log_e("crop_overflow\n");
        return -EINVAL;
    } else if ((SOURCE_WIDTH / width > 3) || (SOURCE_HEIGHT / height > 3)) {
        log_e("crop_overflow!\n");
        return -EINVAL;
    } else {
        crop.x_offset = offset_x;
        crop.y_offset = offset_y;
        crop.width = width;
        crop.height = height;
    }
    log_i("crop : x %d, y %d, crop_w %d, crop_h %d\n", offset_x, offset_y, width, height);

    req.rec.channel     = 0;
    req.rec.state 	= VIDEO_STATE_SET_ENC_CROP;
    req.rec.crop = &crop;
    /* req.display.state 	= VIDEO_STATE_SET_DIS_CROP; */
    /* req.display.crop = &crop; */

    return server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    /* return server_request(__this->video_display[0], VIDEO_REQ_DISPLAY, &req); */
}

static int video0_rec_stop(u8 close)
{
    union video_req req = {0};
    int err;

    log_d("video0_rec_stop\n");

    video_rec_stop_isp_scenes(2, 0);

    if (__this->video_rec0) {
        req.rec.channel = 0;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
        if (err != 0) {
#if (APP_CASE == __WIFI_CAR_CAMERA__)
            video_rec_err_notify("VIDEO_REC_ERR");
#endif
            printf("\nstop rec err 0x%x\n", err);
            return VREC_ERR_V0_REQ_STOP;
        }
    }

    video_rec_close_file(0);

    video_rec_start_isp_scenes();
    if (close) {
        if (__this->video_rec0) {
            server_close(__this->video_rec0);
            __this->video_rec0 = NULL;
        }
    }

    return 0;
}

/*
 *注意：循环录像的时候，虽然要重新传参，但是要和start传的参数保持一致！！！
 */
static int video0_rec_savefile()
{
    union video_req req = {0};
    int err;

    if (!__this->file[0]) {
        log_i("\n\nf0null\n\n");
        return -ENOENT;
    }

    u32 res = db_select("res");

    req.rec.channel = 0;
    req.rec.width 	= rec_pix_w[res];
    req.rec.height 	= rec_pix_h[res];
    req.rec.format 	= VIDEO0_REC_FORMAT;
    req.rec.state 	= VIDEO_STATE_SAVE_FILE;
    req.rec.file    = __this->file[0];
    req.rec.fps 	= 0;
    req.rec.real_fps 	= 0;

#if (defined THREE_WAY_ENABLE || defined CONFIG_USB_VIDEO_OUT)
    req.rec.rec_small_pic 	= 0;
#else
    req.rec.rec_small_pic 	= 1;
#endif


    req.rec.cycle_time = 1 * 60;//db_select("cyc");
#if 0
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }

    req.rec.cycle_time = req.rec.cycle_time * 60;
#else
    req.rec.cycle_time = req.rec.cycle_time;
#endif
    req.rec.audio.sample_rate = net_video_rec_get_audio_rate();
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = 100;
    req.rec.audio.buf = __this->audio_buf[0];
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    if (req.rec.camera_type != VIDEO_CAMERA_UVC) {
        req.rec.tlp_time = db_select("gap");
        if (req.rec.tlp_time) {
            req.rec.real_fps = 1000 / req.rec.tlp_time;
            req.rec.pkg_fps	 = 30;
        }
    } else {
        req.rec.tlp_time = 0;
    }
    get_dec_server_mutex();
    err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    put_dec_server_mutex();
    if (err != 0) {
        printf("\nsave rec err 0x%x\n", err);
        return VREC_ERR_V0_REQ_SAVEFILE;
    }

    return 0;
}

static void video0_rec_close()
{
    if (__this->video_rec0) {
        server_close(__this->video_rec0);
        __this->video_rec0 = NULL;
    }
}

/*
 *必须在启动录像之后才可调用该函数，并且确保启动录像时已经打开了osd
 *新设置的osd的整体结构要和启动录像时一样，只是内容改变!!!
 */
static int video0_rec_set_osd_str(char *str)
{
    union video_req req = {0};
    int err;
    if (!__this->video_rec0) {
        return -1;
    }

    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_SET_OSD_STR;
    req.rec.new_osd_str = str;
    err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    if (err != 0) {
        printf("\nset osd rec0 str err 0x%x\n", err);
        return -1;
    }

    return 0;
}

static int video0_rec_osd_ctl(u8 onoff)
{
    union video_req req = {0};
    struct video_text_osd text_osd;
    int err;

    if (__this->video_rec0) {

        u32 res = db_select("res");
        req.rec.width 	= rec_pix_w[res];
        req.rec.height 	= rec_pix_h[res];

        text_osd.font_w = 16;
        text_osd.font_h = 32;
        text_osd.x = (req.rec.width - strlen(video_rec_osd_buf) * text_osd.font_w) / 64 * 64;
        text_osd.y = (req.rec.height - text_osd.font_h) / 16 * 16;
        /* text_osd.osd_yuv = 0xe20095; */
        text_osd.color[0] = 0xe20095;
        text_osd.bit_mode = 1;
        text_osd.text_format = video_rec_osd_buf;
        text_osd.font_matrix_table = osd_str_total;
        text_osd.font_matrix_base = osd_str_matrix;
        text_osd.font_matrix_len = sizeof(osd_str_matrix);
        text_osd.direction = 1;
        req.rec.text_osd = 0;
        if (onoff) {
            req.rec.text_osd = &text_osd;
        }
        req.rec.channel = 0;
        req.rec.state 	= VIDEO_STATE_SET_OSD;

        err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nset osd rec0 err 0x%x\n", err);
            return -1;
        }
    }

    return 0;
}
#endif









/******* 不要单独调用这些子函数 ********/
#ifdef CONFIG_VIDEO1_ENABLE
static int video1_rec_start()
{
    int err;
    union video_req req = {0};
    struct video_text_osd text_osd;
    struct video_graph_osd graph_osd;
    u16 max_one_line_strnum;
    u16 osd_line_num;

    puts("start_video_rec1 \n");
    if (!__this->video_rec1) {
        __this->video_rec1 = server_open("video_server", "video1.0");
        /* __this->video_rec1 = server_open("video_server", "video4.1"); */
        if (!__this->video_rec1) {
            return VREC_ERR_V1_SERVER_OPEN;
        }

        server_register_event_handler(__this->video_rec1, (void *)1, rec_dev_server_event_handler);
    }

    /* req.rec.channel = 1; */
    req.rec.channel = 0;
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.width 	= AVIN_WIDTH;
    req.rec.height 	= AVIN_HEIGH;
    req.rec.format 	= VIDEO1_REC_FORMAT;
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.file    = __this->file[1];
    req.rec.quality = VIDEO_LOW_Q;
    req.rec.fps 	= 0;
    req.rec.real_fps 	= 15;

    req.rec.audio.fmt_format = AUDIO_FMT_PCM;
    req.rec.audio.sample_rate = net_video_rec_get_audio_rate();;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = 100;
    req.rec.audio.buf = __this->audio_buf[1];
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);
    req.rec.IP_interval = 64;

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    /* req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16; */
    /* req.rec.roi.roio_ratio = 256 * 70 / 100 ; */
    /* req.rec.roi.roi1_xy = 0; */
    /* req.rec.roi.roi2_xy = 0; */
    /* req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0; */
    /* req.rec.roi.roio_ratio1 = 0; */
    /* req.rec.roi.roio_ratio2 = 0; */
    /* req.rec.roi.roio_ratio3 = 256 * 80 / 100; */

    /* 编码参数配置，请仔细参阅说明文档后修改 */
    /* struct app_enc_info app_avc_cfg; */
    /* app_avc_cfg.pre_pskip_limit_flag = 0; 				//SKIP块提前判断限制开关 */
    /* app_avc_cfg.pre_pskip_th = 0;						//SKIP块提前判断阈值 */
    /* app_avc_cfg.common_pskip_limit_flag = 0; 			//SKIP块常规判断限制开关 */
    /* app_avc_cfg.common_pskip_th = 0;					//SKIP块常规判断阈值 */
    /* app_avc_cfg.dsw_size = 0;							//搜索窗口大小（0:3x7  1:5x7） */
    /* app_avc_cfg.fs_x_points = 32;						//水平搜索点数 */
    /* app_avc_cfg.fs_y_points = 24;						//垂直搜索点数 */
    /* app_avc_cfg.f_aq_strength = 0.6f;					//宏块QP自适应强度 */
    /* app_avc_cfg.qp_offset_low = 0;						//宏块QP偏移下限 */
    /* app_avc_cfg.qp_offset_high = 5;						//宏块QP偏移上限 */
    /* app_avc_cfg.nr_reduction = 0;						//DCT降噪开关 */
    /* app_avc_cfg.user_cfg_enable = 1;					//编码参数用户配置使能 */
    /* req.rec.app_avc_cfg = &app_avc_cfg; */
    req.rec.app_avc_cfg = NULL;



    text_osd.font_w = 16;
    text_osd.font_h = 32;

    max_one_line_strnum = strlen(video_rec_osd_buf);//20;
    osd_line_num = 1;
    if (db_select("num")) {
        osd_line_num = 2;
    }
    text_osd.x = (req.rec.width - max_one_line_strnum * text_osd.font_w) / 64 * 64;
    text_osd.y = (req.rec.height - text_osd.font_h * osd_line_num) / 16 * 16;

    text_osd.direction = 1;
    /* text_osd.color[0] = 0xe20095; */
    /* text_osd.bit_mode = 1; */
    text_osd.color[0] = 0x057d88;
    text_osd.color[1] = 0xe20095;
    text_osd.color[2] = 0xe20095;
    text_osd.bit_mode = 2;
    text_osd.text_format = video_rec_osd_buf;
    text_osd.font_matrix_table = osd_str_total;
    /* text_osd.font_matrix_base = osd_str_matrix; */
    /* text_osd.font_matrix_len = sizeof(osd_str_matrix); */
    text_osd.font_matrix_base = osd2_str_matrix;
    text_osd.font_matrix_len = sizeof(osd2_str_matrix);

    req.rec.text_osd = 0;
    if (db_select("dat")) {
        req.rec.text_osd = &text_osd;
    }

#if (defined __CPU_DV15__ || defined __CPU_DV17__ || defined __CPU_AC571X__)
    graph_osd.bit_mode = 16;//2bit的osd需要配置3个color
    graph_osd.x = 0;
    graph_osd.y = 0;
    graph_osd.width = 256;
    graph_osd.height = 256;
    graph_osd.icon = icon_osd_buf;
    graph_osd.icon_size = sizeof(icon_osd_buf);
    req.rec.graph_osd = NULL;//&graph_osd;
#endif

    req.rec.slow_motion = 0;
    if (req.rec.camera_type != VIDEO_CAMERA_UVC) {
        req.rec.tlp_time = db_select("gap");
        if (req.rec.tlp_time) {
            req.rec.real_fps = 1000 / req.rec.tlp_time;
            req.rec.pkg_fps	 = 30;
        }
    } else {
        req.rec.tlp_time = 0;
    }

    if (req.rec.slow_motion || req.rec.tlp_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    }
    req.rec.buf = __this->video_buf[1];
    req.rec.buf_len = VREC1_FBUF_SIZE;

#if (defined THREE_WAY_ENABLE || defined CONFIG_USB_VIDEO_OUT)
    req.rec.rec_small_pic 	= 0;
#else
    req.rec.rec_small_pic 	= 1;
#endif
    req.rec.cycle_time = 2 * 60;
#if 0
    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }

    req.rec.cycle_time = req.rec.cycle_time * 60;
#endif

    get_dec_server_mutex();
    err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
    put_dec_server_mutex();

    if (err != 0) {
        puts("\n\n\nstart rec2 err\n\n\n");
        return VREC_ERR_V1_REQ_START;
    }

    return 0;
}

static int video1_rec_len()
{
    union video_req req = {0};

    if (!__this->video_rec1) {
        return -EINVAL;
    }

    req.rec.state 	= VIDEO_STATE_PKG_LEN;
    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;

    return server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
}

static int video1_rec_aud_mute()
{
    union video_req req = {0};

    if (!__this->video_rec1) {
        return -EINVAL;
    }

    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_PKG_MUTE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    return server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
}

static int video1_rec_set_dr()
{
    union video_req req = {0};

    if (!__this->video_rec1) {
        return -EINVAL;
    }

    req.rec.real_fps = 7;
    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_SET_DR;

    return server_request(__this->video_rec1, VIDEO_REQ_REC, &req);

}


static int video1_rec_stop(u8 close)
{
    union video_req req = {0};
    int err;

    log_d("video1_rec_stop\n");

    if (__this->video_rec1) {
        req.rec.channel = 0;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nstop rec2 err 0x%x\n", err);
            return VREC_ERR_V1_REQ_STOP;
        }
    }

    video_rec_close_file(1);

    if (close) {
        if (__this->video_rec1) {
            server_close(__this->video_rec1);
            __this->video_rec1 = NULL;
        }
    }

    return 0;
}

static int video1_rec_savefile()
{
    union video_req req = {0};
    int err;

    if (__this->video_rec1) {

        if (!__this->file[1]) {
            log_i("\n\nf1null\n\n");
            return -ENOENT;
        }

        req.rec.channel = 0;
        req.rec.width 	= AVIN_WIDTH;
        req.rec.height 	= AVIN_HEIGH;
        req.rec.format 	= VIDEO1_REC_FORMAT;
        req.rec.state 	= VIDEO_STATE_SAVE_FILE;
        req.rec.file    = __this->file[1];
        req.rec.fps 	= 0;
        req.rec.real_fps 	= 0;

#if (defined THREE_WAY_ENABLE || defined CONFIG_USB_VIDEO_OUT)
        req.rec.rec_small_pic 	= 0;
#else
        req.rec.rec_small_pic 	= 1;
#endif

        req.rec.cycle_time = db_select("cyc");
        if (req.rec.cycle_time == 0) {
            req.rec.cycle_time = 5;
        }
        req.rec.cycle_time = req.rec.cycle_time * 60;

        req.rec.audio.fmt_format = AUDIO_FMT_PCM;
        req.rec.audio.sample_rate = net_video_rec_get_audio_rate();;
        req.rec.audio.channel 	= 1;
        req.rec.audio.volume    = 100;
        req.rec.audio.buf = __this->audio_buf[1];
        req.rec.audio.buf_len = AUDIO_BUF_SIZE;
        req.rec.pkg_mute.aud_mute = !db_select("mic");

        if (req.rec.camera_type != VIDEO_CAMERA_UVC) {
            req.rec.tlp_time = db_select("gap");
            if (req.rec.tlp_time) {
                req.rec.real_fps = 1000 / req.rec.tlp_time;
                req.rec.pkg_fps	 = 30;
            }
        } else {
            req.rec.tlp_time = 0;
        }
        get_dec_server_mutex();
        err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
        put_dec_server_mutex();
        if (err != 0) {
            printf("\nsave rec2 err 0x%x\n", err);
            return VREC_ERR_V1_REQ_SAVEFILE;
        }
    }

    return 0;
}

static void video1_rec_close()
{
    if (__this->video_rec1) {
        server_close(__this->video_rec1);
        __this->video_rec1 = NULL;
    }
}


/*
 *必须在启动录像之后才可调用该函数，并且确保启动录像时已经打开了osd
 *新设置的osd的整体结构要和启动录像时一样，只是内容改变!!!
 */
static int video1_rec_set_osd_str(char *str)
{
    union video_req req = {0};
    int err;
    if (!__this->video_rec1) {
        return -1;
    }

    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_SET_OSD_STR;
    req.rec.new_osd_str = str;
    err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
    if (err != 0) {
        printf("\nset osd rec1 str err 0x%x\n", err);
        return -1;
    }

    return 0;
}

static int video1_rec_osd_ctl(u8 onoff)
{
    union video_req req = {0};
    struct video_text_osd text_osd;
    int err;

    if (__this->video_rec1) {
        req.rec.width 	= AVIN_WIDTH;
        req.rec.height 	= AVIN_HEIGH;

        text_osd.font_w = 16;
        text_osd.font_h = 32;
        text_osd.x = (req.rec.width - strlen(osd_str_buf) * text_osd.font_w) / 64 * 64;
        text_osd.y = (req.rec.height - text_osd.font_h) / 16 * 16;
        /* text_osd.osd_yuv = 0xe20095; */
        text_osd.color[0] = 0xe20095;
        text_osd.bit_mode = 1;
        text_osd.text_format = osd_str_buf;
        text_osd.font_matrix_table = osd_str_total;
        text_osd.font_matrix_base = osd_str_matrix;
        text_osd.font_matrix_len = sizeof(osd_str_matrix);
        text_osd.direction = 1;
        req.rec.text_osd = 0;
        if (onoff) {
            req.rec.text_osd = &text_osd;
        }
        req.rec.channel = 0;
        req.rec.state 	= VIDEO_STATE_SET_OSD;

        err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nset osd rec1 err 0x%x\n", err);
            return -1;
        }
    }

    return 0;
}
#endif






#ifdef CONFIG_VIDEO3_ENABLE

static int video3_rec_start()
{
    int err;
    union video_req req = {0};
    struct video_text_osd text_osd;
    struct video_graph_osd graph_osd;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    char name[12];

    void *uvc_fd;
    struct uvc_capability uvc_cap;

    puts("-----start_video_rec3 \n");
    if (!__this->video_rec3) {


        sprintf(name, "video3.%d", 0);

        __this->video_rec3 = server_open("video_server", name);
        if (!__this->video_rec3) {
            log_e("rec3 video_server open err! \n");
            return -EINVAL;
        }

        server_register_event_handler(__this->video_rec3, (void *)3, rec_dev_server_event_handler);
    }

    req.rec.channel = 0;
    req.rec.camera_type = VIDEO_CAMERA_UVC;
#ifdef THREE_WAY_ENABLE
    req.rec.three_way_type = VIDEO_THREE_WAY_JPEG;
    req.rec.IP_interval = 99;
#else
    req.rec.three_way_type = 0;
    req.rec.IP_interval = 128;
#endif
    req.rec.format 	= VIDEO3_REC_FORMAT;
    req.rec.width 	= UVC_ENC_WIDTH;
    req.rec.height 	= UVC_ENC_HEIGH;
    req.rec.src_w   = __this->src_width[3];
    req.rec.src_h   = __this->src_height[3];
    if (req.rec.three_way_type == VIDEO_THREE_WAY_JPEG) {
        uvc_fd = dev_open("uvc", (void *)__this->uvc_id);
        if (uvc_fd) {
            dev_ioctl(uvc_fd, UVCIOC_QUERYCAP, (unsigned int)&uvc_cap);
            req.rec.width 	= uvc_cap.reso[0].width;
            req.rec.height 	= uvc_cap.reso[0].height;
            dev_close(uvc_fd);
        }
    }
    __this->uvc_width = req.rec.width;
    __this->uvc_height = req.rec.height;
    printf("\n\nuvc size %d, %d\n\n", req.rec.width, req.rec.height);
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.file    = __this->file[3];
    req.rec.uvc_id = __this->uvc_id;
    req.rec.quality = VIDEO_LOW_Q;
    req.rec.fps 	= 0;
    req.rec.real_fps 	= 0;

    req.rec.audio.fmt_format = AUDIO_FMT_PCM;
    req.rec.audio.sample_rate = 16000;;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = 100;
    req.rec.audio.buf = __this->audio_buf[3];
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;

#ifdef CONFIG_USB_UVC_AND_UAC_ENABLE
    req.rec.audio.sample_source = "user";
#endif


    req.rec.pkg_mute.aud_mute = !db_select("mic");

    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);


    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    /* req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16; */
    /* req.rec.roi.roio_ratio = 256 * 70 / 100 ; */
    /* req.rec.roi.roi1_xy = 0; */
    /* req.rec.roi.roi2_xy = 0; */
    /* req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0; */
    /* req.rec.roi.roio_ratio1 = 0; */
    /* req.rec.roi.roio_ratio2 = 0; */
    /* req.rec.roi.roio_ratio3 = 256 * 80 / 100; */

    /* 编码参数配置，请仔细参阅说明文档后修改 */
    /* struct app_enc_info app_avc_cfg; */
    /* app_avc_cfg.pre_pskip_limit_flag = 0; 				//SKIP块提前判断限制开关 */
    /* app_avc_cfg.pre_pskip_th = 0;						//SKIP块提前判断阈值 */
    /* app_avc_cfg.common_pskip_limit_flag = 0; 			//SKIP块常规判断限制开关 */
    /* app_avc_cfg.common_pskip_th = 0;					//SKIP块常规判断阈值 */
    /* app_avc_cfg.dsw_size = 0;							//搜索窗口大小（0:3x7  1:5x7） */
    /* app_avc_cfg.fs_x_points = 32;						//水平搜索点数 */
    /* app_avc_cfg.fs_y_points = 24;						//垂直搜索点数 */
    /* app_avc_cfg.f_aq_strength = 0.6f;					//宏块QP自适应强度 */
    /* app_avc_cfg.qp_offset_low = 0;						//宏块QP偏移下限 */
    /* app_avc_cfg.qp_offset_high = 5;						//宏块QP偏移上限 */
    /* app_avc_cfg.nr_reduction = 0;						//DCT降噪开关 */
    /* app_avc_cfg.user_cfg_enable = 1;					//编码参数用户配置使能 */
    /* req.rec.app_avc_cfg = &app_avc_cfg; */
    req.rec.app_avc_cfg = NULL;



    text_osd.font_w = 16;
    text_osd.font_h = 32;

    max_one_line_strnum = strlen(video_rec_osd_buf);//20;
    osd_line_num = 2;
    text_osd.x = (req.rec.width - max_one_line_strnum * text_osd.font_w) / 64 * 64;
    text_osd.y = (req.rec.height - text_osd.font_h * osd_line_num) / 16 * 16;

    text_osd.direction = 1;
    /* text_osd.osd_yuv = 0xe20095; */
    text_osd.color[0] = 0xe20095;
    text_osd.bit_mode = 1;
    text_osd.text_format = video_rec_osd_buf;
    text_osd.font_matrix_table = osd_str_total;
#ifdef THREE_WAY_ENABLE
    text_osd.font_matrix_base = osd_mimc_str_matrix;
    text_osd.font_matrix_len = sizeof(osd_mimc_str_matrix);
#else
    text_osd.font_matrix_base = osd_str_matrix;
    text_osd.font_matrix_len = sizeof(osd_str_matrix);
#endif

#ifndef THREE_WAY_ENABLE
#if (defined __CPU_DV15__ || defined __CPU_DV17__ || defined __CPU_AC571X__)
    text_osd.direction = 1;

    graph_osd.bit_mode = 16;//2bit的osd需要配置3个color
    graph_osd.x = 0;
    graph_osd.y = 0;
    graph_osd.width = 256;
    graph_osd.height = 256;
    graph_osd.icon = icon_16bit_data;//icon_osd_buf;
    graph_osd.icon_size = sizeof(icon_16bit_data);//sizeof(icon_osd_buf);
#endif
#endif

    if (db_select("dat")) {
        req.rec.text_osd = &text_osd;
#ifndef THREE_WAY_ENABLE
#if (defined __CPU_DV15__ || defined __CPU_DV17__ || defined __CPU_AC571X__)
        /* req.rec.graph_osd = &graph_osd; */
#endif
#endif
    }

    req.rec.slow_motion = 0;
    if (req.rec.camera_type != VIDEO_CAMERA_UVC) {
        req.rec.tlp_time = db_select("gap");
        if (req.rec.tlp_time) {
            req.rec.real_fps = 1000 / req.rec.tlp_time;
            req.rec.pkg_fps	 = 30;
        }
    } else {
        req.rec.tlp_time = 0;
    }

    if (req.rec.slow_motion || req.rec.tlp_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    }
    req.rec.buf = __this->video_buf[3];
    req.rec.buf_len = VREC3_FBUF_SIZE;
#if (defined THREE_WAY_ENABLE || defined CONFIG_USB_VIDEO_OUT)
    req.rec.rec_small_pic 	= 0;
#else
    req.rec.rec_small_pic 	= 1;
#endif

    req.rec.cycle_time = 2 * 60;//db_select("cyc");
#if 0
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;
#endif
    get_dec_server_mutex();
    err = server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
    put_dec_server_mutex();

    if (err != 0) {
        log_e("rec3 rec start err! \n");
        return -EINVAL;
    }
    printf("rec3 rec start ok. \n");

    return 0;
}

static int video3_rec_len()
{
    union video_req req = {0};

    if (!__this->video_rec3) {
        return -EINVAL;
    }

    req.rec.state 	= VIDEO_STATE_PKG_LEN;
    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;

    return server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
}

static int video3_rec_aud_mute()
{
    union video_req req = {0};

    if (!__this->video_rec3) {
        return -EINVAL;
    }

    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_PKG_MUTE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    return server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
}


static int video3_rec_stop(u8 close)
{
    union video_req req = {0};
    int err;

    log_d("video3_rec_stop\n");

    if (__this->video_rec3) {
        req.rec.channel = 0;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nstop rec3 err 0x%x\n", err);
            return -EINVAL;
        }
    }

    video_rec_close_file(3);

    if (close) {
        if (__this->video_rec3) {
            server_close(__this->video_rec3);
            __this->video_rec3 = NULL;
        }
    }

    return 0;
}

static int video3_rec_savefile()
{
    union video_req req = {0};
    int err;

    if (__this->video_rec3) {
        printf("video3_rec_savefile in\n");

        if (!__this->file[3]) {
            return -ENOENT;
        }

        req.rec.channel = 0;
#ifdef THREE_WAY_ENABLE
        req.rec.width 	= __this->uvc_width;
        req.rec.height 	= __this->uvc_height;
#else
        req.rec.width 	= UVC_ENC_WIDTH;
        req.rec.height 	= UVC_ENC_HEIGH;
#endif
        req.rec.format 	= VIDEO3_REC_FORMAT;
        req.rec.state 	= VIDEO_STATE_SAVE_FILE;
        req.rec.file    = __this->file[3];
        req.rec.fps 	= 0;
        req.rec.real_fps = 0;
        req.rec.uvc_id = __this->uvc_id;
#if (defined THREE_WAY_ENABLE || defined CONFIG_USB_VIDEO_OUT)
        req.rec.rec_small_pic 	= 0;
#else
        req.rec.rec_small_pic 	= 1;
#endif

        req.rec.cycle_time = db_select("cyc");
        if (req.rec.cycle_time == 0) {
            req.rec.cycle_time = 5;
        }
        req.rec.cycle_time = req.rec.cycle_time * 60;


        req.rec.audio.fmt_format = AUDIO_FMT_PCM;
        req.rec.audio.sample_rate = net_video_rec_get_audio_rate();
        req.rec.audio.channel 	= 1;
        req.rec.audio.volume    = 100;
        req.rec.audio.buf = __this->audio_buf[3];
        req.rec.audio.buf_len = AUDIO_BUF_SIZE;
        req.rec.pkg_mute.aud_mute = !db_select("mic");

        if (req.rec.camera_type != VIDEO_CAMERA_UVC) {
            req.rec.tlp_time = db_select("gap");
            if (req.rec.tlp_time) {
                req.rec.real_fps = 1000 / req.rec.tlp_time;
                req.rec.pkg_fps	 = 30;
            }
        } else {
            req.rec.tlp_time = 0;
        }

        get_dec_server_mutex();
        err = server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
        put_dec_server_mutex();
        if (err != 0) {
            printf("\nsave rec3 err 0x%x\n", err);
            return -EINVAL;
        }

        printf("video3_rec_savefile out\n");
    }

    return 0;
}

static void video3_rec_close()
{
    if (__this->video_rec3) {
        server_close(__this->video_rec3);
        __this->video_rec3 = NULL;
    }
}


/*
 *必须在启动录像之后才可调用该函数，并且确保启动录像时已经打开了osd
 *新设置的osd的整体结构要和启动录像时一样，只是内容改变!!!
 */
static int video3_rec_set_osd_str(char *str)
{
    union video_req req = {0};
    int err;
    if (!__this->video_rec3) {
        return -1;
    }

    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_SET_OSD_STR;
    req.rec.new_osd_str = str;
    err = server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
    if (err != 0) {
        printf("\nset osd rec3 str err 0x%x\n", err);
        return -1;
    }

    return 0;
}

static int video3_rec_osd_ctl(u8 onoff)
{
    union video_req req = {0};
    struct video_text_osd text_osd;
    int err;

    if (__this->video_rec3) {
        req.rec.width 	= UVC_ENC_WIDTH;
        req.rec.height 	= UVC_ENC_HEIGH;

        text_osd.font_w = 16;
        text_osd.font_h = 32;
        text_osd.x = (req.rec.width - strlen(osd_str_buf) * text_osd.font_w) / 64 * 64;
        text_osd.y = (req.rec.height - text_osd.font_h) / 16 * 16;
        /* text_osd.osd_yuv = 0xe20095; */
        text_osd.color[0] = 0xe20095;
        text_osd.bit_mode = 1;
        text_osd.direction = 1;
        text_osd.text_format = osd_str_buf;
        text_osd.font_matrix_table = osd_str_total;
        text_osd.font_matrix_base = osd_str_matrix;
        text_osd.font_matrix_len = sizeof(osd_str_matrix);
        req.rec.text_osd = 0;
        if (onoff) {
            req.rec.text_osd = &text_osd;
        }
        req.rec.channel = 0;
        req.rec.state 	= VIDEO_STATE_SET_OSD;

        err = server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nset osd rec3 err 0x%x\n", err);
            return -1;
        }
    }

    return 0;
}

#endif



#ifdef CONFIG_VIDEO4_ENABLE
/******* 不要单独调用这些子函数 ********/
static int video4_rec_start(u32 id)
{
    log_i("start_video4_rec%d\n", id);
    if (id >= 4) {
        return -1;
    }

    int err;
    union video_req req = {0};
    struct video_text_osd text_osd;
    struct video_graph_osd graph_osd;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;
    char dev_name[20];

    if (!__this->video_rec[id]) {
        sprintf(dev_name, "video4.%d.%d", id, 0);
        __this->video_rec[id] = server_open("video_server", (void *)dev_name);
        if (!__this->video_rec[id]) {
            return VREC_ERR_V0_SERVER_OPEN;
        }

        server_register_event_handler(__this->video_rec[id], (void *)id, rec_dev_server_event_handler);
    }


    u32 res = VIDEO_RES_720P;
    /*
     *通道号，分辨率，封装格式，写卡的路径
     */
    req.rec.channel     = id;
    req.rec.camera_type = VIDEO_CAMERA_MUX;
    req.rec.width 	    = rec_pix_w[res];
    req.rec.height 	    = rec_pix_h[res];
    req.rec.format 	    = VIDEO4_REC_FORMAT;
    req.rec.state 	    = VIDEO_STATE_START;
    req.rec.file        = __this->file[id];

    /*
     *帧率为0表示使用摄像头的帧率
     */
    req.rec.quality     = VIDEO_MID_Q;
    req.rec.fps 	    = 0;
    req.rec.real_fps 	= 0;

    /*
     *采样率，通道数，录像音量，音频使用的循环BUF,录不录声音
     */
    req.rec.audio.sample_rate = 8000;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = 100;
    req.rec.audio.buf = __this->audio_buf[id];
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    /*
     *码率，I帧和P帧比例，必须是偶数（当录MOV的时候才有效）,
     *roio_xy :值表示宏块坐标， [6:0]左边x坐标 ，[14:8]右边x坐标，[22:16]上边y坐标，[30:24]下边y坐标,写0表示1个宏块有效
     * roio_ratio : 区域比例系数
     */

    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);
    req.rec.IP_interval = 32;//128;

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    /* req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16; */
    /* req.rec.roi.roi1_xy = (req.rec.height * 11 / 12 / 16) << 24 | (req.rec.height * 4 / 12 / 16) << 16 | (req.rec.width * 11 / 12 / 16) << 8 | (req.rec.width) * 1 / 12 / 16; */
    /* req.rec.roi.roi2_xy = 0; */
    /* req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0; */
    /* req.rec.roi.roio_ratio = 256 * 70 / 100 ; */
    /* req.rec.roi.roio_ratio1 = 256 * 90 / 100; */
    /* req.rec.roi.roio_ratio2 = 0; */
    /* req.rec.roi.roio_ratio3 = 256 * 80 / 100; */

    /* 编码参数配置，请仔细参阅说明文档后修改 */
    /* struct app_enc_info app_avc_cfg; */
    /* app_avc_cfg.pre_pskip_limit_flag = 0; 				//SKIP块提前判断限制开关 */
    /* app_avc_cfg.pre_pskip_th = 0;						//SKIP块提前判断阈值 */
    /* app_avc_cfg.common_pskip_limit_flag = 0; 			//SKIP块常规判断限制开关 */
    /* app_avc_cfg.common_pskip_th = 0;					//SKIP块常规判断阈值 */
    /* app_avc_cfg.dsw_size = 0;							//搜索窗口大小（0:3x7  1:5x7） */
    /* app_avc_cfg.fs_x_points = 32;						//水平搜索点数 */
    /* app_avc_cfg.fs_y_points = 24;						//垂直搜索点数 */
    /* app_avc_cfg.f_aq_strength = 0.6f;					//宏块QP自适应强度 */
    /* app_avc_cfg.qp_offset_low = 0;						//宏块QP偏移下限 */
    /* app_avc_cfg.qp_offset_high = 5;						//宏块QP偏移上限 */
    /* app_avc_cfg.nr_reduction = 0;						//DCT降噪开关 */
    /* app_avc_cfg.user_cfg_enable = 1;					//编码参数用户配置使能 */
    /* req.rec.app_avc_cfg = &app_avc_cfg; */
    req.rec.app_avc_cfg = NULL;



    /*
     * osd 相关的参数，注意坐标位置，x要64对齐，y要16对齐,底下例子是根据图像大小偏移到右下
     */
#if (APP_CASE == __CAR_CAMERA__)
#ifndef CONFIG_UI_ENABLE
    memcpy(video_rec_osd_buf, osd_str_buf, strlen(osd_str_buf));
#endif
#endif //APP_CASE

    text_osd.font_w = 16;
    text_osd.font_h = 32;
    max_one_line_strnum = strlen(video_rec_osd_buf);

    /* osd_line_num = 1; */
    /* if (db_select("num")) { */
    osd_line_num = 2;
    /* } */

    osd_max_heigh = (req.rec.height == 1088) ? 1080 : req.rec.height ;
    text_osd.x = (req.rec.width - max_one_line_strnum * text_osd.font_w) / 64 * 64;
    text_osd.y = (osd_max_heigh - text_osd.font_h * osd_line_num) / 16 * 16;
    text_osd.color[0] = 0xe20095;
    text_osd.bit_mode = 1;
    text_osd.text_format = video_rec_osd_buf;
    text_osd.font_matrix_table = osd_str_total;
    text_osd.font_matrix_base = osd_mimc_str_matrix;//osd_str_matrix;
    text_osd.font_matrix_len = sizeof(osd_mimc_str_matrix);

    text_osd.direction = 1;

    graph_osd.bit_mode = 16;//2bit的osd需要配置3个color
    graph_osd.x = 0;
    graph_osd.y = 0;
    graph_osd.width = 256;
    graph_osd.height = 256;
    graph_osd.icon = (u8 *)icon_16bit_data;
    graph_osd.icon_size = sizeof(icon_16bit_data);

    req.rec.text_osd = &text_osd;
    req.rec.graph_osd = NULL;//&graph_osd;

    /*
     *慢动作倍数(与延时摄影互斥,无音频); 延时录像的间隔ms(与慢动作互斥,无音频)
     */
    u32 gap_time = db_select("gap");
    req.rec.slow_motion = 0;

    if (gap_time && (req.rec.camera_type != VIDEO_CAMERA_UVC)) {
        /* req.rec.tlp_time = gap_time; */
        /* req.rec.real_fps = 1000 / req.rec.tlp_time; */
        req.rec.gap_fps  = gap_time / 40;
        req.rec.pkg_fps	 = 30;
    }
    if (req.rec.slow_motion || gap_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    }
    req.rec.buf = __this->video_buf[id];
    req.rec.buf_len = VREC4_FBUF_SIZE;
#if (defined THREE_WAY_ENABLE || defined CONFIG_USB_VIDEO_OUT)
    req.rec.rec_small_pic 	= 0;
#else
    req.rec.rec_small_pic 	= 1;
#endif

    /*
     *循环录像时间，文件大小
     */
    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;
    err = server_request(__this->video_rec[id], VIDEO_REQ_REC, &req);
    if (err != 0) {
        log_i("\n\n\nstart video4 rec%d err\n\n\n", id);
        return VREC_ERR_V0_REQ_START;
    }

    /* extern int audio_callback(u8 *buf, u32 len); */
    /* video0_set_audio_callback(__this->video_rec[0],audio_callback); */

    return 0;

}

static int video4_rec_len(u8 id)
{
    union video_req req = {0};

    if (!__this->video_rec[id]) {
        return -EINVAL;
    }

    req.rec.channel = id;
    req.rec.state 	= VIDEO_STATE_PKG_LEN;
    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;

    return server_request(__this->video_rec[id], VIDEO_REQ_REC, &req);
}

static int video4_rec_aud_mute(u8 id)
{
    union video_req req = {0};

    if (!__this->video_rec[id]) {
        return -EINVAL;
    }

    req.rec.channel = id;
    req.rec.state 	= VIDEO_STATE_PKG_MUTE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    return server_request(__this->video_rec[id], VIDEO_REQ_REC, &req);
}

static int video4_rec_set_dr(u8 id, u8 fps)
{
    union video_req req = {0};

    if (!__this->video_rec[id]) {
        return -EINVAL;
    }

    req.rec.real_fps = fps;
    req.rec.channel = id;
    req.rec.state 	= VIDEO_STATE_SET_DR;


    return server_request(__this->video_rec[id], VIDEO_REQ_REC, &req);

}

static int video4_rec_stop(u8 id, u8 close)
{
    union video_req req = {0};
    int err;

    log_d("video%d_rec_stop\n", id);

    if (__this->video_rec[id]) {
        req.rec.channel = id;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this->video_rec[id], VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nvideo4 stop rec%d err 0x%x\n", id, err);
            return VREC_ERR_V0_REQ_STOP;
        }
    }

    video_rec_close_file(id);

    if (close) {
        if (__this->video_rec[id]) {
            server_close(__this->video_rec[id]);
            __this->video_rec[id] = NULL;
        }
    }

    return 0;
}


/*
 *注意：循环录像的时候，虽然要重新传参，但是要和start传的参数保持一致！！！
 */
static int video4_rec_savefile(u8 id)
{
    union video_req req = {0};
    int err;

    if (!__this->file[id]) {
        log_i("\n\nf%dnull\n\n", id);
        return -ENOENT;
    }
    u32 res = VIDEO_RES_720P;

    req.rec.channel = id;
    req.rec.width 	= rec_pix_w[res];
    req.rec.height 	= rec_pix_h[res];
    req.rec.format 	= VIDEO4_REC_FORMAT;
    req.rec.state 	= VIDEO_STATE_SAVE_FILE;
    req.rec.file    = __this->file[id];
    req.rec.fps 	= 0;
    req.rec.real_fps 	= 0;

#if (defined THREE_WAY_ENABLE || defined CONFIG_USB_VIDEO_OUT)
    req.rec.rec_small_pic 	= 0;
#else
    req.rec.rec_small_pic 	= 1;
#endif

    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }

    req.rec.cycle_time = req.rec.cycle_time * 60;

    /*
     *慢动作倍数(与延时摄影互斥,无音频); 延时录像的间隔ms(与慢动作互斥,无音频)
     */
    u32 gap_time = db_select("gap");
    req.rec.slow_motion = 0;

    if (gap_time && (req.rec.camera_type != VIDEO_CAMERA_UVC)) {
        /* req.rec.tlp_time = gap_time; */
        /* req.rec.real_fps = 1000 / req.rec.tlp_time; */
        req.rec.gap_fps  = gap_time / 40;
        req.rec.pkg_fps	 = 30;
    }
    if (req.rec.slow_motion || gap_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    } else {
        req.rec.audio.sample_rate = 8000;
        req.rec.audio.channel 	= 1;
        req.rec.audio.volume    = 100;
        req.rec.audio.buf = __this->audio_buf[id];
        req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    }
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    err = server_request(__this->video_rec[id], VIDEO_REQ_REC, &req);
    if (err != 0) {
        printf("\nvideo4 save rec%d err 0x%x\n", id, err);
        return VREC_ERR_V0_REQ_SAVEFILE;
    }

    return 0;
}


static void video4_rec_close(u8 id)
{
    if (__this->video_rec[id]) {
        server_close(__this->video_rec[id]);
        __this->video_rec[id] = NULL;
    }
}

/*
 *必须在启动录像之后才可调用该函数，并且确保启动录像时已经打开了osd
 *新设置的osd的整体结构要和启动录像时一样，只是内容改变!!!
 */
static int video4_rec_set_osd_str(u8 id, char *str)
{
    union video_req req = {0};
    int err;
    if (!__this->video_rec[id]) {
        return -1;
    }

    req.rec.channel = id;
    req.rec.state 	= VIDEO_STATE_SET_OSD_STR;
    req.rec.new_osd_str = str;
    err = server_request(__this->video_rec[id], VIDEO_REQ_REC, &req);
    if (err != 0) {
        printf("\nvideo4 set osd rec%d str err 0x%x\n", id, err);
        return -1;
    }

    return 0;
}

#endif

/* const static u8 mov_user_data[] = "demo 1234567890 abcdefghijklmnopqrstuvwxyz"; */
/* int mov_pkg_insert_user_data(u8** buf) */
/* { */
/* *buf = mov_user_data; */
/* return sizeof(mov_user_data); */
/* } */

static int video_rec_start()
{
    int err;
    u32 clust;
    u8 state = __this->state;


    __this->char_wait = 0;
    __this->need_restart_rec = 0;

    if (__this->state == VIDREC_STA_START) {
        return 0;
    }

    log_d("(((((( video_rec_start: in\n");

    if (!storage_device_available()) {

        printf("\n no storage\n ");
        return 0;
    }

    /*
     * 申请录像所需要的音频和视频帧buf
     */

#ifndef VIDEO_REC_NO_MALLOC
    int abuf_size[] = {AUDIO_BUF_SIZE, AUDIO_BUF_SIZE, AUDIO_BUF_SIZE, AUDIO_BUF_SIZE};
    for (int i = 0; i < ARRAY_SIZE(abuf_size); i++) {
        if (abuf_size[i]) {
            if (!__this->audio_buf[i]) {
                __this->audio_buf[i] = malloc(abuf_size[i]);
                if (__this->audio_buf[i] == NULL) {
                    log_i("err maloo\n");
                    while (1);
                }
            }
        } else {
            __this->audio_buf[i] = NULL;
        }
    }
#endif




#ifndef VIDEO_REC_NO_MALLOC
#ifdef CONFIG_VIDEO4_ENABLE
    int buf_size[] = {VREC4_FBUF_SIZE, VREC4_FBUF_SIZE, VREC4_FBUF_SIZE, VREC4_FBUF_SIZE};
#else
#ifndef CONFIG_SINGLE_VIDEO_REC_ENABLE
    int buf_size[] = {VREC0_FBUF_SIZE, VREC1_FBUF_SIZE, VREC2_FBUF_SIZE, VREC3_FBUF_SIZE};
#else
    int buf_size[] = {VREC0_FBUF_SIZE};
#endif
    int buf_size[] = {VREC0_FBUF_SIZE, VREC1_FBUF_SIZE, VREC2_FBUF_SIZE, VREC3_FBUF_SIZE};
#endif
    for (int i = 0; i < ARRAY_SIZE(buf_size); i++) {
        if (buf_size[i]) {
            if (!__this->video_buf[i]) {
                __this->video_buf[i] = malloc(buf_size[i]);
                if (__this->video_buf[i] == NULL) {
                    log_i("err maloo\n");
                    while (1);
                }
            }
        } else {
            __this->video_buf[i] = NULL;
        }
    }
#endif

    /*
     * 判断SD卡空间，删除旧文件并创建新文件
     */
    err = video_rec_del_old_file();
    if (err) {
        log_e("start free space err\n");
        video_rec_post_msg("fsErr");
        return VREC_ERR_START_FREE_SPACE;
    }

    for (int i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
        __this->file[i] = __this->new_file[i];
        __this->new_file[i] = NULL;
    }


#ifdef CONFIG_VIDEO0_ENABLE
    err = video0_rec_start();
    if (err) {
        video0_rec_stop(0);
        return err;
    }
#endif

    video_rec_post_msg("onREC");
    video_parking_post_msg("onREC");

#if (defined CONFIG_VIDEO1_ENABLE && !defined CONFIG_SINGLE_VIDEO_REC_ENABLE)
    if (__this->video_online[1] && db_select("two")) {
        err = video1_rec_start();
    }
#endif


#ifdef CONFIG_VIDEO3_ENABLE
    if (__this->video_online[3] && db_select("two")) {
        err = video3_rec_start();
    }
#endif

#ifdef CONFIG_VIDEO4_ENABLE
    int id;
    for (id = 0; id < CONFIG_VIDEO_REC_NUM; id++) {
        if (__this->video_online[id]) {
            err = video4_rec_start(id);
            if (err != 0) {
                log_i("video4 start err out\n");
                return -1;
            }
        }
    }
#endif
    if (__this->gsen_lock == 0xff) {
        video_rec_post_msg("lockREC");
    }

#ifndef CONFIG_VIDEO4_ENABLE
#ifndef CONFIG_DISPLAY_ENABLE
    //video_rec_set_white_balance();
    video_rec_set_exposure(db_select("exp"));
#endif
#endif
    sys_power_auto_shutdown_pause();

    __this->state = VIDREC_STA_START;

    log_d("video_rec_start: out )))))))\n");
    /* malloc_dump(); */
    malloc_stats();

    return 0;
}


static int video_rec_len()
{

    /* if (db_select("mic")) { */
    /* puts("mic on\n"); */
    /* video_rec_post_msg("onMIC"); */
    /* } else { */
    /* puts("mic off\n"); */
    /* video_rec_post_msg("offMIC"); */
    /* } */

    if (__this->state != VIDREC_STA_START) {
        return 0;
    }

#ifdef CONFIG_VIDEO0_ENABLE
    video0_rec_len();
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    video1_rec_len();
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    video3_rec_len();
#endif

#ifdef CONFIG_VIDEO4_ENABLE
    int id;
    for (id = 0; id < CONFIG_VIDEO_REC_NUM; id++) {
        video4_rec_len(id);
    }
#endif

    return 0;
}

static int video_rec_aud_mute()
{

    /* if (db_select("mic")) { */
    /* puts("mic on\n"); */
    /* video_rec_post_msg("onMIC"); */
    /* } else { */
    /* puts("mic off\n"); */
    /* video_rec_post_msg("offMIC"); */
    /* } */

    if (__this->state != VIDREC_STA_START) {
        return 0;
    }

#ifdef CONFIG_VIDEO0_ENABLE
    video0_rec_aud_mute();
#endif

#if (defined CONFIG_VIDEO1_ENABLE && !defined CONFIG_SINGLE_VIDEO_REC_ENABLE)
    video1_rec_aud_mute();
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    video3_rec_aud_mute();
#endif

#ifdef CONFIG_VIDEO4_ENABLE
    int id;
    for (id = 0; id < CONFIG_VIDEO_REC_NUM; id++) {
        video4_rec_aud_mute(id);
    }
#endif

    return 0;
}


static int video_rec_stop(u8 close)
{
    int err;
    __this->need_restart_rec = 0;

    if (__this->state != VIDREC_STA_START) {
        return 0;
    }

    puts("\nvideo_rec_stop\n");

    __this->state = VIDREC_STA_STOPING;

#ifdef CONFIG_VIDEO0_ENABLE
    err = video0_rec_stop(close);
    if (err) {
        puts("\nstop0 err\n");
    }
#endif

#if (defined CONFIG_VIDEO1_ENABLE && !defined CONFIG_SINGLE_VIDEO_REC_ENABLE)
    err = video1_rec_stop(close);
    if (err) {
        puts("\nstop1 err\n");
    }
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    err = video3_rec_stop(close);
    if (err) {
        puts("\nstop3 err\n");
    }
#endif

#ifdef CONFIG_VIDEO4_ENABLE
    int id;
    int other_id;
    union video_req req = {0};
    for (id = 0; id < CONFIG_VIDEO_REC_NUM; id++) {

        /* if ((id == 0) || (id == 1)) { */
        if (id == 0) {
            for (other_id = id + 1; other_id < CONFIG_VIDEO_REC_NUM; other_id++) {
                if (__this->video_rec[other_id]) {
                    req.rec.IP_interval = 1;
                    req.rec.channel = other_id;
                    req.rec.state = VIDEO_STATE_V4_PAUSE_RUN;
                    err = server_request(__this->video_rec[other_id], VIDEO_REQ_REC, &req);
                }
            }
        }
        if (id == 0) {
            for (other_id = id + 1; other_id < CONFIG_VIDEO_REC_NUM; other_id++) {
                if (__this->video_rec[other_id]) {
                    req.rec.pkg_mute.aud_mute = 1;
                    req.rec.channel = other_id;
                    req.rec.state = VIDEO_STATE_PKG_PAUSE_RUN;
                    err = server_request(__this->video_rec[other_id], VIDEO_REQ_REC, &req);
                }
            }
        }

        err = video4_rec_stop(id, close);
        if (err) {
            log_i("\nvideo4 stop%d err\n", id);
        }

        /* if ((id == 0) || (id == 1)) { */
        if (1) {
            /* for (other_id = id + 1; other_id < CONFIG_VIDEO_REC_NUM; other_id++) { */
            other_id = id + 1;
            if (other_id < CONFIG_VIDEO_REC_NUM) {
                if (__this->video_rec[other_id]) {
                    req.rec.IP_interval = 0;
                    req.rec.channel = other_id;
                    req.rec.state = VIDEO_STATE_V4_PAUSE_RUN;
                    err = server_request(__this->video_rec[other_id], VIDEO_REQ_REC, &req);
                }
            }
        }
        /* if (id == 0) { */
        /* [> for (other_id = id + 1; other_id < CONFIG_VIDEO_REC_NUM; other_id++) { <] */
        /* for (other_id = id; other_id < CONFIG_VIDEO_REC_NUM; other_id++) { */
        /* if (__this->video_rec[other_id]) { */
        /* req.rec.pkg_mute.aud_mute = 0; */
        /* req.rec.channel = other_id; */
        /* req.rec.state = VIDEO_STATE_PKG_PAUSE_RUN; */
        /* err = server_request(__this->video_rec[other_id], VIDEO_REQ_REC, &req); */
        /* } */
        /* } */
        /* } */

    }

#endif

#ifndef CONFIG_VIDEO4_ENABLE
    if (__this->disp_state == DISP_BACK_WIN) {
        video_rec_post_msg("HlightOff"); //后视停录像关闭前照灯
    }
#endif

    __this->state = VIDREC_STA_STOP;
    __this->gsen_lock = 0;
    sys_power_auto_shutdown_resume();

    video_rec_get_remain_time();
    video_rec_post_msg("offREC");
    video_home_post_msg("offREC");
    video_parking_post_msg("offREC");

    puts("video_rec_stop: exit\n");
    return 0;
}




static int video_rec_close()
{
#ifdef CONFIG_VIDEO0_ENABLE
    video0_rec_close();
#endif

#if (defined CONFIG_VIDEO1_ENABLE && !defined CONFIG_SINGLE_VIDEO_REC_ENABLE)
    video1_rec_close();
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    video3_rec_close();
#endif

#ifdef CONFIG_VIDEO4_ENABLE
    for (int id = 0; id < CONFIG_VIDEO_REC_NUM; id++) {
        video4_rec_close(id);
    }
#endif


    return 0;
}

static int video_rec_change_source_reso(int dev_id, u16 width, u16 height, u16 fps)
{
#ifdef CONFIG_VIDEO0_ENABLE
    if (dev_id == 0) {

    } else
#endif
#ifdef CONFIG_VIDEO1_ENABLE
        if (dev_id == 1) {

        } else
#endif
#ifdef CONFIG_VIDEO3_ENABLE
            if (dev_id == 3) {
                __this->src_width[3] = width;
                __this->src_height[3] = height;
                if (__this->video_online[3]) {
                    log_d("video3.* change source reso to %d x %d\n", width, height);
                    int rec_state = __this->state;
                    int disp_state = __this->disp_state;
                    void *video3_disp = __this->video_display[3];
                    if (rec_state == VIDREC_STA_START) {
                        video_rec_stop(0);
                    }
                    if (disp_state == DISP_MAIN_WIN ||
                        disp_state == DISP_HALF_WIN ||
#ifndef CONFIG_VIDEO4_ENABLE
                        disp_state == DISP_BACK_WIN ||
#else
                        disp_state == DISP_VIDEO3 ||
#endif
                        disp_state == DISP_PARK_WIN) {
                        if (video3_disp != NULL) {
                            video_disp_stop(3);
                        }
                    }
                    void *uvc_fd = dev_open("uvc", (void *)__this->uvc_id);
                    if (!uvc_fd) {
                        printf("uvc dev_open err!!!\n");
                        return 0;
                    }
                    if (uvc_fd) {
                        dev_ioctl(uvc_fd, UVCIOC_SET_FPS, (unsigned int)fps);
                        dev_close(uvc_fd);
                    }
                    //video3.* record and display must be closed before source reso change
                    if (rec_state == VIDREC_STA_START) {
                        video_rec_start();
                    }
                    if (disp_state == DISP_MAIN_WIN ||
                        disp_state == DISP_HALF_WIN ||
#ifndef CONFIG_VIDEO4_ENABLE
                        disp_state == DISP_BACK_WIN ||
#else
                        disp_state == DISP_VIDEO3 ||
#endif
                        disp_state == DISP_PARK_WIN) {
                        if (video3_disp != NULL) {
#ifdef CONFIG_VIDEO4_ENABLE
                            video_disp_start(3, &disp_window[disp_state][3]);
#elif defined THREE_WAY_ENABLE
                            video_disp_start(3, &disp_window[disp_state][3]);
#else
                            video_disp_start(3, &disp_window[disp_state][1]);
#endif
                        }
                    }
                }
            } else
#endif
            {
                //do not remove this brace
            }
    return 0;
}

static int video_rec_savefile(int dev_id)
{
    int i;
    int err;
    int post_msg = 0;
    union video_req req = {0};

    if (__this->state != VIDREC_STA_START) {
        return 0;
    }

    sys_key_event_disable();
    sys_touch_event_disable();

    if (__this->need_restart_rec) {
        log_d("need restart rec");
        video_rec_stop(0);
        video_rec_start();
        sys_key_event_enable();
        sys_touch_event_enable();
        return 0;
    }

    log_d("\nvideo_rec_start_new_file: %d\n", dev_id);

    /* #ifdef CONFIG_VIDEO4_ENABLE */
    /* int other_id; */
    /* for (other_id = 0; other_id < CONFIG_VIDEO_REC_NUM; other_id++) { */
    /* [> if ((other_id != dev_id) && __this->video_online[other_id] && __this->video_rec[other_id]) { <] */
    /* if (__this->video_online[other_id] && __this->video_rec[other_id]) { */
    /* req.rec.IP_interval = 1; */
    /* req.rec.channel = other_id; */
    /* req.rec.state = VIDEO_STATE_V4_PAUSE_RUN; */
    /* err = server_request(__this->video_rec[other_id], VIDEO_REQ_REC, &req); */
    /* } */
    /* } */
    /* #endif */

    video_rec_close_file(dev_id);

    /* #ifdef CONFIG_VIDEO4_ENABLE */
    /* for (other_id = 0; other_id < CONFIG_VIDEO_REC_NUM; other_id++) { */
    /* [> if ((other_id != dev_id) && __this->video_online[other_id] && __this->video_rec[other_id]) { <] */
    /* if (__this->video_online[other_id] && __this->video_rec[other_id]) { */
    /* req.rec.IP_interval = 0; */
    /* req.rec.channel = other_id; */
    /* req.rec.state = VIDEO_STATE_V4_PAUSE_RUN; */
    /* err = server_request(__this->video_rec[other_id], VIDEO_REQ_REC, &req); */
    /* } */
    /* } */
    /* #endif */


    if (__this->new_file[dev_id] == NULL) {
        err = video_rec_del_old_file();
        if (err) {
            video_rec_post_msg("fsErr");
            goto __err;
        }
        post_msg = 1;
    }
    __this->file[dev_id]     = __this->new_file[dev_id];
    __this->new_file[dev_id] = NULL;

#ifdef CONFIG_VIDEO0_ENABLE
    if (dev_id == 0) {
        err = video0_rec_savefile();
        if (err) {
            log_i("\n\n\nv0_serr %x\n\n", err);
            goto __err;
        }
    }
#endif

    if (post_msg) {
        video_rec_post_msg("saveREC");
        video_home_post_msg("saveREC");//录像切到后台,ui消息由主界面响应
        video_parking_post_msg("saveREC");
    }

#if (defined CONFIG_VIDEO1_ENABLE && !defined CONFIG_SINGLE_VIDEO_REC_ENABLE)
    if (__this->video_online[1] && (dev_id == 1)) {
        err = video1_rec_savefile();
        if (err) {
            log_i("\n\n\nv1_serr %x\n\n", err);
            goto __err;
        }
    }
#endif

    /* puts("\n\n------save2\n\n"); */
#ifdef CONFIG_VIDEO3_ENABLE
    if (__this->video_online[3] && (dev_id == 3)) {
        err = video3_rec_savefile();
        if (err) {
            goto __err;
        }
    }
#endif

#ifdef CONFIG_VIDEO4_ENABLE

    /* for (other_id = 0; other_id < CONFIG_VIDEO_REC_NUM; other_id++) { */
    /* if ((other_id != dev_id) && __this->video_online[other_id] && __this->video_rec[other_id]) { */
    /* req.rec.channel = other_id; */
    /* req.rec.state = VIDEO_STATE_PKG_PAUSE_RUN; */
    /* err = server_request(__this->video_rec[other_id], VIDEO_REQ_REC, &req); */
    /* } */
    /* } */

    if (__this->video_online[dev_id]) {
        err = video4_rec_savefile(dev_id);
        if (err) {
            log_i("\n\n\nv%d_serr %x\n\n", dev_id, err);
            goto __err;
        }
    }

    /* for (other_id = 0; other_id < CONFIG_VIDEO_REC_NUM; other_id++) { */
    /* if ((other_id != dev_id) && __this->video_online[other_id] && __this->video_rec[other_id]) { */
    /* req.rec.channel = other_id; */
    /* req.rec.state = VIDEO_STATE_PKG_PAUSE_RUN; */
    /* err = server_request(__this->video_rec[other_id], VIDEO_REQ_REC, &req); */
    /* } */
    /* } */

#endif
    __this->state = VIDREC_STA_START;

#if (APP_CASE == __WIFI_CAR_CAMERA__)
    video_rec_state_notify();
#endif
    sys_key_event_enable();
    sys_touch_event_enable();

    malloc_stats();


    return 0;


__err:


#ifdef CONFIG_VIDEO4_ENABLE

    /* for (other_id = 0; other_id < CONFIG_VIDEO_REC_NUM; other_id++) { */
    /* [> if ((other_id != dev_id) && __this->video_online[other_id] && __this->video_rec[other_id]) { <] */
    /* if (__this->video_online[other_id] && __this->video_rec[other_id]) { */
    /* req.rec.IP_interval = 0; */
    /* req.rec.channel = other_id; */
    /* req.rec.state = VIDEO_STATE_V4_PAUSE_RUN; */
    /* err = server_request(__this->video_rec[other_id], VIDEO_REQ_REC, &req); */
    /* } */
    /* } */

    for (dev_id = 0; dev_id < CONFIG_VIDEO_REC_NUM; dev_id++) {
        err = video4_rec_stop(dev_id, 0);
        if (err) {
            printf("\nsave wrong%d %x\n", dev_id, err);
        }
    }
#endif


#ifdef CONFIG_VIDEO3_ENABLE
    err = video3_rec_stop(0);
    if (err) {
        printf("\nsave wrong3 %x\n", err);
    }
#endif

#if (defined CONFIG_VIDEO1_ENABLE  && !defined CONFIG_SINGLE_VIDEO_REC_ENABLE)
    err = video1_rec_stop(0);
    if (err) {
        printf("\nsave wrong1 %x\n", err);
    }
#endif

#ifdef CONFIG_VIDEO0_ENABLE
    err = video0_rec_stop(0);
    if (err) {
        printf("\nsave wrong0 %x\n", err);
    }
#endif


    video_rec_post_msg("offREC");
    video_home_post_msg("offREC");//录像切到后台,ui消息由主界面响应
    video_parking_post_msg("offREC");
    __this->state = VIDREC_STA_STOP;

    sys_key_event_enable();
    sys_touch_event_enable();

    return -EFAULT;

}



/*
 * 录像时拍照的控制函数, 不能单独调用，必须录像时才可以调用，实际的调用地方已有
 * 录像时拍照会需要至少1.5M + 400K的空间，请根据实际情况来使用
 */
static int video_rec_take_photo(void)
{
    struct server *server;
    union video_req req = {0};
    struct video_text_osd text_osd;
    struct video_graph_osd graph_osd;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;
    int err;

#ifdef CONFIG_VIDEO4_ENABLE
    server = __this->video_rec[__this->photo_camera_sel];
#else
    if (__this->photo_camera_sel == 0) {
        server = __this->video_rec0;
    } else if (__this->photo_camera_sel == 1) {
        server = __this->video_rec1;
    } else if (__this->photo_camera_sel == 2) {
        server = __this->video_rec2;
    } else {
        /* server = __this->video_rec4; */
        server = NULL;
    }
#endif
    if ((__this->state != VIDREC_STA_START) || (server == NULL)) {
        return -EINVAL;
    }

    if (__this->cap_buf == NULL) {
#ifndef VIDEO_REC_NO_MALLOC
        __this->cap_buf = (u8 *)malloc(CAMERA_CAP_BUF_SIZE);
#endif
        if (!__this->cap_buf) {
            puts("\ntake photo no mem\n");
            return -ENOMEM;
        }
    }

    req.icap.width = rec_pix_w[VIDEO_RES_720P];
    req.icap.height = rec_pix_h[VIDEO_RES_720P];
    req.icap.quality = VIDEO_MID_Q;
    req.icap.text_label = NULL;
    req.icap.buf = __this->cap_buf;
    req.icap.buf_size = CAMERA_CAP_BUF_SIZE;
#ifdef CONFIG_VIDEO4_ENABLE
    req.icap.camera_type = VIDEO_CAMERA_MUX;
#endif
    if (__this->photo_camera_sel == 0) {
        req.icap.path = CAMERA0_CAP_PATH"jpeg****.jpg";
    } else if (__this->photo_camera_sel == 1) {
        req.icap.path = CAMERA1_CAP_PATH"jpeg****.jpg";
    } else if (__this->photo_camera_sel == 2) {
        req.icap.path = CAMERA1_CAP_PATH"jpeg****.jpg";
    } else {
        req.icap.path = CAMERA2_CAP_PATH"jpeg****.jpg";
    }
    req.icap.src_w = __this->src_width[__this->photo_camera_sel];
    req.icap.src_h = __this->src_height[__this->photo_camera_sel];

    /*
     * osd 相关的参数，注意坐标位置，x要64对齐，y要16对齐,底下例子是根据图像大小偏移到右下
     */

#ifndef CONFIG_UI_ENABLE
    memcpy(video_rec_osd_buf, osd_str_buf, strlen(osd_str_buf));
#endif
    text_osd.font_w = 16;
    text_osd.font_h = 32;
    max_one_line_strnum = strlen(video_rec_osd_buf);

    /* osd_line_num = 1; */
    /* if (db_select("num")) { */
    osd_line_num = 2;
    /* } */

    osd_max_heigh = (req.icap.height == 1088) ? 1080 : req.icap.height ;
    text_osd.x = (req.icap.width - max_one_line_strnum * text_osd.font_w) / 64 * 64;
    text_osd.y = (osd_max_heigh - text_osd.font_h * osd_line_num) / 16 * 16;
    text_osd.color[0] = 0xe20095;
    text_osd.bit_mode = 1;
    text_osd.text_format = video_rec_osd_buf;
    text_osd.font_matrix_table = osd_str_total;
    text_osd.font_matrix_base = osd_mimc_str_matrix;//osd_str_matrix;
    text_osd.font_matrix_len = sizeof(osd_mimc_str_matrix);

    text_osd.direction = 1;

    graph_osd.bit_mode = 16;//2bit的osd需要配置3个color
    graph_osd.x = 0;
    graph_osd.y = 0;
    graph_osd.width = 256;
    graph_osd.height = 256;
    graph_osd.icon = (u8 *)icon_16bit_data;
    graph_osd.icon_size = sizeof(icon_16bit_data);

    req.icap.text_label = &text_osd;
    req.icap.graph_label = NULL;//&graph_osd;

    err = server_request(server, VIDEO_REQ_IMAGE_CAPTURE, &req);
    if (err != 0) {
        puts("\n\n\ntake photo err\n\n\n");
        return -EINVAL;
    }

#ifndef VIDEO_REC_NO_MALLOC
    if (__this->cap_buf) {
        free(__this->cap_buf);
        __this->cap_buf = NULL;
    }
#endif

    return 0;
}


#if 0
int video_rec_osd_ctl(u8 onoff)
{
    int err;

    if (__this->state == VIDREC_STA_START) {
        return -EFAULT;
    }

    __this->rec_info->osd_on = onoff;
#ifdef CONFIG_VIDEO0_ENABLE
    err = video0_rec_osd_ctl(__this->rec_info->osd_on);
#endif // VREC0_EN

#if (defined CONFIG_VIDEO1_ENABLE && !defined CONFIG_SINGLE_VIDEO_REC_ENABLE)
    err = video1_rec_osd_ctl(__this->rec_info->osd_on);
#endif

    return err;
}

int video_rec_set_white_balance()
{
    union video_req req = {0};

    if (!__this->white_balance_set) {
        return 0;
    }

    req.camera.mode = ISP_MODE_IMAGE_CAPTURE;
    req.camera.white_blance = __this->rec_info->wb_val;
    req.camera.cmd = SET_CAMERA_WB ;

    if (__this->video_display[0]) {
        server_request(__this->video_display[0], VIDEO_REQ_CAMERA_EFFECT, &req);
    } else if (__this->video_rec0 && (__this->state == VIDREC_STA_START)) {
        server_request(__this->video_rec0, VIDEO_REQ_CAMERA_EFFECT, &req);
    } else {
        puts("\nvrec set wb fail\n");
        return 1;
    }

    __this->white_balance_set = 0;

    return 0;
}
#endif


int video_rec_set_exposure(u32 exp)
{
    union video_req req = {0};

    if (!__this->exposure_set) {
        return 0;
    }

    req.camera.mode = ISP_MODE_IMAGE_CAPTURE;
    req.camera.ev = exp;
    req.camera.cmd = SET_CAMERA_EV;

    if (__this->video_display[0]) {
        server_request(__this->video_display[0], VIDEO_REQ_CAMERA_EFFECT, &req);
    } else if (__this->video_rec0 && (__this->state == VIDREC_STA_START)) {
        server_request(__this->video_rec0, VIDEO_REQ_CAMERA_EFFECT, &req);
    } else {
        return 1;
    }

    __this->exposure_set = 0;

    return 0;
}

/*
 *场景切换使能函数，如果显示打开就用显示句柄控制，否则再尝试用录像句柄控制
 */
static int video_rec_start_isp_scenes()
{

   //  printf("\n ==============page=============%d,%s\n ",__LINE__,__FUNCTION__);
#ifdef CONFIG_VIDEO4_ENABLE
    return 0;
#endif
    if (__this->isp_scenes_status) {
        return 0;
    }

    stop_update_isp_scenes();

    if (__this->video_display[0]) {
        __this->isp_scenes_status = 1;
        return start_update_isp_scenes(__this->video_display[0]);
    } else if (__this->video_rec0 && ((__this->state == VIDREC_STA_START) ||
                                      (__this->state == VIDREC_STA_STARTING))) {
        __this->isp_scenes_status = 2;
        return start_update_isp_scenes(__this->video_rec0);
    }

    __this->isp_scenes_status = 0;

    return 1;
}

static int video_rec_stop_isp_scenes(u8 status, u8 restart)
{

    if (__this->isp_scenes_status == 0) {
        return 0;
    }

    if ((status != __this->isp_scenes_status) && (status != 3)) {
        return 0;
    }

    __this->isp_scenes_status = 0;
    stop_update_isp_scenes();

    if (restart) {
        video_rec_start_isp_scenes();
    }

    return 0;
}

static u8 page_main_flag = 0;
static u8 page_park_flag = 0;
void set_page_main_flag(u8 flag)
{
    page_main_flag = flag;
}

void test(void *priv)
{
    while(!storage_device_ready()) {
        msleep(100);
    }
#if 0
    play_video(CONFIG_ROOT_PATH"2.AVI");
#else

#include "bplus.h"

static bp_db_t db = {0};
bp_open(&db, CONFIG_ROOT_PATH"INDEX.DAT");

    char str[20] = {0};
    snprintf(str, 20, "%d", 1234567);
    bp_key_t key;
    bp_value_t value;

    key.length = strlen(str) + 1;
    key.value  = str;
    int h= 124;
    value.length = 4;
    value.value  = &h;
        bp_set(&db, &key, &value);


    bp_close(&db);

#endif

    while(1){
        msleep(100);
    }
}

static int show_main_ui()
{

//    video_disp_show_diy();
#if 0
	extern void test(void *priv);
    task_create( test, NULL,"test");
#endif

#ifdef CONFIG_UI_ENABLE

#ifdef CONFIG_IPC_UI_ENABLE
    extern  int UVC_DETECT_TIME;
    printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>UVC_DETECT_TIME = %d\n", timer_get_ms() - UVC_DETECT_TIME);
    extern int dec_pro_file(char *path,int flag);
   // dec_pro_file(NULL);
    int screen_number =db_select("screen");
    printf("\n  screen_number=====================%d\n",screen_number);

    if(screen_number>3){

    screen_number=0;
    db_update("screen",screen_number);
    db_flush();
    }

    dec_pro_file(background_cfg_file[screen_number],0); //第一次默认显示第一张图
    show_standby_ui();

#else
    union uireq req;

    if (page_main_flag) {
        return 0;
    }
    if (!__this->ui) {
        return -1;
    }

    puts("show_main_ui\n");
    req.show.id = ID_WINDOW_VIDEO_REC;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
    page_main_flag = 1;
#endif

#endif

    return 0;
}

static int show_park_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (page_park_flag) {
        return 0;
    }
    if (!__this->ui) {
        return -1;
    }

    puts("show_park_ui\n");
//    req.show.id = ID_WINDOW_PARKING;
//    server_request_async(__this->ui, UI_REQ_SHOW, &req);
    page_park_flag = 1;
#endif

    return 0;
}

static int show_lane_set_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        return -1;
    }

//    req.show.id = ID_WINDOW_LANE;
//    server_request_async(__this->ui, UI_REQ_SHOW, &req);
#endif

    return 0;
}

static void hide_main_ui()
{
#ifdef CONFIG_UI_ENABLE

#ifdef CONFIG_IPC_UI_ENABLE
    hide_standby_ui();
#else
    union uireq req;

    if (page_main_flag == 0) {
        return;
    }
    if (!__this->ui) {
        puts("__this->ui == NULL!!!!\n");
        return;
    }

    puts("hide_main_ui\n");

    req.hide.id = ID_WINDOW_VIDEO_REC;
    server_request(__this->ui, UI_REQ_HIDE, &req);
    page_main_flag = 0;
#endif // CONFIG_IPC_UI_ENABLE


#endif
}

static void hide_home_main_ui()
{
#ifdef CONFIG_UI_STYLE_JL02_ENABLE
    union uireq req;

    if (!__this->ui) {
        puts("__this->ui == NULL!!!!\n");
        return;
    }

    puts("hide_home_main_ui\n");

    req.hide.id = ID_WINDOW_MAIN_PAGE;
    server_request(__this->ui, UI_REQ_HIDE, &req);
#endif
}

static void hide_park_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (page_park_flag == 0) {
        video_rec_get_remain_time();
        return;
    }
    if (!__this->ui) {
        puts("__this->ui == NULL!!!!\n");
        return;
    }

    puts("hide_park_ui\n");

    req.hide.id = ID_WINDOW_PARKING;
    server_request(__this->ui, UI_REQ_HIDE, &req);
    page_park_flag = 0;
#endif
}

int video_rec_storage_device_ready(void *p)
{

    __this->sd_wait = 0;

    video_rec_scan_lock_file();

#if (APP_CASE == __WIFI_CAR_CAMERA__)
#ifdef CONFIG_ENABLE_VLIST
    FILE_LIST_SCAN();
#endif
#endif
    if ((int)p == 1) {
#if (APP_CASE == __WIFI_CAR_CAMERA__)
        video_rec_start_notify();//先停止网络实时流再录像,录像完毕再通知APP
#else


#if  1


   printf("\n cyc :%d,%d\n",db_select("cyc"),db_select("mot"));

    printf("\n video_rec_storage_device_ready \n");

    post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_ENTER_CHECK_TIMER_REC);// 检测时间

#endif

#endif

    }

    return 0;
}


static int video_rec_sd_in()
{

#if (APP_CASE == __WIFI_CAR_CAMERA__)
    video_rec_sd_event_ctp_notify(1);
#endif
    if (__this->state == VIDREC_STA_IDLE) {
        video_rec_get_remain_time();
        ve_mdet_stop();
        ve_lane_det_stop(0);
    }

    if (__this->menu_inout == 0) {
        ve_mdet_start();
        ve_lane_det_start(0);
        ve_face_det_start(0);
    }
#if (APP_CASE == __WIFI_CAR_CAMERA__)
    net_video_rec_status_notify();
#endif

    __this->lock_fsize_count = 0;

    return 0;
}

static int video_rec_sd_out()
{
    ve_mdet_stop();
    ve_lane_det_stop(0);

    video_rec_fscan_release(0);
    video_rec_fscan_release(1);

    if (__this->sd_wait == 0) {
        __this->sd_wait = wait_completion(storage_device_ready,
                                          video_rec_storage_device_ready, (void *)1);
    }

#if (APP_CASE == __WIFI_CAR_CAMERA__)
    video_rec_sd_event_ctp_notify(0);
#endif

    return 0;
}

static void video_rec_park_call_back(void *priv)
{
    if (__this->state == VIDREC_STA_START) {
        video_rec_stop(0);
        if (usb_is_charging() && (__this->state == VIDREC_STA_STOP)) {
            video_rec_start();
        } else {
            puts("park rec off power close\n");
            sys_power_shutdown();
        }
    }
}

static int video_rec_park_wait(void *priv)
{
    int err = 0;

    puts("video_rec_park_wait\n");

    if (__this->state != VIDREC_STA_START) {
        puts("park_rec_start\n");
        err = video_rec_start();
    }

    if (err == 0) {
        sys_timeout_add(NULL, video_rec_park_call_back, 30 * 1000);
        if (__this->park_wait_timeout) {
            sys_timeout_del(__this->park_wait_timeout);
            __this->park_wait_timeout = 0;
        }
    }

    return 0;
}

static void video_rec_park_wait_timeout(void *priv)
{
    if (__this->state == VIDREC_STA_START) {
        return;
    }
    puts("park wait timeout power close\n");
    sys_power_shutdown();
}

int lane_det_setting_disp()
{
    u32 err = 0;
#ifdef CONFIG_VIDEO0_ENABLE
    struct video_window win = {0};

    video_disp_stop(1);

    u16 dis_w = 640 * SCREEN_H / 352 / 16 * 16;
    dis_w = dis_w > SCREEN_W ? SCREEN_W : dis_w;

    printf("lane dis %d x %d\n", dis_w, SCREEN_H);

    win.top             = 0;
    win.left            = (SCREEN_W - dis_w) / 2 / 16 * 16;
    win.width           = dis_w;
    win.height          = SCREEN_H;
    win.border_left     = 0;
    win.border_right    = 0;
    win.border_top      = 0;
    win.border_bottom   = 0;
    err = video_disp_start(0, &win);
    show_lane_set_ui();
#endif
    return err;
}

static int video_rec_usb_device_ready(void *p)
{
    __this->usb_wait = 0;

    extern int usb_connect(u32 state);
    usb_connect(USB_CAMERA);

    return 0;
}
void video_rec_buf_malloc(void)
{
    static DEFINE_SPINLOCK(video_rec_buf_malloc_lock);


    spin_lock(&video_rec_buf_malloc_lock);

#ifdef VIDEO_REC_NO_MALLOC
#ifdef CONFIG_VIDEO4_ENABLE
    int buf_size[] = {VREC4_FBUF_SIZE, VREC4_FBUF_SIZE, VREC4_FBUF_SIZE, VREC4_FBUF_SIZE};
#else
#ifndef CONFIG_SINGLE_VIDEO_REC_ENABLE
    int buf_size[] = {VREC0_FBUF_SIZE, VREC1_FBUF_SIZE, VREC2_FBUF_SIZE, VREC3_FBUF_SIZE};
#else
    int buf_size[] = {VREC0_FBUF_SIZE};
#endif
#endif
    for (int i = 0; i < ARRAY_SIZE(buf_size); i++) {
        if (buf_size[i]) {
            if (!__this->video_buf[i]) {
                __this->video_buf[i] = malloc(buf_size[i]);

                if (__this->video_buf[i] == NULL) {
                    log_i("err maloo\n");
                    while (1);
                }
            }
        } else {
            __this->video_buf[i] = NULL;
        }
    }
    if (!__this->cap_buf) {
        __this->cap_buf = (u8 *)malloc(CAMERA_CAP_BUF_SIZE);
    }

#endif
    spin_unlock(&video_rec_buf_malloc_lock);

}

static int video_rec_init()
{
    int err = 0;

    void sd1_out_timeout(u8 timeout);
    sd1_out_timeout(100);


#if (LCD_DEV_WIDTH != 1)
    lcd_init_wait_done();
	dec_server_open();
#endif
   printf("\n video_rec_init  \n");
    ve_server_open(0);


#if (CONFIG_VIDEO_PARK_DECT == 1)
    __this->disp_park_sel = 1;
#elif (CONFIG_VIDEO_PARK_DECT == 3)
    __this->disp_park_sel = 2;
#elif (CONFIG_VIDEO_PARK_DECT == 4)
    __this->disp_park_sel = 3;
#else
    __this->disp_park_sel = 0;
#endif

    video_rec_buf_malloc();



#ifdef CONFIG_VIDEO0_ENABLE
//    __this->video_online[0] = 1;
//    err = video_disp_start(0, &disp_window[DISP_MAIN_WIN][0]);
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    __this->video_online[1] = dev_online("video1.*");
    if (__this->video_online[1]) {
        /* err = video_disp_start(1, &disp_window[DISP_MAIN_WIN][1]); */
    }
#endif

    /* #ifdef CONFIG_VIDEO2_ENABLE */
    /* __this->video_online[2] = 1; */
    /* err = video_disp_start(2, &disp_window[DISP_MAIN_WIN][1]); */
    /* #endif */

#ifdef CONFIG_VIDEO3_ENABLE
    __this->video_online[3] = dev_online("uvc");
    /* #if 1 */
    /*     if (__this->video_online[3]) { */
    /*         err = video_disp_start(3, &disp_window[DISP_MAIN_WIN][1]); */
    /*     } */
    /* #endif */
#endif


#ifdef CONFIG_VIDEO4_ENABLE
    __this->video_online[0] = 1;
    __this->video_online[1] = 1;
    __this->video_online[2] = 1;
    __this->video_online[3] = 1;
    err = video_disp_start(0, &disp_window[DISP_MAIN_WIN][0]);
    /* err = video_disp_start(0, &disp_window[DISP_HALF_WIN][0]); */
    /* err = video_disp_start(1, &disp_window[DISP_HALF_WIN][1]); */
#endif

    __this->disp_state = DISP_MAIN_WIN;
    __this->second_disp_dev = 0;



#ifdef CONFIG_PARK_ENABLE
    if (get_parking_status()) {
        show_park_ui();
    } else {
        show_main_ui();
    }
#else
    show_main_ui();

#endif
    video_rec_get_remain_time();


#ifdef CONFIG_GSENSOR_ENABLE
    if (!strcmp(sys_power_get_wakeup_reason(), "wkup_port:wkup_gsen")) {
        if (db_select("par")) {
            __this->gsen_lock = 0xff;
            __this->park_wait_timeout = sys_timeout_add(NULL, video_rec_park_wait_timeout, 10 * 1000);
            __this->park_wait = wait_completion(storage_device_ready,
                                                video_rec_park_wait, NULL);
        }
        sys_power_clr_wakeup_reason("wkup_port:wkup_gsen");
    } else {
        __this->sd_wait = wait_completion(storage_device_ready,
                                          video_rec_storage_device_ready, 0);
    }
#else
/*
    __this->sd_wait = wait_completion(storage_device_ready,
                                      video_rec_storage_device_ready, (void *)1);
  */
#endif

#if 0
    if (get_parking_status()) {
        video_disp_win_switch(DISP_WIN_SW_SHOW_PARKING, 0);
    } else {
        video_disp_win_switch(DISP_WIN_SW_SHOW_SMALL, 0);
    }
#endif


#ifdef CONFIG_USB_VIDEO_OUT
    extern int usb_device_ready();
    __this->usb_wait = wait_completion(usb_device_ready,
                                       video_rec_usb_device_ready, 0);
#endif

    printf("\n  L\n");

    os_time_dly(100);
    printf("\n  L1\n");

    extern void Motor_Auto_calibration();
    Motor_Auto_calibration();

    return err;
}




static int video_rec_uninit()
{
    int err;
    union video_req req = {0};

    if (__this->state == VIDREC_STA_START) {
        return -EFAULT;
    }
    if (__this->park_wait) {
        wait_completion_del(__this->park_wait);
        __this->park_wait = 0;
    }
    if (__this->sd_wait) {
        wait_completion_del(__this->sd_wait);
        __this->sd_wait = 0;
    }
    if (__this->char_wait) {
        wait_completion_del(__this->char_wait);
        __this->char_wait = 0;
    }
    if (__this->park_wait_timeout) {
        sys_timeout_del(__this->park_wait_timeout);
        __this->park_wait_timeout = 0;
    }
    if (__this->usb_wait) {
        wait_completion_del(__this->usb_wait);
        __this->usb_wait = 0;
    }

    video_rec_stop_isp_scenes(3, 0);

    ve_server_close();

    if (__this->state == VIDREC_STA_START) {
        err = video_rec_stop(1);
    }
    video_rec_close();


    video_rec_fscan_release(0);
    video_rec_fscan_release(1);

    for (int i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
        video_disp_stop(i);
    }

#ifdef VIDEO_REC_NO_MALLOC
    for (int i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
        if (__this->video_buf[i]) {
            free(__this->video_buf[i]);
            __this->video_buf[i] = NULL;
        }
    }
    if (__this->cap_buf) {
        free(__this->cap_buf);
        __this->cap_buf = NULL;
    }
#endif

    __this->disp_state = DISP_FORBIDDEN;
    __this->state = VIDREC_STA_FORBIDDEN;
    __this->lan_det_setting = 0;

    void sd1_out_timeout(u8 timeout);
    sd1_out_timeout(0);

#ifdef CONFIG_USB_VIDEO_OUT
    extern int usb_disconnect();
    usb_disconnect();
#endif

    return 0;

}


static int video_rec_mode_sw()
{
    if (__this->state != VIDREC_STA_FORBIDDEN) {
        return -EFAULT;
    }

#ifndef VIDEO_REC_NO_MALLOC
    for (int i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
        if (__this->video_buf[i]) {
            free(__this->video_buf[i]);
            __this->video_buf[i] = NULL;
        }
    }
#endif



    for (int i = 0; i < CONFIG_VIDEO_REC_NUM; i++) {
        if (__this->audio_buf[i]) {
            free(__this->audio_buf[i]);
            __this->audio_buf[i] = NULL;
        }
    }

    if (__this->cap_buf) {
        free(__this->cap_buf);
        __this->cap_buf = NULL;
    }

    return 0;
}


/*
 *菜单相关的函数
 */
static int video_rec_change_status(struct intent *it)
{
    if (!strcmp(it->data, "opMENU:")) { /* ui要求打开rec菜单 */
        puts("ui ask me to opMENU:.\n");

        if ((__this->state != VIDREC_STA_START) && (__this->state != VIDREC_STA_FORBIDDEN)) { /* 允许ui打开菜单 */
            __this->menu_inout = 1;
            if (db_select("mot")) {
                ve_mdet_stop();
            }
            if (db_select("lan")) {
                ve_lane_det_stop(0);
            }

            it->data = "opMENU:en";
        } else { /* 禁止ui打开菜单 */
            it->data = "opMENU:dis";
        }

    } else if (!strcmp(it->data, "exitMENU")) { /* ui已经关闭rec菜单 */
        puts("ui tell me exitMENU.\n");
        __this->menu_inout = 0;

        video_rec_get_remain_time();
        video_rec_fun_restore();
        if (db_select("mot")) {
            ve_mdet_start();
        }
        if (db_select("lan")) {
            ve_lane_det_start(0);
        }
        if (db_select("fac")) {
            ve_face_det_start(0);
        }
    } else if (!strcmp(it->data, "sdCard:")) {
        video_rec_get_remain_time();
        if (storage_device_ready() == 0) {
            it->data = "offline";
        } else {
            it->data = "online";
        }
    } else {
        puts("unknow status ask by ui.\n");
    }

    return 0;
}

static int video_disp_move(u16 width, u16 height, u16 xoff, u16 yoff,
                           u16 b_left, u16 b_top, u16 b_right, u16 b_bottom)
{
    union video_req req = {0};
    req.display.fb 		= "fb1";

    req.display.left  	= xoff;
    req.display.top 	= yoff;

    req.display.width 	= width;
    req.display.height 	= height;

    req.display.border_left   = b_left;
    req.display.border_top    = b_top;
    req.display.border_right  = b_right;
    req.display.border_bottom = b_bottom;

    req.display.uvc_id = 0;
    req.display.camera_config = NULL;
    req.display.camera_type     = VIDEO_CAMERA_NORMAL;

    req.display.state 	= VIDEO_STATE_CFG;
    req.display.pctl = NULL;

    server_request(__this->video_display[0], VIDEO_REQ_DISPLAY, &req);


    return 0;
}



void  *get_disp_video(){

    return __this->video_display[0];
}

static u8 reverse = 1;
/* static u8 g_fps = 30; */
/*
 *录像的状态机,进入录像app后就是跑这里
 */
u32 call_start_timer=0;
u32 call_end_timer=0;
#if 0
u32 get_start_call_timer(){

    return call_start_timer;


}

u32 get_end_call_timer(){

    return call_end_timer;


}
#endif
static int video_rec_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int err = 0;
    int len;

    union uireq req;


    switch (state) {
    case APP_STA_CREATE:
        puts("--------app_rec: APP_STA_CREATE\n");



        memset(__this, 0, sizeof(struct video_rec_hdl));
        //video_rec_buf_alloc();
        server_load(video_server);
#ifdef CONFIG_UI_ENABLE
        __this->ui = server_open("ui_server", NULL);
        if (!__this->ui) {
            return -EINVAL;
        }
#endif
        video_set_disp_window();
        video_rec_config_init();
        __this->state = VIDREC_STA_IDLE;



        break;
    case APP_STA_START:
        puts("--------app_rec: APP_STA_START\n");
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_VIDEO_REC_MAIN:
            puts("ACTION_VIDEO_REC_MAIN\n");

            if (it->data && !strcmp(it->data, "lan_setting")) {
                __this->lan_det_setting = 1;
                //puts("ACTION_VIDEO_REC_MAIN\n");
                ve_server_open(1);
                lane_det_setting_disp();
            } else {
                video_rec_init();
            }

            break;
        case ACTION_VIDEO_REC_SET_CONFIG:
            if (!__this->ui) {
                return -EINVAL;
            }
            video_rec_set_config(it);
            db_flush();
            if (it->data && !strcmp(it->data, "res")) {
                video_rec_get_remain_time();
            }
            if (it->data && !strcmp(it->data, "gap")) {
                __this->need_restart_rec = 1;
                video_rec_get_remain_time();
            }
            if (it->data && !strcmp(it->data, "mic")) {
                video_rec_aud_mute();
            }
            if (it->data && !strcmp(it->data, "cyc")) {
                video_rec_len();
            }

/************************** 可视对讲增加内容************************/
#ifdef  CONFIG_IPC_UI_ENABLE
            puts("standby_set_config.\n");
            if(it->data){
                printf("it data : %s.\n", it->data);
            }
            else {
                printf("it data : NULL.\n", it->data);
            }

            if (!strcmp(it->data, "rec:dec")) {  //查看视频
                void change_app_to_dec();
                change_app_to_dec();

            }else if (!strcmp(it->data, "rec:cal")) {  //查看视频
                video_disp_stop(0);
                show_main_ui();
            }
#endif
/******************************************************************/
            break;
/************************** 可视对讲增加内容************************/
#ifdef  CONFIG_IPC_UI_ENABLE
        case ACTION_VIDEO_CALLING_GET_CONTACT:          //获取联系人

            if(get_video_call_state() == CALL_STA_IDLE){
                printf("\n/***[ACTION_VIDEO_CALLING_GET_CONTACT] sel_contact\n ");


                video_standby_post_msg("sel_contact:a=%p",get_user_list_t_info());

                printf("\n/***[ACTION_VIDEO_CALLING_GET_CONTACT] sel_contact finish \n");
            }

            break;
        case ACTION_VIDEO_CALLING_SET_CONTACT:          //向选择的联系人拨号


        #if  0
            printf("\n/***[ACTION_VIDEO_CALLING_SET_CONTACT] sel_contact\n ");
            RT_TALK_INFO  info = {0};
            if(find_device_video_play_info(&info) || find_audio_play_info()){

                printf("\n\n\n  find_device_video_play_info(&info) = %d  find_audio_play_info() = %d \n\n\n",find_device_video_play_info(&info), find_audio_play_info());
                notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"calling_fail",0);
                break;
            }

       /*************** 拨号实际逻辑处理 *****************/
            user_list_t *p = get_user_list_t_info();
            printf("\n/*** set_contact = %d \n",it->exdata);
            if(it->exdata < p->users_count){
                void *user = calloc(1,sizeof(user_info_t));
                memcpy(user,&p->users[it->exdata],sizeof(user_info_t));

                post_msg_doorbell_task("doorbell_block_event_task", 3, DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE, backup_wakeup_status,user);
            }else{
                printf("\n /*** call to all\n ");
                post_msg_doorbell_task("doorbell_block_event_task", 3, DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE, backup_wakeup_status,NULL);
            }

            //post_msg_doorbell_task("doorbell_block_event_task", 3, DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE, backup_wakeup_status,NULL);
        /****************** 拨号UI逻辑  *******************/
            sel_contact_flag = 1;
            struct intent it1;
            init_intent(&it1);
            it1.name = "video_rec";
            it1.action = ACTION_VIDEO_REC_CHANGE_WORK;
            it1.data = "calling";
            int err = start_app(&it1);
            if (err) {
                printf("ACTION_VIDEO_REC_CHANGE_WORK err! %d\n", err);
            }
//                video_standby_post_msg("set_contact_finish");
        /****************************************************/
        #else
         printf("\n/***[ACTION_VIDEO_CALLING_SET_CONTACT] sel_contact\n ");
            RT_TALK_INFO  info = {0};
            if(find_device_video_play_info(&info) || find_audio_play_info()){

                printf("\n\n\n  find_device_video_play_info(&info) = %d  find_audio_play_info() = %d \n\n\n",find_device_video_play_info(&info), find_audio_play_info());
                notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"calling_fail",0);
                break;
            }

       /*************** 拨号实际逻辑处理 *****************/
            user_list_t *p = get_user_list_t_info();
            printf("\n/*** set_contact = %d \n",it->exdata);

            printf("\n /*** call to all\n ");
            post_msg_doorbell_task("doorbell_block_event_task", 3, DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE, backup_wakeup_status,NULL);

            //post_msg_doorbell_task("doorbell_block_event_task", 3, DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE, backup_wakeup_status,NULL);
        /****************** 拨号UI逻辑  *******************/
          //  sel_contact_flag = 1;
            sel_contact_flag = 0;
            struct intent it1;
            init_intent(&it1);
            it1.name = "video_rec";
            it1.action = ACTION_VIDEO_REC_CHANGE_WORK;
            it1.data = "calling";
            int err = start_app(&it1);
            if (err) {
                printf("ACTION_VIDEO_REC_CHANGE_WORK err! %d\n", err);
            }

        #endif

            printf("\n/***[ACTION_VIDEO_CALLING_SET_CONTACT] sel_contact finish \n");

            break;

        case ACTION_VIDEO_REC_CHANGE_WORK:
            printf("\n/*** video_call_state = %d\n",get_video_call_state());
             //工作状态变更，立刻执行定时服务并去掉定时器：隐藏通话的提示界面()
            if(rec_call_dispose_timer){
                printf("\n/*** rec_dispose_timer_handler early\n");
                rec_dispose_timer_handler(rec_call_dispose_mode);
            }
            if(hide_ui_busy_timer){
                printf("\n/*** hide_ui_busy_timer_handler early\n");
                hide_ui_busy_timer_handler();
            }

            if (!strcmp(it->data, "calling")) {                 //拨号
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] calling\n ");
                if((get_video_call_state() == CALL_STA_IDLE || \
                   get_video_call_state() == CALL_STA_GET_CONTACT) && \
                   video_net_state == NET_STA_CONNECT_SUCC){
                    video_standby_post_msg("calling_request");
                    calling_timeout_timer = sys_timeout_add(NULL,calling_timeout,30*1000); //呼叫超时
                }else if(video_net_state != NET_STA_CONNECT_SUCC){  //网络未通
                    video_standby_post_msg("calling_net_connectfail");
                    rec_call_dispose_mode = 4;
                    rec_call_dispose_timer = sys_timeout_add(rec_call_dispose_mode,rec_dispose_timer_handler,1000);
                    if(calling_timeout_timer){
                        sys_timer_del(calling_timeout_timer);
                        calling_timeout_timer = NULL;
                    }//拨号失败，删除超时定时器
//                    video_net_state =   NET_STA_DISCONNECT;
                }
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] calling finish \n ");


            }else if (!strcmp(it->data, "machine_close_calling")) { //机器主动挂断
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] machine_close_calling\n ");
                dec_pro_file(NULL,0);
                if(calling_timeout_timer){
                    sys_timeout_del(calling_timeout_timer);
                    calling_timeout_timer = NULL;
                }//已挂断，关掉超时定时器


                #if  0

                video_standby_post_msg("machine_close_calling");
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] machine_close_calling finish \n");
                #else

                rec_call_dispose_mode = 1;
                video_standby_post_msg("machine_close_call:ascii=%p","machine");
                rec_call_dispose_timer = sys_timeout_add(rec_call_dispose_mode,rec_dispose_timer_handler,2000);

                #endif

            }else if (!strcmp(it->data, "machine_close_calling_fail")) { //机器主动挂断失败，允许挂断
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] machine_close_calling\n ");

                video_standby_post_msg("app_audio_talk_busy");
                if(!hide_ui_busy_timer){
                    hide_ui_busy_timer = sys_timeout_add(NULL,hide_ui_busy_timer_handler,1000);
                }
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] machine_close_calling finish \n");


            }else if (!strcmp(it->data, "app_accept_call")) {  //app接通
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] app_accept_call\n ");
                //接收到app接受通话消息时，应start app走这一步
//                video_standby_post_msg("app_accept_call");

                //设备拨号流程，
                if(get_video_call_state()  == CALL_STA_STARTING || get_video_call_state()  == CALL_STA_TALKING_WITHOUT_VIDEO ){
                    if(calling_timeout_timer){
                        sys_timeout_del(calling_timeout_timer);
                        calling_timeout_timer = NULL;
                    }//已接通，关掉超时定时器
                    video_standby_post_msg("app_accept_call");  //ui接通响应

                }else if(get_video_call_state() == CALL_STA_IDLE||get_video_call_state() == CALL_STA_GET_CONTACT){      //设备没有拨号，app直接接通视频对讲
                    video_standby_post_msg("app_launch_call");
                }
                    printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] app_accept_call finish\n ");

                    call_start_timer=timer_get_ms();//记录app 接通电话起始时间


            }else if (!strcmp(it->data, "accept_call_without_video")) {  //app接通只进行语音通话
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] accept_call_without_video\n ");
                if(get_video_call_state()  == CALL_STA_STARTING){               //设备拨号流程
                    if(calling_timeout_timer){
                        sys_timeout_del(calling_timeout_timer);         //已接通，关掉超时定时器
                        calling_timeout_timer = NULL;
                    }

                    video_standby_post_msg("accept_call_without_video");    //ui接通响应
                }
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] accept_call_without_video finish\n ");

            }else if (!strcmp(it->data, "app_close_call")) {    //app挂断
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] app_close_call\n");
                //接收到app挂断通话时，应start app走这一步
                //需要补一个界面提示对方已挂断
                if(get_video_call_state() == CALL_STA_TALKING||get_video_call_state() == CALL_STA_TALKING_WITHOUT_VIDEO){

                    rec_call_dispose_mode = 1;
                    video_standby_post_msg("app_close_call:ascii=%p","app");
                    rec_call_dispose_timer = sys_timeout_add(rec_call_dispose_mode,rec_dispose_timer_handler,2000);
                }

                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] app_close_call finish\n ");


            }else if (!strcmp(it->data, "app_refuse_calling")) {  //app拒接
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] app_refuse_calling\n ");
                //接收到app拒绝通话请求时，应start app走这一步
                //补一个界面提示对方正在忙
//                req.hide.id = LAYER_CALL_REC;
//                server_request(__this->ui, UI_REQ_HIDE, &req);

//                req.show.id = LAYER_STANDBY;
//                server_request(__this->ui, UI_REQ_SHOW, &req);


                if(calling_timeout_timer){
                    sys_timeout_del(calling_timeout_timer);
                    calling_timeout_timer = NULL;
                }  //已被拒接，关掉超时定时器
                video_standby_post_msg("app_refuse_calling");
                rec_call_dispose_mode = 2;
                rec_call_dispose_timer = sys_timeout_add(rec_call_dispose_mode,rec_dispose_timer_handler,1000);

                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] app_refuse_calling finish\n ");


            }else if (!strcmp(it->data, "calling_timeout")) {   //app超时未接听
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] calling_timeout\n ");
                //此处需要加入网络端挂断流程
                //补一界面提示对方未接听
//                req.hide.id = LAYER_CALL_REC;
//                server_request(__this->ui, UI_REQ_HIDE, &req);
//
//                req.show.id = LAYER_STANDBY;
//                server_request(__this->ui, UI_REQ_SHOW, &req);

                video_standby_post_msg("calling_timeout");
                rec_call_dispose_mode = 3;
                rec_call_dispose_timer = sys_timeout_add(rec_call_dispose_mode,rec_dispose_timer_handler,1000);
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] calling_timeout finish\n ");

            }else if (!strcmp(it->data, "calling_net_connectfail")) {   //拨号断网
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] calling_net_connectfail\n ");

                video_standby_post_msg("calling_net_connectfail");
                rec_call_dispose_mode = 4;
                rec_call_dispose_timer = sys_timeout_add(rec_call_dispose_mode,rec_dispose_timer_handler,1000);
                if(calling_timeout_timer){
                    sys_timer_del(calling_timeout_timer);
                    calling_timeout_timer = NULL;
                }//拨号失败，删除超时定时器
                video_net_state =   NET_STA_DISCONNECT;
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] calling_net_connectfail finish\n ");

            }else if (!strcmp(it->data, "calling_fail")) {   //拨号失败
                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] calling_fail\n ");

                video_standby_post_msg("calling_fail");
                rec_call_dispose_mode = 5;
                rec_call_dispose_timer = sys_timeout_add(rec_call_dispose_mode,rec_dispose_timer_handler,1000);
                if(calling_timeout_timer){
                    sys_timer_del(calling_timeout_timer);
                    calling_timeout_timer = NULL;
                }//拨号失败，删除超时定时器

                printf("\n/***[ACTION_VIDEO_REC_CHANGE_WORK] calling_fail finish\n ");
            }
            break;

        case ACTION_VIDEO_DISTRIBUTE_NETWORK:       //配网
            if (!strcmp(it->data, "network_connect_start")) {      //开始配网
                printf("\n/***[ACTION_VIDEO_DISTRIBUTE_NETWORK] network_connect_start \n");

                //隐藏主界面，显示配网界面，还需要配置屏显窗口大小
                video_net_state = NET_STA_START;


                video_standby_post_msg("network_connect_start");
#ifdef CONFIG_VIDEO0_ENABLE



                struct video_window win  = {0};
                win.left  	        = 0;
                win.top 	        = 160;

                win.width 	        = 480; //当宽有值，高没有值时，为裁剪显示，高会根据裁剪后图像比例计算得到，当宽高都有值时，为缩放显示（如果有裁剪那就是裁剪后缩放显示）
                win.height 	        = 480;
                win.border_left     = 0;
                win.border_top      = 0;
                win.border_right    = 0;
                win.border_bottom   = 0;
                __this->video_online[0] = 1;
                err = video_disp_start(0, &win);

                union video_req req = {0};
                static struct video_source_crop crop = {0};
                crop.x_offset = 420;
                crop.y_offset = 0;
                crop.width = 1080;
                crop.height = 1080;
                req.display.mirror   		= 1;
                req.display.state    = VIDEO_STATE_SET_DIS_CROP;
                req.display.crop = &crop;

                err = server_request(__this->video_display[0], VIDEO_REQ_DISPLAY, &req);
//              printf("\n %s simulate_state=%d\n",__func__,simulate_state);
//              if(simulate_state == 1){
//              err = video_disp_start(0, &disp_window[DISP_FRONT_WIN][0]);
//              }



                printf("\n ===scan isp effect=== \n ");
                extern void isp_effect_user_day(u8 state);
                isp_effect_user_day(3);// 扫码效果



#endif

                printf("\n/***[ACTION_VIDEO_DISTRIBUTE_NETWORK] network_connect_start finish\n ");


            }else if (!strcmp(it->data, "network_connecting")) {      //正在连接
                printf("\n/***[ACTION_VIDEO_DISTRIBUTE_NETWORK] connectinging \n");
                if(video_net_state == NET_STA_START){
                    video_net_state = NET_STA_CONNECTING;
//                    if(simulate_state == 1){
//                        video_disp_stop(0);
//                        simulate_state = 0;
//                    }
                    video_disp_stop(0);
                    video_standby_post_msg("network_connecting");
                }

                printf("\n/***[ACTION_VIDEO_DISTRIBUTE_NETWORK] connectinging finish\n ");


            }else if (!strcmp(it->data, "connect_fail")) {      //连接失败
                printf("\n/***[ACTION_VIDEO_DISTRIBUTE_NETWORK] connect_fail \n");
                if(video_net_state == NET_STA_CONNECTING){
                    video_standby_post_msg("connect_fail");
                    //sys_timeout_add(NULL,return_ui_to_main,2000);
                }
                video_net_state = NET_STA_CONNECT_FAIL;

                printf("\n/***[ACTION_VIDEO_DISTRIBUTE_NETWORK] connect_fail finish\n ");


            }else if (!strcmp(it->data, "connect_succ")) {      //连接成功
                printf("\n/***[ACTION_VIDEO_DISTRIBUTE_NETWORK] connect_succ \n");
                if(video_net_state == NET_STA_CONNECTING){
                    dec_pro_file(NULL,0);
                    video_standby_post_msg("connect_succ");
                    sys_timeout_add(NULL,return_ui_to_main,2000);
                }
                video_net_state = NET_STA_CONNECT_SUCC;
                printf("\n/***[ACTION_VIDEO_DISTRIBUTE_NETWORK] connect_succ finish\n ");
            }
            break;


        case ACTION_VIDEO_REC_MSG_WORK:     //留言

            if (!strcmp(it->data, "msg_mode")) {  //进到留言模式
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_mode \n ");
                video_standby_post_msg("msg_mode");
#ifdef CONFIG_VIDEO0_ENABLE
                struct video_window win  = {0};
                win.left  	        = 0;
                win.top 	        = 0;
                win.width 	        = 480; //当宽有值，高没有值时，为裁剪显示，高会根据裁剪后图像比例计算得到，当宽高都有值时，为缩放显示（如果有裁剪那就是裁剪后缩放显示）
                win.height 	        = 800;

                __this->video_online[0] = 1;
                err = video_disp_start(0, &win);


                union video_req req = {0};
                static struct video_source_crop crop = {0};
                crop.x_offset = 636;
                crop.y_offset = 0;
                crop.width = 648;
                crop.height = 1080;

                req.display.state    = VIDEO_STATE_SET_DIS_CROP;
                req.display.crop = &crop;

                err = server_request(__this->video_display[0], VIDEO_REQ_DISPLAY, &req);
#endif
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_mode finish\n ");

            }else if (!strcmp(it->data, "msg_start")) {    //开始留言
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_start \n");

                post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_LOCAL_MSG_REC_START);
                if(!msg_finish_timer){
                    msg_finish_timer = sys_timer_add(0,msg_finish_timer_handler,1*1000);  //  留言倒计时，每秒更新一次倒计时
                }

                //此处定时器模仿留言完毕，需加入留言功能
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_start finish\n ");

            }else if (!strcmp(it->data, "msg_stop")) {    //提前停止留言，发送留言完毕
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_stop \n");

                msg_finish_timer_handler(1);//提前执行定时器服务并删除定时器
//                sys_timeout_del(msg_finish_timer);
//                post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_LOCAL_MSG_REC_STOP);
//                video_standby_post_msg("msg_finish");

                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_stop finish\n ");

            }else if (!strcmp(it->data, "msg_del")) {   //删除留言
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_del \n");
                while(get_local_msg_event_deal_flag()){
                    msleep(30);
                }

                if(now_msg_fname){
                    printf("\n/**** del file:%s\n",now_msg_fname);
                    FILE * now_msg_file = fopen(now_msg_fname,"r");
                    if(!now_msg_file){
                        printf("\n/**** now_msg_file open err\n");
                    }else{
                        fdelete(now_msg_file);
                        fclose(now_msg_file);
                        now_msg_file = NULL;
                        memset(now_msg_fname, 0, sizeof(now_msg_fname));
                    }
                }
#ifdef CONFIG_VIDEO0_ENABLE
               dec_pro_file(NULL,0);
                __this->video_online[0] = 0;
                video_disp_stop(0);
#endif
                video_standby_post_msg("msg_del");
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_del finish\n ");

            }else if (!strcmp(it->data, "msg_save")) {    //保存留言
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_save \n");
                int get_local_msg_event_deal_flag(void);
                while(get_local_msg_event_deal_flag()){
                    msleep(30);
                }

#ifdef CONFIG_VIDEO0_ENABLE
                dec_pro_file(NULL,0);
                __this->video_online[0] = 0;
                video_disp_stop(0);
#endif
                video_standby_post_msg("msg_save");
                printf("\n/**** msg_save:%s\n",now_msg_fname);
                cloud_playback_list_add(now_msg_fname);
                memset(now_msg_fname, 0, sizeof(now_msg_fname));
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_save finish\n ");

            }else if (!strcmp(it->data, "msg_exit")) {    //退出留言
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_exit \n");
#ifdef CONFIG_VIDEO0_ENABLE
                dec_pro_file(NULL,0);
                __this->video_online[0] = 0;
                video_disp_stop(0);
#endif
                video_standby_post_msg("msg_exit");
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_exit finish\n ");
            }else if (!strcmp(it->data, "new_unread_msg")) {    //有未读留言
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] new_unread_msg \n");

                video_standby_post_msg("new_unread_msg");
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] new_unread_msg finish\n ");
            }else if (!strcmp(it->data, "msg_fail")) {    //留言失败（5s内重复留言）
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_fail \n");
                if(!msg_finish_timer){
                    sys_timeout_del(msg_finish_timer);
                    msg_finish_timer = NULL;
                }
                rec_msg_dispose_mode = 6;
                video_standby_post_msg("msg_fail");
#ifdef CONFIG_VIDEO0_ENABLE
                dec_pro_file(NULL,0);
                __this->video_online[0] = 0;
                video_disp_stop(0);
#endif
                rec_msg_dispose_timer = sys_timeout_add(rec_msg_dispose_mode,rec_dispose_timer_handler,1000);
//                video_standby_post_msg("msg_fail");
                printf("\n/***[ACTION_VIDEO_REC_MSG_WORK] msg_fail finish\n ");
            }
            break;

#endif // CONFIG_IPC_UI_ENABLE
/******************************************************************/

        case ACTION_VIDEO_REC_CHANGE_STATUS:
            video_rec_change_status(it);
            break;
#ifdef CONFIG_UI_STYLE_JL02_ENABLE
        case ACTION_VIDEO_REC_CONTROL:
            if (__this->state == VIDREC_STA_START) {
                video_rec_stop(0);
                ve_mdet_reset();
                ve_lane_det_reset();
            } else {
                video_rec_start();
            }
            break;
        case ACTION_VIDEO_REC_LOCK_FILE:
            if (it->data && !strcmp(it->data, "get_lock_statu")) {
                it->exdata = !!__this->gsen_lock;
                break;
            }

            if (__this->state == VIDREC_STA_START) {
                __this->gsen_lock = it->exdata ? 0xff : 0;
            }
            break;
        case ACTION_VIDEO_REC_SWITCH_WIN:
            video_disp_win_switch(DISP_WIN_SW_SHOW_NEXT, 0);

            /* if (--g_fps <= 2) { */
            /* g_fps = 30; */
            /* } */
            /* video0_rec_set_dr(g_fps); */

            /* reverse = !reverse; */
            /* if (reverse) { */
            /* video_set_crop(100, 100, 1720, 880); */
            /* } else { */
            /* video_set_crop(0, 0, 1920, 1080); */
            /* } */

            /* video_rec_take_photo(); */

            /* video_disp_win_switch(DISP_WIN_SW_MIRROR, 0); */

            /* { */
            /* static u8 c = 0; */
            /* c++; */
            /* c = c < 4 ? c : 0; */
            /* int switch_usb_camera(int id); */
            /* switch_usb_camera(c); */
            /* } */

            /* { */
            /* video_disp_pause(0); */
            /* video_rec_take_photo(); */
            /* video_disp_start(0, &disp_window[DISP_MAIN_WIN][0]); */
            /* } */
            break;
#endif
        }
        break;
    case APP_STA_PAUSE:
        puts("--------app_rec: APP_STA_PAUSE\n");
        video_rec_fscan_release(0);
        video_rec_fscan_release(1);
        break;
    case APP_STA_RESUME:
        puts("--------app_rec: APP_STA_RESUME\n");
        break;
    case APP_STA_STOP:
        puts("--------app_rec: APP_STA_STOP\n");

#if (APP_CASE == __WIFI_CAR_CAMERA__)
        video_rec_all_stop_notify();
#endif
        if (__this->state == VIDREC_STA_START) {
            video_rec_stop(0);
            ve_mdet_stop();
            ve_lane_det_stop(0);
        }

        if (video_rec_uninit()) {
            err = 1;
            break;
        }

#ifdef CONFIG_UI_ENABLE
        puts("--------rec hide ui\n");
        hide_main_ui();

#endif
        break;
    case APP_STA_DESTROY:
        puts("--------app_rec: APP_STA_DESTROY\n");

#if (APP_CASE == __WIFI_CAR_CAMERA__)
        printf("\n video_rec_all_stop_notify \n");
        video_rec_all_stop_notify();
#endif
        if (video_rec_mode_sw()) {
            err = 2;
            break;
        }
#ifdef CONFIG_UI_ENABLE
        puts("--------rec close ui\n");
        if (__this->ui) {
            server_close(__this->ui);
            __this->ui = NULL;
        }
#endif
        f_free_cache(CONFIG_ROOT_PATH);

        printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
        post_msg_doorbell_task("audio_dec_task", 1, RT_AUDIO_CTRL_STOP);
        printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);

        dec_server_close();
        printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);

        post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_LOCAL_MSG_REC_STOP);

        printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
        doorbell_close_all_rt_stream();
            printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
        extern void clear_cloud_storage_list(void);
        clear_cloud_storage_list();
            printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
        malloc_stats();
        log_d("<<<<<<< video_rec: destroy\n");
        malloc_stats();
        break;
    }

    return err;
}

void start_cloud_video( int wakeup_status){

        u8 contact_num = 0;
        user_list_t *p;

        int avsdk_get_user_list(int32_t user_type, user_list_t *p);
        printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);

        int32_t user_type = get_user_type();
        user_list_t *p1 = get_user_list_t_info();
        avsdk_get_user_list(user_type,p1);
        contact_num = p1->users_count;
        printf("\n contact_num=================%d\n ",contact_num);

        RT_TALK_INFO  info = {0};
        if(find_device_video_play_info(&info) || find_audio_play_info()){

            return;
        }
        backup_wakeup_status =wakeup_status;// MOVT_WAKEUP; //
        void *user = calloc(1,sizeof(user_info_t));
        memcpy(user,&p1->users[0],sizeof(user_info_t));
        post_msg_doorbell_task("doorbell_block_event_task", 3, DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE, backup_wakeup_status,user);
}

/*
 *录像app的按键响应函数
 */
static int video_rec_key_event_handler(struct sys_event *event)
{
    int err;
    struct intent it;
    struct key_event *key = (struct key_event *)&event->u.key;

    if(get_video_call_state() != CALL_STA_IDLE){        //空闲时拨号
        printf("\n >>>>>>>>>>>>>>>>>>> discard key event\n");
        return true;//
    }
    printf("\n function: %s,%d, %d\n ",__FUNCTION__,key->event,key->value);
    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {


        case KEY_OK:
        case KEY_DOWN:
            printf("video_rec_key_ok: %d\n", __this->state);


            #if  0
                    extern void test_host_mute();
                    extern s32 usb_host_speaker_set_mute(u8 mute);

                    static  int flag=0;
                    usb_host_speaker_set_mute(flag++%2);
                    test_host_mute();

                      return true;
            #endif









            backup_wakeup_status = (int)event->arg;


#ifdef CONFIG_IPC_UI_ENABLE
            printf("\n\n >>>>>>backup_wakeup_status = %d\n\n",backup_wakeup_status);
            printf("\n\n >>>>>>get_video_call_state() = %d\n\n",get_video_call_state());

#ifdef LONG_POWER_IPC
            backup_wakeup_status = KEY_WAKEUP; // 强制按键拨号   (int)event->arg;
#endif

            printf("\n\n >>>>>>backup_wakeup_status = %d\n\n",backup_wakeup_status);
            if(get_video_call_state() == CALL_STA_IDLE){        //空闲时拨号
//              u8* get_contact_data();
//              u8 get_contact_num();
//              u8* data =  get_contact_data();
//              u8 contact_num = get_contact_num();
                u8 contact_num = 0;
                user_list_t *p;


                int avsdk_get_user_list(int32_t user_type, user_list_t *p);
                printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);

                int32_t user_type = get_user_type();
                user_list_t *p1 = get_user_list_t_info();
                avsdk_get_user_list(user_type,p1);
                contact_num = p1->users_count;
                printf("\n contact_num=================%d\n ",contact_num);


                if(contact_num<=0){

                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "bonud_err.adp");

                break;

                }


                if(contact_num > 1){
                    printf("/********* %s %d",__func__,__LINE__);
                    struct intent it;
                    init_intent(&it);
                    it.name	= "video_rec";
                   // it.action = ACTION_VIDEO_CALLING_GET_CONTACT;
                    it.action = ACTION_VIDEO_CALLING_SET_CONTACT;

                    int err = start_app(&it);
                    if (err) {
                        printf("rec STANDBY change err! %d\n", err);
                    }
                }else if(contact_num <= 1){
                    printf("/********* %s %d",__func__,__LINE__);
                    /***********拨号实际逻辑***********/
                    RT_TALK_INFO  info = {0};
                    if(find_device_video_play_info(&info) || find_audio_play_info()){

                        printf("\n\n\n  find_device_video_play_info(&info) = %d  find_audio_play_info() = %d \n\n\n",find_device_video_play_info(&info), find_audio_play_info());
                        notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"calling_fail",0);
                        break;
                    }



                    void *user = calloc(1,sizeof(user_info_t));
                    memcpy(user,&p1->users[0],sizeof(user_info_t));

                    post_msg_doorbell_task("doorbell_block_event_task", 3, DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE, backup_wakeup_status,user);

                    /************拨号UI逻辑**************/
                    struct intent it;
                    init_intent(&it);
                    it.name	= "video_rec";
                    it.action = ACTION_VIDEO_REC_CHANGE_WORK;
                    it.data = "calling";
                    int err = start_app(&it);
                    if (err) {
                        printf("ACTION_VIDEO_REC_CHANGE_WORK err! %d\n", err);
                    }
                    /*************************************/
                }else if(contact_num == 0){ //无联系人时操作

                    printf("/********* %s %d",__func__,__LINE__);
                }
            }
#else

            post_msg_doorbell_task("doorbell_block_event_task", 3, DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE, backup_wakeup_status,NULL);
#endif
            return true;
            break;

        case KEY_MENU:
            break;
        case KEY_MODE:
            puts("rec key mode\n");
#if 0
            if ((__this->state != VIDREC_STA_STOP) && (__this->state != VIDREC_STA_IDLE)) {
                if (__this->state == VIDREC_STA_START) {
                    if (!__this->gsen_lock) {
                        __this->gsen_lock = 0xff;
                        video_rec_post_msg("lockREC");
                    } else {
                        __this->gsen_lock = 0;
                        video_rec_post_msg("unlockREC");
                    }
                }

                return true;
            }
#else

            return false;
#endif
            break;
        case KEY_UP:
#ifdef CONFIG_IPC_UI_ENABLE


            if(get_video_rec_in_state()==REC_STA_IN_IDLE){
                init_intent(&it);
                it.name	= "video_rec";
                it.action = ACTION_VIDEO_REC_MSG_WORK;
                it.data = "msg_mode";
                err = start_app(&it);
            }else{
                printf("\n/*** video_rec_in_state != REC_STA_IN_IDLE\n ");
//                init_intent(&it);
//                it.name	= "video_rec";
//                it.action = ACTION_VIDEO_REC_MSG_WORK;
//                it.data = "rec:cal";
//                err = start_app(&it);
//                msg_flag = 0;
            }

#else

            video_disp_win_switch(DISP_WIN_SW_SHOW_NEXT, 0);
#endif // CONFIG_IPC_UI_ENABLE
            break;


#if  0
        case KEY_DOWN:


#ifdef CONFIG_IPC_UI_ENABLE

            //此处后续还需要添加区分主界面和留言界面的操作
//            printf("\n >>>>>>>>>>>>>>>> KEY_OK \n");
//            post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_LOCAL_MSG_REC_START);

            init_intent(&it);
            it.name	= "video_rec";
            it.action = ACTION_VIDEO_REC_SET_CONFIG;
            it.data = "rec:dec";
            err = start_app_async(&it, NULL, NULL);
#else
            mic_set_toggle();
            video_rec_aud_mute();
#endif

            return true;
#endif


            break;
        default:
            break;
        }
#ifdef LONG_POWER_IPC
        break ;
       case   KEY_EVENT_HOLD:

        static int count_net=0;
        switch (key->value) {
        #ifdef XD_CONFIG_BOARD_DEV_WIFI_5713_20231208
        case KEY_POWER:
        case KEY_OK:
		#else
		case KEY_NET:
		#endif

        printf("\n ENTER_RESET_CPU_MODE, %s,%d\n ",__FUNCTION__,__LINE__);
        post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_ENTER_RESET_CPU);

        break;
    default:
        break;
        }
#endif
       break;
    default:
        break;

    }

    return false;
}



void test_timer(void *priv)
{
    extern void net_video_handler_init(void);
    net_video_handler_init();
    msleep(20 * 1000);


#if 0
    //测试双码流配网功能，需要注意的是屏显用那一路，才可以用那一路的设备号
    void qr_net_cfg_init(void);
    qr_net_cfg_init();
    return;
#endif




#if 1
    while (1) {
//循环测试双路码流出帧

        int user_net_video3_recx_start(u8 uvc_id, u8 is_stream);
        user_net_video3_recx_start(0, 1);
        user_net_video3_recx_start(1, 1);
//        user_net_video3_recx_start(2,1);
//        user_net_video3_recx_start(3,1);
        msleep(10 * 1000);


#if CONFIG_SHARED_REC_HDL_ENABLE
        user_net_video3_recx_stop(0, 0);
#else
        user_net_video3_recx_stop(0, 1);
#endif
#if CONFIG_SHARED_REC_HDL_ENABLE
        user_net_video3_recx_stop(1, 0);
#else
        user_net_video3_recx_stop(1, 1);
#endif
//#if CONFIG_SHARED_REC_HDL_ENABLE
//       user_net_video3_recx_stop(2,0);
//#else
//       user_net_video3_recx_stop(2,1);
//#endif
//#if CONFIG_SHARED_REC_HDL_ENABLE
//       user_net_video3_recx_stop(3,0);
//#else
//       user_net_video3_recx_stop(3,1);
//#endif
        msleep(10 * 1000);

    }
#endif // 0



#if 0
    while (1) {
//循环测试本地写卡
        user_net_video3_recx_start(0, 0);
        user_net_video3_recx_start(1, 0);
        user_net_video3_recx_start(2, 0);
//        user_net_video3_recx_start(3,0);
        msleep(10 * 1000);
#if CONFIG_SHARED_REC_HDL_ENABLE
        user_net_video3_recx_stop(0, 0);
        user_net_video3_recx_stop(1, 0);
        user_net_video3_recx_stop(2, 0);
//        user_net_video3_recx_stop(3,0);
#else
        user_net_video3_recx_stop(0, 1);
        user_net_video3_recx_stop(1, 1);
        user_net_video3_recx_stop(2, 1);
//        user_net_video3_recx_stop(3,1);
#endif
        msleep(10 * 1000);
    }

#endif



#if 1
    static int count;
    while (1) {
//新开server拍照试验
        printf("\n\n >>>>>>>>>>>>>>>count = %d\n\n", count++);

        video3_norec_take_photo_save_file(0);
        video3_norec_take_photo_save_file(1);
        video3_norec_take_photo_save_file(2);
//        video3_norec_take_photo_save_file(3);
        msleep(10 * 1000);
    }

#endif



#if 0

    while (1) {
        //录像过程中抓拍试验
        //由于imc不足，最多只能同时启用2个测试
//        video3_recining_take_photo_save_file(0);
//        video3_recining_take_photo_save_file(1);
        video3_recining_take_photo_save_file(2);
        video3_recining_take_photo_save_file(3);
        msleep(5000);
    }

#endif


//获取uvcx对应接入的hub的port号
#if 0
    while (1) {
        u32 get_uvc_usb_port(u8 uvc_id);
        printf("\nuvc%d port = %d\n", 0, get_uvc_usb_port(0));
        printf("\nuvc%d port = %d\n", 1, get_uvc_usb_port(1));
        printf("\nuvc%d port = %d\n", 2, get_uvc_usb_port(2));
//        printf("\nuvc%d port = %d\n",3,get_uvc_usb_port(3));
        msleep(10 * 1000);
    }
#endif // 0
}

struct server *video3_display[CONFIG_VIDEO_REC_NUM];

/**
 * @brief       video3 指定uvc 显示
 *
 * @param: sub_id: uvc id号
 * @param: win:显示窗口配置
 *
 * @return:
 **/
int video3_disp_start(int sub_id, const struct video_window *win)
{
    int err = 0;
    union video_req req = {0};
    static char dev_name[20];
#ifdef CONFIG_DISPLAY_ENABLE

    u8 id = sub_id;

    if (win->width == (u16) - 1) {
        puts("video_disp_hide\n");
        return 0;
    }

    sprintf(dev_name, "video3.%d", sub_id * 5);
    if (sub_id >= 2) {
        sprintf(dev_name, "video3.%d", sub_id + 6);
    }
    log_d("video_disp_start: %s, %d x %d\n", dev_name, win->width, win->height);

    if (!video3_display[id]) {
        video3_display[id] = server_open("video_server", (void *)dev_name);
        if (!video3_display[id]) {
            log_e("open video_server: faild, id = %d\n", id);
            return -EFAULT;
        }
    }


    req.display.fb 		        = "fb1";
    req.display.left  	        = win->left;
    req.display.top 	        = win->top;
    req.display.width 	        = win->width;
    req.display.height 	        = win->height;
    req.display.border_left     = win->border_left;
    req.display.border_top      = win->border_top;
    req.display.border_right    = win->border_right;
    req.display.border_bottom   = win->border_bottom;
    req.display.mirror   		= win->mirror;
    req.display.jaggy			= 0;	// IMC 抗锯齿

    req.display.uvc_id = sub_id;
    req.display.camera_config = NULL;
    req.display.camera_type = VIDEO_CAMERA_UVC;
    req.display.src_w = __this->src_width[3];
    req.display.src_h = __this->src_height[3];
    //旋转参数配置:
    //0:不旋转,不镜像 (原图)
    //1:逆时针旋转90度,不镜像
    //2:逆时针旋转270度,不镜像
    //3:逆时针旋转90度后,再垂直镜像
    //4:逆时针旋转90度后,再水平镜像
    req.display.rotate = 0; //usb后视频图像旋转显示

    req.display.state 	        = VIDEO_STATE_START;
    req.display.pctl            = NULL;

    sys_key_event_disable();
    sys_touch_event_disable();
    err = server_request(video3_display[id], VIDEO_REQ_DISPLAY, &req);
    if (err) {
        printf("display req err = %d!!\n", err);
        server_close(video3_display[id]);
        video3_display[id] = NULL;
    }
#ifndef CONFIG_VIDEO4_ENABLE

    video_rec_start_isp_scenes();

#endif
#endif
    sys_key_event_enable();
    sys_touch_event_enable();

    return err;
}
void video3_disp_stop(int id)
{
#ifdef CONFIG_DISPLAY_ENABLE
    union video_req req = {0};

    if (video3_display[id]) {

        req.display.state 	= VIDEO_STATE_STOP;
        server_request(video3_display[id], VIDEO_REQ_DISPLAY, &req);

        server_close(video3_display[id]);
        video3_display[id] = NULL;

    }
#endif
}

void usb_speak_function(void *state){

#ifdef  ENABLE_CONTROL_USB_SEPAK_MUTE

   printf("\n *state=================%s\n ",state);
    struct sys_event e = {0};
    e.type = SYS_DEVICE_EVENT;
    e.arg = "onff_usb_speaker";
    if(!strcmp(state, "on")){
    e.u.dev.event = DEVICE_EVENT_IN;
    }else{

    e.u.dev.event = DEVICE_EVENT_OUT;
    }
    sys_event_notify(&e);

#endif


}

void usb_speak_switch(void *state){

    if(!strcmp(state, "on")){

    usb_speak_function("on");

    }else{

   // sys_hi_timeout_add(state, usb_speak_function, 1 * 1000);
     usb_speak_function("off");

    }


}

/*
 *录像app的设备响应函数
 */
static u8 usb_speak_init_flag=0;
static int video_rec_device_event_handler(struct sys_event *event)
{
    int err;
    struct intent it;


//    printf("\n event->arg=============================%s\n ",event->arg);

    if (!ASCII_StrCmp(event->arg, "sd*", 4)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            video_rec_sd_in();
            break;
        case DEVICE_EVENT_OUT:
            if (!fdir_exist(CONFIG_STORAGE_PATH)) {
                video_rec_sd_out();
            }

            void sd1_out_finish(void);
            sd1_out_finish();
            break;
        }
    } else if (!ASCII_StrCmp(event->arg, "sys_power", 7)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_POWER_CHARGER_IN:
            puts("\n\ncharger in\n\n");
            if ((__this->state == VIDREC_STA_IDLE) ||
                (__this->state == VIDREC_STA_STOP)) {
                if (__this->char_wait == 0) {
//                    __this->char_wait = wait_completion(storage_device_ready,
//                                                        video_rec_start, (void *)0);
                    /* video_rec_storage_device_ready, (void*)1); */
                    /* video_rec_start(); */
                }
            }
            break;
        case DEVICE_EVENT_POWER_CHARGER_OUT:
            puts("charger out\n");
            /*if (__this->state == VIDREC_STA_START) {
                video_rec_stop(0);
            }*/
            break;
        }
    } else if (!ASCII_StrCmp(event->arg, "parking", 7)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            puts("parking on\n");	//parking on

            hide_main_ui();
#ifdef CONFIG_UI_STYLE_JL02_ENABLE
            hide_home_main_ui();//录像在后台进入倒车隐藏主界面
#endif
            show_park_ui();
            video_disp_win_switch(DISP_WIN_SW_SHOW_PARKING, 0);
            sys_power_auto_shutdown_pause();

            return true;

        case DEVICE_EVENT_OUT://parking off
            hide_park_ui();
            show_main_ui();
            puts("parking off\n");
            video_disp_win_switch(DISP_WIN_SW_HIDE_PARKING, 0);
            video_rec_get_remain_time();
            if (__this->state == VIDREC_STA_START) {
                sys_power_auto_shutdown_pause();
            }
            return true;
        }
    }
#if (defined CONFIG_VIDEO1_ENABLE && !defined CONFIG_SINGLE_VIDEO_REC_ENABLE)
    else if (!strncmp(event->arg, "video1", 6)) {
#if (APP_CASE == __WIFI_CAR_CAMERA__)
        net_video_rec_event_notify();
#endif
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            if (!__this->video_online[1]) {
                __this->video_online[1] = true;

                video_disp_win_switch(DISP_WIN_SW_DEV_IN, 1);
                video_rec_get_remain_time();

                if (__this->state == VIDREC_STA_START) {
#if (APP_CASE == __WIFI_CAR_CAMERA__)
                    net_video_rec_event_stop();
#endif

                    video_rec_stop(0);
                    video_rec_start();

#if (APP_CASE == __WIFI_CAR_CAMERA__)
                    net_video_rec_event_start();
#endif

                }
            }
            break;
        case DEVICE_EVENT_OUT:
            if (__this->video_online[1]) {
                __this->video_online[1] = false;

                video_disp_win_switch(DISP_WIN_SW_DEV_OUT, 1);
                video_rec_get_remain_time();

                if (__this->state == VIDREC_STA_START) {
#if (APP_CASE == __WIFI_CAR_CAMERA__)
                    net_video_rec_event_stop();
#endif
                    video_rec_stop(0);
                    video_rec_start();
#if (APP_CASE == __WIFI_CAR_CAMERA__)
                    net_video_rec_event_start();
#endif

                }
            }
            break;
        }
    }
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    else if (!strncmp((char *)event->arg, "uvc", 3)) {
#if (APP_CASE == __WIFI_CAR_CAMERA__)
        net_video_rec_event_notify();
#endif

        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            puts("DEVICE_EVENT_ONLINE: uvc\n");
            printf("DEVICE_EVENT_ONLINE: uvc_id = %d\n", ((char *)event->arg)[3] - '0');
            printf("\n event->arg = %s __this->video_online[3] = %d \n", event->arg, __this->video_online[3]);
#ifndef CONFIG_UVCX_ENABLE
            if (!__this->video_online[3]) {
                if (!usb_is_charging()) {
                    break;
                }
                __this->video_online[3] = true;
                __this->uvc_id = ((char *)event->arg)[3] - '0';

                printf("DEVICE_EVENT_ONLINE: uvc_id = %d\n", __this->uvc_id);
                video_disp_win_switch(DISP_WIN_SW_DEV_IN, 3);
                video_rec_get_remain_time();

                if (__this->state == VIDREC_STA_START) {
#if (APP_CASE == __WIFI_CAR_CAMERA__)
                    net_video_rec_event_stop();
#endif

                    video_rec_stop(0);
                    video_rec_start();
#if (APP_CASE == __WIFI_CAR_CAMERA__)
                    net_video_rec_event_start();
#endif
                }


                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_REOPEN_RT_STREAM, 0);

                /* extern  int UVC_DETECT_TIME; */
                /* printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>UVC_DETECT_TIME = %d\n", timer_get_ms() - UVC_DETECT_TIME); */
            }
#else

            if (!strncmp((char *)event->arg, "uvc3", 4)) {
#if (CONFIG_DISP_DEVICE & CONFIG_UVC3_DEVICE)
                struct video_window win = {0};
                win.left   = LCD_DEV_WIDTH / 2;
                win.top    = 0;
                win.width  = LCD_DEV_WIDTH / 2;
                win.height = LCD_DEV_HIGHT;
                video3_disp_start(3, &win);
#endif
            }

            if (!strncmp((char *)event->arg, "uvc2", 4)) {
#if (CONFIG_DISP_DEVICE & CONFIG_UVC2_DEVICE)

                struct video_window win = {0};
                win.left   = LCD_DEV_WIDTH / 2;
                win.top    = 0;
                win.width  = LCD_DEV_WIDTH / 2;
                win.height = LCD_DEV_HIGHT;
                video3_disp_start(2, &win);
#endif
            }

            if (!strncmp((char *)event->arg, "uvc1", 4)) {
                printf("\n>>>>>>>>>>>>>>>>>>> %s %d\n", __func__, __LINE__);
                printf("DEVICE_EVENT_ONLINE: uvc_id = %d\n", ((char *)event->arg)[3] - '0');
                printf("\n>>>>>>>>>>>>>>>>>>> %s %d\n", __func__, __LINE__);
#if (CONFIG_DISP_DEVICE & CONFIG_UVC1_DEVICE)
                struct video_window win = {0};
                win.left   = LCD_DEV_WIDTH / 2;
                win.top    = 0;
                win.width  = LCD_DEV_WIDTH / 2;
                win.height = LCD_DEV_HIGHT;
                video3_disp_start(1, &win);
#endif

//                video_disp_win_switch(DISP_WIN_SW_DEV_IN, 3);
#if (CONFIG_NET_STREAM_DEVICE & CONFIG_UVC1_DEVICE)
                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_REOPEN_RT_STREAM, 1);
#endif


            }


            if (!__this->video_online[3]) {
                if (!usb_is_charging()) {
                    break;
                }
                __this->video_online[3] = true;
                __this->uvc_id = ((char *)event->arg)[3] - '0';

                printf("DEVICE_EVENT_ONLINE: uvc_id = %d\n", __this->uvc_id);


#if (CONFIG_DISP_DEVICE & CONFIG_UVC0_DEVICE)

                struct video_window win = {0};
                win.left   = 0;
                win.top    = 0;
                win.width  = 320;
                win.height = 240;
                video3_disp_start(0, &win);
#endif
                video_rec_get_remain_time();



#if (CONFIG_NET_STREAM_DEVICE & CONFIG_UVC0_DEVICE)
                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_REOPEN_RT_STREAM, 0);
#endif
                /* extern  int UVC_DETECT_TIME; */
                /* printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>UVC_DETECT_TIME = %d\n", timer_get_ms() - UVC_DETECT_TIME); */
            }

#ifdef CONFIG_UVCX_TEST_ENABLE
            static int flag;
            if (!flag) {
                flag = 1;
                thread_fork("test_timer", 17, 0x2E00, 256, 0, test_timer, NULL);
            }

#endif

#endif // CONFIG_UVCX_ENABLE

            break;
        case DEVICE_EVENT_OUT:
            puts("\n DEVICE_EVENT_OUT: uvc\n");
            printf("\n DEVICE_EVENT_OUT: uvc_id = %d\n", ((char *)event->arg)[3] - '0');
#ifndef CONFIG_UVCX_ENABLE
            if (__this->video_online[3]) {
                __this->video_online[3] = false;

                video_disp_win_switch(DISP_WIN_SW_DEV_OUT, 3);
                video_rec_get_remain_time();

                if (__this->state == VIDREC_STA_START) {
#if (APP_CASE == __WIFI_CAR_CAMERA__)
                    net_video_rec_event_stop();
#endif
                    video_rec_stop(0);
                    video_rec_start();
#if (APP_CASE == __WIFI_CAR_CAMERA__)
                    net_video_rec_event_start();
#endif

                }


                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_CLOSE_RT_STREAM, 0);

            }

#else
            printf("\n 2event->arg = %s __this->video_online[3] = %d \n", event->arg, __this->video_online[3]);
            if (!strncmp((char *)event->arg, "uvc3", 4)) {

#if (CONFIG_DISP_DEVICE & CONFIG_UVC3_DEVICE)
                video3_disp_stop(3);
#endif
            }

            if (!strncmp((char *)event->arg, "uvc2", 4)) {

#if (CONFIG_DISP_DEVICE & CONFIG_UVC2_DEVICE)
                video3_disp_stop(2);
#endif
            }

            if (!strncmp((char *)event->arg, "uvc1", 4)) {
                printf("\n>>>>>>>>>>>>>>>>>>> %s %d\n", __func__, __LINE__);
                printf("DEVICE_EVENT_OUT: uvc_id = %d\n", ((char *)event->arg)[3] - '0');
                printf("\n>>>>>>>>>>>>>>>>>>> %s %d\n", __func__, __LINE__);
#if (CONFIG_DISP_DEVICE & CONFIG_UVC1_DEVICE)
                video3_disp_stop(1);
#endif

#if (CONFIG_NET_STREAM_DEVICE & CONFIG_UVC1_DEVICE)
                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_CLOSE_RT_STREAM, 1);
#endif

            }

            if (__this->video_online[3] && !strncmp((char *)event->arg, "uvc0", 4)) {
                __this->video_online[3] = false;
#if (CONFIG_DISP_DEVICE & CONFIG_UVC0_DEVICE)
                video3_disp_stop(0);
#endif

#if (CONFIG_NET_STREAM_DEVICE & CONFIG_UVC0_DEVICE)
                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_CLOSE_RT_STREAM, 0);
#endif
            }

#endif // CONFIG_UVCX_ENABLE

            break;
        }
    }
#endif



#ifdef CONFIG_GSENSOR_ENABLE
    else if (!strcmp(event->arg, "gsen_lock")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_CHANGE:
            if (__this->state == VIDREC_STA_START) {
                if (db_select("gra")) {
                    __this->gsen_lock = 0xff;
                    video_rec_post_msg("lockREC");
                }
            }
            break;
        }
    }
#endif

    else if (!strncmp(event->arg, "lane_set_open", strlen("lane_set_open"))) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_CHANGE: {
            u32 aToint;
            ASCII_StrToInt(event->arg + strlen("lane_set_open"), &aToint, strlen(event->arg) - strlen("lane_set_open"));
            __this->car_head_y = aToint & 0x0000ffff;
            __this->vanish_y   = (aToint >> 16) & 0x0000ffff;
            ve_lane_det_start(1);
        }
        break;
        }
    } else if (!strncmp(event->arg, "lane_set_close", strlen("lane_set_close"))) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_CHANGE:
            ve_lane_det_stop(1);
            break;
        }
    } else if (!strcmp(event->arg, "camera0_err")) {
        video_disp_win_switch(DISP_WIN_SW_DEV_OUT, 0);

        if (__this->state == VIDREC_STA_START) {
            video_rec_stop(0);
            ve_mdet_reset();
            ve_lane_det_reset();
            video_rec_start();
        }
    } else if (!strncmp((char *)event->arg, "rec", 3)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_OFFLINE:
            if (__this->state == VIDREC_STA_START) {
                log_e("video rec write error");
                video_rec_stop(0);
            }
            break;
        }
    }
#ifdef CONFIG_USB_UVC_AND_UAC_ENABLE
    else if (!strcmp((char *)event->arg, "usb mic")) {
        extern int play_usb_mic_start();
        extern int play_usb_mic_stop();
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            play_usb_mic_start();
            usb_speak_init_flag=1;
            break;
        case DEVICE_EVENT_OUT:
            play_usb_mic_stop();
            usb_speak_init_flag=0;
            break;
        }
    } else if (!strcmp((char *)event->arg, "usb speaker")) {
        if(!usb_speak_init_flag)
        {

         return false;
        }
        extern int play_usb_speaker_start();
        extern int play_usb_speaker_stop();
        switch (event->u.dev.event) {

        case DEVICE_EVENT_IN:
           play_usb_speaker_start();
            break;
        case DEVICE_EVENT_OUT:
           play_usb_speaker_stop();
            break;
        }
    }else if (!strcmp((char *)event->arg, "onff_usb_speaker")) {
        extern int play_usb_speaker_start();
        extern int play_usb_speaker_stop();

        printf("onff_usb_speaker in_or_out :%d,%s\n",event->u.dev.event, __FUNCTION__);
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:

           play_usb_speaker_start();
            break;
        case DEVICE_EVENT_OUT:
           play_usb_speaker_stop();

           sys_key_event_enable();
            break;
        }
    }
#endif
    else if (!strncmp((char *)event->arg, "cdc", 3)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:

          #if  1
            printf("\n >>>>>>>>>>>> event->arg = %s \n",event->arg);
            if(cdc_host_init()){
                printf("\n >>>>>>>>>>>> event->arg = %s  err \n",event->arg);
            }
          #endif
            break;
        case DEVICE_EVENT_OUT:
             printf("\n >>>>>>>>>>>> event->arg = %s \n",event->arg);
             cdc_host_uninit();
            break;
        }
    }

    return false;
}
static int video_rec_net_event_handler(struct sys_event *event)
{
    struct key_event *net = (struct net_event *)&event->u.net;


    switch(net->event){
        case NET_EVENT_CFG:
            puts("\n>>>>>>>>> NET_EVENT_CFG\n");
            void get_yuv_uninit(void);
            get_yuv_uninit();
            notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"network_connect_start",0);
            if(!get_cpu_reset_state()){//清除设备 不理解播报
            post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetCfgEnter.adp");
            }
            extern void qr_net_cfg_init(void);
            qr_net_cfg_init();

            return true;
        case NET_EVENT_CONNECTING:
            puts("\n>>>>>>>>> NET_EVENT_CONNECTING\n");
            //显示正在连接界面
            notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"network_connecting",0);
            return true;
        case NET_EVENT_CONNECTED:
            puts("\n>>>>>>>>> NET_EVENT_CONNECTED\n");
            notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"connect_succ",0);
//
//           extern  int cloud_platform_init(void);
//                        cloud_platform_init();




             return true;

        case NET_EVENT_GET_IP_INFO:
#ifdef CONFIG_WIFI_HI3861L_ENABLE
             avsdk_cmd_get_ip();
#endif // CONFIG_WIFI_HI3861L_ENABLE

             return true;
        case NET_EVENT_DISCONNECTED:
            puts("\n>>>>>>>>> NET_EVENT_DISCONNECTED\n");
            post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetDisc.adp");
             return true;
        case NET_EVENT_CONNECTFAIL: //断网走这一条
            puts("\n>>>>>>>>> NET_EVENT_CONNECTFAIL\n");
            printf("\n last video_net_state = %d\n",video_net_state);
            if(video_net_state == NET_STA_CONNECTING){
                notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"connect_fail",0);
            }else if(video_net_state == NET_STA_CONNECT_SUCC){
                notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"calling_net_connectfail",0);
            }


            post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetCfgFail.adp");
            return true;
        case NET_EVENT_AP_MODE_CFG:
            char *str = NULL;
            char did[32] = {""};
            char ssid[33], pwd[64];
            extern int doorbell_read_cfg_did(char *buf, int buf_len);
            doorbell_read_cfg_did(did, sizeof(did));
            if (strlen(did)) {
                str = did + strlen(did);
                str -= 6;
                snprintf(ssid, sizeof(ssid), "LLM_%s", str);
                printf("\n ssid = %s\n", ssid);
#ifdef CONFIG_WIFI_HI3861L_ENABLE
                avsdk_cmd_ap_net_cionfig(ssid, "12345678");
#endif
            }
            return true;
        case NET_EVENT_AP_OPEN_SUC:
#ifdef CONFIG_WIFI_HI3861L_ENABLE
            avsdk_cmd_get_ip();
#endif
            return true;
        case NET_EVENT_PASSWORD_ERR:
                    printf("\n >>>>>>>>>>>>>>>>>>%s %d \n",__func__,__LINE__);
            notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"connect_fail",0);
            post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetCfgFail.adp");

            void get_yuv_uninit(void);
            get_yuv_uninit();
            video_disp_stop(0);

            msleep(1000);
            notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"network_connect_start",0);

            extern void qr_net_cfg_init(void);
            qr_net_cfg_init();
        printf("\n >>>>>>>>>>>>>>>>>>%s %d \n",__func__,__LINE__);
            return true;
        default:
            break;
    }
            printf("\n >>>>>>>>>>>>>>>>>>%s %d \n",__func__,__LINE__);
    return false;
}
/*录像app的事件总入口*/
static int video_rec_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
      //  printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>1122\n");
        return video_rec_key_event_handler(event);
    case SYS_DEVICE_EVENT:
        return video_rec_device_event_handler(event);
    case SYS_NET_EVENT:
        return video_rec_net_event_handler(event);
    default:
        return false;
    }
}

static const struct application_operation video_rec_ops = {
    .state_machine  = video_rec_state_machine,
    .event_handler 	= video_rec_event_handler,
};

REGISTER_APPLICATION(app_video_rec) = {
    .name 	= "video_rec",
    .action	= ACTION_VIDEO_REC_MAIN,
    .ops 	= &video_rec_ops,
    .state  = APP_STA_DESTROY,
};




#if (APP_CASE == __WIFI_CAR_CAMERA__)
/******************************用于网络实时流*************************************/
static void ve_mdet_reset();
static int video_rec_sd_in();
static int video_rec_sd_out();
static int video_rec_device_event_handler(struct sys_event *event);
void ve_lane_det_reset();
extern char *video_rec_finish_get_name(FILE *fd, int index, u8 is_emf); //index：video0则0，video1则1，video2则2
extern int video_rec_finish_notify(char *path);
extern int video_rec_delect_notify(FILE *fd, int id);
extern int video_rec_err_notify(const char *method);
extern int video_rec_state_notify(void);
extern int video_rec_all_stop_notify(void);
extern int net_video_rec_event_notify(void);
extern int net_video_rec_event_stop(void);
extern int net_video_rec_event_start(void);

static int video_rec_get_abr(u32 width);
static void video_disp_stop(int id);
static int video_disp_start(int id, const struct video_window *win);
static int show_main_ui();
static void hide_main_ui();




int video_rec_get_audio_sampel_rate(void)
{
#ifdef  VIDEO_REC_AUDIO_SAMPLE_RATE
    return VIDEO_REC_AUDIO_SAMPLE_RATE;
#else
    return 8000;
#endif
}
int video_rec_control_start(void)
{
    int err;
    err = video_rec_start();
    return err;
}

int video_rec_control_doing(void)
{
    int err;
    if (__this->state == VIDREC_STA_START) {
        err = video_rec_stop(0);
        ve_mdet_reset();
        ve_lane_det_reset();
    } else {
        err = video_rec_start();
    }
    return err;
}

int video_rec_device_event_action(struct sys_event *event)
{
    return video_rec_device_event_handler(event);
}

int video_rec_sd_in_notify(void)
{
    return video_rec_sd_in();
}

int video_rec_sd_out_notify(void)
{
    return video_rec_sd_out();
}

int video_rec_get_abr_from(u32 width)
{
    return video_rec_get_abr(width);
}


int net_video_disp_start(int id)
{
    video_disp_start(id, &disp_window[0][0]);
    return 0;
}

int net_video_disp_stop(int id)
{
    video_disp_stop(id);
    return 0;
}



/******************************************************************/
#endif
int net_hide_main_ui(void)
{
    hide_main_ui();
    return 0;
}
int net_show_main_ui(void)
{
    show_main_ui();
    return 0;
}
int doorbell_video_disp_start(int id,const struct video_window *win)
{
    video_disp_start(id, win);
    return 0;
}

int doorbell_video_disp_stop(int id)
{
      printf("\n %s %d\n", __func__, __LINE__);
    video_disp_stop(id);
      printf("\n %s %d\n", __func__, __LINE__);
    return 0;
}

int video_rec_get_fps()
{
#ifdef LOCAL_VIDEO_REC_FPS
    return LOCAL_VIDEO_REC_FPS;
#else
    return 0;
#endif
}

void *get_video_rec_handler(void)
{
    return (void *)&rec_handler;
}

int video_rec_control_start(void)
{
    int err;
    err = video_rec_start();
    return err;
}

int video_rec_control_stop(void)
{
    int err;
    err = video_rec_stop(0);
    return err;
}

u8 is_video_rec_mode(void)
{
    struct intent it;
    struct application *app;
    app = get_current_app();
    if (app && !strcmp(app->name, "video_rec")) {
        return 1;
    }
    return 0;
}

u8 is_usb_mode(void)
{
    struct intent it;
    struct application *app;
    app = get_current_app();
    if (app && !strcmp(app->name, "usb_app")) {
        return 1;
    }
    return 0;
}

u8 is_video_dec_mode(void)
{
    struct intent it;
    struct application *app;
    app = get_current_app();
    if (app && !strcmp(app->name, "video_dec")) {
        return 1;
    }
    return 0;
}



void video_rec_set_osd(struct video_text_osd *text_osd, int width, int height)
{
    char *__osd_str_total = NULL;
    char *__osd_str_matrix = NULL;
    int osd_str_matrix_len;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;

    if (width >= 1440) {
        text_osd->font_w = 32;
        text_osd->font_h = 64;
        __osd_str_total = osd_2bit_32x64_str_total;
        __osd_str_matrix = osd_2bit_32x64_str_matrix;
        osd_str_matrix_len = sizeof(osd_2bit_32x64_str_matrix);
    } else if (width >= 1280) {
        text_osd->font_w = 16;
        text_osd->font_h = 32;
        __osd_str_total = osd_2bit_16x32_str_total;
        __osd_str_matrix = osd_2bit_16x32_str_matrix;
        osd_str_matrix_len = sizeof(osd_2bit_16x32_str_matrix);
    } else {
        text_osd->font_w = 16;
        text_osd->font_h = 32;
        __osd_str_total = osd_2bit_16x32_str_total;
        __osd_str_matrix = osd_2bit_16x32_str_matrix;
        osd_str_matrix_len = sizeof(osd_2bit_16x32_str_matrix);

    }
    /*
    ** osd 相关的参数，注意坐标位置，x要64对齐，y要16对齐,底下例子是根据图像大小偏移到右下
    */

    printf("video_rec_osd_buf = %s len = %d", video_rec_osd_buf, strlen(video_rec_osd_buf)); //需要满足 \yyyy-nn-dd/hh:mm:ss

    max_one_line_strnum = strlen(video_rec_osd_buf);//21;


    osd_line_num = 1;
    /*   if (db_select("num")) { */
    /* osd_line_num = 2; */
    /* } */
    osd_max_heigh = (height == 1088) ? 1080 : height ;
    text_osd->x = (width - max_one_line_strnum * text_osd->font_w) / 64 * 64;
    text_osd->y = (osd_max_heigh - text_osd->font_h * osd_line_num) / 16 * 16;
//     text_osd.color[0] = 0xe20095;
//     text_osd.bit_mode = 1;
    text_osd->color[0] = 0xe20095; //黑色
    text_osd->color[1] = 0xe20095;
    text_osd->color[2] = 0xe20095;
    text_osd->bit_mode = 2;
    text_osd->text_format = video_rec_osd_buf;
    text_osd->font_matrix_table = __osd_str_total;
    /*     text_osd.font_matrix_base = osd_str_matrix; */
    /* text_osd.font_matrix_len = sizeof(osd_str_matrix); */
    text_osd->font_matrix_base = __osd_str_matrix;
    text_osd->font_matrix_len = osd_str_matrix_len;


}
/* uvc主机发命令(PU)给从机demo示例 */
void video3_stream_cmd_test(void)
{
    enum {
        ID_USB_CAM0 = 0,
        ID_USB_CAM1 = 1,
    };
    enum {
        CMD_NONE = 0,
        CMD_FORCE_IFRAME,
        CMD_SET_FPS,
        CMD_SET_KBPS,
    };
    struct usb_video_cmd_t {
        u8 id;
        u8 cmd;
        u16 data;
    };

    int err = 0 ;
    struct usb_video_cmd_t usb_video_cmd;
    struct uvc_processing_unit uvc_pu;
    uvc_pu.index = 0x0200;
    uvc_pu.request = 0x01;
    uvc_pu.type = 0x21;
    uvc_pu.value = 0x0100;
    uvc_pu.len = 4;

    if (!dev_online("uvc")) {
        log_e("uvc offline!\n");
        return ;
    }
    void *fd = dev_open("uvc", (void *)0);
    if (!fd) {
        puts("fd uvc open faild!\n");
        return ;
    }

    //set fps
    usb_video_cmd.id = ID_USB_CAM1;
    usb_video_cmd.cmd = CMD_SET_FPS;
    u16 fps = 15;
    usb_video_cmd.data = fps;
    memcpy(uvc_pu.buf, &usb_video_cmd, sizeof(struct usb_video_cmd_t));

    err = dev_ioctl(fd, UVCIOC_REQ_PROCESSING_UNIT, (unsigned int)&uvc_pu);
    printf("err=%d\n", err);

    os_time_dly(20);

    //set kbps
    usb_video_cmd.id = ID_USB_CAM1;
    usb_video_cmd.cmd = CMD_SET_KBPS;
    u16 kbps = 2000;
    usb_video_cmd.data = kbps;
    memcpy(uvc_pu.buf, &usb_video_cmd, sizeof(struct usb_video_cmd_t));

    err = dev_ioctl(fd, UVCIOC_REQ_PROCESSING_UNIT, (unsigned int)&uvc_pu);
    printf("err=%d\n", err);

    dev_close(fd); //记得关闭uvc设备句柄
}


int get_video3_online_flag(void)
{
    return  __this->video_online[3];
}

void *get_video0_rec_hdl(void)
{
    if (!__this->video_rec0) {
        __this->video_rec0 = server_open("video_server", "video0.0");
        /* __this->video_rec0 = server_open("video_server", "video4.0"); */

        server_register_event_handler(__this->video_rec0, (void *)0, rec_dev_server_event_handler);
    }
    return  __this->video_rec0;
}

#if  0

// ... 初始化硬件接口和文件系统库 ...

void test_write_speed(char* filename, size_t filesize) {
    // 初始化文件写入操作
    uint8_t *write_buffer = NULL;

   // char filename[] = "testfile.bin";
    uint32_t write_time, read_time;
    size_t total_bytes = 10 * 1024 * 1024; /* 10MB */
    size_t block_size = 4096; /* 4KB */
    size_t blocks = total_bytes / block_size;
    size_t bw, br;
    char file[128]={0};
    int res;
    FILE *fd=NULL;

    // 写入数据并记录时间
    /* 分配缓冲区 */
    write_buffer = (uint8_t *)malloc(block_size);
     if (!write_buffer) {
        printf("Failed to allocate buffers.\n");
        return ;
    }
   /*创建或覆盖文件用于测试 */

    snprintf(file,"%s%s",CONFIG_ROOT_PATH,filename);
    fd = fopen(file, "w+");
    if (!fd) {

        free(write_buffer);

        return ;
    }
    /* 初始化缓冲区（为简化，这里只是用固定值填充） */
    for (size_t i = 0; i < block_size; i++) {
        write_buffer[i] = i % 256;
    }
    time_t start_time = timer_get_ms(); // 获取当前时间
    for (size_t i = 0; i < blocks; i++) {
        // 写入数据到文件
        res = fwrite(fd, write_buffer, block_size);
        if (res != 1 ) {
            printf("Write error");
            fclose(fd);
            free(write_buffer);

            return ;
    }
    }
    time_t end_time = timer_get_ms(); // 获取结束时间

    // 计算并打印写入速度
    double write_speed = total_bytes / (end_time - start_time); // 假设time_t是以秒为单位的
    printf("Write speed: %.2f KB/s\n", write_speed / 1024);




}

void test_read_speed(char* filename, size_t filesize) {
    // 初始化文件读取操作
    uint8_t *read_buffer = NULL;
   // char filename[] = "testfile.bin";
    uint32_t write_time, read_time;
    size_t total_bytes = 10 * 1024 * 1024; /* 10MB */
    size_t block_size = 4096; /* 4KB */
    size_t blocks = total_bytes / block_size;
    size_t bw, br;
    int res;
    FILE *fd=NULL;
    char file[128]={0};
    // 读取数据并记录时间

       // 写入数据并记录时间
    /* 分配缓冲区 */
    read_buffer = (uint8_t *)malloc(block_size);
     if (!read_buffer) {
        printf("Failed to allocate buffers.\n");
        return ;
    }
    /*创建或覆盖文件用于测试 */
   // fd = fopen(CONFIG_ROOT_PATH, "r+");
    snprintf(file,"%s%s",CONFIG_ROOT_PATH,filename);
    fd = fopen(file, "r+");
    if (!fd) {

        free(read_buffer);

        return ;
    }
    time_t start_time = get_current_time(); // 获取当前时间
    while (block_size < total_bytes) {
        res = fread(fd, read_buffer, sizeof(read_buffer));
        block_size += res;
        block_size>total_bytes:
        if (!res) {
            /* 处理错误 */
            fclose(fd);

            free(read_buffer);
            return ;
        }
        // 这里可以添加数据验证逻辑，检查read_buffer的内容是否正确
    }
    time_t end_time = get_current_time(); // 获取结束时间

    // 计算并打印读取速度
    double read_speed = filesize / (end_time - start_time); // 假设time_t是以秒为单位的
    printf("Read speed: %.2f KB/s\n", read_speed / 1024);

}

int enter_test_tf() {
    // 初始化硬件接口和文件系统库
    // ...

    // 进行读写速度测试

    while(!storage_device_available()){

        printf("\n no sd\n");
    }
    test_write_speed("testfile.bin", 1024 * 1024); // 写入1MB数据
    test_read_speed("testfile.bin", 1024 * 1024); // 读取1MB数据

    // 清理并退出
    // ...
    return 0;
}

#endif

/*********************** 以下为可视对讲项目新增内容***************************/
#ifdef  CONFIG_IPC_UI_ENABLE
static u8 page_standby_flag = 0;
static u8 page_dist_net_flag = 0;

void video_standby_post_msg(const char *msg, ...)
{
    #ifdef CONFIG_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_MAIN_AND_REC;
        req.msg.msg = msg;
        req.msg.exdata = argptr;

        server_request(__this->ui, UI_REQ_MSG, &req);
    }
    else{
        printf("\n/****** post err\n");
    }

    va_end(argptr);

    if(msg){
     //   printf("\n/**** [%s] %s\n",__func__,msg);
//        while(1){
//            int i=0;
//
//            if (!strcmp(msg_to_page_id[i].msg, "NULL")){
//                if (!strcmp(msg, msg_to_page_id[i].msg){
//
//                }
//            }
//            else{
//                break;
//            }
//
//        }
    }
    else{
        printf("/**** post null\n");
    }


    #endif


}

void video_dist_net_post_msg(const char *msg, ...)
{
    if(msg){
        printf("/**** [%s] %s\n",__func__,msg);
    }
    else{
        printf("/**** post null\n");
    }
        #ifdef CONFIG_UI_ENABLE
        union uireq req;
        va_list argptr;

        va_start(argptr, msg);

        if (__this->ui) {
            req.msg.receiver = ID_WINDOW_DIST_NET;
            req.msg.msg = msg;
            req.msg.exdata = argptr;

            server_request(__this->ui, UI_REQ_MSG, &req);
        }
        else{
            printf("/****** post err\n");
        }

        va_end(argptr);

    #endif


}

static void update_standby_ui_info()
{
    video_standby_post_msg("updateTIME");
    video_standby_post_msg("updateWEATHER");
    standby_update_timer = sys_timer_add("updateTIME",video_standby_post_msg,1000);
    standby_update_weather = sys_timer_add("updateWEATHER",video_standby_post_msg,30*60*1000);
}

static int show_standby_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;
      printf("\n %s %d\n", __func__, __LINE__);
    if (page_standby_flag) {
        return 0;
    }
          printf("\n %s %d\n", __func__, __LINE__);
    if (!__this->ui) {
        return -1;
    }
      printf("\n %s %d\n", __func__, __LINE__);
    puts("show_standby_ui\n");
    ui_page_now_id = LAYER_STANDBY;
    req.show.id = ID_WINDOW_MAIN_AND_REC;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
    page_standby_flag = 1;
    if(!standby_update_timer){
//        video_standby_post_msg("updateTIME");
//        wait_completion(get_avsdk_connect_flag,video_standby_post_msg,"updateTIME");
//        wait_completion(get_avsdk_connect_flag,video_standby_post_msg,"updateWEATHER");
//        standby_update_timer = sys_timer_add("updateTIME",video_standby_post_msg,1000);
//        standby_update_weather = sys_timer_add("updateWEATHER",video_standby_post_msg,30*60*1000);
        wait_completion(get_avsdk_connect_flag,update_standby_ui_info,NULL);
        printf("/**** standby_update_timer updateTIME");
    }

#endif

    return 0;
}

//static void hide_standby_ui()
 void hide_standby_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (page_standby_flag == 0) {
        return;
    }
    if (!__this->ui) {
        puts("__this->ui == NULL!!!!\n");
        return;
    }

    puts("hide_main_ui\n");

    req.hide.id = ID_WINDOW_MAIN_AND_REC;
    server_request(__this->ui, UI_REQ_HIDE, &req);
    page_standby_flag = 0;

#endif
}

//static int show_upgrade_ui()
 int show_upgrade_ui()
{
#ifdef CONFIG_UI_ENABLE

    union uireq req;

     if (!__this->ui) {
        return -1;
    }

    puts("show_upgrade_ui\n");
    req.show.id = ID_WINDOW_UPDRADE;
   // server_request_async(ui, UI_REQ_SHOW, &req);
     server_request(__this->ui, UI_REQ_HIDE, &req);
#endif

    return 0;
}


void return_ui_to_main()
{
    video_standby_post_msg("return_ui_to_main");

//    hide_dist_net_ui();
//    show_main_ui();
}

void return_ui_to_connect()
{
    video_standby_post_msg("return_ui_to_connect");
}

static void change_app_to_dec()
{
    struct intent it;
    struct application *app;
    init_intent(&it);
    app = get_current_app();
    if (app) {



        it.action = ACTION_BACK;
        start_app(&it);


        it.name = "video_dec";
        it.action = ACTION_VIDEO_DEC_MAIN;
        start_app(&it);
    }
}

void return_app_to_rec()
{
    printf("\n/********* return_app_to_dec\n");
    struct intent it;
    struct application *app;
    init_intent(&it);
    app = get_current_app();
    if (app) {
        it.action = ACTION_BACK;
        start_app(&it);

        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_MAIN;
        start_app(&it);
    }
}

void calling_timeout()
{
    printf("\n/***** post_msg_doorbell_task calling_timeout\n");
    post_msg_doorbell_task("video_dec_task", 1, APP_ANSWER_TIMEOUT_STATE);
}

#ifdef CONFIG_IPC_UI_ENABLE

void rec_dispose_timer_handler(u8 mode)
{
    if(rec_call_dispose_timer){
        sys_timeout_del(rec_call_dispose_timer);
        rec_call_dispose_timer = NULL;
    }
    union uireq req;
 //   printf("\n[%s] %d,%d\n",__func__,mode,__LINE__);

    req.hide.id = LAYER_CALLING_EVENT;      //隐藏事件提示ui界面
    server_request(__this->ui, UI_REQ_HIDE, &req);
    ui_page_now_id = LAYER_STANDBY;
    switch(mode){               //预留用于想做不同处理时，在对应处添加
    case 1:     //拨号对方挂断
        dec_pro_file(NULL,0);
        req.show.id = LAYER_STANDBY;
        server_request(__this->ui, UI_REQ_SHOW, &req);
        break;
    case 2:     //拨号对方拒接
        req.show.id = LAYER_STANDBY;
        server_request(__this->ui, UI_REQ_SHOW, &req);
        break;
    case 3:     //拨号超时
        req.show.id = LAYER_STANDBY;
        server_request(__this->ui, UI_REQ_SHOW, &req);
        break;
    case 4:     //拨号断网
        req.show.id = LAYER_STANDBY;
        server_request(__this->ui, UI_REQ_SHOW, &req);
        break;
    case 5:     //拨号失败
        req.show.id = LAYER_STANDBY;
        server_request(__this->ui, UI_REQ_SHOW, &req);
        break;
    case 6:     //留言失败
        req.show.id = LAYER_STANDBY;
        server_request(__this->ui, UI_REQ_SHOW, &req);
        break;
    }


}

#endif // CONFIG_IPC_UI_ENABLE
void hide_ui_busy_timer_handler()       //定时隐藏忙碌提示
{
    if(hide_ui_busy_timer){
        sys_timeout_del(hide_ui_busy_timer);
        hide_ui_busy_timer = NULL;
    }
    union uireq req;
    printf("\n [%s] %d\n ",__func__,__LINE__);
    video_standby_post_msg("app_audio_talk_hide_busy");
}

void msg_finish_timer_handler(u8 mode)       //1强制执行并清除该定时器
{
    union uireq req;
    printf("\n [%s] %d\n ",__func__,__LINE__);
    if((msging_countdown++ >= MSG_TIME) || mode){
        if(msg_finish_timer){
            sys_timer_del(msg_finish_timer);
            msg_finish_timer = NULL;
        }
        video_standby_post_msg("msg_finish");
        post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_LOCAL_MSG_REC_STOP);
    }else{
        video_standby_post_msg("msg_update_time");
    }

}


#endif // CONFIG_IPC_UI_ENABLE

