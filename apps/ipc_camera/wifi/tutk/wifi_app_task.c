
#include "server/wifi_connect.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "os/os_compat.h"
#include "wifi_ext.h"
#include "system/init.h"
#include "lwip.h"
#include "device/device.h"
#include "system/app_core.h"
#include "server/server_core.h"
#include "action.h"
#include "system/timer.h"
#include "asm/debug.h"
#include "app_config.h"
#include "device/wifi_dev.h"
#include "system/timer.h"

#include "database.h"

#include "asm/gpio.h"
#include "gpio.h"





#define WIFI_APP_TASK_NAME "wifi_app_task"

enum WIFI_APP_MSG_CODE {
    WIFI_MSG_TICK_1_SEC,
    WIFI_MSG_SMP_CFG_COMPLETED,
    WIFI_MSG_STA_SCAN_COMPLETED,
    WIFI_MSG_STA_NETWORK_STACK_DHCP_SUCC,
};

extern u8 airkiss_calcrc_bytes(u8 *p, unsigned int num_of_bytes);
extern unsigned int time_lapse(unsigned int *handle, unsigned int time_out);
static u8 mac_addr[6];
static char g_ssid[32];
static char g_pwd[64];
static void *wifi_dev = NULL;
static struct airkiss_result {
    struct smp_cfg_result result;
    char scan_ssid_found;
} airkiss_result;

static struct voiceprint_result {
    char rand_str[8];
} voiceprint_result;

void wifi_smp_connect(char *ssid, char *pwd, void *rand_str)
{
    struct cfg_info info = {0};
    if (ssid) {
        strcpy(voiceprint_result.rand_str, rand_str);
        info.mode = STA_MODE;
        info.ssid = ssid;
        info.pwd = pwd;
        dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
        dev_ioctl(wifi_dev, DEV_CHANGE_SAVING_MODE, (u32)&info);
    } else {

        dev_ioctl(wifi_dev, DEV_GET_WIFI_SMP_RESULT, (u32)&info);

        if (info.smp_cfg.type == AIRKISS_SMP_CFG) {
            printf("\r\n AIRKISS INFO, SSID = %s, PWD = %s, ssid_crc = 0x%x, ran_val = 0x%x \r\n", info.smp_cfg.ssid, info.smp_cfg.passphrase, info.smp_cfg.ssid_crc, info.smp_cfg.random_val);
            airkiss_result.result.type = AIRKISS_SMP_CFG ;
            airkiss_result.result.ssid_crc = info.smp_cfg.ssid_crc;
            airkiss_result.result.random_val = info.smp_cfg.random_val;
            strcpy(airkiss_result.result.ssid, info.smp_cfg.ssid);
            strcpy(airkiss_result.result.passphrase, info.smp_cfg.passphrase);

            if (airkiss_result.result.ssid_crc == airkiss_calcrc_bytes((u8 *)airkiss_result.result.ssid, strlen(airkiss_result.result.ssid))) {
                airkiss_result.scan_ssid_found = 1;
                info.mode = STA_MODE;
                info.ssid = info.smp_cfg.ssid;
                info.pwd = info.smp_cfg.passphrase;
                dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
                dev_ioctl(wifi_dev, DEV_CHANGE_SAVING_MODE, (u32)&info);
            } else {
                dev_ioctl(wifi_dev, DEV_NET_SCANF, 0);
            }
        } else {
            info.mode = STA_MODE;
            info.ssid = info.smp_cfg.ssid;
            info.pwd = info.smp_cfg.passphrase;
            dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
            dev_ioctl(wifi_dev, DEV_CHANGE_SAVING_MODE, (u32)&info);
        }
    }
}

static void airkiss_ssid_check(void)
{
    u32 i;
    struct cfg_info info = {0};

    if (airkiss_result.result.type != AIRKISS_SMP_CFG ||  airkiss_result.scan_ssid_found) {
        return;
    }

    dev_ioctl(wifi_dev, DEV_GET_STA_SSID_INFO, (u32)&info);

    for (i = 0; i < info.sta_ssid_num; i++) {
        if (!strncmp(airkiss_result.result.ssid, info.sta_ssid_info[i].ssid, strlen(airkiss_result.result.ssid))) {
CHECK_AIRKISS_SSID_CRC:
            if (airkiss_result.result.ssid_crc == airkiss_calcrc_bytes((u8 *)info.sta_ssid_info[i].ssid, strlen(info.sta_ssid_info[i].ssid))) {
                printf("find airkiss ssid = [%s]\r\n", info.sta_ssid_info[i].ssid);
                strcpy(airkiss_result.result.ssid, info.sta_ssid_info[i].ssid);
                airkiss_result.scan_ssid_found = 1;

                info.mode = STA_MODE;
                info.ssid = airkiss_result.result.ssid;
                info.pwd = airkiss_result.result.passphrase;
                dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
                dev_ioctl(wifi_dev, DEV_CHANGE_SAVING_MODE, (u32)&info);
                return;
            }
        } else {
            /*goto CHECK_AIRKISS_SSID_CRC;*/
        }
    }

    printf("cannot found airkiss ssid[%s] !!! \n\n", airkiss_result.result.ssid);
}

static void airkiss_broadcast(void)
{
    int i, ret;
    int onOff = 1;
    int sock;
    struct sockaddr_in dest_addr;

    if (airkiss_result.result.type != AIRKISS_SMP_CFG) {
        return;
    }

    puts("airkiss_broadcast random_val \n");

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        printf("%s %d->Error in socket()\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_port = 0;
    ret = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (ret == -1) {
        printf("%s %d->Error in bind()\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }

    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                     (char *)&onOff, sizeof(onOff));
    if (ret == -1) {
        printf("%s %d->Error in setsockopt() SO_BROADCAST\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    inet_pton(AF_INET, "255.255.255.255", &dest_addr.sin_addr.s_addr);
    dest_addr.sin_port = htons(10000);

    for (i = 0; i < 50; i++) {
        ret = sendto(sock, (unsigned char *)&airkiss_result.result.random_val, 1, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
        if (ret == -1) {
            printf("%s %d->Error in sendto\n", __FUNCTION__, __LINE__);
        }
        msleep(20);
    }

    memset(&airkiss_result, 0, sizeof(airkiss_result));

EXIT:
    if (sock != -1) {
        closesocket(sock);
    }
}
u32 led_hdl = 0;
static void led_s()
{
    static int ret = 0;
    ret ^= 1;
    printf("ret=%d\n", ret);
    gpio_direction_output(IO_PORTA_05, ret);

}

static int network_user_callback(void *network_ctx, enum NETWORK_EVENT state, void *priv)
{
    struct cfg_info info = {0};

    switch (state) {

    case WIFI_EVENT_MODULE_INIT:

#if 0
        gpio_set_hd(IO_PORTG_00, 0);
        gpio_set_hd(IO_PORTG_01, 0);
        gpio_set_hd(IO_PORTG_02, 0);
        gpio_set_hd(IO_PORTG_03, 0);
        gpio_set_hd(IO_PORTG_04, 0);
        gpio_set_hd(IO_PORTG_05, 0);

#endif


//wifi module port seting
        info.port_status = 0;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);

        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_CS, (u32)&info);

        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_WKUP, (u32)&info);

        msleep(100);

        info.mode = AP_MODE;
        info.force_default_mode = 0;

        info.ssid = "xxxx";
        info.pwd = "";


        dev_ioctl(wifi_dev, DEV_SET_DEFAULT_MODE, (u32)&info);
        break;

    case WIFI_EVENT_MODULE_START:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START\n");

//void linked_info_dump(int padapter, u8 benable);
//void *netdev_priv(struct net_device *dev);
//void  *net_dev_find(void);
//linked_info_dump(*(int*)netdev_priv(net_dev_find()), 1);



//设置VM保存AP_MODE
#if defined (WIFI_CAM_SUFFIX)

        sprintf(g_ssid, WIFI_CAM_PREFIX WIFI_CAM_SUFFIX);

#else
        dev_ioctl(wifi_dev, DEV_GET_MAC, (u32)&mac_addr);
        sprintf(g_ssid, "wifi_camera_ac57_%02x%02x%02x%02x%02x%02x"
                , mac_addr[0]
                , mac_addr[1]
                , mac_addr[2]
                , mac_addr[3]
                , mac_addr[4]
                , mac_addr[5]);

#endif


        info.ssid = g_ssid;
        info.pwd = "12345678";
        info.mode = AP_MODE;
        info.force_default_mode = 0;
        dev_ioctl(wifi_dev, DEV_SAVE_DEFAULT_MODE, (u32)&info);

        break;

    case WIFI_EVENT_MODULE_STOP:
        puts("|network_user_callback->WIFI_EVENT_MODULE_STOP\n");
        break;

    case WIFI_EVENT_AP_START:
        puts("|network_user_callback->WIFI_EVENT_AP_START\n");
        gpio_direction_output(IO_PORTA_05, 0);

        break;

    case WIFI_EVENT_AP_STOP:
        puts("|network_user_callback->WIFI_EVENT_AP_STOP\n");

#if 0//8801要if 1，8189不能开，开了切换不了模式
        info.port_status = 0;
        printf("---------DEV_SET_WIFI_POWER-OFF-----------\r\n");
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        printf("---------DEV_SET_WIFI_POWER-ON-----------\r\n");
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
#endif
        break;

    case WIFI_EVENT_STA_START:
        puts("|network_user_callback->WIFI_EVENT_STA_START\n");
        led_hdl = sys_timer_add(NULL, led_s, 300);
        break;
    case WIFI_EVENT_MODULE_START_ERR:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START_ERR\n");
        break;

    case WIFI_EVENT_STA_STOP:
        puts("|network_user_callback->WIFI_EVENT_STA_STOP\n");
#if 0
        info.port_status = 0;
        printf("---------DEV_SET_WIFI_POWER-OFF-----------\r\n");
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        printf("---------DEV_SET_WIFI_POWER-ON-----------\r\n");
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
#endif
        break;

    case WIFI_EVENT_STA_DISCONNECT:
        puts("|network_user_callback->WIFI_STA_DISCONNECT\n");
        break;

    case WIFI_EVENT_STA_SCAN_COMPLETED:

        /* if(wpa_supplicant_get_state() != STA_WPA_CONNECT_COMPLETED) */
        /* { */
        puts("|network_user_callback->WIFI_STA_SCAN_COMPLETED\n");
        os_taskq_post(WIFI_APP_TASK_NAME, 1, WIFI_MSG_STA_SCAN_COMPLETED);
        /* } */
        break;

    case WIFI_EVENT_STA_CONNECT_SUCC:
        dev_ioctl(wifi_dev, DEV_GET_WIFI_CHANNEL, (u32)&info);
        printf("|network_user_callback->WIFI_STA_CONNECT_SUCC,CH=%d\r\n", info.sta_channel);
        break;

    case WIFI_EVENT_MP_TEST_START:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_START\n");
        break;
    case WIFI_EVENT_MP_TEST_STOP:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_STOP\n");
        break;

    case WIFI_EVENT_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID:
        puts("|network_user_callback->WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID\n");
        break;
    case WIFI_EVENT_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL:
        puts("|network_user_callback->WIFI_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL\n");
        break;
    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_SUCC\n");
        os_taskq_post(WIFI_APP_TASK_NAME, 1, WIFI_MSG_STA_NETWORK_STACK_DHCP_SUCC);
        break;
    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_TIMEOUT\n");
        break;

    case WIFI_EVENT_P2P_START:
        puts("|network_user_callback->WIFI_EVENT_P2P_START\n");
        break;
    case WIFI_EVENT_P2P_STOP:
        puts("|network_user_callback->WIFI_EVENT_P2P_STOP\n");
        break;
    case WIFI_EVENT_P2P_GC_DISCONNECTED:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_DISCONNECTED\n");
        break;
    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC\n");
        break;
    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT\n");
        break;

    case WIFI_EVENT_SMP_CFG_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_TIMEOUT\n");
        break;
    case WIFI_EVENT_SMP_CFG_COMPLETED:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_COMPLETED\n");
        os_taskq_post(WIFI_APP_TASK_NAME, 1, WIFI_MSG_SMP_CFG_COMPLETED);
        break;

    case WIFI_EVENT_PM_SUSPEND:
        puts("|network_user_callback->WIFI_EVENT_PM_SUSPEND\n");
        break;
    case WIFI_EVENT_PM_RESUME:
        puts("|network_user_callback->WIFI_EVENT_PM_RESUME\n");
        break;

    case WIFI_EVENT_AP_ON_ASSOC:
        ;
        struct eth_addr *hwaddr = (struct eth_addr *)network_ctx;
        printf("WIFI_EVENT_AP_ON_ASSOC hwaddr = %02x:%02x:%02x:%02x:%02x:%02x \r\n\r\n",
               hwaddr->addr[0], hwaddr->addr[1], hwaddr->addr[2], hwaddr->addr[3], hwaddr->addr[4], hwaddr->addr[5]);
        break;
    case WIFI_EVENT_AP_ON_DISCONNECTED:
        hwaddr = (struct eth_addr *)network_ctx;
        printf("WIFI_EVENT_AP_ON_DISCONNECTED hwaddr = %02x:%02x:%02x:%02x:%02x:%02x \r\n\r\n",
               hwaddr->addr[0], hwaddr->addr[1], hwaddr->addr[2], hwaddr->addr[3], hwaddr->addr[4], hwaddr->addr[5]);

        break;
    default:
        break;
    }

    return 0;
}


static void wifi_set_lan_setting_info(void)
{
    struct lan_setting lan_setting_info = {

        .WIRELESS_IP_ADDR0  = 192,
        .WIRELESS_IP_ADDR1  = 168,
        .WIRELESS_IP_ADDR2  = 1,
        .WIRELESS_IP_ADDR3  = 1,

        .WIRELESS_NETMASK0  = 255,
        .WIRELESS_NETMASK1  = 255,
        .WIRELESS_NETMASK2  = 255,
        .WIRELESS_NETMASK3  = 0,

        .WIRELESS_GATEWAY0  = 192,
        .WIRELESS_GATEWAY1  = 168,
        .WIRELESS_GATEWAY2  = 1,
        .WIRELESS_GATEWAY3  = 1,

        .SERVER_IPADDR1  = 192,
        .SERVER_IPADDR2  = 168,
        .SERVER_IPADDR3  = 1,
        .SERVER_IPADDR4  = 1,

        .CLIENT_IPADDR1  = 192,
        .CLIENT_IPADDR2  = 168,
        .CLIENT_IPADDR3  = 1,
        .CLIENT_IPADDR4  = 2,

        .SUB_NET_MASK1   = 255,
        .SUB_NET_MASK2   = 255,
        .SUB_NET_MASK3   = 255,
        .SUB_NET_MASK4   = 0,
    };

    struct cfg_info info;
    info.__lan_setting_info = &lan_setting_info;
    dev_ioctl(wifi_dev, DEV_SET_LAN_SETTING, (u32)&info);
}



void net_app_init(void)
{

    extern dhcps_offer_dns();

    dhcps_offer_dns();
    printf("get_cur_wifi_info()->mode=%d\n", get_cur_wifi_info()->mode);
    extern time_t time(time_t *timer);
    time(0);
    extern int tutk_platform_init(const char *username, const char *password);
    tutk_platform_init("aaaa", "12345678");




}
void net_app_uninit(void)
{

}

void wifi_on(void)
{
    dev_ioctl(wifi_dev, DEV_NETWORK_START, 0);
    net_app_init();
}


void wifi_off(void)
{
    net_app_uninit();
    dev_ioctl(wifi_dev, DEV_NETWORK_STOP, 0);
}

#ifdef CONFIG_QRCODE_ENABLE
#include "qrcode.h"
#include "get_image_data.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
static void *decoder = NULL;
static u8 scan_enable = 1;
void qrcode_process(char *inputFrame)
{
    static char *last_buf = NULL;
    static int last_size = 0;
    static int last_jiffies = 0;
    char m_ssid[64];
    char m_pwd[64];

    unsigned int start_time = jiffies;
    char *buf;
    int buf_size = 0;
    int ret = 0;
    json_object *new_obj = NULL; //
    json_object *ssid = NULL;
    json_object *pwd = NULL;


    if (!decoder) {
        log_e("qrcode not init");
        return;
    }
    if (!scan_enable) {
        return;
    }


    //md_detected：是否检测到运动物体
    int md_detected = 0;
    qrcode_detectAndDecode(decoder, inputFrame, &md_detected);

    //printf("md = %d\n",md_detected);

    ret = qrcode_get_result(decoder, &buf, &buf_size);
    //printf("\n buf_size================%d\n",buf_size);
    if (buf_size > 7 && ret == 0) {
        //过滤低于8位的码，以免误判
        if (buf[buf_size - 1] != 0x0A) {
            buf[buf_size] = 0x0A;
            buf_size++;
        }
        buf[buf_size] = 0;

        printf("decode[in %4dms,buf_size=%d]: %s\n", jiffies_to_msecs(jiffies - start_time), buf_size, buf);
        //printf("decode[in %4dms]:\n", jiffies_to_msecs(jiffies - start_time) );
        if (jiffies_to_msecs(jiffies - last_jiffies) < (120)) {
            if (last_size == buf_size) {
                if (last_buf != NULL) {
                    if (!strncmp(last_buf, buf, buf_size)) {
                        //规定时间内过滤相同的码
                        return;
                    }
                }
            }
        }

        scan_enable = 0;
        new_obj = json_tokener_parse(buf);
        ssid =  json_object_object_get(new_obj, "ssid");
        pwd =  json_object_object_get(new_obj, "pwd");

        const char *ssid = json_object_get_string(json_object_object_get(new_obj, "ssid"));
        const char *pwd = json_object_get_string(json_object_object_get(new_obj, "pwd"));
        strcpy(m_ssid, ssid);
        strcpy(m_pwd, pwd);
        json_object_put(new_obj);



        printf("m_ssid:%s   m_pwd:%s\n", m_ssid, m_pwd);
        struct cfg_info info = {0};
        info.mode = STA_MODE;
        info.ssid = m_ssid;
        info.pwd  = m_pwd;
        info.force_default_mode  = 1;
        dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);

    } else {
        //printf("no image[in %4dms]",((jiffies - start_time)*2));
    }


    return;
}
#endif




void wifi_enter_smp_cfg_mode(void)
{

#ifdef CONFIG_QRCODE_ENABLE
    decoder = qrcode_init(YUV_DATA_WIDTH, YUV_DATA_HEIGHT, YUV_DATA_WIDTH, QRCODE_MODE_NORMAL/* QRCODE_MODE_FAST*/, SYM_QRCODE, 2, 60, SYM_DIFF, SYM_RIFILE_BOLT);
    get_yuv_init(qrcode_process);
#endif

    struct cfg_info info = {0};
    info.timeout = 100;
    dev_ioctl(wifi_dev, DEV_SMP_MODE, 0);
    dev_ioctl(wifi_dev, DEV_SET_SMP_CONFIG_TIMEOUT_SEC, (u32)&info);

}

int get_wifi_is_smp_mode(void)
{
    struct cfg_info info = {0};
    info.mode = NONE_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&info);
    return info.mode == SMP_CFG_MODE;
}

void wifi_return_sta_mode(void)
{
    struct cfg_info info = {0};
    info.mode = STA_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&info);
    dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
}


static void wifi_app_timer_func(void *p)
{
    os_taskq_post(WIFI_APP_TASK_NAME, 1, WIFI_MSG_TICK_1_SEC);
}


void wifi_app_task(void *priv)
{
    int msg[32];
    int res;
    unsigned int timehdl = 0;
    struct cfg_info info = {0};

#ifdef PRODUCT_TEST_ENABLE
    u8 product_enter_check(void);
    if (product_enter_check()) {
        //进入产测模式后，将直接退出
        return;
    }
#endif
    /* os_time_dly(1000); */

//wifi app start
    wifi_dev = dev_open("wifi", NULL);


    info.cb = network_user_callback;
    info.net_priv = NULL;
    dev_ioctl(wifi_dev, DEV_SET_CB_FUNC, (u32)&info);

#if 0
    printf("\n >>>> DEV_SET_WIFI_POWER_SAVE<<<<   \n");
    dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER_SAVE, 0);//打开就启用低功耗模式, 只有STA模式才有用
#endif

#if 0
    printf("\n >>>> DEV_SET_WIFI_TX_PWR_BY_RATE<<<   \n");

    info.tx_pwr_lmt_enable = 0;//  解除WIFI发送功率限制
    dev_ioctl(wifi_dev, DEV_SET_WIFI_TX_PWR_LMT_ENABLE, (u32)&info);
    info.tx_pwr_by_rate = 1;// 设置WIFI根据不同datarate打不同power
    dev_ioctl(wifi_dev, DEV_SET_WIFI_TX_PWR_BY_RATE, (u32)&info);
#endif

    wifi_set_lan_setting_info();

    wifi_on();


    sys_timer_add(NULL, wifi_app_timer_func, 1000);

    while (1) {
        res = os_task_pend("taskq",  msg, ARRAY_SIZE(msg));

        switch (msg[1]) {
        case WIFI_MSG_TICK_1_SEC:

            if (time_lapse(&timehdl, 2 * 1000)) {
                extern void malloc_stats(void);
                if (wifi_module_is_init()) {
                    malloc_stats();
                    printf("WIFI U= %d KB/s, D= %d KB/s\r\n", wifi_get_upload_rate() / 1024, wifi_get_download_rate() / 1024);
                }
            }

            break;


        case WIFI_MSG_SMP_CFG_COMPLETED:
            memset(&airkiss_result, 0, sizeof(airkiss_result));
            wifi_smp_connect(NULL, NULL, NULL);

#ifdef CONFIG_QRCODE_ENABLE
            {
                get_yuv_uninit();
                qrcode_deinit(decoder);
            }
#endif

            break;

        case WIFI_MSG_STA_SCAN_COMPLETED:
            airkiss_ssid_check();
            break;
        case WIFI_MSG_STA_NETWORK_STACK_DHCP_SUCC:
            airkiss_broadcast();
            sys_timer_del(led_hdl);

            gpio_direction_output(IO_PORTA_05, 0);


            break;

        default :
            break;


        }
    }
}

int wireless_net_init(void)//主要是create wifi 线程的
{
    puts("wifi early init \n\n\n\n\n\n");
    thread_fork(WIFI_APP_TASK_NAME, 10, 0x4000, 64, 0, wifi_app_task, NULL);
    return 0;
}

#if defined CONFIG_WIFI_ENABLE
late_initcall(wireless_net_init);
#endif

