#include "mssdp/mssdp.h"
#include "fs/fs.h"
#include "app_config.h"
#include "dev_desc.h"
#include "system/event.h"
#include "database.h"
#include "doorbell_event.h"

#if defined CONFIG_NET_SERVER || defined CONFIG_NET_CLIENT
static void network_ssdp_cb(u32 dest_ipaddr, enum mssdp_recv_msg_type type, char *buf, void *priv)
{
    if (type == MSSDP_SEARCH_MSG) {
        printf("ssdp client[0x%x] search, %s\n", dest_ipaddr, buf);
    } else if (type == MSSDP_NOTIFY_MSG) {
        printf("sssss\n");
//TODO
#if 1
        struct sys_event e;
        e.arg = "dhcp_srv";
        e.type = SYS_DEVICE_EVENT;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        sys_event_notify(&e);
#endif


        mssdp_uninit();

    }
}
#else

#define JSON_DOC "aaa"

static char json_buffer[1024];

const char *__attribute__((weak))get_dev_desc_doc()
{
    int ret;

#if 0
    FILE *fd = fopen("mnt/spiflash/res/dev_desc.txt", "r");

    if (fd == NULL) {
        printf("%s~~~~~~~~%d   open fail\n", __func__, __LINE__);
        return NULL;
    }

    memset(json_buffer, 0, 1024);
    ret = fread(fd, json_buffer, 1024);
    if (ret <= 0) {
        printf("%s~~~~~~~~%d   read fail\n", __func__, __LINE__);
        return NULL;
    }

    fclose(fd);
#else
    memset(json_buffer, 0, 1024);
    memcpy(json_buffer, DEV_DESC_CONTENT, strlen(DEV_DESC_CONTENT));
#endif

    return json_buffer;
}

char get_MassProduction(void)
{
    return 0;
}

const char *create_mssdp_notify_msg(char *msg, u16_t dev_online_flag)
{

    char ip_addr[64];
    extern u8 get_rx_signal(void);
    extern void Get_IPAddress(char is_wireless, char *ipaddr);
    extern int storage_device_ready();
    char *str = NULL;
    static char ver[64] = {""};
    static char did[32] = {""};
    static char *cfg = NULL;
    if (!strlen(ver)) {
        memset(ver, 0, sizeof(ver));
        extern char *get_hardware_version(void);
        memcpy(ver, get_hardware_version(), sizeof(ver));
        printf("\nver = %s\n", ver);
    }

    Get_IPAddress(1, ip_addr);
    if (!cfg) {
        cfg = calloc(1, 1024);
        if (!cfg) {
            printf("\n %s %d no mem\n", __func__, __LINE__);
            return "";
        }
    }

    if (!strlen(did)) {
        extern void doorbell_read_cfg_info(u8 * cfg, int len);
        doorbell_read_cfg_info(cfg, 1024);
        if (strlen(cfg)) {
            str = strstr(cfg, "did = ");
            if (str) {
                str += strlen("did = ");
                memset(did, 0, sizeof(did));
                memcpy(did, str, strstr(str, "\n") - str);
                printf("\n did = %s\n", did);
            }
        }
    }



    extern int get_dB();
    extern struct NETWORK_INFO *sys_get_network_info(void);
    extern int get_pir_trigger_state();
    extern int system_get_battary_info_voltage(void);
    printf("\n v = %d\n", system_get_battary_info_voltage());
    struct NETWORK_INFO *network_info = sys_get_network_info();
    printf("\n qos = %lld\n", network_info->qos);



    sprintf(msg, "{\"RTSP\":\"rtsp://%s/mass_pro_rt/front.sd\",\
\"SIGNAL\":\"%lld\",\
\"SD_STATUS\":\"%d\",\
\"DEV_ONLINE_FLAG\":\"%d\",\
\"dB\":\"%d\",\
\"UID\":\"%s\",\
\"PIR\":\"%d\",\
\"BAT_VOL\": \"%d\",\
\"VER\":\"%s\"\
}", \
            ip_addr, \
            network_info->qos, \
            storage_device_ready(), \
            dev_online_flag, \
            get_dB(), \
            did, \
            get_pir_trigger_state(), \
            system_get_battary_info_voltage(), \
            ver
           );

    printf("\n msg = %s strlen(msg) = %d\n", msg, strlen(msg));

    return msg;
}
int mssdp_recv_get_param(char *mssdp_buf)
{
    printf("\n mssdp_buf = %s\n", mssdp_buf);
    return 0;
}

static void network_ssdp_cb(u32 dest_ipaddr, enum mssdp_recv_msg_type type, char *buf, void *priv)
{
    char msg[256];
    switch (type) {
    case MSSDP_SEARCH_MSG:
        printf("ssdp client[0x%x] search, %s\n", dest_ipaddr, buf);
        break;
    case MSSDP_BEFORE_SEND_NOTIFY_MSG:
        if (get_MassProduction()) {
            mssdp_set_notify_msg((const char *)create_mssdp_notify_msg(msg, 0), 2);
        }
        break;
    case MSSDP_USER_MSG:
        mssdp_recv_get_param(buf);
        break;
    default :
        break;
    }

}
#endif

void network_mssdp_init(void)
{
    char msg[256];
    puts("mssdp run \n");
    if (get_MassProduction()) {
        mssdp_init("MSSDP_SEARCH ", "MSSDP_NOTIFY ", "MSSDP_REGISTER ", 3333, network_ssdp_cb, NULL);
        mssdp_set_notify_msg((const char *)create_mssdp_notify_msg(msg, 0), 2);
    } else {
        mssdp_init("MSSDP_SEARCH ", "MSSDP_NOTIFY ", "MSSDP_REGISTER ", 3889, network_ssdp_cb, NULL);

#if defined  CONFIG_NET_SERVER
        mssdp_set_search_msg("xyz", 30); //主动3秒
#endif
#if defined  CONFIG_NET_CLIENT
        mssdp_set_notify_msg("xy", 5); //被动30秒
#endif

#if (!defined CONFIG_NET_CLIENT) && (!defined CONFIG_NET_SERVER)
        mssdp_set_notify_msg((const char *)get_dev_desc_doc(), 60);
#endif
    }
}

void network_mssdp_uninit(void)
{
    mssdp_uninit();
}



