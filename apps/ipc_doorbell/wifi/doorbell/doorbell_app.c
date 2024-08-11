
#define _STDIO_H_
#include "app_config.h"
#include "xciot_api.h"
#include "system/includes.h"
#include "action.h"
#include "server/network_mssdp.h"
#include "streaming_media_server/fenice_config.h"
#include "sock_api/sock_api.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "doorbell_event.h"


void calculate_frame_rate(CAL_INFO *info)
{


    if (info->t == 0) {
        info->t = timer_get_ms();
    }
    info->fps++;

    if (timer_get_ms() - info->t >= 1000) {
        printf("\n %s %d  %s fps = %d\n",info->func,info->line,info->des,info->fps);
        info->fps = 0;
        info->t = timer_get_ms();
    }
}

static LIST_HEAD(rt_talk_info_list_head);
static OS_MUTEX rt_talk_info_head_mutex;
RT_TALK_INFO  rt_talk_info[20];
static RT_TALK_INFO *find_empty_rt_talk_info(void)
{
    int i;
    RT_TALK_INFO *info = NULL;

    os_mutex_pend(&rt_talk_info_head_mutex, 0);
    for (i = 0; i < sizeof(rt_talk_info); i++) {
        if (rt_talk_info[i].conn_id == 0) {
            rt_talk_info[i].conn_id = !0;
            info = &rt_talk_info[i];
            break;
        }
    }
    os_mutex_post(&rt_talk_info_head_mutex);
    return info;
}

static void free_used_rt_talk_info(RT_TALK_INFO *info)
{
    os_mutex_pend(&rt_talk_info_head_mutex, 0);
    info->conn_id = 0;
    os_mutex_post(&rt_talk_info_head_mutex);
}


void add_audio_play_info(int conn_id)
{
     RT_TALK_INFO  *info = NULL;
    u8 find = 0;
    os_mutex_pend(&rt_talk_info_head_mutex,0);

    if(list_empty(&rt_talk_info_list_head)){
            printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
         info = find_empty_rt_talk_info();
        if(info){
           info->conn_id = conn_id;
           info->app_audio_play = 1;
           list_add_tail(&info->entry, &rt_talk_info_list_head);
        }
    }else{
            RT_TALK_INFO *p = NULL,*n = NULL;
            list_for_each_entry_safe(p, n,  &rt_talk_info_list_head, entry) {
                if(p->conn_id == conn_id){
                    find = 1;
            printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
                    break;
                }
            }
            if(find){
                p->app_audio_play = 1;
            }else{
                info = find_empty_rt_talk_info();
                if(info){
                   info->conn_id = conn_id;
                   info->app_audio_play = 1;
                   list_add_tail(&info->entry,  &rt_talk_info_list_head);
                    printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
                }
            }
    }
    os_mutex_post(&rt_talk_info_head_mutex);
}

void del_audio_play_info(int conn_id)
{
    RT_TALK_INFO  *info = NULL;
    u8 find = 0;
    os_mutex_pend(&rt_talk_info_head_mutex,0);

    if(list_empty(&rt_talk_info_list_head)){
               printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
        os_mutex_post(&rt_talk_info_head_mutex);

        return;
    }

    {
        RT_TALK_INFO *p = NULL,*n = NULL;
        list_for_each_entry_safe(p, n,  &rt_talk_info_list_head, entry) {
            if(p->conn_id == conn_id){
                   printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
                find = 1;
                break;
            }
        }
        if(find){
            p->app_audio_play = 0;
            if(!p->app_audio_play && !p->app_video_play && !p->device_video_play ){
                list_del(&p->entry);
                free_used_rt_talk_info(p);
                   printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
            }
        }
    }

    os_mutex_post(&rt_talk_info_head_mutex);
}

u8 find_audio_play_info(void)
{
    RT_TALK_INFO  *info = NULL;
    u8 find = 0;
    os_mutex_pend(&rt_talk_info_head_mutex,0);

    if(list_empty(&rt_talk_info_list_head)){
        os_mutex_post(&rt_talk_info_head_mutex);
           printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
        return 0;
    }

    {
        RT_TALK_INFO *p = NULL,*n = NULL;
        list_for_each_entry_safe(p, n,  &rt_talk_info_list_head, entry) {
            if(p->app_audio_play){
                find = 1;
               printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
                break;
            }
        }
    }

    os_mutex_post(&rt_talk_info_head_mutex);
    return find;
}




void add_video_play_info(int conn_id)
{
    RT_TALK_INFO  *info = NULL;
    u8 find = 0;

    printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
    os_mutex_pend(&rt_talk_info_head_mutex,0);
  printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
    if(list_empty( &rt_talk_info_list_head)){
              printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
        info = find_empty_rt_talk_info();
        if(info){
           info->conn_id = conn_id;
           info->app_video_play = 1;
           list_add_tail(&info->entry,  &rt_talk_info_list_head);
              printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
        }
    }else{
          printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
            RT_TALK_INFO *p = NULL,*n = NULL;
            list_for_each_entry_safe(p, n,  &rt_talk_info_list_head, entry) {
                if(p->conn_id == conn_id){
                    find = 1;
                    break;
                }
            }
            if(find){
                  printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
                p->app_video_play = 1;
            }else{
               info = find_empty_rt_talk_info();
                if(info){
                   info->conn_id = conn_id;
                   info->app_video_play = 1;
                   list_add_tail(&info->entry,  &rt_talk_info_list_head);
                      printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
                }
            }
    }
    os_mutex_post(&rt_talk_info_head_mutex);
}

void del_video_play_info(int conn_id)
{
    RT_TALK_INFO  *info = NULL;
    u8 find = 0;
    os_mutex_pend(&rt_talk_info_head_mutex,0);

    if(list_empty(&rt_talk_info_list_head)){
        os_mutex_post(&rt_talk_info_head_mutex);
        return;
    }

    {
        RT_TALK_INFO *p = NULL,*n = NULL;
        list_for_each_entry_safe(p, n,  &rt_talk_info_list_head, entry) {
            if(p->conn_id == conn_id){
                find = 1;
                break;
            }
        }
        if(find){
            p->app_video_play = 0;
            if(!p->app_audio_play && !p->app_video_play && !p->device_video_play ){
                list_del(&p->entry);
                free_used_rt_talk_info(p);
                   printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
            }
        }
    }

    os_mutex_post(&rt_talk_info_head_mutex);
}

u8 find_video_play_info(void)
{
    RT_TALK_INFO  *info = NULL;
    u8 find = 0;
    os_mutex_pend(&rt_talk_info_head_mutex,0);

    if(list_empty(&rt_talk_info_list_head)){
        os_mutex_post(&rt_talk_info_head_mutex);
        printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
        return 0;
    }

    {
        RT_TALK_INFO *p = NULL,*n = NULL;
        list_for_each_entry_safe(p, n,  &rt_talk_info_list_head, entry) {
            if(p->app_video_play){
                find = 1;
                printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
                break;
            }
        }
    }

    os_mutex_post(&rt_talk_info_head_mutex);
    return find;
}


void add_device_video_play_info(int conn_id,int ch)
{
    RT_TALK_INFO  *info = NULL;
    u8 find = 0;
    os_mutex_pend(&rt_talk_info_head_mutex,0);
  printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
    if(list_empty(&rt_talk_info_list_head)){

         info = find_empty_rt_talk_info();
          printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
        if(info){

           info->conn_id = conn_id;
           info->device_video_play = 1;
           info->device_video_play_ch = ch;
           list_add_tail(&info->entry,  &rt_talk_info_list_head);
           printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
        }
      printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
    }else{
          printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
            RT_TALK_INFO *p = NULL,*n = NULL;
            list_for_each_entry_safe(p, n,  &rt_talk_info_list_head, entry) {
                if(p->conn_id == conn_id){
                    find = 1;
                    printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
                    break;
                }
            }
            if(find){

               p->device_video_play = 1;
               p->device_video_play_ch = ch;
                 printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
            }else{
                 info = find_empty_rt_talk_info();
                if(info){

                   info->conn_id = conn_id;
                   info->device_video_play = 1;
                   info->device_video_play_ch = ch;
                   list_add_tail(&info->entry,  &rt_talk_info_list_head);
                   printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
                }
            }
    }
      printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
    os_mutex_post(&rt_talk_info_head_mutex);
      printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
}

void del_device_video_play_info(int conn_id)
{
    RT_TALK_INFO  *info = NULL;
    u8 find = 0;
    os_mutex_pend(&rt_talk_info_head_mutex,0);

    if(list_empty(&rt_talk_info_list_head)){
        os_mutex_post(&rt_talk_info_head_mutex);
        return;
    }

    {
        RT_TALK_INFO *p = NULL,*n = NULL;
        list_for_each_entry_safe(p, n,  &rt_talk_info_list_head, entry) {
            if(p->conn_id == conn_id){
                find = 1;
                printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
                break;
            }
        }
        if(find){
            p->device_video_play = 0;
            if(!p->app_audio_play && !p->app_video_play && !p->device_video_play ){
                list_del(&p->entry);
                free_used_rt_talk_info(p);
                printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
            }
        }
    }

    os_mutex_post(&rt_talk_info_head_mutex);
}


u8 find_device_video_play_info(RT_TALK_INFO  *tmp)
{
    RT_TALK_INFO  *info = NULL;
    u8 find = 0;
    os_mutex_pend(&rt_talk_info_head_mutex,0);

    if(list_empty(&rt_talk_info_list_head)){
        os_mutex_post(&rt_talk_info_head_mutex);

        printf("\n >>>>>>>>>>>%s %d\n",__func__,__LINE__);
        return 0;
    }

    {
        RT_TALK_INFO *p = NULL,*n = NULL;
        list_for_each_entry_safe(p, n,  &rt_talk_info_list_head, entry) {
            memcpy(tmp,p,sizeof(RT_TALK_INFO));
            if(p->device_video_play){

                find = 1;
                printf("\n talk_video_list_is_ok\n");
              #if 0
			    tmp->conn_id = p->conn_id;
                tmp->device_video_play_ch = p->device_video_play_ch;
              #endif
			    break;
            }
        }
    }

    os_mutex_post(&rt_talk_info_head_mutex);
    return find;
}



void clean_rt_talk_info(int32_t conn_id)
{

  printf("\n %s %d  conn_id = %d \n",__func__,__LINE__,conn_id);
  int _conn_id;
  int _ch;

  RT_TALK_INFO  info = {0};

  if(find_audio_play_info() || find_device_video_play_info(&info)){
      post_msg_doorbell_task("video_dec_task", 3, APP_STOP_VIDEO_CALL_STATE,APP_REFUSE_STATE,conn_id);
  }else{
      del_video_play_info(conn_id);
      del_audio_play_info(conn_id);
      del_device_video_play_info(conn_id);
      printf("\n\n\n %s find_device_video_play_info(&info) = %d  find_audio_play_info() = %d \n\n\n",__func__,find_device_video_play_info(&info), find_audio_play_info());
  }

}



static int  storage_format_timer_id;
static int percent;


static int local_msg_event_deal_flag = 0;
typedef struct  {
    uint64_t start_time;
}LOCAL_MSG_INFO;

LOCAL_MSG_INFO local_msg_info = {0};









int notify_msg_to_video_rec(int action,const char *data,u32 exdata)
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
        start_app(&it);
    }else{
        ret = -1;
    }
    return ret;
}

void  set_local_msg_start_time(uint64_t value)
{
    local_msg_info.start_time = value;
}
uint64_t get_local_msg_start_time(void)
{
    return local_msg_info.start_time;
}
uint64_t get_local_msg_end_time(void)
{
    return local_msg_info.start_time + 30 * 1000;
}

int get_local_msg_event_deal_flag(void)
{
    return local_msg_event_deal_flag;
}

void local_msg_event_deal(void)
{
    printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
    if(!local_msg_event_deal_flag){
        set_wakeup_status(LOCAL_MSG_WAKEUP,NULL);
        doorbell_start_msg_rec();
        //local_msg_event_deal_timer_id = sys_timeout_add_to_task("sys_timer", NULL, local_msg_event_deal_timer, 30 * 1000);
       local_msg_event_deal_flag = 1;
    }
}

void local_msg_event_stop(void)
{
    printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
    if(local_msg_event_deal_flag){
        printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
        int dalay_ms = get_utc_ms() - get_local_msg_start_time();
        if(  dalay_ms <  2000){
            printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);

            msleep(2000 - dalay_ms);
            printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
        }
        printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
        void doorbell_stop_msg_rec(void);
        printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
        doorbell_stop_msg_rec();
        local_msg_event_deal_flag = 0;
    }
    printf("\n\n >>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
}

static void storage_format_timer(void *arg)
{
    int32_t  conn_id = (int32_t)(arg);

    percent += 2;
    if (percent < 99) {
        avsdk_storage_format_rate(conn_id, 0, percent);
    }

}

void storage_format_thread(void *arg)
{

    int32_t  conn_id = (int32_t)(arg);
    printf("conn_id: %d ", conn_id);
    // here we notice the user progress of upgrading

    avsdk_storage_format_rate(conn_id, 0, 0);
    storage_format_timer_id = sys_timer_add(arg, storage_format_timer, 1000);
    int err = storage_device_format();
    percent = err ? -1 : 100;
    // reach progress 100%, upgrade finish success
    avsdk_storage_format_rate(conn_id, 0, percent);
    sys_timer_del(storage_format_timer_id);
    storage_format_timer_id = 0;
    extern int cloud_playback_list_clear(void);
    cloud_playback_list_clear();
    percent = 0;

    return ;
}
void storage_format_thread_create(void *arg)
{
    thread_fork("storage_format_thread", 20, 0x1000, 0, 0, storage_format_thread, arg);
}


static int doorbell_query_battery_timer_id;

void doorbell_query_battery_timer(void *priv)
{
    post_msg_doorbell_task("doorbell_heart_task", 1, DOORBELL_EVENT_QUERY_BATTARY);
}
void doorbell_query_battery_timer_add(void)
{
    if (!doorbell_query_battery_timer_id) {
        doorbell_query_battery_timer_id = sys_timer_add_to_task("sys_timer", NULL, doorbell_query_battery_timer, 2500);
    }
}

static int doorbell_query_qos_timer_id;
void doorbell_query_qos_timer(void *priv)
{
    extern u8 get_app_connect_flag(void);
    if (get_app_connect_flag()) {
        os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_QOS);
    }

}

void doorbell_query_qos_timer_add(void)
{
    if (!doorbell_query_qos_timer_id) {
        doorbell_query_qos_timer_id = sys_timer_add_to_task("sys_timer", NULL, doorbell_query_qos_timer, 5 * 60 * 1000);
    }
}

static int doorbell_netcfg_timeout_timer_id;
void doorbell_netcfg_timeout_timer(void *priv)
{
    if (!get_sdcard_upgrade_status()) {
        post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_ENTER_SLEEP, NULL);
    }
    doorbell_netcfg_timeout_timer_id = 0;
}

void doorbell_netcfg_timeout_timer_add(void)
{
    if (!doorbell_netcfg_timeout_timer_id) {
        doorbell_netcfg_timeout_timer_id = sys_timeout_add_to_task("sys_timer", NULL, doorbell_netcfg_timeout_timer, CONFIG_NETCONFIG_TIMEOUT);
    }
}

void doorbell_netcfg_timeout_timer_del(void)
{
    if (doorbell_netcfg_timeout_timer_id) {
        sys_timer_del(doorbell_netcfg_timeout_timer_id);
    }
}

void doorbell_blue_led_off_timer(void *priv)
{
    extern void doorbell_red_blue_off(void);
    doorbell_red_blue_off();
}

#define RT_TALK_BUFFER_SIZE     128 * 1024
static char buffer[RT_TALK_BUFFER_SIZE];
static cbuffer_t __cbuffer;
static void *audio_dev = NULL;
static int bindex = 0xff;

static void audio_dec_irq_handler(void *priv, void *data, int len)
{
    if(bindex == 0xff){
        memset((u8 *)data, 0, len);

        return;
    }
   // printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    int rlen = len;
    static u8 buf[1024];
    u16 data_len;
    cbuffer_t *cbuffer = (cbuffer_t *)priv;
    data_len = cbuf_read(cbuffer, buf, rlen);
    if (data_len == 0) {
        memset((u8 *)data, 0, len);
        printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
        dev_ioctl(audio_dev, AUDIOC_STREAM_OFF, bindex);
       // putchar('f');
        bindex = 0xff;
      #if 1
        void usb_speak_switch(void *state);
        usb_speak_switch("off");
      #endif

    } else {
   // printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
        memcpy(data, buf, data_len);
        if (data_len < rlen) {
            memset((u8 *)data + data_len, 0, len - data_len);
        }
       // putchar('n');
    }
}

struct adpcm_state {
    int valprev;
    int index;
};
static int old_volume;
static int audio_dev_init(void)
{
    struct audio_format f = {0};
    if (!audio_dev) {
        cbuf_init(&__cbuffer, buffer, RT_TALK_BUFFER_SIZE);
        audio_dev = dev_open("audio", (void *)AUDIO_TYPE_DEC);
        f.volume        = net_video_rec_get_dac_volume();
        f.channel       = 1;
        f.sample_rate   = net_video_rec_get_audio_rate();
        f.priority      = 1;
        f.type          = 0x1;
        f.frame_len     = 1024;
#ifndef CONFIG_USB_UVC_AND_UAC_ENABLE
        f.sample_source     = "dac";
#else
        f.sample_source     = "user";
#endif // CONFIG_USB_UVC_AND_UAC_ENABLE
        int err = dev_ioctl(audio_dev, AUDIOC_SET_FMT, (u32)&f);
        u32 arg[2];
        arg[0] = (u32)&__cbuffer;
        arg[1] = (u32)audio_dec_irq_handler;
        dev_ioctl(audio_dev, IOCTL_REGISTER_IRQ_HANDLER, (u32)arg);
        old_volume = f.volume;
    } else if (audio_dev && old_volume != net_video_rec_get_dac_volume()) {
        dev_close(audio_dev);
        cbuf_clear(&__cbuffer);
        audio_dev = dev_open("audio", (void *)AUDIO_TYPE_DEC);
        f.volume        = net_video_rec_get_dac_volume();
        f.channel       = 1;
        f.sample_rate   = net_video_rec_get_audio_rate();
        f.priority      = 1;
        f.type          = 0x1;
        f.frame_len     = 1024;
#ifndef CONFIG_USB_UVC_AND_UAC_ENABLE
        f.sample_source     = "dac";
#else
        f.sample_source     = "user";
#endif // CONFIG_USB_UVC_AND_UAC_ENABLE
        int err = dev_ioctl(audio_dev, AUDIOC_SET_FMT, (u32)&f);
        u32 arg[2];
        arg[0] = (u32)&__cbuffer;
        arg[1] = (u32)audio_dec_irq_handler;
        dev_ioctl(audio_dev, IOCTL_REGISTER_IRQ_HANDLER, (u32)arg);
        old_volume = f.volume;
    }
}




extern void adpcm_decoder(char *indata, short *outdata, int len, struct adpcm_state *state);
static void doorbell_play_voice_file(void *path)
{

    int len;
    void *fd = NULL;
    char *input_buffer = NULL;
    char *output_buffer = NULL;
    struct adpcm_state state = {0};


    audio_dev_init();

    fd = fopen(path, "r");
    if (!fd) {
        return;
    }
    len = flen(fd);
    printf("\n>>>>>>>>>>>>>>>>>>>>>> len =%d \n", len);
    input_buffer = malloc(len);
    output_buffer = malloc(len * 4);
    if (!input_buffer || !output_buffer) {

        goto __exit;
    }

    fread(fd, input_buffer, len);


    adpcm_decoder(input_buffer, output_buffer, len * 2, &state);

    cbuf_write(&__cbuffer, output_buffer, len * 4);

    fclose(fd);
    doorbell_event_dac_mute(1);
  //void usb_speak_switch(void *state);
  //usb_speak_switch("on");

    if( bindex == 0xff){
        dev_ioctl(audio_dev, AUDIOC_STREAM_ON, (u32)&bindex);
    }





__exit:
    if (input_buffer) {
        free(input_buffer);
    }
    if (output_buffer) {
        free(output_buffer);
    }
    fclose(fd);
}

void doorbell_play_voice(const char *name)
{
    int ret;
    static int retry;
    char path[128];

    snprintf(path, sizeof(path), "mnt/spiflash/audlogo/%s", name);

    doorbell_play_voice_file(path);


    #if 0
    if (!strcmp(name, "dingdong.adp")) {
        doorbell_blue_led_on();
        sys_timeout_add(NULL, doorbell_blue_led_off_timer, 3000);
    }

    if (!strcmp(name, "NetCfgEnter.adp")) {
        doorbell_red_led_on();
//        doorbell_netcfg_timeout_timer_add();
    }

    if (!strcmp(name, "SsidRecv.adp")) {
        doorbell_red_led_off();
//        doorbell_netcfg_timeout_timer_del();
    }

   #endif
}


struct isp_effect {
    u8 *buf;
    u32 size;
};
// 效果文件和res 文件夹下的对应
static const char *isp_cfg_file[] = {
    "mnt/spiflash/res/isp_cfg_1.bin",
    "mnt/spiflash/res/isp_cfg_3.bin",
    "mnt/spiflash/res/isp_cfg_4.bin",
    "mnt/spiflash/res/isp_cfg_scan.bin", // 扫码效果
};
static struct isp_effect effect[sizeof(isp_cfg_file) / sizeof(char *)];

void isp_effect_init(void)
{
    void *fd = NULL;
     printf("\n =========page=============%d, %s\n ",__LINE__,__FUNCTION__);
    printf("\n sizeof(effect) / sizeof(struct isp_effect) = %d \n", sizeof(effect) / sizeof(struct isp_effect));
    for (u8 i = 0; i < sizeof(effect) / sizeof(struct isp_effect); i++) {
        fd = fopen(isp_cfg_file[i], "r");
        if (!fd) {
            puts("cfg file err.\n");
            return;
        }
        effect[i].size = flen(fd);
        effect[i].buf = malloc(effect[i].size);
        if (!effect[i].buf) {
            puts("no mem.\n");
            return;
        }
        printf("\n read ok file\n");
        fread(fd, effect[i].buf, effect[i].size);
        fclose(fd);
    }
}


void isp_effect_user_day(u8 state)
{
    if (!effect[0].buf) {
        return;
    }

    extern void doorbell_irled_off(void);
    doorbell_irled_off();
 #if  1

    printf("\n state==================%d\n ",state);
    switch(state){


    case 0:

    if (ircut_set_isp_scenes(0, effect[0].buf, effect[0].size)) {
        //关闭IRCUT失败，置IRCUT状态为开启状态
        set_ircut_status(IRCUT_ON);
    } else {
        //关闭ircut
        extern void doorbell_ircut_off();
        doorbell_ircut_off();
        #ifdef CONFIG_IRCUT_ENABLE
        set_pre_isp_effect_state(0);
        #endif
        printf("\n  indoor isp effect \n");
    }
    break ;

    case 1:

    if (ircut_set_isp_scenes(1, effect[1].buf, effect[1].size)) {
        //关闭IRCUT失败，置IRCUT状态为开启状态
        set_ircut_status(IRCUT_ON);
    } else {
        //关闭ircut
        extern void doorbell_ircut_off();
        doorbell_ircut_off();
        #ifdef CONFIG_IRCUT_ENABLE
        set_pre_isp_effect_state(1);
        #endif
        printf("\n  outdoor isp effect \n");
    }

    break ;


    case 3: //配网效果

    if (ircut_set_isp_scenes(3, effect[3].buf, effect[3].size)) {

    } else {

    }

    break ;


     default:
     printf("\n 0 state\n");


    if (ircut_set_isp_scenes(0, effect[0].buf, effect[0].size)) {
        //关闭IRCUT失败，置IRCUT状态为开启状态
        set_ircut_status(IRCUT_ON);
    } else {
        //关闭ircut
        extern void doorbell_ircut_off();
        doorbell_ircut_off();
        #ifdef CONFIG_IRCUT_ENABLE
        set_pre_isp_effect_state(0);
        #endif
        printf("\n  indoor isp effect \n");
    }

     break;

    }

  #else
      //关闭ircut
        extern void doorbell_ircut_off();
        doorbell_ircut_off();

  #endif
}

void isp_effect_user_night(void)
{
    if (!effect[0].buf) {
        return;
    }
    //开启ircut

    #if  1
    if (ircut_set_isp_scenes(2, effect[2].buf, effect[2].size)) {
        //开启IRCUT失败，置IRCUT状态为关闭状态
        printf("\n scense fail \n ");
        set_ircut_status(IRCUT_OFF);
    } else {

        extern void doorbell_ircut_on();
        doorbell_ircut_on();
        //打开红外灯
        extern void doorbell_irled_on(void);
        doorbell_irled_on();
        #ifdef CONFIG_IRCUT_ENABLE
        set_pre_isp_effect_state(2);
        #endif
        printf("\n  night isp effect \n");
    }
    #else


         extern void doorbell_ircut_on();
        doorbell_ircut_on();
        //打开红外灯
        extern void doorbell_irled_on(void);
        doorbell_irled_on();

    #endif

}



void avsdk_cmd_sleep_timer(void *priv)
{
    printf("\n >>>>>>>>>>>>>>%s\n", __func__, __LINE__);
    hi3861l_reset();
    printf("\n >>>>>>>>>>>>>>%s\n", __func__, __LINE__);
}

void doorbell_sleep_timeout(void)
{
    sys_timeout_add(NULL, avsdk_cmd_sleep_timer, 15 * 1000);
}

static int doorbell_wait_completion_id;


void video_rec_control_start_cb(void *priv)
{
    printf("\n %s %d\n", __func__, __LINE__);
    post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_START_REC);
    wait_completion_del(doorbell_wait_completion_id);
    doorbell_wait_completion_id = 0;
}
int video_rec_control_start_condition(void)
{
    if (is_video_rec_mode() && storage_device_ready()) {
        return 1;
    }
    return 0;
}

static u8 need_notify_wakeup_status(void)
{
    static int timeout = 0;
    //5s内重复按过滤
    if (timeout == 0 || (timer_get_ms() - timeout) >= 3000) {
        timeout = timer_get_ms();
        return 1;
    }
    return 0;
}
static int network_status = 0;
int get_network_status()
{
    return network_status;
}
void doorbell_app_deal_network_status(int status)
{
    printf("\n status = %d last network_status = %d \n",status,network_status);
    if (status == NETWORK_CONFIG) {
         struct sys_event e;
         e.type = SYS_NET_EVENT;
         e.u.net.event = NET_EVENT_CFG;
         sys_event_notify(&e);
        printf("\n >>>>>>>>>>>>>>>>>>%s %d \n",__func__,__LINE__);
//        post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetCfgEnter.adp");
//        extern void qr_net_cfg_init(void);
//        qr_net_cfg_init();
    } else if (status == CONNECTING || status ==  CONNECT_SUC) {
            printf("\n >>>>>>>>>>>>>>>>>>%s %d \n",__func__,__LINE__);
             struct sys_event e;
             e.type = SYS_NET_EVENT;
             e.u.net.event = NET_EVENT_CONNECTING;
             sys_event_notify(&e);

    } else if (status == DISCONNECT) {
        printf("\n >>>>>>>>>>>>>>>>>>%s %d \n",__func__,__LINE__);
            struct sys_event e;
            e.type = SYS_NET_EVENT;
            e.u.net.event = NET_EVENT_DISCONNECTED;
            sys_event_notify(&e);
        printf("\n >>>>>>>>>>>>>>>>>>%s %d \n",__func__,__LINE__);
    } else if (status == DHCP_SUC) {
        printf("\n >>>>>>>>>>>>>>>>>>%s %d \n",__func__,__LINE__);
            struct sys_event e;
            e.type = SYS_NET_EVENT;
            e.u.net.event = NET_EVENT_GET_IP_INFO;
            sys_event_notify(&e);

    } else if (status == DHCP_FAIL || status == NOT_FOUND_SSID ) {
            struct sys_event e;
            e.type = SYS_NET_EVENT ;
            e.u.net.event = NET_EVENT_CONNECTFAIL;
            sys_event_notify(&e);
    }else if (status == PASSWORD_ERR || status == OTHER_ERR){
            struct sys_event e;
            e.type = SYS_NET_EVENT;
            e.u.net.event = NET_EVENT_PASSWORD_ERR;
            sys_event_notify(&e);
    }else if (status == AP_MODE_CFG) {
            struct sys_event e;
            e.type = SYS_NET_EVENT;
            e.u.net.event = NET_EVENT_AP_MODE_CFG;
            sys_event_notify(&e);


    } else if (status == AP_MODE_OPEN) {
            struct sys_event e;
            e.type = SYS_NET_EVENT;
            e.u.net.event = NET_EVENT_AP_OPEN_SUC;
            sys_event_notify(&e);
    }
    printf("\n >>>>>>>>>>>>>>>>>>%s %d \n",__func__,__LINE__);
    network_status = status;
}

void doorbell_app_deal_wakeup_source(int status,void *user)
{
    if (is_product_mode()) {
        goto __exit;
    }

    printf("\n status:%d\n ",status);
    if (status == INIT_WAKEUP) {
        set_wakeup_status(status,user);
    } else if (status == NETWORK_WAKEUP) {
        set_wakeup_status(status,user);
    } else if (status == PIR_WAKEUP || status == KEY_WAKEUP) {
        //按键唤醒间隔3s播提示音
        if (status == KEY_WAKEUP) {
            if (need_notify_wakeup_status()) {
              //  post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "dingdong.adp");
                post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "start_call.adp");

                extern void doorbell_433_ctrl(void);
                doorbell_433_ctrl();
            }
        }
        set_wakeup_status(status,user);

        if (get_video_rec_state() == 0 && doorbell_wait_completion_id == 0) {
            printf("\n %s %d state = %d, completion_id = %d\n", __func__, __LINE__,get_video_rec_state(),doorbell_wait_completion_id);
            char *str = NULL;
            if (status == PIR_WAKEUP) {
                str = "PIR";
            } else if (status == KEY_WAKEUP) {
                str = "KEY";
            }
         //   set_rec_file_name_prefix(str);
         //   doorbell_wait_completion_id = wait_completion(video_rec_control_start_condition, video_rec_control_start_cb, (void *)0);
        }
    } else if (status == BREAK_WAKEUP) {
        set_wakeup_status(status,user);
        //增加强拆提示音
        post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "alarm.adp");
    } else if (status == LOWPOWER_WAKEUP) {
        post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "LowBattery.adp");
        set_wakeup_status(status,user);
    }else if (status == MOVT_WAKEUP) {

         printf("\n MOVT_WAKEUP\n ");
        set_wakeup_status(status,user);
    }
__exit:

    if(user){
        free(user);
    }
}

void mass_production_socket_accpet(void *priv)
{
    void *sock_hdl = priv;
    void *cli = NULL;
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int ret;
    char *buf = calloc(1, 1024);
    if (!buf) {
        printf("\n %s %d no mem\n", __func__, __LINE__);
        goto __exit;
    }

    while (1) {
        cli  = sock_accept(sock_hdl, (struct sockaddr *)&addr, &len, NULL, NULL);
        if (cli == NULL) {
            printf("\n[ %s sock_accept err ]\n", __func__);
            continue;
        }
        memset(buf, 0, 1024);
        int offset = 0;
__recv_again:
        ret = sock_recv(cli, buf + offset, 1024 - offset, 0);
        if (ret > 0) {
            offset += ret;
            json_object *root = NULL;
            json_object *key_cmd = NULL;
            json_object *key_len = NULL;
            json_object *key_data = NULL;
            root =  json_tokener_parse(buf);
            if (root) {
                key_cmd =  json_object_object_get(root, "cmd");
                key_len =  json_object_object_get(root, "len");
                key_data =  json_object_object_get(root, "data");
                if (key_cmd && key_len && key_data) {
                    extern void doorbell_write_cfg_buf_info(void *buf, int len);
                    doorbell_write_cfg_buf_info(json_object_get_string(key_data), json_object_get_int(key_len));
                    char str[] = "{\"cmd\":0,\"len\":3,\"data\":\"suc\"}";
                    sock_send(cli, str, sizeof(str), 0);
                    printf("\n did data is write ok\n");
                    sock_unreg(cli);
                    continue;
                }
            } else {
                printf("\n%s %d no enough data\n", __func__, __LINE__);
                goto __recv_again;
            }
        } else {
            printf("\n%s %d cli disconnect\n", __func__, __LINE__);
            sock_unreg(cli);
            continue;
        }
    }
__exit:
    if (buf) {
        free(buf);
    }
    if (cli) {
        sock_unreg(cli);
    }
    if (sock_hdl) {
        sock_unreg(sock_hdl);
    }

}

void mass_production_socket_init(void)
{
    void *sock_hdl = NULL;

    sock_hdl = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL);
    u32 opt = 1;
    if (sock_setsockopt(sock_hdl, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        printf("\n[ %s sock_bind fail ]\n", __FILE__);
        goto exit;
    }
    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY) ;
    local_addr.sin_port = htons(3335);
    if (sock_bind(sock_hdl, (struct sockaddr *)&local_addr, sizeof(struct sockaddr))) {
        printf("\n[ %s sock_bind fail ]\n", __FILE__);
        goto exit;
    }
    sock_listen(sock_hdl, 0x5);
    thread_fork("mass_production_socket_accpet", 25, 0x400, 0, 0, mass_production_socket_accpet, sock_hdl);
exit:
    return;
}




void mass_production_init(void)
{

    printf("network mssdp init\n");
    network_mssdp_init();
    /*
    *代码段功能:修改RTSP的URL
    *默认配置  :URL为rtsp://192.168.1.1/avi_pcm_rt/front.sd,//(avi_pcma_rt 传G7111音频)传JPEG实时流
    *
    */
#if 1
    char *user_custom_name = "mass_pro_rt";
    char *user_custom_content =
        "stream\r\n \
    file_ext_name 264\r\n \
    media_source live\r\n \
    priority 1\r\n \
    payload_type 96\r\n \
    clock_rate 90000\r\n \
    encoding_name H264\r\n \
    coding_type frame\r\n \
    byte_per_pckt 1458\r\n \
    stream_end\r\n \
    stream\r\n \
    file_ext_name pcm\r\n \
    media_source live\r\n \
    priority 1\r\n \
    payload_type 8\r\n \
    encoding_name PCMA\r\n \
    clock_rate 8000\r\n \
    stream_end";
    extern void rtsp_modify_url(const char *user_custom_name, const char *user_custom_content);
    rtsp_modify_url(user_custom_name, user_custom_content);
#endif

    extern int stream_media_server_init(struct fenice_config * conf);
    extern int fenice_get_video_info(struct fenice_source_info * info);
    extern int fenice_get_audio_info(struct fenice_source_info * info);
    extern int fenice_set_media_info(struct fenice_source_info * info);
    extern int fenice_video_rec_setup(void);
    extern int fenice_video_rec_exit(void);
    struct fenice_config conf;

    strncpy(conf.protocol, "TCP", 3);
    conf.exit = fenice_video_rec_exit;
    conf.setup = fenice_video_rec_setup;
    conf.get_video_info = fenice_get_video_info;
    conf.get_audio_info = fenice_get_audio_info;
    conf.set_media_info = fenice_set_media_info;
    conf.port = 0;  // 当为0时,用默认端口554
    stream_media_server_init(&conf);

    mass_production_socket_init();
}





extern int doorbell_event_init(void);//主要是create wifi 线程的
extern int Hi3861L_init(void) ;//主要是create wifi 线程的
extern int avsdk_cmd_init(void);
extern s32 hi3861l_hichannel_init();
extern int sdcard_upgrade_init();
extern void doorbell_write_cfg_info(void);
extern int cloud_playback_list_init();
extern int cloud_rt_talk_init(void);
extern int cloud_storage_init();


int calculate_utf8_length(char *str)
{
    int length = 0;
    int i = 0;
    while (str[i] != '\0') {
        if ((str[i] & 0xc0) != 0x80) {  // 判断是否为UTF-8编码的第一个字节
            if ((str[i] & 0xe0) == 0xe0) {  // 判断是否为中文字符的UTF-8编码
                length += 3;  // 中文字符在UTF-8编码中占据3个字节，但只计算为2个字符长度
            } else {
                length++;  // 英文字符在UTF-8编码中占据1个字节，计算为1个字符长度
            }
        }
        i++;
    }

    printf("\n >>>>>>>>>>>>>>>>>>>length  %d\n",length);

    return length;
}

void doorbell_app_init()
{



    printf("storage_device_ready() = %d \n", storage_device_ready());


    extern void get_flash_idle_info(void);
    get_flash_idle_info();

    net_media_dec_init();

    os_mutex_create(&rt_talk_info_head_mutex);
#if  0
    if (!product_enter_check()) {
    }
#endif

    extern void net_video_handler_init(void);
    net_video_handler_init();

    /*使底层sdio接收数据线程先运行起来*/

    /*建立门铃的事件处理函数，所有的门铃事件统一由这个任务处理*/
    doorbell_event_init();

    cloud_platform_init();

#ifdef CONFIG_WIFI_HI3861L_ENABLE
    /*初始化与Hi3861L命令传输协议*/
    avsdk_cmd_init();
    /*建立与Hi3861L 通信的任务，所有命令的发送和接受统一由此函数处理*/
    Hi3861L_init();

    /*初始化hichannel通道，建立跟Hi3861L的通信*/
    if (hi3861l_hichannel_init()) {
        hi3861l_reset();
        return;
    }
#endif // CONFIG_WIFI_HI3861L_ENABLE

#ifdef LONG_POWER_IPC
  //  messgae_task_init();
#endif





    /*检查光敏电阻的阻值*/
    extern int light_adc_init(void);
    light_adc_init();


    /*初始化云存上传任务*/
    cloud_storage_init();
    /*初始化对讲功能*/
    cloud_rt_talk_init();
#ifndef LONG_POWER_IPC
    post_msg_doorbell_task("doorbell_heart_task", 1, DOORBELL_EVENT_QUERY_BATTARY);
    /*发送获取所有配置信息的命令给3861L*/
    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_ALL_CFG);

#endif
    // msleep(200);
    /*初始化回看的文件列表*/
    //cloud_playback_list_init();

#ifdef LONG_POWER_IPC
      post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_ENTER_NET_CONFIG_MODE);

  //    post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_ENTER_CHECK_TIMER_REC);
#endif
    /*启动休眠休眠检查线程*/
//    extern void doorbell_sleep_thread_init(void);
//    doorbell_sleep_thread_init();

}




