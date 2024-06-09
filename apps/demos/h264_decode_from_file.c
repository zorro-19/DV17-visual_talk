/******************************************************************************
* File:             h264_decode_from_file.c
*
* Author:           jiabum
* Created:          08/21/23
* Description:      实现从sd卡加载h264裸流解码显示demo示例
*****************************************************************************/

#include "device/video.h"
#include "system/init.h"
#include "device/device.h"
#include "asm/jpeg_codec.h"
#include "device/videobuf.h"
#include "device/audio_dev.h"
#include "os/os_compat.h"
#include "system/timer.h"
#include "system/includes.h"
#include "generic/circular_buf.h"
#include "asm/lcd_config.h"

/* #define H264_DECODE_TEST_FROM_FILE__ENABLE */

#ifdef H264_DECODE_TEST_FROM_FILE__ENABLE

struct frame_t {
    struct list_head entry;
    u32 frame_num;  //帧号
    u32 frame_len;  //帧长
    u32 frame_addr; //帧所在地址
};
static struct list_head frames_list = {0};
static void *dec_fd = NULL;

static void dec_h264_task(void *priv);
static int is_start_code(unsigned char *data)
{
    return (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x01) ? 1 : 0;
}

static void frames_list_free(struct list_head *head)
{
    struct frame_t *p, *n;
    list_for_each_entry_safe(p, n, head, entry) {
        /* log_d("%d : %x -- %d\n",p->frame_num,p->frame_addr,p->frame_len ); */
        list_del(&p->entry);
        free(p);
    }
}


int dec_h264_test(void)
{
    struct video_format f = {0};
    struct video_mirror vmirror = {0};
    dec_fd = dev_open("video_dec", NULL);
    if (!dec_fd) {
        ASSERT(0, "video_dec no define");
    }
    f.type              = VIDEO_BUF_TYPE_VIDEO_PLAY;
    f.src_width         = 0;
    f.src_height        = 0;
    f.pixelformat       = VIDEO_PIX_FMT_H264; // VIDEO_PIX_FMT_JPEG
    f.private_data      = "fb1";
    f.win.left          = 0;
    f.win.top           = 0;
    f.win.width         = LCD_DEV_WIDTH;
    f.win.height        = LCD_DEV_HIGHT;
    f.win.border_left   = 0;
    f.win.border_top    = 0;
    f.win.border_right  = 0;
    f.win.border_bottom = 0;
    //旋转参数配置:
    //0:不旋转,不镜像 (原图)
    //1:逆时针旋转90度,不镜像
    //2:逆时针旋转270度,不镜像
    //3:逆时针旋转90度后,再垂直镜像
    //4:逆时针旋转90度后,再水平镜像
    f.dis_rotate = 0; //rotate

    dev_ioctl(dec_fd, VIDIOC_SET_FMT, (u32)&f);

    vmirror.mirror = 0; //mirror
    dev_ioctl(dec_fd, VIDIOC_SET_MIRROR, (u32)&vmirror);

    dev_ioctl(dec_fd, VIDIOC_PLAY, 1);
    thread_fork("dec_h264_test", 20, 1024, 0, NULL, dec_h264_task, NULL);

    return 0;
}

static void read_raw_h264(FILE *f)
{
#define READ_SIZE   512
    log_d("read_raw_h264...\n");
    int ret;
    unsigned char data[READ_SIZE];
    int cur_addr = 0;
    int frame_num = 0;
    struct frame_t *_frame = NULL;
    u32 last_frame_addr = 0;

    if (!f) {
        log_e("can not open 264 file\n");
        return ;
    }
    while (1) {
        ret = fread(f, data, READ_SIZE);
        if (ret != READ_SIZE) {
            /* log_d("ret=%d\n",ret ); */
            break;
        }
        if (is_start_code(data)) {
            if (frame_num) {
                _frame->frame_len = cur_addr - last_frame_addr;
                list_add_tail(&_frame->entry, &frames_list);
            }
            frame_num++;
            _frame = malloc(sizeof(struct frame_t));
            _frame->frame_num = frame_num;
            _frame->frame_addr = cur_addr;
            last_frame_addr = cur_addr;
        }
        cur_addr += READ_SIZE;
    }

    log_d("frame_num =%d\n", frame_num);
    /* frames_list_free(&frames_list); */
}
static void dec_h264_task(void *priv)
{
    struct frame_t *p;
    u8 *buf;
    FILE *f = fopen(CONFIG_ROOT_PATH"H264001.264", "r");
    if (!f) {
        puts("264 file open fail!\n");
        return;
    }
    INIT_LIST_HEAD(&frames_list);

    read_raw_h264(f); //预读出h264数据帧

    fseek(f, 0, SEEK_SET);

    while (1) {
        if (f) {
            log_d("start dec h264 stream...");
            list_for_each_entry(p, &frames_list, entry) {
                buf = malloc(p->frame_len);
                fseek(f, p->frame_addr, SEEK_SET);
                fread(f, buf, p->frame_len); //取出对应h264帧
                /* 送入解码器解码显示 */
                dev_write(dec_fd, buf, p->frame_len);
                os_time_dly(4); //1000ms/25fps = 40ms
                free(buf);
                buf = NULL;
            }
        }
        log_d("start dec h264 stream end.");
        os_time_dly(300);
        break;
    }

    if (f) {
        fclose(f);
        frames_list_free(&frames_list);
        dev_ioctl(dec_fd, VIDIOC_PLAY, 1);
        dev_close(dec_fd);
        dec_fd = NULL;
    }
}


#endif
