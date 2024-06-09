#ifndef VIDEO_SERVER_H
#define VIDEO_SERVER_H

#include "server/server_core.h"
#include "server/vpkg_server.h"
#include "system/includes.h"


#define VIDEO_SERVER_PKG_ERR       0x01
#define VIDEO_SERVER_UVM_ERR       0x02
#define VIDEO_SERVER_PKG_END       0x03
#define VIDEO_SERVER_NET_ERR       0x04


enum video_state {
    VIDEO_STATE_START,
    VIDEO_STATE_PAUSE,
    VIDEO_STATE_RESUME,
    VIDEO_STATE_STOP,
    VIDEO_STATE_SAVE_FILE,
    VIDEO_STATE_SET_OSD,
    VIDEO_STATE_SET_VOICE,
    VIDEO_STATE_GET_INFO,
    VIDEO_STATE_PKG_LEN,
    VIDEO_STATE_PKG_MUTE,
    VIDEO_STATE_SET_OSD_STR,
    VIDEO_STATE_CFG,
    VIDEO_STATE_CFG_ROI,
    VIDEO_STATE_GET_PKG_TIME,
    VIDEO_STATE_SET_NEXT_IFRAME,
    VIDEO_STATE_STOP_COUNT_DOWN,
    VIDEO_STATE_SET_DR,
    VIDEO_STATE_CAP_IFRAME,
    VIDEO_STATE_RESET_BITS_RATE,
    VIDEO_STATE_PKG_PAUSE_RUN,
    VIDEO_STATE_V4_PAUSE_RUN,
    VIDEO_STATE_SET_ENC_CROP,
    VIDEO_STATE_SET_DIS_CROP,
    VIDEO_STATE_SET_AUDIO_CALLBACK,
    VIDEO_STATE_SET_DIS_PAUSE,
};

enum video_rec_format {
    VIDEO_FMT_AVI,
    VIDEO_FMT_MOV,
    VIDEO_FMT_YUYV,
    VIDEO_FMT_NV12,
    VIDEO_FMT_TS,
};


struct out_inf {
    char *path;
    void *arg;
    void *(*open)(const char *path, void *arg);
    int (*send)(void *, char *, unsigned int, unsigned int);
    void (*close)(void *);
};


struct vs_audio {
    u8 channel;
    u8 volume;
    u16 kbps;
    u16 sample_rate;
    u16 aud_interval_size;
    u8 *buf;
    int buf_len;
    const char *fname;
    u8 share; //用于公用音频通道,当编码aac可以开这参数
    u8 type;
    enum  audio_fmt_format fmt_format;
    u8 *sample_source;
};
struct net_rec_par {
    char netpath[64];
};
struct vs_buf_dev {
    const char *name;
    u8  enable;
    u32 addr;
    u32 size;
};
struct change_qp {
    u8 static_min;
    u8 static_max;
    u8 dynamic_min;
    u8 dynamic_max;
    u8 enable_change_qp;
};


struct vs_video_rec {
    u8  state;
    u8  format;
    u8  channel;
    u8 real_fps;
    u8 fps;                         //需要录的帧率
    u8 slow_motion;                 //慢动作倍数(与延时摄影互斥,无音频)
    u8 camera_type;
    u8 three_way_type;
    u8 uvc_id;
    u8 usb_cam;
    u8 target;
    struct out_inf out;
    u8 rec_small_pic;
    char *rec_small_pic_path;
    u16 small_pic_width;
    u16 small_pic_height;
    u8 manu_enc;
    u16 width;
    u16 height;
    u8 *buf;
    char *new_osd_str;
    u32 cycle_time;
    u32 count_down;
    u32 buf_len;
    u32 tlp_time;                   //延时录像的间隔ms(与慢动作互斥,无音频)
    u32 pkg_fps;					//封装帧率
    u32 gap_fps;					//间隔帧数(延时录像方式二：间隔编码，仅支持四路)
    u32 abr_kbps;
    u32 IP_interval;                //max IP interval
    u32 delay_limit;
    struct roi_cfg roi;
    u32 fsize;
    FILE *file;
    const char *usb_cam_name;
    enum video_rec_quality quality;//(图片质量(高中低))
    struct vs_audio audio;
    struct video_text_osd *text_osd;
    struct video_graph_osd *graph_osd;
    struct vpkg_get_info get_info;
    struct vpkg_aud_mute pkg_mute;
    struct app_enc_info *app_avc_cfg;
    int (*camera_config)(u32 lv, void *arg);
    struct video_source_crop *crop;
    u32 mirror;
    u32 jaggy;
    u32 rotate;
    int (*callback)(u8 *buf, u32 len);
    struct net_rec_par net_par;
    int (*audio_callback)(u8 *buf, u32 len);
    u16 src_w;
    u16 src_h;
    struct change_qp qp_attr;
};


struct vs_video_display {
    u16 left;
    u16 top;
    u16 width;
    u16 height;
    u16 border_left;
    u16 border_top;
    u16 border_right;
    u16 border_bottom;
    u8  camera_type;
    u8 three_way_type;
    u8  uvc_id;
    const char *fb;
    struct imc_presca_ctl *pctl;
    enum video_state state;
    int (*camera_config)(u32 lv, void *arg);
    struct video_source_crop *crop;
    u32 mirror;
    u32 jaggy;
    u32 rotate;
    u16 src_w;
    u16 src_h;
};

struct vs_image_capture {
    u16 width;
    u16 height;
    u8 camera_type;
    u8 uvc_id;
    u8 target;
    u8 type;
    u32 quality;
    u8  *buf;
    u32 buf_size;
    struct icap_auxiliary_mem *aux_mem;
    struct jpg_thumbnail *thumbnails;
    struct video_text_osd *text_label;
    struct video_graph_osd *graph_label;
    struct image_sticker *sticker;
    const char *path;
    struct jpg_q_table *qt;
    struct video_source_crop *crop;

    u8 *exif;
    int exif_size;
    u32 mirror;
    char *file_name;
    char save_cap_buf;
    u32 jaggy;
    u8 linear_scale;		// 线性插值标志
    u16 src_w;
    u16 src_h;
};

#define  SET_CAMERA_MODE        BIT(0)
#define  SET_CAMERA_EV          BIT(1)
#define  SET_CAMERA_WB          BIT(2)
#define  SET_CAMERA_SHP_LEVEL   BIT(3)
#define  SET_CAMERA_DRC_ENABLE  BIT(4)
#define  GET_CAMERA_LV          BIT(5)
#define  GET_CAMERA_INFO        BIT(6)
#define  SET_CUSTOMIZE_CFG      BIT(7)
#define  GET_CUSTOMIZE_CFG      BIT(8)

struct vs_camera_effect {
    u8 mode;
    s8 ev;
    u8 white_blance;
    u8 shpn_level;
    u8 drc;
    u32 cmd;
    int lv;
    void *customize_cfg;
};

struct vs_camera_sca {
    u8 step;
    u8 max_sca;// max sca ratio
    u8 sca_modify;//1 big, 0 small
};

struct vs_image_info {
    u8 *buf;
    u32 size;
};

#define VIDEO_TO_USB    0x10
#define VIDEO_TO_OUT    0x20
#define VIDEO_USE_STD   0x01  //是否使用标准头
/*
 * video_server支持的请求命令列表，每个请求命令对应union video_req中的一个结构体
 */
enum {
    VIDEO_REQ_REC,
    VIDEO_REQ_DISPLAY,
    VIDEO_REQ_IMAGE_CAPTURE,
    VIDEO_REQ_CAMERA_EFFECT,
    VIDEO_REQ_CAMERA_SCA,
    VIDEO_REQ_GET_IMAGE,
    VIDEO_REQ_GET_YUV,
    VIDEO_REQ_CLOSE_CAMERA,
};

union video_req {
    struct vs_video_rec rec;
    struct vs_image_capture icap;
    struct vs_video_display display;
    struct vs_camera_effect camera;
    struct vs_camera_sca sca;
    struct vs_image_info image;
};































#endif

