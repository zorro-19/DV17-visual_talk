#ifndef __VIDEO_PHOTO_H_
#define __VIDEO_PHOTO_H_

#include "system/includes.h"
#include "server/video_server.h"
#include "app_config.h"


//#define CAMERA_CAP_BUF_SIZE        (1*1024*1024)
// #define CAMERA_CAP_BUF_SIZE         (2 * 1024 * 1024)
#define     LOW_QUALITY      (1 * 1024 * 1024 + 512 * 1024)
#define     MED_QUALITY      (1 * 1024 * 1024 + 768 * 1024)
#define     HIG_QUALITY      (9 * 1024 * 1024 )
#define     IMAGE_AUX_BUF_SIZE        HIG_QUALITY //LOW_QUALITY
#define     IMAGE_THUMB_BUF_SIZE      (64 * 1024)

#define PHOTO_CAMERA_NUM        4

#define JPG_THUMBNAILS_ENABLE

struct photo_camera {
    u8 id;
    u8 state;
    u16 prev_width;
    u16 prev_height;
    int width;
    int height;
    struct server *server;
    u16 src_w;
    u16 src_h;
};

struct video_photo_handle {
    u8 state;
    u8 camera_id;//
    u8 uvc_id;
    int timeout;
    int qscan_timeout;
    int delay_ms;
    int timer;
    u8 *cap_buf;
    u8 *aux_buf;
    u8 *thumbnails_buf;
    struct photo_camera camera[PHOTO_CAMERA_NUM];
    struct server *ui;
    struct server *display;
    struct server *video_dec;
    struct video_text_osd label;
    char file_str[8];
    u16 sd_wait;
};

const static u8 label_font_32x64[] ALIGNE(64) = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x1F, 0xF8, 0x00,
    0x00, 0x3C, 0x3E, 0x00, 0x00, 0x78, 0x0F, 0x00, 0x00, 0xF0, 0x07, 0x00, 0x01, 0xE0, 0x07, 0x80,
    0x03, 0xE0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x07, 0xC0, 0x03, 0xE0, 0x07, 0xC0, 0x01, 0xE0,
    0x07, 0x80, 0x01, 0xE0, 0x0F, 0x80, 0x01, 0xF0, 0x0F, 0x80, 0x01, 0xF0, 0x0F, 0x80, 0x01, 0xF0,
    0x0F, 0x80, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0xF8,
    0x1F, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0xF8,
    0x1F, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0xF8,
    0x1F, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0xF8,
    0x0F, 0x80, 0x00, 0xF0, 0x0F, 0x80, 0x01, 0xF0, 0x0F, 0x80, 0x01, 0xF0, 0x0F, 0x80, 0x01, 0xF0,
    0x07, 0x80, 0x01, 0xE0, 0x07, 0xC0, 0x01, 0xE0, 0x07, 0xC0, 0x03, 0xE0, 0x03, 0xC0, 0x03, 0xC0,
    0x03, 0xE0, 0x03, 0xC0, 0x01, 0xE0, 0x07, 0x80, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0x78, 0x0E, 0x00,
    0x00, 0x3C, 0x3C, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"0",0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0xC0, 0x00,
    0x00, 0x01, 0xC0, 0x00, 0x00, 0x07, 0xC0, 0x00, 0x01, 0xFF, 0xC0, 0x00, 0x01, 0xFF, 0xC0, 0x00,
    0x00, 0x07, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00,
    0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00,
    0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00,
    0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00,
    0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00,
    0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00,
    0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00,
    0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00,
    0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x0F, 0xF0, 0x00,
    0x01, 0xFF, 0xFF, 0x80, 0x01, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"1",1
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x3F, 0xFC, 0x00,
    0x00, 0xF0, 0x3F, 0x00, 0x01, 0xC0, 0x1F, 0x80, 0x03, 0x80, 0x0F, 0x80, 0x07, 0x80, 0x07, 0xC0,
    0x07, 0x00, 0x07, 0xC0, 0x0F, 0x00, 0x03, 0xE0, 0x0F, 0x00, 0x03, 0xE0, 0x0F, 0x80, 0x03, 0xE0,
    0x0F, 0x80, 0x03, 0xE0, 0x0F, 0xC0, 0x03, 0xE0, 0x0F, 0xC0, 0x03, 0xE0, 0x0F, 0xC0, 0x03, 0xE0,
    0x07, 0x80, 0x03, 0xC0, 0x00, 0x00, 0x07, 0xC0, 0x00, 0x00, 0x07, 0xC0, 0x00, 0x00, 0x07, 0x80,
    0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x3C, 0x00,
    0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x01, 0xC0, 0x00,
    0x00, 0x01, 0xC0, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00,
    0x00, 0x1C, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x30,
    0x01, 0xC0, 0x00, 0x30, 0x03, 0x80, 0x00, 0x30, 0x03, 0x00, 0x00, 0x30, 0x07, 0x00, 0x00, 0x60,
    0x0E, 0x00, 0x00, 0xE0, 0x0C, 0x00, 0x01, 0xE0, 0x1F, 0xFF, 0xFF, 0xE0, 0x1F, 0xFF, 0xFF, 0xE0,
    0x1F, 0xFF, 0xFF, 0xE0, 0x1F, 0xFF, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"2",2
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x7F, 0xF8, 0x00,
    0x00, 0xF0, 0x7C, 0x00, 0x01, 0xC0, 0x3E, 0x00, 0x03, 0x80, 0x1F, 0x00, 0x03, 0x80, 0x0F, 0x80,
    0x07, 0x80, 0x0F, 0x80, 0x07, 0x80, 0x07, 0xC0, 0x07, 0xC0, 0x07, 0xC0, 0x07, 0xC0, 0x07, 0xC0,
    0x07, 0xC0, 0x07, 0xC0, 0x03, 0x80, 0x07, 0xC0, 0x00, 0x00, 0x07, 0xC0, 0x00, 0x00, 0x07, 0xC0,
    0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x1E, 0x00,
    0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x0F, 0xF8, 0x00,
    0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x07, 0xC0,
    0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xE0, 0x00, 0x00, 0x01, 0xE0, 0x00, 0x00, 0x01, 0xF0,
    0x00, 0x00, 0x01, 0xF0, 0x00, 0x00, 0x01, 0xF0, 0x00, 0x00, 0x01, 0xF0, 0x03, 0x80, 0x01, 0xF0,
    0x07, 0xC0, 0x01, 0xF0, 0x0F, 0xC0, 0x01, 0xF0, 0x0F, 0xC0, 0x01, 0xE0, 0x0F, 0xC0, 0x03, 0xE0,
    0x0F, 0x80, 0x03, 0xE0, 0x07, 0x80, 0x07, 0xC0, 0x07, 0x80, 0x07, 0x80, 0x03, 0xC0, 0x0F, 0x00,
    0x01, 0xF0, 0x3E, 0x00, 0x00, 0x7F, 0xFC, 0x00, 0x00, 0x1F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"3",3
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00,
    0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x3E, 0x00,
    0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0xDE, 0x00, 0x00, 0x00, 0xDE, 0x00, 0x00, 0x01, 0x9E, 0x00,
    0x00, 0x03, 0x9E, 0x00, 0x00, 0x03, 0x1E, 0x00, 0x00, 0x06, 0x1E, 0x00, 0x00, 0x0E, 0x1E, 0x00,
    0x00, 0x0C, 0x1E, 0x00, 0x00, 0x18, 0x1E, 0x00, 0x00, 0x18, 0x1E, 0x00, 0x00, 0x30, 0x1E, 0x00,
    0x00, 0x60, 0x1E, 0x00, 0x00, 0x60, 0x1E, 0x00, 0x00, 0xC0, 0x1E, 0x00, 0x01, 0xC0, 0x1E, 0x00,
    0x01, 0x80, 0x1E, 0x00, 0x03, 0x00, 0x1E, 0x00, 0x03, 0x00, 0x1E, 0x00, 0x06, 0x00, 0x1E, 0x00,
    0x0C, 0x00, 0x1E, 0x00, 0x0C, 0x00, 0x1E, 0x00, 0x18, 0x00, 0x1E, 0x00, 0x3F, 0xFF, 0xFF, 0xFC,
    0x3F, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x1E, 0x00,
    0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x1E, 0x00,
    0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x3F, 0x00,
    0x00, 0x0F, 0xFF, 0xF8, 0x00, 0x0F, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"4",4
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xE0,
    0x01, 0xFF, 0xFF, 0xE0, 0x01, 0xFF, 0xFF, 0xE0, 0x01, 0xFF, 0xFF, 0xC0, 0x01, 0x80, 0x00, 0x00,
    0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00,
    0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x03, 0xF8, 0x00,
    0x03, 0x1F, 0xFE, 0x00, 0x03, 0x3F, 0xFF, 0x00, 0x03, 0x78, 0x1F, 0x80, 0x03, 0xE0, 0x0F, 0xC0,
    0x03, 0xC0, 0x07, 0xC0, 0x03, 0x80, 0x03, 0xE0, 0x03, 0x00, 0x03, 0xE0, 0x00, 0x00, 0x03, 0xE0,
    0x00, 0x00, 0x01, 0xF0, 0x00, 0x00, 0x01, 0xF0, 0x00, 0x00, 0x01, 0xF0, 0x00, 0x00, 0x01, 0xF0,
    0x00, 0x00, 0x01, 0xF0, 0x00, 0x00, 0x01, 0xF0, 0x03, 0x80, 0x01, 0xF0, 0x07, 0xC0, 0x01, 0xF0,
    0x0F, 0xC0, 0x01, 0xF0, 0x0F, 0xC0, 0x01, 0xE0, 0x0F, 0xC0, 0x03, 0xE0, 0x0F, 0x80, 0x03, 0xE0,
    0x0F, 0x80, 0x03, 0xC0, 0x07, 0x80, 0x07, 0xC0, 0x03, 0x80, 0x07, 0x80, 0x01, 0xC0, 0x0F, 0x00,
    0x00, 0xF0, 0x3E, 0x00, 0x00, 0x7F, 0xFC, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"5",5
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFC, 0x00, 0x00, 0x07, 0xFF, 0x00,
    0x00, 0x1F, 0x07, 0x80, 0x00, 0x3C, 0x03, 0xC0, 0x00, 0x70, 0x03, 0xE0, 0x00, 0xE0, 0x03, 0xE0,
    0x01, 0xE0, 0x03, 0xE0, 0x01, 0xC0, 0x03, 0xE0, 0x03, 0xC0, 0x01, 0xC0, 0x03, 0x80, 0x00, 0x00,
    0x07, 0x80, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00,
    0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x01, 0xF8, 0x00, 0x1F, 0x0F, 0xFF, 0x00,
    0x1F, 0x1F, 0xFF, 0x80, 0x1F, 0x3E, 0x0F, 0xC0, 0x1F, 0x78, 0x03, 0xE0, 0x1F, 0x60, 0x01, 0xE0,
    0x1F, 0xC0, 0x01, 0xF0, 0x1F, 0xC0, 0x00, 0xF0, 0x1F, 0x80, 0x00, 0xF0, 0x1F, 0x80, 0x00, 0x78,
    0x1F, 0x00, 0x00, 0x78, 0x1F, 0x00, 0x00, 0x78, 0x1F, 0x00, 0x00, 0x78, 0x1F, 0x00, 0x00, 0x78,
    0x1F, 0x00, 0x00, 0x78, 0x0F, 0x00, 0x00, 0x78, 0x0F, 0x00, 0x00, 0x78, 0x0F, 0x80, 0x00, 0x78,
    0x0F, 0x80, 0x00, 0x78, 0x07, 0x80, 0x00, 0x70, 0x07, 0xC0, 0x00, 0xF0, 0x07, 0xC0, 0x00, 0xF0,
    0x03, 0xE0, 0x00, 0xE0, 0x03, 0xE0, 0x01, 0xE0, 0x01, 0xF0, 0x01, 0xC0, 0x00, 0xF8, 0x03, 0x80,
    0x00, 0x7E, 0x0F, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x00, 0x07, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"6",6
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0xF0,
    0x07, 0xFF, 0xFF, 0xF0, 0x07, 0xFF, 0xFF, 0xF0, 0x07, 0xFF, 0xFF, 0xE0, 0x07, 0x80, 0x00, 0xC0,
    0x07, 0x00, 0x00, 0xC0, 0x06, 0x00, 0x01, 0x80, 0x06, 0x00, 0x01, 0x80, 0x0C, 0x00, 0x03, 0x00,
    0x0C, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0C, 0x00,
    0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x38, 0x00,
    0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xE0, 0x00,
    0x00, 0x00, 0xE0, 0x00, 0x00, 0x01, 0xE0, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00,
    0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x07, 0x80, 0x00,
    0x00, 0x07, 0x80, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x0F, 0x80, 0x00,
    0x00, 0x0F, 0x80, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x1F, 0x80, 0x00, 0x00, 0x1F, 0x80, 0x00,
    0x00, 0x1F, 0x80, 0x00, 0x00, 0x1F, 0x80, 0x00, 0x00, 0x1F, 0x80, 0x00, 0x00, 0x1F, 0x80, 0x00,
    0x00, 0x1F, 0x80, 0x00, 0x00, 0x1F, 0x80, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"7",7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x7F, 0xFE, 0x00,
    0x00, 0xF8, 0x1F, 0x00, 0x01, 0xE0, 0x07, 0x80, 0x03, 0xC0, 0x03, 0xC0, 0x07, 0x80, 0x01, 0xE0,
    0x07, 0x80, 0x01, 0xE0, 0x0F, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0xF0,
    0x0F, 0x00, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0xF0, 0x0F, 0x80, 0x00, 0xF0, 0x0F, 0x80, 0x00, 0xF0,
    0x07, 0xC0, 0x01, 0xE0, 0x07, 0xE0, 0x01, 0xE0, 0x03, 0xF0, 0x01, 0xC0, 0x01, 0xFC, 0x03, 0x80,
    0x00, 0xFE, 0x07, 0x00, 0x00, 0x7F, 0xCE, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x1F, 0xF8, 0x00,
    0x00, 0x73, 0xFE, 0x00, 0x00, 0xE0, 0xFF, 0x00, 0x01, 0xC0, 0x3F, 0x80, 0x03, 0xC0, 0x1F, 0xC0,
    0x07, 0x80, 0x07, 0xE0, 0x07, 0x00, 0x03, 0xE0, 0x0F, 0x00, 0x01, 0xF0, 0x0F, 0x00, 0x01, 0xF0,
    0x1E, 0x00, 0x00, 0xF8, 0x1E, 0x00, 0x00, 0x78, 0x1E, 0x00, 0x00, 0x78, 0x1E, 0x00, 0x00, 0x78,
    0x1E, 0x00, 0x00, 0x78, 0x1E, 0x00, 0x00, 0x78, 0x1E, 0x00, 0x00, 0x78, 0x0F, 0x00, 0x00, 0x70,
    0x0F, 0x00, 0x00, 0xF0, 0x07, 0x80, 0x00, 0xE0, 0x03, 0xC0, 0x01, 0xE0, 0x01, 0xE0, 0x03, 0xC0,
    0x00, 0xF8, 0x0F, 0x00, 0x00, 0x3F, 0xFE, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"8",8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x00, 0x7F, 0xFC, 0x00,
    0x00, 0xF0, 0x3E, 0x00, 0x01, 0xE0, 0x0F, 0x00, 0x03, 0xC0, 0x07, 0x80, 0x07, 0x80, 0x03, 0xC0,
    0x07, 0x00, 0x03, 0xC0, 0x0F, 0x00, 0x01, 0xE0, 0x0F, 0x00, 0x01, 0xE0, 0x0E, 0x00, 0x01, 0xF0,
    0x1E, 0x00, 0x00, 0xF0, 0x1E, 0x00, 0x00, 0xF0, 0x1E, 0x00, 0x00, 0xF0, 0x1E, 0x00, 0x00, 0xF8,
    0x1E, 0x00, 0x00, 0xF8, 0x1E, 0x00, 0x00, 0xF8, 0x1E, 0x00, 0x00, 0xF8, 0x1E, 0x00, 0x00, 0xF8,
    0x1E, 0x00, 0x01, 0xF8, 0x1F, 0x00, 0x01, 0xF8, 0x0F, 0x00, 0x03, 0xF8, 0x0F, 0x80, 0x07, 0xF8,
    0x0F, 0x80, 0x0E, 0xF8, 0x07, 0xC0, 0x1E, 0xF8, 0x03, 0xF0, 0x7C, 0xF8, 0x01, 0xFF, 0xF8, 0xF8,
    0x00, 0xFF, 0xE0, 0xF8, 0x00, 0x3F, 0x81, 0xF0, 0x00, 0x00, 0x01, 0xF0, 0x00, 0x00, 0x01, 0xF0,
    0x00, 0x00, 0x01, 0xF0, 0x00, 0x00, 0x01, 0xF0, 0x00, 0x00, 0x01, 0xE0, 0x00, 0x00, 0x03, 0xE0,
    0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x03, 0x80, 0x07, 0xC0, 0x07, 0xC0, 0x07, 0x80,
    0x07, 0xC0, 0x0F, 0x00, 0x07, 0xC0, 0x0F, 0x00, 0x07, 0xC0, 0x1E, 0x00, 0x03, 0xC0, 0x3C, 0x00,
    0x03, 0xE0, 0xF8, 0x00, 0x00, 0xFF, 0xE0, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"9",9
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x30,
    0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0xC0,
    0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0x80,
    0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0E, 0x00,
    0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x38, 0x00,
    0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x60, 0x00,
    0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x01, 0x80, 0x00,
    0x00, 0x03, 0x80, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
    0x00, 0x0E, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
    0x00, 0x18, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00,
    0x00, 0x60, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00,
    0x01, 0x80, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
    0x1C, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"/",10
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x03, 0xC0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x0F, 0xF0, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x03, 0xC0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x00,
    0x00, 0x07, 0xE0, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x0F, 0xF0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//":",11
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3F, 0xFF, 0xFF, 0xFC, 0x3F, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"-",12
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//" ",13
};

const static u8 label_font_8x16[] ALIGNE(64) = {
    0x00, 0x00, 0x00, 0x18, 0x24, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00, 0x00,//"0"0
    0x00, 0x00, 0x00, 0x10, 0x70, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7C, 0x00, 0x00,//"1"1
    0x00, 0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x04, 0x04, 0x08, 0x10, 0x20, 0x42, 0x7E, 0x00, 0x00,//"2"2
    0x00, 0x00, 0x00, 0x3C, 0x42, 0x42, 0x04, 0x18, 0x04, 0x02, 0x02, 0x42, 0x44, 0x38, 0x00, 0x00,//"3"3
    0x00, 0x00, 0x00, 0x04, 0x0C, 0x14, 0x24, 0x24, 0x44, 0x44, 0x7E, 0x04, 0x04, 0x1E, 0x00, 0x00,//"4"4
    0x00, 0x00, 0x00, 0x7E, 0x40, 0x40, 0x40, 0x58, 0x64, 0x02, 0x02, 0x42, 0x44, 0x38, 0x00, 0x00,//"5"5
    0x00, 0x00, 0x00, 0x1C, 0x24, 0x40, 0x40, 0x58, 0x64, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00, 0x00,//"6"6
    0x00, 0x00, 0x00, 0x7E, 0x44, 0x44, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00,//"7"7
    0x00, 0x00, 0x00, 0x3C, 0x42, 0x42, 0x42, 0x24, 0x18, 0x24, 0x42, 0x42, 0x42, 0x3C, 0x00, 0x00,//"8"8
    0x00, 0x00, 0x00, 0x18, 0x24, 0x42, 0x42, 0x42, 0x26, 0x1A, 0x02, 0x02, 0x24, 0x38, 0x00, 0x00,//"9"9
    0x00, 0x00, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x00,//"/"10
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00,//":"11
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//"-"12
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//" "13
};

const static char label_index_tbl[] = "0123456789/:- ";

static char label_format[] = "yyyy-nn-dd hh:mm:ss";
static char label_format1[] = "  yyyy-nn-dd hh:mm:ss   ";

const static struct jpg_q_table jpg_Q_table[] = {
    {
        //50
        {
            0x0040, 0x0043, 0x004e, 0x0036, 0x002a, 0x0020, 0x0025, 0x003c,
            0x003d, 0x002c, 0x0028, 0x0021, 0x001c, 0x0010, 0x0016, 0x0030,
            0x0037, 0x002b, 0x0025, 0x001b, 0x0013, 0x0011, 0x0014, 0x0032,
            0x003e, 0x0024, 0x001e, 0x0019, 0x0011, 0x000c, 0x0014, 0x0032,
            0x0038, 0x0021, 0x0015, 0x000f, 0x000f, 0x000b, 0x0012, 0x0030,
            0x0036, 0x001a, 0x0012, 0x0011, 0x0010, 0x000f, 0x0015, 0x0033,
            0x0026, 0x0015, 0x0012, 0x0012, 0x0012, 0x0013, 0x001d, 0x0043,
            0x0033, 0x001d, 0x001d, 0x0020, 0x0021, 0x002f, 0x0042, 0x0087,
        },

        {
            0x003c, 0x0029, 0x0020, 0x0012, 0x000a, 0x000d, 0x0013, 0x0025,
            0x0029, 0x0019, 0x0015, 0x0009, 0x0007, 0x0009, 0x000d, 0x001b,
            0x0020, 0x0015, 0x000a, 0x0006, 0x0007, 0x000a, 0x000e, 0x001c,
            0x0012, 0x0009, 0x0006, 0x0007, 0x0008, 0x000b, 0x0010, 0x001f,
            0x000a, 0x0007, 0x0007, 0x0008, 0x000a, 0x000d, 0x0013, 0x0025,
            0x000d, 0x0009, 0x000a, 0x000b, 0x000d, 0x0010, 0x0018, 0x002f,
            0x0013, 0x000d, 0x000e, 0x0010, 0x0013, 0x0018, 0x0023, 0x0045,
            0x0025, 0x001b, 0x001c, 0x001f, 0x0025, 0x002f, 0x0045, 0x0087,
        },

        {
            0xFF, 0xDB, 0x00, 0x43, 0x00,
            0x10, 0x0b, 0x0c, 0x0e, 0x0c, 0x0a, 0x10, 0x0e, 0x0d, 0x0e, 0x12, 0x11, 0x10, 0x13, 0x18, 0x28,
            0x1a, 0x18, 0x16, 0x16, 0x18, 0x31, 0x23, 0x25, 0x1d, 0x28, 0x3a, 0x33, 0x3d, 0x3c, 0x39, 0x33,
            0x38, 0x37, 0x40, 0x48, 0x5c, 0x4e, 0x40, 0x44, 0x57, 0x45, 0x37, 0x38, 0x50, 0x6d, 0x51, 0x57,
            0x5f, 0x62, 0x67, 0x68, 0x67, 0x3e, 0x4d, 0x71, 0x79, 0x70, 0x64, 0x78, 0x5c, 0x65, 0x67, 0x63,
            0xFF, 0xDB, 0x00, 0x43, 0x01,
            0x11, 0x12, 0x12, 0x18, 0x15, 0x18, 0x2f, 0x1a, 0x1a, 0x2f, 0x63, 0x42, 0x38, 0x42, 0x63, 0x63,
            0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
            0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
            0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
        },
    },

//85
    {
        {
            0x00cc, 0x00f6, 0x0105, 0x00ae, 0x0092, 0x006c, 0x007e, 0x00ce,
            0x00b8, 0x0085, 0x008d, 0x0068, 0x005c, 0x0037, 0x004b, 0x009d,
            0x00c3, 0x008d, 0x0077, 0x005f, 0x0041, 0x003a, 0x0044, 0x00a7,
            0x00d9, 0x007d, 0x005f, 0x0052, 0x003a, 0x002a, 0x0043, 0x00a6,
            0x00cc, 0x0069, 0x0047, 0x0033, 0x0033, 0x0027, 0x003d, 0x00a1,
            0x00ba, 0x0055, 0x003a, 0x003a, 0x0036, 0x0035, 0x0046, 0x00a8,
            0x007e, 0x0047, 0x003e, 0x003d, 0x003d, 0x0042, 0x0061, 0x00e4,
            0x00a8, 0x005f, 0x0061, 0x006c, 0x006d, 0x009d, 0x00dd, 0x01c0,
        },

        {
            0x00cc, 0x0093, 0x006f, 0x003e, 0x0022, 0x002b, 0x003f, 0x007b,
            0x0093, 0x0058, 0x0046, 0x001f, 0x0018, 0x001f, 0x002d, 0x0059,
            0x006f, 0x0046, 0x0023, 0x0016, 0x001a, 0x0021, 0x0030, 0x005e,
            0x003e, 0x001f, 0x0016, 0x0018, 0x001d, 0x0024, 0x0035, 0x0069,
            0x0022, 0x0018, 0x001a, 0x001d, 0x0022, 0x002b, 0x003f, 0x007b,
            0x002b, 0x001f, 0x0021, 0x0024, 0x002b, 0x0037, 0x0050, 0x009d,
            0x003f, 0x002d, 0x0030, 0x0035, 0x003f, 0x0050, 0x0074, 0x00e4,
            0x007b, 0x0059, 0x005e, 0x0069, 0x007b, 0x009d, 0x00e4, 0x01c0,
        },

        {
            0xFF, 0xDB, 0x00, 0x43, 0x00,
            0x05, 0x03, 0x04, 0x04, 0x04, 0x03, 0x05, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, 0x07, 0x0c,
            0x08, 0x07, 0x07, 0x07, 0x07, 0x0f, 0x0b, 0x0b, 0x09, 0x0c, 0x11, 0x0f, 0x12, 0x12, 0x11, 0x0f,
            0x11, 0x11, 0x13, 0x16, 0x1c, 0x17, 0x13, 0x14, 0x1a, 0x15, 0x11, 0x11, 0x18, 0x21, 0x18, 0x1a,
            0x1d, 0x1d, 0x1f, 0x1f, 0x1f, 0x13, 0x17, 0x22, 0x24, 0x22, 0x1e, 0x24, 0x1c, 0x1e, 0x1f, 0x1e,
            0xFF, 0xDB, 0x00, 0x43, 0x01,
            0x05, 0x05, 0x05, 0x07, 0x06, 0x07, 0x0e, 0x08, 0x08, 0x0e, 0x1e, 0x14, 0x11, 0x14, 0x1e, 0x1e,
            0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
            0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
            0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
        },
    },
//100
    {

        {
            0x0400, 0x02e2, 0x030f, 0x0366, 0x0400, 0x0517, 0x0764, 0x0e7f,
            0x02e2, 0x0214, 0x0235, 0x0273, 0x02e2, 0x03ab, 0x0554, 0x0a73,
            0x030f, 0x0235, 0x0257, 0x029a, 0x030f, 0x03e5, 0x05a8, 0x0b18,
            0x0366, 0x0273, 0x029a, 0x02e4, 0x0366, 0x022a, 0x0324, 0x0c54,
            0x0400, 0x02e2, 0x030f, 0x0366, 0x0400, 0x028b, 0x03b2, 0x073f,
            0x0517, 0x03ab, 0x03e5, 0x0454, 0x028b, 0x033d, 0x04b4, 0x0939,
            0x0764, 0x0554, 0x02d4, 0x0324, 0x03b2, 0x04b4, 0x06d4, 0x0d64,
            0x0e7f, 0x0539, 0x058c, 0x062a, 0x073f, 0x0939, 0x0d64, 0x1a46,
        },

        {
            0x0400, 0x02e2, 0x030f, 0x0366, 0x0200, 0x028b, 0x03b2, 0x073f,
            0x02e2, 0x0214, 0x0235, 0x0273, 0x0171, 0x01d5, 0x02aa, 0x0539,
            0x030f, 0x0235, 0x0257, 0x014d, 0x0187, 0x01f2, 0x02d4, 0x058c,
            0x0366, 0x0273, 0x014d, 0x0172, 0x01b3, 0x022a, 0x0324, 0x062a,
            0x0200, 0x0171, 0x0187, 0x01b3, 0x0200, 0x028b, 0x03b2, 0x073f,
            0x028b, 0x01d5, 0x01f2, 0x022a, 0x028b, 0x033d, 0x04b4, 0x0939,
            0x03b2, 0x02aa, 0x02d4, 0x0324, 0x03b2, 0x04b4, 0x06d4, 0x0d64,
            0x073f, 0x0539, 0x058c, 0x062a, 0x073f, 0x0939, 0x0d64, 0x1a46,
        },

        {
            0xFF, 0xDB, 0x00, 0x43, 0x00,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,
            0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
            0xFF, 0xDB, 0x00, 0x43, 0x01,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x02, 0x02,
            0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
            0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
            0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        },

    },
};

extern int set_camera_config(struct photo_camera *camera);
extern int video_photo_set_config(struct intent *it);
void video_photo_cfg_reset(void);
extern void video_photo_restore();
#endif
