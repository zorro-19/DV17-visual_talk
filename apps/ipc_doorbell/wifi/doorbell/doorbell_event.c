#define _STDIO_H_
#include "xciot_api.h"
#include "system/includes.h"
#include "action.h"
#include "doorbell_event.h"



#ifdef  LONG_POWER_IPC

#include "server/wifi_connect.h"
void set_poweron_net_config_state(u8 flag){
// 0 初始化状态  , 1 正在配网  2 配网成功，3 配网失败

  db_update("net_state",flag);
  db_flush();

  u8 net_state=db_select("net_state");
  printf("\n set_net_state:%d\n ",net_state);
}


u8 get_poweron_net_config_state(){


    static u8 net_state;

    net_state=db_select("net_state");
    //printf("\n get_net_state=========%d\n ",net_state);

    return  net_state;

}

void set_cpu_reset_state(u8 flag){

  db_update("reset_state",flag);
  db_flush();

  u8 reset_state=db_select("reset_state");
  printf("\n reset_state:%d\n ",reset_state);

}
u8 get_cpu_reset_state(){


    static u8 reset_state;

    reset_state=db_select("reset_state");
    printf("\n get_reset_state=========%d\n ",reset_state);

    return  reset_state;




}

void avsdk_wifi_sta_mode_info(char *ssid, char *pwd)
{
    int rc = 0;
    printf("\n>>> %s %d\n", __func__, __LINE__);

    struct WIFI_INIT_INFO wifi_info = {0};;
    wifi_info.mode = STA_MODE;
    strcpy(wifi_info.ssid, ssid);
    strcpy(wifi_info.pwd, pwd);
    db_update_buffer(WIFI_INFO, (char *)&wifi_info, sizeof(struct WIFI_INIT_INFO));


}
static int doorbell_rec_timer_check_id;
extern bool judge_plan_video_timer();
void doorbell_rec_timer_check(){


    if(db_select("mot")){
        printf("\n mot is open\n");
        return ;
    }

    if(get_poweron_net_config_state()==0){
        printf("\n not config \n");
        return ;
    }

    printf("\n task check rec timer \n");
    if(judge_plan_video_timer()){

     printf("\n rec timer is ok \n");
    post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_ENTER_CHECK_TIMER_REC);
    }else{
     printf("\n not rec timer \n");
    }
}

void doorbell_rec_check_timer_add(void)
{
    if (!doorbell_rec_timer_check_id) {
        doorbell_rec_timer_check_id = sys_timer_add_to_task("sys_timer", NULL, doorbell_rec_timer_check, 1000*50);
    }
}

#endif


#include "mbedtls/md5.h"
int get_picture_md5_crc( char *ibuf,char *obuf,int len){


    mbedtls_md5_context md5_ctx;
    unsigned char md5sum[16];
    int use_len =16;

    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts(&md5_ctx);
    mbedtls_md5_update(&md5_ctx, ibuf, len);
    mbedtls_md5_finish(&md5_ctx, md5sum);
    put_buf(md5sum,sizeof(md5sum));

     if (use_len <= 16) {
        printf("\n A\n");
        memcpy(obuf, md5sum, sizeof(md5sum));
        printf("\n B\n");
        mbedtls_md5_free(&md5_ctx);
        return 1;
    }else{


        return 0;
    }

    return 1;
}

void test_function(){

       sys_key_event_disable();
                 printf("\n DOORBELL_EVENT_ENTER_RESET_CPU\n");
                   wifi_off();
                   extern void net_config_before(void);
                    net_config_before();
                    set_poweron_net_config_state(0);// 设置重新配网标志

                    printf("\n wifi offs\n");

                 //   post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetCfgEnter.adp");=
                    os_time_dly(100);
                    cpu_reset();


}

void doorbell_event_task(void *priv)
{
    int res;
    int msg[32];
    void *fd;
    int ret;
    struct intent it;
    struct application *app;
    u32 free_addr;
    u32 free_space;
    u8 uvc_id;
#if 0
    extern u8 *g_ref_addr[2];
    g_ref_addr[0] = malloc(3575584 + 1024);
    g_ref_addr[1] = malloc(3575584 + 1024);
    if (!g_ref_addr[0] ||  !g_ref_addr[1]) {
        printf("\n %s %d\n", __func__, __LINE__);
        while (1);
    }
#endif


    isp_effect_init();

    while (1) {

        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case DOORBELL_EVENT_PLAY_VOICE:
                    puts("\n DOORBELL_EVENT_PLAY_VOICE\n");


                    usb_speak_switch("on");//打开usb speak
                    doorbell_play_voice(msg[2]);
                    break;
                case DOORBELL_EVENT_IRCUT_OFF:
                    puts("DOORBELL_EVENT_IRCUT_OFF\n");
                    #ifdef CONFIG_IRCUT_ENABLE
                    extern u8 get_isp_effect_state();
                    isp_effect_user_day(get_isp_effect_state());
                    #endif
                    break;
                case DOORBELL_EVENT_IRCUT_ON:
                    puts("DOORBELL_EVENT_IRCUT_ON\n");
                    isp_effect_user_night();
                    break;
                case DOORBELL_EVENT_IN_USB:

                    doorbell_in_usb();
                    break;

                case DOORBELL_EVENT_OPEN_RT_STREAM:
                    puts("DOORBELL_EVENT_OPEN_RT_STREAM\n");
                    extern void *get_uvc_camera_device(void);
                    if (!is_usb_mode()) {


                        doorbell_open_rt_stream(0);
                       // printf("\n switch isp effect \n\n");
                       // extern void light_adc_value_check(void *priv);
                       // light_adc_value_check(NULL);

                    }


                    break;
                case DOORBELL_EVENT_CLOSE_RT_STREAM:
                    puts("DOORBELL_EVENT_CLOSE_RT_STREAM\n");
                    uvc_id = msg[2];
                    doorbell_close_rt_stream(uvc_id);
                    break;
                case DOORBELL_EVENT_REOPEN_RT_STREAM:
                    puts("DOORBELL_EVENT_REOPEN_RT_STREAM\n");
                    uvc_id = msg[2];
                    void doorbell_reopen_rt_stream(u8 uvc_id);
                    doorbell_reopen_rt_stream(uvc_id);
                    break;
                case DOORBELL_EVENT_START_REC:
                    puts("DOORBELL_EVENT_START_REC\n");
                    doorbell_start_rec();

                    break;
                case DOORBELL_EVENT_STOP_REC:
                    puts("DOORBELL_EVENT_STOP_REC\n");
                    doorbell_stop_rec();

                    break;
                case DOORBELL_EVENT_CHANGE_CYC_TIME:
                    puts("DOORBELL_EVENT_CHANGE_CYC_TIME\n");

                    if (get_video_rec_state()) {
                        doorbell_stop_rec();
                        doorbell_start_rec();
                    }

                    break;
                case DOORBELL_EVENT_ENTER_SLEEP:
                    puts("DOORBELL_EVENT_ENTER_SLEEP \n");
                    doorbell_close_all_rt_stream();
                    //如果在录像就需要停止录像
                    doorbell_stop_rec();

#ifdef CONFIG_AEC_DNS_TEST_ENABLE
                    extern void aec_dns_test_file_save();
                    aec_dns_test_file_save();

                    msleep(5000);//延时5s 让文件保存成功
#endif
                    os_taskq_post("hi_channel_task", 2, HI_CHANNEL_CMD_SLEEP, msg[2]);
                    break;
                case DOORBELL_EVENT_SD_FORMAT:
                    puts("DOORBELL_EVENT_SD_FORMAT \n");
                    doorbell_stop_rec();
                    storage_format_thread_create((void *)msg[2]);
                    break;

                case DOORBELL_EVENT_RESET_CMD:
                    doorbell_close_all_rt_stream();

                    //如果在录像就需要停止录像
                    doorbell_stop_rec();

                    #ifdef LONG_POWER_IPC
                   // doorbell_open_all_rt_stream();

                    void net_config_before(void);
                     net_config_before();
                     avsdk_unbind();
                     avsdk_fini();
                     post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_ENTER_CLEAR_SYSTEM);
                    #endif

                    #ifndef LONG_POWER_IPC
                    os_taskq_post("hi_channel_task", 2, HI_CHANNEL_CMD_RESET_SET, msg[2]);
                    #endif


                    break;
                case DOORBELL_EVENT_AP_CHANGE_STA:
                    doorbell_close_all_rt_stream();
                    //如果在录像就需要停止录像
                    doorbell_stop_rec();
                    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_AP_CHANGE_STA);
                    break;
                case DOORBELL_EVENT_PIR_STATUS_GET:
                    doorbell_stop_rec();
                    extern u8 is_pir_wakeup_status();
                    if (is_pir_wakeup_status()) {
                        os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_PIR);
                    } else {
                        extern void set_system_running(u8 value);
                        set_system_running(0);
                    }
                    break;
                case DOORBELL_EVENT_ALL_REOPEN_RT_STREAM:
                    doorbell_close_all_rt_stream();
                    doorbell_open_all_rt_stream();
                    break;

                case DOORBELL_EVENT_LOCAL_MSG_REC_START:{
                        void local_msg_event_deal(void);
                        local_msg_event_deal();
                    }
                    break;
                case DOORBELL_EVENT_LOCAL_MSG_REC_STOP:{
                        local_msg_event_stop();
                    }

                    break;
                #ifdef  LONG_POWER_IPC

                case DOORBELL_EVENT_ENTER_RESET_CPU: // 复位系统重新进入配网

                    sys_key_event_disable();
                    printf("\n DOORBELL_EVENT_ENTER_RESET_CPU\n");
                   extern void net_config_before(void);
                    net_config_before();
                    set_poweron_net_config_state(0);// 设置重新配网标志

                    printf("\n wifi offs\n");
                    wifi_off();

                 //   post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetCfgEnter.adp");

                    os_time_dly(200);

                    cpu_reset();

                    break;
                #if 1
                case DOORBELL_EVENT_ENTER_CLEAR_SYSTEM: // 清设备

                    sys_key_event_disable();
                    printf("\n DOORBELL_EVENT_ENTER_CLEAR_SYSTEM\n");
                    set_poweron_net_config_state(0);// 设置重新配网标志
                    set_cpu_reset_state(1);// 清设备信息标志
                    printf("\n wifi offs\n");
                  //  if(get_poweron_net_config_state()!=0){

                    wifi_off();
                  //  }
                    printf("\n wifi offe\n");
                 //   post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "NetCfgEnter.adp");

                    os_time_dly(200);

                    cpu_reset();
                #endif
                    break;

                case DOORBELL_EVENT_ENTER_NET_CONFIG_MODE:


                    printf("\n get_poweron_net_config_state()=========%d\n ",get_poweron_net_config_state());


                    if(get_poweron_net_config_state()==0){


                     set_poweron_net_config_state(0);


                    extern void doorbell_app_deal_network_status(int status);
                    doorbell_app_deal_network_status(NETWORK_CONFIG);


                    }

                    break ;


                case DOORBELL_EVENT_ENTER_COMPLETE_NETWORK_CONFIG:


                    puts("\n HI_CHANNEL_NETWORK_CONFIG \n");

                    //set_poweron_net_config_state(1);  //正在连接

                    extern void doorbell_app_deal_network_status(int status);
                    doorbell_app_deal_network_status(CONNECTING);


                    //video_dist_net_post_msg("net_dis_tate");

                    void qr_net_cfg_uninit(void);
                    qr_net_cfg_uninit();
                    extern int get_net_cfg_info(char *ssid, char *pwd, char *vcode);

                    char ssid[33];
                    char pwd[64];
                    get_net_cfg_info(ssid, pwd, NULL);
                    avsdk_wifi_sta_mode_info(ssid, pwd);

                  //  if(get_poweron_net_config_state()!=0){
                    printf("\n wifi off \n");
                    wifi_off();
                    printf("\n wifi on \n");
                    wifi_on();

                    //}

                    break ;
                case DOORBELL_EVENT_MONTOR_CONTROL_CONFIG:


                	printf("\n =========page=============msg[1]:%d,msg[2]:%d, %d,%s\n ",msg[1],msg[2],__LINE__,__FUNCTION__);
                	extern void step_montor_control(u8 dir,int req);
                 	step_montor_control(msg[2],0);

                    break;
                case  DOORBELL_EVENT_DEVICE_REBOOOT_CMD:  ///


                    #if  0
                    extern void test_host_mute();
                    extern s32 usb_host_speaker_set_mute(u8 mute);

                    static  int flag=0;
                    usb_host_speaker_set_mute(flag++%2);
                    test_host_mute();


                    #endif

                 // post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "rec.adp");

                // post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "stop_call.adp");

                    #ifdef LONG_POWER_IPC

                    os_time_dly(60);//等先应答app
                    cpu_reset();
                    #endif
                    break;
                case  DOORBELL_EVENT_DEVICE_SCREEN_CMD:

                    ipc_ExecIOTCMD_Req  *req_info = (ipc_ExecIOTCMD_Req *)msg[2];

                    #if 0
                    extern const char *background_cfg_file[];
                     dec_pro_file(background_cfg_file[msg[2]-1],1);
                     db_update("screen",msg[2]-1);
                     db_flush();
                    #else
                     extern const char *background_cfg_file[];
                     int ret=read_back_crc(background_cfg_file[req_info->iot_cmds.cmds[0].pint[1]-1],req_info->iot_cmds.cmds[0].pstr[0],req_info->iot_cmds.cmds[0].pstr[1]);
                     printf("\n ret======%d\n ",ret);
                     if(ret==-1){
                        break;
                     }
                     //int err=dec_pro_file(background_cfg_file[req_info->iot_cmds.cmds[0].pint[1]-1],1);
                     int err=dec_pro_file(background_cfg_file[ret],1);
                     printf("\n err==============%d\n",err);
                    // db_update("screen",req_info->iot_cmds.cmds[0].pint[1]-1);

                     db_update("screen",ret);
                     db_flush();
                    #endif




                    break;
                  case  DOORBELL_EVENT_DEVICE_UART_UPGRADE_UI_CMD:


                    printf("\n =========page=============msg[1]:%d,msg[2]:%d, %d,%s\n ",msg[1],msg[2],__LINE__,__FUNCTION__);
                    video_standby_post_msg("upgrade_show");
                    break;
                  case DOORBELL_EVENT_ENTER_CHECK_TIMER_REC:

                    puts(" \n CHECK_TIMER DOORBELL_EVENT_START_REC\n");
                        printf("\n get_poweron_net_config_state()8=================%d,%d\n",get_poweron_net_config_state(),get_cpu_reset_state());

                    doorbell_rec_check_timer_add();
                    doorbell_start_rec();
                    break ;


                #endif
                default:
                    break;
                }
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}



void doorbell_block_event_task(void *priv)
{
    int res;
    int msg[32];
    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE:
                    puts("\n DOORBELL_EVENT_GET_GET_WAKEUP_SOURCE\n");
                    extern void doorbell_app_deal_wakeup_source(int status,void *user);
                    doorbell_app_deal_wakeup_source(msg[2],(void *)msg[3]);
                    break;
                case DOORBELL_EVENT_REPORT_QOS:
                    puts("DOORBELL_EVENT_REPORT_QOS\n");
                    extern void doorbell_query_qos_timer_add(void);
                    doorbell_query_qos_timer_add();
                case DOORBELL_EVENT_REPORT_BATTARY:
#if 0
                    puts("DOORBELL_EVENT_REPORT_BATTARY\n");
#endif

                    extern u8 get_app_connect_flag(void);
                    if (get_app_connect_flag()) {
                        static int old_batteryLevel = 0, old_powerModel = 0, old_qos = 0;
//
                        if (!old_batteryLevel || msg[2]) {
//                                //第一次上报延时200ms
                            dev_base_info_t info = {0};
                            info.batteryLevel = system_get_battary_info_batteryLevel();
                            info.powerModel = system_get_battary_info_powerModel();
                            info.netQos = system_get_network_info_qos();
                            info.timeOffset = db_select("tzone");
                            info.ttcmdCount = 0;
                            avsdk_push_info(&info);
//
                            old_batteryLevel = system_get_battary_info_batteryLevel();
                            old_powerModel = system_get_battary_info_powerModel();
                            old_qos = system_get_network_info_qos();
                        } else if ((old_batteryLevel - system_get_battary_info_batteryLevel() > 50) || old_powerModel != system_get_battary_info_powerModel() || old_qos != system_get_network_info_qos()) {
//
                            dev_base_info_t info = {0};
                            info.batteryLevel = system_get_battary_info_batteryLevel();
                            info.powerModel = system_get_battary_info_powerModel();
                            info.netQos = system_get_network_info_qos();
                            info.timeOffset = db_select("tzone");
                            info.ttcmdCount = 0;
                            avsdk_push_info(&info);
//
                            old_batteryLevel = system_get_battary_info_batteryLevel();
                            old_powerModel = system_get_battary_info_powerModel();
                            old_qos = system_get_network_info_qos();
                        }

                  //  uint64_t get_utc_ms(void);
                  //  get_utc_ms();



                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
}

static OS_SEM query_battery_timer_sem;
void doorbell_query_battery_sem_post(void)
{
    os_sem_post(&query_battery_timer_sem);
}

void doorbell_heart_task(void *priv)
{

    int res;
    int msg[32];

    os_sem_create(&query_battery_timer_sem, 0);

    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case DOORBELL_EVENT_QUERY_BATTARY:
                    doorbell_query_battery_timer_add();
                    os_taskq_post("hi_channel_task", 1, HI_CHANNEL_CMD_GET_BATTERY_VALUE);
                    static int lost_heartbeat_cnt = 0;
                    if (os_sem_pend(&query_battery_timer_sem, 200) != OS_NO_ERR) {
                        lost_heartbeat_cnt++;
                        if (lost_heartbeat_cnt >= 5) {
                            printf("\nHI_CHANNEL_CMD_GET_BATTERY_VALUE no replay and lost_heartbeat_cnt >= 5 \n");
                            extern void hi3861l_reset(void);
                            hi3861l_reset();
                        }
                    } else {
                        lost_heartbeat_cnt = 0;
                    }
                    os_sem_set(&query_battery_timer_sem, 0);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void doorbell_7016_task(void *priv)
{
    int res;
    int msg[32];
    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case 0:
                    avsdk_recv_ble_data(msg[2],msg[3]);
                    free(msg[2]);
                    break;
                }
                break;
            }
            break;
        }
    }
}

int doorbell_event_init(void)//主要是create wifi 线程的
{

    puts("doorbell_event_init \n");
    //负责处理实时消息
    thread_fork("doorbell_event_task", 18, 0xC00+0xC00, 1024*2, 0, doorbell_event_task, NULL);
    //负责处理有可能出现阻塞或者不需要立马响应的消息
    thread_fork("doorbell_block_event_task", 18, 0xC00, 256, 0, doorbell_block_event_task, NULL);
    //负责一些需要定时去查询的消息
    thread_fork("doorbell_heart_task", 18, 0xC00, 256, 0, doorbell_heart_task, NULL);
#ifdef CONFIG_AC7016_ENABLE
    //负责转发7016的蓝牙数据
    thread_fork("doorbell_7016_task", 18, 0x1000, 256, 0, doorbell_7016_task, NULL);
#endif // CONFIG_AC7016_ENABLE
    return 0;
}
//late_initcall(doorbell_event_init);
