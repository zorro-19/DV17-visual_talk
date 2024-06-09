/*************************************************************************
	> File Name: usb_app.c
	> Author:
	> Mail:
	> Created Time: Tue 14 Feb 2017 09:14:45 AM HKT
 ************************************************************************/

#include "system/includes.h"
#include "server/usb_server.h"
#include "app_config.h"
#include "action.h"
#include "server/ui_server.h"
#include "ui_ctr_def.h"
#include "style.h"
#include "storage_device.h"
#include "user_isp_cfg.h"
#include "server/video_server.h"
#include "asm/lcd_config.h"
#include "usb_cam_h264.h"

enum {
    USB_SLAVE_MASS_STORAGE,
    USB_SLAVE_CAMERA,
    USB_SLAVE_ISP_TOOL,
    USB_SLAVE_MAX,
};

enum {
    UVC_ENC_R0 = 0, //不旋转
    UVC_ENC_R90,    //旋转90
    UVC_ENC_R270,   //旋转270
};

#define CUSTOM_RESOLUTION        1
#define USER_DEFINED_TOOL        1
#define UPDATE_UVC_ISP_SCENES    0
#define UVC_CUSTOM_UPGRADE_EN    0   //uvc升级功能,需要搭配pc上位机

#define UVC_TWO_H264_OUTPUT      0   //uvc双h264码流输出

#define USB_STATE_NO_DEV         0x0
#define USB_STATE_DEVICE_MOUNT   0x1

struct usb_app_handle {
    u8 state;
    u8 mode;
#if UPDATE_UVC_ISP_SCENES
    u8 isp_scene;
#endif
    struct server *usb_slave;
    struct server *usb_host;
    struct server *ui;
    u8 *buf;
    u32 buf_size;
};

static struct usb_app_handle handler = {
    .state = USB_STATE_NO_DEV,
};

#define __this  (&handler)

extern int usb_app_set_config(struct intent *it);
extern int usb_app_get_config(struct intent *it);
/*
static int usb_app_mode_start(void)
{
    int err = 0;

    if (!__this->usb_slave){

        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave){
            return -EFAULT;
        }
    }

    return err;
}
*/

/*
 *mass storage挂载的设备列表
 */
const static char *mass_storage_dev_list[] = {
    SDX_DEV,
};


static int show_slave_list(void)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        return -EINVAL;
    }
    req.show.id = ID_WINDOW_USB_SLAVE;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
#endif
    return 0;
}

static int hide_slave_list(void)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        return -EINVAL;
    }
    req.hide.id = ID_WINDOW_USB_SLAVE;
    server_request(__this->ui, UI_REQ_HIDE, &req);

#endif
    return 0;

}
static int usb_slave_start(void)
{
    show_slave_list();

    /* usb_connect(USB_CAMERA); */
    //usb_isp_tool_test();

    return 0;
}

static int usb_slave_stop(void)
{
    /*
     *关闭服务、释放资源
     */
#if UPDATE_UVC_ISP_SCENES
    if (__this->isp_scene) {
        stop_update_isp_scenes();
    }
#endif

    if (__this->usb_slave) {
        server_close(__this->usb_slave);
        __this->usb_slave = NULL;
        unmount_sd_to_fs(CONFIG_STORAGE_PATH);
        mount_sd_to_fs(SDX_DEV);
    }

    if (__this->buf) {
        free(__this->buf);
        __this->buf = NULL;
    }


    __this->state = USB_STATE_NO_DEV;

    hide_slave_list();
    return 0;
}


#ifdef CONFIG_VIDEO4_ENABLE
struct server *video_display[CONFIG_VIDEO_REC_NUM];
static void usb_disp_camera_start()
{
    int err = 0;
    union video_req vreq = {0};
    video_display[0] = server_open("video_server", (void *)"video4.0.0");
    if (video_display[0]) {
        vreq.display.fb 		        = "fb1";
        vreq.display.left  	        = 0;
        vreq.display.top 	        = 0;
        vreq.display.width 	        = LCD_DEV_WIDTH / 2;
        vreq.display.height 	        = LCD_DEV_HIGHT / 2;
        vreq.display.border_left     = 0;
        vreq.display.border_top      = 0;
        vreq.display.border_right    = 0;
        vreq.display.border_bottom   = 0;

        vreq.display.camera_config   = NULL;
        /* req.display.camera_config   = load_default_camera_config; */
        vreq.display.camera_type     = VIDEO_CAMERA_MUX;
        /* vreq.display.camera_type     = VIDEO_CAMERA_NORMAL; */
        err = server_request(video_display[0], VIDEO_REQ_DISPLAY, &vreq);
        if (err) {
            printf("display req err = %d!!\n", err);
            server_close(video_display[0]);
            video_display[0] = NULL;
        }

        video_display[1] = server_open("video_server", (void *)"video4.1.0");
        vreq.display.left  	        = LCD_DEV_WIDTH / 2;
        vreq.display.top 	        = 0;
        err = server_request(video_display[1], VIDEO_REQ_DISPLAY, &vreq);
        if (err) {
            printf("display req err = %d!!\n", err);
            server_close(video_display[1]);
            video_display[1] = NULL;
        }

        video_display[2] = server_open("video_server", (void *)"video4.2.0");
        vreq.display.left  	        = 0;
        vreq.display.top 	        = LCD_DEV_HIGHT / 2;
        err = server_request(video_display[2], VIDEO_REQ_DISPLAY, &vreq);
        if (err) {
            printf("display req err = %d!!\n", err);
            server_close(video_display[2]);
            video_display[2] = NULL;
        }

        video_display[3] = server_open("video_server", (void *)"video4.3.0");
        vreq.display.left  	        = LCD_DEV_WIDTH / 2;
        vreq.display.top 	        = LCD_DEV_HIGHT / 2;
        err = server_request(video_display[3], VIDEO_REQ_DISPLAY, &vreq);
        if (err) {
            printf("display req err = %d!!\n", err);
            server_close(video_display[3]);
            video_display[3] = NULL;
        }
    }
}


static void usb_disp_camera_stop()
{
    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return;
    }
    if (__this->mode == 2) {
        union video_req req = {0};

        for (int id = 0; id < CONFIG_VIDEO_REC_NUM; id++) {
            if (video_display[id]) {
                req.display.state 	= VIDEO_STATE_STOP;
                server_request(video_display[id], VIDEO_REQ_DISPLAY, &req);

                server_close(video_display[id]);
                video_display[id] = NULL;
            }
        }
        /* struct sys_event e; */
        /* e.type = SYS_DEVICE_EVENT; */
        /* e.arg = "sys_power"; */
        /* e.u.dev.event = DEVICE_EVENT_POWER_SHUTDOWN; */
        /* sys_event_notify(&e); */
    }
}
#endif


static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    extern int set_usb_mass_storage();
    extern int set_usb_camera();

    switch (state) {
    case APP_STA_CREATE:
        memset(__this, 0x0, sizeof(*__this));
#ifdef CONFIG_UI_ENABLE
        __this->ui = server_open("ui_server", NULL);
        if (!__this->ui) {
            return -EFAULT;
        }
#endif
        break;
    case APP_STA_START:
        switch (it->action) {
        case ACTION_USB_SLAVE_MAIN:
#ifdef PRODUCT_TEST_ENABLE
            u8 product_main(void);
            if (product_main()) {
                usb_connect(USB_CAMERA | USB_MIC | USB_SPEAKER);
                break;
            }
#endif
            usb_slave_start();
            __this->mode = 0;
            break;
        case ACTION_USB_SLAVE_SET_CONFIG:
            __this->mode = usb_app_set_config(it);
            break;
        case ACTION_USB_SLAVE_GET_CONFIG:
            usb_app_get_config(it);
            break;
        }
        break;
    case APP_STA_PAUSE:
    /* return -EFAULT; */
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        usb_slave_stop();
        /* #ifdef CONFIG_VIDEO4_ENABLE */
        /* usb_disp_camera_stop(); */
        /* #endif */
        break;
    case APP_STA_DESTROY:
        if (__this->ui) {
            server_close(__this->ui);
        }
        break;
    }

    return 0;

}
static int usb_app_key_event_handler(struct key_event *key)
{

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_OK:
            //usb_app_select();
            break;
        case KEY_UP:

            break;
        case KEY_DOWN:

            break;
        default:
            break;
        }
        break;
    case KEY_EVENT_LONG:
        if (key->value == KEY_POWER) {
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

static int usb_app_device_event_handler(struct sys_event *event)
{

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        break;
    case DEVICE_EVENT_ONLINE:
        break;
    case DEVICE_EVENT_OUT:
        //usb_app_pause();
        break;
    }
    return false;
}

static int event_handler(struct application *app, struct sys_event *event)
{

    switch (event->type) {
    case SYS_KEY_EVENT:
        return usb_app_key_event_handler(&event->u.key);
    case SYS_DEVICE_EVENT:
        return usb_app_device_event_handler(event);
    default:
        return false;
    }
    return false;
}


static const struct application_operation usb_app_ops = {
    .state_machine  = state_machine,
    .event_handler  = event_handler,
};

REGISTER_APPLICATION(app_usb) = {
    .name 	= "usb_app",
    .action	= ACTION_USB_SLAVE_MAIN,
    .ops 	= &usb_app_ops,
    .state  = APP_STA_DESTROY,
};


#if USER_DEFINED_TOOL
/*
 *user defined tool demo
 */
static int usb_tool_response(void *_tool, void *buf, int len)
{
    struct usb_tool *tool = (struct usb_tool *)_tool;
    u8 *cmd = (u8 *)buf;

    if (!tool || !buf) {
        return -EINVAL;
    }

    if (cmd[0] != USER_DEFINED_TOOL_CMD) {
        log_w("not user defined tool");
        return 0;
    }

    switch (cmd[1]) {
    case 0x0:
        break;
    default:
        break;
    }
    return 0;
}
#endif
static void set_sd_card_protect_status(u8 protect)
{
    void *sd_dev = NULL;
    sd_dev = dev_open(SDX_DEV, 0);
    if (!sd_dev) {
        log_e("%s open falid!\n", SDX_DEV);
        return;
    }
    dev_ioctl(sd_dev, IOCTL_SET_WRITE_PROTECT, protect);
    dev_close(sd_dev);

}
/*
 *设置mass storage
 */
int set_usb_mass_storage(void)
{
    struct usb_req req = {0};
    int err = 0;

    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return 0;
    }

    if (!__this->usb_slave) {
        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave) {
            return -EFAULT;
        }
    }
    //printf("debug isd download.\n");
    /*
     *设置请求参数：
     *类型：USB_MASS_STORAGE
     *挂载设备数量
     *挂载设备列表
     *状态：从机连接
     */
    req.type = USB_MASS_STORAGE;
    req.storage.dev_num = 1;
    req.storage.dev = mass_storage_dev_list;
    /*req.storage.name = "hunter"; //PC上显示的卷标*/
#if USER_DEFINED_TOOL
    req.storage.tool_response = usb_tool_response;
#endif
    req.state = USB_STATE_SLAVE_CONNECT;
    set_sd_card_protect_status(0); //0:不写保护 1:写保护
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }

    return 0;
}

#if CUSTOM_RESOLUTION
static struct uvc_reso_info uvc_mjpg_reso_info[2] = {
    {
        .num = 3,
        .reso = {
            {1280, 720, 25},
            {640, 480, 25},
            {320, 240, 25},
        },
    },
    {
        .num = 3,
        .reso = {
            {1280, 720, 25},
            {640, 480, 25},
            {320, 240, 25},
        },
    }
};
static struct uvc_reso_info uvc_h264_reso_info = {
    .num = 5,
    .reso = {
        {1920, 1080},
        {1280, 720},
        {800, 600},
        {640, 480},
        {320, 240},

        /* {1088, 1920}, */
        /* {720, 1280}, */
        /* {480, 640}, */
    },
};
#endif

#if UVC_CUSTOM_UPGRADE_EN
static void go_mask_usb_update(void)
{
    strcpy((u8 *)(ISR_ENTRY - 128), "usb_update_mode");//usb uprgade
    PWR_CON |= BIT(4);
    while (1);
}
static int uvc_processing_unit_response(struct uvc_unit_ctrl *ctl_req)
{
    u8 upgd[] = {'u', 'p', 'g', 'd'};
    static int pos = 0;
    /* printf("=======>> request : 0x%x, unit : 0x%x\n", ctl_req->request, ctl_req->unit); */
    /* put_buf(ctl_req->data, ctl_req->len); */
    int value = 0;
    int msg[32];
    int err = 0;
    static u8 i = 0;
    if (ctl_req->unit == 0x0a) {//white_blance
        switch (ctl_req->request) {
        case 0x01:
            /* put_buf(ctl_req->data, ctl_req->len); */
            /* if (ctl_req->bState == 2) */
        {
            put_buf(ctl_req->data, ctl_req->len);
            log_d("in pos %d\n", pos);
            log_d("ctl_req->data[0] 0x%x, upgd[pos] 0x%x\n", ctl_req->data[0], upgd[pos]);
            if (ctl_req->data[0] == upgd[pos]) {
                pos++;
                if (pos == sizeof(upgd)) {
                    puts("<<<<<<enter>>>>>>>>\n");
                    go_mask_usb_update();
                }
            } else {
                pos = 0;
            }
            return 1;
        }
        break;
        default:
            return -EINVAL;
            break;
        }
    } else if (ctl_req->unit == 0xaa) {
        switch (ctl_req->request) {
        case 0x01:
            break;
        default:
            return -EINVAL;
            break;
        }

    } else {
        return -EINVAL;
    }

    return 1;
}
#endif

/* int jpeg_frame_size_align(int size) */
/* { */
/* return size;     */
/* } */
#if 0
void user_uvc_camera_set_crop(void *fd, u16 width, u16 height)
{
    static struct video_source_crop crop = {0};
    if (width == 640 && height == 480) {

        crop.x_offset = 240;
        crop.y_offset = 0;
        crop.width = 1920 - crop.x_offset * 2;
        crop.height = 1080;

        dev_ioctl(fd, VIDIOC_SET_ENC_CROP, (u32)&crop);
    }
}
#endif

int jpeg_frame_size_align(int size)
{
    struct application *app = get_current_app();
    if (app && !strcmp(app->name, "usb_app")) {
        return size;
    }

    return ((size + 0x200 - 1) / 0x200) * 0x200; //末尾512对齐，会导致jpeg帧不是FFD9结尾
}
/*
 *设置PC Camera
 */
int set_usb_camera(void)
{
    struct usb_req req = {0};
    int err = 0;

    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return 0;
    }

    if (!__this->usb_slave) {
        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave) {
            return -EFAULT;
        }
    }
    /*
     *设置请求参数
     *类型：USB_CAMERA (可增加isp工具，调屏工具选项)
     *设置camera的编码所需buffer、buffer长度
     *quality -- 设置图像质量
     */
#ifdef CONFIG_VIDEO4_ENABLE
    /* req.type = USB_SCREEN_TOOL; */
    req.type = USB_CAMERA | USB_SCREEN_TOOL;
    req.camera[0].name = "video4.0.0";
#else
    req.type = USB_CAMERA | USB_ISP_TOOL | USB_SCREEN_TOOL;
    req.camera[0].name = "video0.0";
    /* req.type = USB_CAMERA | USB_ISP_TOOL; */
#ifdef CONFIG_UVC_SLAVE_ENABLE
    req.type = USB_CAMERA;
#endif
#endif
    if (!__this->buf) {
        __this->buf = (u8 *)malloc(USB_CAMERA_BUF_SIZE);
        if (!__this->buf) {
            return -ENOMEM;
        }
    }
    __this->buf_size = USB_CAMERA_BUF_SIZE;

#if !UVC_TWO_H264_OUTPUT
    //usb camera0 为MJPEG/YUYV视频流定义
    req.camera[0].enable = 1;
    req.camera[0].format = USB_VIDEO_FMT_MJPG;// | USB_VIDEO_FMT_YUY2;
    req.camera[0].buf = __this->buf;
    req.camera[0].buf_size = __this->buf_size;
    req.camera[0].info = NULL;
    req.camera[0].quality = 1;
#if CUSTOM_RESOLUTION
    req.camera[0].info = uvc_mjpg_reso_info;
    req.camera[0].fmt_num = 2;

    req.camera[0].rotate = UVC_ENC_R0;
#endif

#else
    //UVC_TWO_H264_OUTPUT config
    req.camera[0].enable = 1;
    req.camera[0].format = USB_VIDEO_FMT_H264;
    req.camera[0].name = "usb_cam1"; //device 设备名，需要在板级文件中定义
    req.camera[0].video_open = usb_video_h264_open2; //h264 视频流open回调
    req.camera[0].video_close = usb_video_h264_close2; //h264 视频流close回调
    req.camera[0].info = &uvc_h264_reso_info;
    req.camera[0].fmt_num = 1;
    req.camera[0].buf = NULL;
    req.camera[0].buf_size = 0;
    if (__this->buf) {
        free(__this->buf);
        __this->buf = NULL;
    }
    req.camera[0].rotate = UVC_ENC_R0;
#endif
#if UVC_CUSTOM_UPGRADE_EN
    req.camera[0].processing_unit_response = uvc_processing_unit_response;
#endif

    //usb camera1 为H264视频流定义
    req.camera[1].enable = 0; //使能开关
    req.camera[1].split_cam_enable = 0; // 表示是否分离出第2个uvc设备,双码流UVC设备
#if UVC_TWO_H264_OUTPUT
    req.camera[1].enable = 1; //使能开关
    req.camera[1].split_cam_enable = 2; //
#endif
    req.camera[1].name = "usb_cam0"; //device 设备名，需要在板级文件中定义
    req.camera[1].info = &uvc_h264_reso_info;
    req.camera[1].format = USB_VIDEO_FMT_H264;
    req.camera[1].video_open = usb_video_h264_open; //h264 视频流open回调
    req.camera[1].video_close = usb_video_h264_close; //h264 视频流close回调
    req.camera[1].rotate = UVC_ENC_R0;
#ifdef PRODUCT_TEST_ENABLE
    req.camera[1].split_cam_enable = 0;
#endif

    req.state = USB_STATE_SLAVE_CONNECT;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }

#if UPDATE_UVC_ISP_SCENES
    start_update_isp_scenes(__this->usb_slave);
    __this->isp_scene = 1;
#endif

    /* #ifdef CONFIG_VIDEO4_ENABLE */
    /* usb_disp_camera_start(); */
    /* #endif */

    return 0;
}

int switch_usb_camera(int id)
{
    struct usb_req req = {0};
    int err = 0;
    static char dev_name[32] = {0};

    if (__this->state != USB_STATE_DEVICE_MOUNT) {
        return -EFAULT;
    }

    /*
     *设置请求参数
     *类型：USB_CAMERA (可增加isp工具，调屏工具选项)
     *设置camera的编码所需buffer、buffer长度
     *quality -- 设置图像质量
     */
#ifdef CONFIG_VIDEO4_ENABLE
    /* req.camera[0].name = "video4.2.0"; */
    sprintf(dev_name, "video4.%d.0", id);
#else
    /* req.camera[0].name = "video0.0"; */
    sprintf(dev_name, "video%d.0", id);
#endif
    log_d("switch uvc camera %s", dev_name);
    req.camera[0].name = dev_name;
    req.type = USB_CAMERA ;

    req.state = UVC_STATE_SLAVE_SWITCH_CAMERA;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }
    return 0;
}

/*
 *设置PC Audio
 */
/* ******************************************* /
 *  uac microphone采样率格式配置:
 *  num: 采样率个数
 *  channel: 声道数
 *  bitwidth : 数据位数
 *  samplerate : 采样率
 *  packetsize : uac每包长度
 * *******************************************/
static struct uac_micreso_info microphone_info[] = {
    {
        .num = 4,
        {
            //channel bitwidt samplerate packetsize
            {1, 16, 8000,	256},
            {1, 16, 16000,	256},
            {1, 16, 44100,	256},
            {1, 16, 48000,	256},
        },
    },
};
int set_usb_microphone(void)
{
    struct usb_req req = {0};
    int err = 0;

    if (!__this->usb_slave) {
        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave) {
            return -EFAULT;
        }
    }

    req.type = USB_MIC;
#ifdef CONFIG_DIGITAL_MIC_ENABLE
    req.microphone.name = "plnk";  //数据源
#else
    req.microphone.name = "mic";  //数据源
#endif
    req.microphone.format = "pcm";//编码格式
    req.microphone.buf = NULL;
    req.microphone.buf_size = 0;
    req.microphone.info = &microphone_info[0];

    req.state = USB_STATE_SLAVE_CONNECT;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }

    return 0;
}

/* ******************************************* /
 *  uac speaker采样率格式配置:
 *  num: 采样率个数
 *  channel: 声道数
 *  bitwidth : 数据位数
 *  samplerate : 采样率
 * *******************************************/
static struct uac_micreso_info speaker_info[] = {
    {
        .num = 4,
        {
            //channel bitwidt samplerate packetsize
            {1, 16, 8000},
            {1, 16, 16000},
            {1, 16, 44100},
            {1, 16, 48000},
        },
    },
};
int set_usb_speaker(void)
{
    struct usb_req req = {0};
    int err = 0;

    if (!__this->usb_slave) {
        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave) {
            return -EFAULT;
        }
    }

    req.type = USB_SPEAKER;
    req.speaker.name = NULL;
    req.speaker.format = NULL;
    req.speaker.buf = NULL;
    req.speaker.buf_size = 0;
    req.speaker.info = &speaker_info[0];

    req.state = USB_STATE_SLAVE_CONNECT;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }

    return 0;
}





static int cdc_user_output(u8 *buf, u32 len)
{
    puts("cdc output\n");
    put_buf(buf, len);
    return len;
}
int cdc_user_input(u8 *buf, u32 len)
{
    struct usb_req req = {0};
    int err = 0;

    if (!__this->usb_slave) {
        log_e("usb slave not open\n");
        return -EFAULT;
    }

    req.type = USB_CDC;
    req.cdc.ibuf = buf;
    req.cdc.ilen = len;

    req.state = USB_STATE_SLAVE_DATA_TRANSFER;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err) {
        printf("usb_server request CDC fail\n");
        return err;
    }
    return 0;
}
int set_usb_cdc(int (*output)(u8 *obuf, u32 olen))
{
    struct usb_req req = {0};
    int err = 0;

    if (!__this->usb_slave) {
        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave) {
            return -EFAULT;
        }
    }
    req.type = USB_CDC;
    req.cdc.output = output;

    req.state = USB_STATE_SLAVE_CONNECT;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }

    return 0;
}




/*
 *返回录像模式
 */
int back_to_video_rec(void)
{

    struct intent it;
    struct application *app;

    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return 0;
    }
    init_intent(&it);
    app = get_current_app();
    if (app) {
        it.action = ACTION_BACK;
        start_app(&it);

        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_MAIN;
        start_app(&it);
    }

    return 0;
}

u8 get_usb_in_status()
{
    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return 1;
    }
    return 0;
}

int usb_device_ready()
{
    return dev_online("usb0");
}

int usb_connect(u32 state)
{
    u8 err = 0;

    if (state & USB_MASS_STORAGE) {
        err = set_usb_mass_storage();
        if (err) {
            return err;
        }
    }

    if (state & USB_CAMERA) {
        err = set_usb_camera();
        if (err) {
            return err;
        }
    }

    if (state & USB_MIC) {
        err = set_usb_microphone();
        if (err) {
            return err;
        }
    }
    if (state & USB_SPEAKER) {
        err = set_usb_speaker();
        if (err) {
            return err;
        }
    }
    if (state & USB_CDC) {
        err = set_usb_cdc(cdc_user_output);
        if (err) {
            return err;
        }
    }

    __this->state = USB_STATE_DEVICE_MOUNT;
    sys_power_auto_shutdown_pause();

    return 0;
}

int usb_disconnect(void)
{
    struct usb_req req = {0};
    int err = 0;

    if (__this->state != USB_STATE_DEVICE_MOUNT) {
        puts("usb slave not mount.\n");
        return 0;
    }

    if (!__this->usb_slave) {
        puts("usb slave err.\n");
        return -EFAULT;
    }

    /* req.state = USB_STATE_SLAVE_RESET; */
    /* err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req); */
    /* if (err != 0) { */
    /* puts("usb slave request err.\n"); */
    /* return -EFAULT; */
    /* } */

    server_close(__this->usb_slave);
    __this->usb_slave = NULL;
    __this->state = USB_STATE_NO_DEV;

    return 0;
}



#if 0
#define mbe32buf_to_cpu(x)   (((u32)(((u8 *)(x))[0]) << 24) | \
                        	((u32)(((u8 *)(x))[1]) << 16) | \
                        	((u32)(((u8 *)(x))[2]) << 8) | \
                        	((u32)((u8 *)(x))[3]))
#define mbe16buf_to_cpu(x)   (((u16)(((u8 *)(x))[0]) << 8) | (u16)(((u8 *)(x))[1]))

int user_cmd_read(struct scsi_private_request *req)
{
    u32 cmd;
    u16 len;
    u8 buf[16];//每次传输数据少于512byte

    cmd = mbe32buf_to_cpu(req->cmd + 2);
    len = mbe16buf_to_cpu(req->cmd + 6);

    switch (cmd) {
    case 0x12345678:
        //fill data to buf
        strcpy((char *)buf, "demo");
        //send data to pc
        req->ops.send(req->dev, (void *)buf, len);
        break;
    default:
        break;
    }
    return 0;
}


int user_cmd_write(struct scsi_private_request *req)
{
    u32 cmd;
    u16 len;
    u32 get_time;
    u8 buf[16];//每次传输数据少于512byte
    cmd = mbe32buf_to_cpu(req->cmd + 2);
    len = mbe16buf_to_cpu(req->cmd + 6);

    switch (cmd) {
    case 0x12345678:
        //get data form pc
        req->ops.read(req->dev, (void *)buf, len);
        /* printf("\n %s\n",buf); */
        break;
    default:
        break;
    }
    return 0;
}

#endif

#if 0
static const u8 user_device_desc[] = { //<Device Descriptor
    18,      // bLength: Size of descriptor
    1,       // bDescriptorType: Device
    0x00, 0x02,     // bcdUSB: USB 2.0
    0xef,      // bDeviceClass: Multi-interface Function 0xEF
    0x02,       // bDeviceSubClass: Multi-interface Function 2
    0x01,       // bDeviceProtocol: Multi-interface Function 1
    0x40,      // bMaxPacketSize0: 8/64 bytes
    0x24, 0x12,     // idVendor
    0x25, 0x27,     // idProduct
    0x00, 0x01,     // bcdDevice
    1,       // iManufacturer: Index to string descriptor that contains the string <Your Name> in Unicode
    2,       // iProduct: Index to string descriptor that contains the string <Your Product Name> in Unicode
    0,       // iSerialNumber: none
    0x01        // bNumConfigurations: 1
};

static const u8 user_serial_number_desc[] = {
    22,
    0x03,
    '1', 0,
    '2', 0,
    '3', 0,
    '4', 0,
    '5', 0,
    '6', 0,
    '7', 0,
    '8', 0,
    '9', 0,
    '0', 0,
};

/**@brief   拦截库里面的usb请求的setup阶段和stage_in阶段
 * @param   *setup_pkt  usb请求包
 * @param   *buf  stage_in阶段需要发送的数据
 * @param   *len  stage_in阶段需要发送数据的长度
 * @return  是否拦截usb库的默认请求响应
 *          - 1  拦截，由这个函数处理请求
 *          - 0  不拦截，走库里面的请求处理
 * @note    WEEK函数，库里面有同名函数，返回false，如果有另外定义这个名称的
 *          不带WEEK的函数，链接器会链接它。user_setup_handler()处理setup阶
 *          段和stage_in阶段，stage_out阶段由user_stage_out()处理
 */
int user_setup_handler(const u8 *setup_pkt, u8 *buf, u32 *len)
{
    int retval = false;
    u8 request_type, request;
    u16 wvalue, windex, wlength;
    request_type = setup_pkt[0];
    request = setup_pkt[1];
    wvalue  = ((u16)setup_pkt[3] << 8) | ((u16)setup_pkt[2]);
    windex  = ((u16)setup_pkt[5] << 8) | ((u16)setup_pkt[4]);
    wlength = ((u16)setup_pkt[7] << 8) | ((u16)setup_pkt[6]);
    u32 cur_len;

    switch (request_type & 0x1f) {  //recip
    case 0:  //device
        switch (request_type & 0x60) {  //type
        case 0x00:  //standard request
            switch (request) {
            case 0x06:  //USB_REQ_GET_DESCRIPTOR
                switch ((wvalue >> 8) & 0xff) {
                case 1:  //USB_DT_DEVICE
                    printf(">>> GET_DEVICE_DESCRIPTOR <<<\n");
                    cur_len = wlength > sizeof(user_device_desc) ? sizeof(user_device_desc) : wlength;
                    memcpy(buf, (u8 *)user_device_desc, cur_len);
                    //默认serial number index是0（关闭），需要填3让主机发请求下来
                    buf[16] = 3;
                    //填写实际要发送的长度
                    *len = cur_len;
                    retval = true;
                    break;
                case 3:  //USB_DT_STRING
                    switch ((wvalue & 0xff)) {
                    case 1:  //Manufacturer String
                        break;
                    case 2:  //Product String
                        break;
                    case 3:  //Serial Number String
                        printf(">>> GET_STRING_DESCRIPTOR: SERIAL_NUMBER <<<\n");
                        cur_len = wlength > sizeof(user_serial_number_desc) ? sizeof(user_serial_number_desc) : wlength;
                        memcpy(buf, user_serial_number_desc, cur_len);
                        *len = cur_len;
                        retval = true;
                        break;
                    }
                    break;
                }
                break;
            }
            break;
        case 0x20:  //class-specified request
            break;
        case 0x40:  //vendor-specified request
            break;
        }
        break;
    case 1:  //interface
        break;
    case 2:  //endpoint
        break;
    }
    return retval;
}

/**@brief   拦截库里面的usb请求的stage_out阶段
 * @param   *setup_pkt  usb请求包
 * @param   *buf  stage_out阶段接收到的数据
 * @param   *len  stage_out阶段接收到数据的长度
 * @return  是否拦截usb库的默认请求响应
 *          - 1  拦截，由这个函数处理请求
 *          - 0  不拦截，走库里面的请求处理
 * @note    WEEK函数，库里面有同名函数，返回false，如果有另外定义这个名称的
 *          不带WEEK的函数，链接器会链接它。user_setup_handler()处理setup阶
 *          段和stage_in阶段，stage_out阶段由user_stage_out()处理
 */
int user_stage_out(const u8 *setup_pkt, const u8 *buf, u32 len)
{
    return false;
}
#endif

#include "ch9.h"
#include "asm/usb_inc.h"

//当采用4录码流时才需要开启端点分配函数
#if 0
/**
 * @brief   用户usb ep弱定义函数,实现
 * @param:  type: USB设备类  ep_bitmap:ep端点占用情况
 * @return: 返回分配的ep号
 **/
u32 user_usb_ep_alloc(u32 type, u32 *ep_bitmap)
{
    u32 ep = -1;
    if (type == USB_CLASS_VIDEO) {
        if (*ep_bitmap & BIT(USB_EP3)) {
            ep = USB_EP3;
        } else if (*ep_bitmap & BIT(USB_EP4)) {
            ep = USB_EP4;
        } else if (*ep_bitmap & BIT(USB_EP1)) {
            ep = USB_EP1;
        } else if (*ep_bitmap & BIT(USB_EP2)) {
            ep = USB_EP2;
        }

    } else if (type == USB_CLASS_AUDIO) {
        if (*ep_bitmap & BIT(USB_EP6)) {
            /*如果uac采用ep6，最大支持采样率16000 单声道，并且需要在usb_host_user中将
            spk_host->ep_buffer = &audio_ep_buffer[512];改为spk_host->ep_buffer = &audio_ep_buffer[32];
            */
            ep = USB_EP6;
        }
    }

    return ep;

}
#endif // 0


#ifdef CONFIG_UVCX_ENABLE

/* 主机强制忽略uvc的中断端点 */
u8 ignore_uvc_int_ep(void)
{
    return 1; //强制忽略
}

#endif