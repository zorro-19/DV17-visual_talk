#include "product_main.h"

#ifdef PRODUCT_TEST_ENABLE

static OS_MUTEX rsp_mutex;
static OS_MUTEX list_mutex;

static struct {
    u8 *test_buf;
    u8 *recv_buf;
    u8 *resp_buf;
    u8 enter_flag;
    u32 license_size;
    u32 startup_screens_size;
    u32 shutdown_screens_size;
    struct list_head msg_list;
} product_hdl = {0};


#define __this		(&product_hdl)


static u8 mac_str_to_bin(char *str, unsigned char *mac)
{
    int i;
    char *s, *e = NULL;

    if ((mac == NULL) || (str == NULL)) {
        return -1;
    }

    s = (char *) str;

    for (i = 0; i < 6; ++i) {
        mac[i] = s ? strtoul(s, &e, 16) : 0;
        if (s) {
            s = (*e) ? e + 1 : e;
        }
    }
    return 0;
}


static u8 *mac_bin_to_str(unsigned char *mac, u8 *str)
{
    memset(str, 0, sizeof(str));
    sprintf(str, "%X:%X:%X:%X:%X:%X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return str;
}


static u8 product_enter_io_trigger(void)
{
    u8 cnt = 0;

    gpio_set_pull_up(CONFIG_PD_TRIGGER_IO_PORT, !CONFIG_PD_TRIGGER_IO_STATE);
    gpio_set_pull_down(CONFIG_PD_TRIGGER_IO_PORT, CONFIG_PD_TRIGGER_IO_STATE);
    gpio_direction_input(CONFIG_PD_TRIGGER_IO_PORT);

    for (cnt = 0; cnt < CONFIG_PD_TRIGGER_IO_CNT; cnt++) {
        if (gpio_read(CONFIG_PD_TRIGGER_IO_PORT) != CONFIG_PD_TRIGGER_IO_STATE) {
            break;
        }
        os_time_dly(1);
    }
    return (cnt >= CONFIG_PD_TRIGGER_IO_CNT) ? 1 : 0;
}






__attribute__((weak))u8 product_enter_user_def(void)
{
#if 0
    u8 ver[56];
    u8 ret;
    memset(ver, 0, sizeof(ver));
    product_read_options_string("ver", ver);
    log_i("ver = %s\n", ver);
    ret = strncmp(ver, "JL_SDK", 5);
    return !!ret;
#else


   //u8  ret=enter_product_mode_product_file();
  // printf("\n if enter product_ret==================%d\n ",ret);
#ifdef CONFIG_ISP_PCCAMERA_MODE_ENABLE

    return 0;
#else
    return 1;
#endif
#endif
}




u8 product_enter_check(void)
{
#if (CONFIG_PD_TRIGGER_MODE == IO_TRIGGER_MODE)
    __this->enter_flag = product_enter_io_trigger();
#elif (CONFIG_PD_TRIGGER_MODE == USER_DEF_MODE)

    __this->enter_flag = product_enter_user_def();
#elif (CONFIG_PD_TRIGGER_MODE == AUTO_STARTUP_MODE)
    __this->enter_flag = 1;
#else
    __this->enter_flag = 0;
#endif
    return __this->enter_flag;
}


#ifdef MSG_TEST_DEBUG

static void create_msg_demo(u8 *msg)
{
    u32 crc_len;
    u16 *crc_value;
    static u8 init = 0;
    struct comm_head *head = (struct comm_head *)__this->test_buf;

    memset(__this->test_buf, 0, TEST_BUFFER_SIZE);
    if (!init) {
        head->mark[0] = 'J';
        head->mark[1] = 'L';
        head->idx     = 0;
        head->type    = DATA_TYPE_OPCODE;
    }

    head->len = (strlen(msg) + DATA_BUFFER_ALIGN - 1) / DATA_BUFFER_ALIGN * DATA_BUFFER_ALIGN;
    strcpy(__this->test_buf + sizeof(struct comm_head), msg);

    crc_len = sizeof(struct comm_head) + head->len;
    crc_value = (u16 *)(__this->test_buf + crc_len);
    *crc_value = CRC16(head, crc_len);

    log_info("%s, len = %d, %d\n", msg, head->len, strlen(msg));
    put_buf(__this->test_buf, crc_len + sizeof(u16));
}


static void msg_demo_display(void)
{
    u8 *demo[] = {
        "{\"opcode\":\"0\",\"rscorr\":\"0\",\"params\":\" \"}",
        "{\"opcode\":\"2\",\"rscorr\":\"0\",\"params\":{\"uuid\":\"0123456789abcdef\"}}",
        "{\"opcode\":\"3\",\"rscorr\":\"0\",\"params\":{\"mac\":\"50:B6:CC:34:8E:09\"}}",
        "{\"opcode\":\"4\",\"rscorr\":\"0\",\"params\":{\"mac\":\"50:B6:CC:34:8E:0A\"}}",
        "{\"opcode\":\"5\",\"rscorr\":\"0\",\"params\":{\"mac\":\"50:B6:CC:34:8E:0B\"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"0\",\"cmd\":\"2\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"1\",\"cmd\":\"2\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"1\",\"cmd\":\"7\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"1\",\"cmd\":\"8\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"6\",\"cmd\":\"2\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"7\",\"cmd\":\"3\",\"args\":{\"mode\":\"1\",\"ssid\":\"GJ1\",\"pwd\":\"8888888899\"}}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"7\",\"cmd\":\"3\",\"args\":{\"mode\":\"2\",\"ssid\":\"mmmmyyy\",\"pwd\":\"8888888899\"}}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"7\",\"cmd\":\"4\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"7\",\"cmd\":\"5\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"7\",\"cmd\":\"6\",\"args\":\" \"}}",


        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"4\",\"cmd\":\"0\",\"args\":[{\"addr\":\"0xf0\", \"value\":\" \"},{\"addr\":\"0xf1\", \"value\":\" \"}]}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"4\",\"cmd\":\"1\",\"args\":[{\"addr\":\"0x1234\", \"value\":\"0x4567\"},{\"addr\":\"0x1234\", \"value\":\"0x4567\"}]}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"4\",\"cmd\":\"2\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"4\",\"cmd\":\"9\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"4\",\"cmd\":\"10\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"4\",\"cmd\":\"11\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"4\",\"cmd\":\"12\",\"args\":\" \"}}",
        "{\"opcode\":\"6\",\"rscorr\":\"0\",\"params\":{\"sn\":\"0123456789abcdef\"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"0\",\"cmd\":\"13\",\"args\":\" \"}}",
        "{\"opcode\":\"7\",\"rscorr\":\"0\",\"params\":[{\"option1\":\"value1\"}, {\"option2\":\"value2\"}, {\"option3\":\"value3\"}]}",

        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"7\",\"cmd\":\"14\",\"args\":{\"ip\":\"120.24.247.138\",\"interval\":\"1000\",\"count\":\"10\"}}}",
        "{\"opcode\":\"8\",\"rscorr\":\"0\",\"params\":{\"size\":\"999999999\"}}",
        "{\"opcode\":\"9\",\"rscorr\":\"0\",\"params\":\" \"}",
        "{\"opcode\":\"10\",\"rscorr\":\"0\",\"params\":\" \"}",
        "{\"opcode\":\"11\",\"rscorr\":\"0\",\"params\":\" \"}",
        "{\"opcode\":\"12\",\"rscorr\":\"0\",\"params\":\" \"}",
        "{\"opcode\":\"13\",\"rscorr\":\"0\",\"params\":\" \"}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"8\",\"cmd\":\"16\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"8\",\"cmd\":\"17\",\"args\":\" \"}}",
        "{\"opcode\":\"1\",\"rscorr\":\"0\",\"params\":{\"id\":\"0\",\"type\":\"7\",\"cmd\":\"19\",\"args\":\" \"}}",
        "{\"opcode\":\"14\",\"rscorr\":\"0\",\"params\":\" \"}",
    };

    u8 *tips[] = {
        "获取小机的设备列表，在上位机中显示设备列表",
        "写入UUID",
        "写入EDR_MAC",
        "写入BLE_MAC",
        "写入WIFI_MAC",
        "获取SD设备信息：在线状态/容量大小/块大小(单位：字节)",
        "获取LCD设备信息：长度/宽度",
        "开启LCD设备颜色闪烁",
        "关闭LCD设备颜色闪烁",
        "获取电池设备信息：电量百分比",
        "设置WIFI进入STA模式,连接指定的路由器",
        "设置WIFI进入AP模式",
        "进入STA模式后，启动扫描当前周围的wifi信号",
        "启动扫描后，4s后可获取扫描结果，扫描结果：账号/RSSI/SNR",
        "进入AP模式后，获取当前已连接的设备信息：MAC/RSSI/EVM",
        "摄像头读寄存器",
        "摄像头写寄存器",
        "摄像头信息获取",
        "摄像头打开夜视",
        "摄像头关闭夜视",
        "摄像头打开补光",
        "摄像头关闭补光",
        "写入SN",
        "SD卡读写测试",
        "写入配置表",
        "ping->120.24.247.138",
        "写入文件信息",
        "读取UUID",
        "读取EDR_MAC",
        "读取BLE_MAC",
        "读取WIFI_MAC",
        "读取SN",
        "打开PIR状态监测",
        "关闭PIR状态监测",
        "获取路由信息",
        "获取LICENSE写入状态",
    };

    u8 *err[] = {
        "无错误",
        "参数错误",
        "不支持该设备类型",
        "不支持该设备操作",
        "设备操作失败",
        "内存申请失败",
        "文件读写错误",
        "文件校验错误",
        "写入重复数据",
        "网络未连接",
        "文件写入空间不足",
    };

    for (u8 i = 0; i < ARRAY_SIZE(err); i++) {
        log_info("错误码：%d = %s\n", i, err[i]);
    }

    for (u8 i = 0; i < ARRAY_SIZE(demo); i++) {
        log_info(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        log_info("%s\n", tips[i]);
        create_msg_demo(demo[i]);
        log_info("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
    }
}

#endif


void data_respond(u8 idx, u8 type, u8 *data, u32 len)
{
    u32 crc_len;
    u16 *crc_value;
    struct comm_head *head = (struct comm_head *)__this->resp_buf;

    os_mutex_pend(&rsp_mutex, 0);
    os_time_dly(50);

    memset(__this->resp_buf, 0, RESP_BUFFER_SIZE);
    head->mark[0] = 'J';
    head->mark[1] = 'L';
    head->idx     = idx;
    head->type    = type;

    head->len = (len + DATA_BUFFER_ALIGN - 1) / DATA_BUFFER_ALIGN * DATA_BUFFER_ALIGN;
    memcpy(__this->resp_buf + sizeof(struct comm_head), data, len);

    crc_len = sizeof(struct comm_head) + head->len;
    crc_value = (u16 *)(__this->resp_buf + crc_len);
    *crc_value = CRC16(head, crc_len);

    if (comm_ops()->write(__this->resp_buf, crc_len + sizeof(u16)) != crc_len + sizeof(u16)) {
        log_err("%s error\n", __func__);
    }

    os_mutex_post(&rsp_mutex);
    //s32 ret, retry_cnt = 0;
    //do {
    //    ret = comm_ops()->write(__this->resp_buf, crc_len + sizeof(u16));
    //    log_info("%s, retry = %d, ret = %d / %d\n", __func__, retry_cnt, ret, crc_len + sizeof(u16));
    //    retry_cnt++;
    //} while ((ret != crc_len + sizeof(u16)) && retry_cnt < 10);
}


static void msg_handle_task(void *priv)
{
    u16 len;
    struct comm_msg *msg;
    struct comm_msg *n;
    struct product_rtc_time time = {0};
    json_object *new_obj, *sub_obj, *opcode_obj, *rscorr_obj, *params_obj;
    u8 mac[PRODUCT_MAC_SIZE], uuid[PRODUCT_UUID_SIZE + 1] = {0}, sn[PRODUCT_SN_SIZE + 1] = {0}, macstr[3][32] = {0}, *str, opcode, rscorr, rscorr_str[4] = {0}, free_flag, license_flag = 0;

    while(1){
        os_mutex_pend(&list_mutex,0);
        list_for_each_entry_safe(msg, n ,&__this->msg_list, entry) {

            log_info("--->recv cjson\n\n%s\n\n", msg->data);
            if (!(new_obj = json_tokener_parse(msg->data))) {
                goto _exit_list_;
            }

            opcode_obj = json_object_object_get(new_obj, "opcode");
            rscorr_obj = json_object_object_get(new_obj, "rscorr");
            params_obj = json_object_object_get(new_obj, "params");

            if (!opcode_obj || !rscorr_obj || !params_obj) {
                goto _exit_list_;
            }

            str = NULL;
            opcode = json_object_get_int(opcode_obj);

            switch (opcode) {
            case OPC_GET_PROFILE:
                product_rtc_default_wr(&time, 0);
                product_license_flag_wr(&license_flag, 0);
                asprintf(&str, "{\"opcode\":\"%d\",\"rscorr\":\"%d\",\"params\":{\"ver_tool\":\"%s\",\"ver_chip\":\"%s\",\"ver_firmware\":\"%s\",\"license_flag\":\"%d\",\"UUID\":\"%s\",\"SN\":\"%s\",\"WIFI_MAC\":\"%s\",\"EDR_MAC\":\"%s\",\"BLE_MAC\":\"%s\",\"RTC_DTIME\":\"%d:%d:%d:%d:%d:%d\",%s}}", \
                         OPC_GET_PROFILE, ERR_NULL, PD_TOOL_VERSION, PD_CHIP_VERSION, PD_SDK_VERSION, \
                         license_flag, \
                         product_uuid_wr(uuid, 0) ? "NULL" : uuid, \
                         product_sn_wr(sn, 0) ? "NULL" : sn, \
                         product_rf_mac_wr("WIFI", mac, 0) ? "NULL" : mac_bin_to_str(mac, macstr[0]), \
                         product_rf_mac_wr("EDR", mac, 0) ? "NULL" : mac_bin_to_str(mac, macstr[1]), \
                         product_rf_mac_wr("BLE", mac, 0) ? "NULL" : mac_bin_to_str(mac, macstr[2]), \
                         time.year, time.month, time.day, time.hour, time.min, time.sec, \
                         DEVICES_LIST);
                break;

            case OPC_DEV_CTL:
                rscorr = devices_control_handler(params_obj);
                asprintf(&str, "{\"opcode\":\"%d\",\"rscorr\":\"%d\",\"params\":{\"id\":\"%d\",\"type\":\"%d\",\"cmd\":\"%d\",\"args\":%s}}", opcode, rscorr, \
                         json_object_get_int(json_object_object_get(params_obj, "id")),   \
                         json_object_get_int(json_object_object_get(params_obj, "type")), \
                         json_object_get_int(json_object_object_get(params_obj, "cmd")),  \
                         get_args_str() ? get_args_str() : "\" \"");

                if (get_args_str()) {
                    free(get_args_str());
                }
                break;

            case OPC_WRITE_UUID:
                sub_obj = json_object_object_get(params_obj, "uuid");
                rscorr = product_uuid_wr(json_object_get_string(sub_obj), 1);
                break;

            case OPC_WRITE_SN:
                sub_obj = json_object_object_get(params_obj, "sn");
                rscorr = product_sn_wr(json_object_get_string(sub_obj), 1);
                break;

            case OPC_WRITE_MAC_EDR:
                sub_obj = json_object_object_get(params_obj, "mac");
                mac_str_to_bin(json_object_get_string(sub_obj), mac);
                rscorr = product_rf_mac_wr("EDR", mac, 1);
                break;

            case OPC_WRITE_MAC_BLE:
                sub_obj = json_object_object_get(params_obj, "mac");
                mac_str_to_bin(json_object_get_string(sub_obj), mac);
                rscorr = product_rf_mac_wr("BLE", mac, 1);
                break;

            case OPC_WRITE_MAC_WIFI:
                sub_obj = json_object_object_get(params_obj, "mac");
                mac_str_to_bin(json_object_get_string(sub_obj), mac);
                rscorr = product_rf_mac_wr("WIFI", mac, 1);
                break;

            case OPC_READ_UUID:
                rscorr = product_uuid_wr(uuid, 0);
                asprintf(&str, "{\"opcode\":\"%d\",\"rscorr\":\"%d\",\"params\":{\"uuid\":\"%s\"}}", opcode, rscorr, rscorr ? "NULL" : uuid);
                break;

            case OPC_READ_SN:
                rscorr = product_sn_wr(sn, 0);
                asprintf(&str, "{\"opcode\":\"%d\",\"rscorr\":\"%d\",\"params\":{\"sn\":\"%s\"}}", opcode, rscorr, rscorr ? "NULL" : sn);
                break;

            case OPC_READ_MAC_EDR:
                rscorr = product_rf_mac_wr("EDR", mac, 0);
                asprintf(&str, "{\"opcode\":\"%d\",\"rscorr\":\"%d\",\"params\":{\"mac\":\"%s\"}}", opcode, rscorr, rscorr ? "NULL" : mac_bin_to_str(mac, macstr[0]));
                break;

            case OPC_READ_MAC_BLE:
                rscorr = product_rf_mac_wr("BLE", mac, 0);
                asprintf(&str, "{\"opcode\":\"%d\",\"rscorr\":\"%d\",\"params\":{\"mac\":\"%s\"}}", opcode, rscorr, rscorr ? "NULL" : mac_bin_to_str(mac, macstr[0]));
                break;

            case OPC_READ_MAC_WIFI:
                rscorr = product_rf_mac_wr("WIFI", mac, 0);
                asprintf(&str, "{\"opcode\":\"%d\",\"rscorr\":\"%d\",\"params\":{\"mac\":\"%s\"}}", opcode, rscorr, rscorr ? "NULL" : mac_bin_to_str(mac, macstr[0]));
                break;

            case OPC_WRITE_OPTIONS:
                rscorr = product_write_options(params_obj);
                break;

            case OPC_WRITE_FILE_INFO:
                rscorr = ERR_NULL;
                sub_obj = json_object_object_get(params_obj, "type");
                switch (json_object_get_int(sub_obj)) {
                case DATA_TYPE_LICENSE_WRITE:
                    __this->license_size = json_object_get_int(json_object_object_get(params_obj, "size"));
                    log_info("license_size = %d\n", __this->license_size);
                    break;
                case DATA_TYPE_STARTUP_SCREENS:
                    __this->startup_screens_size = json_object_get_int(json_object_object_get(params_obj, "size"));
                    log_info("startup_screens_size = %d\n", __this->startup_screens_size);
                    break;
                case DATA_TYPE_SHUTDOWN_SCREENS:
                    __this->shutdown_screens_size = json_object_get_int(json_object_object_get(params_obj, "size"));
                    log_info("shutdown_screens_size = %d\n", __this->shutdown_screens_size);
                    break;

                default:
                    rscorr = ERR_PARAMS;
                    break;
                }
                break;

            case OPC_GET_LICENSE_INFO:
                product_license_flag_wr(&license_flag, 0);
                asprintf(&str, "{\"opcode\":\"%d\",\"rscorr\":\"%d\",\"params\":{\"license_flag\":\"%d\"}}", opcode, rscorr, license_flag);
                rscorr = ERR_NULL;
                break;

            case OPC_RTC_DEF_TIME_WRITE:
                time.year  = json_object_get_int(json_object_object_get(params_obj, "year"));
                time.month = json_object_get_int(json_object_object_get(params_obj, "month"));
                time.day   = json_object_get_int(json_object_object_get(params_obj, "day"));
                time.hour  = json_object_get_int(json_object_object_get(params_obj, "hour"));
                time.min   = json_object_get_int(json_object_object_get(params_obj, "min"));
                time.sec   = json_object_get_int(json_object_object_get(params_obj, "sec"));
                rscorr = product_rtc_default_wr(&time, 1);
                break;

            case OPC_RTC_DEF_TIME_READ:
                rscorr = product_rtc_default_wr(&time, 0);
                asprintf(&str, "{\"opcode\":\"%d\",\"rscorr\":\"%d\",\"params\":{\"year\":\"%d\",\"month\":\"%d\",\"day\":\"%d\",\"hour\":\"%d\",\"min\":\"%d\",\"sec\":\"%d\"}}", \
                         opcode, rscorr, time.year, time.month, time.day, time.hour, time.min, time.sec);
                break;

            case OPC_ERASE_BOOTSCREENS:
                rscorr = product_erase_bootscreens();
                break;

            default:
                break;
            }

            if (str) {
                free_flag = 1;
            } else {
                free_flag = 0;
                sprintf(rscorr_str, "%d", rscorr);
                json_object_string_set(rscorr_obj, rscorr_str, strlen(rscorr_str));
                str = json_object_get_string(new_obj);
            }

            log_info("--->resp cjson\n\n%s\n\n", str);
            data_respond(0, DATA_TYPE_OPCODE, str, strlen(str));

            if (free_flag) {
                free(str);
            }
_exit_list_:
            json_object_put(new_obj);
            list_del(&msg->entry);
            free(msg->data);
            free(msg->self);
            break;
        }
        os_mutex_post(&list_mutex);
    }
}


static void data_handle_task(void *priv)
{
    s32 len;
    u16 *crc_value;
    u8 rscorr, *data, *read_addr, *l_data, l_idx, flag;
    u32 crc_len, recv_size, read_size, total_size, remain_size, l_len;
    struct comm_msg *msg;
    struct comm_head *head;

    head = (struct comm_head *)__this->recv_buf;
    data = __this->recv_buf + sizeof(struct comm_head);

    for (;;) {
        flag = 0;
        read_addr = __this->recv_buf;
        read_size = sizeof(u8);
        if (comm_ops()->read(read_addr, read_size) == read_size) {
            if (head->mark[0] == 'J') {
                read_addr += read_size;
                read_size = sizeof(u8);
                if (comm_ops()->read(read_addr, read_size) == read_size) {
                    if (head->mark[1] == 'L') {
                        flag = 1;
                    }
                }
            }
        }

        if (!flag) {
            continue;
        }

        read_addr += read_size;
        recv_size = sizeof(u16);
        total_size = sizeof(struct comm_head);
        while (recv_size < total_size) {
            remain_size = total_size - recv_size;
            read_size = (remain_size > MAXP_SIZE_CDC_DATA) ? MAXP_SIZE_CDC_DATA : remain_size;
            if ((len = comm_ops()->read(read_addr, read_size)) <= 0) {
                break;
            }
            recv_size += len;
            read_addr += len;
        }

        if (recv_size < total_size) {
            continue;
        }

        read_addr = __this->recv_buf + sizeof(struct comm_head);
        recv_size = sizeof(struct comm_head);
        total_size = sizeof(struct comm_head) + head->len + sizeof(u16);

        while (recv_size < total_size) {
            remain_size = total_size - recv_size;
            read_size = (remain_size > MAXP_SIZE_CDC_DATA) ? MAXP_SIZE_CDC_DATA : remain_size;
            if ((len = comm_ops()->read(read_addr, read_size)) <= 0) {
                break;
            }
            recv_size += len;
            read_addr += len;
        }

        if (recv_size < total_size) {
            continue;
        }

        //put_buf(__this->recv_buf, total_size);

        crc_len = sizeof(struct comm_head) + head->len;
        crc_value = (u16 *)(__this->recv_buf + crc_len);
        if (CRC16(head, crc_len) != *crc_value) {
            continue;
        }

        switch (head->type) {
        case DATA_TYPE_OPCODE:
            msg = zalloc(sizeof(struct comm_msg));
            msg->data = zalloc(head->len);
            if (!msg || !msg->data) {
                break;
            }
            msg->len = head->len;
            msg->self = msg;
            memcpy(msg->data, data, head->len);
            os_mutex_pend(&list_mutex,0);
            list_add_tail(&msg->entry, &__this->msg_list);
            os_mutex_post(&list_mutex);
            break;

        case DATA_TYPE_LICENSE_WRITE:
            rscorr = product_write_license(head->idx, data, head->len, __this->license_size);
            data_respond(head->idx, head->type, &rscorr, 1);
            break;

        case DATA_TYPE_STARTUP_SCREENS:
            rscorr = product_write_startup_screens(head->idx, data, head->len, __this->startup_screens_size);
            data_respond(head->idx, head->type, &rscorr, 1);
            break;

        case DATA_TYPE_LICENSE_READ:
            l_data = zalloc(PRODUCT_LICENSE_INFO_SIZE);
            rscorr = product_read_license(&l_idx, l_data + sizeof(u32), &l_len);
            *((u32 *)l_data) = rscorr;
            data_respond(l_idx, head->type, l_data, l_len + sizeof(u32));
            free(l_data);
            break;

        case DATA_TYPE_LICENSE_ERASE:
            rscorr = product_erase_license();
            data_respond(head->idx, head->type, &rscorr, 1);
            break;

        case DATA_TYPE_SHUTDOWN_SCREENS:
            rscorr = product_write_shutdown_screens(head->idx, data, head->len, __this->shutdown_screens_size);
            data_respond(head->idx, head->type, &rscorr, 1);
            break;

        default:
            break;
        }
    }
}


u8 is_product_mode(void)
{
    return __this->enter_flag;
}

u8 set_product_mode(u8 mode){

__this->enter_flag=mode;


}

u8 product_main(void)
{
    #if 0
    if (!product_enter_check()) {
        return 0;
    }
    #endif
    log_info("\r\n================ENTER PRODUCT TEST MODE================\r\n");

    while (!comm_ops()->online()) {
        log_info("wait usb online");
        os_time_dly(100);
    }

#ifdef MSG_TEST_DEBUG
    __this->test_buf = zalloc(TEST_BUFFER_SIZE);
    ASSERT(__this->test_buf, "product tool zalloc buffer err");
    msg_demo_display();
#endif

    __this->enter_flag = 1;
    __this->recv_buf = zalloc(RECV_BUFFER_SIZE);
    __this->resp_buf = zalloc(RESP_BUFFER_SIZE);
    ASSERT(__this->recv_buf && __this->recv_buf, "product tool zalloc buffer err");

    INIT_LIST_HEAD(&__this->msg_list);

    comm_ops()->init();
    devices_module_init();

    os_mutex_create(&rsp_mutex);
    os_mutex_create(&list_mutex);
    thread_fork("msg_handle_task", 10, 1024, 0, NULL, msg_handle_task, NULL);
    thread_fork("data_handle_task", 30, 1024, 0, NULL, data_handle_task, NULL);

    return 1;
}


#endif


