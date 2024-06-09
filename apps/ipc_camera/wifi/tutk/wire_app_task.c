
#include "os/os_compat.h"
#include "system/init.h"
#include "system/timer.h"
#include "app_config.h"
#include "wifi_dev.h"
#include "lwip/port/lwip.h"


#if defined CONFIG_ETH_ENABLE
static void *dev = NULL ;


static void wifi_set_lan_setting_info(void)
{
    struct lan_setting lan_setting_info = {

        .WIRELESS_IP_ADDR0  = 172,
        .WIRELESS_IP_ADDR1  = 16,
        .WIRELESS_IP_ADDR2  = 23,
        .WIRELESS_IP_ADDR3  = 151,

        .WIRELESS_NETMASK0  = 255,
        .WIRELESS_NETMASK1  = 255,
        .WIRELESS_NETMASK2  = 255,
        .WIRELESS_NETMASK3  = 0,

        .WIRELESS_GATEWAY0  = 172,
        .WIRELESS_GATEWAY1  = 16,
        .WIRELESS_GATEWAY2  = 23,
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
    dev_ioctl(dev, DEV_SET_LAN_SETTING, (u32)&info);
}




static int eth_state_cb(void *hdl, enum NETWORK_EVENT event, void *priv)
{
    if (ETH_EVENT_LINK_ON) {
        puts("eth on \r\n");

    } else if (ETH_EVENT_LINK_OFF) {
        puts("eth off \r\n");
    }
    return 0;
}




static void net_task(void *arg)
{


    char mac_addr[] = {0x9c, 0x37, 0xf4, 0x0e, 0x4d, 0xc8};
    dev = dev_open("eth0", NULL);

    dev_ioctl(dev, DEV_SET_MAC, (int)mac_addr);

    wifi_set_lan_setting_info();

    dev_ioctl(dev, DEV_NETWORK_START, DHCP_IP);

    struct cfg_info info;
    info.cb = eth_state_cb;
    dev_ioctl(dev, DEV_SET_CB_FUNC, (u32)&info);

    /*     extern time_t time(time_t *timer); */
    /* time(0); */
    /* extern int tutk_platform_init(const char *username, const char *password); */
    /* tutk_platform_init("aaaa", "12345678"); */



    while (1) {
        msleep(1000);

    }

}

static int wire_net_init(void)//主要是有线网卡线程的
{
    puts("wire early init \n\n\n\n\n\n");
    thread_fork("net_task", 20, 0x1000, 0, 0, net_task, NULL);
    return 0;
}
late_initcall(wire_net_init);
#endif

