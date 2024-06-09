#include "fs.h"
#include "server/server_core.h"
#include "system/includes.h"
#include "os/os_compat.h"
#include "server/net_server.h"
#include "server/ctp_server.h"
#include "http/http_server.h"
#include "storage_device.h"
#include "server/vunpkg_server.h"
#include "fs/fs.h"
#include "lwip/sockets.h"
#include "server/simple_mov_unpkg.h"

#include "time.h"
#include "sys/time.h"
#include "common/common.h"


//#include "rt_stream_pkg.h"
/* #include "action.h" */
#include "app_config.h"

extern void *memmem(const void *__haystack, size_t __haystacklen,
                    const void *__needle, size_t __needlelen);


#define VIDEO_FILE_LIST_JSON_HEAD  "{\"file_list\":["
#define VIDEO_JSON_MEM   "{\"y\":%d,\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"s\":\"%d\"},"
#define VIDEO_JSON_APP   "{\"op\":\"NOTIFY\",\"param\":{\"status\":\"%d\",\"desc\":\"{\\\"y\\\":%d,\\\"f\\\":\\\"%s%s\\\",\\\"t\\\":\\\"%04d%02d%02d%02d%02d%02d\\\",\\\"s\\\":\\\"%d\\\"}\"}}"
#define PICTURE_JSON_MEM   "{\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"s\":\"%d\"},"
#define PICTURE_JSON_APP    "{\"op\":\"NOTIFY\",\"param\":{\"desc\":\"{\\\"f\\\":\\\"%s%s\\\",\\\"t\\\":\\\"%04d%02d%02d%02d%02d%02d\\\",\\\"s\\\":\\\"%d\\\"}\"}}"
#define MAX_NUM  600
#define INFO_LEN 256
#define FILE_MAX_NUM    99999



const char __fs_arg[3][15] = {  "-tMOV -st",
                                "-tJPG  -st",
                                "-tMOVJPG -st"
                             };


struct file_info {
    struct list_head entry;
    u32 id;
    u32 len;
    u8(*fd)[INFO_LEN];
};



struct media_file_info {
    FILE *fd;
    u32  vaild;
    int _attr;
    struct vfs_attr attr;
    struct __mov_unpkg_info info;
    u8 channel; // 前后视区分
    u8 type;// 1视频  2 图片  4，，，
    u8 namebuf[64];
    u16 height; //use in jpg
    u16 width; // use in jpg

};



static struct list_head forward_file_list_head;
static struct list_head behind_file_list_head;
static int FMAX_ID = 0;
static int BMAX_ID = 0;
static u32 initing = 0;
static u8 forward_file_mem[MAX_NUM + 2][INFO_LEN];
static u8 behind_file_mem[MAX_NUM + 2][INFO_LEN];
static u8 f_path[64];
static u8 b_path[64];
static OS_MUTEX file_list_mutex;
static u32 mutex_init = 0;

int send_json(struct media_file_info *__info, u32 status);
const char *get_rec_path_1()
{
    return CONFIG_REC_PATH_0;
}
const char *get_rec_path_2()
{
#if defined CONFIG_VIDEO1_ENABLE
    return CONFIG_REC_PATH_1;
#else
    return CONFIG_REC_PATH_3;
#endif
}



static u8 *__find_emtry_block_f(struct file_info *info)
{
    int i = 1;
    static int f_jpg_id = 1;
    u8 *str = NULL;
    /* os_mutex_pend(&file_list_mutex, 0); */
    for (i = 1; i < MAX_NUM; i++) {
        if (forward_file_mem[i][0] != '{') {
            /* printf("forward_file_mem[%d]\n", i); */
            info->id = i;
            FMAX_ID = MAX(FMAX_ID, i);
            /* os_mutex_post(&file_list_mutex); */
            return forward_file_mem[i];
        }
    }
    //如果没有找到空
    while (f_jpg_id < MAX_NUM) {
        if (memmem(forward_file_mem[f_jpg_id], INFO_LEN, "JPG", 3)) {
            printf("delete jpg to set ,forward_file_mem[%d]\n", f_jpg_id);
            str = forward_file_mem[f_jpg_id];
            memset(str, ' ', INFO_LEN);

        } else {
            printf("this is MOV%d\n", f_jpg_id);

        }


        f_jpg_id++;

        if (str != NULL) {
            /* os_mutex_post(&file_list_mutex); */
            return str;
        }

    }
    f_jpg_id = 1;



    /* os_mutex_post(&file_list_mutex); */
    printf("file mem is full,please delete some file\n");

    return NULL;
}
static u8 *__find_emtry_block_b(struct file_info *info)
{
    int i = 1;
    static int b_jpg_id = 1;
    u8 *str = NULL;
    /* os_mutex_pend(&file_list_mutex, 0); */
    for (i = 1; i < MAX_NUM; i++) {
        if (behind_file_mem[i][0] != '{') {
            /* printf("behind_file_mem[%d]\n", i); */
            info->id = i;
            BMAX_ID = MAX(BMAX_ID, i);
            /* os_mutex_post(&file_list_mutex); */
            return behind_file_mem[i];
        }
    }
    while (b_jpg_id < MAX_NUM) {
        if (memmem(behind_file_mem[b_jpg_id], INFO_LEN, "JPG", 3)) {
            printf("delete jpg to set ,behind_file_mem[%d]\n", b_jpg_id);
            str =  behind_file_mem[b_jpg_id];
            memset(str, ' ', INFO_LEN);
        }
        b_jpg_id++;

        if (str != NULL) {
            /* os_mutex_post(&file_list_mutex); */
            return str;
        }

    }
    b_jpg_id = 1;

    /* os_mutex_post(&file_list_mutex); */
    printf("file mem is full,please delete some file\n");

    return NULL;
}

static size_t forward_write_block(u8 *buffer, size_t len)
{

    struct file_info *info = malloc(sizeof(struct file_info));
    if (info == NULL) {
        return -1;
    }


    os_mutex_pend(&file_list_mutex, 0);
    u8 *p = __find_emtry_block_f(info);
    if (p == NULL) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }


    memcpy(p, buffer, len);

    info->len = len;
    info->fd = (u8(*)[INFO_LEN])p;

    /* printf("info->fd=%s   info->len=%d\n", info->fd, info->len); */
    list_add_tail(&info->entry, &forward_file_list_head);
    os_mutex_post(&file_list_mutex);
    return len;

}

static int forward_remove_block(const char *fname)
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;

    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&forward_file_list_head)) {

        os_mutex_post(&file_list_mutex);
        return -1;
    }
    list_for_each_safe(pos, node, &forward_file_list_head) {
        __info = list_entry(pos, struct file_info, entry);
        if (memmem(__info->fd, __info->len, fname, strlen(fname))) {
            memset(__info->fd, ' ', INFO_LEN);

            list_del(&__info->entry);
            free(__info);
            os_mutex_post(&file_list_mutex);
            return 0;
        }
    }
    os_mutex_post(&file_list_mutex);
    return -1;
}
static int forward_change_block(const char *fname, char attr)
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;
    char *str = NULL;

    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&forward_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }
    list_for_each_safe(pos, node, &forward_file_list_head) {
        __info = list_entry(pos, struct file_info, entry);
        if (memmem(__info->fd, __info->len, fname, strlen(fname))) {
            str = memmem(__info->fd, __info->len, "\"y\":", 4);
            printf("attr=%c \n", attr);
            if (str) {
                *(str + 4) = attr;
                os_mutex_post(&file_list_mutex);
                return 0;
            }
        }

    }
    os_mutex_post(&file_list_mutex);
    return -1;
}

static int forward_remove_block_all()
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&forward_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &forward_file_list_head) {
        __info = list_entry(pos, struct file_info, entry);
        list_del(&__info->entry);
        memset(__info->fd, ' ', INFO_LEN);
        free(__info);
    }
    os_mutex_post(&file_list_mutex);
    return 0;

}



static size_t behind_write_block(u8 *buffer, size_t len)
{

    /* printf("p addr ->0x%x\n",p); */

    struct file_info *info = malloc(sizeof(struct file_info));
    if (info == NULL) {
        return -1;
    }

    os_mutex_pend(&file_list_mutex, 0);
    u8 *p = __find_emtry_block_b(info);
    if (p == NULL) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    memcpy(p, buffer, len);

    info->len = len;
    info->fd = (u8(*)[INFO_LEN])p;

    /* printf("info->fd=%s   info->len=%d\n", info->fd, info->len); */
    list_add_tail(&info->entry, &behind_file_list_head);

    os_mutex_post(&file_list_mutex);
    return len;

}

static int behind_remove_block(const char *fname)
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&behind_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &behind_file_list_head) {
        __info = list_entry(pos, struct file_info, entry);
        if (memmem(__info->fd, __info->len, fname, strlen(fname))) {
            memset(__info->fd, ' ', __info->len);
            list_del(&__info->entry);
            free(__info);
            os_mutex_post(&file_list_mutex);
            return 0;
        }
    }
    os_mutex_post(&file_list_mutex);
    return -1;
}

static int behind_change_block(const char *fname, char attr)
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;

    char *str = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&behind_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &behind_file_list_head) {
        __info = list_entry(pos, struct file_info, entry);

        if (memmem(__info->fd, __info->len, fname, strlen(fname))) {
            str = memmem(__info->fd, __info->len, "\"y\":", 4);
            if (str) {
                *(str + 4) = attr;
                os_mutex_post(&file_list_mutex);
                return 0;
            }
        }
    }
    os_mutex_post(&file_list_mutex);
    return -1;
}




static int behind_remove_block_all()
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&behind_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &behind_file_list_head) {
        __info = list_entry(pos, struct file_info, entry);
        list_del(&__info->entry);
        memset(__info->fd, ' ', INFO_LEN);
        free(__info);
    }
    os_mutex_post(&file_list_mutex);
    return 0;


}

void FILE_REMOVE_ALL()
{

    if (FILE_INITIND_CHECK()) {
        forward_remove_block_all();

#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
        behind_remove_block_all();
#endif
    }
}


void __FILE_LIST_INIT(u8 is_forward, u32 file_num)
{

    FILE *fd = NULL;
    u32 count = 0;
    u32 flag = 0;
    char res[32];
    char path[64];
    struct vfscan *fs = NULL;
    struct media_file_info media_info;
    memset(res, 0, 32);

    if (is_forward) {
        strcpy(res, get_rec_path_1());

        INIT_LIST_HEAD(&forward_file_list_head);
        memset(forward_file_mem, ' ', (MAX_NUM + 2) * INFO_LEN);
        flag = 0;
    } else {
        strcpy(res, get_rec_path_2());
        INIT_LIST_HEAD(&behind_file_list_head);
        memset(behind_file_mem, ' ', (MAX_NUM + 2)* INFO_LEN);
        flag = 1;
    }


    fs = fscan(res, "-tMOVJPG -st");
    if (fs == NULL) {
        return;
    }
    while (1) {
        if (fd == NULL) {
            fd = fselect(fs, FSEL_FIRST_FILE, 0);
        } else {
            fd = fselect(fs, FSEL_NEXT_FILE, 0);
        }
        if (fd == NULL || count >= file_num) {
            goto close1;
        }
        media_info.fd = fd;
        media_info.channel = flag;

        send_json(&media_info, 0x2);
        fclose(fd);


        count++;

    }
close1:

    fscan_release(fs);

}


void FILE_DELETE(char *fname, u8 create_file)
{
    printf("vf_list detele:%s\n", fname);

    if (FILE_INITIND_CHECK()) {
        return;
    }
    if (fname == NULL) {
        forward_remove_block_all();

#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
        behind_remove_block_all();
#endif
    }

    if (strstr(fname, get_rec_path_1())) {
        forward_remove_block(fname);
    } else if (strstr(fname, get_rec_path_2())) {

#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
        behind_remove_block(fname);
#endif
    }
}

void FILE_CHANGE_ATTR(const char *fname, char attr)
{
    if (FILE_INITIND_CHECK()) {
        return;
    }

    if (fname == NULL) {
        printf("file name is NULL\n");
        return ;
    }

    if (strstr(fname, get_rec_path_1())) {
        forward_change_block(fname, attr);
    } else if (strstr(fname, get_rec_path_2())) {
#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
        behind_change_block(fname, attr);
#endif
    }

}


void FILE_GEN(void)
{

    FILE *fd = NULL;
    FILE *fd2 = NULL;
    u8 *str_ptr = NULL;
    u8 *str_ptr2 = NULL;
    char path[64];
    int j = 0;
    os_mutex_pend(&file_list_mutex, 0);
    sprintf(path, "%s%s", get_rec_path_1(), "vf_list.txt");
    fd = fopen(path, "w+");

    //写json_head
    str_ptr = forward_file_mem[0];
    memcpy(str_ptr, VIDEO_FILE_LIST_JSON_HEAD, strlen(VIDEO_FILE_LIST_JSON_HEAD));
    str_ptr = forward_file_mem[FMAX_ID];
    if (FMAX_ID) {
        str_ptr = str_ptr + INFO_LEN;
        while (*(str_ptr--) != ',') {
            j++;
        }
        printf("file list ptr offset:%d\n", j);
        *(++str_ptr) = ' ';
        str_ptr2 = str_ptr;
    }
    str_ptr = forward_file_mem[FMAX_ID + 1];
    memcpy(str_ptr, "]}", 2);

    fwrite(fd, forward_file_mem, (FMAX_ID + 2)*INFO_LEN);

    fclose(fd);
    *(str_ptr2) = ',';
    /* os_mutex_post(&file_list_mutex); */

    printf("%s  %d\n", __func__, __LINE__);

#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
    sprintf(path, "%s%s", get_rec_path_2(), "vf_list.txt");
    fd2 = fopen(path, "w+");
    /* os_mutex_pend(&file_list_mutex, 0); */
    str_ptr = behind_file_mem[0];
    memcpy(str_ptr, VIDEO_FILE_LIST_JSON_HEAD, strlen(VIDEO_FILE_LIST_JSON_HEAD));
    str_ptr = behind_file_mem[BMAX_ID];
    if (BMAX_ID) {
        str_ptr = str_ptr + INFO_LEN;
        while (*(str_ptr--) != ',');
        *(++str_ptr) = ' ';
        str_ptr2 = str_ptr;
    }
    str_ptr = behind_file_mem[BMAX_ID + 1];
    memcpy(str_ptr, "]}", 2);
    fwrite(fd2, behind_file_mem, (BMAX_ID + 2) *INFO_LEN);
    fclose(fd2);
    *(str_ptr2) = ',';
#endif
    os_mutex_post(&file_list_mutex);
    printf("FMAX_ID= %d  BMAX_ID=%d\n", FMAX_ID, BMAX_ID);

}

void FILE_LIST_ADD(u32 status, const char *path, u8 create_file)
{
    puts("FILE_LIST_ADD\n");
    u32 len;
    struct media_file_info media_info;
    FILE *fd = NULL;
    u8 flag = 0;
    printf("fs->%s\n", path);

    len = strlen(path);
    if (len < 3) {
        return;
    }
    if (!storage_device_ready()) {
        return;
    }
    printf("file open %s\n", path);
    fd = fopen(path, "r");
    if (fd == NULL) {
        return;
    }

    if (strstr(path, get_rec_path_1())) {
        flag = 0;
    } else {
        flag = 1;
    }
    media_info.fd = fd;
    media_info.channel = flag;
    send_json(&media_info, status);

    fclose(fd);

}










void __FILE_LIST_INIT(u8 is_forward, u32 file_num);

extern int send_ctp_string(int cmd_type, char *buf, char *_req, void *priv);






int get_media_file_info(struct media_file_info *__info)
{
    int ret = 0;
    if (__info == NULL || __info->fd == NULL) {
        printf("file is not open\n");
        return -1;
    }
    fget_name(__info->fd, __info->namebuf, sizeof(__info->namebuf));
    ret = strlen((const char *)__info->namebuf) - 3;
    fget_attrs(__info->fd, &__info->attr);
    fget_attr(__info->fd, &__info->_attr);

    if (!memcmp((__info->namebuf + ret), "MOV", 3)) {
        /* if (is_vaild_mov_file(__info->fd)) { */
#if 0 //不读属性
        if (0 != read_stts(__info->fd, &__info->info)) {
            __info->vaild = 0x0;
            printf("Invalid file\n");
            return -1;

        }
        if (0 != read_time_scale_dur(__info->fd, &__info->info)) {
            __info->vaild = 0x0;
            printf("Invalid file\n");
            return -1;
        }
        if (0 != read_height_and_length(__info->fd, &__info->info)) {
            __info->vaild = 0x0;
            printf("Invalid file\n");
            return -1;
        }
#endif

        if (__info->_attr & F_ATTR_RO) {
            __info->vaild = 0x2;
        } else {
            __info->vaild = 0x1;
        }
        __info->type = 0x1;

        /* } else { */
        /* __info->vaild = 0x0; */
        /* printf("Invalid file\n"); */
        /* return -1; */
        /* } */

    } else if (!memcmp((__info->namebuf + ret), "JPG", 3)) {
        __info->type = 0x2;
#if 0
        fseek(__info->fd, 0x174, SEEK_SET);
        fread(__info->fd, &__info->height, 2);
        fread(__info->fd, &__info->width, 2);
        __info->height = lwip_htons(__info->height);
        __info->width  = lwip_htons(__info->width);
#endif
    } else {
        printf("FILE NAME:%s err!!!!!!\n", __info->namebuf);
        return -1;

    }
    return 0;

}






static void incr_date_time(struct tm *tm_time, int incr)
{
    do {
        if (incr >= 60) {
            tm_time->tm_sec += 60;
            incr -= 60;
        } else {
            tm_time->tm_sec += incr;
            incr = 0;
        }

        if (tm_time->tm_sec > 59) {
            tm_time->tm_sec -= 60;
            tm_time->tm_min += 1;

            if (tm_time->tm_min > 59) {
                tm_time->tm_min -= 60;
                tm_time->tm_hour += 1;

                if (tm_time->tm_hour > 23) {
                    tm_time->tm_hour -= 24;
                    tm_time->tm_mday += 1;

                    if (tm_time->tm_mday > cal_days(tm_time->tm_year, tm_time->tm_mon)) {
                        tm_time->tm_mday = 1;
                        tm_time->tm_mon += 1;

                        if (tm_time->tm_mon > 12) {
                            tm_time->tm_mon = 1;
                            ++tm_time->tm_year;
                        }
                    }
                }
            }
        }
    } while (incr);
}



int send_json(struct media_file_info *__info, u32 status)
{
    int ret = 0;

    struct tm _tm;
    char *buffer = malloc(256);
    if (buffer == NULL) {
        printf("malloc fail \n");
        return -1;
    }

    if (get_media_file_info(__info)) {
        printf("get_media_file_info fail\n");
        free(buffer);
        return -1;
    }

    memset(buffer, 0, 256);
    _tm.tm_year  = __info->attr.crt_time.year;
    _tm.tm_mon  = __info->attr.crt_time.month;
    _tm.tm_mday  = __info->attr.crt_time.day;
    _tm.tm_hour  = __info->attr.crt_time.hour;
    _tm.tm_min  = __info->attr.crt_time.min;
    _tm.tm_sec  = __info->attr.crt_time.sec;

    printf("start=%d-%d-%d-%d-%d-%d\n"
           , _tm.tm_year
           , _tm.tm_mon
           , _tm.tm_mday
           , _tm.tm_hour
           , _tm.tm_min
           , _tm.tm_sec);


    if (__info->type & 0x1) {
        incr_date_time(&_tm, __info->info.durition / __info->info.scale);
        ret = sprintf(buffer, VIDEO_JSON_MEM
                      , __info->vaild
                      , __info->channel ? get_rec_path_2() : get_rec_path_1()
                      , __info->namebuf
                      , __info->attr.crt_time.year
                      , __info->attr.crt_time.month
                      , __info->attr.crt_time.day
                      , __info->attr.crt_time.hour
                      , __info->attr.crt_time.min
                      , __info->attr.crt_time.sec
                      , flen(__info->fd)
                     );

        /* printf("buffer=>%s\n", buffer); */
        if (!__info->channel) {
            forward_write_block((u8 *)buffer, ret);
        } else {
            behind_write_block((u8 *)buffer, ret);
        }

        if (status <= 1) {
            memset(buffer, 0, 256);
            ret = sprintf(buffer, VIDEO_JSON_APP
                          , status
                          , __info->vaild
                          , __info->channel ? get_rec_path_2() : get_rec_path_1()
                          , __info->namebuf
                          , __info->attr.crt_time.year
                          , __info->attr.crt_time.month
                          , __info->attr.crt_time.day
                          , __info->attr.crt_time.hour
                          , __info->attr.crt_time.min
                          , __info->attr.crt_time.sec
                          , flen(__info->fd)
                         );

            printf("buffer=>%s\n", buffer);
            send_ctp_string(CTP_NOTIFY_COMMAND, buffer, "VIDEO_FINISH", NULL);
        }
    } else if (__info->type & 0x2) {
        ret = sprintf(buffer, PICTURE_JSON_MEM
                      , __info->channel ? get_rec_path_2() : get_rec_path_1()
                      , __info->namebuf
                      , __info->attr.crt_time.year
                      , __info->attr.crt_time.month
                      , __info->attr.crt_time.day
                      , __info->attr.crt_time.hour
                      , __info->attr.crt_time.min
                      , __info->attr.crt_time.sec
                      , flen(__info->fd)
                     );

        printf("buffer=>%s\n", buffer);
        if (!__info->channel) {
            forward_write_block((u8 *)buffer, ret);
        } else {
            behind_write_block((u8 *)buffer, ret);
        }

        if (status <= 1) {
            memset(buffer, 0, 256);
            sprintf(buffer, PICTURE_JSON_APP
                    , __info->channel ? get_rec_path_2() : get_rec_path_1()
                    , __info->namebuf
                    , __info->attr.crt_time.year
                    , __info->attr.crt_time.month
                    , __info->attr.crt_time.day
                    , __info->attr.crt_time.hour
                    , __info->attr.crt_time.min
                    , __info->attr.crt_time.sec
                    , flen(__info->fd)
                   );

            printf("buffer=>%s\n", buffer);
            send_ctp_string(CTP_NOTIFY_COMMAND, buffer, "PHOTO_CTRL", NULL);
        }

    }
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    return 0;
}



int FILE_GEN_SYNC(const char *filename, const char *dir)
{

    FILE *fd = NULL;
    struct vfscan *fs = NULL;
    int ret  = 0;
    char path[64];
    u32 file_count = 0;
    struct media_file_info __info;
    printf("dir->%s\n", dir);
    fs = fscan(dir, "-tMOVJPG -sn");
    if (fs == NULL) {
        /* printf("%s  %d\n",__func__,__LINE__); */
        return -1;
    }

    char *buffer = malloc(128 * 1024);
    char *tmp = buffer;

    memcpy(buffer, VIDEO_FILE_LIST_JSON_HEAD, strlen(VIDEO_FILE_LIST_JSON_HEAD));
    buffer += strlen(VIDEO_FILE_LIST_JSON_HEAD);


    while (1) {
        if (fd == NULL) {
            fd = fselect(fs, FSEL_FIRST_FILE, 0);
        } else {
            fd = fselect(fs, FSEL_NEXT_FILE, 0);
        }
        if (fd == NULL) {
            /* printf("%s  %d\n",__func__,__LINE__); */
            goto close1;
        }

        /* printf("%s  %d\n",__func__,__LINE__); */
        __info.fd = fd;
        get_media_file_info(&__info);


        ret = sprintf(buffer, VIDEO_JSON_MEM
                      , __info.vaild
                      , dir
                      , __info.namebuf
                      , __info.attr.crt_time.year
                      , __info.attr.crt_time.month
                      , __info.attr.crt_time.day
                      , __info.attr.crt_time.hour
                      , __info.attr.crt_time.min
                      , __info.attr.crt_time.sec
                      , flen(__info.fd)
                     );

        buffer += ret;
        file_count++;
        fclose(fd);
    }

close1:
    buffer -= ret;
    memset(buffer, 0, ret);


    fdelete_by_name(filename);
    fd = fopen(filename, "w+");
    printf("fd=%x\n", fd);
    if (file_count > 1) {
        memcpy(buffer - 1, "]}", 2);
    } else {

        memcpy(buffer, "]}", 2);
    }

    if (file_count > 1) {
        fwrite(fd, tmp, buffer - tmp + 1);
    } else {

        fwrite(fd, tmp, buffer - tmp + 2);
    }

    fclose(fd);

    free(tmp);
    printf("len->%d\n", buffer - tmp + 1);
    fscan_release(fs);
    if (file_count > 1) {
        return buffer - tmp + 1;
    } else {
        return buffer - tmp + 2;
    }

}






int FILE_LIST_INIT(u32 flag)
{
    char buf[128];
    u32 err = 0;
    if (initing) {
        printf("File list in doing, Please waiting\n");
        return -1;
    }
    initing = 1;
    if (!mutex_init) {

        err = os_mutex_create(&file_list_mutex);
        if (err != OS_NO_ERR) {
            printf("os mutex create fail \n");
            return -1;
        }
        mutex_init = 1;

    }
    __FILE_LIST_INIT(1, FILE_MAX_NUM);


#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
    __FILE_LIST_INIT(0, FILE_MAX_NUM);

#endif
    printf("flag=%d\n", flag);
    if (flag) {
        FILE_GEN();
        sprintf(buf, "type:1,path:%s%s", get_rec_path_1(), "vf_list.txt");
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "FORWARD_MEDIA_FILES_LIST", "NOTIFY", buf);

#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
        sprintf(buf, "type:1,path:%s%s", get_rec_path_2(), "vf_list.txt");
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "BEHIND_MEDIA_FILES_LIST", "NOTIFY", buf);
#endif
    }


    initing = 0;
    return 0;

}




void file_list_thread(void *arg)
{
    FILE_LIST_INIT((u32)arg);
}





void FILE_LIST_INIT_SMALL(u32 file_num)
{
    __FILE_LIST_INIT(2, file_num);
#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
    __FILE_LIST_INIT(3, file_num);
#endif
}




int FILE_INITIND_CHECK()
{
    return initing;
}


void FILE_LIST_IN_MEM(u32 flag)
{
    static u32 count = 0;
    char buf[32];
    u32 space = 0;

    sprintf(buf, "file_list_thread%d", count++);
    thread_fork(buf, 10, 0x1000, 0, 0, file_list_thread, (void *)flag/*(void *)&flag*/);

}







#if 1
extern u32 ASCII_StrCmp(const char *src, const char *dst, u32 len);
static void wired_event_handler(struct sys_event *event)
{
    char buf[256];
    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        msleep(100);
        /* FILE_GEN_SYNC(NULL,get_rec_path_1()); */
        strcpy(buf, "online:1");
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "SD_STATUS", "NOTIFY", buf);
        u32 space = 0;
        struct vfs_partition *part = NULL;
        if (storage_device_ready() == 0) {
            CTP_CMD_COMBINED(NULL, CTP_SD_OFFLINE, "TF_CAP", "NOTIFY", CTP_SD_OFFLINE_MSG);
        } else {
            part = fget_partition(CONFIG_ROOT_PATH);
            fget_free_space(CONFIG_ROOT_PATH, &space);
            sprintf(buf, "left:%d,total:%d", space / 1024, part->total_size / 1024);
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "TF_CAP", "NOTIFY", buf);
            FILE_LIST_IN_MEM(1);

        }

        break;
    case DEVICE_EVENT_OUT:
        FILE_REMOVE_ALL();
        strcpy(buf, "online:0");
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "SD_STATUS", "NOTIFY", buf);
        break;
    }
}


static void charge_event_handler(struct sys_event *event)
{
    char buf[256];
    switch (event->u.dev.event) {
    case DEVICE_EVENT_POWER_CHARGER_IN:

        printf("charger charger \n\n\n\n\n");
        if (storage_device_ready() == 0) {
        } else {

            FILE_LIST_IN_MEM(0);
        }

        break;
    case DEVICE_EVENT_POWER_CHARGER_OUT:
        printf("charger charger \n\n\n\n\n");


        break;
    }
}




static void device_event_handler4(struct sys_event *event)
{
    if (!ASCII_StrCmp(event->arg, "sd*", 4)) {
        wired_event_handler(event);
    } else if (!ASCII_StrCmp(event->arg, "sys_power", 7)) {
        charge_event_handler(event);

    }

}
/*
 * 静态注册设备事件回调函数，优先级为0
 */
/* SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, device_event_handler4, 0); */
#endif






























