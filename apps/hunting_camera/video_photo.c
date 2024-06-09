
/*************************************************************************
	> File Name: video_photo.c
	> Author:
	> Mail:
	> Created Time: Thu 16 Feb 2017 09:35:08 AM HKT
 ************************************************************************/

#include "system/includes.h"
#include "server/video_server.h"
#include "server/video_dec_server.h"
#include "server/ui_server.h"
#include "action.h"
#include "style.h"
#include "app_config.h"
#include "storage_device.h"
#include "video_photo.h"
#include "video_system.h"
#include "vrec_osd.h"
#include "user_isp_cfg.h"
#include "key_voice.h"
#include "app_database.h"
#include "asm/lcd_config.h"
//#include "isp_effect_data_.h"
#include "key_commucation.h"

#define PHOTO_MODE_UNINIT   0x0
#define PHOTO_MODE_ACTIVE   0x1
#define PHOTO_MODE_STOP     0x2
#define PHOTO_MODE_QSCAN    0x4


#define CAMERA_OFFLINE          0x0
#define CAMERA_ONLINE           0x1
#define CAMERA_BUSY             0x2
#define MAX_PHOTO_NUMBER        4096

struct photo_resolution {
    u16 width;
    u16 height;
};

static const char *photo_path[4] = {
    CAMERA0_CAP_PATH"img_***.jpg",
    CAMERA1_CAP_PATH"img_***.jpg",
    CAMERA2_CAP_PATH"img_***.jpg",
    CAMERA3_CAP_PATH"img_***.jpg",
};

struct video_photo_handle tph_handler = {
    .state = PHOTO_MODE_UNINIT,
};

static struct photo_resolution camera0_reso[] = {
    //  {640,  480},  //                VGA
    //  {1280, 720},  // {1024, 768}    1M
    {1920, 1080}, // {1600, 1200}   2M
    //  {2048, 1536}, //                3M
//   {2560, 1936}, //                4M
#if  1
    {2560, 1728}, //                5M
#else

    {1920, 1080},

#endif
    //  {3072, 1944}, //                6M
    {3456, 1952}, //                7M
    // {3264, 2432}, //    2160            8M
    // {3456, 2592}, //                9M
    //  {3648, 2736}, //                10M

    /* 新增加的分辨率，默认快速插值，
     * 用线性插值需将linear_scale置1
     * */
    // {4352, 3264}, //                14M */
    {5504, 2944}, //                   16M
    /* {5632, 3072}, //                17M */
    /* {5760, 3200}, //                18M */
    {5632, 4224}, //                24M
    {6528, 4864}, //                32M
    /* {7936, 5504}, //                43M */
    /* {9600, 5120}, //                48M */
    /* {10240, 5760}, //               58M(64M)目前最大为720P纵横扩大8倍 */

    //  {4032, 2736}, //                11M
    //  {4032, 3024}, //                12M
};

enum {
    CAMERA_RESO_VGA = 0x0,
    CAMERA_RESO_1M,
    CAMERA_RESO_2M,
    CAMERA_RESO_3M,
    CAMERA_RESO_4M,
    CAMERA_RESO_5M,
    CAMERA_RESO_6M,
    CAMERA_RESO_7M,
    CAMERA_RESO_8M,
    CAMERA_RESO_9M,
    CAMERA_RES0_10M,
    CAMERA_RESO_11M,
    CAMERA_RESO_12M,
};

#define __this  (&tph_handler)

//#define CAMERA0_CAP_PATH        CONFIG_REC_PATH"/C/1/"
//#define CAMERA1_CAP_PATH        CONFIG_REC_PATH"/C/2/"

/*
const u16 video_photo_reso_w[] = {800, 1280, 1600, 1920, 2208, 2560, 3072, 3712};
const u16 video_photo_reso_h[] = {600, 720, 912, 1088, 1536, 1936, 2208, 1952};
*/
static OS_SEM psem;
static int photo_switch_camera(void);
static int video_take_photo(struct photo_camera *camera);
static void camera_close(struct photo_camera *camera);
static int photo_camera_stop_display(void);
static int photo_camera_display(void);

struct imc_presca_ctl *video_disp_get_pctl();
extern int wbl_set_function_camera0(u32 parm);


void  set_isp_capture()
{


    isp0_ioctrl(NULL, ISP_CMD_SET_CAPTURE_MODE);

}

//设置camera 尺寸，用于动态修改imc模块的输入尺寸
void set_imc_size()
{

    void *fd = dev_open("video0.0", NULL);
    if (!fd) {
        return ;
    }
    struct camera_device_info camera_info = {0};
    camera_info.fps = 30;
    camera_info.sen_width = 1920;
    camera_info.sen_height = 1080;
    camera_info.width = 1920;
    camera_info.height = 1080;
    camera_info.real_fps = 30;
    if (fd) {
        dev_ioctl(fd, VIDIOC_SET_CAMERA_INFO, (u32)&camera_info);
        // dev_close(fd);
    }
    printf("\n set_imc_size>>>>>>>>>>>>>>>>>>>\n ");

}
void  isp_test()
{


    // printf("\n  isp_test_start>>>>>>>>>>>>>>>>>>>\n");
    set_isp_capture();
    set_imc_size();

    //  printf("\n  isp_test_end>>>>>>>>>>>>>>>>>>>\n");


    // extern isp0_abnormal();
    // isp0_abnormal();
    //    extern void  set_isp0_abnormal();
    //  set_isp0_abnormal();
    //  video0_disp_stop();
    //video0_disp_start(1920, 1280, 0, 0);


    // video_disp_stop(0);


    // video_disp_start(0, &disp_window[0][0]);
    // extern int video_rec_init();
    //  video_rec_init();



}
void video_photo_post_msg(const char *msg, ...)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_VIDEO_TPH;
        req.msg.msg = msg;
        req.msg.exdata = argptr;
        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);
#endif

}

static u32 photo_db_select(const char *table)
{
    if (__this->camera_id == 0) {
        return db_select(table);
    } else {
        if ((strcmp(table, "cyt") == 0) ||
            (strcmp(table, "pdat") == 0) ||
            (strcmp(table, "sca") == 0) ||
            (strcmp(table, "qua") == 0) ||
            (strcmp(table, "phm") == 0)) {
            return db_select(table);
        }
    }
    return 0;
}

void  switch_to_rec_mode()
{


    printf("\n switch_rec:%d\n ", db_select("mode"));

#if  0
    if (db_select("mode") == 2) {


        video_photo_post_msg("switch_rec");

    }

#else

    printf("\n tph_switch_rec_handler>>>>>>>>>>>>>\n");
    if (db_select("mode") == 2) { // 拍完照 切到录像模式 录像
        struct intent it;
        //struct application *app;

        it.name = "video_photo";
        init_intent(&it);
        it.action = ACTION_BACK;
#if  0
        start_app(&it);
#else
        start_app_async(&it, NULL, NULL);
#endif
        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_MAIN ;
#if  0
        start_app(&it);
#else
        start_app_async(&it, NULL, NULL);
#endif
    }


#endif


}
int set_camera0_reso(u8 res)
{
#ifdef CONFIG_VIDEO4_ENABLE
    for (int i = 0; i < 4; i++) {
        __this->camera[i].id = i;
        __this->camera[i].state = CAMERA_ONLINE;
        __this->camera[i].width = camera0_reso[res].width;
        __this->camera[i].height = camera0_reso[res].height;
    }
#else
    printf("\n  p_res============================%d\n", res);
    printf("\n camera0_reso[res:%d].width:%d,height:%d \n", res, camera0_reso[res].width, camera0_reso[res].height);
    __this->camera[0].width = camera0_reso[res].width;
    __this->camera[0].height = camera0_reso[res].height;
#endif

    return 0;
}


static int set_label_config(u8 camera_id, u16 image_width, u16 image_height, u32 font_color,
                            struct video_text_osd *label)
{
    if (!label) {
        return 0;
    }

    /*
     *日期标签设置
     *1920以下使用 16x32字体大小，以上使用32x64字体
     */
    label->direction = 0;
    /* if (image_width > 1920) { */
    printf("\n image_width==================%d\n", image_width);
    if (image_width > 2304) {
        label->font_w = 32;
        label->font_h = 64;
        label->text_format = label_format;
        label->font_matrix_table = (char *)label_index_tbl;
        label->font_matrix_base = (u8 *)label_font_32x64;
        label->font_matrix_len = sizeof(label_font_32x64);
    } else {
        label->font_w = 16;
        label->font_h = 32;
        label->text_format = label_format;
        label->font_matrix_table = osd_str_total;
        /* if (camera_id == 3) { */
        /* label->font_matrix_base = osd_mimc_str_matrix; */
        /* label->font_matrix_len = sizeof(osd_mimc_str_matrix); */
        /* } else { */
        label->font_matrix_base = osd_str_matrix;
        label->font_matrix_len = sizeof(osd_str_matrix);
        /* } */

    }
    /* label->osd_yuv = font_color; */
    label->color[0] = font_color;
    label->bit_mode = 1;

    label->x = (image_width / 2 - strlen(label_format) * label->font_w) / 64 * 64;
    label->y = (image_height / 2 - label->font_h - 16) / 16 * 16;
// 水印位置根据实际分辨率去调整

    if (image_width == 1920) {
        label->x = (image_width  - strlen(label_format) * label->font_w) / 64 * 64;
        label->y = (image_height  - label->font_h - 16) / 16 * 16;

    } else if (image_width == 2560) {



        label->x = (image_width / 2 - strlen(label_format) * label->font_w + 512 + 64) / 32 * 32;
        label->y = (image_height / 2 - label->font_h - 32 + 256) / 16 * 16;


    } else if (image_width == 3456) {

        label->x = (image_width / 2 - strlen(label_format) * label->font_w + 128 + 64) / 64 * 64;
        label->y = (image_height / 2 - label->font_h - 16) / 16 * 16;
    }

    /* 大尺寸拍照的时候，用四倍或八倍源yuv插值 */
    if (image_width > 4608) {
        if (image_width <= 5120) {
            label->x = (image_width / 2 - strlen(label_format) * label->font_w) / 64 * 64;
            label->y = (image_height / 2 - label->font_h - 16) / 16 * 16;
        } else {
            label->x = (image_width / 4 - strlen(label_format) * label->font_w) / 64 * 64;
            label->y = (image_height / 4 - label->font_h - 16) / 16 * 16;
        }
    }
    /* if (image_width >= 5120) { */
    /* label->x = (1280 - strlen(label_format) * label->font_w) / 64 * 64; */
    /* label->y = (720 - label->font_h - 16) / 16 * 16; */
    /* } */

    return 0;
}



/*
 *停止快速预览
 */
static void stop_quick_scan(void *p)
{
    union video_dec_req dec_req;

    if (!(__this->state & PHOTO_MODE_QSCAN)) {
        return;
    }

    /*
     *删除timeout超时
     */
    if (__this->qscan_timeout) {
        sys_timeout_del(__this->qscan_timeout);
        __this->qscan_timeout = 0;
    }

    /*
     *请求停止解码
     */
    if (__this->video_dec) {
        server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &dec_req);
    }

    __this->state &= ~PHOTO_MODE_QSCAN;

    if (!(__this->state & PHOTO_MODE_STOP)) {
        /*
         *恢复摄像头显示
         */
        photo_camera_display();
    }
    sys_key_event_enable();
    sys_touch_event_enable();

}


/*
 *快速预览功能
 */
static int photo_quick_scan(struct photo_camera *camera)
{
    int err = 0;
    int timeout;
    union video_req req = {0};
    union video_dec_req dec_req;

    memset(&dec_req, 0x0, sizeof(dec_req));

    /*
     *获取延时秒数
     */
    timeout = photo_db_select("sca") * 1000;
    log_i("scan time %d\n", timeout);
    if (timeout == 0) {
        return 0;
    }

    /*
     *获取拍摄的IMAGE
     */
    err = server_request(camera->server, VIDEO_REQ_GET_IMAGE, &req);
    if (!err) {
        photo_camera_stop_display();

        if (camera->server) {
            server_close(camera->server);
            camera->server = NULL;
        }

        if (!__this->video_dec) {
            struct video_dec_arg arg = {0};

            arg.dev_name = "video_dec";
            arg.audio_buf_size = 512;
            arg.video_buf_size = 512;
            __this->video_dec = server_open("video_dec_server", (void *)&arg);
            if (!__this->video_dec) {
                return -EFAULT;
            }
        }

        /*
         *预览参数设置
         */
        dec_req.dec.fb = "fb1";
        dec_req.dec.left = 0;
        dec_req.dec.top = 0;
        dec_req.dec.width = 0;
        dec_req.dec.height = 0;

        dec_req.dec.pctl = NULL;
        dec_req.dec.thm_first = 1;
        dec_req.dec.preview = 1;
        dec_req.dec.image.buf = req.image.buf;
        dec_req.dec.image.size = req.image.size;
        if (__this->aux_buf) {
            dec_req.dec.image.aux_buf = __this->aux_buf;
            dec_req.dec.image.aux_buf_size = IMAGE_AUX_BUF_SIZE;
        }

        /*
         *解码拍摄的IMAGE
         */
        err = server_request_async(__this->video_dec, VIDEO_REQ_DEC_IMAGE, &dec_req);

        __this->state |= PHOTO_MODE_QSCAN;
        __this->qscan_timeout = sys_timeout_add(NULL, stop_quick_scan, timeout);

    }

    return err;
}

/*
 *关闭camera
 */
static void camera_close(struct photo_camera *camera)
{
    if (camera->server) {
        server_close(camera->server);
        camera->server = NULL;
    }

    if (__this->cap_buf) {
        free(__this->cap_buf);
        __this->cap_buf = NULL;
    }

    if (__this->aux_buf) {
        free(__this->aux_buf);
        __this->aux_buf = NULL;
    }

    if (__this->thumbnails_buf) {
        free(__this->thumbnails_buf);
        __this->thumbnails_buf = NULL;
    }
}

static int storage_device_available()
{
    struct vfs_partition *part;

    if (!storage_device_ready()) {
        if (!dev_online(SDX_DEV)) {
            video_photo_post_msg("noCard");
        } else {
            video_photo_post_msg("fsErr");
        }
        return false;
    } else {
        part = fget_partition(CONFIG_ROOT_PATH);
        if (part->clust_size < 32 || (part->fs_attr & F_ATTR_RO)) {
            video_photo_post_msg("fsErr");
            return false;
        }
    }

    return true;
}

static int take_photo_num_calc(void *p)
{
    int err;
    int file_num;
    int coef;
    u32 free_space = 0;

    puts("take_photo_num_calc\n");

    if (!storage_device_available()) {

        video_photo_post_msg("reNum:num=%p", "00000");
        return -EINVAL;
    }

    err = fget_free_space(CONFIG_ROOT_PATH, &free_space);
    if (err) {
        return err;
    } else {

#ifdef __CPU_AC521x__
        /*****720P的镜头*****/
        /* coef = photo_db_select("pres") > PHOTO_RES_5M ? 96 : 36; */
        coef = photo_db_select("pres") > PHOTO_RES_5M ? 96 : 61;
#else

        /* coef = photo_db_select("pres") > PHOTO_RES_8M ? 10 : 30; */
        coef = photo_db_select("pres") > PHOTO_RES_5M ? 60 : 40;
#endif
        coef = coef / (photo_db_select("qua") + 1);
        file_num = free_space / (__this->camera[__this->camera_id].width *
                                 __this->camera[__this->camera_id].height / coef / 1024);
        printf("file_num = %d", file_num);
        if (file_num > 99999) {
            file_num = 99999;
        }
    }
    if (__this->camera_id == 0) {
        sprintf(__this->file_str, "%05d", file_num);
    } else {
#ifdef CONFIG_VIDEO1_ENABLE
        sprintf(__this->file_str, "%05d", file_num);
#elif defined CONFIG_VIDEO3_ENABLE
        sprintf(__this->file_str, "%s", "N/A");
#endif
    }

    log_d("take_photo_num_calc: %s\n", __this->file_str);

    video_photo_post_msg("reNum:num=%p", __this->file_str);

    return 0;
}

void thread_sdstorage()
{


    while (1) {
        os_sem_pend(&psem, 0);


        os_time_dly(1);






    }




}

int  check_tf_space()
{
    int err;
    u32 cur_space;
    u32 need_space = 0;
    err = fget_free_space(CONFIG_ROOT_PATH, &cur_space);
    if (err) {


        printf("\n get space errrrrrrrrrrrr \n");
        return -1;
    }

    printf("space: %d MB \n", cur_space / 1024);  //29713MB
    if (cur_space  < (250 * 1024)) {

        printf("\n  not  space return  -2 \n");
        return -2;
    }

    return 1;

}

static u8 save_flasg = 0;
FILE *pir_file = NULL;
static  u8  enter_workflag = 0; //  0: 初始状态 1:工作状态  2: 关机档位

int  set_workmode_bit(u8 flag)
{

    enter_workflag = flag;

}

int  get_workmode_bit()
{



    return   enter_workflag;

}
static int photo_quick_save(struct photo_camera *camera)
{
    int err = 0;
    struct vfs_partition *part;
    union video_req req;

#if   0
    thread_fork("thread_sdstorage", 10, 100, 64, NULL, thread_sdstorage, NULL);
#endif
    /*
     *获取拍摄的IMAGE
     */
    if (!storage_device_ready()) { //挂在卡

        if (get_menu_state() && (lcd_disp_busy_state())) {
            if (!dev_online(SDX_DEV)) {
                video_photo_post_msg("noCard");
            } else {
                video_photo_post_msg("fsErr");
            }
        }

        return -ENODEV;
    } else {
        part = fget_partition(CONFIG_ROOT_PATH);
        if (part->clust_size < 32 || (part->fs_attr & F_ATTR_RO)) {

            if (get_menu_state() && (lcd_disp_busy_state())) {
                video_photo_post_msg("fsErr");
            }

            return -ENODEV;
        }
    }

    err = server_request(camera->server, VIDEO_REQ_GET_IMAGE, &req);
    printf("\n <<<<<<<<<<<<<<<<<<<zzzzzzzzz_OK>>>>>>>>>>>>>>>>>>>>>len:%d KB \n", req.image.size / 1024);
    if (!err) {
#if 1
        // 判断TF 卡容量是否足够
        if (check_tf_space() != 1) {

            printf(" TF SPACE ERR   \n");
            return -ENODEV ;
        }
#endif
        //开启延时拍照功能 走视频合成
        if (db_select("delay_pv") && switch_work_on_state()) {
            char name[64] = {0};




            printf("\n convert >>>>>>>>>>>>>>>>>>>>>\n");
            struct vfscan *hfs = NULL;
            hfs = fscan(CONFIG_DEC_PATH_1, "-tAVI -sn");
            pir_file = fselect(hfs, FSEL_LAST_FILE, 0);


            if (!pir_file) {


                printf("\n not avi file \n");
                return err;
            }

            int file_size = flen(pir_file);
            printf("\n file_size=================%dMB, %dKB,set_big_filelen=%d KB\n", file_size / 1024 / 1024, file_size / 1024, 1024 * 1024);
            // 判断合成视频文件大小，超过1G则重新创建文件
            if ((file_size / 1024) > (1024 * 1024)) {


                fscan_release(hfs);
                fclose(pir_file);
                hfs = NULL;
                pir_file = NULL;

                db_update("key_state", 1); // 更新延时拍照另创建文件标志
                db_flush();
                printf("\n  conver avi file is big end \n");

                return -ENODEV;
            }


            int flen = fget_name(pir_file, name, sizeof(name));
            printf("\n get_last_file_name:%s,flen:%d\n ", name, flen);

#if  1
            char lname[64] = {0};
            FILE *fp_avi;
            // int flen=fget_name(file_avi,name,sizeof(name));
            //printf("\n get_combine_file_name:%s,flen:%d\n ",name,flen);
            sprintf(lname, "%s%s", CONFIG_REC_PATH_0, name);
            printf("\n lname::::::%s\n", lname);
            fp_avi = fopen(lname, "r+"); /* 打开待修改的AVI  固定文件测试用*/
#endif

            int jpg_frames_insert_to_avi_demo(char *buf, int len, FILE * fp_avi) ;
            jpg_frames_insert_to_avi_demo(req.image.buf, req.image.size, fp_avi);

            fscan_release(hfs);
            fclose(pir_file);
            hfs = NULL;
            pir_file = NULL;


        } else {
            //照片存卡方式

            FILE *fp = NULL;
            fp = fopen(camera->id == 0 ? CAMERA0_CAP_PATH"img_****.jpg" : CAMERA1_CAP_PATH"img_****.jpg", "w+");

            if (!fp) {

                printf("\n not open picture \n");
                return err;
            }
            if (switch_work_on_state()) {
                set_workmode_bit(1);
            }
            fwrite(fp, req.image.buf, req.image.size);
            fclose(fp);
            printf("\n fclose_end \n");
            fp = NULL;
        }



    }
#ifdef  CONFIG_IO_MEASURE_TIMER_ENABLE

    set_io_measure_state(1);
#endif
    return err;
}

#ifdef STORAGE_PICTURE_BUFFER


extern int test;

void  test_save()
{

    printf("\n test_save>>>>>>>>>>>>>>>>>>>>>>>>>\n ");
}
#define PCM_TYPE_AUDIO      1
#define JPEG_TYPE_VIDEO     2
#define H264_TYPE_VIDEO     3
#define PREVIEW_TYPE        4
#define DATE_TIME_TYPE      5
#define MEDIA_INFO_TYPE  	6
#define PLAY_OVER_TYPE      7
#define GPS_INFO_TYPE	    8
#define NO_GPS_DATA_TYPE	9
#define G729_TYPE_AUDIO    10
#define META_DATA          11

typedef struct media_info {
    int type;
    u32 len;
    u32 difftime;
    u8 is_key_frame;
    u8 data[0];
} MEDIA_INFO;
static struct lbuff_head *lbuf_handle = NULL;
static OS_SEM cloud_storage_sem;
#if SDRAM_SIZE == (64 * 1024 * 1024)

#define CLOUD_STORAGE_LBUF_SIZE   (4 * 1024 * 1024)

#else

#define CLOUD_STORAGE_LBUF_SIZE   (2 * 1024 * 1024)

#endif
static u8 media_buf[CLOUD_STORAGE_LBUF_SIZE];


int storage_video_write(int type, u8 is_key_frame, int difftime, char *buffer, int len)
{
    MEDIA_INFO  *lbuf = NULL;

    printf("\n lbuf_free_space(lbuf_handle) :%d,len:%d\n", lbuf_free_space(lbuf_handle), len);
    if (lbuf_free_space(lbuf_handle) < len) {
        printf("\n>>>>>> The data cache area is full, ending the current cloud storage \n");

        return 0;
    }
    lbuf = (MEDIA_INFO *)lbuf_alloc(lbuf_handle, len);  //lbuf内申请一块空间
    if (lbuf) {
        lbuf->type = type;
        lbuf->is_key_frame = is_key_frame;
        lbuf->difftime = difftime;
        lbuf-> len = len;
        memcpy(&lbuf->data[0], buffer, len);
        lbuf_push(lbuf_handle, lbuf, BIT(0)); //把数据块推送更新到lbuf的通道0
    }
    return len;
}

void photo_save_test(int id, char *buf, int len)
{
    int err = 0;
    if (!err) {
        FILE *fp = NULL;
        fp = fopen(id == 0 ? CAMERA0_CAP_PATH"img_***.jpg" : CAMERA1_CAP_PATH"img_***.jpg", "w+");

        if (fp) {
            fwrite(fp, buf, len);
            fclose(fp);
        } else {
            log_e("open file err\n");
        }
    }



}
static  int tf_is_ok = 0;
void ready_flag()
{

    printf("\n tf_is_ready>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
    tf_is_ok = 1;


}
void video_storage_task(void *priv)
{
    const char *file_name = "./xxx/cloud_file";//?éò??áD′μ????t??
    int64_t ret = 0;
    MEDIA_INFO *rbuf = NULL;
    int  rc = 0;

    while (1) {

        ret = os_sem_pend(&cloud_storage_sem, 0);


        wait_completion(storage_device_ready, ready_flag, 1);
        os_time_dly(300);
        printf("\n video_storage_task>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%d \n ", ret);
        if (ret != OS_NO_ERR) {
            msleep(10);
            continue;
        }



        if (ret < 0) {


        } else {


            if (!rc) {


                do {

                    if (lbuf_empty(lbuf_handle)) { //查询LBUF内是否有数据帧
                        //                extern void vPortYield(void);
                        //                vPortYield();
                        msleep(10);


                        continue;
                    }
#if  0
                    if (!tf_is_ok) {

                        msleep(10);
                        printf("\n <<wait_sd>>\n");
                        continue;
                    }

#endif

                    rbuf = (MEDIA_INFO *)lbuf_pop(lbuf_handle, BIT(0));

                    if (rbuf) {
                        printf("\n rbuf->data[0]>>>>>>>>>>>>>>>>>>>>>%x,%x,%x\n", rbuf->data[0], rbuf->data[1], rbuf->data[2]);
                        if (rbuf->type == JPEG_TYPE_VIDEO) {

                            photo_save_test(0, &rbuf->data[0], rbuf->len) ; // &rbuf->data[0],// rbuf->len
                            os_time_dly(5);

                            //put_buf(&rbuf->data[0],64);
                            if (ret == -3 || ret == -2 || ret == -6) {
                                printf("\n %s %d ret = %d\n", __func__, __LINE__, ret);
                            }


                        } else {

                            if (ret == -3 || ret == -2 || ret == -6) {
                                printf("\n %s %d ret = %d\n", __func__, __LINE__, ret);
                            }
                        }


                    }
                } while (1);


            }

        }


    }
}

int  photo_storage_init()
{

    os_sem_create(&cloud_storage_sem, 0);
    lbuf_handle = lbuf_init(media_buf, CLOUD_STORAGE_LBUF_SIZE, 4, sizeof(MEDIA_INFO));
    printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>> %s %d %d\n", __func__, __LINE__, lbuf_free_space(lbuf_handle));
    return thread_fork("video_storage_task", 10, 0x1000, 0, 0, video_storage_task, NULL);
}

void  get_picture(void *camera, char *buf,  int *len)
{


    int err = 0;

    union video_req req;

    if (camera) {
        err = server_request(camera, VIDEO_REQ_GET_IMAGE, &req);
//   put_buf(req.image.buf,20);

        storage_video_write(JPEG_TYPE_VIDEO, 1, 100,  req.image.buf,  req.image.size);
        //printf("\n  camera==================%d,%d\n",camera,req.image.size);




    }







}


#endif

char *pdata[5] = {NULL};
void save_photo_buf(struct photo_camera *camera)
{


    union video_req req = {0};
    int err = server_request(camera->server, VIDEO_REQ_GET_IMAGE, &req);
    if (!err) {

        // FILE *fp = NULL;
        // fp = fopen(camera->id == 0 ? CAMERA0_CAP_PATH"img_***.jpg" : CAMERA1_CAP_PATH"img_***.jpg", "w+");
        // if (fp) {

        //fwrite(fp, req.image.buf, req.image.size);

        *pdata[0] = malloc(req.image.size);

        memcpy(&pdata[0], req.image.buf, req.image.size);

        put_buf(&pdata[0], 16);

    }

}

/*

目前小于14M分辨率可采样线性插值方式 清晰度高，插值时 src取的1080
宽高需要满足如下公式

h_num=src_h/ ( dst_h/16 )


h_num 需要为偶数, 否则异常
src_h 镜头原始高度1080
dst_h 目标分辨率的高度

注意拍照分辨率小于14M时需要采样线性插值 所需要时间少， 如下
   if(camera->width<=4608){

    req.icap.linear_scale = 1;	// 线性插值选项
    }else{

    req.icap.linear_scale = 0;	// 非线性插值选项
    }

*/

static int video_take_photo(struct photo_camera *camera)
{
    int err = 0;
    union video_req req = {0};
    struct icap_auxiliary_mem aux_mem;
    struct jpg_thumbnail thumbnails;
    static struct video_source_crop crop = {0};
    char video_name[16];
    int i = 1;

#ifdef  STORAGE_PICTURE_BUFFER
    photo_storage_init();
#endif
#if 0//ndef CONFIG_FAST_CAPTURE
    if (!storage_device_available()) {
        return -EINVAL;
    }
#else
    //   void ispt_set_block_frame_count(u32 c);
    //  ispt_set_block_frame_count(6);
#endif


    if (!camera || (camera->state != CAMERA_ONLINE)) {
        return -EINVAL;
    }

    /*
     *打开相机对应的video服务
     */
    /* #ifdef CONFIG_VIDEO1_ENABLE */
    /* sprintf(video_name, "video%d", __this->camera_id); */
    /* #elif (defined CONFIG_VIDEO2_ENABLE) */
    /* sprintf(video_name, "video%d", (__this->camera_id == 1) ? 3 : 0); */
    /* #endif */
#ifdef CONFIG_VIDEO4_ENABLE
    sprintf(video_name, "video4.%d.0", __this->camera_id);
#else
    sprintf(video_name, "video%d.0", __this->camera_id);
#endif

    if (!camera->server) {
        camera->server = server_open("video_server", video_name);
    } else {
        if ((camera->prev_width != camera->width) || (camera->prev_height != camera->height)) {
            server_close(camera->server);
            /******一般尺寸拍照去掉2个buffer,节省动态空间*****/
            if (camera->width < 1920) {
                /* if (__this->aux_buf) { */
                /* free(__this->aux_buf);	 */
                /* __this->aux_buf = NULL; */
                /* } */

                /*
                if (__this->thumbnails_buf) {
                    free(__this->thumbnails_buf);
                    __this->thumbnails_buf = NULL;
                }
                */
            }
            camera->server = server_open("video_server", video_name);
        }
    }
    // printf("\n pre_width:%d,pre_height:%d,camera->width:%d, camera->height:%d\n ",camera->prev_width, camera->prev_height,camera->width,camera->height);


    //camera->width=1920;
    //camera->height=1080;

    camera->prev_width = camera->width;
    camera->prev_height = camera->height;





    if (!camera->server) {
        return -EFAULT;
    }

    /*
     *设置拍照所需要的buffer
     */
    if (!__this->cap_buf) {
        __this->cap_buf = (u8 *)malloc(CAMERA_CAP_BUF_SIZE);
    }

    if (camera->width > 1920 && !__this->aux_buf) {
        /*
         *设置尺寸较大时缩放所需要的buffer
         */
        __this->aux_buf = (u8 *)malloc(IMAGE_AUX_BUF_SIZE);
        /*__this->thumbnails_buf = (u8 *)malloc(IMAGE_THUMB_BUF_SIZE);*/
        if (!__this->aux_buf/* || !__this->thumbnails_buf*/) {
            err = -ENOMEM;
            goto __err;
        }
    }

    if (!__this->cap_buf) {
        err = -ENOMEM;
        goto __err;
    }

    aux_mem.addr = __this->aux_buf;
    aux_mem.size = IMAGE_AUX_BUF_SIZE;

    /*
     *配置拍照服务参数
     *尺寸、buffer、质量、日期标签
     */

    req.icap.exif = NULL;
    req.icap.exif_size = 0;
    req.icap.width = camera->width;
    req.icap.height = camera->height;
    printf("\n req.icap.width:%d,req.icap.height:%d\n ", req.icap.width, req.icap.height);
    req.icap.quality = photo_db_select("qua");

#if 1//def CONFIG_FAST_CAPTURE
    req.icap.path = NULL;
#else
    req.icap.path = photo_path[camera->id];
#endif
    /* if (camera->id == 0) { */
    /* req.icap.path = CAMERA0_CAP_PATH"img_***.jpg"; */
    /* } else if (camera->id == 1) { */
    /* req.icap.path = CAMERA1_CAP_PATH"img_***.jpg"; */
    /* } else { */
    /* req.icap.path = CAMERA2_CAP_PATH"img_***.jpg"; */
    /* } */

    /*req.icap.path = camera->id == 0 ? CAMERA0_CAP_PATH"img_***.jpg" : CAMERA1_CAP_PATH"img_***.jpg";*/
    req.icap.buf = __this->cap_buf;
    req.icap.buf_size = CAMERA_CAP_BUF_SIZE;
    req.icap.aux_mem = &aux_mem;
    req.icap.camera_type = VIDEO_CAMERA_NORMAL;
#ifdef JPG_THUMBNAILS_ENABLE
    if (camera->width > 1920) {
        thumbnails.enable = 1;
        thumbnails.quality = 10;
        thumbnails.width = 480;
        thumbnails.height = 320;
        /**将缩略图buffer和拍照缓存复用**/
        thumbnails.buf = __this->cap_buf + CAMERA_CAP_BUF_SIZE - IMAGE_THUMB_BUF_SIZE;//thumbnails_buf;
        thumbnails.len = IMAGE_THUMB_BUF_SIZE;
        req.icap.thumbnails = &thumbnails;
    }
#endif

    if (camera->id == 3) {
        req.icap.camera_type = VIDEO_CAMERA_UVC;
        req.icap.uvc_id = __this->uvc_id;
    }

    /* if(camera->id == 1){ */
    /* req.icap.mirror = 1;	 */
    /* }else{ */
    req.icap.mirror = 0;
    req.icap.jaggy = 0;

    if (req.icap.width <= 4608) {

        req.icap.linear_scale = 1;	// 线性插值选项
    } else {

        req.icap.linear_scale = 0;	// 非线性插值选项
    }


    /* } */

    req.icap.text_label = photo_db_select("pdat") ? &__this->label : NULL;
    req.icap.src_w = camera->src_w;
    req.icap.src_h = camera->src_h;
// 水印根据具体位置去调整
    set_label_config(__this->camera_id, req.icap.width, req.icap.height, 0xe20095, req.icap.text_label);


    i = photo_db_select("cyt");
    if (db_select("delay_pv") && switch_work_on_state()) {

        i = 1;  // 只拍一张

    }


//#ifdef  CONFIG_IO_MEASURE_TIMER_ENABLE
//
//
//     set_io_measure_state(0);
//#endif
    if (get_menu_state() && (lcd_disp_busy_state())) {
        printf("\n  menu>>>>>>>>>>>>>>>>>>>\n");
        video_photo_post_msg("tphin");
    }
#if  0  // 不用
    static int flag = 0;

    if (!flag) {
        flag = 1;
        req.icap.width = 128;
        req.icap.height = 64;
        server_request(camera->server, VIDEO_REQ_IMAGE_CAPTURE, &req);

        //wait_completion(storage_device_ready, photo_quick_save, camera);

        req.icap.width = camera->width;
        req.icap.height = camera->height;
        start_update_isp_scenes(camera->server);

        // os_time_dly(50);
    }


#endif


    printf("\n picture_number.........................i:%d \n", i);
#ifdef  CONFIG_IO_MEASURE_TIMER_ENABLE


    set_io_measure_state(1);
#endif
    while (i--) {
        //printf("\n capture.........................i:%d \n",i);

        err = server_request(camera->server, VIDEO_REQ_IMAGE_CAPTURE, &req);// 1080p  560ms   , 5M  1.28s

        if (err != 0) {
            puts("take photo err.\n");
            goto __err;
        }
        // malloc_stats();

        if (get_menu_state() && (lcd_disp_busy_state())) {
            // video_photo_post_msg("tphend");
        }
        printf("\n sd will save picture\n");
#ifdef  STORAGE_PICTURE_BUFFER

        u8 *buf = calloc(1, 300 * 1024);
        if (buf) {
            int  len;
            get_picture(camera->server, buf, &len);
            printf("\n  len=======================%d\n", len);


        }
        free(buf);
#else

#ifdef  CONFIG_IO_MEASURE_TIMER_ENABLE
        set_io_measure_state(0);
#endif

        wait_completion(storage_device_ready, photo_quick_save, camera);

#endif



    }

#ifdef  STORAGE_PICTURE_BUFFER
    os_sem_post(&cloud_storage_sem);//  一次性存卡
#endif


    if (get_menu_state() && (lcd_disp_busy_state())) { // MENU 状态

        printf("\n calc:>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n ");

        take_photo_num_calc(NULL);
        video_photo_post_msg("tphout");


    }


    return 0;
__err:

    log_w("video take photo err : %d\n", err);
    camera_close(camera);

    return err;

}



static void take_photo_delay(void *p)
{
    if (__this->delay_ms) {
        video_take_photo(&__this->camera[__this->camera_id]);
        __this->delay_ms = 0;
    }
}


static int video_delay_take_photo(struct photo_camera *camera)
{
    int delay_sec;
    struct vfs_partition *part;

    if (!storage_device_available()) {
        return -EINVAL;
    }

    if (__this->delay_ms > 0) {
        return 0;
    }

    delay_sec = photo_db_select("phm");
    __this->delay_ms = delay_sec * 1000;


    if (__this->timeout) {
        sys_timeout_del(__this->timeout);
        __this->timeout = 0;
    }

    video_photo_post_msg("dlyTPH:ms=%4", __this->delay_ms);
    /*
     *设置超时函数，超过delay_ms拍照
     */
    __this->timeout = sys_timeout_add((void *)camera, take_photo_delay, __this->delay_ms);
    if (!__this->timeout) {
        return -EFAULT;
    }
    sys_key_event_disable();
    sys_touch_event_disable();

    return 0;
}

static int show_main_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        return -EINVAL;
    }
    req.show.id = ID_WINDOW_VIDEO_TPH;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
#endif
    return 0;
}


static void hide_main_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        return;
    }


    req.hide.id = ID_WINDOW_VIDEO_TPH;
    server_request(__this->ui, UI_REQ_HIDE, &req);
#endif
}

static int photo_camera_display(void)
{
    union video_req req = {0};
    char video_name[16];

#ifdef CONFIG_DISPLAY_ENABLE
    if ((get_menu_state() && (lcd_disp_busy_state())) || get_dec_to_disp_poweroff_flag()) {
        if (__this->display) {
            return 0;
        }

        if (__this->camera[__this->camera_id].state == CAMERA_ONLINE) {

            /*
             *打开对应摄像头的服务
             */
#ifdef CONFIG_VIDEO4_ENABLE
            sprintf(video_name, "video4.%d.0", __this->camera_id);
#else
            sprintf(video_name, "video%d.0", __this->camera_id);
#endif

            __this->display = server_open("video_server", video_name);

            if (!__this->display) {
                puts("server_open:faild\n");
                return -EFAULT;
            }

            /*
             *设置显示参数
             */
            memset(&req, 0x0, sizeof(req));
            req.display.fb 		= "fb1";
            req.display.left  	= 0;//dc->rect.left;
            req.display.top 	= 0;//dc->rect.top;
            req.display.state 	= VIDEO_STATE_START;
            req.display.pctl = video_disp_get_pctl();
#ifdef CONFIG_UI_STYLE_JL02_ENABLE
            /*
             * 切割尺寸
             * */
            req.display.width 	= 1280;
            // 拍照APP启动导致复位
            req.display.height 	= 0;//720;// height;
            /* req.display.border_left  = 0; */
            /* req.display.border_top   = (req.display.height - 480) / 2 / 16 * 16; // 0; */
            /* req.display.border_right = 0; */
            /* req.display.border_bottom = (req.display.height - 480) / 2 / 16 * 16; // 0; */
#elif (defined CONFIG_UI_STYLE_LY_LONGSCREEN_ENABLE)
            req.display.width 	= LCD_DEV_WIDTH;//dc->rect.width;
            req.display.height 	= LCD_DEV_HIGHT;//dc->rect.height;
            req.display.border_left = 0;
            req.display.border_top  = 0;
            req.display.border_right = 0;
            req.display.border_bottom = 0;
#else
            req.display.width 	= LCD_DEV_WIDTH;//dc->rect.width;
            req.display.height 	= LCD_DEV_HIGHT;//dc->rect.height;
            /* req.display.width 	= 0;//dc->rect.width; */
            /* req.display.height 	= 0;//dc->rect.height; */
#endif

            req.display.camera_type = VIDEO_CAMERA_NORMAL;
#ifdef CONFIG_VIDEO3_ENABLE
            if (__this->camera_id == 3) {
                req.display.camera_type = VIDEO_CAMERA_UVC;
                req.display.uvc_id = __this->uvc_id;
            }
#endif
            req.display.src_w = __this->camera[__this->camera_id].src_w;
            req.display.src_h = __this->camera[__this->camera_id].src_h;


            /* if(__this->camera_id == 1){ */
            /* req.display.mirror = 1;	 */
            /* }else{ */
            req.display.mirror = 0;
            req.display.jaggy = 0;
            /* } */

            server_request(__this->display, VIDEO_REQ_DISPLAY, &req);
            if (__this->camera_id == 0) {
#ifndef CONFIG_VIDEO4_ENABLE
#if  0
                int color = photo_db_select("col");
                if (color != PHOTO_COLOR_NORMAL) {
                    set_isp_special_effect(__this->display, color);
                } else {
                    start_update_isp_scenes(__this->display);
                }

#endif
#endif
            }
            /*
             *设置当前模式下摄像头的参数 : 曝光补偿/白平衡/锐化
             */
#if  0
            if (__this->camera_id == 0) {
#ifndef CONFIG_VIDEO4_ENABLE
                set_camera_config(&__this->camera[__this->camera_id]);
#endif
            }

#endif
        }
    }
#endif
    return 0;
}

static int photo_camera_stop_display(void)
{
    union video_req req = {0};

    if (__this->display) {
        /*
        req.camera.mode = ISP_MODE_REC;
        req.camera.cmd = SET_CAMERA_MODE;
        server_request(__this->display, VIDEO_REQ_CAMERA_EFFECT, &req);
        */
        if (__this->camera_id == 0) {
            //    stop_update_isp_scenes();
        }

        req.display.state = VIDEO_STATE_STOP;

        server_request(__this->display, VIDEO_REQ_DISPLAY, &req);
        server_close(__this->display);
        __this->display = NULL;

    }

    return 0;
}


static int video_photo_start(void)
{
    int err;
    int res = db_select("pres");
// 开启定时拍照走视频合成固 1080P
    if (db_select("delay_pv")) {

        res = 2;

    }

    switch (res) {
    case 2:

        res = 0;
        break ;



    case 5:

        res = 1;
        break ;


    case 8:
        res = 2;

        break ;


    case 16:

        res = 3;

        break ;


    case 24:


        res = 4;

        break ;

    case  36:


        res = 5;


        break ;

    default:

        res = 0;
        break;

    }
    bool online;
    /*
     *相机初始化，后拉默认VGA分辨率
     */
#ifdef CONFIG_VIDEO0_ENABLE
    __this->camera[0].id = 0;
    __this->camera[0].state = CAMERA_ONLINE;
    __this->camera[0].width = camera0_reso[res].width;
    __this->camera[0].height = camera0_reso[res].height;
    printf("\n __this->camera[0].width:%d,__this->camera[0].height:%d\n ", __this->camera[0].width, __this->camera[0].height);
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    online = dev_online("video1.*");
    __this->camera[1].id = 1;
    __this->camera[1].state = online ? CAMERA_ONLINE : CAMERA_OFFLINE;
#endif
#ifdef CONFIG_VIDEO3_ENABLE
    online = dev_online("uvc");
    __this->camera[3].id = 3;
    __this->camera[3].state = online ? CAMERA_ONLINE : CAMERA_OFFLINE;
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    __this->camera[1].width = 640;
    __this->camera[1].height = 480;
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    __this->camera[3].width = 1280;//640;
    __this->camera[3].height = 720;//480;
#endif


#ifdef CONFIG_VIDEO4_ENABLE
    for (int i = 0; i < 4; i++) {
        __this->camera[i].id = i;
        __this->camera[i].state = CAMERA_ONLINE;
        __this->camera[i].width = camera0_reso[res].width;
        __this->camera[i].height = camera0_reso[res].height;
    }
#endif


    __this->camera_id = 0;
    puts("video_photo start.\n");

    err = photo_camera_display();
    if (err) {


        printf("\n  err????????????????\n");
        return err;
    }

    if (get_menu_state() && (lcd_disp_busy_state())) {
        show_main_ui();
    }

    if (get_menu_state() && (lcd_disp_busy_state())) { // 开机进入菜单不自动进行拍照，需要手动
        __this->sd_wait = wait_completion(storage_device_ready, take_photo_num_calc, NULL);
    }
    __this->state = PHOTO_MODE_ACTIVE;

#ifdef  CONFIG_IO_MEASURE_TIMER_ENABLE

    // set_io_measure_state(1);
#endif
#if 1


    if (get_menu_state() && (switch_work_on_state())) { //工作模式进入拍照


        video_take_photo(&__this->camera[__this->camera_id]);

    }
#endif

    return 0;
}

static int photo_disp_start(void)
{
    int err;
    int res = db_select("pres");



    bool online;
    /*
     *相机初始化，后拉默认VGA分辨率
     */
//   res=get_table_photo_res();//  获取分辨率

    switch (res) {

    case 5:

        res = 0;

        break ;


    case 8:
        res = 1;

        break ;


    case 16:

        res = 2;

        break ;


    case 24:


        res = 3;

        break ;

    case  36:


        res = 4;


        break ;

    default:

        res = 0;
        break;

    }

#ifdef CONFIG_VIDEO0_ENABLE
    __this->camera[0].id = 0;
    __this->camera[0].state = CAMERA_ONLINE;
    __this->camera[0].width = camera0_reso[res].width;
    __this->camera[0].height = camera0_reso[res].height;


    printf("\n __this->camera[0].width:%d,__this->camera[0].height:%d\n ", __this->camera[0].width, __this->camera[0].height);


#endif

#ifdef CONFIG_VIDEO1_ENABLE
    online = dev_online("video1.*");
    __this->camera[1].id = 1;
    __this->camera[1].state = online ? CAMERA_ONLINE : CAMERA_OFFLINE;
#endif
#ifdef CONFIG_VIDEO3_ENABLE
    online = dev_online("uvc");
    __this->camera[3].id = 3;
    __this->camera[3].state = online ? CAMERA_ONLINE : CAMERA_OFFLINE;
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    __this->camera[1].width = 640;
    __this->camera[1].height = 480;
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    __this->camera[3].width = 1280;//640;
    __this->camera[3].height = 720;//480;
#endif


#ifdef CONFIG_VIDEO4_ENABLE
    for (int i = 0; i < 4; i++) {
        __this->camera[i].id = i;
        __this->camera[i].state = CAMERA_ONLINE;
        __this->camera[i].width = camera0_reso[res].width;
        __this->camera[i].height = camera0_reso[res].height;
    }
#endif


    __this->camera_id = 0;
    puts("video_photo start.\n");
    if ((get_menu_state() && (lcd_disp_busy_state())) || get_dec_to_disp_poweroff_flag()) {

        err = photo_camera_display();
        if (err) {
            return err;
        }
    }
    if ((get_menu_state() && (lcd_disp_busy_state())) || get_dec_to_disp_poweroff_flag()) {

        show_main_ui();
    }

    if (lcd_disp_busy_state() || get_dec_to_disp_poweroff_flag()) { // 开机进入菜单不自动进行拍照，需要手动


        printf("\n  photo_disp_start>>>>>>>>>>>>>>>>\n");

        __this->sd_wait = wait_completion(storage_device_ready, take_photo_num_calc, NULL);


    }
    __this->state = PHOTO_MODE_ACTIVE;



    return 0;
}

static int video_photo_stop(void)
{
    __this->state |= PHOTO_MODE_STOP;

    /*
     *停止快速预览
     */
    stop_quick_scan(NULL);
    /*
     *关闭相机0，相机1，关闭显示
     */
    camera_close(&__this->camera[0]);
    camera_close(&__this->camera[1]);
    camera_close(&__this->camera[2]);
    camera_close(&__this->camera[3]);

    photo_camera_stop_display();

    if (__this->video_dec) {
        server_close(__this->video_dec);
        __this->video_dec = NULL;
    }

    if (__this->timeout) {
        __this->delay_ms = 0;
        sys_timeout_del(__this->timeout);
        __this->timeout = 0;
        //防止拍照计时插USB，按键和touch失效
    }
    if (get_menu_state() && (lcd_disp_busy_state())) {
        hide_main_ui();
    }
    return 0;
}



static void photo_mode_init(void)
{
    if (__this->state == PHOTO_MODE_UNINIT) {
        memset(__this, 0, sizeof(*__this));
    }

    if (!__this->cap_buf) {
        __this->cap_buf = (u8 *)malloc(CAMERA_CAP_BUF_SIZE);
    }
    if (!__this->aux_buf) {
        __this->aux_buf = (u8 *)malloc(IMAGE_AUX_BUF_SIZE);
    }

    __this->state = PHOTO_MODE_ACTIVE;
}

/*
 * 摄像头切换
 */
static int photo_switch_camera(void)
{
    int err;
    u8 switch_id;

    switch_id = __this->camera_id;
    do {
        if (++switch_id >= PHOTO_CAMERA_NUM) {
            switch_id = 0;
        }

        if (switch_id == __this->camera_id) {
            return 0;
        }

        if (__this->camera[switch_id].state == CAMERA_ONLINE) {
            break;
        }

    } while (switch_id != __this->camera_id);

    stop_quick_scan(NULL);
    camera_close(&__this->camera[__this->camera_id]);
    //video_photo_cfg_reset();
    photo_camera_stop_display();

    __this->camera_id = switch_id;

    take_photo_num_calc(NULL);

    err = photo_camera_display();
    if (err) {
        return err;
    }
    printf("camera_id = %d\n",  __this->camera_id);
    video_photo_post_msg("swCAM:id=%1", __this->camera_id);

    return 0;
}

static int video_photo_change_status(struct intent *it)
{
    int err;
    u32 free_space = 0;
    int file_num;
    int coef;
#ifdef CONFIG_UI_ENABLE
    if (get_menu_state() && (lcd_disp_busy_state())) {
        struct key_event org = { KEY_EVENT_CLICK, KEY_OK };
        struct key_event new = { KEY_EVENT_CLICK, KEY_PHOTO };

        if (!strcmp(it->data, "opMENU:")) { /* ui要求打开rec菜单 */
            puts("ui ask me to opMENU:.\n");
            if (1) { /* 允许ui打开菜单 */
                it->data = "opMENU:en";
                sys_key_event_unmap(&org, &new);
            } else { /* 禁止ui打开菜单 */

                it->data = "opMENU:dis";
            }

        } else if (!strcmp(it->data, "exitMENU")) { /* ui已经关闭rec菜单 */
            sys_key_event_map(&org, &new);
            puts("ui tell me exitMENU.\n");
            video_photo_restore();
            take_photo_num_calc(NULL);
        } else if (!strcmp(it->data, "idCAM:")) { /* 获取摄像头id */
            if (__this->camera_id == 0) {
                it->data = "idCAM:0";
            } else {
                it->data = "idCAM:1";
            }
        } else {
            puts("unknow status ask by ui.\n");
        }
    }
#endif
    return 0;
}

static int video_photo_change_source_reso(int dev_id, u16 width, u16 height)
{
    __this->camera[dev_id].src_w = width;
    __this->camera[dev_id].src_h = height;
    if (__this->camera[__this->camera_id].state == CAMERA_ONLINE) {
        log_d("video%d.* change source reso to %d x %d\n", dev_id, width, height);
        if (__this->camera_id == dev_id) {
            if ((__this->state & PHOTO_MODE_ACTIVE) &&
                !(__this->state & PHOTO_MODE_STOP) &&
                !(__this->state & PHOTO_MODE_QSCAN)) {
                photo_camera_stop_display();
                if (__this->timeout) {
                    sys_timeout_del(__this->timeout);
                }
                camera_close(&__this->camera[__this->camera_id]);
                //video2.* photo taking and display must be closed before source reso changing
                photo_camera_display();
                if (__this->timeout) {
                    __this->timeout = sys_timeout_add((void *)&__this->camera[__this->camera_id], take_photo_delay, __this->delay_ms);
                }
            }
        }
    }
    return 0;
}


static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int len;
    struct intent mit;
    struct key_event org = { KEY_EVENT_CLICK, KEY_OK };
    struct key_event new = { KEY_EVENT_CLICK, KEY_PHOTO };


    printf("\n  state:%d\n", state);
    switch (state) {
    case APP_STA_CREATE:
        photo_mode_init();
        server_load(video_server);



#ifdef CONFIG_UI_ENABLE
        if ((get_menu_state() && (lcd_disp_busy_state())) || get_dec_to_disp_poweroff_flag()) {
            printf("\n open ui server \n");
            __this->ui = server_open("ui_server", NULL);
            if (!__this->ui) {
                return -EINVAL;
            }
        }
#endif
        sys_key_event_map(&org, &new);
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        printf("\n photo_it->action:%x\n ", it->action);
        switch (it->action) {
        case ACTION_PHOTO_TAKE_MAIN:

            printf("\n work is run\n");
            if (switch_work_on_state()) {
                set_workmode_bit(1);// 设置工作完关机标志
            }
            video_photo_start();

            if (db_select("delay_pv")) {

                printf("\n  mersh photo to video end poweroff\n");
                auto_power_down();

            } else if (db_select("mode") == 0 && (switch_work_on_state())) {


                printf("\n  take photo end to poweroff\n");
                auto_power_down();




            } else if (db_select("mode") == 2 && (switch_work_on_state())) { //  拍完照片进入rec 模式， 可优化



#if  1

                printf("\n <<<<<<<<<<<<<<<<<switch to rec>>>>>>>>>>>>>\n ");
                sys_hi_timeout_add(NULL, switch_to_rec_mode, 200);
#endif

            }

            malloc_stats();
            break;
        case   ACTION_PHOTO_DISP_MAIN:

            printf("\n only disp\n");

            if (get_menu_state() || get_dec_to_disp_poweroff_flag()) {
                photo_disp_start();
            }


            malloc_stats();
            break;
        case ACTION_PHOTO_TAKE_SET_CONFIG:
            if (!__this->ui) {
                return -EINVAL;
            }
            video_photo_set_config(it);
            db_flush();
            if (it->data && !strcmp(it->data, "pres")) {

                printf("\n ACTION_PHOTO_TAKE_SET_CONFIG>>>>>>>>>>>>>>>>>\n ");
                take_photo_num_calc(NULL);
            }
            break;
        case ACTION_PHOTO_TAKE_CHANGE_STATUS:
            video_photo_change_status(it);
            break;
#ifdef CONFIG_UI_STYLE_JL02_ENABLE
        case ACTION_PHOTO_TAKE_CONTROL:
            puts("---------to take photo----------.\n");
            if (__this->state & PHOTO_MODE_QSCAN) {
                break;
            }

            if (photo_db_select("phm") == 0) {
                /*
                 *正常拍照
                 */
                video_take_photo(&__this->camera[__this->camera_id]);
            } else {
                /*
                 *延时拍照
                 */
                video_delay_take_photo(&__this->camera[__this->camera_id]);
            }
            break;
        case ACTION_PHOTO_TAKE_SWITCH_WIN:
            photo_switch_camera();
            break;
#endif
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        // video_photo_cfg_reset();
        video_photo_stop();
        if (__this->sd_wait) {
            wait_completion_del(__this->sd_wait);
            __this->sd_wait = 0;
        }
        break;
    case APP_STA_DESTROY:
        if (__this->ui) {
            server_close(__this->ui);
            __this->ui = NULL;
        }
        sys_key_event_unmap(&org, &new);
        f_free_cache(CONFIG_ROOT_PATH);
        malloc_stats();
        log_d("<<<<<< video_photo: destroy\n");
        break;
    }

    return 0;

}

static int video_photo_key_event_handler(struct key_event *key)
{
    struct intent it;

    printf("\n key->event:::%d,key->value::::%d\n", key->event, key->value);
    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_PHOTO:
        case SHOT:
            if (__this->state & PHOTO_MODE_QSCAN) {
                break;
            }

            if (photo_db_select("phm") == 0) {
                /*
                 *正常拍照
                 */
                video_take_photo(&__this->camera[__this->camera_id]);
            } else {
                /*
                 *延时拍照
                 */
                video_delay_take_photo(&__this->camera[__this->camera_id]);
            }
            break;
        case KEY_MODE:
            break;
        case KEY_MENU:
            break;
        case KEY_UP:
            photo_switch_camera();
            break;
        case KEY_DOWN:



            //  extern int jpg_frames_insert_to_avi_demo(char *avi_name,int len,char *jpg_name);
            ///    jpg_frames_insert_to_avi_demo(0,0,0 );



            break;
        case  KEY_5:

            // extern u8 at_rec_start_key();
            //  at_rec_start_key();
            break ;


        default:
            break;
        }
        break;
    default:
        break;
    }

    return false;
}

static int video_photo_device_event_handler(struct sys_event *e)
{
    int err;

    if (!strcmp(e->arg, "video1")) {
        switch (e->u.dev.event) {
        case DEVICE_EVENT_IN:
            if (__this->camera_id == 1 && __this->camera[1].state == CAMERA_OFFLINE) {
                err = photo_camera_display();
                if (err) {
                    return err;
                }
            }
            __this->camera[1].state = CAMERA_ONLINE;
            break;
        case DEVICE_EVENT_OUT:
            if (__this->camera_id == 1) {
                photo_switch_camera();
            }
            __this->camera[1].state = CAMERA_OFFLINE;
            break;
        default:
            break;
        }
    }

    if (!strncmp((char *)e->arg, "uvc", 3)) {
        switch (e->u.dev.event) {
        case DEVICE_EVENT_IN:
            __this->uvc_id = ((char *)e->arg)[3] - '0';
            if (__this->camera_id == 3 && __this->camera[3].state == CAMERA_OFFLINE) {
                err = photo_camera_display();
                if (err) {
                    return err;
                }
            }
            __this->camera[3].state = CAMERA_ONLINE;
            break;
        case DEVICE_EVENT_OUT:
            if (__this->camera_id == 3) {
                photo_switch_camera();
            }
            __this->camera[3].state = CAMERA_OFFLINE;
            break;

        }
    }

    if (!strcmp((char *)e->arg, "camera0_err")) {
        photo_camera_stop_display();
        photo_camera_display();
    }

    if (!ASCII_StrCmp(e->arg, "sd*", 4)) {

        // printf("\n video_photo_device_event_handler >>>>>>take_photo_num_calc>>>>>>>>>>>>\n ");
        if (lcd_disp_busy_state()) {
            take_photo_num_calc(NULL);
        }
    }

    return false;
}

static int event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return video_photo_key_event_handler(&event->u.key);
    case SYS_DEVICE_EVENT:
        return video_photo_device_event_handler(event);
    default:
        return false;
    }
    return false;
}

static const struct application_operation video_photo_ops = {
    .state_machine  = state_machine,
    .event_handler  = event_handler,
};

REGISTER_APPLICATION(app_video_photo) = {
    .name 	= "video_photo",
    .action	= ACTION_PHOTO_TAKE_MAIN,
    .ops 	= &video_photo_ops,
    .state  = APP_STA_DESTROY,
};


