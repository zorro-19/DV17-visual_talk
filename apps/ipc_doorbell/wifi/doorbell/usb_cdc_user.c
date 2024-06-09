#include "system/includes.h"
#include "server/usb_server.h"
#include "doorbell_event.h"
#include "usb_audio.h"
#include "usb_host_user.h"
extern void cdc_write_data(u8 *data,u8 len);
//�ӻ�����
#if 0
//���
static int cdc_user_output(u8 *buf, u32 len)
{

     puts("cdc output\n");
     put_buf(buf, len);

    return len;
}
//����
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
	u16 data_len;//��ʾ�����ж�������
	u16 head_crc16;
	//���ڽ������ݵ�crcֻ����data
} __attribute__((packed));


enum{
    SHANG_YI_SHOU = 0,
    XIA_YI_SHOU = 1,
    ZHAN_TING_BO_FANG = 2,
    BO_FANG_YIN_YUE = 3,
    ZHENG_JIA_YIN_LIANG = 4,
    JIANG_DI_YIN_LIANG = 5,
    HUANG_XING_CI = 6,//���Ѵ�
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
    //��������
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




// ���豸������0-14��ӳ�䵽app������0-10��
int mapDeviceVolumeToAppVolume(int deviceVolume) {
    // ��������Ƿ�����Ч��Χ��
    if (deviceVolume < 0 || deviceVolume > 14) {
        return -1; // ������Ч
    }

    // ����ӳ�乫ʽ�����������뵽��ӽ�������
    double appVolumeDouble = (deviceVolume * 10.0) / 14.0;
    printf("\n appVolumeDouble:%f\n",appVolumeDouble);
    int appVolume = round(appVolumeDouble);

    // ȷ��app������0-10��Χ��
    return (appVolume < 0) ? 0 : ((appVolume > 10) ? 10 : appVolume);
}

// ��app������0-10��ӳ����豸������0-14��
int mapAppVolumeToDeviceVolume(int appVolume) {
    // ��������Ƿ�����Ч��Χ��
    if (appVolume < 0 || appVolume > 10) {
        return -1; // ������Ч
    }

    // ����ӳ�乫ʽ�����������뵽��ӽ�������
    double deviceVolumeDouble = (appVolume * 14.0) / 10.0;
     printf("\n deviceVolumeDouble:%f\n",deviceVolumeDouble);

    int deviceVolume = round(deviceVolumeDouble);

    // ȷ���豸������0-14��Χ��
    return (deviceVolume < 0) ? 0 : ((deviceVolume > 14) ? 14 : deviceVolume);
}

#endif

#if 0


// �豸������app������ӳ������ұ�
int deviceToAppMap[15] = {
    0,  // �豸���� 0 ӳ�䵽 app ���� 0
    0,  // �豸���� 1 ӳ�䵽 app ���� 0����1��ȡ���ھ���ӳ����ԣ�
    1,  // �豸���� 2 ӳ�䵽 app ���� 1
    1,  // �豸���� 3 ӳ�䵽 app ���� 1
    2,  // �豸���� 4 ӳ�䵽 app ���� 2
    2,  // �豸���� 5 ӳ�䵽 app ���� 2
    3,  // �豸���� 6 ӳ�䵽 app ���� 3
    3,  // �豸���� 7 ӳ�䵽 app ���� 3
    4,  // �豸���� 8 ӳ�䵽 app ���� 4
    5,  // �豸���� 9 ӳ�䵽 app ���� 5
    6,  // �豸���� 10 ӳ�䵽 app ���� 6
    7,  // �豸���� 11 ӳ�䵽 app ���� 7
    8,  // �豸���� 12 ӳ�䵽 app ���� 8
    9,  // �豸���� 13 ӳ�䵽 app ���� 9
    10  // �豸���� 14 ӳ�䵽 app ���� 10
};

// app�������豸�����ķ���ӳ���
int appToDeviceMap[11] = {
    0,  // app���� 0 ӳ��� �豸���� 0
    2,  // app���� 1 ӳ��� �豸���� 2
    4,  // app���� 2 ӳ��� �豸���� 4
    6,  // app���� 3 ӳ��� �豸���� 6
    8,  // app���� 4 ӳ��� �豸���� 8
    9,  // app���� 5 ӳ��� �豸���� 9
    10,  // app���� 6 ӳ��� �豸���� 10
    11,  // app���� 7 ӳ��� �豸���� 11
    12,  // app���� 8 ӳ��� �豸���� 12
    13,  // app���� 9 ӳ��� �豸���� 13
    14,  // app���� 10 ӳ��� �豸���� 14

    // ... ��Ҫ����deviceToAppMap����������
    // ע�⣺������Ҫ����deviceToAppMap��ȷ��ӳ���ǿ����
};

// ��ʼ������ӳ���
void initReverseMap() {
    for (int i = 0; i < 15; ++i) {
        int appVolume = deviceToAppMap[i];
        if (appToDeviceMap[appVolume] == 0) { // ȷ���������Ѵ��ڵ�ӳ��
            appToDeviceMap[appVolume] = i;
        }
    }
}

// ���豸����ӳ�䵽app����
int mapDeviceVolumeToAppVolume(int deviceVolume) {
    if (deviceVolume < 0 || deviceVolume > 14) {
        return -1; // ������Ч
    }
    return deviceToAppMap[deviceVolume];
}

// ��app����ӳ����豸����
int mapAppVolumeToDeviceVolume(int appVolume) {
    if (appVolume < 0 || appVolume > 10) {
        return -1; // ������Ч
    }
    return appToDeviceMap[appVolume];
}

int test_map() {
    // ��ʼ������ӳ���
    initReverseMap();

    // ʾ�������豸����ӳ�䵽app������Ȼ����ӳ����豸����
    for (int deviceVolume = 0; deviceVolume <= 14; ++deviceVolume) {
        int appVolume = mapDeviceVolumeToAppVolume(deviceVolume);
        if (appVolume != -1) {
            printf("�豸���� %d ��Ӧ�� app ������ %d\n", deviceVolume, appVolume);

            // �ٽ�app����ӳ����豸����
            int mappedDeviceVolume = mapAppVolumeToDeviceVolume(appVolume);
            if (mappedDeviceVolume != -1) {
                printf("app ���� %d ӳ��ص��豸������ %d\n", appVolume, mappedDeviceVolume);
                // ����ʹ���˲��ұ�mappedDeviceVolume Ӧ�����ǵ��� deviceVolume
            } else {
                printf("ӳ��app����ʱ����\n");
            }
        } else {
            printf("������豸����ֵ��Ч\n");
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

    if(cmd==HUANG_XING_CI){ //���Ѵ�

    post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "rec.adp");

    }


    if(cmd != GUANG_BI_PING_MU){
        //����Ļ
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
        //��һ��

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
        //��һ��
        break;
    case ZHAN_TING_BO_FANG:
        //��ͣ����
        break;
    case BO_FANG_YIN_YUE:
        //��������
        break;
    case ZHENG_JIA_YIN_LIANG:
       // ac7016_volume += 5;
        ac7016_volume += 2;
        if(ac7016_volume>=14){//���������������10, �ӻ��������14


        ac7016_volume=14;
        }
        u8 data[2];
        printf("\n >>>>>>>>>ZHENG_JIA_YIN_LIANG_ac7016_volume = %d\n",ac7016_volume);

        //��������
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
        //��������
        data[0] = GET_VOLUME;
        data[1] = 0;
        cdc_write_data(data,sizeof(data));
        break;
    case GUANG_BI_PING_MU:
        //�ر���Ļ
        event.arg = "VoiceCmd";
        event.type = SYS_DEVICE_EVENT;
        event.u.dev.event  = 0;
        sys_event_notify(&event);
        break;
    case DA_KAI_PING_MU:
        //����Ļ

        break;
    case HU_JIAO_DIAN_HUA:
        //���е绰
        event.arg  = (void *)KEY_WAKEUP;
        event.type = SYS_KEY_EVENT;
        event.u.key.event = KEY_EVENT_CLICK;
        event.u.key.value = KEY_OK;
        sys_event_notify(&event);

        break;
    case DA_KAI_LU_XIANG:
        //��¼��
        break;
    case GUANG_BI_LU_XIANG:
        //�ر�¼��
        break;
    case ZHUA_PAI_ZHAO_PIAN:
        //ץ����Ƭ
        break;
    case DA_KAI_HUI_FANG:
        //�򿪻ط�
        break;
    case DA_KAI_LIU_YAN:
        //������

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
//        }else{ //�����һ��
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
	u16 data_len;//��ʾ�����ж�������
	u16 head_crc16;
	//���ڽ������ݵ�crcֻ����data
} __attribute__((packed));

*/

/**
 * @brief cdc�������������庯��
 *
 * @param buf: ��������buffer
 * @param len: ���յ����ݳ���,С��������
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
        //�����������С����С������ͷ+����+����crc�Ȳ�����
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
        //�ȴ�����ȷ�ģ���Ϊ�������Ǻ��ٳ���
        if(head_crc16 == info->head_crc16){
            //˵�����ݽ��ղ�����
            if(info->data_len > cbuf_get_data_size(&__cbuffer)){
                offset = info_len; //������ǰͷ��
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
                offset = info_len - 1;//��ǰ�Ѿ����յ���ô�������
                //����ǰ��
                buffer[0] = buffer[1];
                buffer[1] = buffer[2];
                buffer[2] = buffer[3];
                buffer[3] = buffer[4];
                buffer[4] = buffer[5];
                continue;
            }else if(buffer[0] == 0x55 && buffer[1] != 0xAA){
                offset = info_len - 1;
                //����ǰ��
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
    char *buf = malloc(wlen);//����ͷ���� + �ӳ��� + 2���ֽڵ�crc
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

     //����������
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

     //����������
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

     //����������
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

     //����������
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
    cdc_dev = dev_open("cdc0", NULL); /* ��Ҫ�ڰ弶����cdc0�豸 */
    msleep(20);
    u8 data[2];
    //��ȡ����
    data[0] = GET_VOLUME;
    data[1] = 0;
    cdc_write_data(data,sizeof(data));
   if(get_poweron_net_config_state()==0){ //����ģʽ��  ������
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

//CDC��������
static void host_cdc_send_demo(void)
{
    int len = 0;
    void *cdc_dev = NULL;
    cdc_dev = dev_open("cdc0", NULL); /* ��Ҫ�ڰ弶����cdc0�豸 */
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

//����cdcö��ʧ�ܴ�ӡ������ʹ��
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
