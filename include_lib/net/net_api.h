#ifndef NET_API_H
#define NET_API_H

#include "lwip/inet.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "lwip/autoip.h"
#include "lwip/dns.h"
#include "lwip/etharp.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/sys.h"
#include "sock_api/sock_api.h"

#include "device/ethernet_device.h"


struct net_config_info {
    bool dhcp_enable;
    bool set_default;
    ip4_addr_t ipaddr;
    ip4_addr_t mask;
    ip4_addr_t gw;
    const char *local_host_name;
    union ethdev_cfg  cfg;
};



int netapi_icfg_get(const char *netdev, struct net_config_info *cfg);


int netapi_icfg_restart(const char *netdev, struct net_config_info *cfg,
                        void (*complete)(int));











#endif
