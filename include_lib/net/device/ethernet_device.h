#ifndef ETHERNET_DEV_IF_H
#define ETHERNET_DEV_IF_H

#include "typedef.h"



enum wifi_chip_mode {
    WIFI_CHIP_MODE_INIT,
    WIFI_CHIP_MODE_AP,
    WIFI_CHIP_MODE_STA,
    WIFI_CHIP_MODE_P2P,
    WIFI_CHIP_MODE_TEST,
    WIFI_CHIP_MODE_SMP,
};

enum wifi_p2p_role {
    WIFI_P2P_ROLE_DEVICE,
    WIFI_P2P_ROLE_CLIENT,
};


struct wifi_chip {
    int (*init)();
    int (*exit_mode)(enum wifi_chip_mode);
    int (*enter_ap_mode)(const char *ssid, const char *password);
    int (*enter_sta_mode)(const char *ssid, const char *password);
    int (*enter_p2p_mode)(enum wifi_p2p_role);
    int (*enter_test_mode)();
    int (*enter_smp_mode)();
};







struct ethernet_device;
union ethdev_cfg;


typedef void (*eth_rx_handler_t)(void *, void *, u8 *, int);

struct ethernet_dev_ops {
    int (*init)(struct ethernet_device *);
    void (*get_mac_addr)(struct ethernet_device *, u8 *addr);
    void (*set_mac_addr)(struct ethernet_device *, u8 *addr);
    void (*set_rx_handler)(struct ethernet_device *,
                           void *priv, eth_rx_handler_t handler);
    void (*put_rx_packet)(struct ethernet_device *, void *packet_head);
    void (*drop_rx_packet)(struct ethernet_device *, void *packet_head);
    void *(*get_tx_buff)(struct ethernet_device *, u8 **payload, int len);
    void (*put_tx_buff)(struct ethernet_device *, void *buff_head);

    int (*set_cfg)(struct ethernet_device *, union ethdev_cfg *);
};

enum ethdev_event {
    ETHDEV_EVT_WIFI_SCAN_COMMPLETED,
    ETHDEV_EVT_WIFI_CONNECT_COMMPLETED,
    ETHDEV_EVT_WIFI_AP_START_SUSS,
    ETHDEV_EVT_WIFI_SSID_NOT_FOUND,
    ETHDEV_EVT_WIFI_ASSOCIAT_TIMEOUT,
    ETHDEV_EVT_WIFI_DISCONNECTED,
    ETHDEV_EVT_WIFI_ENTER_MODE,
    ETHDEV_EVT_WIFI_EXIT_MODE,
};

enum ethdev_type {
    ETHDEV_TYPE_LAN,
    ETHDEV_TYPE_WIFI,
};

enum ethdev_wifi_mode  {
    ETHDEV_WIFI_AP_MODE,
    ETHDEV_WIFI_STA_MODE,
};

struct ethdev_wifi_cfg {
    enum ethdev_wifi_mode mode;
    const char *ssid;
    const char *password;
};

struct ethdev_lan_cfg {

};

union ethdev_cfg {
    struct ethdev_lan_cfg  lan;
    struct ethdev_wifi_cfg wifi;
};

struct ethernet_device {
    const char *name;
    enum ethdev_type dev_type;
    union ethdev_cfg cfg;
    void *private_data;
    void *rx_priv;
    eth_rx_handler_t rx_handler;
    void (*event_notify)(struct ethernet_device *, enum ethdev_event event, void *arg);
    const struct ethernet_dev_ops *ops;
};

extern struct ethernet_device ethernet_device_begin[];
extern struct ethernet_device ethernet_device_end[];

#define list_for_each_net_device(dev) \
    for (dev = ethernet_device_begin; dev < ethernet_device_end; dev++)


#define REGISTER_ETHERNET_DEVICE(device) \
    static struct ethernet_device device sec(.ethernet_device)


#endif

