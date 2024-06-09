
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

#if defined CONFIG_ETH_ENABLE


#define CONFIG_ETH_RECV_TEST //仅供测试
//#define CONFIG_ETH_SEND_TEST
static void *dev = NULL ;

static int eth_state_cb(void *hdl, enum NETWORK_EVENT event, void *priv)
{
    if (ETH_EVENT_LINK_ON) {
        puts("eth on \r\n");
    } else if (ETH_EVENT_LINK_OFF) {
        puts("eth off \r\n");
    }
    return 0;
}

#include "ftpclient/ftpclient.h"
char mac_buf[1500] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static int ftp_read_dir_func(void *ftp_ctx, char *file_name, void *priv)
{
    printf("file_name=%s\n", file_name);
    return 0;


}


static int ftp_read_func(void *ftp_ctx, char *buf, unsigned int bufsize, void *priv)
{

    printf("len=>%d\n", bufsize);
    return bufsize;

}


#include "lwip/port/lwip.h"
static void wifi_set_lan_setting_info(void)
{
    struct lan_setting lan_setting_info = {

        .WIRELESS_IP_ADDR0  = 172,
        .WIRELESS_IP_ADDR1  = 16,
        .WIRELESS_IP_ADDR2  = 23,

#if defined (CONFIG_ETH_SEND_TEST)
        .WIRELESS_IP_ADDR3  = 151,
#endif
#if defined (CONFIG_ETH_RECV_TEST)
        .WIRELESS_IP_ADDR3  = 152,
#endif

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

#include "sock_api/sock_api.h"

static buffer[44 * 1460];
static void tcp_test_task(void *arg)
{
    void *socket_fd ;
    struct sockaddr_in addr;
    int ret = 0;


    socket_fd = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL);


    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("172.16.23.44");
    addr.sin_port = htons(17777);


    ret = sock_connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr));

    if (ret < 0) {
        printf("Connect fail\n");
        return ;


    }

    printf("connect successful!!!!\n");


    while (1) {

        ret = sock_send(socket_fd, buffer, 44 * 1460, 0);
        if (ret <= 0) {
            break;
        }
        /* msleep(10); */

    }

    printf("%s end \n\n\n", __func__);

}

static void udp_test_task(void *arg)
{
    void *socket_fd ;
    struct sockaddr_in addr;
    int ret = 0;


    socket_fd = sock_reg(AF_INET, SOCK_DGRAM, 0, NULL, NULL);


    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("172.16.23.44");
    addr.sin_port = htons(17777);


    while (1) {

        ret = sock_sendto(socket_fd, buffer, 1460 * 5, 0, (struct sockaddr *)&addr, sizeof(addr));


    }

    printf("%s end \n\n\n", __func__);

}




static void net_task(void *arg)
{
#if defined (CONFIG_ETH_SEND_TEST)
    char mac_addr[] = {0x9c, 0x37, 0xf4, 0x0e, 0x4d, 0xc8};
#endif

#if defined (CONFIG_ETH_RECV_TEST)
    char mac_addr[] = {0x9c, 0x37, 0xf4, 0x0e, 0x4d, 0xc9};
#endif
    dev = dev_open("eth0", NULL);

    dev_ioctl(dev, DEV_SET_MAC, (int)mac_addr);

    wifi_set_lan_setting_info();

#if defined (CONFIG_ETH_SEND_TEST) || defined (CONFIG_ETH_RECV_TEST)
    dev_ioctl(dev, DEV_NETWORK_START, STATIC_IP);
#else
    dev_ioctl(dev, DEV_NETWORK_START, DHCP_IP);
#endif

    struct cfg_info info;
    info.cb = eth_state_cb;
    dev_ioctl(dev, DEV_SET_CB_FUNC, (u32)&info);


#if defined (CONFIG_ETH_SEND_TEST)
    extern void net_app_init(void);
    net_app_init();
#endif
#if defined (CONFIG_ETH_RECV_TEST)
    extern void *net_ctp_init(void);
    net_ctp_init();
#endif

    while (1) {
        msleep(3000);

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

