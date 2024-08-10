
/*****************************************************************************
 * ut_ipc_cmd.c
 *
 ****************************************************************************/

/*****************************************************************************
 * Included Files
 ****************************************************************************/
#define _STDIO_H_
#include "app_config.h"
#include "fs.h"
#include "app_core.h"
#include "system/database.h"
#include "hi3861l_task.h"
#include "os/os_compat.h"
#include "doorbell_event.h"
#include "action.h"

#include "xciot_api.h"
#include <stdbool.h>

#if ENABLE_DEMO_AV > 1
#include "example_ctx.h"
#endif

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "asm/isp_dev.h"
#include "camera.h"

/*****************************************************************************
 * Trace Definitions
 ****************************************************************************/

#define EVOS_LOG_COMPILE_LEVEL EVOS_LOG_LEVEL_DEBUG
#define EVOS_LOG_RUN_LEVEL     EVOS_LOG_LEVEL_DEBUG
#define EVOS_LOG_DOMAIN        "ut_dev_ipc_cmd.c"
#include "xc_log.h"



#ifdef ENABLE_OSAL
#include "osal/osal_api.h"
#endif

/*****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define OSAL_POINTER_TO_UINT(x) ((uintptr_t)(x))
#define OSAL_UINT_TO_POINTER(x) ((void *)(uintptr_t)(x))
#define OSAL_POINTER_TO_INT(x) ((intptr_t)(x))
#define OSAL_INT_TO_POINTER(x) ((void *)(intptr_t)(x))

/*****************************************************************************
 * Private Types
 ****************************************************************************/
#define USE_CONTINUOUS_VIDEO 0
/*****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/*****************************************************************************
 * Private Data
 ****************************************************************************/

/*****************************************************************************
 * Public Data
 ****************************************************************************/
#if 0
extern pthread_ctx_t thr_hv_ctx;
extern char g_history_h264_file[2048];
extern void *video_history_thread(void *);
#endif


extern void set_utc_ms(uint64_t utc_ms);
/*****************************************************************************
 * Private Functions
 ****************************************************************************/
#include "iotsdk_xspp.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"


extern u8 find_device_video_play_info(RT_TALK_INFO  *tmp);
static int32_t user_type = 0;
static user_list_t p;
extern  void dev_write_plane_time_vm(uint8_t *data, size_t len);
int get_user_type()
{
    return user_type;
}
void *get_user_list_t_info(void)
{

    return &p;
}




#if 1

#if 0
struct test {
    char wifi_ssid[50];
    char wifi_key[50];
    int32_t video_qos;
    int32_t video_speed;
    int32_t flip;  //翻转信息: 1: Upright, 2: Flip Horizontal，3 :Flip Vertical，4: turn 180
    uint32_t volume_value;

    int32_t  history_plan_record_type;           // 录像类型: 1, 连续录像; 2, 事件录像
    xciot_Timetask  history_plan_tt[3];             // 时间段(最多3个)
    int32_t   history_plan_plan_count;               // 个数
    int32_t   history_plan_enable;                    //  是否开启录像；0：disable;1 enable

    int32_t  led_mode;            //  LED状态: 1 常开；2 常关；
    int32_t  ircut_mode;          // IRCut状态: 1 常开；2 常关； 3 自动
    int32_t  secret_mode;         // 私密模式: 0 不开启私有模式; 1 开始私有模式
    ipc_ConfigGet_NotifyInfo  notify;          //  通知参数
    uint32_t power_freq;

    char time_zone[32];              // 字符串，采用标准时区，时区信息携带了夏令时等信息
    int32_t time_dst;                  //  是否开启夏令时: 0: disable; 1: enable
    int32_t time_offset;			// 时间偏移，分钟
    ipc_MotionzoneSet_Req_mz_t motion_mz;              //  将显示区域划分为: 22x18的区块，共需要50个字节来表示，该区块被选中，则为1，反之为0
    ipc_MotionzoneSet_XYPoint  motion_xy_point[5];              //  界面设置的坐标
    int32_t   motion_num_point;                 //  坐标的点数, 最大5个

    ipc_PspList_PspInfo  pspinfo_arr[5];

    int32_t  Timedcruise_states;                //  status: 0 disable; 1 enable;
    int32_t  Timedcruise_mode;                  //  巡航模式: 1, 完整巡航; 2, 预置点巡航;
    xciot_Timetask Timedcruise_tt;                //  时间参数
    int32_t  Timedcruise_interval;               // 巡航时间间隔,单位秒；取值范围: 600-43200 之间的数据
    int64_t  autotrack;                         //自动跟踪: 0, 关闭; 1, 开启，

    int32_t motion_detection;
    int32_t opensound_detection;
    int32_t smoke_detection;
    int32_t shadow_detection;

    uint32_t  event_record_duration; //录像时间长度

};

struct test g_test_t;

void cmd_test_init(void)
{

    memset(&g_test_t, 0, sizeof(g_test_t));
    strcpy(g_test_t.wifi_ssid, "ssid");
    strcpy(g_test_t.wifi_key, "wifi_key");
    g_test_t.video_qos = 15;
    g_test_t.flip = 1;
    g_test_t.volume_value = 2;
    g_test_t.history_plan_record_type = 1;

    g_test_t.history_plan_plan_count = 0;
    g_test_t.history_plan_enable = 1;
    g_test_t.led_mode = 1;
    g_test_t.ircut_mode = 1;
    g_test_t.secret_mode = 0;

    g_test_t.power_freq = 50;
    strcpy(g_test_t.time_zone, "UTC-8");
    g_test_t.time_dst = 1;
    g_test_t.notify.states = 1;
    g_test_t.notify.level = 1;
    g_test_t.motion_mz.bytes[0] = 1;

    g_test_t.motion_num_point = 3;
    for (int i = 0; i < g_test_t.motion_num_point; i++) {
        g_test_t.motion_xy_point[i].leftup_x = 20 + i;
        g_test_t.motion_xy_point[i].leftup_y = i;
        g_test_t.motion_xy_point[i].rightdown_x = 10 + i;
        g_test_t.motion_xy_point[i].rightdown_y = 5 + i;
    }

    g_test_t.Timedcruise_states = 1;
    g_test_t.motion_detection = 1;
    g_test_t.smoke_detection = 1;
    g_test_t.shadow_detection = 1;
    g_test_t.event_record_duration = 2;
}
#endif // 0

const unsigned char utf8JLML_skill[] = {
    0x7B, 0x22, 0x62, 0x61, 0x73, 0x65, 0x22, 0x3A, 0x7B, 0x22, 0x70, 0x72, 0x6F, 0x74, 0x6F, 0x63,
    0x6F, 0x6C, 0x22, 0x3A, 0x22, 0x70, 0x70, 0x72, 0x70, 0x63, 0x22, 0x2C, 0x22, 0x65, 0x6E, 0x63,
    0x6F, 0x64, 0x65, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x65, 0x6E, 0x63, 0x72, 0x79, 0x70, 0x74, 0x22,
    0x3A, 0x33, 0x2C, 0x22, 0x73, 0x65, 0x63, 0x5F, 0x63, 0x6F, 0x64, 0x65, 0x22, 0x3A, 0x30, 0x2C,
    0x22, 0x68, 0x62, 0x5F, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x76, 0x61, 0x6C, 0x22, 0x3A, 0x30, 0x2C,
    0x22, 0x6F, 0x66, 0x66, 0x6C, 0x69, 0x6E, 0x65, 0x5F, 0x63, 0x6D, 0x64, 0x22, 0x3A, 0x31, 0x2C,
    0x22, 0x6C, 0x65, 0x64, 0x22, 0x3A, 0x32, 0x2C, 0x22, 0x62, 0x61, 0x74, 0x74, 0x65, 0x72, 0x79,
    0x22, 0x3A, 0x31, 0x2C, 0x22, 0x73, 0x65, 0x6E, 0x64, 0x5F, 0x6D, 0x6F, 0x64, 0x65, 0x22, 0x3A,
    0x30, 0x2C, 0x22, 0x61, 0x70, 0x73, 0x65, 0x63, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x65, 0x76, 0x65,
    0x6E, 0x74, 0x73, 0x22, 0x3A, 0x5B, 0x38, 0x33, 0x2C, 0x33, 0x5D, 0x2C, 0x22, 0x74, 0x61, 0x6D,
    0x70, 0x65, 0x72, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x73, 0x79, 0x6E, 0x63, 0x5F, 0x6E, 0x61, 0x6D,
    0x65, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x74, 0x68, 0x72, 0x65, 0x73, 0x68, 0x6F, 0x6C, 0x64, 0x22,
    0x3A, 0x30, 0x2C, 0x22, 0x66, 0x69, 0x6C, 0x65, 0x5F, 0x66, 0x6F, 0x72, 0x77, 0x61, 0x72, 0x64,
    0x69, 0x6E, 0x67, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x72, 0x65, 0x62, 0x6F, 0x6F, 0x74, 0x5F, 0x74,
    0x69, 0x6D, 0x65, 0x22, 0x3A, 0x36, 0x30, 0x7D, 0x2C, 0x22, 0x6D, 0x63, 0x6F, 0x6E, 0x66, 0x22,
    0x3A, 0x5B, 0x7B, 0x22, 0x63, 0x6C, 0x61, 0x73, 0x73, 0x5F, 0x63, 0x6F, 0x64, 0x65, 0x22, 0x3A,
    0x22, 0x49, 0x50, 0x41, 0x56, 0x22, 0x2C, 0x22, 0x63, 0x6F, 0x6E, 0x66, 0x22, 0x3A, 0x7B, 0x22,
    0x73, 0x70, 0x65, 0x63, 0x22, 0x3A, 0x33, 0x2C, 0x22, 0x61, 0x75, 0x64, 0x69, 0x6F, 0x22, 0x3A,
    0x32, 0x2C, 0x22, 0x6D, 0x69, 0x63, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x73, 0x70, 0x65, 0x61, 0x6B,
    0x65, 0x72, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x73, 0x69, 0x72, 0x65, 0x6E, 0x22, 0x3A, 0x30, 0x2C,
    0x22, 0x6C, 0x69, 0x67, 0x68, 0x74, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x70, 0x69, 0x78, 0x65, 0x6C,
    0x22, 0x3A, 0x5B, 0x34, 0x2C, 0x32, 0x2C, 0x31, 0x5D, 0x2C, 0x22, 0x70, 0x69, 0x78, 0x65, 0x6C,
    0x5F, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x22, 0x3A, 0x5B, 0x34, 0x5D, 0x2C, 0x22, 0x73, 0x64, 0x63,
    0x61, 0x72, 0x64, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x61, 0x73, 0x70, 0x65, 0x63, 0x74, 0x5F, 0x72,
    0x61, 0x74, 0x69, 0x6F, 0x22, 0x3A, 0x22, 0x34, 0x3A, 0x33, 0x22, 0x2C, 0x22, 0x6D, 0x6F, 0x74,
    0x69, 0x6F, 0x6E, 0x7A, 0x6F, 0x6E, 0x65, 0x73, 0x22, 0x3A, 0x32, 0x2C, 0x22, 0x70, 0x74, 0x7A,
    0x22, 0x3A, 0x30, 0x2C, 0x22, 0x66, 0x61, 0x63, 0x65, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x63, 0x6C,
    0x6F, 0x75, 0x64, 0x73, 0x22, 0x3A, 0x32, 0x2C, 0x22, 0x7A, 0x6F, 0x6F, 0x6D, 0x22, 0x3A, 0x31,
    0x2C, 0x22, 0x61, 0x69, 0x5F, 0x76, 0x65, 0x6E, 0x64, 0x6F, 0x72, 0x22, 0x3A, 0x30, 0x2C, 0x22,
    0x70, 0x69, 0x72, 0x22, 0x3A, 0x7B, 0x22, 0x6E, 0x75, 0x6D, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x72,
    0x61, 0x6E, 0x67, 0x69, 0x6E, 0x67, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x76, 0x61, 0x6C, 0x75, 0x65,
    0x73, 0x22, 0x3A, 0x5B, 0x30, 0x2C, 0x31, 0x2C, 0x32, 0x2C, 0x33, 0x5D, 0x7D, 0x2C, 0x22, 0x66,
    0x6C, 0x69, 0x70, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x6F, 0x73, 0x64, 0x22, 0x3A, 0x30, 0x2C, 0x22,
    0x70, 0x73, 0x70, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x63, 0x72, 0x75, 0x69, 0x73, 0x65, 0x22, 0x3A,
    0x30, 0x2C, 0x22, 0x73, 0x6F, 0x75, 0x6E, 0x64, 0x5F, 0x64, 0x65, 0x74, 0x65, 0x63, 0x74, 0x22,
    0x3A, 0x31, 0x2C, 0x22, 0x61, 0x75, 0x64, 0x69, 0x6F, 0x5F, 0x63, 0x6F, 0x64, 0x65, 0x63, 0x22,
    0x3A, 0x30, 0x2C, 0x22, 0x76, 0x69, 0x64, 0x65, 0x6F, 0x5F, 0x63, 0x6F, 0x64, 0x65, 0x63, 0x22,
    0x3A, 0x30, 0x2C, 0x22, 0x67, 0x70, 0x73, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x6C, 0x6F, 0x63, 0x61,
    0x6C, 0x5F, 0x68, 0x74, 0x74, 0x70, 0x64, 0x6F, 0x77, 0x6E, 0x22, 0x3A, 0x30, 0x2C, 0x22, 0x72,
    0x65, 0x6D, 0x6F, 0x74, 0x65, 0x5F, 0x61, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x22, 0x3A, 0x30, 0x2C,
    0x22, 0x6E, 0x69, 0x67, 0x68, 0x74, 0x5F, 0x6C, 0x69, 0x67, 0x68, 0x74, 0x22, 0x3A, 0x30, 0x2C,
    0x22, 0x72, 0x65, 0x63, 0x6F, 0x72, 0x64, 0x5F, 0x6D, 0x6F, 0x64, 0x65, 0x22, 0x3A, 0x30, 0x2C,
    0x22, 0x70, 0x6F, 0x77, 0x65, 0x72, 0x5F, 0x66, 0x72, 0x65, 0x71, 0x22, 0x3A, 0x5B, 0x5D, 0x2C,
    0x22, 0x6D, 0x6F, 0x74, 0x69, 0x6F, 0x6E, 0x5F, 0x67, 0x72, 0x69, 0x64, 0x5F, 0x73, 0x63, 0x61,
    0x6C, 0x65, 0x22, 0x3A, 0x22, 0x34, 0x3A, 0x33, 0x22, 0x2C, 0x22, 0x72, 0x65, 0x6E, 0x64, 0x65,
    0x72, 0x22, 0x3A, 0x31, 0x2C, 0x22, 0x63, 0x68, 0x61, 0x6E, 0x73, 0x22, 0x3A, 0x31, 0x2C, 0x22,
    0x64, 0x65, 0x63, 0x5F, 0x6D, 0x69, 0x78, 0x22, 0x3A, 0x31, 0x7D, 0x7D, 0x5D, 0x7D, 0x00
};


bool dev_on_ipc_Discovery(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_Discovery_Req *req,
    /* output  */ ipc_Discovery_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_I("conn[%d]ipc_Discovery_Req:", conn_id);
    T_T("\t did       :%s", req->did);
    T_T("\t class_code:%s", req->class_code);

    /*注意，该接口只需填入mode、ipaddr、netmask三个参数，其他内部复写填充*/

    /*设备模式(wifi)： 1 AP 模式; 2 STA模式;*/
    rsp->mode = 2;

    /*需要用户填写真实的ip和netmask*/
    char ip[16];
    Get_IPAddress(1, ip);
    snprintf(rsp->ipaddr, sizeof(rsp->ipaddr), "%s", ip);
    snprintf(
        rsp->netmask, sizeof(rsp->netmask), "%s", "255.255.255.0");
    /* 设备特征信息，在web后台下载，每类设备唯一，
     * 需要无外网访问功能的必须填写
     */
    //从内存中读取,skill_buff使用文本工具，将"替换成\"即可
    rsp->skill.size = strlen(utf8JLML_skill);
    memcpy(rsp->skill.bytes, utf8JLML_skill, rsp->skill.size);
    T_TH_HEX("skill json", rsp->skill.bytes, rsp->skill.size);


    return retval;
}

bool dev_on_ipc_WifiAPGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_WifiAPGet_Req *req,
    /* output  */ ipc_WifiAPGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_WifiAPGet_Req:", conn_id);
    T_I("\t channel:%d", req->channel);

    struct WIFI_LIST_INFO *get_wifi_list_info(void);
    struct WIFI_LIST_INFO *wifi_list_info = get_wifi_list_info();
    extern int get_wifi_list_info_cnt(void);
    // todo 填充设备扫描到wifi列表

    rsp->support = 1;
    rsp->wifis_count = 0;
    for (int i = 0; i < get_wifi_list_info_cnt(); i++) {
        if (strlen(wifi_list_info[i].ssid)) {
            rsp->wifis[rsp->wifis_count].qos  = wifi_list_info[i].qos;
            strcpy(rsp->wifis[rsp->wifis_count].ssid, wifi_list_info[i].ssid);
            //T_D("wifi_info->info_arr_[%d].ssid_ :%s ", i, rsp->wifis[i].ssid);
            rsp->wifis_count++;
        }
    }

    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_WIFI_LIST);

    return retval;
}

bool dev_on_ipc_WifiSet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_WifiSet_Req *req,
    /* output  */ ipc_WifiSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_WifiSet_Req:", conn_id);
    T_I("\t channel:%d", req->channel);
    T_I("\t ssid   :%s", req->ssid);
    T_I("\t pwd    :%s", req->pwd);

    //@todo 退出AP模式，去连接下发wifi网络

    set_net_cfg_info(req->ssid, req->pwd, NULL);

    post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_AP_CHANGE_STA);


    return retval;
}

bool dev_on_ipc_WifiGet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_WifiGet_Req *req,
    /* output  */ ipc_WifiGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_WifiGet_Req:", conn_id);
    T_I("\t channel:%d", req->channel);
#if 0
    //todo填充wifi信息
    if (strlen(g_test_t.wifi_ssid)) {
        strcpy(rsp->ssid, g_test_t.wifi_ssid);
    } else {
        strcpy(rsp->ssid, "wifi_ssid");
    }
#else
    strcpy(rsp->ssid, "wifi_ssid");
#endif
    rsp->qos = -60;
    rsp->support = 1;


    return retval;
}

bool dev_on_ipc_VideoPlay(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_VideoPlay_Req *req,
    /* output  */ ipc_VideoPlay_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_VideoPlay_Req:", conn_id);

    if(is_video_dec_mode()){
        return false;
    }
    T_I("\t channel:%d", req->channel);
    T_I("\t qos    :%d", req->qos);
    T_I("\t speed  :%d", req->speed);
    T_I("\t secret  :%d",db_select("secret") );


    rsp->code=db_select("secret");

    /*将连接ID添加到视频播放用户组*/
#ifndef CONFIG_NET_DUAL_STREAM_ENABLE

   if(db_select("secret")!=1){
    avsdk_video_add_conn(conn_id, req->channel); //私密就不跑
    }
#else
    avsdk_video_add_conn(conn_id, 1);
    avsdk_video_add_conn(conn_id, 2);
#endif // CONFIG_NET_DUAL_STREAM_ENABLE

    doorbell_video_rec_get_iframe(CONFIG_NET_SUB_ID);

    //todo填充视频信息
    rsp->qos = db_select("vqua");
    rsp->fps = 15;
    rsp->format = E_IOT_MEDIASTREAM_TYPE_H264;
    rsp->render = 0;
    add_video_play_info(conn_id);

    return retval;
}

bool dev_on_ipc_VideoPause(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_VideoPause_Req *req,
    /* output  */ ipc_VideoPause_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_VideoPause_Req:", conn_id);
    T_I("\t channel:%d", req->channel);

    /*将连接ID从视频播放用户组删除*/
    avsdk_video_del_conn(conn_id, req->channel);
    del_video_play_info(conn_id);
    return retval;
}

bool dev_on_ipc_VideoQosSet(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_VideoQosSet_Req *req,
    /* output  */ ipc_VideoQosSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_VideoQosSet_Req:", conn_id);
    T_I("\t channel:%d", req->channel);
    T_I("\t qos    :%d", req->qos);
    T_I("\t speed  :%d", req->speed);

    //todo设置视频质量
    db_update("vqua", req->qos);
    db_flush();

    //APP切换分辨率时，所有的码流必须重开
    post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_ALL_REOPEN_RT_STREAM);

    return retval;
}

extern struct  camera_platform_data sensor_type;
extern s32 GC1084_DVP_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio);
extern s32 sc2336p_mipi_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio);

extern s32 SC2336_mipi_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio);
//镜头翻转
static int sensor_flip_rotate_type(ipc_FlipSet_Req *flip)
{
    int ret ;
    isp_sen_t *c;
    list_for_each_camera(c) {
        printf("\n  (const char *)(c->logo) ::::%s\n",(const char *)(c->logo));
        if (!strcmp((const char *)(c->logo), "sc2336p")) {

            #if 0
            if(!sc2336p_mipi_check(0, sensor_type.reset_gpio, sensor_type.pwdn_gpio)){
            #else
             if(!SC2336_mipi_check(0, sensor_type.reset_gpio, sensor_type.pwdn_gpio)){
            #endif
            printf("\n sensor 2336P \n");

            extern void sensor_sc2336p_flip_rotate( u8 state);
            sensor_sc2336p_flip_rotate(flip->flip );


             break;
            }

        }else if(!strcmp((const char *)(c->logo), "GC1084M")){


         if(!GC1084_DVP_check(0, sensor_type.reset_gpio, sensor_type.pwdn_gpio)){

            printf("\n sensor GC1084 \n");
            extern void sensor_gc1084_flip_rotate( u8 state);
            sensor_gc1084_flip_rotate(flip->flip );
            break;
            }

        }


    }

    return ret;
}

bool dev_on_ipc_FlipSet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_FlipSet_Req *req,
    /* output  */ ipc_FlipSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    printf("\n req->flip===============%d,rsp->dummy_field:%d\n ",req->flip,rsp->dummy_field);
    // todo 视频翻转
//	g_test_t.flip = req->flip;
   db_update("flip",req->flip);
   db_flush();

   sensor_flip_rotate_type(req);

   #if 0
   #if (defined XD_CONFIG_BOARD_DEV_WIFI_5713_20231208)||(defined  IPC_50_NUMBER_TEST_DEMO)

    extern void sensor_sc2336p_flip_rotate( u8 state);
    sensor_sc2336p_flip_rotate(req->flip );
   #else if( LP_CONFIG_BOARD_DEV_WIFI_5713_20240426)
    extern void sensor_gc1084_flip_rotate( u8 state);
    sensor_gc1084_flip_rotate(req->flip );
   #endif
   #endif

    return retval;

}

bool dev_on_ipc_AudioPlay(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_AudioPlay_Req *req,
    /* output  */ ipc_AudioPlay_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_I("conn[%d]ipc_AudioPlay_Req:", conn_id);
    T_I("\t channel:%d", req->channel);

    /*将连接ID添加到音频播放用户组*/
    avsdk_audio_add_conn(conn_id);

    //todo 填充音频参数
    rsp->bit = 16;
    rsp->code = 0;
#if (CONFIG_AUDIO_FORMAT_FOR_MIC == AUDIO_FORMAT_AAC)
    rsp->codec = E_IOT_MEDIASTREAM_TYPE_AAC;
#else
    rsp->codec = E_IOT_MEDIASTREAM_TYPE_PCM;
#endif
    rsp->track = 1;
    rsp->rate = net_video_rec_get_audio_rate();

    T_I("rsp->bit:%d", rsp->bit);
    T_I("rsp->code:%d", rsp->code);
    T_I("rsp->codec:%d", rsp->codec);
    T_I("rsp->track:%d", rsp->track);
    T_I("rsp->rate:%d", rsp->rate);
    return retval;
}

bool dev_on_ipc_AudioPause(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_AudioPause_Req *req,
    /* output  */ ipc_AudioPause_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d]ipc_AudioPause_Req:", conn_id);
    T_I("\t channel:%d", req->channel);

    /*将连接ID从音频播放用户组删除*/
    avsdk_audio_del_conn(conn_id);

    return retval;
}

bool dev_on_ipc_TalkbackPlay(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_TalkbackPlay_Req *req,
    /* output  */ ipc_TalkbackPlay_Resp *rsp)
{
    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    /* retval - true: support, false: unsupport */
    bool retval = true;
    if (find_audio_play_info() || get_video_rec_in_state() == REC_STA_IN_MSG  || is_video_dec_mode())  {
        return false;
    }
    post_msg_doorbell_task("audio_dec_task", 1, RT_AUDIO_CTRL_START);
    /* 返回设置支持的音频格式 */
#if (CONFIG_AUDIO_FORMAT_FOR_SPEARK == AUDIO_FORMAT_AAC)

    rsp->codec = E_IOT_MEDIASTREAM_TYPE_AAC;       	// codec type
#else
    rsp->codec = E_IOT_MEDIASTREAM_TYPE_PCM;
#endif
    rsp->rate  = CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK;               					// 8000? audio frequency
    rsp->bit   = 16;                 					// 16bit?
    rsp->track = 1;                  					// mono
    rsp->code  = 0;                  					// normal mode


    add_audio_play_info(conn_id);

    extern void net_set_phone_status(u8 status);
    net_set_phone_status(1);

    void usb_speak_switch(void *state);
    usb_speak_switch("on");

    return retval;
}

bool dev_on_ipc_TalkbackPause(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_TalkbackPause_Req *req,
    /* output  */ ipc_TalkbackPause_Resp *rsp)
{
    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    /* retval - true: support, false: unsupport */
    bool retval = true;

    del_audio_play_info(conn_id);
    extern void net_set_phone_status(u8 status);
    net_set_phone_status(0);



   // void usb_speak_switch(void *state);
   // usb_speak_switch("off");

    RT_TALK_INFO info = {0};
    if(!find_device_video_play_info(&info) && get_video_call_state() != 0){
      post_msg_doorbell_task("video_dec_task", 3, APP_STOP_VIDEO_CALL_STATE,APP_REFUSE_STATE,conn_id);

    }




    return retval;
}



struct  video_splan {

    int32_t  history_plan_record_type;           // 录像类型: 1, 连续录像; 2, 事件录像
    xciot_Timetask  history_plan_tt[3];             // 时间段(最多3个)
    int32_t   history_plan_plan_count;               // 个数
    int32_t   history_plan_enable;                    //  是否开启录像；0：disable;1 enable

};
struct video_splan video_plan={0 } ;;
#include "video_system.h"
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
void clear_time_data(){


    memset(&video_plan,0,sizeof(struct video_splan));
    dev_write_plane_time_vm(&video_plan,sizeof(struct video_splan));


}
//  默认全天 ，事件录像

void default_history_planetime(struct video_splan *rsp){

#if 1
    rsp->history_plan_record_type = 2;
	rsp->history_plan_plan_count = 1;//默认一组
	rsp->history_plan_enable = 1;
    for(int32_t i=0; i<rsp->history_plan_plan_count; i++) {
         rsp->history_plan_tt[i].days_count=0;
         rsp->history_plan_tt[i].week_day_count=7;
         rsp->history_plan_tt[i].week_day[0]=0;
         rsp->history_plan_tt[i].week_day[1]=1;
         rsp->history_plan_tt[i].week_day[2]=2;
         rsp->history_plan_tt[i].week_day[3]=3;
         rsp->history_plan_tt[i].week_day[4]=4;
         rsp->history_plan_tt[i].week_day[5]=5;
         rsp->history_plan_tt[i].week_day[6]=6;
         rsp->history_plan_tt[i].time_count=1;
         rsp->history_plan_tt[i].time[i].start_sec=0;
         rsp->history_plan_tt[i].time[i].end_sec=86347;
         rsp->history_plan_tt[i].enable=1;
    for(int k=0;k< sizeof(rsp->history_plan_tt[i].days)/sizeof( rsp->history_plan_tt[i].days[0]);  k++ )
      {
          rsp->history_plan_tt[k].days[k]=0;

      }


    }
#endif

}

void default_plan_timer(){


   // db_update("cyc",0);
    db_update("mot",1);
    db_flush();
    memset(&video_plan,0,sizeof(struct video_splan));
    default_history_planetime(&video_plan);//默认事件 全天录像
    dev_write_plane_time_vm(&video_plan,sizeof(struct video_splan));//保存vm


}



bool judge_plan_video_timer(){


    #ifdef ENABLE_VIDEO_FILE_AUTO_ADD
    if(!get_avsdk_connect_flag()){

    printf("\n no connect platform \n");
     return 1;  //未联上平台 继续循环录像

    }
    #endif

    extern int dev_read_plane_time_vm(char *data, int len);
    dev_read_plane_time_vm(&video_plan, sizeof(struct video_splan));

    //printf("\n  video_plan.history_plan_plan_count=======%d,max_size:%d\n",video_plan.history_plan_plan_count,sizeof(video_plan.history_plan_tt)/sizeof(video_plan.history_plan_tt[0]));
    printf("\n  plan_count=======%d\n",video_plan.history_plan_plan_count);

    //printf("\n time_count: %d\n",video_plan.history_plan_tt[0].time_count);
    if(video_plan.history_plan_plan_count>3 ||video_plan.history_plan_plan_count<=0||  video_plan.history_plan_tt[0].time_count>3||video_plan.history_plan_tt[0].enable==0 ||video_plan.history_plan_record_type>2){

      //memset(&video_plan,0,sizeof(struct video_splan));

        default_plan_timer();//默认事件 全天录像

        printf("\n  -------------no set timer---------------- \n");
        return 1;

    }

    struct sys_time time;
    struct tm standby_time= {0};
    covUnixTimeStp2Beijing(get_utc_ms() / 1000,&standby_time);
    if (__get_sys_time(&time) == 0) {
  //  sprintf(file_name, "%s_%d%02d%02d_%02d%02d%02d.MOV", file_name_prefix,
  //                  time.year, time.month, time.day, time.hour, time.min, time.sec);

    printf("convert is: %04d/%02d/%02d-%02d:%02d:%02d cur_weed = %d year_day = %d\n",
            standby_time.tm_year, standby_time.tm_mon, standby_time.tm_mday, \
            standby_time.tm_hour, standby_time.tm_min, standby_time.tm_sec,standby_time.tm_wday, standby_time.tm_yday);
    printf("\n time.year:%d\n",time.year);
    if(time.year<2024){// 未联网情况下 默认开启录像

        return 1;
    }
    int cur_timer= standby_time.tm_hour*60*60+standby_time.tm_min*60+standby_time.tm_sec;
    int cur_timer_min= standby_time.tm_hour*60+standby_time.tm_min;

    printf("\n cur_s:%d,cur_min:%d\n",cur_timer,cur_timer_min);
    for(int32_t i=0; i<video_plan.history_plan_plan_count; i++) {

         int plan_s_miute =video_plan.history_plan_tt[i].time[0].start_sec/60;
         int plan_e_miute =video_plan.history_plan_tt[i].time[0].end_sec/60;

        for(int32_t j=0; j<video_plan.history_plan_tt[i].time_count; j++) {

        T_D("justset[%d] start sec:%d end sec :%d,week_day_count:%d\n",i, video_plan.history_plan_tt[i].time[j].start_sec, video_plan.history_plan_tt[i].time[j].end_sec,video_plan.history_plan_tt[i].week_day_count);
        for(int m=0;m<7;m++  ){

             T_D(" justset week count_time:%d week[%d] :%d\n",i,m, video_plan.history_plan_tt[i].week_day[m]);

            }
        }

       for(int k=0;k<video_plan.history_plan_tt[i].week_day_count;k++){//每一个计划 有几天

       //static u8 plan_count=0;
       //printf("\n curr:%d\n ",cur_timer);
       //printf("\n  i:%d,k:%d, plan_week:%d\n",i,k,video_plan.history_plan_tt[i].week_day[k]);
       //printf("\n  standby_time.tm_wday:%d\n",standby_time.tm_wday);





       if(video_plan.history_plan_tt[i].week_day[k]==standby_time.tm_wday)//先判断星期
        {
          putchar('F');

         if(video_plan.history_plan_tt[i].time[0].end_sec<=video_plan.history_plan_tt[i].time[0].start_sec){ //时间隔天了
            putchar('H');
          // if( (cur_timer) > video_plan.history_plan_tt[i].time[0].start_sec){
           if( (cur_timer_min) >= plan_s_miute){
             putchar('I');
           // printf("\n g_i:%d,k:%d\n",i,k);
            return 1;

           }

         }else{
         putchar('J');

         //printf("\n a:%d\n",cur_timer);
        // printf("\n b:%d\n",video_plan.history_plan_tt[i].time[0].start_sec);//3547
        // printf("\n c:%d\n",video_plan.history_plan_tt[i].time[0].end_sec);

        //if( ((cur_timer) >=video_plan.history_plan_tt[i].time[0].start_sec )&&((cur_timer)<=video_plan.history_plan_tt[i].time[0].end_sec) )
        if( ((cur_timer_min) >=plan_s_miute )&&((cur_timer_min)<=plan_e_miute) )
        {

           //printf("\n i:%d,k:%d\n",i,k);
            putchar('K');
            return 1;

         }
        }
      }else{ // 隔天情况


          putchar('L');

           if( (video_plan.history_plan_tt[i].week_day[k]+1)==standby_time.tm_wday)  // k跨天时， 刚好实际时间过了凌晨  则需要 app+1
            {
             putchar('M');
               if(video_plan.history_plan_tt[i].time[0].end_sec<=video_plan.history_plan_tt[i].time[0].start_sec){ //app 设置隔天情况
                    putchar('N');
                 if( (cur_timer_min<=plan_e_miute) ){
                     putchar('O');
                    return 1;


                 }
            }
         }

      }

    }

      // return 0;  //  20:  19:20
    }
}
    putchar('N');
    return 0;

}

#if 0


bool dev_on_ipc_HistoryPlanSet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryPlanSet_Req * req,
    /* output  */ ipc_HistoryPlanSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

	T_D("%s called ", __func__);

	//todo 配置录像方式

    T_D("plan_count: %d ", req->tt_count);
    T_D("enable = %d", req->enable);

	g_test_t.history_plan_record_type = req->record_type;
    g_test_t.history_plan_plan_count = req->tt_count;
    for(int i=0; i<req->tt_count; i++) {
        memcpy(&g_test_t.history_plan_tt[i], &req->tt[i], sizeof(req->tt[i]));

        T_D("[%d], day_count: %d ",i, req->tt[i].days_count);//为0
        for(int j=0; j<req->tt[i].days_count; j++) {
            T_D("set[%d] days[%d] :%d\n",i, j, req->tt[i].days[j]);
        }

        T_D("[%d], week_day_count: %d ",i, req->tt[i].week_day_count);
        for(int j=0; j<req->tt[i].week_day_count; j++) {
            T_D("set[%d] week days[%d] :%d\n",i, j, req->tt[i].week_day[j]);
        }

        T_D("[%d], time_count: %d ",i, req->tt[i].time_count);
        for(int j=0; j<req->tt[i].time_count; j++) {
            T_D("set[%d] start sec:%d end sec :%d\n",i, req->tt[i].time[j].start_sec, req->tt[i].time[j].end_sec);
        }

    }
    g_test_t.history_plan_enable = req->enable;

    return retval;
}

bool dev_on_ipc_HistoryPlanGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryPlanGet_Req * req,
    /* output  */ ipc_HistoryPlanGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

	//todo 返回录像方式

    memset(rsp->tt, 0, sizeof(rsp->tt));
	rsp->record_type = g_test_t.history_plan_record_type;
	rsp->tt_count = g_test_t.history_plan_plan_count;

    for(int32_t i=0; i<g_test_t.history_plan_plan_count; i++) {
        memcpy(&rsp->tt[i], &g_test_t.history_plan_tt[i], sizeof(g_test_t.history_plan_tt[i]));
    }
    rsp->enable = g_test_t.history_plan_enable;

    T_D("plan_count: %d ", rsp->tt_count);
    T_D("enable = %d", rsp->enable);
    for(int32_t i=0; i<rsp->tt_count; i++) {
        memcpy(&g_test_t.history_plan_tt[i], &rsp->tt[i], sizeof(rsp->tt[i]));
        for(int j=0; j<rsp->tt[i].time_count; j++) {
            T_D("resp[%d]: start sec:%d end sec :%d\n",i, rsp->tt[i].time[j].start_sec, rsp->tt[i].time[j].end_sec);
        }

    }


    return retval;
}


#else
bool dev_on_ipc_HistoryPlanSet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryPlanSet_Req *req,
    /* output  */ ipc_HistoryPlanSet_Resp *rsp)
{


    /* retval - true: support, false: unsupport */
    bool retval = true;
    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    T_D("%s called ", __func__);

    //todo 配置录像方式

    if(req->record_type==1){

   // db_update("cyc",req->record_type);
    db_update("mot",!req->record_type);

    }else{
   // db_update("cyc",0);
    db_update("mot",1);

    }
    db_flush();

    T_D("plan_count: %d ", req->tt_count);
    T_D("enable = %d", req->enable);
    T_D("channel = %d", req->channel);
    T_D("record_type = %d", req->record_type);





    video_plan.history_plan_record_type = req->record_type;
    video_plan.history_plan_plan_count = req->tt_count;

    #if 1
    for(int i=0; i<req->tt_count; i++) {
        memcpy(&video_plan.history_plan_tt[i], &req->tt[i], sizeof(req->tt[i]));
         T_D("req->tt[%d].time_count = %d",i,req->tt[i].time_count);
         T_D("req->tt[%d].days_count = %d",i, req->tt[i].days_count);

        for(uint32_t j=0; j<req->tt[i].time_count; j++) {
            T_D("set[%d] start sec:%d end sec :%d,week_day_count:%d\n",i, req->tt[i].time[j].start_sec, req->tt[i].time[j].end_sec,req->tt[i].week_day_count);
           for(int m=0;m<7;m++  ){
             T_D("set week count_time:%d week[%d] :%d\n",i,m, req->tt[i].week_day[m]);

            }
        }
      //  for(int32_t j=0; j<req->tt[i].week_day_count; j++) {
           // T_D("set[%d] start sec:%d end sec :%d\n",i, req->tt[i].week_day[j]);
        //}

    }

    #else
        for(int i=0; i<req->tt_count; i++) {

        memcpy(&video_plan.history_plan_tt[i], &req->tt[i], sizeof(req->tt[i]));

        T_D("[%d], day_count: %d ",i, req->tt[i].days_count);//为0
        for(int j=0; j<req->tt[i].days_count; j++) {
            T_D("set[%d] days[%d] :%d\n",i, j, req->tt[i].days[j]);
        }

        T_D("[%d], week_day_count: %d ",i, req->tt[i].week_day_count);
        for(int j=0; j<req->tt[i].week_day_count; j++) {
            T_D("set[%d] week days[%d] :%d\n",i, j, req->tt[i].week_day[j]);
        }

        T_D("[%d], time_count: %d ",i, req->tt[i].time_count);
        for(int j=0; j<req->tt[i].time_count; j++) {
            T_D("set[%d] start sec:%d end sec :%d\n",i, req->tt[i].time[j].start_sec, req->tt[i].time[j].end_sec);
        }

    }

    #endif

    video_plan.history_plan_enable = req->enable;

  //  db_update_buffer(VIDEO_PLAN_VM_INDEX, &video_plan, sizeof(struct video_splan));

    dev_write_plane_time_vm(&video_plan,sizeof(struct video_splan));



    int ret=judge_plan_video_timer();
    if(ret){

    printf("set_cyc:%d,%d\n",db_select("cyc"),db_select("mot"));
    if(db_select("cyc")&&!db_select("mot")){

    post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_START_REC);// 循环录像开启
    }

    }

    printf("\n set_ret=====%d\n",ret);

    return retval;





}







bool dev_on_ipc_HistoryPlanGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryPlanGet_Req *req,
    /* output  */ ipc_HistoryPlanGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    //todo 返回录像方式


    memset(rsp->tt, 0, sizeof(rsp->tt));

   // db_select_buffer(VIDEO_PLAN_VM_INDEX, &video_plan, sizeof(struct video_splan));
    extern int dev_read_plane_time_vm(char *data, int len);
    dev_read_plane_time_vm(&video_plan, sizeof(struct video_splan));

    printf("\n  history_plan_plan_count=======%d\n",video_plan.history_plan_plan_count);

    if(video_plan.history_plan_plan_count>3||video_plan.history_plan_plan_count<=0 ||  video_plan.history_plan_tt[0].time_count>3||video_plan.history_plan_tt[0].enable==0 ||video_plan.history_plan_record_type>2){

        printf("\n get plane is not set \n");
        default_plan_timer();
    }

    #if 1
	rsp->record_type = video_plan.history_plan_record_type;
	rsp->tt_count = video_plan.history_plan_plan_count;

    for(int i=0; i<video_plan.history_plan_plan_count; i++) {
        memcpy(&rsp->tt[i], &video_plan.history_plan_tt[i], sizeof(video_plan.history_plan_tt[i]));
    }
    rsp->enable = video_plan.history_plan_enable;

    T_D("plan_count: %d ", rsp->tt_count);
    T_D("enable = %d", rsp->enable);
    for(int i=0; i<rsp->tt_count; i++) {
        memcpy(&video_plan.history_plan_tt[i], &rsp->tt[i], sizeof(rsp->tt[i]));

        for(int j=0; j<rsp->tt[i].time_count; j++) {
            T_D("resp[%d]: start sec:%d end sec :%d,week_day_count:%d\n",i, rsp->tt[i].time[j].start_sec, rsp->tt[i].time[j].end_sec,rsp->tt[i].week_day_count);
               T_D("rsp->tt[i].enable= %d", rsp->tt[i].enable);

             for(int m=0;m<7;m++  ){

             T_D("set week count_time:%d week[%d] :%d\n",i,m, rsp->tt[i].week_day[m]);

            }
        }
      #if  0
      for(int k=0;k< sizeof(rsp->tt[i].days)/sizeof( rsp->tt[i].days[0]);  k++ )
      {
         T_D("rsp->tt[%d].days[%d]= %d",k,k, rsp->tt[i].days[k]);

      }
      #endif
    }

    #else
   //memset(rsp->tt, 0, sizeof(rsp->tt));
	rsp->record_type = video_plan.history_plan_record_type;
	rsp->tt_count = video_plan.history_plan_plan_count;

    for(int32_t i=0; i<video_plan.history_plan_plan_count; i++) {
        memcpy(&rsp->tt[i], &video_plan.history_plan_tt[i], sizeof(video_plan.history_plan_tt[i]));
    }
    rsp->enable = video_plan.history_plan_enable;

    T_D("plan_count: %d ", rsp->tt_count);
    T_D("enable = %d", rsp->enable);
    for(int32_t i=0; i<rsp->tt_count; i++) {
        memcpy(&video_plan.history_plan_tt[i], &rsp->tt[i], sizeof(rsp->tt[i]));
        for(int j=0; j<rsp->tt[i].time_count; j++) {
            T_D("resp[%d]: start sec:%d end sec :%d\n",i, rsp->tt[i].time[j].start_sec, rsp->tt[i].time[j].end_sec);
        }

    }

    #endif


    int ret=judge_plan_video_timer();

    printf("\n get_ret====%d\n",ret);
    return retval;


}
#endif
bool dev_on_ipc_HistoryDays(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_HistoryDays_Req *req,
    /* output  */ ipc_HistoryDays_Resp *rsp)
{

    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d>>>>>>>>>>>>>>>>>>\n", __func__, __LINE__);
    /* retval - true: support, false: unsupport */
    bool retval = true;

    // todo 返回历史记录天数
    void cloud_playback_list_get_days(void *__req, void *__rsp);
    cloud_playback_list_get_days(req, rsp);

    return retval;
}

bool dev_on_ipc_HistoryDayList(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryDayList_Req *req,
    /* output  */ ipc_HistoryDayList_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_I("----------dev_on_ipc_HistoryDayList--------");
    // todo 返回历史记录列表
    /* Fill output rsp, after this function, sdk will send rsp to remote */
    /* 录像分为两类，1.事件录像；2.全添录像,选择一种填写 rsp */



  //  printf("\n req->start_time:::%%llu\n ",req->start_time);
  //  printf("\n req->start_time:::%llu,req->channel:%d\n ",req->start_time,req->channel);



/*

        printf("\n Ap->start_time==================%llu\n,\
        rsp->historys_count:%d\n,\
        req->rtype:%d\n,\
        req->page:%d\n,\
        req->page_size:%d\n,\
        req->history_type_count:%d\n,\
        req->history_type[0]:%d\n,\
        req->history_type[1]:%d,\
        req->order:%d \n",\

           req->start_time,\
           rsp->historys_count,\
           req->rtype,\
           req->page,\
           req->page_size,\
           req->history_type_count,\
           req->history_type[0],\
           req->history_type[1],\
           req->order);
*/
    extern int cloud_playback_list_get(void *req, void *rsp);
    cloud_playback_list_get(req, rsp);





    return retval;
}

bool dev_on_ipc_HistoryPlay(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_HistoryPlay_Req *req,
    /* output  */ ipc_HistoryPlay_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_I("conn[%d]dev_on_ipc_HistoryPlay:", conn_id);
    T_I("\t channel:%d", req->channel);
    T_D("\t file_id:%lld", req->file_id);
  //  T_D("\t start_time:%lld", req->start_time);
    T_D("\t start_time:%llu", req->start_time);



    //todo 播放file_id响应的历史视频

    //sprintf(g_history_h264_file, "%llu.h264", req->file_id)

    /* Fill output rsp, after this function, sdk will send rsp to remote */
    {

        rsp->code = 0; //成功
        //本地录像无mp4封装，声音格式只能为pcm
        rsp->codec = E_IOT_MEDIASTREAM_TYPE_PCM;
        rsp->rate = net_video_rec_get_audio_rate();
        rsp->bit = 16;
        rsp->track = 1;
        rsp->fps = net_video_rec_get_fps();;
        extern int cloud_playback_init(int32_t conn_id,  uint64_t file_id, int64_t start_time,int32_t rtype);
        if(req->file_id){

         cloud_playback_init(conn_id, req->file_id, req->start_time / 1000,1);

        }else{
         cloud_playback_init(conn_id, req->file_id, req->start_time / 1000,0);

        }
    }

    return retval;
}

bool dev_on_ipc_HistoryPause(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_HistoryPause_Req *req,
    /* output  */ ipc_HistoryPause_Resp *rsp)
{
    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    T_D("----------------dev_on_ipc_HistoryPause-----------------");
    /* retval - true: support, false: unsupport */
    bool retval = true;

    extern int cloud_playback_uninit(int32_t conn_id,  uint64_t file_id);
    cloud_playback_uninit(conn_id, req->file_id);

#if ENABLE_DEMO_AV > 1
    // todo 停止播放历史视频
    if (thr_hv_ctx.init == false) {
        /* 未建立线程 */
    } else {
        thr_hv_ctx.init = false;
        thr_hv_ctx.conn_id = -1;
        thr_hv_ctx.toexit = true;
        T_D("----------------close_video_history_thread-----------------");
    }
#endif
    return retval;
}

bool dev_on_ipc_HistoryThumGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_HistoryThumGet_Req *req,
    /* output  */ ipc_HistoryThumGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    // todo 获取历史视频的缩略图

    int get_file_thm(char *name, char type, char *thm_addr, int len);

    char path[128]  = {0};
    struct tm st_time={0};struct tm file_time={0};;

    printf("\n req->thum_fid =%llu \n", req->thum_fid);

    extern void cloud_playback_list_get_name_for_start_time(uint64_t start_time,uint64_t *fd_time, char *path,int32_t rtype,struct tm *st_time,struct tm *file_time);
    cloud_playback_list_get_name_for_start_time(req->thum_fid,0, path,1,NULL,NULL);
    printf("\n path = %s\n",path);
    if (strlen(path)) {
        rsp->thum_body.size  = get_file_thm(path, 0, rsp->thum_body.bytes, sizeof(rsp->thum_body.bytes));
    }
#if 0
    /* Fill output rsp, after this function, sdk will send rsp to remote */
    {
        FILE *fp = fopen("thum.jpg", "rb");
        if (fp != NULL) { // 打开文件失败
            fseek(fp, 0, SEEK_END);//定位文件指针到文件尾。
            extern int flen(FILE * file);
            rsp->thum_body.size = flen(fp); //获取文件指针偏移量，即文件大小。
            T_D("resp_thum_body_length: %d ", rsp->thum_body.size);

            fseek(fp, 0, SEEK_SET);//定位文件指针到文件头。

            fread(rsp->thum_body.bytes, 1, rsp->thum_body.size, fp);

            fclose(fp);
            T_D("thum_body data[0]: 0x%02x, data[1]: 0x%02x ", rsp->thum_body.bytes[0], rsp->thum_body.bytes[1]);
        }
    }
#endif
    return retval;
}

bool dev_on_ipc_HistoryDel(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_HistoryDel_Req *req,
    /* output  */ ipc_HistoryDel_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    printf("\n >>>>>>>>>>>>>>>>>>>>  %s %d\n", __func__, __LINE__);
    //todo 删除对应的历史视频
    extern int cloud_playback_list_remove(void *__req, void *__rsp);
    cloud_playback_list_remove(req, rsp);
    return retval;
}



bool dev_on_ipc_ConfigGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_ConfigGet_Req *req,
    /* output  */ ipc_ConfigGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------dev_on_ipc_ConfigGet-------");
    //todo 获取所有的配置信息
    printf("\n  dev_on_ipc_ConfigGet \n");
    /* Fill output rsp, after this function, sdk will send rsp to remote */
    {
        T_D("dev_on_ipc_ConfigGet");
        rsp->flip = db_select("flip");
        rsp->led_mode = db_select("stled");
        rsp->ircut_mode = db_select("ircut");

        rsp->secret_mode = db_select("secret");

        //memcpy(&rsp->notify, &g_test_t.notify, sizeof(g_test_t.notify));

        rsp->power_freq = db_select("fre");
        rsp->volume = db_select("vol");
        rsp->duration = db_select("mot");
        printf("\n >>>>>>>>>>>> rsp->volume  = %d\n", rsp->volume);
        printf("\n rsp->duration = %d\n", rsp->duration);

        // 注意云台图片上传的排列顺序和   background_cfg_file数组需要对应起来，否则出现显示和序号不对应情况
         // 云台背景图上传按数字从小到大依次上传
        int64_t screen_number =db_select("screen");
       // printf("\n  report_picture====%d\n",screen_number);
        report_topic_cmmid(screen_number+1);


    }


    return retval;
}

void demo_report_timezone(void *arg)
{
    int rc = 0;
    int32_t  conn_id = OSAL_POINTER_TO_UINT(arg);
    T_D("conn_id: %d ", conn_id);
    char time_zone[32];              // 字符串，采用标准时区，时区信息携带了夏令时等信息
    db_select_buffer(TIMEZONE_VM_INDEX, time_zone, sizeof(time_zone));
    avsdk_report_TimeZone(time_zone);

}

bool dev_on_ipc_TimeSet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_TimeSet_Req *req,
    /* output  */ ipc_TimeSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("-------dev_on_ipc_TimeSet--------");
    T_D("req->now_time = %lld", req->now_time);
    T_D("req->dst = %d", req->dst);
    T_D("req->offset = %d", req->offset);
    T_D("req->time_zone = %s", req->time_zone);
    // todo 设置设备的时间


    char time_zone[32];              // 字符串，采用标准时区，时区信息携带了夏令时等信息
    strcpy(time_zone, req->time_zone);
    db_update_buffer(TIMEZONE_VM_INDEX, time_zone, sizeof(time_zone));
    db_update("tzone", req->offset);
    db_flush();

    set_utc_ms(req->now_time);
    thread_fork("demo_report_timezone", 8, 0x1000, 0, 0, demo_report_timezone, OSAL_UINT_TO_POINTER(conn_id));

    return retval;
}

bool dev_on_ipc_TimeGet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_TimeGet_Req *req,
    /* output  */ ipc_TimeGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("-------dev_on_ipc_TimeGet--------");
    // todo 获取设备的时间
    rsp->now_time    = time(0);

    char time_zone[32];              // 字符串，采用标准时区，时区信息携带了夏令时等信息
    if (db_select_buffer(TIMEZONE_VM_INDEX, time_zone, sizeof(time_zone) != sizeof(time_zone))) {
        strcpy(time_zone, "Asia/Shanghai");
    }
    strcpy(rsp->time_zone, time_zone);

    rsp->dst = 0;
    rsp->offset =  db_select("tzone");

    return retval;
}

bool dev_on_ipc_LedSet(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LedSet_Req *req,
    /* output  */ ipc_LedSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("-------dev_on_ipc_LedSet--------");
    // todo 设置LED模式S
    db_update("stled", req->mode);
    db_flush();

    return retval;
}

bool dev_on_ipc_LedGet(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LedGet_Req *req,
    /* output  */ ipc_LedGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("-------dev_on_ipc_LedGet--------");
    //todo 获取led的模式
    rsp->mode = db_select("stled");


    return retval;
}

bool dev_on_ipc_IRCutSet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_IRCutSet_Req *req,
    /* output  */ ipc_IRCutSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    puts("\n dev_on_ipc_IRCutSet \n");
    // todo 设置夜视模式
    db_update("ircut", req->mode);
    db_flush();
    return retval;
}

bool dev_on_ipc_IRCutGet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_IRCutGet_Req *req,
    /* output  */ ipc_IRCutGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    puts("\n dev_on_ipc_IRCutGet \n");
    //todo 获取夜视模式
    rsp->mode = db_select("ircut");

    return retval;
}

bool dev_on_ipc_SecretSet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_SecretSet_Req *req,
    /* output  */ ipc_SecretSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    puts("\n dev_on_ipc_SecretSet \n");
    // todo 设置私有模式
    printf("\n req->secret::%d\n",req->secret);
    db_update("secret", req->secret);
    db_flush();

     #if 1
     if(req->secret){
        //关闭屏幕

        static struct sys_event event = {0};

        event.arg = "VoiceCmd";
        event.type = SYS_DEVICE_EVENT;
        event.u.dev.event  = 0;
        sys_event_notify(&event);
    }else{
    //开屏幕
        static struct sys_event event = {0};

        event.arg = "VoiceCmd";
        event.type = SYS_DEVICE_EVENT;
        event.u.dev.event  = 1;
        sys_event_notify(&event);
    }
    #endif

    return retval;
}

bool dev_on_ipc_SecretGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_SecretGet_Req *req,
    /* output  */ ipc_SecretGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
     puts("\n dev_on_ipc_SecretGet \n");
    // todo 返回私有模式
    rsp->secret = db_select("secret");
    #if 0
      //打开屏幕
       static struct sys_event event = {0};
        event.arg = "VoiceCmd";
        event.type = SYS_DEVICE_EVENT;
        event.u.dev.event  = 0;
        sys_event_notify(&event);
    #endif
    return retval;
}

bool dev_on_ipc_NotifySet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_NotifySet_Req *req,
    /* output  */ ipc_NotifySet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    //todo 设置事件触发方式
#if 0
    /* Analysis input req */
    {
        g_test_t.notify.states = req->states;
        T_D("days_count_: %d ", req->tt.days_count);
        for (int i = 0; i < req->tt.days_count; i++) {
            T_D("days_[%d]: %d ", i, req->tt.days[i]);
        }

        T_D("week_day_count_: %d ", req->tt.week_day_count);
        for (int i = 0; i < req->tt.week_day_count; i++) {
            T_D("week_day_[%d]: %d ", i, req->tt.week_day[i]);
        }

        T_D("time_count_: %d ", req->tt.time_count);
        for (int i = 0; i < req->tt.time_count; i++) {
            T_D("time_info_[%d].start_sec_: %d ", i, req->tt.time[i].start_sec);
            T_D("time_info_[%d].end_sec_: %d ", i, req->tt.time[i].end_sec);
        }

        memcpy(&g_test_t.notify.tt, &req->tt, sizeof(req->tt));
        g_test_t.notify.level = req->level;
    }
#endif

    return retval;
}

bool dev_on_ipc_NotifyGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_NotifyGet_Req *req,
    /* output  */ ipc_NotifyGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    //todo 获取事件触发方式
#if 0
    /* Fill output rsp, after this function, sdk will send rsp to remote */
    {
        memcpy(&rsp->tt, &g_test_t.notify.tt, sizeof(g_test_t.notify.tt));
        rsp->level  = g_test_t.notify.level;
        rsp->states = g_test_t.notify.states;

        T_D("resp_level: %d ", rsp->level);
        T_D("resp_states: %d ", rsp->states);

        T_D("days_count_: %d ", rsp->tt.days_count);
        for (int i = 0; i < rsp->tt.days_count; i++) {
            T_D("days_[%d]: %d ", i, rsp->tt.days[i]);
        }

        T_D("week_day_count_: %d ", rsp->tt.week_day_count);
        for (int i = 0; i < rsp->tt.week_day_count ; i++) {
            T_D("week_day_[%d]: %d ", i, rsp->tt.week_day[i]);
        }

        T_D("time_count_: %d ", rsp->tt.time_count);
        for (int i = 0; i < rsp->tt.time_count ; i++) {
            T_D("time_info_[%d].start_sec_: %d ", i, rsp->tt.time[i].start_sec);
            T_D("time_info_[%d].end_sec_: %d ", i, rsp->tt.time[i].end_sec);
        }
    }
#endif
    return retval;
}

bool dev_on_ipc_MotionzoneSet(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_MotionzoneSet_Req *req,
    /* output  */ ipc_MotionzoneSet_Resp *rsp)
{
    T_I("---------dev_on_ipc_MotionzoneSet--------");
    /* retval - true: support, false: unsupport */
    bool retval = true;
#if 0
    /* Motionzone */
    {
        T_I("---------MotionzoneSet  mz.size=%d--------", req->mz.size);
        for (int i = 0; i < req->mz.size; i++) {
            T_I("[%d]=%d", i, req->mz.bytes[i]);
        }
    }
    /* Analysis XYPoint req */
    {
        for (int i = 0; i < req->points_count; i++) {
            T_D("xy_point[%d].leftup_x: %d ", i, req->points[i].leftup_x);
            T_D("xy_point[%d].leftup_y: %d ", i, req->points[i].leftup_y);
            T_D("xy_point[%d].rightdown_x: %d ", i, req->points[i].rightdown_x);
            T_D("xy_point[%d].rightdown_y: %d ", i, req->points[i].rightdown_y);

        }

        memcpy(&g_test_t.motion_mz, &req->mz, sizeof(ipc_MotionzoneSet_Req_mz_t));
        g_test_t.motion_num_point = req->points_count;
        for (int i = 0; i < req->points_count; i++) {
            g_test_t.motion_xy_point[i].leftup_x = req->points[i].leftup_x;
            g_test_t.motion_xy_point[i].leftup_y = req->points[i].leftup_y;
            g_test_t.motion_xy_point[i].rightdown_x = req->points[i].rightdown_x;
            g_test_t.motion_xy_point[i].rightdown_y = req->points[i].rightdown_y;
        }
    }
#endif
    return retval;
}

bool dev_on_ipc_MotionzoneGet(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_MotionzoneGet_Req *req,
    /* output  */ ipc_MotionzoneGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
#if 0
    memcpy(&rsp->mz, &g_test_t.motion_mz, sizeof(ipc_MotionzoneGet_Resp_mz_t));
    rsp->points_count = g_test_t.motion_num_point;
    for (int i = 0; i < rsp->points_count; i++) {
        rsp->points[i].leftup_x = g_test_t.motion_xy_point[i].leftup_x;
        rsp->points[i].leftup_y = g_test_t.motion_xy_point[i].leftup_y;
        rsp->points[i].rightdown_x = g_test_t.motion_xy_point[i].rightdown_x;
        rsp->points[i].rightdown_y = g_test_t.motion_xy_point[i].rightdown_y;
    }
#endif

    return retval;
}

bool dev_on_ipc_PspAdd(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_PspAdd_Req *req,
    /* output  */ ipc_PspAdd_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
#if 0
    for (int index = 0; index < 5; index++) {
        T_D("g_test_t.pspinfo_arr[%d].psp_id: %d ", index, g_test_t.pspinfo_arr[index].psp_id);
        if (g_test_t.pspinfo_arr[index].psp_id == 0) {
            g_test_t.pspinfo_arr[index].psp_id = index + 1;
            memcpy(g_test_t.pspinfo_arr[index].psp_name, req->psp_name, 16);
            g_test_t.pspinfo_arr[index].is_def = req->is_def;
            break;
        }
    }
#endif

    return retval;
}

bool dev_on_ipc_PspDel(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_PspDel_Req *req,
    /* output  */ ipc_PspDel_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

#if 0
    for (int index = 0; index < 5; index++) {
        if (g_test_t.pspinfo_arr[index].psp_id == req->psp_id) {
            memset(&g_test_t.pspinfo_arr[index], 0, sizeof(ipc_PspList_PspInfo));
        }
    }
#endif

    return retval;
}

bool dev_on_ipc_PspList(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_PspList_Req *req,
    /* output  */ ipc_PspList_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

#if 0
    int num = 0;
    for (int index = 0; index < 5; index++) {
        T_D("g_test_t.pspinfo_arr[%d].psp_id_: %d ", index, g_test_t.pspinfo_arr[index].psp_id);
        if (g_test_t.pspinfo_arr[index].psp_id != 0) {
            memcpy(&rsp->psps[num], &g_test_t.pspinfo_arr[index], sizeof(ipc_PspList_PspInfo));
            num++;
        }
    }
    rsp->psps_count = num;
#endif
    T_D("resp_pspinfo_count: %d ", rsp->psps_count);

    return retval;
}

bool dev_on_ipc_PspCall(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_PspCall_Req *req,
    /* output  */ ipc_PspCall_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_I("dev_on_ipc_PspCall");
    T_I("psp_id=%d", req->psp_id);
    return retval;
}


//需要异步处理
bool dev_on_ipc_Reboot(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_Reboot_Req *req,
    /* output  */ ipc_Reboot_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("conn[%d].ipc.Reboot.channel:%d", conn_id, req->channel);

    #ifndef LONG_POWER_IPC
    post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_DEVICE_REBOOOT_CMD, 1);
    #else
    post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_DEVICE_REBOOOT_CMD, 1);

    #endif
    return retval;
}

//需要异步
bool dev_on_ipc_Reset(
    /* conn_id */ int32_t         conn_id,
    /* input   */ ipc_Reset_Req *req,
    /* output  */ ipc_Reset_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_I("conn[%d].ipc.Reset.channel:%d", conn_id, req->channel);



    post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_RESET_CMD, 2);

    return retval;
}

bool dev_on_ipc_FlipGet(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_FlipGet_Req *req,
    /* output  */ ipc_FlipGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

//    rsp->flip = g_test_t.flip;

     rsp->flip = db_select("flip");


     printf("\n rsp->flip========================%d\n  ",rsp->flip);
    return retval;
}

bool dev_on_ipc_LanAuth(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_LanAuth_Req *req,
    /* output  */ ipc_LanAuth_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    return retval;
}

bool dev_on_ipc_TimedcruiseSet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_TimedcruiseSet_Req *req,
    /* output  */ ipc_TimedcruiseSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_I("dev_on_ipc_TimedcruiseSet");
#if 0
    g_test_t.Timedcruise_states = req->states;
    g_test_t.Timedcruise_mode = req->mode;
    g_test_t.Timedcruise_interval = req->interval;
    memcpy(&g_test_t.Timedcruise_tt, &req->tt, sizeof(xciot_Timetask));
#endif
    return retval;
}

bool dev_on_ipc_TimedcruiseGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_TimedcruiseGet_Req *req,
    /* output  */ ipc_TimedcruiseGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("dev_on_ipc_TimedcruiseGet");
#if 0
    rsp->states = g_test_t.Timedcruise_states;
    rsp->mode = g_test_t.Timedcruise_mode;
    rsp->interval = g_test_t.Timedcruise_interval;
    memcpy(&rsp->tt, &g_test_t.Timedcruise_tt, sizeof(xciot_Timetask));
#endif

    return retval;
}

bool dev_on_ipc_StorageInfo(
    /* conn_id */ int32_t               conn_id,
    /* input   */ ipc_StorageInfo_Req *req,
    /* output  */ ipc_StorageInfo_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("------------dev_on_ipc_StorageInfo-----------");

    extern int storage_device_ready();
    if (!storage_device_ready()) {
        rsp->status = 4;
    } else {
        struct vfs_partition *part;
        part = fget_partition(CONFIG_ROOT_PATH);
        u32 cur_space;
        fget_free_space(CONFIG_ROOT_PATH, &cur_space);

        rsp->status = 0;
        rsp->total_size =  part->total_size / 1024;
        rsp->use_size = (part->total_size - cur_space) / 1024;

        T_D("status =%d", rsp->status);
        T_D("total_size =%lld", rsp->total_size);
        T_D("use_size =%lld", rsp->use_size);
    }



    return retval;
}


bool dev_on_ipc_StorageFormat(
    /* conn_id */ int32_t                 conn_id,
    /* input   */ ipc_StorageFormat_Req *req,
    /* output  */ ipc_StorageFormat_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("--------------dev_on_ipc_StorageFormat-------------------");

    if (!storage_device_ready()) {
        return false;
    }
    post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_SD_FORMAT, OSAL_UINT_TO_POINTER(conn_id));

    return retval;
}



bool dev_on_ipc_LogSet(
    /* conn_id */ int32_t          conn_id,
    /* input   */ ipc_LogSet_Req *req,
    /* output  */ ipc_LogSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("--------------dev_on_ipc_LogSet-------------------");

    T_D("channel =%s", req->channel);
    T_D("log_ipaddr =%d", req->states);

    return retval;
}



bool dev_on_ipc_VolumeSet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_VolumeSet_Req *req,
    /* output  */ ipc_VolumeSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

//	g_test_t.volume_value = req->volume;
    printf("\n >>>>>>>>>>>> req->volume  = %d\n", req->volume);
#ifdef LONG_POWER_IPC

    db_update("vol", req->volume);
    db_flush();
    #if  0
    if(!req->volume){


    }else{

     req->volume+=4;

    }

     u8 data[2];
    //设置音量
    data[0] = 0x80;//音量
    data[1] = req->volume;

    printf("\n req->volume+4 ===============%d\n",req->volume);
    cdc_write_data(data,sizeof(data));

    #else
    int convert_dev_vol=mapAppVolumeToDeviceVolume(req->volume);
    u8 data[2];
    //设置音量
    data[0] = 0x80;//音量
    data[1] = convert_dev_vol;

    printf("\n convert_dev_vol ===============%d\n",convert_dev_vol);
    cdc_write_data(data,sizeof(data));

    #endif

#else
    db_update("vol", req->volume);
    db_flush();
#endif

    return retval;
}

bool dev_on_ipc_PowerFreqSet(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_PowerFreqSet_Req *req,
    /* output  */ ipc_PowerFreqSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("----------dev_on_ipc_PowerFreqSet-----------");
    T_D("power freq = %d", req->power_freq);

    db_update("fre", req->power_freq);
    db_flush();

    return retval;
}

bool dev_on_ipc_PowerFreqGet(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_PowerFreqGet_Req *req,
    /* output  */ ipc_PowerFreqGet_Resp *rsp)
{
    T_D("----------dev_on_ipc_PowerFreqGet-----------");


    /* retval - true: support, false: unsupport */
    bool retval = true;

    rsp->power_freq = db_select("fre");
    T_D("power freq = %d", rsp->power_freq);
    return retval;
}

bool dev_on_ipc_VolumeGet(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_VolumeGet_Req *req,
    /* output  */ ipc_VolumeGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("----------dev_on_ipc_VolumeGet-----------");
    rsp->volume = db_select("vol");

    printf("\n >>>>>>>>>>>> rsp->volume  = %d\n", rsp->volume);
    return retval;
}

bool dev_on_ipc_AlarmSet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_AlarmSet_Req *req,
    /* output  */ ipc_AlarmSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
#if 0
    g_test_t.motion_detection = req->motion;
    g_test_t.opensound_detection = req->sound;
    g_test_t.smoke_detection = req->smoke;
    g_test_t.shadow_detection = req->shadow;
#endif

    return retval;
}

bool dev_on_ipc_AlarmGet(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_AlarmGet_Req *req,
    /* output  */ ipc_AlarmGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("dev_on_ipc_AlarmGet");
#if 0
    rsp->motion = g_test_t.motion_detection;
    rsp->sound = g_test_t.opensound_detection;
    rsp->smoke = g_test_t.smoke_detection;
    rsp->shadow = g_test_t.shadow_detection;
#endif

    return retval;
}

//背景图上报
int report_topic_cmmid(int64_t  pid){


 //  int avsdk_firmware_rate_yun_217(int ch, int rate, int error)
//{
    T_D("-------report_topic_cmmid--------------");
    int rc = 0;
    uint64_t utcms = osal_clock_utctime();
    iot_PpiotCmd_t *cmd = (iot_PpiotCmd_t *)malloc(sizeof(iot_PpiotCmd_t));
    if(!cmd) {
        T_E("malloc error!");
        return -1;
    }
    memset(cmd,0,sizeof(iot_PpiotCmd_t));
    cmd->cmds_count = 1;
    cmd->seq = 0;
    cmd->ans = false;

    /* 每个命令可能不一样 */
    cmd->cmds[0].cid = 3122;

    /* int 类型参数的个数 */
    cmd->cmds[0].pint_count = 2;
    cmd->cmds[0].pint[0] = 0; //通道编号;0 表示设备自身
    cmd->cmds[0].pint[1] = pid;//图片序号
   // cmd->cmds[0].pint[2] = 0;//rateType=2, 命令ID； 否则为0；
   // cmd->cmds[0].pint[3] = rate;//进度百分比: 1-100; 101: 错误
   // if(rate == 101) {
     //   cmd->cmds[0].pint[4] = error;//rate=101: 错误代码; 不同设备的命令错误代码定义参见错误代码定义;
   // }
    cmd->cmds[0].pint[5] = get_utc_ms();

    //发送到平台
    rc = avsdk_send_iotcmd("/xc/2p/cmdans", cmd);

    free(cmd);

    printf("\n pid===========%llu\n",pid);

}



bool dev_on_ipc_Screenshot(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_Screenshot_Req *req,
    /* output  */ ipc_Screenshot_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("dev_on_ipc_Screenshot");
    T_D("sizeof(rsp->body.bytes) = %d", sizeof(rsp->body.bytes));


    printf("\ntime1 =  %d\n", timer_get_ms());
    u32 buffer_len = doorbell_video_rec_take_photo(0, rsp->body.bytes, sizeof(rsp->body.bytes));
    printf("\ntime2 =  %d\n", timer_get_ms());
    rsp->format = 2; // 1 png; 2jpg
    rsp->body.size = buffer_len;


    return retval;
}
#if 1
typedef __uint8_t uint8 ;
extern int montor_timer_hander;
extern   void step_motor_drive(uint8 dire,uint8 speed,int angle,u8 point_step);
extern void step_motor_lf_stop(void);
extern void step_motor_ud_stop(void);
#include "gpio.h"

#define SFR_SET(sfr, start, len, dat) (sfr = sfr & ~((~(0xffffffff << len)) << start) | ((dat & (~(0xffffffff << len))) << start))
#endif
bool dev_on_ipc_PtzCtrl(
    /* conn_id */ int32_t           conn_id,
    /* input   */ ipc_PtzCtrl_Req *req,
    /* output  */ ipc_PtzCtrl_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    printf("\n ==========page=========%d,%s\n",__LINE__,__FUNCTION__);
    T_D("req_func_code: %d ", req->func_code);
    T_D("req_para1: %d ", req->para1);
    T_D("req_para2: %d ", req->para2);
#ifdef  LP_CONFIG_BOARD_DEV_WIFI_5713_20240426
   switch(req->func_code){

    case 11:
    case 12:

     printf("\n  RIGHT_LEFT_OPEN\n");
   #if 0
    SFR_SET(DAA_CON0,2,1,0);
   // DAA_CON0&=~BIT(2);
    gpio_direction_output(IO_PORTH_06,0);
   #else


    montor_control_gpio(0);//选择上下转动
    #endif
    break ;


    case 13:
    case 14:

      #if 0

    printf("\n  UP_DOWN_OPEN\n");
     SFR_SET(DAA_CON0,2,1,0);
   //  DAA_CON0&=~BIT(2);
     gpio_direction_output(IO_PORTH_06,1);
     #else


    montor_control_gpio(1);//选择左右转动
    #endif

    break ;

    default:
     break;

    }

#endif
#if  1 // 快速响应停止
    if(req->func_code==10){

    if(montor_timer_hander){
    printf("\n delete time \n");
    sys_timer_del(montor_timer_hander);
    montor_timer_hander=0;
    }
   // step_motor_drive(3,2,10,1);
    step_motor_lf_stop();
    step_motor_ud_stop();


    }
#endif
    post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_MONTOR_CONTROL_CONFIG, req->func_code);


    return retval;
}

bool dev_on_ipc_SetAutoTrack(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_SetAutoTrack_Req  *req,
    /* output  */ ipc_SetAutoTrack_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("dev_on_ipc_SetAutoTrack");
    T_D("autoTrack: %d ", req->autoTrack);
#if 0
    g_test_t.autotrack = req->autoTrack;
#endif
    rsp->respCode = 0;

    return retval;
}

bool dev_on_ipc_GetAutoTrack(
    /* conn_id */ int32_t                conn_id,
    /* input   */ ipc_GetAutoTrack_Req  *req,
    /* output  */ ipc_GetAutoTrack_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("dev_on_ipc_GetAutoTrack");
#if 0
    T_D("autoTrack: %d ", g_test_t.autotrack);
    rsp->autoTrack = g_test_t.autotrack;
#endif
    return retval;
}

void avsdk_firmware_rate_callback(void *arg, int index)
{
    int32_t  conn_id = OSAL_POINTER_TO_UINT(arg);
    avsdk_firmware_rate(conn_id, 0, index);
}


void firmware_notify_thread(void *arg)
{

    ipc_FirmwareCheckByDevice_Resp rsp;
    avsdk_firmware_poll(&rsp);
    printf("\n rsp.is_update = %d     ,"
           "\n rsp.firmware_ver = %s  ,"
           "\n rsp.firmware_url = %s  ,"
           "\n rsp.firmware_type =  %d,"
           "\n rsp.firmware_size  %d   "
           ,
           rsp.is_update,
           rsp.firmware_ver,
           rsp.firmware_url,
           rsp.firmware_type,
           rsp.firmware_size);
    if (rsp.is_update == 1) {
        extern void cloud_upgrade_firmware(void *rsp, int notify, void *arg);
        cloud_upgrade_firmware(&rsp, 1, arg);
        // return;
    }
}

bool dev_on_ipc_FirmwareNotify(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_FirmwareNotify_Req  *req,
    /* output  */ ipc_FirmwareNotify_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    /** TODO:异步操作
     * 1.调用avsdk_firmware_poll查询固件信息和url。
     * 2.使用http下载固件。
     * 3.调用avsdk_firmware_rate上报升级进度
     * 4.更新固件重启。
     */
    thread_fork("firmware_notify_thread", 18, 0x1000, 0, 0, firmware_notify_thread, OSAL_UINT_TO_POINTER(conn_id));
    rsp->rate = 0;
    return retval;
}


bool dev_on_ipc_EventRecordGet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_EventRecordGet_Req *req,
    /* output  */ ipc_EventRecordGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_EventRecordGet------------------");
   // rsp->duration = db_select("cyc");
    rsp->duration = db_select("mot");
    printf("\nrsp->duration = %d \n", rsp->duration);

    return retval;
}



bool dev_on_ipc_EventRecordSet(
    /* conn_id */ int32_t                  conn_id,
    /* input   */ ipc_EventRecordSet_Req *req,
    /* output  */ ipc_EventRecordSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_EventRecordSet------------------");
    printf("\nrsp->duration = %d \n", req->duration);

    post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_CHANGE_CYC_TIME, req->duration);

    return retval;
}

bool dev_on_ipc_GetNetworkInfo(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_GetNetworkInfo_Req *req,
    /* output  */ ipc_GetNetworkInfo_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("---------------dev_on_ipc_GetNetworkInfo------------------");

    struct NETWORK_INFO *network_info = sys_get_network_info();
    rsp->qos = system_get_network_info_qos();
    snprintf(rsp->ssid, sizeof(rsp->ssid), "%s", network_info->ssid);
    snprintf(rsp->ipaddr, sizeof(rsp->ipaddr), "%s",  network_info->ip);
    snprintf(rsp->netmask, sizeof(rsp->netmask), "%s",  network_info->nm);
    snprintf(rsp->gateway, sizeof(rsp->gateway), "%s",  network_info->gw);
    snprintf(rsp->dns1, sizeof(rsp->dns1), "%s",  network_info->dns1);
    snprintf(rsp->dns2, sizeof(rsp->dns2), "%s",  network_info->dns2);
    snprintf(rsp->mac, sizeof(rsp->mac), "%s",  network_info->mac);

    T_D("channel: %d ", req->channel);
    T_D("ssid: %s ", rsp->ssid);
    T_D("qos: %d ", rsp->qos);
    T_D("ipaddr: %s ", rsp->ipaddr);
    T_D("netmask: %s ", rsp->netmask);
    T_D("gateway: %s ", rsp->gateway);
    T_D("dns1: %s ", rsp->dns1);
    T_D("dns2: %s ", rsp->dns2);
    T_D("mac: %s ", rsp->mac);


    return retval;
}



bool dev_on_ipc_FileStart(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_FileStart_Req *req,
    /* output  */ ipc_FileStart_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_FileStart------------------");

    T_D("req_func_code: %lld ", req->file_id);
    T_D("req_para1: %lld ", req->length);
    T_D("req_para2: %lld ", req->offset);

    return retval;
}

bool dev_on_ipc_FileStop(
    /* conn_id */ int32_t            conn_id,
    /* input   */ ipc_FileStop_Req *req,
    /* output  */ ipc_FileStop_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("---------------dev_on_ipc_FileStop------------------");
    T_D("req_func_code: %lld ", req->file_id);

    return retval;
}

bool dev_on_ipc_CustomCmd(
    /* conn_id */ int32_t             conn_id,
    /* input   */ ipc_CustomCmd_Req *req,
    /* output  */ ipc_CustomCmd_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    static char time = 0;
    avsdk_custom_add_conn(conn_id);
    T_D("req_byte: len = %d", req->arg_int32_count);

    for (char i = 0; i < req->arg_int32_count; i++) {
        T_D("req_int [%d]=%d", i, req->arg_int32[i]);
    }

    T_D("req_byte: len = %d", req->arg_bytes.size);
    for (int i = 0; i < req->arg_bytes.size; i++) {
        T_D("req_byte[%d] = %02x", i, req->arg_bytes.bytes[i]);
    }

    for (int index = 0; index < req->arg_string_count; index++) {
        T_D("req_str_[%d]: %s, len = %d", index, req->arg_string[index], strlen(req->arg_string[index]));

    }


    //if 收到什么，将connid加入custom用户组
    //avsdk_custom_add_conn(conn_id);
    //if 收到什么，将connid从custom用户组删除
    //avsdk_custom_del_conn(conn_id);

    return retval;
}

bool dev_on_ipc_PirSet(
    /* conn_id */ int32_t			 conn_id,
    /* input   */ ipc_PirSet_Req *req,
    /* output  */ ipc_PirSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_PirSet------------------");

    T_D("req->pir.num: %d ", req->pir.num);
    T_D("req->pir.level: %d ", req->pir.level);

    extern void set_pir_state(int state);
    set_pir_state(req->pir.level);
    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_SET_PIR);
    return retval;
}


bool dev_on_ipc_PirGet(
    /* conn_id */ int32_t			 conn_id,
    /* input   */ ipc_PirGet_Req *req,
    /* output  */ ipc_PirGet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_PirGet------------------");


    rsp->pirs_count = 1;
    rsp->pirs[0].num = 1;
    extern int get_pir_state();
    rsp->pirs[0].level = get_pir_state();
    T_D("rsp->pirs_count: %d ", rsp->pirs_count);
    T_D("rsp->pirs[0].num: %d ", rsp->pirs[0].num);
    T_D("rsp->pirs[0].level: %d ", rsp->pirs[0].level);



    return retval;
}

bool dev_on_ipc_ChanState(
    /* conn_id */ int32_t			  conn_id,
    /* input   */ ipc_ChanState_Req  *req,
    /* output  */ ipc_ChanState_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_ChanState------------------");



    rsp->chans_count = 1;
    rsp->chans[0].channel = 0;
    rsp->chans[0].state = 1;

    return retval;
}

bool dev_on_ipc_VideoChanChange(
    /* conn_id */ int32_t			 conn_id,
    /* input   */ ipc_VideoChanChange_Req *req,
    /* output  */ ipc_VideoChanChange_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------dev_on_ipc_VideoChanChange------------------");

    T_D("rsp->channel_count: %d ", req->channel_count);
    avsdk_video_del_conn(conn_id, -1);

    for (int i = 0; i < req->channel_count; i++) {
        T_D("rsp->channel[%d]: %d ", i, req->channel[i]);
        avsdk_video_add_conn(conn_id, req->channel[i]);
    }

    rsp->render_count = 1;
    rsp->render[0] = 5;

    return retval;
}

//所有的发送api都不能在回调里面调用
static int tamper_set_thread_pid;
void tamper_set_thread(void *priv)
{
    int state = (u32) priv;
    extern void set_tamper_state(int state);
    set_tamper_state(state);
    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_SET_TAMPER);
    if (state) {
        avsdk_report_tamper_status(E_IOT_TAMPER_ON);
    } else {
        avsdk_report_tamper_status(E_IOT_TAMPER_OFF);
    }
}

//强拆报警
bool dev_on_ipc_TamperSet(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_TamperSet_Req *req,
    /* output  */ ipc_TamperSet_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    //参考iot_tamper_status_t
    T_D("TamperSet state: %lld", req->state);

    int state = req->state;


    thread_kill(&tamper_set_thread_pid, 0);

    thread_fork("tamper_set_thread", 8, 0x1000, 0, &tamper_set_thread_pid, tamper_set_thread, (void *)state);

    return retval;
}


bool dev_on_ipc_DirCreate(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_DirCreate_Req *req,
    /* output  */ ipc_DirCreate_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------%s------------------", OSAL_STRFUNC);
    T_D("dir_name:%s", req->dir_name);
    T_D("dir_name:%s", req->dir_path);
    return retval;
}

bool dev_on_ipc_DirDel(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_DirDel_Req *req,
    /* output  */ ipc_DirDel_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("---------------%s------------------", OSAL_STRFUNC);

    T_D("dir_name:%s", req->dir_name);
    T_D("dir_name:%s", req->dir_path);

    return retval;
}

bool dev_on_ipc_DirEdit(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_DirEdit_Req *req,
    /* output  */ ipc_DirEdit_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("---------------%s------------------", OSAL_STRFUNC);


    T_D("dir_name:%s", req->dir_path);
    T_D("old_dir_name:%s", req->old_dir_name);
    T_D("new_dir_name:%s", req->new_dir_name);

    return retval;
}

bool dev_on_ipc_DirList(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_DirList_Req *req,
    /* output  */ ipc_DirList_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_D("---------------%s------------------", OSAL_STRFUNC);
    T_D("dir_name:%s", req->dir_path);
    T_D("page:%d", req->page);
    T_D("page_size:%d", req->page_size);

    return retval;
}

#if 1 //文件传输

typedef struct {
    int32_t conn_id;
    bool tosend;
    bool recv_end;
    bool init;
    uint64_t total_size;
    uint64_t last_offset;
    ipc_FileRecvRate_Req    req;

} frr_ctx_t;


frr_ctx_t frr;

bool dev_on_ipc_FileAdd(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileAdd_Req *req,
    /* output  */ ipc_FileAdd_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_D("---------------%s------------------", OSAL_STRFUNC);

    T_I("req->dir_path:%s", req->dir_path);

    T_I("req->file_name:%s", req->file_name);
    T_I("req->file_id:%lld", req->file_id);
    T_I("req->total_size:%lld", req->total_size);

    frr.total_size = req->total_size;

    return retval;
}


void *report_recvfile_thread(void *arg)
{

    frr_ctx_t *frrp = &frr;
    int32_t  conn_id;
    int rc = 0;
    ipc_FileRecvRate_Req    req;
    ipc_FileRecvRate_Resp   rsp;
#if 0
    while (1) {

        /* diff > 40K */
        if (!frrp->tosend && (frrp->req.recv_size != 0) && (frrp->req.recv_size - frrp->last_offset > 40000)) {
            T_I("set tosend =1, (receive,last)  (%lld-%lld)", frrp->req.recv_size, frrp->last_offset);
            frrp->tosend = true;
        }

        /* 判定是不是文件结束 */
        if (!frrp->tosend && (frrp->total_size == frrp->req.recv_size)) {
            if (frrp->last_offset != frrp->total_size) {

                frrp->tosend = true;
                T_I("receive file end %lld", req.recv_size);
            } else {
                /* 已经发送了最后一包 */
                frr.last_offset = 0;
                frr.recv_end = false;
                frr.tosend = false;
                frr.total_size = 0;
                frr.conn_id = -1;
                memset(&frr.req, 0, sizeof(ipc_FileRecvRate_Req));
                memset(&req, 0, sizeof(ipc_FileRecvRate_Req));
            }

        }

        if (frrp->tosend) {
            if (frrp->req.recv_size - frrp->last_offset == 0) {
                frrp->tosend = false;
                continue ;
            }

            if (frrp->conn_id < 0) {
                continue;
            }
            conn_id = frrp->conn_id;
            memcpy(&req, &frrp->req, sizeof(req));
            rc = avsdk_report_FileRecvRate(conn_id,
                                           &req,
                                           &rsp);
            if (rc != 0 && rc != 9 && rc != 5) {
                T_E("error: avsdk_report_FileRecvRate, rc=%d", rc);
            } else {
                T_W("report diff = %lld", (req.recv_size - frrp->last_offset));

                frrp->last_offset = req.recv_size;

                T_W("report_recvfile filed == %lld,(total-recv-last) (%lld-%lld-%lld)",
                    frrp->req.file_id,
                    frrp->total_size,
                    req.recv_size,
                    frrp->last_offset);

                frrp->tosend = false;

            }
        } else {
            usleep(10);
        }
    }
#endif

    return NULL;
}


void dev_on_RecvFile(int32_t            conn_id,
                     ipc_RecvFile_Req   *req,
                     ipc_RecvFile_Resp  *rsp)
{
    if (req->offset % 100 * 1024 == 0) {
        T_I("dev_on_RecvFile");
        T_D("conn_id=%d", conn_id);
        T_I("file_id=%"PRIi64, req->file_id);
        T_I("offset=%"PRIi64, req->offset);
        T_D("payload_len=%"PRIi64, req->payload_len);
        T_T("msg:%s", req->payload);
    }

    frr.req.file_id = req->file_id;
    frr.req.recv_size = req->offset + req->payload_len;
    frr.conn_id = conn_id;
#if 0
    if (!frr.init) {
        pthread_t frr_tidp;
        pthread_create(&frr_tidp, NULL, report_recvfile_thread, (void *)&frr);
        frr.init = true;
    }
#endif
    rsp->recode = 0;
}

#endif //文件传输demo

bool dev_on_ipc_FileDel(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileDel_Req *req,
    /* output  */ ipc_FileDel_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_D("---------------%s------------------", OSAL_STRFUNC);

    return retval;
}

bool dev_on_ipc_FileEdit(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileEdit_Req *req,
    /* output  */ ipc_FileEdit_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_D("---------------%s------------------", OSAL_STRFUNC);

    return retval;
}

bool dev_on_ipc_FileRecvRate(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileRecvRate_Req *req,
    /* output  */ ipc_FileRecvRate_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_D("---------------%s------------------", OSAL_STRFUNC);

    return retval;
}

bool dev_on_ipc_FileThumList(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_FileThumList_Req *req,
    /* output  */ ipc_FileThumList_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;


    T_D("---------------%s------------------", OSAL_STRFUNC);

    return retval;
}



bool dev_on_ipc_ExecIOTCMD(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_ExecIOTCMD_Req *req,
    /* output  */ ipc_ExecIOTCMD_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    /* 注意：
     * req中iot_cmds与ttcmd为互斥，每次命令只会有其中一个赋值，
     * 可以通过cmds_count是否为0，来判定命令为iot_cmds还是ttcmd。
     */
    T_D("---------------%s------------------", OSAL_STRFUNC); //dev_on_ipc_ExecIOTCMD
  //  printf("\n %s,---------------%s------------------",__FUNCTION__, OSAL_STRFUNC);
    if (req->iot_cmds.cmds_count) {
        /* 打印iot_cmds所有的参数 */
        avsdk_dump_PpiotCmd("req->iot_cmds", &req->iot_cmds);
        for (int i = 0; i < req->iot_cmds.cmds_count; i++) {
            if (req->iot_cmds.cmds[i].cid == 590) {
//                iot_PpiotCmd_t *ppiot = &req->iot_cmds;
                doorbell_video_rec_get_iframe(CONFIG_NET_SUB_ID);
            }

           if (req->iot_cmds.cmds[i].cid == 3122) {

        //    printf("\n timestamp:%llu,req->cmds[0].pstr[0] : %s\n req->cmds[0].pstr[1] :%s\n ",req->iot_cmds.timestamp,req->iot_cmds.cmds[0].pstr[0],req->iot_cmds.cmds[0].pstr[1]);
        //    printf("\n req->iot_cmds.cmds[0].pint[2]:%llu,%llu,: %s\n req->cmds[0].pint[3] :%s\n ",req->iot_cmds.cmds[0].pint[2],req->iot_cmds.cmds[0].pint[3]);

          //post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_DEVICE_SCREEN_CMD, req->iot_cmds.cmds[0].pint[1],req->iot_cmds.cmds[0].pstr[0],req->iot_cmds.cmds[0].pstr[1]);
            post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_DEVICE_SCREEN_CMD, req);

            switch(req->iot_cmds.cmds[0].pint[1] ){

             case 1:

             break ;



            }
            rsp->iot_cmds.ans =true;
            req->iot_cmds.ans =true;
            return retval;
            }


        }

    } else {
        //todo:无效命令
        printf("\n %s %d\n", __func__, __LINE__);
    }


    return retval;
}


bool dev_on_ipc_SetLowPower(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_SetLowPower_Req *req,
    /* output  */ ipc_SetLowPower_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------%s------------------", OSAL_STRFUNC);
    db_update("lowpower", req->state);
    db_flush();
    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_SET_POWER_MODE);
    return retval;
}

bool dev_on_ipc_GetLowPower(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_GetLowPower_Req *req,
    /* output  */ ipc_GetLowPower_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
    T_D("---------------%s------------------", OSAL_STRFUNC);
    rsp->state = db_select("lowpower");
    return retval;
}

bool dev_on_ipc_ForceIFrame(
    /* conn_id */ int32_t conn_id,
    /* input   */ ipc_ForceIFrame_Req *req,
    /* output  */ ipc_ForceIFrame_Resp *rsp)
{
    T_D("---------------%s------------------", OSAL_STRFUNC);


    /* retval - true: support, false: unsupport */
    bool retval = true;
    doorbell_video_rec_get_iframe(CONFIG_NET_SUB_ID);
    return retval;
}






void dev_on_ipc_media(int32_t              conn_id,
                           iot_exhdr_media_t    *exhdr,
                           const uint8_t        *media_data,
                           size_t               media_len)
{
#define PCM_TYPE_AUDIO      1
#define JPEG_TYPE_VIDEO     2
#define H264_TYPE_VIDEO     3
  //  T_I("---------Speak Date[%d] avformat = %d\n", media_len,exhdr->avformat.streamtype);



 //   printf("---------Speak Date[%d] avformat = %d\n", media_len,exhdr->avformat.streamtype);


    uint64_t get_utc_ms(void);

    // printf("\nexhdr->timestamp = %llu  get_utc_time() = %llu\n",exhdr->timestamp ,get_utc_ms());
    if (exhdr->avformat.streamtype == E_IOT_MEDIASTREAM_TYPE_PCM) {
        extern void net_audio_wrete_frame(int type, u32 len, u32 timestamp, u8 is_key_frame, char *buf);
        net_audio_wrete_frame(PCM_TYPE_AUDIO, media_len, exhdr->timestamp, exhdr->avformat.iskeyframe, media_data);
    } else if (exhdr->avformat.streamtype == E_IOT_MEDIASTREAM_TYPE_AAC) {
        extern void net_audio_wrete_frame(int type, u32 len, u32 timestamp, u8 is_key_frame, char *buf);
        net_audio_wrete_frame(PCM_TYPE_AUDIO, media_len, exhdr->timestamp, exhdr->avformat.iskeyframe, media_data);
    } else if (exhdr->avformat.streamtype == E_IOT_MEDIASTREAM_TYPE_H264) {
        extern void net_video_wrete_frame(int type, u32 len, u32 timestamp, u8 is_key_frame, char *buf);
        net_video_wrete_frame(H264_TYPE_VIDEO, media_len, exhdr->timestamp, exhdr->avformat.iskeyframe, media_data);
    } else if (exhdr->avformat.streamtype == E_IOT_MEDIASTREAM_TYPE_MJPEG) {
        extern void net_video_wrete_frame(int type, u32 len, u32 timestamp, u8 is_key_frame, char *buf);

    #if  0
    static void *tfile=NULL;
    if(!tfile)
      {
        tfile = fopen(CONFIG_STORAGE_PATH"/C/test_****.jpg", "w+");
        if(tfile){


          int len3 = fwrite(tfile,media_data, media_len);
          printf("\n len3:%d\n",len3);
          fclose(tfile);
          tfile=NULL;
        }

      }
    #endif

        net_video_wrete_frame(JPEG_TYPE_VIDEO, media_len, exhdr->timestamp, 1, media_data);
    }


}


#if 1 //留言功能

//获事件列表
int user_alert_get_list(void)
{
    int rc = 0;
    alert_list_req_t req = {0};
    alert_list_resp_t rsp = {0};

    req.event_id = E_IOT_EVENT_LEAVE_MESSAGE_APP;
    req.page = 1;
    req.page_size = 5;//最大5
    rc = avsdk_alert_get_list(&req, &rsp);
    if(rc) {
        T_I("avsdk_alert_get_list, rc=%d", rc);
    }
    T_I("total=%lld, count=%d", rsp.total, rsp.alerts_count);
    for(int i=0; i<rsp.alerts_count; i++) {
        T_I("[%d]:is_read=%d, clouds_size=%d, uid=%s, name=%s, time=%lld, url=%s",
            i,
            rsp.alerts[i].is_read,
            rsp.alerts[i].clouds_size,
            rsp.alerts[i].pstr[0],
            rsp.alerts[i].pstr[1],
            rsp.alerts[i].timestamp,
            rsp.alerts[i].down_url);//下载留言文件的url
    }
    return rc;
}

int user_alert_set_read(int64_t utcms)
{
    int rc = 0;
    T_I("input utcms=%lld", utcms);
    alert_arr_t tmp = {0};
    tmp.count = 1;
    tmp.utcms[0] = utcms;
    rc = avsdk_alert_set_read(&tmp);
    if(rc) {
        T_I("avsdk_alert_set_read, rc=%d", rc);
    }
    return rc;
}

void dev_on_LeaveMessage(void)
{
    printf("\n >>>>>>>>%s%d\n",__func__,__LINE__);
    //收到新的留言消息
    //todo
    //1.调用user_alert_get_list(avsdk_alert_get_list)查询云端给设备下发的留言信息以及下载地址
    //2.调用user_alert_set_read(avsdk_alert_set_read)将消息设为已读。
    //可以参考上面的user_alert_get_list/user_alert_set_read
    //user_alert_get_list();

}
#endif

bool dev_on_StateChangeByApp(
    /* conn_id */ int32_t                    conn_id,
    /* input   */ ipc_StateChangeByApp_Req  *req,
    /* output  */ ipc_StateChangeByApp_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;

    T_I("---------------%s------------------", OSAL_STRFUNC);
    T_I("key=%lld, value=%lld", req->key, req->value);


    if(req->key == 83){
        if(req->value == 1){
            post_msg_doorbell_task("video_dec_task", 1, APP_ANSWER_STATE);

        }else{
            post_msg_doorbell_task("video_dec_task", 1, APP_REFUSE_STATE);
        }
    }

    if(req->key == 2000 || req->key == 2001){
        int avsdk_get_user_list(int32_t user_type, user_list_t *p);

        printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);
        avsdk_get_user_list(user_type,&p);
        printf("\n\n p.users_count = %d\n\n",p.users_count );
        for(int i = 0;i< p.users_count;i++){

            printf("\n\n p.users[%d].uid = %s\n\n",i,p.users[i].uid );
            printf("\n\n p.users[%d].name = %s\n\n",i,p.users[i].name );
            put_buf(p.users[i].name,sizeof(p.users[i].name));
            printf("\n\n p.users[%d].uid = %d\n\n",i,p.users[i].user_type );
        }
    }




    return retval;
}


bool dev_on_ipc_VideoCall(int32_t conn_id,
                            ipc_VideoCall_Req *req,
                            ipc_VideoCall_Resp *rsp
                          )
{
  bool retval = true;
  T_D("---------------%s------------------", OSAL_STRFUNC);

  int64_t action = req->action;
  printf("\n >>>>>>>>> action == %lld\n",  action);


  rsp->av_format = E_IOT_MEDIASTREAM_TYPE_MJPEG;

  RT_TALK_INFO  tmp = {0};
  rsp->ans_code = find_device_video_play_info(&tmp) || get_video_rec_in_state() == REC_STA_IN_MSG ||  is_video_dec_mode();

  printf("\n >>>>>>>>>>>>>>>>>>>>>>>> rsp->ans_code  =%d\n",rsp->ans_code );

//  if(get_video_rec_in_state() != REC_STA_IN_MSG){
    printf("\n /***** get_video_rec_in_state != tmp");
    json_object *root = json_object_new_array();
#if 1
#include "asm\lcd_config.h"
    int width = LCD_DEV_WIDTH / 16 * 16;
    int Hight = LCD_DEV_HIGHT / 16 * 16;
    char buf[64];
    snprintf(buf,sizeof(buf),"%dx%d",width,Hight);
    json_object_array_add(root, json_object_new_string(buf));
#else
    json_object_array_add(root, json_object_new_string("320x480"));
#endif // 0
    printf("\n root = %s\n", json_object_get_string(root));

    snprintf(rsp->pixels,sizeof(rsp->pixels),"%s",json_object_get_string(root));
    json_object_put(root);
//  }


  //如果回复码为0
  if(!rsp->ans_code && action == 1){

     add_device_video_play_info(conn_id,req->ch);
     post_msg_doorbell_task("video_dec_task", 1, APP_START_VIDEO_CALL_STATE);
    void usb_speak_switch(void *state);
    usb_speak_switch("on");
  }else if(rsp->ans_code && action == 2){
    //如果回复码为1
    void usb_speak_switch(void *state);
     usb_speak_switch("off");
     post_msg_doorbell_task("video_dec_task", 3, APP_STOP_VIDEO_CALL_STATE,APP_REFUSE_STATE,conn_id);
     del_device_video_play_info(conn_id);
  }




  return retval;
}


bool dev_on_ipc_PauseAllAv(
    /* conn_id */ int32_t              conn_id,
    /* input   */ ipc_PauseAllAv_Req  *req,
    /* output  */ ipc_PauseAllAv_Resp *rsp)
{
    /* retval - true: support, false: unsupport */
    bool retval = true;
     printf("\n\n\n\n---------------%s------------------\n\n\n\n", __func__);
    //暂停所有的音视频播放，包括视频对讲

    /*将连接ID从视频播放用户组删除*/
    avsdk_video_del_conn(conn_id, req->ch);
    /*将连接ID从音频播放用户组删除*/
    avsdk_audio_del_conn(conn_id);

 //   void usb_speak_switch(u8 state);
 //   usb_speak_switch(0);


    //todo 退出视频对讲
    post_msg_doorbell_task("video_dec_task", 3, APP_STOP_VIDEO_CALL_STATE,APP_REFUSE_STATE,conn_id);


    //todo 退出tf回放

    //0, 正常响应; 大于0失败;
	rsp->code = 0;

    return retval;
}

#endif

/*****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
