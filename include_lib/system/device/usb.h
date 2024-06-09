
#ifndef __USB__API_H__
#define __USB__API_H__
#include "typedef.h"
#include "ioctl.h"
#ifdef __cplusplus
extern "C" {
#endif



/* typedef struct usb_wifi_fun{                                                                      */
/*     s32 (*usb_bulk_only_receive_int)(u8 ep,u8 *pbuf,u32 len);                                     */
/*     s32 (*usb_bulk_only_send)(u8 ep,u8 *pbuf,u32 len);                                            */
/*     s32 (*usb_ctl_msg)(u8 RequestType, u8 Request, u16 Value, u16 Index, u16 Length, void *pBuf); */
/* }USB_WIFI_FUN;                                                                                    */

struct uvc_parm {
    s16 brightness_min;
    s16 brightness_max;
    s16 brightness_def;
    s16 brightness_res;
    s16 brightness_cur;

    s16 contrast_min;
    s16 contrast_max;
    s16 contrast_def;
    s16 contrast_res;
    s16 contrast_cur;

    s16 hue_min;
    s16 hue_max;
    s16 hue_def;
    s16 hue_res;
    s16 hue_cur;

    s16 saturation_min;
    s16 saturation_max;
    s16 saturation_def;
    s16 saturation_res;
    s16 saturation_cur;

    s16 sharpness_min;
    s16 sharpness_max;
    s16 sharpness_def;
    s16 sharpness_res;
    s16 sharpness_cur;

    s16 gamma_min;
    s16 gamma_max;
    s16 gamma_def;
    s16 gamma_res;
    s16 gamma_cur;

    s16 white_balance_temp_min;
    s16 white_balance_temp_max;
    s16 white_balance_temp_def;
    s16 white_balance_temp_res;
    s16 white_balance_temp_cur;

    s16 power_line_freq_min;
    s16 power_line_freq_max;
    s16 power_line_freq_def;
    s16 power_line_freq_res;
    s16 power_line_freq_cur;
};
struct usb_host_parm {
    u32 hspeed;
    u32 ot;
};


//video ch1和hid互斥，打开hid会失能video ch1
#ifndef     USB_MSD_EN
#define     USB_MSD_EN           1
#define     USB_MIC_EN           1
#define     USB_SPEAKER_EN       1
#define     USB_HID_EN           1
#define     USB_VIDEO_EN         1
#define     USB_VIDEO_PRIVATE_EN 1
#define     USB_CDC_EN           1

// #if USB_HID_EN && USB_VIDEO_PRIVATE_EN
// #error USB_HID_EN and USB_VIDEO_PRIVATE_EN conflicts
// #endif

// #if USB_CDC_EN && USB_VIDEO_PRIVATE_EN
// #error USB_CDC_EN and USB_VIDEO_PRIVATE_EN conflicts
// #endif
#endif

#define MASSSTORAGE_CLASS   (BIT(0))
#define SPEAKER_CLASS       (BIT(1))
#define MIC_CLASS           (BIT(2))
#define AUDIO_CLASS         ((BIT(1)|BIT(2)))
#define HID_CLASS           (BIT(3))
#define VIDEO_CLASS         (BIT(4))
#define CDC_CLASS           (BIT(5))

typedef struct _usb_slv_str_desc {
    u32 vid_pid;
    const u8 *manufacture;
    const u8 *product_desc;
    const u8 *chipserial;
    const u8 *msd_desc;
    const u8 *spk_desc;
    const u8 *mic_desc;
    const u8 *video_desc;
} USB_SLV_STR_DESC;

struct usb_slv_arg {
    u16 ot;
    const USB_SLV_STR_DESC *pstr_desc;
    void *parm;

};
struct uvc_frame_info {
    u16 width;
    u16 height;
};
typedef struct camera_info {
    struct uvc_frame_info frame[5];
} CAMERA_INFO;


typedef enum _usb_audio_ctl_type {
    USB_MIC_VOLUME = 0x10,
    USB_MIC_LEFT_VOLUME,
    USB_MIC_RIGHT_VOLUME,
    USB_MIC_MUTE,

    USB_SPK_VOLUME = 0x20,
    USB_SPK_LEFT_VOLUME,
    USB_SPK_RIGHT_VOLUME,
    USB_SPK_MUTE,
} USB_AUDIO_CTL_TYPE;

/****uvc从机事件****/
enum uvc_event {
    UVC_BUTTON_PRESSED = 0,
    UVC_BUTTON_RELEASED,
    UVC_STREAM_ERROR,
    UVC_CONTROL_CHANGE,
};

typedef u16(* USB_MIC_GET_DATA)(void *ptr, u16 len);
typedef void(* USB_SPK_WRITE_DATA)(void *ptr, u16 len);
typedef u32(* USB_AUDIO_VOLUME_CTL)(USB_AUDIO_CTL_TYPE type, u32 volume); //

typedef void (*OPEN_CAMERA)(u32 frame_index);
typedef void (*CLOSE_CAMERA)();
typedef void *(*CONFIG_CMAERA)(u32 cmd, void *parm);
typedef u32(*USB_VIDEO_INDATA)(void *ptr, u32 len, u32 *is_frame_end);

struct jl_msg_ops {
    int (*read)(void *dev, void *buf, u32 len);
    int (*send)(void *dev, const void *buf, u32 len);
};

struct scsi_private_request {
    struct jl_msg_ops ops;
    void *dev;
    u8 *cmd;
    u32 cmd_len;
};

struct uvc_unit_ctrl {
    u8  request;
    u8  unit;
    u32 len;
    u8 *data;
};

struct usb_mass_storage {
    int (*dev_attribute)(int id);
    int (*dev_read)(int id, u8 *buf, u32 addr, int len);
    int (*dev_write)(int id, u8 *buf, u32 addr, int len);
    int (*dev_ask_block_num)(int id);
    int (*dev_ask_block_size)(int id);
    int (*dev_ask_lun_num)(void);
    int (*dev_state_check)(int id);
    int (*scsi_private)(struct scsi_private_request *req);
};

#define FMT_MAX_RESO_NUM    15

struct uvc_format {
    u8 format;
    u8 active_num;
    u8 split_cam_enable; //2个独立的uvc设备
    u16 width[2][FMT_MAX_RESO_NUM * 2];
    u16 height[2][FMT_MAX_RESO_NUM * 2];
    u16 fps[2][FMT_MAX_RESO_NUM * 2];
    int (*video_open)(int id, int fmt, int frame_id, int fps);
    int (*video_reqbuf)(int id, void *buf, u32 len, u32 *frame_end);
    int (*video_close)(int id);
    int (*processing_unit_response)(struct uvc_unit_ctrl *ctl_req);
    int (*extension_unit_response)(struct uvc_unit_ctrl *ctl_req);
    int (*private_cmd_response)(u16 cmd, u16 data);

    s16(*get_isp_brightness)();
    s16(*get_isp_contrast)();
    s16(*get_isp_saturation)();
    s16(*get_isp_sharpness)();
    s16(*get_isp_gamma)();
    s16(*get_isp_gain)();
    u8(*get_isp_powerlinefrequency)();

    int (*set_isp_brightness)(s16 val);
    int (*set_isp_contrast)(s16 val);
    int (*set_isp_saturation)(s16 val);
    int (*set_isp_sharpness)(s16 val);
    int (*set_isp_gamma)(s16 val);
    int (*set_isp_gain)(s16 val);
    int (*set_isp_powerlinefrequency)(u8 val);
};


struct uac_micreso {
    u16 channel;
    u16 bitwidth;
    u32 samplerate;
    u32 max_packet_size;
};

struct uac_micreso_info {
    int num;
    struct uac_micreso reso[32];
};

struct uac_micformat {
    struct uac_micreso_info *info;
    int (*audio_open)(int fmt, int channel, int bitwidth, int samplerate);
    int (*audio_reqbuf)(void *buf, int len);
    int (*audio_close)();
};
struct uac_spkformat {
    struct uac_micreso_info *info;
    int (*audio_open)(int fmt, int volume, int channel, int bitwidth, int samplerate);
    int (*audio_putbuf)(void *buf, int len);
    int (*audio_close)();
};


struct usb_uvc_camera {
    u8 bind;
    u8 format;
    u8 interface_num;
    u8 split_cam_enable;
    u16 width[2][FMT_MAX_RESO_NUM * 2];
    u16 height[2][FMT_MAX_RESO_NUM * 2];
    u16 fps[2][FMT_MAX_RESO_NUM * 2];
    int (*video_open)(int id, int fmt, int frame_id, int fps);
    int (*video_reqbuf)(int id, void *buf, u32 len, u32 *frame_end);
    int (*video_close)(int id);
    int (*processing_unit_response)(struct uvc_unit_ctrl *ctl_req);
    int (*extension_unit_response)(struct uvc_unit_ctrl *ctl_req);
    int (*private_cmd_response)(u16 cmd, u16 data);

    s16(*get_isp_brightness)();
    s16(*get_isp_contrast)();
    s16(*get_isp_saturation)();
    s16(*get_isp_sharpness)();
    s16(*get_isp_gamma)();
    s16(*get_isp_gain)();
    u8(*get_isp_powerlinefrequency)();

    int (*set_isp_brightness)(s16 val);
    int (*set_isp_contrast)(s16 val);
    int (*set_isp_saturation)(s16 val);
    int (*set_isp_sharpness)(s16 val);
    int (*set_isp_gamma)(s16 val);
    int (*set_isp_gain)(s16 val);
    int (*set_isp_powerlinefrequency)(u8 val);
};

struct usb_uac_spk {
    u8 bind;
    u8 format;
    u32 samplerate;
    u16 mute;
    u16 volume[5];
    struct  usb_uac_spk *mic_control;
    struct uac_micreso_info info;
    int (*audio_open)(int fmt, int volume, int channel, int bitwidth, int samplerate);
    int (*audio_putbuf)(void *buf, int len);
    int (*audio_close)();
};
struct usb_uac_microphone {
    u8 bind;
    u8 format;
    u8 mute;
    u16 volume[5];
    u32 samplerate;
    u16 interval_to_fill_point;
    u16 cur_points;
    struct  usb_uac_spk *mic_control;
    struct uac_micreso_info info;
    int (*audio_open)(int fmt, int change_fmt, int bitwidth, int samplerate);
    int (*audio_reqbuf)(void *buf, int len);
    int (*audio_close)();
};


typedef struct _usb_audio_arg {
    u32 uac_magic;
    USB_SPK_WRITE_DATA usb_spk_write_data;
    USB_MIC_GET_DATA usb_mic_get_data;
    USB_AUDIO_VOLUME_CTL usb_audio_volume_ctl;
} USB_AUDIO_ARG;


#define HID_FMT_NONE            0x00
#define HID_FMT_KEYBOARD        0x01
#define HID_FMT_USER            0x02

#define HID_FMT_KB_PKG          0x80
#define HID_FMT_KB_SPEC         0x81
#define HID_FMT_KB_HOLD         0x82
#define HID_FMT_KB_HEND         0x83

struct usb_hid_arg {
    int subtype;
    void *private_data;
    u32(*get_one_frame)(void *, u8 *buf, u32 len);
    int (*output)(u8 *buf, u32 len);
    u8 cust_rp_desc[128];
    u32 desc_len;
    int timer;
    int cfg_done;
    int trig;
};

struct usb_cdc_arg {
    void *priv;
    int (*output)(u8 *obuf, u32 olen);
    u32(*read_buffer)(void *, u8 *buf, u32 len);
    u32(*intr_read_buffer)(void *, u8 *buf, u32 len);
    int timer;
    u8 bmTransceiver;
    u8 trig;
    u8 trig_intr;
    u32 last_len;
    u8 subtype_data[64];
};

/*
 * uvc host
 */
struct uvc_reqbufs {
    void *buf;
    int size;
};

struct uvc_capability {
    int fmt;
    int fps;
    int reso_num;
    struct uvc_frame_info reso[8];
};

struct uvc_processing_unit {
    u8 request;
    u8 type;
    u16 value;
    u16 index;
    u8 buf[4];
    int len;
};

struct uvc_event_listener {
    void *priv;
    int (*handler)(void *priv, enum uvc_event event);
};

struct usb_device_id {
    u16 vendor;
    u16 product;
};

struct usb_string {
    u8 code_type;
    u16 language;
    int len;
    u8 *string;
};

struct usb_host_user_ops {
    s32(*mount)(void *hdl, int usb_id);
    s32(*unmount)(void *hdl, int usb_id);
    void (*isr)(void *hdl, u32 intrrx, u32 intrtx);
};

#define UVC_CAMERA_FMT_YUY2     (1 << 0)
#define UVC_CAMERA_FMT_MJPG     (1 << 1)
#define UVC_CAMERA_FMT_H264     (1 << 2)

#define USBIOC_MASS_STORAGE_CONNECT     _IOW('U', 0, sizeof(struct usb_mass_storage))
#define USBIOC_UVC_CAMERA_CONNECT       _IOW('U', 1, sizeof(struct uvc_format))
#define USBIOC_UAC_MICROPHONE_CONNECT   _IOW('U', 2, sizeof(struct uvc_format))
//#define USBIOC_UVC_CAMERA1_CONNECT      _IOW('U', 2, sizeof(struct uvc_format))
#define USBIOC_SLAVE_MODE_START         _IOW('U', 3, sizeof(unsigned int))
#define USBIOC_SLAVE_DISCONNECT         _IOW('U', 4, sizeof(unsigned int))
#define UVCIOC_QUERYCAP                 _IOR('U', 5, sizeof(struct uvc_capability))
#define UVCIOC_SET_CAP_SIZE             _IOW('U', 5, sizeof(unsigned int))
#define UVCIOC_SET_FPS                  _IOW('U', 6, sizeof(unsigned int))
#define UVCIOC_STREAM_ON                _IOW('U', 7, sizeof(unsigned int))
#define UVCIOC_STREAM_OFF               _IOW('U', 8, sizeof(unsigned int))
#define UVCIOC_REQBUFS                  _IOW('U', 9, sizeof(unsigned int))
#define UVCIOC_DQBUF                    _IOW('U', 10, sizeof(unsigned int))
#define UVCIOC_QBUF                     _IOW('U', 11, sizeof(unsigned int))
#define UVCIOC_RESET                    _IOW('U', 12, sizeof(unsigned int))
#define UVCIOC_REQ_PROCESSING_UNIT      _IOR('U', 13, sizeof(struct uvc_processing_unit))
#define UVCIOC_SET_PROCESSING_UNIT      _IOW('U', 14, sizeof(struct uvc_processing_unit))
#define UVCIOC_GET_DEVICE_ID            _IOR('U', 15, sizeof(struct usb_device_id))

#define USBIOC_SLAVE_RESET              _IOW('U', 16, sizeof(unsigned int))

/***********2018-06-21************/
#define USBIOC_GET_DEVICE_ID            _IOR('U', 17, sizeof(struct usb_device_id))
#define USBIOC_GET_MANUFACTURER         _IOR('U', 18, sizeof(struct usb_string))
#define USBIOC_GET_PRODUCT_NAME         _IOR('U', 19, sizeof(struct usb_string))
/*********************************/
/***********2018-07-30************/
#define UVCIOC_SET_EVENT_LISTENER       _IOW('U', 19, sizeof(struct uvc_event_listener))
/*********************************/

#define USBIOC_HID_CONNECT              _IOW('U', 20, sizeof(struct usb_hid_arg))
#define USBIOC_HID_CONTROL              _IOW('U', 21, sizeof(struct usb_hid_arg))
#define USBIOC_CDC_CONNECT              _IOW('U', 22, sizeof(struct usb_cdc_arg))
#define USBIOC_CDC_CONTROL              _IOW('U', 23, sizeof(unsigned int))
#define USBIOC_UAC_SPEAKER_CONNECT      _IOW('U', 24, sizeof(struct _usb_audio_arg))
#define USBIOC_GET_USB_PORT             _IOR('U', 25, sizeof(unsigned int))

#define     USB_SLAVE_RUN            1002
#define     USB_DISABLECONNECT       1003
#define     USB_ENABLECONNECT        1004
#define     USB_FORCE_RESET          1005

#define     IOCTL_GET_MAX_LUN        1006
#define     IOCTL_GET_CUR_LUN        1007
#define     IOCTL_SET_CUR_LUN        1008

struct usb_platform_data {
    u32 id;
    u32 ctl_irq_int;
    u32 online_check_cnt;
    u32 offline_check_cnt;
    u32 isr_priority;
    u32 host_ot;
    u32 host_speed;
    u32 slave_ot;
};
#define USB_PLATFORM_DATA_BEGIN(data) \
    static const struct usb_platform_data data = {\


#define USB_PLATFORM_DATA_END()  \
};

struct uvc_platform_data {
    u16 width;
    u16 height;
    int fps;
    int fmt;
    int mem_size;
    int timeout;
    u8  put_msg;
};

#define UVC_PLATFORM_DATA_BEGIN(data) \
    static const struct uvc_platform_data data = {\

#define UVC_PLATFORM_DATA_END()  \
};

struct usb_cdc_platform_data {
    u32 baud_rate;     /*Data terminal rate, in bits per second*/
    u8  stop_bits;     /*Stop bits*/
    u8  parity;        /* Parity */
    u8  data_bits;     /* Data bits (5, 6, 7, 8 or 16). */
};

#define USB_CDC_PLATFORM_DATA_BEGIN(data) \
    static const struct usb_cdc_platform_data data = {\

#define USB_CDC_PLATFORM_DATA_END()  \
};

extern const struct device_operations usb_dev_ops;
extern const struct device_operations mass_storage_ops;
extern const struct device_operations uvc_host_ops;
extern const struct device_operations uvc_dev_ops;
extern const struct device_operations usb_cdc_ops;

/*****************USB WIFI**********************/
#include "os/os_compat.h"
#include "device/device.h"
#define USB_WIFI_GET_NUM	_IOR('W',0,sizeof(int))
#define USB_WIFI_GET_FUNC	_IOR('W',1,sizeof(int))
#define USB_LTE_AT_SEND	    _IOR('W',2,sizeof(int))
#define USB_LTE_AT_RECV   	_IOR('W',3,sizeof(int))

typedef struct usb_wifi_fun {
    /*usb_wifi_dev lib function*/
    void *(*usb_open)(void *p);
    int (*usb_close)(void);
    int (*usb_online)(char *name);
    int (*usb_ioctl)(struct device *device, u32 cmd, u32 arg);

    /*wifi_todo function*/
    int (*usb_bulk_only_receive_int)(u8 ep, u8 *pbuf, u32 len);
    int (*usb_bulk_only_send)(u8 ep, u8 *pbuf, u32 len);
    int (*usb_ctl_msg)(u8 Request, u8 RequestType, u16 Value, u16 Index, void *Date, u16 Length);
} USB_WIFI_FUN;

typedef void(* USB_WIFI_RX_COMPLETE_CALLBACK)(u32);
extern void set_usb_wifi_rx_complete_callback(USB_WIFI_RX_COMPLETE_CALLBACK func, char usb_index);
extern char *usb_wifi_get_name(void);
extern const struct device_operations usbwifi_dev_ops;

typedef struct usbwifi_platform_data {
    char usb_index;
    char *name;
    void *prive;
    struct usb_device *usb_dev;
    struct device dev;

} HOST_USB_WIFI;

/*************END OF USB WIFI***************/
#ifdef __cplusplus
}
#endif

#endif
