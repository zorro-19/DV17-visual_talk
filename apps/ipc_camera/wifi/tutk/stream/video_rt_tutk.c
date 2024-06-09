//////////////////////////////////////////////
//net net net net net net
//
//test
//
//test
//
//
//
//
//
//////////////////////////////////////////////
//
//
//
#include "system/includes.h"
#include "server/video_server.h"
#include "server/video_engine_server.h"
#include "ui_ctr_def.h"
#include "video_rec.h"


#include "app_config.h"
#include "asm/debug.h"
#include "vrec_osd.h"
#include "vrec_icon_osd.h"







#include "IOTCAPIs.h"
#include "AVAPIs.h"
#include "P2PCam/AVIOCTRLDEFs.h"
#include "P2PCam/AVFRAMEINFO.h"

#define LOW_QUA   700
#define MID_QUA   1000
#define HIGH_QUA  2000

#define VIDEO_STREAM_CHANNEL  0x3
#define AUDIO_STREAM_CHANNEL  0x2

static int AuthCallBackFn(char *viewAcc, char *viewPwd)
{
#if 0
    if (strcmp(viewAcc, tinfo.username) == 0 && strcmp(viewPwd, tinfo.password) == 0) {
        printf("\n %s suc\n", __func__);
        return 1;
    }
    printf("\n tutk_srv.username = %s viewAcc = %s\n", tinfo.username, viewAcc);
    printf("\n tutk_srv.password = %s viewPwd = %s \n", tinfo.password, viewPwd);
#endif
    return 1;
}


#define AAC_TYPE_AUDIO      0
#define PCM_TYPE_AUDIO      1
#define JPEG_TYPE_VIDEO     2
#define H264_TYPE_VIDEO     3
#define PREVIEW_TYPE        4
#define DATE_TIME_TYPE      5
#define MEDIA_INFO_TYPE     6
#define PLAY_OVER_TYPE      7
#define GPS_INFO_TYPE       8
#define NO_GPS_DATA_TYPE    9
#define G729_TYPE_AUDIO    10


struct frm_head {
    u8 type;
    u8 res;
#if 0
    u8 sample_seq;
#endif
    u16 payload_size;
    u32 seq;
    u32 frm_sz;
    u32 offset;
    u32 timestamp;
} __attribute__((packed));


static void rec_dev_server_event_handler(void *priv, int argc, int *argv)
{
    /*
     *该回调函数会在录像过程中，写卡出错被当前录像APP调用，例如录像过程中突然拔卡
     */
    switch (argv[0]) {
    case VIDEO_SERVER_UVM_ERR:
        log_e("APP_UVM_DEAL_ERR\n");
        break;
    case VIDEO_SERVER_PKG_ERR:
        log_e("TUTK STREAM VIDEO_SERVER_PKG_ERR\n");
        break;
    case VIDEO_SERVER_PKG_END:
        log_e("TUTK STREAM VIDEO_SERVER_PKG_END\n");
        break;
    case VIDEO_SERVER_NET_ERR:
        log_e("TUTK STREAM VIDEO_SERVER_NET_ERR\n");
        break;
    default :
        log_e("unknow rec server cmd %x , %x!\n", argv[0], (int)priv);
        break;
    }
}


static void *xopen(const char *path, void *arg)
{
    log_d("\n\n\nxopen\n\n");
    int nResend = -1;
    struct tutk_client_info *cinfo = (struct tutk_client_info *)arg;
    log_d("cinfo->session_id=%d\n", cinfo->session_id);
    cinfo->streamindex = avServStart3(cinfo->session_id
                                      , AuthCallBackFn
                                      , 5
                                      , 0
                                      , VIDEO_STREAM_CHANNEL
                                      , &nResend);
    if (cinfo->streamindex < 0) {
        log_e("tutk stream open fail streamindex=%d\n", cinfo->streamindex);
        return NULL;
    }
    log_d("cinfo xopen end\n\n");

    return cinfo;

}

static int xsend(void *fd, char *data, int len, unsigned int flag)
{
    int ret = 0;
    struct tutk_client_info *cinfo = (struct tutk_client_info *)fd;
    /* log_d("111cinfo->session_id=%d\n",cinfo->session_id); */
    struct frm_head frame_head = {0};
    static u32 v_seq = 0, a_seq = 0;
    /* printf("flag:%d\n", flag); */
    if (flag == H264_TYPE_VIDEO) {
        frame_head.type = H264_TYPE_VIDEO;
        frame_head.seq = (v_seq++);
        frame_head.timestamp += 0;
        /* putchar('V'); */
    } else if (flag == PCM_TYPE_AUDIO) {
        frame_head.type = PCM_TYPE_AUDIO;
        frame_head.seq = (a_seq++);
        frame_head.timestamp += 0;
        /* putchar('A'); */
    }



    frame_head.frm_sz = len;
    do {
        if (cinfo->streamtask_kill) {
            return -1;

        }


        ret = avSendFrameData(cinfo->streamindex, data, len, &frame_head, sizeof(struct frm_head));
        if (ret < 0) {
            if (ret == AV_ER_EXCEED_MAX_SIZE) {
                putchar('A');
                os_time_dly(20);
                continue;

            }
            log_e("\n\n\n tutk_send err ret=%d\n", ret);
            return ret;

        }

    } while (ret != AV_ER_NoERROR);

    return len;
}

static void xclose(void *fd)
{
    struct tutk_client_info *cinfo = (struct tutk_client_info *)fd;
    log_d("cinfo->session_id=%d\n", cinfo->session_id);
    avServStop(cinfo->streamindex);
    log_d("\n\n\nxclose\n\n\n");
}

#define VIDEO_BUF_LEN  (1*1024*1024)
#define AUDIO_BUF_LEN  (32 * 1024)

static struct server *video_out = NULL;
static char *videobuf = NULL;
static char *audiobuf = NULL;
int video0_tutk_start(void *arg, u16 width, u16 height, u8 fps, u32 rate, u32 format)
{

    struct tutk_client_info *cinfo = (struct tutk_client_info *)arg;
    /* struct server * video_out = (struct server *)cinfo->video_out; */
    int err;
    union video_req req = {0};
    struct video_text_osd text_osd;
    struct video_graph_osd graph_osd;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;

    puts("start_video_to_out\n");
    if (cinfo->stream_vaild) {

        printf("same open , fail,plase close \n ");
        return -1;
    }


    videobuf = malloc(VIDEO_BUF_LEN);
    if (!videobuf) {
        return -1;
    }

    audiobuf = malloc(AUDIO_BUF_LEN);
    if (!audiobuf) {
        free(audiobuf);
        return -1;
    }


    if (!video_out) {
        video_out = server_open("video_server", "video0.1");
        if (!video_out) {
            free(videobuf);
            free(audiobuf);
            return VREC_ERR_V0_SERVER_OPEN;
        }

    }

    /* u32 res = db_select("res"); */
    u32 res = 0;//db_select("res");
    /*
     *通道号，分辨率，封装格式，写卡的路径
     */
    req.rec.channel     = cinfo->session_id + 1;
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.width 	    = width;//rec_pix_w[res];
    req.rec.height 	    = height;//rec_pix_h[res];
    req.rec.format 	    = format;
    req.rec.state 	    = VIDEO_STATE_START;

    /*
     *帧率为0表示使用摄像头的帧率
     */
    req.rec.quality     = VIDEO_MID_Q;
    /* req.rec.fps 	    = 0; */
    req.rec.fps 	    = fps;
    req.rec.real_fps 	= fps;

    /*
     *采样率，通道数，录像音量，音频使用的循环BUF,录不录声音
     */
    /* req.rec.audio.sample_rate = 8000; */
    /* req.rec.audio.channel 	= 1; */
    /* req.rec.audio.volume    = 63; */
    /* req.rec.audio.buf = __this->audio_buf; */
    /* req.rec.audio.buf_len = AUDIO_BUF_SIZE; */
    req.rec.audio.fmt_format =  AUDIO_FMT_PCM;
    req.rec.audio.sample_rate = 0;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = 63;
    req.rec.audio.buf = audiobuf;
    req.rec.audio.buf_len = 32 * 1024;
    req.rec.pkg_mute.aud_mute = 0;//!db_select("mic");
    req.rec.audio.aud_interval_size = 8192;  // 20ms





    /*
     *码率，I帧和P帧比例，必须是偶数（当录MOV的时候才有效）,
     *roio_xy :值表示宏块坐标， [6:0]左边x坐标 ，[14:8]右边x坐标，[22:16]上边y坐标，[30:24]下边y坐标,写0表示1个宏块有效
     * roio_ratio : 区域比例系数
     */
    /* u32 vqua = db_select("vqua"); */
    u32 kbps = 0;
    switch (width) {
    case 640:
        kbps = LOW_QUA;
        req.rec.qp_attr.static_min = 13;
        req.rec.qp_attr.static_max = 50;
        req.rec.qp_attr.dynamic_min = 7;
        req.rec.qp_attr.dynamic_max = 50;


        break;
    case 1280:
        kbps = MID_QUA;

        req.rec.qp_attr.static_min = 29;
        req.rec.qp_attr.static_max = 50;
        req.rec.qp_attr.dynamic_min = 20;
        req.rec.qp_attr.dynamic_max = 50;

        break;
    case 1920:
        kbps = HIGH_QUA;
        req.rec.qp_attr.static_min = 34;
        req.rec.qp_attr.static_max = 45;
        req.rec.qp_attr.dynamic_min = 34;
        req.rec.qp_attr.dynamic_max = 36;

        break;
    default:
        kbps = MID_QUA;
        break;

    }



    /* kbps = 96;//100 ; */
    /* kbps = 0;//100 ; */

    //外部调节 动态和静态的QP dep on CONFIG_VE_MOTION_DETECT_MODE_ISP
    req.rec.qp_attr.enable_change_qp = 1;





    req.rec.abr_kbps = kbps;// video_rec_get_abr(req.rec.width);
    printf("rec with bitrate %d kbps \r\n", kbps) ;
#if defined __CPU_AC5401__
    req.rec.IP_interval = 0;
#elif defined __CPU_AC5601__
    if (req.rec.height > 720) {
        req.rec.IP_interval = 1;
    } else {
        req.rec.IP_interval = 0;
    }
#else
    req.rec.IP_interval = 128;
#endif
    req.rec.IP_interval = 128;//96;

#if 0
    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roi1_xy = (req.rec.height * 11 / 12 / 16) << 24 | (req.rec.height * 4 / 12 / 16) << 16 | (req.rec.width * 11 / 12 / 16) << 8 | (req.rec.width) * 1 / 12 / 16;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roio_ratio1 = 256 * 90 / 100;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;
#endif

#if 1
    /*
     * osd 相关的参数，注意坐标位置，x要64对齐，y要16对齐,底下例子是根据图像大小偏移到右下
     */

#ifndef CONFIG_UI_ENABLE
    /* memcpy(video_rec_osd_buf, osd_str_buf, strlen(osd_str_buf)); */
#endif

    printf("osd_str_buf=>%s  len:%d\n", video_rec_osd_buf, strlen(osd_str_buf));
    text_osd.font_w = 16;
    text_osd.font_h = 32;
    max_one_line_strnum = strlen(video_rec_osd_buf);//21;

    osd_line_num = 2;
    /* if (db_select("num")) { */
    /* osd_line_num = 2; */
    /* } */
    osd_max_heigh = (req.rec.height == 1088) ? 1080 : req.rec.height ;
    text_osd.x = (req.rec.width - max_one_line_strnum * text_osd.font_w) / 64 * 64;
    text_osd.y = (osd_max_heigh - text_osd.font_h * osd_line_num) / 16 * 16;
    text_osd.color[0] = 0xe20095;
    text_osd.bit_mode = 1;
    /* text_osd.color[0] = 0x057d88; */
    /* text_osd.color[1] = 0xe20095; */
    /* text_osd.color[2] = 0xe20095; */
    /* text_osd.bit_mode = 2; */
    text_osd.text_format = video_rec_osd_buf;
    text_osd.font_matrix_table = osd_str_total;
    text_osd.font_matrix_base = osd_str_matrix;
    text_osd.font_matrix_len = sizeof(osd_str_matrix);
    /* text_osd.font_matrix_base = osd2_str_matrix; */
    /* text_osd.font_matrix_len = sizeof(osd2_str_matrix); */
#if (defined __CPU_DV15__ || defined __CPU_DV17__ )
    text_osd.direction = 1;

    graph_osd.bit_mode = 16;//2bit的osd需要配置3个color
    graph_osd.x = 0;
    graph_osd.y = 0;
    graph_osd.width = 256;
    graph_osd.height = 256;
    graph_osd.icon = icon_16bit_data;//icon_osd_buf;
    graph_osd.icon_size = sizeof(icon_16bit_data);//sizeof(icon_osd_buf);
#else
    text_osd.direction = 0;
#endif

    /* 编码参数配置，请仔细参阅说明文档后修改 */
    struct app_enc_info app_avc_cfg;
    app_avc_cfg.pre_pskip_limit_flag = 1; 				//SKIP块提前判断限制开关
    app_avc_cfg.pre_pskip_th = 0;						//SKIP块提前判断阈值
    app_avc_cfg.common_pskip_limit_flag = 0; 			//SKIP块常规判断限制开关
    app_avc_cfg.common_pskip_th = 0;					//SKIP块常规判断阈值
    app_avc_cfg.dsw_size = 1;							//搜索窗口大小（0:3x7  1:5x7）
    app_avc_cfg.fs_x_points = 44;						//水平搜索点数
    app_avc_cfg.fs_y_points = 24;						//垂直搜索点数
    app_avc_cfg.f_aq_strength = 1.0f;					//宏块QP自适应强度
    app_avc_cfg.qp_offset_low = -2;						//宏块QP偏移下限
    app_avc_cfg.qp_offset_high = 6;						//宏块QP偏移上限
    app_avc_cfg.nr_reduction = 0;						//DCT降噪开关
    app_avc_cfg.user_cfg_enable = 1;					//编码参数用户配置使能
    req.rec.app_avc_cfg = &app_avc_cfg;
    /* req.rec.app_avc_cfg = NULL; */




    /* if (db_select("dat")) { */
    if (1) {
        req.rec.text_osd = &text_osd;
#if (defined __CPU_DV15__ || defined __CPU_DV17__ )
        /* req.rec.graph_osd = &graph_osd; */
#endif
    }
#endif
    /* req.rec.text_osd = 0; */
    /* req.rec.graph_osd = 0; */

    /*
     *慢动作倍数(与延时摄影互斥,无音频); 延时录像的间隔ms(与慢动作互斥,无音频)
     */
    req.rec.slow_motion = 0;
    req.rec.tlp_time = 0;//db_select("gap");

    if (req.rec.tlp_time && (req.rec.camera_type != VIDEO_CAMERA_UVC)) {
        req.rec.real_fps = 1000 / req.rec.tlp_time;
    }
    if (req.rec.slow_motion || req.rec.tlp_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    }
    req.rec.buf = videobuf;
    req.rec.buf_len = 1 * 1024 * 1024;
#ifdef CONFIG_FILE_PREVIEW_ENABLE
    req.rec.rec_small_pic 	= 1;
#else
    req.rec.rec_small_pic 	= 0;
#endif
    /* req.rec.rec_small_pic 	= 1; */

    /*
     *循环录像时间，文件大小
     */
    /* req.rec.cycle_time = db_select("cyc"); */
    /* if (req.rec.cycle_time == 0) { */
    /* req.rec.cycle_time = 50; */
    /* } */

    /* req.rec.cycle_time = 60 ;// req.rec.cycle_time * 60; */
    //数据外引，用于网络
    req.rec.target = VIDEO_TO_OUT | VIDEO_USE_STD;
    req.rec.out.path = "tutk://192.168.1.2:8080";
    req.rec.out.arg  = arg;
    req.rec.out.open = xopen;
    req.rec.out.send = xsend;
    req.rec.out.close = xclose;

    req.rec.cycle_time = 60;// req.rec.cycle_time * 60;
    err = server_request(video_out, VIDEO_REQ_REC, &req);
    if (err != 0) {
        puts("\n\n\nstart rec err\n\n\n");

        cinfo->streamtask_kill = 1; //解决线程阻塞，导致关闭不了问题
        req.rec.channel = cinfo->session_id + 1;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(video_out, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nstop rec err 0x%x\n", err);
            return VREC_ERR_V0_REQ_STOP;
        }
        cinfo->streamtask_kill = 0; //解决线程阻塞，导致关闭不了问题



        server_close(video_out);
        video_out = NULL;
        printf("%s  %d\n", __func__, __LINE__);
        return -1;
    }

    cinfo->stream_vaild = 1;
    start_update_isp_scenes(video_out);
    return 0;
}


int video0_tutk_stop(void *arg)
{


    struct tutk_client_info *cinfo = (struct tutk_client_info *)arg;
    union video_req req;
    int err;

    log_d("video0_tutk_stop\n");

    if (!cinfo->stream_vaild) {

        log_d("video0 is not open\n");
        return 0;

    }

    cinfo->stream_vaild = 0;

    stop_update_isp_scenes();
    if (video_out) {
        cinfo->streamtask_kill = 1; //解决线程阻塞，导致关闭不了问题
        req.rec.channel = cinfo->session_id + 1;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(video_out, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nstop rec err 0x%x\n", err);
            return VREC_ERR_V0_REQ_STOP;
        }
        cinfo->streamtask_kill = 0; //解决线程阻塞，导致关闭不了问题
        free(videobuf);
        free(audiobuf);
    }


    return 0;
}


int video0_tutk_change_bits_rate(void *arg, u32 level)
{
    struct server *server = NULL;
    union video_req req = {0};
    u32 bits_rate = 0;


    /* server = type == 0 ? __this->video_mjpg : __this->video_h264; */
    server = video_out;

    switch (level) {
    case 0:
        bits_rate = LOW_QUA;
        break;
    case 1:
        bits_rate = MID_QUA;
        break;
    case 2:
        bits_rate = HIGH_QUA;
        break;
    default:
        bits_rate = HIGH_QUA;
        break;

    }

    if (server) {
        req.rec.channel = 1;//type == 0 ? 1 : 0;
        /* printf("%s bits rate : %d\n", type == 0 ? "MJPG" : "H.264", bits_rate); */
        req.rec.state = VIDEO_STATE_RESET_BITS_RATE;
        req.rec.abr_kbps = bits_rate;
        server_request(server, VIDEO_REQ_REC, &req);
        return 0;
    } else {
        return -1;
    }
}







///////////////////////////////////////////////////
//
//对讲
//
//
//
//
////////////////////////////////////////////////////
//
#if 1
#include "generic/circular_buf.h"

static char buffer[8192 + 128];
static void tutk_audio_task(void *arg)
{

    int ret = 0;
    int len = 0;

    int nResend = -1;
    unsigned int frmNo;
    int outBufSize = 0;
    int outFrmSize = 0;
    int outFrmInfoSize = 0;
    struct frm_head frameInfo;
    int ioType = 0;
    int srvType = 0;



    struct tutk_client_info *cinfo = (struct tutk_client_info *)arg;


    printf("cinfo->session_id=%d\n", cinfo->session_id);
#if 0
    cinfo->speakindex = avServStart3(cinfo->session_id
                                     , AuthCallBackFn
                                     , 10
                                     , 0
                                     , AUDIO_STREAM_CHANNEL
                                     , &nResend);
    if (cinfo->speakindex < 0) {
        return;
    }
#endif


    printf("%s   %d\n", __func__, __LINE__);
    cinfo->speakindex = avClientStart2(cinfo->session_id, "asdsadasdsa", "123213213", 20, &srvType, 0x2, &nResend);
    if (cinfo->speakindex < 0) {
        printf("%s   %d\n", __func__, __LINE__);
        return;
    }
    printf("%s   %d\n", __func__, __LINE__);

    while (1) {
        if (cinfo->speakkill) {
            break;
        }

agin:
        ret = avRecvFrameData2(cinfo->speakindex, buffer, 8192, &outBufSize, &outFrmSize, (char *)&frameInfo, sizeof(struct frm_head), &outFrmInfoSize, &frmNo);
        if (ret < 0) {
            if (ret == AV_ER_DATA_NOREADY) {
                msleep(50);
                goto agin;
            }
            return ;
        }
        /* printf("ret=%d\n", ret); */


        len = cbuf_write(&cinfo->audio_cbuf_handler, buffer, ret);
        if (len == 0) {
            putchar('B');
            cbuf_clear(&cinfo->audio_cbuf_handler);
        }
    }

    printf("end tutk_audio_task\n");


}

static u8 buf[2048];
static void audio_dec_irq_handler(void *priv, void *data, int len)
{
    int rlen = len;
    u16 data_len;
    struct tutk_client_info *cinfo = (struct tutk_client_info *)priv;

    int remain_len = cbuf_get_data_size(&cinfo->audio_cbuf_handler);
    if (remain_len < 256) {
        printf("data not enough\n");
        memset((u8 *)data, 0, len);
        return;
    }

    data_len = cbuf_read(&cinfo->audio_cbuf_handler, buf, rlen);
    if (data_len == 0) {
        memset((u8 *)data, 0, len);
    } else {
        memcpy(data, buf, data_len);
        if (data_len < rlen) {
            memset((u8 *)data + data_len, 0, len - data_len);
        }
    }
}




int tutk_speak_init(void *hdl, u32 format, u32 rate)
{
    struct tutk_client_info *cinfo = (struct tutk_client_info *)hdl;
    int err;
    u32 arg[2];
    struct audio_format f;
    int bindex = 0;
    char task_name[64];

    if (cinfo->speakvaild) {
        printf("audio is same open , plase close \n");
        return -1;

    }

    cinfo->audio_dev = dev_open("audio", (void *)AUDIO_TYPE_DEC);
    if (!cinfo->audio_dev) {
        /* avServStop(cinfo->speakindex); */
        return -1;
    }

    f.volume        = -1;
    f.channel       = 1;
    f.sample_rate   = 8000;
    f.priority      = 10;
    f.type          = 0x1;
    f.frame_len     = 1024;
    f.sample_source     = "dac";
    err = dev_ioctl(cinfo->audio_dev, AUDIOC_SET_FMT, (u32)&f);
    if (err) {
        puts("format_err\n");
        dev_close(cinfo->audio_dev);
        /* avServStop(cinfo->speakindex); */
        return -1;
    }

    cbuf_init(&cinfo->audio_cbuf_handler, cinfo->abuffer, 4 * 1024 * 4);


    arg[0] = (u32)cinfo;
    arg[1] = (u32)audio_dec_irq_handler;
    dev_ioctl(cinfo->audio_dev, IOCTL_REGISTER_IRQ_HANDLER, (u32)arg);

    sprintf(task_name, "tutk_audio_task%d", cinfo->session_id);
    thread_fork(task_name, 22, 0x1000, 0, &cinfo->speak_task_pid, tutk_audio_task, cinfo);

    dev_ioctl(cinfo->audio_dev, AUDIOC_STREAM_ON, (u32)&bindex);
    cinfo->speakvaild = 1;

    return 0;
}



int tutk_speak_uninit(void *hdl)
{

    struct tutk_client_info *cinfo = (struct tutk_client_info *)hdl;
    if (!cinfo->speakvaild) {
        return -1;
    }
    printf("%s  %d\n", __func__, __LINE__);

    /* avServStop(cinfo->speakindex); */
    avClientStop(cinfo->speakindex);
    cinfo->speakkill = 1;
    thread_kill(&cinfo->speak_task_pid, KILL_WAIT);
    cinfo->speakkill = 0;



    int bindex = 0;

    dev_ioctl(cinfo->audio_dev, AUDIOC_STREAM_OFF, (u32)&bindex);

    cbuf_clear(&cinfo->audio_cbuf_handler);

    dev_close(cinfo->audio_dev);
    cinfo->speakvaild = 0;
    return 0;
}





#endif








