#define	_STDIO_H_
#include "system/includes.h"
#include "os/os_api.h"
#include "server/rt_stream_pkg.h"
#include "xciot_api.h"
#include "ut_dev_ipc_cmd.h"
#include "xc_cs_type.h"
#include "system/timer.h"
#include "generic/lbuf.h"
#include "os/os_compat.h"
#include "doorbell_event.h"
#include "generic/list.h"

#include "action.h"

#define CLOUD_STORAGE_DEBUG

/***注意：
**运存接口不支持多线程操作
**/

typedef struct  {
    struct list_head entry;
    int wakeup_status;
    int uploading;
    u8 isCs;
    user_info_t *user;
    user_info_t  user_info;
} WAKEUP_STATUS_LIST_INFO;




typedef struct cloud_media_info {
    struct list_head entry;
    int type;
    u32 len;
    u32 difftime;
    u8 is_key_frame;
    u8 ch;
    u8 data[0];
} CLOUD_MEDIA_INFO;

static LIST_HEAD(wakeup_status_list_head);
static OS_MUTEX list_head_mutex;


static OS_SEM cloud_storage_sem;
static int cloud_storage_timer_id;

static old_wakeup_status = INIT_WAKEUP;
static u8 cloud_storage_timer_cnt;
static u8 cloud_storage_timeout_cnt;
static u8 cloud_storage_up_state = CLOUD_STORAGE_IDLE;
static int64_t start_time = 0;
static cs_hdl_ref_t cs_hdl = NULL;
static int event_id;

static OS_MUTEX cloud_storage_mutex; //用于互斥云存状态的互斥量


struct list_head cloud_media_info_list_head[CONFIG_CLOUD_STORAGE_NUM];
static OS_MUTEX cloud_media_info_list_mutex[CONFIG_CLOUD_STORAGE_NUM];

static int cloud_storage_fps_cnt[CONFIG_CLOUD_STORAGE_NUM];    //用于控制两路云存的帧率统计
static u8 cloud_storage_first_frame[CONFIG_CLOUD_STORAGE_NUM];//用于判断当前这一路的云存第一帧是否为I帧
static u8 is_wait_iframe[CONFIG_CLOUD_STORAGE_NUM];           //用于等I帧的标志位
static u8 audio_frame_start[CONFIG_CLOUD_STORAGE_NUM];        //用于当前路云存是否开始缓存音频
static int video_frame_count[CONFIG_CLOUD_STORAGE_NUM];       //用于判断当前路的云存有多少视频帧

static int first_I_timestamp[CONFIG_CLOUD_STORAGE_NUM];       //用于判断当前路的云存有多少视频帧
static WAKEUP_STATUS_LIST_INFO cur_wakeup_info;
static WAKEUP_STATUS_LIST_INFO  wakeup_status_backup_info[12];



#define CLOUD_STORAGE_TIME_CNT    120
#if SDRAM_SIZE == (32 * 1024 * 1024)
#define AVSDK_MEM_SIZE            (1 * 1024 * 1024)
#else
#define AVSDK_MEM_SIZE            (1 * 1024 * 1024)
#endif // SDRAM_SIZE


static WAKEUP_STATUS_LIST_INFO *find_wakeup_status_backup_info(void)
{
    int i;
    WAKEUP_STATUS_LIST_INFO *info = NULL;

    os_mutex_pend(&list_head_mutex, 0);
    for (i = 0; i < sizeof(wakeup_status_backup_info); i++) {
        if (wakeup_status_backup_info[i].wakeup_status == 0) {
            wakeup_status_backup_info[i].wakeup_status = !0;
            info = &wakeup_status_backup_info[i];
            break;
        }
    }
    os_mutex_post(&list_head_mutex);
    return info;
}

static void free_wakeup_status_backup_info(WAKEUP_STATUS_LIST_INFO *info)
{
    os_mutex_pend(&list_head_mutex, 0);
    info->wakeup_status = 0;
    os_mutex_post(&list_head_mutex);
}

u8 is_pir_wakeup_status()
{
    WAKEUP_STATUS_LIST_INFO *p, *n;
    os_mutex_pend(&list_head_mutex, 0);
    if (list_empty(&wakeup_status_list_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&list_head_mutex);
        return 0;
    }
    list_for_each_entry_reverse_safe(p, n, &wakeup_status_list_head, entry) {
        if (p->wakeup_status == PIR_WAKEUP) {
            os_mutex_post(&list_head_mutex);
            return 1;
        }
    }
    os_mutex_post(&list_head_mutex);

}
int is_same_wakeup_status(int wakeup_status)
{
    WAKEUP_STATUS_LIST_INFO *p, *n;
    os_mutex_pend(&list_head_mutex, 0);
    if (list_empty(&wakeup_status_list_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&list_head_mutex);
        return 0;
    }
    list_for_each_entry_reverse_safe(p, n, &wakeup_status_list_head, entry) {
        if (p->wakeup_status == wakeup_status) {
            os_mutex_post(&list_head_mutex);
            return 1;
        }
    }
    os_mutex_post(&list_head_mutex);
    return 0;
}
int wakeup_status_list_head_clear(void)
{
    WAKEUP_STATUS_LIST_INFO *p, *n;
    os_mutex_pend(&list_head_mutex, 0);
    if (list_empty(&wakeup_status_list_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&list_head_mutex);
        return 0;
    }
    list_for_each_entry_reverse_safe(p, n, &wakeup_status_list_head, entry) {
        list_del(&p->entry);
        free_wakeup_status_backup_info(p);
    }
    os_mutex_post(&list_head_mutex);
    return 0;
}

static void cloud_storage_timer_del(void)
{
    os_mutex_pend(&cloud_storage_mutex, 0);

    if ((cloud_storage_up_state == CLOUD_STORAGE_START) || (cloud_storage_up_state == CLOUD_STORAGE_STARTING)) {
        cloud_storage_timer_cnt = 0;
        if(cloud_storage_timer_id){
            sys_timer_del(cloud_storage_timer_id);
            cur_wakeup_info.wakeup_status = INIT_WAKEUP ;
            cloud_storage_timer_id = 0;
            cloud_storage_up_state = CLOUD_STORAGE_STOPING;
        }
    } else {
        printf("\n>>>>>>>The cloud storage timer has been deleted,cloud_storage_timer_id = %d\n", cloud_storage_timer_id);
    }
    os_mutex_post(&cloud_storage_mutex);

}

int get_avsdk_event_id(int wakeup_status)
{
    int event_id;
    if (wakeup_status == PIR_WAKEUP) {
        event_id = E_IOT_EVENT_SENSE_HUMAN;
    } else if (wakeup_status == KEY_WAKEUP) {
        event_id = E_IOT_EVENT_VISUAL_DOORBELL;
    } else if (wakeup_status == BREAK_WAKEUP) {
        event_id = E_IOT_EVENT_PICKPROOF_ALARM;
    } else if (wakeup_status == LOWPOWER_WAKEUP) {
        event_id = E_IOT_EVENT_LOW_BATTERY;
    }else if (wakeup_status == LOCAL_MSG_WAKEUP) {
        event_id = 62;
    }else if (wakeup_status ==MOVT_WAKEUP ) {
        event_id = E_IOT_EVENT_MOTION_DETECTION;
    }

    return event_id;
}

int64_t set_avsdk_push_event(int wakeup_status, u8 uploading,u8 isCS,void *user)
{
    printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    int64_t ret = -1;
    iot_event_opt_t  event = {0};
    user_info_t *info = (user_info_t *)user;
#if 0
    int avsdk_get_user_list(int32_t user_type, user_list_t *p);
    int32_t user_type = 0;
    user_list_t p;
    printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    avsdk_get_user_list(user_type,&p);
    printf("\n\n p.users_count = %d\n\n",p.users_count );
    for(int i = 0;i< p.users_count;i++){

        printf("\n\n p.users[%d].uid = %s\n\n",i,p.users[i].uid );
        printf("\n\n p.users[%d].name = %s\n\n",i,p.users[i].name );
        printf("\n\n p.users[%d].uid = %d\n\n",i,p.users[i].user_type );
    }
#endif
    printf("\n >>>>>>>>>>>>%d, %s %d\n",get_avsdk_connect_flag(),__func__,__LINE__);
    if (!get_avsdk_connect_flag()) {
        goto __exit;
    }
    event.channel = 0;
    event.isCS = isCS;
    event.value = 0;
    event.utc_ms = uploading ? start_time : get_utc_ms();
  //  event.local = true;
    event.local = false;

    event.id = get_avsdk_event_id(wakeup_status);


    if(info){
        event.uid = info->uid;
        printf("\n\n\n\n info->uid = %s \n\n\n\n",info->uid);
    }
    int retry = 0;

__again:
    ret =  avsdk_push_event(&event);
    if (ret <  0) {
        if (ret == -2 && is_same_wakeup_status(wakeup_status)) {
            printf("\n >>>>>> The same event cannot be triggered repeatedly within 5S\n");
#ifdef CONFIG_IPC_UI_ENABLE
            u8 get_ui_call_state = get_video_call_state();
            u8 get_ui_msg_state = get_video_msg_state();
            if(get_ui_call_state != CALL_STA_IDLE){
                printf("\n /***** get_ui_call_state = %d\n ",get_ui_call_state);
                notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"calling_fail",0);
            }else if(get_ui_msg_state != MSG_STA_IDLE){
                printf("\n /***** get_ui_call_state = %d\n ",get_ui_call_state);
//                notify_msg_to_video_rec(ACTION_VIDEO_REC_MSG_WORK,"msg_fail",0);
            }

#endif // CONFIG_IPC_UI_ENABLE
            goto __exit;
        } else if (ret != -1 && retry < 5) {
            retry++;
            msleep(1000);
            printf("\n >>>>>> set_avsdk_push_event retry\n");
            goto __again;
        }
        goto __exit;
    } else if (ret == 0) {
        printf("\nThe cloud storage function is not enabled for the current device\n");
        //即使没有开通云存，也需要保存状态，因为本地录像需要使用
        goto __exit;
    }
    if (!uploading) {
        start_time = ret;
    }
    if (event.isCS == false) {
        ret = 0;
    }
__exit:
    if (!is_same_wakeup_status(wakeup_status)) {
        WAKEUP_STATUS_LIST_INFO *info = find_wakeup_status_backup_info();
        if (info) {
            info->wakeup_status = wakeup_status;
            os_mutex_pend(&list_head_mutex, 0);
            list_add_tail(&info->entry, &wakeup_status_list_head);
            os_mutex_post(&list_head_mutex);
        }
    }
    return ret;
}



void wakeup_status_backup_cb(void *priv)
{

    WAKEUP_STATUS_LIST_INFO *backup_info = (WAKEUP_STATUS_LIST_INFO *)priv;
    printf("\n >>>>>>>>>wakeup_status_backup_cb backup_info->wakeup_status = %d\n", backup_info->wakeup_status);

    if (!is_same_wakeup_status(backup_info->wakeup_status)) {
        set_avsdk_push_event(backup_info->wakeup_status, backup_info->uploading,backup_info->isCs,backup_info->user);
    }

    free_wakeup_status_backup_info(backup_info);
}

int wakeup_status_backup_cb_condition(void)
{
    return (get_avsdk_connect_flag() && cloud_storage_up_state == CLOUD_STORAGE_START);
}

//保留云存定时器的目的是为了应对不联网的情况下，云存的启动
#define CLOUD_STORAGE_TIME_INTERVAL     100// 判断一次100ms
void cloud_storage_timer(void *priv)
{
    static int cnt;
    if (get_avsdk_connect_flag()) {
        //云存未开始则需要启动云存
        os_mutex_pend(&cloud_storage_mutex, 0);
        if (cloud_storage_up_state == CLOUD_STORAGE_STARTING) {
            if (os_sem_valid(&cloud_storage_sem) && !os_sem_query(&cloud_storage_sem)) {
                os_sem_post(&cloud_storage_sem);
                cnt = 0;
            }
        }
        os_mutex_post(&cloud_storage_mutex);
#if 0
        //采用定时器定时方式退出云存
        cnt++;
        if ((cloud_storage_up_state == CLOUD_STORAGE_START) && (cnt > (1000) / CLOUD_STORAGE_TIME_INTERVAL)) {
            cloud_storage_timer_cnt++;
            cnt = 0;
        }

        if (cloud_storage_timer_cnt >= (cloud_storage_timeout_cnt + 2)) {
            cloud_storage_timer_del();
        }

#else
        int is_del = 0;
        //采用数帧数方式退出云存
        for (int i = 0; i < CONFIG_CLOUD_STORAGE_NUM; i++) {
            printf("\n cloud_storage_fps_cnt[%d] = %d\n", i, cloud_storage_fps_cnt[i]);
            if (cloud_storage_fps_cnt[i] >= db_select("cyc") * net_video_rec_get_fps()) {
                is_del++;
            }
        }
        if (is_del >= CONFIG_CLOUD_STORAGE_NUM) {
            cloud_storage_timer_del();
        }

#endif // 0
    }
}


u8 get_cloud_storage_up_state(void)
{
    return cloud_storage_up_state;
}

u8 get_wakeup_status()
{
    return cur_wakeup_info.wakeup_status;
}

static int do_event_push_id;

void do_event_push(void *priv)
{
    WAKEUP_STATUS_LIST_INFO *info =(  WAKEUP_STATUS_LIST_INFO  *)priv;

    printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    int retry = 5000 / 100; //5s 事件触发5s后，平台如果没有连接上，应该认定为网络超时
    while(!get_avsdk_connect_flag()){
        msleep(100);
        retry--;
        if(retry <= 0){
            if(info->wakeup_status == KEY_WAKEUP){
                printf("\n >>>>>>>>>>>>%s %d net err\n",__func__,__LINE__);
//                notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"calling_net_connectfail",0);
                //TODO 此处应该显示发UI显示网络异常
            }
            sys_timeout_del(do_event_push_id);
            do_event_push_id = 0;
            return;
        }
    }
    printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    int ret = os_mutex_pend(&cloud_storage_mutex, 300);
    if (ret != OS_NO_ERR) {
        printf("\n >>>>>>%s %d  os_mutex_pend err\n", __func__, __LINE__);
        sys_timeout_del(do_event_push_id);
        do_event_push_id = 0;
        return;
    }
    printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    printf("\n >>>>>>>>>>>>>>>>>>>info->isCs = %d\n",info->isCs);
    if(!info->isCs){
          set_avsdk_push_event(info->wakeup_status,info->uploading,info->isCs,info->user);
    }else{

        //如果云存还没有启动，新建一个云存
         printf("\n >>>>>>>>>>>>%d, %s %d\n",cloud_storage_up_state,__func__,__LINE__);
        if (cloud_storage_up_state == CLOUD_STORAGE_STOP || cloud_storage_up_state == CLOUD_STORAGE_IDLE) {
            cloud_storage_up_state = CLOUD_STORAGE_STARTING;
            extern void doorbell_video_rec_get_iframe(u8 uvc_id);
            doorbell_video_rec_get_iframe(CONFIG_NET_SUB_ID);
            event_id = get_avsdk_event_id(info->wakeup_status);
            if (!cloud_storage_timer_id) {
                cloud_storage_timer_cnt = 0;
                memset(cloud_storage_fps_cnt, 0, sizeof(cloud_storage_fps_cnt));
                cloud_storage_timeout_cnt = db_select("cyc");
                cloud_storage_timer_id = sys_timer_add_to_task("sys_timer", NULL, cloud_storage_timer, CLOUD_STORAGE_TIME_INTERVAL);

            }
        } else {
        /*如果云存已启动*/
            if (cloud_storage_up_state == CLOUD_STORAGE_START) {
                if (is_same_wakeup_status(info->wakeup_status) &&  info->wakeup_status == PIR_WAKEUP) {
                    printf("\n >>>>>>%s %d is_same_wakeup_status(wakeup_status) = 1 &&  wakeup_status == PIR_WAKEUP \n", __func__, __LINE__);
                    goto __exit;
                }
                if (set_avsdk_push_event(info->wakeup_status, info->uploading,info->isCs,info->user) >= 0 && cloud_storage_timeout_cnt < CLOUD_STORAGE_TIME_CNT) {
                    cloud_storage_timeout_cnt += db_select("cyc");
                }
            } else if (cloud_storage_up_state == CLOUD_STORAGE_STOPING) { //如果云存停止中，只发通知
                if (is_same_wakeup_status(cur_wakeup_info.wakeup_status) &&  cur_wakeup_info.wakeup_status == PIR_WAKEUP) {
                    printf("\n >>>>>>%s %d is_same_wakeup_status(wakeup_status) = 1 &&  wakeup_status == PIR_WAKEUP \n", __func__, __LINE__);
                    goto __exit;
                }
                set_avsdk_push_event(info->wakeup_status, info->uploading,info->isCs,info->user);
            } else if (cloud_storage_up_state == CLOUD_STORAGE_STARTING) {
                //如果云存还没开始，不能再一次发通知,使用wait_completion等待平台连接成功再上报
                if (is_same_wakeup_status(cur_wakeup_info.wakeup_status) &&  cur_wakeup_info.wakeup_status == PIR_WAKEUP) {
                    printf("\n >>>>>>%s %d is_same_wakeup_status(wakeup_status) = 1 &&  wakeup_status == PIR_WAKEUP \n", __func__, __LINE__);
                    goto __exit;
                }
                printf("\n >>>>>>>>>cloud_storage_up_state == CLOUD_STORAGE_STARTING backup wakeup_status\n");
                WAKEUP_STATUS_LIST_INFO *backup_info = find_wakeup_status_backup_info();
                backup_info->wakeup_status = cur_wakeup_info.wakeup_status;
                backup_info->uploading = 1;
                wait_completion(wakeup_status_backup_cb_condition, wakeup_status_backup_cb, backup_info);

            }
        }
    }

__exit:
    sys_timeout_del(do_event_push_id);
    do_event_push_id = 0;

    free_wakeup_status_backup_info(info);
    os_mutex_post(&cloud_storage_mutex);

    return;
}
u8 get_wakeup_status_isCs(int wakeup_status)
{
    u8 isCs = 0;

    if(wakeup_status == LOCAL_MSG_WAKEUP){
        isCs = 1;
    }


    return isCs;
}

void set_wakeup_status(u8 status,void *user)
{
    printf("\n >>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    if ((status == INIT_WAKEUP) || (status == NETWORK_WAKEUP)) {
        return;
    }
    printf("\n >>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    int ret = os_mutex_pend(&cloud_storage_mutex, 300);
    if (ret != OS_NO_ERR) {
        printf("\n >>>>>>%s %d  os_mutex_pend err\n", __func__, __LINE__);
        return;
    }
        printf("\n >>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    cur_wakeup_info.wakeup_status = status;

    cur_wakeup_info.isCs = get_wakeup_status_isCs(cur_wakeup_info.wakeup_status);
    printf("\n >>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    //在这里判断事件是否需要启动云存
    printf("\n >>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);

    WAKEUP_STATUS_LIST_INFO *backup_info = (WAKEUP_STATUS_LIST_INFO *)calloc(1, sizeof(WAKEUP_STATUS_LIST_INFO));
    backup_info->wakeup_status = cur_wakeup_info.wakeup_status;
    backup_info->uploading = cloud_storage_up_state == cloud_storage_up_state ? 1 : 0;
    backup_info->isCs = cur_wakeup_info.isCs;
        printf("\n >>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    if(user){
        backup_info->user = &backup_info->user_info;
        memcpy(backup_info->user,user,sizeof(user_info_t));
    }else{
        backup_info->user = NULL;
    }

    printf("\n >>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    if(!do_event_push_id){
        do_event_push_id = sys_timeout_add_to_task("doorbell_block_event_task",backup_info,do_event_push,10);
    }



    os_mutex_post(&cloud_storage_mutex);

}


int cloud_storage_video_write(int type, u8 is_key_frame, int timestamp, char *buffer, int len, u8 ch)
{
    CLOUD_MEDIA_INFO  *p = NULL, *n = NULL;
    CLOUD_MEDIA_INFO  *info = NULL;
    u8 i = ch > 0 ? ch - 1 : ch;

    if(!os_mutex_valid(&cloud_media_info_list_mutex[i])){
        return 0;
    }

    os_mutex_pend(&cloud_media_info_list_mutex[i], 0);

    if (cloud_storage_up_state == CLOUD_STORAGE_STOPING) {
        os_mutex_post(&cloud_media_info_list_mutex[i]);
        return 0;
    }


    if (is_wait_iframe[i] && !is_key_frame) {
        os_mutex_post(&cloud_media_info_list_mutex[i]);
        return 0;
    }
#ifdef CLOUD_STORAGE_DEBUG
    if (is_wait_iframe[ch] && is_key_frame) {
        ASSERT(list_empty(&cloud_media_info_list_head[i]), "cloud_media_info_list_head[%d] no empty", i);
    }
#endif // CLOUD_STORAGE_DEBUG

    if (cloud_storage_up_state == CLOUD_STORAGE_STOP || cloud_storage_up_state == CLOUD_STORAGE_IDLE) {
        int Iframe_cnt = 0;

        //printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>net_video_rec_get_fps() *3.5 = %d\n",(int)(net_video_rec_get_fps() * 3.5));
        if ((video_frame_count[ch] + 1) >= (int)(net_video_rec_get_fps() * 3.5)) {
            //printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>video_frame_count = %d net_video_rec_get_fps() *3.5 = %d\n",video_frame_count[ch],(int)(net_video_rec_get_fps() * 3.5));
            list_for_each_entry_safe(p, n, &cloud_media_info_list_head[i], entry) {
                if (Iframe_cnt == 0) {
                    if (p->type == H264_TYPE_VIDEO && p->is_key_frame) {
                        Iframe_cnt = 1;
                    }
                } else {
                    if (p->type == H264_TYPE_VIDEO && p->is_key_frame) {
                        break;
                    }
                }
                if (p->type == H264_TYPE_VIDEO) {
                    video_frame_count[i]--;
                }
                list_del(&p->entry);
                free(p);
            }
#ifdef CLOUD_STORAGE_DEBUG
            ASSERT(!list_empty(&cloud_media_info_list_head[i]), "\n cloud_media_info_list_head[%d] empty  %s %d\n ", i, __func__, __LINE__);
            ASSERT(Iframe_cnt, "\n lost frame Must be Iframe_cnt ==%d  video_frame_count[%d] = %d\n", Iframe_cnt, i, video_frame_count[i]);
#endif // CLOUD_STORAGE_DEBUG
        }
    }

    if (cloud_storage_up_state == CLOUD_STORAGE_STARTING) {
        int Iframe_cnt = 0;

        printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>net_video_rec_get_fps() *3.5 = %d\n", (int)(net_video_rec_get_fps() * 3.5));
        if ((video_frame_count[ch] + 1) >= (int)(net_video_rec_get_fps() * 6)) {
            printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>video_frame_count[%d] = %d net_video_rec_get_fps() *3.5 = %d\n", i, video_frame_count[i], (int)(net_video_rec_get_fps() * 3.5));
            list_for_each_entry_safe(p, n, &cloud_media_info_list_head[ch], entry) {
                if (Iframe_cnt == 0) {
                    if (p->type == H264_TYPE_VIDEO && p->is_key_frame) {
                        Iframe_cnt = 1;
                    }
                } else {
                    if (p->type == H264_TYPE_VIDEO && p->is_key_frame) {
                        break;
                    }
                }
                if (p->type == H264_TYPE_VIDEO) {
                    video_frame_count[i]--;
                }
                list_del(&p->entry);
                free(p);
            }
#ifdef CLOUD_STORAGE_DEBUG
            ASSERT(!list_empty(&cloud_media_info_list_head[i]), "\n cloud_media_info_list_head empty[%d]  %s %d\n ", i, __func__, __LINE__);
            ASSERT(Iframe_cnt, "\n lost frame Must be Iframe_cnt ==%d  video_frame_count[%d]= %d\n", Iframe_cnt, i, video_frame_count[i]);
#endif // CLOUD_STORAGE_DEBUG
        }
    }

    if (is_wait_iframe[i]) {
        first_I_timestamp[i] = timestamp;
    }

    info = (CLOUD_MEDIA_INFO *)calloc(1, sizeof(CLOUD_MEDIA_INFO) + len); //lbuf内申请一块空间
    if (info) {
        info->type = type;
        info->is_key_frame = is_key_frame;
        info->difftime = timestamp - first_I_timestamp[i];
        info->ch = ch;
        info-> len = len;
        memcpy(&info->data[0], buffer, len);
        list_add_tail(&info->entry, &cloud_media_info_list_head[i]);
    } else {
#ifdef CLOUD_STORAGE_DEBUG
        ASSERT(0, "\n %s %d no mem \n", __func__, __LINE__);
#endif // CLOUD_STORAGE_DEBUG
    }

#ifdef CLOUD_STORAGE_DEBUG
    if (cloud_storage_up_state != CLOUD_STORAGE_STOPING && cloud_storage_up_state != CLOUD_STORAGE_START) {
        p = list_first_entry(&cloud_media_info_list_head[i], CLOUD_MEDIA_INFO, entry);
        ASSERT(p->is_key_frame, "\n list_first_entry no I frame %s %d\n", __func__, __LINE__);
    }
#endif // CLOUD_STORAGE_DEBUG

    video_frame_count[i]++;
    is_wait_iframe[i] = 0;
    audio_frame_start[i] = 1;
    os_mutex_post(&cloud_media_info_list_mutex[i]);
    return len;
}

int cloud_storage_audio_write(int type, int timestamp, char *buffer, int len, u8 ch)
{
    CLOUD_MEDIA_INFO  *p = NULL, *n = NULL;
    CLOUD_MEDIA_INFO  *info = NULL;
    u8 i = ch > 0 ? ch - 1 : ch;
    if(!os_mutex_valid(&cloud_media_info_list_mutex[i])){
        return 0;
    }
    os_mutex_pend(&cloud_media_info_list_mutex[i], 0);
    if (cloud_storage_up_state == CLOUD_STORAGE_STOPING) {
        os_mutex_post(&cloud_media_info_list_mutex[i]);
        return 0;
    }
    if (!audio_frame_start[i]) {
        os_mutex_post(&cloud_media_info_list_mutex[i]);
        return 0;
    }
#ifdef CLOUD_STORAGE_DEBUG
    if (cloud_storage_up_state != CLOUD_STORAGE_STOPING && cloud_storage_up_state != CLOUD_STORAGE_START) {
        ASSERT(!list_empty(&cloud_media_info_list_head[i]), "cloud_storage_audio_write cloud_media_info_list_head  empty");
        p = list_first_entry(&cloud_media_info_list_head[i], CLOUD_MEDIA_INFO, entry);
        ASSERT(p->is_key_frame, "\n list_first_entry no I frame %s %d  \n", __func__, __LINE__);
    }
#endif // CLOUD_STORAGE_DEBUG

    info = (CLOUD_MEDIA_INFO *)calloc(1, sizeof(CLOUD_MEDIA_INFO) + len); //lbuf内申请一块空间
    if (info) {
        info->type = type;
        info->is_key_frame = 0;
        info->difftime = timestamp - first_I_timestamp[i];;
        info->ch = ch;
        info-> len = len;
        memcpy(&info->data[0], buffer, len);
        list_add_tail(&info->entry, &cloud_media_info_list_head[i]);
    } else {
#ifdef CLOUD_STORAGE_DEBUG
        ASSERT(0, "\n %s %d no mem \n", __func__, __LINE__);
#endif // CLOUD_STORAGE_DEBUG
    }
    os_mutex_post(&cloud_media_info_list_mutex[i]);
    return len;
}
void cloud_storage_uploading(int uploading)
{

    int64_t ret = 0;
    CLOUD_MEDIA_INFO *p = NULL, *n = NULL;

    if (!uploading) {
        do {
            for (int i = 0; i < CONFIG_CLOUD_STORAGE_NUM; i++) {
                os_mutex_pend(&cloud_media_info_list_mutex[i], 0);
                if (get_cloud_storage_up_state() != CLOUD_STORAGE_START) {
                    printf("\n 111 time3 =  %d\n", timer_get_ms());
                    os_mutex_post(&cloud_media_info_list_mutex[i]);
                    break;
                }
                if (list_empty(&cloud_media_info_list_head[i])) {
                    os_mutex_post(&cloud_media_info_list_mutex[i]);
                    msleep(10);
                    continue;
                }
                list_for_each_entry_safe(p, n, &cloud_media_info_list_head[i], entry) {
                    list_del(&p->entry);
                    break;
                }
                if (p) {
                    if (get_cloud_storage_up_state() == CLOUD_STORAGE_START) {
                        if (p->type == H264_TYPE_VIDEO) {
                            cloud_storage_fps_cnt[i]++;
                        }
                    }
                    free(p);
                }
                os_mutex_post(&cloud_media_info_list_mutex[i]);

            }
        } while (get_cloud_storage_up_state() == CLOUD_STORAGE_START);
    } else {

        do {

            for (int i = 0; i < CONFIG_CLOUD_STORAGE_NUM; i++) {

                os_mutex_pend(&cloud_media_info_list_mutex[i], 0);
                if (get_cloud_storage_up_state() != CLOUD_STORAGE_START) {
                    printf("\n222 time3 =  %d\n", timer_get_ms());
                    os_mutex_post(&cloud_media_info_list_mutex[i]);
                    break;
                }
                if (list_empty(&cloud_media_info_list_head[i]) || (cloud_storage_fps_cnt[i] >= db_select("cyc") * net_video_rec_get_fps())) { //查询LBUF内是否有数据帧
                    os_mutex_post(&cloud_media_info_list_mutex[i]);
                    msleep(10);
                    continue;
                }
                list_for_each_entry_safe(p, n, &cloud_media_info_list_head[i], entry) {
                    list_del(&p->entry);
                    break;
                }
                os_mutex_post(&cloud_media_info_list_mutex[i]);
                if (p) {
                    if (!cloud_storage_first_frame[i]) {
                        cloud_storage_first_frame[i] = 1;
#ifdef CLOUD_STORAGE_DEBUG
                        if (p->type == H264_TYPE_VIDEO && !p->is_key_frame) {
                            //防止出现第一帧是音频帧或者不是I帧的情况出现，测试打开，生产可以关闭
                            ASSERT(0, "\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>cloud_storage_first_frame err H264 NO I\n");
                        } else if (p->type != H264_TYPE_VIDEO) {
                            ASSERT(0, "\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>cloud_storage_first_frame err  AUDIO frame\n");

                        }
#endif

                    }
                    if (get_cloud_storage_up_state() == CLOUD_STORAGE_START) {
#ifdef CLOUD_STORAGE_DEBUG

                        printf("\n p->difftime  = %d\n ", p->difftime);
#endif
                        if (p->type == H264_TYPE_VIDEO) {
                            ret = avsdk_cs_append_video_v2(cs_hdl,
                                                           p->ch,
                                                           p->is_key_frame,
                                                           e_cs_frame_none,
                                                           p->difftime,
                                                           NULL,
                                                           &p->data[0],
                                                           p->len
                                                          );

                            if (ret) {
								free(p);
                                printf("\n %s %d ret = %d\n", __func__, __LINE__, ret);
                                return;
                            }
                            cloud_storage_fps_cnt[i]++;
                        } else {
                            ret =  avsdk_cs_append_audio_v2(cs_hdl,
                                                            p->ch,
                                                            e_cs_frame_none,
                                                            p->difftime,
                                                            &p->data[0],
                                                            p->len
                                                           );
                            if (ret) {
                                printf("\n %s %d ret = %d\n", __func__, __LINE__, ret);
								free(p);
                                return;
                            }
                        }
                    }
                    free(p);
                }

            }

        } while (get_cloud_storage_up_state() == CLOUD_STORAGE_START);
    }
}

void clear_cloud_storage_list(void)
{
    CLOUD_MEDIA_INFO *p = NULL, *n = NULL;
    for (int i = 0; i < CONFIG_CLOUD_STORAGE_NUM; i++) {
        if (!list_empty(&cloud_media_info_list_head[i])) {
            os_mutex_pend(&cloud_media_info_list_mutex[i], 0);
            list_for_each_entry_safe(p, n, &cloud_media_info_list_head[i], entry) {
                list_del(&p->entry);
                free(p);
            }
            os_mutex_post(&cloud_media_info_list_mutex[i]);
        }
    }
}

void set_cloud_storage_end(int uploading)
{
    int64_t ret = 0;
    CLOUD_MEDIA_INFO *p = NULL, *n = NULL;

    cloud_storage_timer_del();

    for (int i = 0; i < CONFIG_CLOUD_STORAGE_NUM; i++) {
        if (!list_empty(&cloud_media_info_list_head[i])) {
            os_mutex_pend(&cloud_media_info_list_mutex[i], 0);
            list_for_each_entry_safe(p, n, &cloud_media_info_list_head[i], entry) {
                list_del(&p->entry);
                free(p);
            }
            os_mutex_post(&cloud_media_info_list_mutex[i]);
        }
    }


    os_mutex_pend(&cloud_storage_mutex, 0);
    if (uploading) {
        puts("\n avsdk_cs_end 1\n");
        ret  = avsdk_cs_end(&cs_hdl, NULL);
        puts("\n avsdk_cs_end 2\n");
    }
    cloud_storage_up_state = CLOUD_STORAGE_STOP;
    memset(video_frame_count, 0, sizeof(video_frame_count));
    memset(is_wait_iframe, 1, sizeof(is_wait_iframe));
    memset(audio_frame_start, 0, sizeof(audio_frame_start));
    memset(cloud_storage_first_frame, 0, sizeof(cloud_storage_first_frame));
    memset(cloud_storage_fps_cnt, 0, sizeof(cloud_storage_fps_cnt));
    os_sem_set(&cloud_storage_sem, 0);
    wakeup_status_list_head_clear();
    os_mutex_post(&cloud_storage_mutex);

}

void cloud_storage_task(void *priv)
{
    const char *file_name = "./xxx/cloud_file";//?éò??áD′μ????t??
    int64_t ret = 0;
    CLOUD_MEDIA_INFO *p = NULL, *n = NULL;


    avsdk_cs_set_log_level(LOG_LEVEL_NONE);

    while (1) {
        printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);
        ret = os_sem_pend(&cloud_storage_sem, 0);
        printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);
        if (ret != OS_NO_ERR) {
            msleep(10);
            continue;
        }

        ret = set_avsdk_push_event(cur_wakeup_info.wakeup_status, 0,1,NULL);


        os_mutex_pend(&cloud_storage_mutex, 0);
        cloud_storage_up_state = CLOUD_STORAGE_START;
        os_mutex_post(&cloud_storage_mutex);

        if (ret <= 0) {
            cloud_storage_uploading(0);
            set_cloud_storage_end(0);
        } else {
            puts("\n avsdk_cs_start 1\n");
            cs_start_opt_t opt = {
                .event_id = event_id,
                .start_timems = start_time,
                .channel = 0,
                .memsize = AVSDK_MEM_SIZE,
                .upload_way = e_cs_upload_way_memory,
                .file_name = "./cloud_file",
            };
            /*开始云储存*/
            int rc = avsdk_cs_start(&cs_hdl, &opt);
            printf("avsdk_cs_start, rc=%d", rc);
            if (!rc) {
                puts("\n avsdk_cs_start 2\n");
                uint8_t picture_num = CONFIG_CLOUD_STORAGE_NUM;
#if (CONFIG_AUDIO_FORMAT_FOR_MIC == AUDIO_FORMAT_AAC)

                avsdk_cs_set_format_v2(cs_hdl, e_cs_encrypt_none, e_cs_stream_h264, net_video_rec_get_fps(),
                                       e_cs_stream_aac,
                                       e_cs_sound_frequeency_16000,
                                       e_cs_sound_depth_16bit,
                                       e_cs_sound_channel_mono,
                                       picture_num,
                                       e_cs_picture_type_thumb);

#else
                avsdk_cs_set_format_v2(cs_hdl, e_cs_encrypt_none, e_cs_stream_h264, net_video_rec_get_fps(),
                                       e_cs_stream_pcm,
                                       e_cs_sound_frequeency_16000,
                                       e_cs_sound_depth_16bit,
                                       e_cs_sound_channel_mono,
                                       picture_num,
                                       e_cs_picture_type_thumb);
#endif

                u8 *buffer = calloc(1, 32 * 1024);
                u32 buffer_len = 0;
#if (defined CONFIG_VIDEO0_ENABLE)
                    if (buffer) {
                        printf("\ntime1 =  %d\n", timer_get_ms());
                        buffer_len = doorbell_video_rec_take_photo(CONFIG_NET_SUB_ID, buffer, 32 * 1024);
                        printf("\ntime2 =  %d\n", timer_get_ms());
                    }
                    printf("\n buffer_len = %d\n", buffer_len);
                    if (buffer_len) {
                        avsdk_cs_append_thumbnail_v2(cs_hdl, buffer, buffer_len, NULL);

                    } else {
                        //在这里启动拍照，将照片数据传递
                        avsdk_cs_append_thumbnail_v2(cs_hdl, NULL, 0, NULL);
                    }
#else
                for (int i = 0; i < CONFIG_CLOUD_STORAGE_NUM; i++) {
                    if (buffer) {
                        printf("\ntime1 =  %d\n", timer_get_ms());
                        buffer_len = doorbell_video_rec_take_photo(i, buffer, 32 * 1024);
                        printf("\ntime2 =  %d\n", timer_get_ms());
                    }
                    printf("\n buffer_len = %d\n", buffer_len);
                    if (buffer_len) {
                        avsdk_cs_append_thumbnail_v2(cs_hdl, buffer, buffer_len, NULL);

                    } else {
                        //在这里启动拍照，将照片数据传递
                        avsdk_cs_append_thumbnail_v2(cs_hdl, NULL, 0, NULL);
                    }
                }
#endif


                if (buffer) {
                    free(buffer);
                }

                cloud_storage_uploading(1);
                set_cloud_storage_end(1);

            } else {
                puts("\n avsdk_cs_start fail\n");
                cloud_storage_uploading(0);
                set_cloud_storage_end(0);
            }

        }
        extern void set_system_running(u8 value);
        set_system_running(1);
        post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_PIR_STATUS_GET);
    }
}

int cloud_storage_init()
{
    os_mutex_create(&cloud_storage_mutex);
    os_mutex_create(&list_head_mutex);
    os_sem_create(&cloud_storage_sem, 0);

    for (int i = 0; i < CONFIG_CLOUD_STORAGE_NUM; i++) {
        INIT_LIST_HEAD(&cloud_media_info_list_head[i]);
        os_mutex_create(&cloud_media_info_list_mutex[i]);
    }


    return thread_fork("cloud_storage_task", 8, 0x1000, 0, 0, cloud_storage_task, NULL);
}
//late_initcall(cloud_storage_init);








