
#include "os/os_compat.h"
#include "system/init.h"
#include "device/device.h"
#include "system/app_core.h"
#include "server/server_core.h"
#include "system/timer.h"
#include "app_config.h"
#include "wifi_dev.h"
#include "database.h"
#include "action.h"
#include "asm/gpio.h"
#include "gpio.h"
#include "lwip.h"
#include "lwip/inet.h"

#if defined CONFIG_LTE_ENABLE

static u8 mac_addr[6];

static void *dev = NULL ;
static OS_SEM sem;
static int adb_pid = 0, lte_pid = 0;

u32 wifi_network_get_scan_ssid(struct netdev_ssid_info *netdev_ssid_info, u32 ssid_cnt)
{
    return 0;
}
int wifi_network_sta_connect(char *sta_ssid, char *sta_pwd)
{
    return 0;
}
int wifi_network_req_scan(void)
{
    return 0;
}
u8 *wifi_get_module_mac_addr(void)
{
    return (u8 *)&mac_addr;
}
/*
 * LTE事件处理
 */


void adb_to_lte(void);
void lte_to_adb(void);
u8 adb_to_lte_check(void);
u8 lte_to_adb_check(void);
static void lte_app_init(void);
static void lte_app_uninit(void);

static void adb_to_lte_task(void *priv)
{
    for (;;) {
        if (!os_sem_accept(&sem)) {
            adb_pid = 0;
            break;
        }

        if (!adb_to_lte_check()) {
            adb_to_lte();
            break;
        }
        os_time_dly(1);
    }
}


static void lte_to_adb_task(void *priv)
{
    for (;;) {
        if (!os_sem_accept(&sem)) {
            lte_pid = 0;
            break;
        }

        if (!lte_to_adb_check()) {
            lte_to_adb();
            break;
        }
        extern void vPortYield(void);
        vPortYield();
    }
}


static void adb_to_lte_cue_start(void)
{
    if (adb_pid) {
        return;
    }
    thread_fork("adb_to_lte_task", 2, 512, 0, &adb_pid, adb_to_lte_task, NULL);
}


static void adb_to_lte_cue_stop(void)
{
    if (adb_pid) {
        return;
    }
    os_sem_post(&sem);
}


static void lte_to_adb_cue_start(void)
{
    if (lte_pid) {
        return;
    }
    thread_fork("lte_to_adb_task", 6, 512, 0, &lte_pid, lte_to_adb_task, NULL);
}

int   lwip_event_cb(void *lwip_ctx, enum LWIP_EVENT event)
{

    if(event == LWIP_LTE_DHCP_BOUND_SUCC){

        os_taskq_post("lte_event_task",1,0);
    }
    return 0;
}


static void lte_to_adb_cue_stop(void)
{
    if (lte_pid) {
        return;
    }
    os_sem_post(&sem);
}


static void lte_event_handler(struct sys_event *event)
{
    extern u32 ASCII_StrCmp(const char *src, const char *dst, u32 len);
    printf("%s, dev = %s, event = %d\n", __FUNCTION__, event->arg, event->u.dev.event);
    if (!ASCII_StrCmp(event->arg, "lte*", 5)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN: {
            dev = dev_open("lte0", 0);
            ASSERT(dev);
            void rndis_operatrions(void);
            rndis_operatrions();
            if (dev)  {
                //重新获取DHCP
                lwip_netif_set_up(LTE_NETIF);
                lwip_set_default_netif(LTE_NETIF);
                lwip_renew(LTE_NETIF, 1);
                //lte_app_init();

            }
            lte_to_adb_cue_start();
        }
        break;
        case DEVICE_EVENT_OUT: {
            if (dev) {
                //lte_app_uninit();
                lwip_dhcp_release_and_stop(LTE_NETIF);
                dev_close(dev);
                dev = NULL;
            }
            lte_to_adb_cue_stop();
        }

        }
    } else if (!ASCII_StrCmp(event->arg, "adb*", 3)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            adb_to_lte_cue_start();
            break;

        case DEVICE_EVENT_OUT:
            adb_to_lte_cue_stop();
            break;

        default:
            break;
        }
    }
}
/*
 * 静态注册设备事件回调函数，优先级为0
 */
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, lte_event_handler, 0);


static void ping_cb(void *priv, u32 cnt, u32 time)
{
    printf("cnt = %d, time = %d\n", cnt, time);
}


static void lte_app_init_task(void *priv)
{
    //for test
    //ping_init("120.24.247.138", 1000, 10, ping_cb, NULL);
    void net_app_init(void);
    //net_app_init();
}


static void lte_app_init(void)
{
    thread_fork("lte_app_init_task", 20, 0x1000, 0, 0, lte_app_init_task, NULL);
}


static void lte_app_uninit_task(void *priv)
{
    void net_app_uninit(void);
    //net_app_uninit();
}


static void lte_app_uninit(void)
{
    thread_fork("lte_app_uninit_task", 20, 0x1000, 0, 0, lte_app_uninit_task, NULL);
}

DEFINE_SPINLOCK(__lock);
static u32 cur_upload, cur_download, upload_rec, download_rec;

void set_cur_upload(int len)
{
    spin_lock(&__lock);
    cur_upload += len;
    spin_unlock(&__lock);
}


void set_cur_download(int len)
{
        spin_lock(&__lock);
cur_download+= len;
 spin_unlock(&__lock);

}
static void cal_updownload_timer(void)
{
         spin_lock(&__lock);
    static u32 cur_ts;
    u32 tdiff = time_lapse(&cur_ts, 1000);
    if (tdiff) {
        upload_rec = cur_upload;
        download_rec = cur_download;
        cur_upload = cur_download = 0;
    }
     spin_unlock(&__lock);
}

void wifi_rate_log_output2(void *priv)
{
    printf("\n 2WIFI U= %d KB/s, D= %d KB/s\r\n", upload_rec / 1024, download_rec / 1024);
}

#include "lwip\netdb.h"
void lte_event_task(void *priv)
{
    int res;
    int msg[32];
    char ssid[33];
    char pwd[64];

    sys_hi_timer_add(NULL,cal_updownload_timer,20);
    extern void wifi_rate_log_output2(void *priv);
        sys_timer_add(NULL, wifi_rate_log_output2, 1000);

    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case 0:
         const ip_addr_t *dns_getserver(u8_t numdns);
            ip_addr_t *__dnsserver;
            __dnsserver = dns_getserver(0);
            printf("\n dns1get = %s\n", inet_ntoa(__dnsserver->addr));

                    ip_addr_t dnsserver;
        dnsserver.addr = inet_addr("114.114.114.114");
        void dns_setserver(u8_t numdns, const ip_addr_t *dnsserver);
        dns_setserver(0, &dnsserver);


struct hostent *server;
         server = gethostbyname("baidu.com");
         if (server == NULL) {
                    printf("\nDNS lookup fail\n");
             continue;
         }

         struct ip4_addr *ip4_addr = NULL;
         ip4_addr = (struct ip4_addr *)server->h_addr_list[0];
         printf("\n1 DNS lookup succeeded. IP=%s\n", inet_ntoa(*ip4_addr));

            extern int cloud_platform_start(void);
            cloud_platform_start();

            //int error = get_update_data("http://cn.rom.xcthings.com:80/JLML/0/1.02.03.06_231116/JL_AC5X.bfu", 0, 0);


                    break;
                }
                break;
            }
            break;
        }
    }
}

static int lte_net_init(void)//主要是有线网卡线程的
{
    puts("lte early init \n\n\n\n\n\n");
    os_sem_create(&sem, 0);
    Init_LwIP(LTE_NETIF);

    thread_fork("lte_event_task", 17, 0x2E00, 256, 0, lte_event_task, NULL);
    return 0;
}

late_initcall(lte_net_init);
#endif


