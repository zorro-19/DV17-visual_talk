#include "system/includes.h"
#include "os/os_compat.h"
#include "app_config.h"
#include "get_image_data.h"

static get_yuv_cfg  __info;
static void *video_dev_fd = NULL, *fb2 = NULL;


void get_yuv_task(void *priv)
{
    int ret;
    //数据来源video，输出到fb2
    struct video_format f = {0};
    struct fb_var_screeninfo fb2_info = {0};
    const char *dev_name = YUV_DATA_SOURCE;
    const char *fb_name = "fb2";

#if (defined CONFIG_VIDEO3_ENABLE)
    int cnt = 0;
    while (1) {
        msleep(300);
        if (get_video3_online_flag() == true) {
            cnt++;
            if (cnt >= 5) {
                break;
            }
        } else {
            cnt = 0;
        }
    }
#elif (defined CONFIG_VIDEO0_ENABLE)


#ifdef CONFIG_VIDEO0_INIT_ASYNC
    extern int os_query_init_video_sem(void);
    if (os_query_init_video_sem()) {
        printf("\n os_query_init_video_sem err\n");
        hi3861l_reset();
    }
#endif // CONFIG_VIDEO0_INIT_ASYNC

#endif // CONFIG_VIDEO3_ENABLE

__again:

    video_dev_fd = dev_open(dev_name, NULL);
    if (video_dev_fd == NULL) {
        log_e(">>>>>>>> %s %d\n", __func__, __LINE__);
        return;
    }
    f.type  = VIDEO_BUF_TYPE_VIDEO_OVERLAY;
    f.win.left = 0;
    f.win.top    = 0;
    f.win.width  = YUV_DATA_WIDTH;
    f.win.height = YUV_DATA_HEIGHT;
    f.win.border_left   = 0;
    f.win.border_top    = 0;
    f.win.border_right  = 0;
    f.win.border_bottom = 0;
    f.private_data   = fb_name;
    ret = dev_ioctl(video_dev_fd, VIDIOC_SET_FMT, (u32)&f);
    if (ret) {
        log_e(">>>>>>>> %s %d\n", __func__, __LINE__);
        dev_close(video_dev_fd);
        return;
    }
    if (f.src_width == 0) {
        log_e(">>>>>>>> %s %d\n", __func__, __LINE__);
        dev_close(video_dev_fd);
        video_dev_fd = NULL;
        msleep(100);
        goto __again;
    }

#if 0
#include "isp.h"
    ispt_params_flush();
    ispt_params_set_smooth_step(1);
    struct ispt_customize_cfg cfg = {0};;
    cfg.mode = ISP_CUSTOMIZE_MODE_FILE;
    cfg.data = data;
    cfg.len =  sizeof(data);
    int err = dev_ioctl(video_dev_fd, ISP_IOCTL_SET_CUSTOMIZE_CFG, (unsigned int)&cfg);
    if (err) {
        log_e(">>>>>>>> %s %d\n", __func__, __LINE__);
        dev_close(video_dev_fd);
        video_dev_fd = NULL;
        return;
    }
#endif // 0

    ret = dev_ioctl(video_dev_fd, VIDIOC_OVERLAY, 1);
    if (ret) {
        log_e(">>>>>>>> %s %d\n", __func__, __LINE__);
        dev_close(video_dev_fd);
        return;
    }


    //打开显示通道
    fb2 = dev_open(f.private_data, (void *)FB_COLOR_FORMAT_YUV420);
    if (!fb2) {
        log_e(">>>>>>>> %s %d\n", __func__, __LINE__);
        dev_close(video_dev_fd);
        return;
    }



    struct fb_map_user map1;

    while (1) {

#ifdef CONFIG_VIDEO3_ENABLE
        if ((get_video3_online_flag() == FALSE) || !dev_online("uvc")) {
            __info.exit_state = 0;
            break;
        }
#endif // CONFIG_VIDEO3_ENABLE

        if (__info.exit_state) {
            __info.exit_state = 0;
            break;
        }
        printf("\n scan\n");
        dev_ioctl(fb2, FBIOGET_FBUFFER_INUSED, (int)&map1);
        if ((u32)map1.baddr) {
//            printf("\n %s %d\n",__func__,__LINE__);
            //需要使用YUV数据的任务的回调
            //printf("\nmap1.baddr is 0x%x\n",map1.baddr);
            __info.cb(map1.baddr);
            //获取到才释放
            dev_ioctl(fb2, FBIOPUT_FBUFFER_INUSED, (int)&map1);

            static int fps;
            static int t = 0;
            fps++;
            if (t == 0) {
                t = timer_get_ms();
            }
            if (timer_get_ms() - t >= 1000) {
                printf("\n %s %d decode fps = %d\n", __func__, __LINE__, fps);
                t = timer_get_ms();
                fps = 0;
            }


        } else {
            //获取不到重试
            printf("\n %s %d no yuv data\n", __func__, __LINE__);
            continue;
        }

    }

    dev_close(fb2);
    dev_ioctl(video_dev_fd, VIDIOC_OVERLAY, 0);
    dev_close(video_dev_fd);
    printf("get_yuv_task exit\n\n ");

}

static u8 get_yuv_init_flag;
void get_yuv_init(void (*cb)(u8 *data))
{

    if (get_yuv_init_flag == 1) {
        return;
    }


    get_yuv_init_flag = 1;
    __info.cb = cb;
    __info.exit_state = 0;


    thread_fork("GET_YUV_TASK", 8, 0x2000, 0, &__info.pid, get_yuv_task, NULL);
}
void get_yuv_uninit(void)
{
    if (get_yuv_init_flag == 0) {
        return;
    }
    get_yuv_init_flag = 0;
    __info.exit_state = 1;
    thread_kill(&__info.pid, KILL_WAIT);
}
