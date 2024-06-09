#include "system/includes.h"
#include "server/usb_server.h"
#include "doorbell_event.h"
#include "usb_audio.h"
#include "usb_host_user.h"
extern void cdc_write_data(u8 *data,u8 len);
//从机例子
#if 0
//输出
static int cdc_user_output(u8 *buf, u32 len)
{

     puts("cdc output\n");
     put_buf(buf, len);

    return len;
}
//输入
int cdc_user_input(u8 *buf, u32 len)
{

}

int usb_cdc_user_init()
{
    int set_usb_cdc(int (*output)(u8 * obuf, u32 olen));
    set_usb_cdc(cdc_user_output);
}






#endif // 0

static cbuffer_t __cbuffer;

#define RT_TALK_BUFFER_SIZE   64
static char buffer[RT_TALK_BUFFER_SIZE];
struct cdc_info{
    u8 head0;
    u8 head1;
	u16 data_len;//表示后面有多少数据
	u16 head_crc16;
	//存在接收数据的crc只计算data
} __attribute__((packed));


enum{
    SHANG_YI_SHOU = 0,
    XIA_YI_SHOU = 1,
    ZHAN_TING_BO_FANG = 2,
    BO_FANG_YIN_YUE = 3,
    ZHENG_JIA_YIN_LIANG = 4,
    JIANG_DI_YIN_LIANG = 5,
    HUANG_XING_CI = 6,//唤醒词
    DA_KAI_PING_MU = 7,
    GUANG_BI_PING_MU = 8,
    HU_JIAO_DIAN_HUA = 9,
    DA_KAI_LU_XIANG = 12,
    GUANG_BI_LU_XIANG = 13,
    ZHUA_PAI_ZHAO_PIAN = 14,
    DA_KAI_HUI_FANG = 15,
    DA_KAI_LIU_YAN = 18,
    SET_VOLUME = 0x80,
    SET_PHONE_STATUS = 0x81,
    GET_VOLUME = 0x82,
    RECV_BLE_DATA = 0x83,
    SEND_BLE_DATA = 0x84,
    SET_BLE_NAME = 0x85,
    SET_BLE_ON = 0x86,
    SET_BLE_OFF = 0x87,
};

static int ac7016_volume = 0;

typedef struct {
    u8 cmd;
    u8 data[];
}CMD_INFO __attribute__((packed));


void net_set_phone_status(u8 status)
{
    u8 data[2];
    //设置音量
    data[0] = SET_PHONE_STATUS;
    data[1] = status;
    cdc_write_data(data,sizeof(data));
}
#if  1
/*
typedef struct {
    u8 cmd;
    u8 data[];
}CMD_INFO __attribute__((packed));

*/




// 将设备音量（0-14）映射到app音量（0-10）
int mapDeviceVolumeToAppVolume(int deviceVolume) {
    // 检查输入是否在有效范围内
    if (deviceVolume < 0 || deviceVolume > 14) {
        return -1; // 输入无效
    }

    // 线性映射公式，并四舍五入到最接近的整数
    double appVolumeDouble = (deviceVolume * 10.0) / 14.0;
    printf("\n appVolumeDouble:%f\n",appVolumeDouble);
    int appVolume = round(appVolumeDouble);

    // 确保app音量在0-10范围内
    return (appVolume < 0) ? 0 : ((appVolume > 10) ? 10 : appVolume);
}

// 将app音量（0-10）映射回设备音量（0-14）
int mapAppVolumeToDeviceVolume(int appVolume) {
    // 检查输入是否在有效范围内
    if (appVolume < 0 || appVolume > 10) {
        return -1; // 输入无效
    }

    // 线性映射公式，并四舍五入到最接近的整数
    double deviceVolumeDouble = (appVolume * 14.0) / 10.0;
     printf("\n deviceVolumeDouble:%f\n",deviceVolumeDouble);

    int deviceVolume = round(deviceVolumeDouble);

    // 确保设备音量在0-14范围内
    return (deviceVolume < 0) ? 0 : ((deviceVolume > 14) ? 14 : deviceVolume);
}

#endif

#if 0


// 设备音量到app音量的映射表（查找表）
int deviceToAppMap[15] = {
    0,  // 设备音量 0 映射到 app 音量 0
    0,  // 设备音量 1 映射到 app 音量 0（或1，取决于具体映射策略）
    1,  // 设备音量 2 映射到 app 音量 1
    1,  // 设备音量 3 映射到 app 音量 1
    2,  // 设备音量 4 映射到 app 音量 2
    2,  // 设备音量 5 映射到 app 音量 2
    3,  // 设备音量 6 映射到 app 音量 3
    3,  // 设备音量 7 映射到 app 音量 3
    4,  // 设备音量 8 映射到 app 音量 4
    5,  // 设备音量 9 映射到 app 音量 5
    6,  // 设备音量 10 映射到 app 音量 6
    7,  // 设备音量 11 映射到 app 音量 7
    8,  // 设备音量 12 映射到 app 音量 8
    9,  // 设备音量 13 映射到 app 音量 9
    10  // 设备音量 14 映射到 app 音量 10
};

// app音量到设备音量的反向映射表
int appToDeviceMap[11] = {
    0,  // app音量 0 映射回 设备音量 0
    2,  // app音量 1 映射回 设备音量 2
    4,  // app音量 2 映射回 设备音量 4
    6,  // app音量 3 映射回 设备音量 6
    8,  // app音量 4 映射回 设备音量 8
    9,  // app音量 5 映射回 设备音量 9
    10,  // app音量 6 映射回 设备音量 10
    11,  // app音量 7 映射回 设备音量 11
    12,  // app音量 8 映射回 设备音量 12
    13,  // app音量 9 映射回 设备音量 13
    14,  // app音量 10 映射回 设备音量 14

    // ... 需要根据deviceToAppMap反向计算填充
    // 注意：这里需要根据deviceToAppMap来确保映射是可逆的
};

// 初始化反向映射表
void initReverseMap() {
    for (int i = 0; i < 15; ++i) {
        int appVolume = deviceToAppMap[i];
        if (appToDeviceMap[appVolume] == 0) { // 确保不覆盖已存在的映射
            appToDeviceMap[appVolume] = i;
        }
    }
}

// 将设备音量映射到app音量
int mapDeviceVolumeToAppVolume(int deviceVolume) {
    if (deviceVolume < 0 || deviceVolume > 14) {
        return -1; // 输入无效
    }
    return deviceToAppMap[deviceVolume];
}

// 将app音量映射回设备音量
int mapAppVolumeToDeviceVolume(int appVolume) {
    if (appVolume < 0 || appVolume > 10) {
        return -1; // 输入无效
    }
    return appToDeviceMap[appVolume];
}

int test_map() {
    // 初始化反向映射表
    initReverseMap();

    // 示例：将设备音量映射到app音量，然后再映射回设备音量
    for (int deviceVolume = 0; deviceVolume <= 14; ++deviceVolume) {
        int appVolume = mapDeviceVolumeToAppVolume(deviceVolume);
        if (appVolume != -1) {
            printf("设备音量 %d 对应的 app 音量是 %d\n", deviceVolume, appVolume);

            // 再将app音量映射回设备音量
            int mappedDeviceVolume = mapAppVolumeToDeviceVolume(appVolume);
            if (mappedDeviceVolume != -1) {
                printf("app 音量 %d 映射回的设备音量是 %d\n", appVolume, mappedDeviceVolume);
                // 由于使用了查找表，mappedDeviceVolume 应该总是等于 deviceVolume
            } else {
                printf("映射app音量时出错\n");
            }
        } else {
            printf("输入的设备音量值无效\n");
        }
    }

    return 0;
}


#endif // 0


static int cdc_host_cmd_deal(u8 *buf,int buf_len)
{
    static struct host_speaker_attr spk_attr = {0};
    static struct host_mic_attr mic_attr = {0};
    static struct sys_event event = {0};
    CMD_INFO *info = (CMD_INFO *)buf;
    u8 cmd = info->cmd;
    printf("\n >>>>>>>>>>>>>>>>cmd = 0x%x\n",cmd);

    if(cmd==HUANG_XING_CI){ //唤醒词

    post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "rec.adp");

    }


    if(cmd != GUANG_BI_PING_MU){
        //打开屏幕
        if(get_lcd_pro_flag()){
            event.arg = "VoiceCmd";
            event.type = SYS_DEVICE_EVENT;
            event.u.dev.event  = 1;
            sys_event_notify(&event);
        }

    }
    memset(&event,0,sizeof(event));
    switch(cmd){
    case SHANG_YI_SHOU:
        //上一首

        event.type = SYS_KEY_EVENT;
        event.u.key.event = KEY_EVENT_CLICK;
        event.u.key.value = KEY_UP;
        sys_event_notify(&event);
        break;
    case XIA_YI_SHOU:

        event.type = SYS_KEY_EVENT;
        event.u.key.event = KEY_EVENT_CLICK;
        event.u.key.value = KEY_DOWN;
        sys_event_notify(&event);
        //下一首
        break;
    case ZHAN_TING_BO_FANG:
        //暂停播放
        break;
    case BO_FANG_YIN_YUE:
        //播放音乐
        break;
    case ZHENG_JIA_YIN_LIANG:
       // ac7016_volume += 5;
        ac7016_volume += 2;
        if(ac7016_volume>=14){//主机音量最大限制10, 从机音量最大14


        ac7016_volume=14;
        }
        u8 data[2];
        printf("\n >>>>>>>>>ZHENG_JIA_YIN_LIANG_ac7016_volume = %d\n",ac7016_volume);

        //设置音量
        data[0] = SET_VOLUME;
        data[1] = ac7016_volume;
        cdc_write_data(data,sizeof(data));

        data[0] = GET_VOLUME;
        data[1] = ac7016_volume;
        cdc_write_data(data,sizeof(data));
        break;
    case JIANG_DI_YIN_LIANG:


        ac7016_volume -= 2;

        printf("\n  ac7016_volume::::%d\n",ac7016_volume);

       // ac7016_volume -= 1;

        if(ac7016_volume <1){

        ac7016_volume=0;
        }
        printf("\n >>>>>>>>>JIANG_DI_YIN_LIANG_ac7016_volume = %d\n",ac7016_volume);
        data[0] = SET_VOLUME;
        data[1] = ac7016_volume;
        cdc_write_data(data,sizeof(data));
        //设置音量
        data[0] = GET_VOLUME;
        data[1] = 0;
        cdc_write_data(data,sizeof(data));
        break;
    case GUANG_BI_PING_MU:
        //关闭屏幕
        event.arg = "VoiceCmd";
        event.type = SYS_DEVICE_EVENT;
        event.u.dev.event  = 0;
        sys_event_notify(&event);
        break;
    case DA_KAI_PING_MU:
        //打开屏幕

        break;
    case HU_JIAO_DIAN_HUA:
        //呼叫电话
        event.arg  = (void *)KEY_WAKEUP;
        event.type = SYS_KEY_EVENT;
        event.u.key.event = KEY_EVENT_CLICK;
        event.u.key.value = KEY_OK;
        sys_event_notify(&event);

        break;
    case DA_KAI_LU_XIANG:
        //打开录像
        break;
    case GUANG_BI_LU_XIANG:
        //关闭录像
        break;
    case ZHUA_PAI_ZHAO_PIAN:
        //抓拍照片
        break;
    case DA_KAI_HUI_FANG:
        //打开回放
        break;
    case DA_KAI_LIU_YAN:
        //打开留言

        event.type = SYS_KEY_EVENT;
        event.u.key.event = KEY_EVENT_CLICK;
        event.u.key.value = KEY_UP;
        sys_event_notify(&event);
        break;

    case GET_VOLUME:
        ac7016_volume = (u8 *)info->data[0];

        printf("\n GET_SALVE_VOLUME>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ac7016_volume = %d\n",ac7016_volume);


        int app_volume=mapDeviceVolumeToAppVolume(ac7016_volume);

        printf("\n convert_app_volume>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> = %d,db_select(vol):%d\n",app_volume,db_select("vol"));

        if(db_select("vol")!=app_volume){

        db_update("vol", app_volume);
        db_flush();

        }

      //  mapDeviceVolumeToAppVolume(ac7016_volume);

        #if  0
//       // mapDeviceVolumeToAppVolume();

        for(int dev=0;dev<=14;dev++){



         int app_vol=mapDeviceVolumeToAppVolume(dev);
         int dev_vol=mapAppVolumeToDeviceVolume(app_vol);


         printf("\n  dev_vol::::%d,app_vol:%d\n",dev_vol,app_vol);

         }




    printf("\n zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n");
        for(int app=0;app<=10;app++){


          int dev_vol=mapAppVolumeToDeviceVolume(app);
         int app_vol=mapDeviceVolumeToAppVolume(dev_vol);



         printf("\n  dev_vol::::%d,app_vol:%d\n",dev_vol,app_vol);

         }

       // test_map();
        #endif

        #if 0

        if(ac7016_volume>10){

        ac7016_volume-=4;

        }else{


        }

        #endif // 0



//        if(db_select("vol")==ac7016_volume){    /0-10   0-14    10-14(7-10)
//
//
//        }else{ //如果不一样
//
//           if(!ac7016_volume){
//
//            }else{
//
//            if( ac7016_volume<4){
//            ac7016_volume+=3;
//            }
//            ac7016_volume-=2;
//
//
//            }
//          db_update("vol", ac7016_volume);
//          db_flush();
//        }


        printf("\n GET>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ac7016_volume = %d\n",ac7016_volume);
        break;
    case RECV_BLE_DATA:
        recv_ble_data_for_7016(info->data,buf_len - 1);
        break;
    default:
        break;
    }
}
/*
struct cdc_info{
    u8 head0;
    u8 head1;
	u16 data_len;//表示后面有多少数据
	u16 head_crc16;
	//存在接收数据的crc只计算data
} __attribute__((packed));

*/

/**
 * @brief cdc接收数据弱定义函数
 *
 * @param buf: 接收数据buffer
 * @param len: 接收到数据长度,小于最大包长
 */
int cdc_rx_callback(void *buf, int len)
{
    int info_len = sizeof(struct cdc_info);
    u8 *buffer = malloc(info_len);
    if(!buffer){
        goto __exit;
    }
    put_buf(buf,len);

    cbuf_write(&__cbuffer,buf,len);



    printf("\n cbuf_get_data_size(&__cbuffer) = %d \n",cbuf_get_data_size(&__cbuffer));
    static int offset = 0;


    do{
        int len;
        //如果总数据量小于最小的数据头+数据+数据crc先不处理
        int data_len = cbuf_get_data_size(&__cbuffer) ;
        printf("\n cbuf_get_data_size(&__cbuffer) = %d \n",cbuf_get_data_size(&__cbuffer));
        if(data_len + offset < ( info_len + 2)){
            printf("\n data_len  = %d\n",data_len);
            break;
        }
        if(offset < info_len){
            cbuf_read(&__cbuffer,buffer+offset,info_len - offset);
        }

        put_buf(buffer,info_len);
        struct cdc_info *info = (struct cdc_info *)buffer;
        u16 head_crc16 = CRC16(buffer,info_len - sizeof(info->head_crc16));

        printf("\n head_crc16  = 0x%x info->head_crc16 = 0x%x\n",head_crc16,info->head_crc16);
        //先处理正确的，因为错误总是很少出现
        if(head_crc16 == info->head_crc16){
            //说明数据接收不完整
            if(info->data_len > cbuf_get_data_size(&__cbuffer)){
                offset = info_len; //保留当前头部
            }else{
                char *data = malloc(info->data_len);
                if(data){
                    u16 crc16 = 0;
                    cbuf_read(&__cbuffer,data,info->data_len);
                    int ret = cbuf_read(&__cbuffer,&crc16,2);
                    printf("\n >>>>>>>>>>>>>>ret = %d\n",ret);
                    put_buf(info,info_len);
                    printf("\n data:");
                    put_buf(data,info->data_len);

                    printf("\n crc16 = 0x%x CRC16(data,info.data_len) = 0x%x \n",crc16,CRC16(data,info->data_len) );

                    if(crc16 == CRC16(data,info->data_len)){
                        cdc_host_cmd_deal(data,info->data_len);
                    }
                    free(data);
                }else{
                    printf("\n >>>>>>>>>>>>>>>>>>>malloc fail\n");
                }
                offset = 0;
            }

        }else{
            if(buffer[0] != 0x55){
                offset = info_len - 1;//表当前已经接收到这么多的数据
                //数据前移
                buffer[0] = buffer[1];
                buffer[1] = buffer[2];
                buffer[2] = buffer[3];
                buffer[3] = buffer[4];
                buffer[4] = buffer[5];
                continue;
            }else if(buffer[0] == 0x55 && buffer[1] != 0xAA){
                offset = info_len - 1;
                //数据前移
                buffer[0] = buffer[1];
                buffer[1] = buffer[2];
                buffer[2] = buffer[3];
                buffer[3] = buffer[4];
                buffer[4] = buffer[5];
                continue;
            }else{
                offset = 0;
                continue;
            }
        }
    }while(1);

__exit:
    if(buffer){
        free(buffer);
    }
    return 0;
}
void *cdc_dev = NULL;

void cdc_write_data(u8 *data,u8 len)
{
    int offset  = 0;
    int wlen = sizeof(struct cdc_info) + len + 2;
    char *buf = malloc(wlen);//数据头数据 + 加长度 + 2个字节的crc
    struct cdc_info *info = (struct cdc_info *)buf;

    info->head0 =  0x55;
    info->head1 = 0xAA;
    info->data_len = len;
    info->head_crc16 = CRC16(info,sizeof(struct cdc_info) - sizeof(info->head_crc16));
    offset+=sizeof(struct cdc_info);
    memcpy(buf + sizeof(struct cdc_info ),data,len);
    offset += len;
    u16 crc16  = CRC16(data,len);
    memcpy(&buf[offset] ,&crc16,sizeof(crc16));
  //  printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>1\n");
    put_buf(buf,wlen);
    printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>2\n");
    if(cdc_dev){
        printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>3\n");
        dev_write(cdc_dev,buf,wlen);
    //    printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>4\n");
    }
    free(buf);
    printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>5\n");
}



int recv_ble_data_for_7016(u8 *buffer,int buffer_len)
{
    u8 *data = malloc(buffer_len);
    if(data){
        memcpy(data,buffer,buffer_len);
        printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
        os_taskq_post("doorbell_7016_task", 3, 0, data, buffer_len);
    }

}

int send_ble_data_to_7016(u8 *data,int data_len)
{
    int len = 0;
    len = sizeof(CMD_INFO) + data_len;

    printf("\n data len = %d\n",data_len);

    printf("\n >>>>>>>>>>>>%s %d \n",__func__,__LINE__);
    printf("\n sizeof(CMD_INFO) = %d \n",sizeof(CMD_INFO));
    CMD_INFO *info = NULL;
    info = malloc(len);

     //设置蓝牙名
    info->cmd = SEND_BLE_DATA;
    memcpy( info->data, data,data_len);
    put_buf(info,len);
    cdc_write_data(info,len);
    free(info);
    return 0;
}


int set_7016_ble_name(u8 *ble_name)
{

 //   printf("\n set_7016_ble_name\n ");
    int len = 0;
    len = sizeof(CMD_INFO) + strlen(ble_name);
    printf("\n sizeof(CMD_INFO) = %d \n",sizeof(CMD_INFO));
    CMD_INFO *info = NULL;
    info = malloc(len);

     //设置蓝牙名
    info->cmd = SET_BLE_NAME;
    memcpy( info->data, ble_name,strlen(ble_name));
    put_buf(info,len);
    cdc_write_data(info,len);
    free(info);
    return 0;
}


int set_7016_ble_on(void)
{

   //  printf("\n set_7016_ble_on\n ");
    int len = 0;
    len = sizeof(CMD_INFO) + 1;
    printf("\n sizeof(CMD_INFO) = %d \n",sizeof(CMD_INFO));
    CMD_INFO *info = NULL;
    info = malloc(len);

     //设置蓝牙名
    info->cmd = SET_BLE_ON;
    info->data[0] = 0;
    put_buf(info,len);
    cdc_write_data(info,len);
    free(info);
    return 0;
}


int set_7016_ble_off(void)
{

    printf("\n set_7016_ble_off\n ");
    int len = 0;
    len = sizeof(CMD_INFO) + 1;
    printf("\n sizeof(CMD_INFO) = %d \n",sizeof(CMD_INFO));
    CMD_INFO *info = NULL;
    info = malloc(len);

     //设置蓝牙名
    info->cmd = SET_BLE_OFF;
    info->data[0] = 0;
    put_buf(info,len);
    cdc_write_data(info,len);
    free(info);
    return 0;
}


int set_7016_ble_name_on(){


    char ssid[20] = {0};
    char pass[20] = {0};
    avsdk_get_wifi_conf(ssid, pass);
    printf("\n >>>>>>>>>>>>>>>>>>>>>ble name=%s\n", ssid); //JIELI1JLLPV1001395190B0F

    set_7016_ble_name(ssid);



}

int cdc_host_init(void)
{

    cbuf_init(&__cbuffer, buffer, RT_TALK_BUFFER_SIZE);
    cdc_dev = dev_open("cdc0", NULL); /* 需要在板级定义cdc0设备 */
    msleep(20);
    u8 data[2];
    //获取音量
    data[0] = GET_VOLUME;
    data[1] = 0;
    cdc_write_data(data,sizeof(data));
   if(get_poweron_net_config_state()==0){ //配网模式下  打开蓝牙
    char ssid[20] = {0};
    char pass[20] = {0};
    avsdk_get_wifi_conf(ssid, pass);
    printf("\n >>>>>>>>>>>>>>>>>>>>>netconfig ble name=%s\n", ssid);
#if  1
   // set_7016_ble_name(ssid);
   if(cdc_dev){
    printf("\n cdc open ok\n");
    sys_timeout_add(NULL,set_7016_ble_name_on , 2000);
    }
#else

    set_7016_ble_name(ssid);

#endif

   }
    return cdc_dev ? 0 : -1;
}
int cdc_host_uninit(void)
{
    if(cdc_dev){
        dev_close(cdc_dev);
        cdc_dev = NULL;
    }
}

//CDC主机例子
static void host_cdc_send_demo(void)
{
    int len = 0;
    void *cdc_dev = NULL;
    cdc_dev = dev_open("cdc0", NULL); /* 需要在板级定义cdc0设备 */
    if (cdc_dev) {
        /* os_time_dly(100); */
        puts("cdc dev open success\n");
        u8 buf[4] = {0x01, 0x02, 0x03, 0x55};
        len = dev_write(cdc_dev, buf, 4);
        printf("len=%d\n", len);
        u8 buf1[4] = {0x09, 0x0a, 0x0b, 0x5a};
        len = dev_write(cdc_dev, buf1, 4);
        printf("len=%d\n", len);
        dev_close(cdc_dev);
    }
}

//用于cdc枚举失败打印描述符使用
#if 0
u8 * user_usb_cdc_interface_descriptor(u8 *pBuf, struct usb_interface_descriptor *if_descint, int total_len)
{
	u8 *tmp = pBuf;

    memcpy(if_descint, pBuf + 35, pBuf[0]);
    put_buf(if_descint,44);
	tmp += 44;
    put_buf(tmp,44);
	return tmp;

    //return NULL;
}
#endif // 0
