#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "qrcode.h"
#include "get_image_data.h"
#include "hi3861l_task.h"
#include "gpio.h"
#include "doorbell_event.h"
#define _STDIO_H_
#include "xciot_api.h"
typedef struct net_cfg_info {
    char ssid[33];
    char pwd[64];
    char vcode[13];
} NET_CFG_INFO;

static void *decoder = NULL;
static NET_CFG_INFO info;

void set_net_cfg_info(char *ssid, char *pwd, char *vcode)
{
    if (ssid) {
        strcpy(info.ssid, ssid);
    }
    if (pwd) {
        strcpy(info.pwd, pwd);
    }
    if (vcode) {
        strcpy(info.vcode, vcode);
    }

}

int get_net_cfg_info(char *ssid, char *pwd, char *vcode)
{
    if (!info.ssid[0]) {
        return -1;
    }
    if (ssid) {
        strcpy(ssid, info.ssid);
    }
    if (pwd) {
        strcpy(pwd, info.pwd);
    }
    if (vcode) {
        strcpy(vcode, info.vcode);
    }
    return 0;
}


void qrcode_process(char *inputFrame)
{
    char *buf;
    int buf_size = 0;
    int ret = 0;
    char ssid[33];
    char pwd[64];
    char vcode[13];
    int ssid_len;
    int pwd_len;
    int offset = 0;
    int enc_type = 0;

    if (!decoder) {
        log_e("qrcode not init");
        return;
    }
    //md_detected   ：   是否检测到运动物体
    int md_detected = 0;
    qrcode_detectAndDecode(decoder, inputFrame, &md_detected);
  //  printf("md_detected:%d\n",md_detected);
    ret = qrcode_get_result(decoder, &buf, &buf_size, &enc_type);
    if (buf_size) {
        printf("\n buf================%s \n", buf);
        ipc_netcfg_t wifi = {0};
        int err=avsdk_qr_netcfg_decode(buf,buf_size,&wifi);
        if(err){
            return ;
        }
        if (decoder) {
            qrcode_deinit(decoder);
            decoder = NULL;
        }
        printf("\nssid =  %s pwd =  %s vcode =  %s\n", wifi.ssid, wifi.pwd, wifi.vcode);
        void set_net_cfg_info(char *ssid, char *pwd, char *vcode);
        set_net_cfg_info(wifi.ssid, wifi.pwd, wifi.vcode);
		#ifndef LONG_POWER_IPC
        os_taskq_post("hi_channel_task", 1, HI_CHANNEL_NETWORK_CONFIG);
        #else

        post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_ENTER_COMPLETE_NETWORK_CONFIG);
	    #endif
        post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "SsidRecv.adp");
#if 0
        ssid_len = ((buf[offset] - '0') * 10) + (buf[offset + 1] - '0');
        int min_remain = buf_size - 2 - ssid_len - 2 - 0;//(默认没有密码)
        printf("\n ssid_len = %d min_remain = %d\n", ssid_len, min_remain);
        if (ssid_len > 32 || min_remain  < 6) {
            return;
        }
        memcpy(ssid, buf + 2, ssid_len);
        ssid[ssid_len] = 0;
        offset = 2 + ssid_len;
        pwd_len = ((buf[offset] - '0') * 10) + (buf[offset + 1] - '0');
        min_remain = buf_size - 2 - ssid_len - 2 - pwd_len;
        printf("\n pwd_len = %d min_remain = %d\n", pwd_len, min_remain);
        if (pwd_len > 63 || min_remain < 6) {
            return;
        }
        memcpy(pwd, buf + 2 + ssid_len + 2, pwd_len);
        pwd[pwd_len] = 0;

        memcpy(vcode, buf + 2 + ssid_len + 2 + pwd_len, buf_size - (2 + ssid_len + 2 + pwd_len));

        vcode[buf_size - (2 + ssid_len + 2 + pwd_len)] = 0;
        if (decoder) {
            qrcode_deinit(decoder);
            decoder = NULL;
        }
        printf("\nssid =  %s pwd =  %s vcode =  %s\n", ssid, pwd, vcode);
        void set_net_cfg_info(char *ssid, char *pwd, char *vcode);
        set_net_cfg_info(ssid, pwd, vcode);
        os_taskq_post("hi_channel_task", 1, HI_CHANNEL_NETWORK_CONFIG);
        post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "SsidRecv.adp");
#endif
    }
}

void net_config_before(void)
{
    extern int db_reset();;
    db_reset();
    extern void clean_fastdata_to_flash(void);
    extern void clear_wifi_info();
    clear_wifi_info();
    clean_fastdata_to_flash();
    clean_backup_data_to_flash();
    extern void clear_time_data();
    clear_time_data();

    int vol=db_select("vol");
    printf("\n  reset_dev_vol==================%d\n",vol);

    u8 data[2];
    //设置音量
    data[0] = 0x80;//音量
    data[1] = 14; //最大音量

    printf("\n reset_vol ===============%d\n",vol);
    cdc_write_data(data,sizeof(data));



}

iot_bdn_ref_t ble_hdl = NULL;

int avsdk_recv_ble_data(u8 *data,int data_len)
{
    if(ble_hdl){
  //  put_buf(data,data_len);
        avsdk_bdn_recv(ble_hdl,data,data_len);
    }
    return 0;
}




static int dev_ble_connect_wifi(char *ssid,char *pwd)
{
    static int flag;
      printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>%s %d\n",ssid,pwd);
    if(flag == 0){
        flag = 1;
        void set_net_cfg_info(char *ssid, char *pwd, char *vcode);
        set_net_cfg_info(ssid, pwd, NULL);
        #ifndef LONG_POWER_IPC
        os_taskq_post("hi_channel_task", 1, HI_CHANNEL_NETWORK_CONFIG);
        #else

        post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_ENTER_COMPLETE_NETWORK_CONFIG);
	    #endif



    }


    return 0;
}

 extern int set_7016_ble_name_on();
void qr_net_cfg_init(void)
{
    //不镜像
    decoder = qrcode_init(YUV_DATA_WIDTH, YUV_DATA_HEIGHT, YUV_DATA_WIDTH, QRCODE_MODE_NORMAL/* QRCODE_MODE_FAST*/, 60, 2, 0);
    //垂直镜像
  //  decoder = qrcode_init(YUV_DATA_WIDTH, YUV_DATA_HEIGHT, YUV_DATA_WIDTH, QRCODE_MODE_NORMAL/* QRCODE_MODE_FAST*/, 60, 2,1);
    //水平镜像
 //   decoder = qrcode_init(YUV_DATA_WIDTH, YUV_DATA_HEIGHT, YUV_DATA_WIDTH, QRCODE_MODE_NORMAL/* QRCODE_MODE_FAST*/, 60, 2,2);
    if (!decoder) {
        printf("\n qrdecode init err\n");
    }

    net_config_before();

#ifdef CONFIG_AC7016_ENABLE

    extern int send_ble_data_to_7016(u8 *data,int data_len);   //
    avsdk_bdn_init(&ble_hdl,
                   send_ble_data_to_7016, dev_ble_connect_wifi);
#if 1
    set_7016_ble_name_on();//设置蓝牙名,从机则会打开蓝牙
#else
    set_7016_ble_on();

#endif
     // set_7016_ble_on();
   // sys_timeout_add(NULL,set_7016_ble_name_on , 6000);
#endif // CONFIG_AC7016_ENABLE

    get_yuv_init(qrcode_process);

}


void qr_net_cfg_uninit(void)
{

    printf("\n qr_net_cfg_uninit \n");
    get_yuv_uninit();
#ifdef CONFIG_AC7016_ENABLE
    msleep(1000);
    set_7016_ble_off();
    avsdk_bdn_fini(&ble_hdl);
#endif // CONFIG_AC7016_ENABLE
}
#if 0
#include "fs.h"
void qr_test_thread(void *priv)
{
    int buf_size = 0;
    int ret = 0;
    int ssid_len;
    int pwd_len;
    int offset = 0;
    int enc_type = 0;
    decoder = qrcode_init(YUV_DATA_WIDTH, YUV_DATA_HEIGHT, YUV_DATA_WIDTH, QRCODE_MODE_NORMAL/* QRCODE_MODE_FAST*/, 60, 2, 0);
    void *fd = fopen(CONFIG_ROOT_PATH"SAVE.YUV", "r");
    if (!fd) {
        while (1) {
            putchar('A');
        }
    }
    static u8 buf[460800];
    char *buffer;
    int i = 0;
    while (1) {
        ret = fread(fd, buf, sizeof(buf));
        if (ret == 0) {
            break;
        }
        printf("\n ret = %d\n", ret);
        int md_detected = 0;
        printf("\n >>>>>>>>>>>>>>>>>i = %d\n", i);
        u8 *inputFrame = buf;
        qrcode_detectAndDecode(decoder, inputFrame, &md_detected);
        ret = qrcode_get_result(decoder, &buffer, &buf_size, &enc_type);
        if (buf_size) {
            printf("\n buf================%s \n", buffer);
            free(buffer);
        }
        memset(buf, 0, sizeof(buf));
        i ++;
        printf("\n >>>>>>>>>>>>>>>>>t = %d\n", i);
    }
}
void qr_test()
{
    extern int thread_fork(const char *thread_name, int prio, int stk_size, u32 q_size, int *pid, void (*func)(void *), void *parm);
    thread_fork("qr_test_thread", 8, 0x2000, 0, 0, qr_test_thread, NULL);
}
#endif
