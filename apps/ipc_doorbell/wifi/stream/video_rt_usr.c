
#define _STDIO_H_
#include "xciot_api.h"
#include "server/rt_stream_pkg.h"
#include "stream_core.h"
#include "sock_api/sock_api.h"
#include "app_config.h"
#include "doorbell_event.h"
/**************************************************************************************************************
1、修改自定义路径，net_config.h打开CONFIG_NET_UDP_ENABLE，改写NET_USR_PATH的自定义路径
2、参照user_video_rec.c两个打开和关闭函数
3、当打开video_rt_usr_init()函数返回非NULL，打开完成之后，有数据会调用到video_rt_usr_send()函数，
	type的类型是JPEG_TYPE_VIDEO则为视频帧，PCM_TYPE_AUDIO则为音频。
**************************************************************************************************************/
static u32(*video_rt_cb)(void *, u8 *, u32);
static void *cb_priv;

static const char Iframe[5] = {0x0, 0x0, 0x0, 0x1, 0x67};
static const char Pframe[5] = {0x0, 0x0, 0x0, 0x1, 0x41};







void set_video_rt_cb(u32(*cb)(void *, u8 *, u32), void *priv)
{
    video_rt_cb = cb;
    cb_priv = priv;
}



extern void jl_abr_change_qp_release_privileged_mode(void);
extern void jl_abr_change_qp_range_privileged(u8 static_min, u8 static_max, u8 dynamic_min, u8 dynamic_max);
extern u8 get_avsdk_init_flag(void);
static void net_video_change_qb(int len)
{
    static int index;
    static int frame_size[15];
    static int tmr;
    static u8 static_min = 31;
    static u8 static_max = 35;
    static u8 dynamic_min = 32;
    static u8 dynamic_max = 34;

    frame_size[index] = len;
    index = ++index == 15 ? 0 : index;
    if (tmr == 0) {
        tmr = timer_get_ms();
    }
    if (timer_get_ms() - tmr >= 1000) {
        tmr = timer_get_ms();
        u32 sum = 0;
        for (u8 i = 0; i < 15; i++) {
            sum += frame_size[i];
        }
        if (sum <= 512 * 1024) {
            if (static_max > 35) {
                static_min -= 1;
                static_max -= 1;
                dynamic_min -= 1;
                dynamic_max -= 1;
                jl_abr_change_qp_range_privileged(static_min, static_max, dynamic_min, dynamic_max);
            } else {
                jl_abr_change_qp_release_privileged_mode();
            }
        } else {
            int diff = 5;
            do {
                if (static_max + diff  < 51) {
                    break;
                }
                diff--;
            } while (1);
            static_min += diff;
            static_max += diff;
            dynamic_min += diff;
            dynamic_max += diff;
            jl_abr_change_qp_range_privileged(static_min, static_max, dynamic_min, dynamic_max);
        }
    }
}





extern u32 FIRST_FRAME_TIME;
int send_video_data(void *hdl, u8 *data, u32 len, u8 type)
{
    u8 is_key_frame;
    if (len == 8) {
        return len;
    }

    static int flag;
    if (!flag) {
        flag = 1;
        printf("\n\n\n>>>>>>>>>>> FIRST_FRAME_TIME = %d \n\n", timer_get_ms() - FIRST_FRAME_TIME);

    }

    net_video_change_qb(len);

//    put_buf(data,64);
#if CONFIG_H264_STD_HEAD_ENABLE

    if (!memcmp(data, Iframe, 5)) {
        is_key_frame = 1;
    } else if (!memcmp(data, Pframe, 5)) {
        is_key_frame = 0;
    }

    if (get_avsdk_init_flag() == 2) {
       //   printf("\n w ");
        avsdk_write_video(E_IOT_MEDIASTREAM_TYPE_H264, is_key_frame, 0, get_utc_ms(), NULL, data, len);
    }

    cloud_storage_video_write(type, is_key_frame, timer_get_ms(), data, len, 0);

#else
    static u8 vframe[512 * 1024];
    if (len > sizeof(vframe)) {
        printf("\n vframe len = %d\n", len / 1024);
        return len;
    }

    memcpy(vframe, data, len);
    u32 start_code = 0x01000000;
    u32 tmp2;
    u32 tmp;
    if (*((char *)(vframe + 4)) == 0x67) {
        memcpy(&tmp, vframe, 4);
        tmp = htonl(tmp);
        memcpy(vframe, &start_code, 4);
        memcpy(&tmp2, vframe + tmp + 4, 4);
        tmp2 = htonl(tmp2);
        memcpy(vframe + tmp + 4, &start_code, 4);
        memcpy(vframe + tmp + tmp2 + 8, &start_code, 4);
        is_key_frame = 1;

    } else {
        memcpy(vframe, &start_code, 4);
        is_key_frame = 0;
    }
    if (get_avsdk_init_flag() == 2) {

        avsdk_write_video(E_IOT_MEDIASTREAM_TYPE_H264, is_key_frame, 0, get_utc_ms(), NULL, vframe, len);
    }

    cloud_storage_video_write(type, is_key_frame, timer_get_ms(), vframe, len, 0);


#endif // CONFIG_SHARED_REC_HDL_ENABLE
    return len;
}

#define AUDIO_PACKET_LEN        2048
int send_audio_data(void *hdl, u8 *data, u32 len, u8 type)
{
    uint64_t utc_ms = 0;
    int wlen;
    int offset = 0;
    int total_len = len;

#if (CONFIG_AUDIO_FORMAT_FOR_MIC == AUDIO_FORMAT_AAC)
    int stream_type = E_IOT_MEDIASTREAM_TYPE_AAC;
#else
    int stream_type = E_IOT_MEDIASTREAM_TYPE_PCM;
#endif

#ifndef CONFIG_SHARED_REC_HDL_ENABLE

    if (get_avsdk_init_flag() == 2) {
     //   putchar('A');
        avsdk_write_audio(stream_type, 0, get_utc_ms(), NULL, data, len);
    }

    cloud_storage_audio_write(type, timer_get_ms(), data, len, 0);

#else
    utc_ms = get_utc_ms();
    while (total_len) {
        wlen = total_len > AUDIO_PACKET_LEN ? AUDIO_PACKET_LEN : total_len;
        avsdk_write_audio(stream_type, 0, utc_ms, NULL, data + offset, wlen);
        utc_ms +=  wlen / (net_video_rec_get_audio_rate() / 500);
        offset += wlen;
        total_len -= wlen;
    }


    int64_t adifftime = 0;
    adifftime = timer_get_ms();
    total_len = len;
    offset = 0;
    while (total_len) {
        wlen = total_len > AUDIO_PACKET_LEN ? AUDIO_PACKET_LEN : total_len;
        cloud_storage_audio_write(type, adifftime, data + offset, wlen, 0);
        adifftime +=  wlen / (net_video_rec_get_audio_rate() / 500);
        offset += wlen;
        total_len -= wlen;
    }

#endif
    return len;
}


static void *video_rt_usr_init(const char *path, const char *mode)
{
    return (void *)1;
}

static int video_rt_usr_send(void *hdr, void *data, u32 len, u8 type)
{


    if (type == H264_TYPE_VIDEO) {
       // putchar('V');
        static CAL_INFO cal_info = {
            .func = __func__,
            .line = __LINE__,
            .des = "enc",
        };

    static int start=0,ts=0,sum=0,fps=0;
    if (start == 0) {
        start=1;
        ts = timer_get_ms();
    }
   // printf("\n len:%d\n",len);
    sum+=len;
    fps++;
    if (timer_get_ms() - ts >= 1000) {
      //  printf("\n sum: %d KB, fps = %d\n",sum/1024,fps);
        fps = 0;
        sum=0;
        ts = timer_get_ms();
    }

    //    calculate_frame_rate(&cal_info);
#ifndef CONFIG_UVCX_TEST_ENABLE
//        put_buf(data,64);
        send_video_data(hdr, data, len, type);
#endif // CONFIG_UVCX_TEST_ENABLE
    } else {
      //  putchar('A');
#ifndef CONFIG_UVCX_TEST_ENABLE
        send_audio_data(hdr, data, len, type);
#endif // CONFIG_UVCX_TEST_ENABLE
    }


    return len;
}

static int video_rt_usr_uninit(void *hdr)
{
    return 0;
}



REGISTER_NET_VIDEO_STREAM_SUDDEV(usr_video_stream_sub) = {
    .name = "usr",
    .open =  video_rt_usr_init,
    .write = video_rt_usr_send,
    .close = video_rt_usr_uninit,
};




static void *video_rt_uvc0_init(const char *path, const char *mode)
{
    return (void *)1;
}

static int video_rt_uvc0_send(void *hdr, void *data, u32 len, u8 type)
{

    u8 is_key_frame;
    if (type == H264_TYPE_VIDEO || type == JPEG_TYPE_VIDEO) {
      //  putchar('V');
        static CAL_INFO cal_info = {
            .func = __func__,
            .line = __LINE__,
            .des = "enc",
        };
        calculate_frame_rate(&cal_info);
////        put_buf(data,64);
        if (!memcmp(data, Iframe, 5)) {
            is_key_frame = 1;
        } else if (!memcmp(data, Pframe, 5)) {
            is_key_frame = 0;
        }

        if (get_avsdk_init_flag() == 2) {
#ifndef CONFIG_NET_DUAL_STREAM_ENABLE
            avsdk_write_video(E_IOT_MEDIASTREAM_TYPE_H264, is_key_frame, 0, get_utc_ms(), NULL, data, len);
#else
            avsdk_write_video(E_IOT_MEDIASTREAM_TYPE_H264, is_key_frame, 1, get_utc_ms(), NULL, data, len);
#endif // CONFIG_NET_DUAL_STREAM_ENABLE

        }
#if  (CONFIG_CLOUD_STORAGE_NUM == 2)
        cloud_storage_video_write(type, is_key_frame, timer_get_ms(), data, len, 1);
#endif
    } else {
      //  putchar('A');
#if (CONFIG_AUDIO_FORMAT_FOR_MIC == AUDIO_FORMAT_AAC)
        int stream_type = E_IOT_MEDIASTREAM_TYPE_AAC;
#else
        int stream_type = E_IOT_MEDIASTREAM_TYPE_PCM;
#endif
        if (get_avsdk_init_flag() == 2) {
            avsdk_write_audio(stream_type, 0, get_utc_ms(), NULL, data, len);
        }
   		cloud_storage_audio_write(type, timer_get_ms(), data, len, 0);
    }


    return len;
}
static int video_rt_uvc0_uninit(void *hdr)
{
    return 0;
}

REGISTER_NET_VIDEO_STREAM_SUDDEV(uvc0_video_stream_sub) = {
    .name = "uvc0",
    .open =  video_rt_uvc0_init,
    .write = video_rt_uvc0_send,
    .close = video_rt_uvc0_uninit,
};
static void *video_rt_uvc1_init(const char *path, const char *mode)
{
    return (void *)1;
}


static int video_rt_uvc1_send(void *hdr, void *data, u32 len, u8 type)
{


    u8 is_key_frame;
    if (type == H264_TYPE_VIDEO || type == JPEG_TYPE_VIDEO) {
        putchar('H');
        static CAL_INFO cal_info = {
            .func = __func__,
            .line = __LINE__,
            .des = "enc",
        };
        calculate_frame_rate(&cal_info);
        if (!memcmp(data, Iframe, 5)) {
            is_key_frame = 1;
        } else if (!memcmp(data, Pframe, 5)) {
            is_key_frame = 0;
        }

        if (get_avsdk_init_flag() == 2) {
            avsdk_write_video(E_IOT_MEDIASTREAM_TYPE_H264, is_key_frame, 2, get_utc_ms(), NULL, data, len);
        }
#if  (CONFIG_CLOUD_STORAGE_NUM == 2)
        cloud_storage_video_write(type, is_key_frame, timer_get_ms(), data, len, 2);
#endif
    } else {
        putchar('I');
    }


    return len;
}
static int video_rt_uvc1_uninit(void *hdr)
{
    return 0;
}
REGISTER_NET_VIDEO_STREAM_SUDDEV(uvc1_video_stream_sub) = {
    .name = "uvc1",
    .open =  video_rt_uvc1_init,
    .write = video_rt_uvc1_send,
    .close = video_rt_uvc1_uninit,
};
static void *video_rt_uvc2_init(const char *path, const char *mode)
{
    return (void *)1;
}

static int video_rt_uvc2_send(void *hdr, void *data, u32 len, u8 type)
{


    if (type == H264_TYPE_VIDEO || type == JPEG_TYPE_VIDEO) {
        putchar('U');
        static CAL_INFO cal_info = {
            .func = __func__,
            .line = __LINE__,
            .des = "enc",
        };
        calculate_frame_rate(&cal_info);
    } else {
        putchar('J');
    }


    return len;
}
static int video_rt_uvc2_uninit(void *hdr)
{
    return 0;
}
REGISTER_NET_VIDEO_STREAM_SUDDEV(uvc2_video_stream_sub) = {
    .name = "uvc2",
    .open =  video_rt_uvc2_init,
    .write = video_rt_uvc2_send,
    .close = video_rt_uvc2_uninit,
};

static void *video_rt_uvc3_init(const char *path, const char *mode)
{
    return (void *)1;
}

static int video_rt_uvc3_send(void *hdr, void *data, u32 len, u8 type)
{


    if (type == H264_TYPE_VIDEO || type == JPEG_TYPE_VIDEO) {
        putchar('Q');
        static CAL_INFO cal_info = {
            .func = __func__,
            .line = __LINE__,
            .des = "enc",
        };
        calculate_frame_rate(&cal_info);
    } else {
        putchar('Z');

    }


    return len;
}
static int video_rt_uvc3_uninit(void *hdr)
{
    return 0;
}
REGISTER_NET_VIDEO_STREAM_SUDDEV(uvc3_video_stream_sub) = {
    .name = "uvc3",
    .open =  video_rt_uvc3_init,
    .write = video_rt_uvc3_send,
    .close = video_rt_uvc3_uninit,
};


