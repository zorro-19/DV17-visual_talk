
#include "system/includes.h"
#include "generic/ioctl.h"
#include "circular_buf.h"
#include "audio_dev.h"
#include "database.h"
#include "gpio.h"
#include "doorbell_event.h"
#include "action.h"

extern int net_video_rec_get_audio_rate();

#define RT_TALK_BUFFER_SIZE     32 * 1024
static buffer[RT_TALK_BUFFER_SIZE];
static cbuffer_t __cbuffer;
static u8 cloud_rt_talk_init_flag = 0;
static void *audio_dev;
static int close_aac_flag = 0;





#include "server/audio_server.h"
static struct server *audio = NULL;
static int bindex = 0xff;
OS_SEM  r_sem;
#define INT_MAX 0x7FFFFFFF

static void audio_server_event_handler(void *priv, int argc, int *argv)
{
    printf("argc:%d agrv[0]:0x%x\n", argc, argv[0]);
    union audio_req r = {0};
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_DEC_END:
        if (audio) {
            r.dec.cmd = AUDIO_DEC_STOP;
            server_request(audio, AUDIO_REQ_DEC, &r);
            server_close(audio);
            audio = NULL;
            /* kvoice_disable = 0; */
        }
        break;
    case AUDIO_SERVER_EVENT_ENC_ERR:
        break;

    }
}

static int stream_fread(void *file, void *buf, u32 len)
{
    int ret;
__again:
    if (cbuf_get_data_size(&__cbuffer) < len)  {
        ret = os_sem_pend(&r_sem, 20);
        if(close_aac_flag){
            return !0;
        }
        goto __again;
    }
    if(close_aac_flag){
        return !0;
    }
    return cbuf_read(&__cbuffer, buf, len);
}

static int stream_fseek(void *file, u32 offset, int seek_mode)
{
    return 0;
}

static int stream_flen(void *file)
{
    return INT_MAX;
}

static const struct audio_vfs_ops stream_vfs = {
    .fread = stream_fread,
    .fseek = stream_fseek,
    .flen = stream_flen,
};


void aac_decode_init(void)
{
    int ret;
    union audio_req r = {0};

    audio = server_open("audio_server", "dec");
    if (!audio) {
        return;
    }
    r.dec.cmd             = AUDIO_DEC_OPEN;
    r.dec.volume          = net_video_rec_get_dac_volume();//20;
    r.dec.output_buf      = NULL;
    r.dec.output_buf_len  = 1024 * 4;
    r.dec.file            = 0;
    r.dec.type        = "m4a";
    r.dec.sample_rate     = 0;//必须写零，由文件指定,否则不会解码文件信息
    r.dec.priority        = 0;
    r.dec.sample_source = "user";//"iis0";
    r.dec.vfs_ops      = &stream_vfs;
    ret = server_request(audio, AUDIO_REQ_DEC, &r);
    printf("ret:%d line:%d\n", ret, __LINE__);
    printf("total_time= %d \n", r.dec.total_time); //获取文件播放长度，可以用于ui显示
    printf("sample_rate= %d \n", r.dec.sample_rate); //获取文件采样率，可以用于ui显示

    r.dec.cmd = AUDIO_DEC_START;
    ret = server_request(audio, AUDIO_REQ_DEC, &r);
    printf("ret:%d line:%d\n", ret, __LINE__);
    server_register_event_handler(audio, NULL, audio_server_event_handler);//播放结束回调函数
}


void aac_decode_uninit(void)
{
     printf("\n\n >>>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
    union audio_req r = {0};

    if(audio){
        close_aac_flag = 1;
             printf("\n\n >>>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);

         r.dec.cmd = AUDIO_DEC_PAUSE;
         server_request(audio, AUDIO_REQ_DEC, &r);

         r.dec.cmd = AUDIO_DEC_STOP;
         server_request(audio, AUDIO_REQ_DEC, &r);
        printf("\n\n >>>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
        server_close(audio);
        audio = NULL;
         printf("\n\n >>>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
        close_aac_flag = 0;
    }
}


int pcm_change_sampleRate(int insr, int outsr, u8  *in, u32 in_len, u8  *out, u32 out_cap)
{

    int rate = insr / outsr;
    int out_len = in_len / rate;

    //48k采样率，降到16k,则rate=3; 48转8有问题
    for (int i = 0, j = 0; i < in_len; i += 2 * rate, j += 2) {
        out[j] = in[i];
        out[j + 1] = in[i + 1];
    }

    return (out_len);
}

static u8 flag = 0;
static u8 buffer2[4096+2048];
static u8 output_buffer[4096+2048];
static cbuffer_t cbuffer2;
int m4a_decoder_output(u8 *buf, u32 len)
{

    if(!flag){
        cbuf_init(&cbuffer2, buffer2, sizeof(buffer2));
        flag = 1;
    }

    //printf("\n\n >>>>>>>>>>>>>>>>>>>>>>>>len = %d\n\n",len);
    cbuf_write(&cbuffer2,buf,len);//len 同时表示当前允许返回的最大数据长度

    int rlen = cbuf_get_data_size(&cbuffer2);
    if(rlen >= 4){
        int rlenMax = rlen / 4 * 4;
        if(rlenMax <= (len * 2)){
            rlen = rlenMax;
        }else{
            rlen = len * 2;
        }
        cbuf_read(&cbuffer2,output_buffer,rlen);
        s16 *tmpbuf = (s16 *)output_buffer;
        s16 *output_buf = (s16 *)buf;
        for(int i = 0 ; i < rlen / 4 ; i++){
            output_buf[i] = tmpbuf[i * 2];
        }
    }else{
        rlen = 0;
    }

    //printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>rlen = %d\n",rlen / 2);
    return rlen / 2;
}

#define  DAC_DEC_LEN    512
static u8 tmp_data1[3 * DAC_DEC_LEN];

static int audio_dec_irq_handler(void *priv, void *data, int len)
{
    int tmp_data1_len;
    if(bindex == 0xff){
        memset(data, 0, len);
        printf("\n\n >>>>>>>>>>>>>>>>>>>>>>>ret = %s %d\n\n",__func__,__LINE__);
        return len;
    }
    //printf("\n\n >>>>>>>>>>>>>> len = %d\n\n",len);
    ASSERT(len <= DAC_DEC_LEN,"\n Please modify DAC_ DEC_ LEN \n" )

#if (CONFIG_AUDIO_FORMAT_FOR_SPEARK == AUDIO_FORMAT_AAC)

#if (CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK != CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC)
    /**
        dac 中断每次取512字节
        AAC 解码出来的声音是双声道的
        app过来的声音是48k的
        512 对应的原始双声道的数据长度是 512 * 3 * 2，对应的单声道数据是512 * 3

    */
    memset(data, 0, len);
    tmp_data1_len = 3 * len;
    int ret = user_dec_get_data(tmp_data1, tmp_data1_len);
    if (ret) {
        pcm_change_sampleRate(CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK, CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC, tmp_data1, tmp_data1_len, data, len);
    }
#else


    memset(data, 0, len);
    int ret = user_dec_get_data(data, DAC_DEC_LEN);
#endif // CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK

#else

    cbuffer_t *cbuffer = (cbuffer_t *)priv;
#if (CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK != CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC)
    memset(data, 0, len);
    tmp_data1_len = 3 * len;
    cbuf_read(cbuffer, tmp_data1, tmp_data1_len);
    pcm_change_sampleRate(CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK, CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC, tmp_data1, tmp_data1_len, data, len);
#else
    cbuf_read(cbuffer, data, len);
#endif

#endif

    return len;
}

void pcm_decode_init(void)
{

    struct audio_format f = {0};
    audio_dev = dev_open("audio", (void *)AUDIO_TYPE_DEC);
    f.volume        = net_video_rec_get_dac_volume();
    f.channel       = 1;
    f.sample_rate   = net_video_rec_get_audio_rate();
    f.priority      = 0;
    f.type          = 0x1;
    f.frame_len     = 1024;
#ifndef CONFIG_USB_UVC_AND_UAC_ENABLE
    f.sample_source     = "dac";
#else
    f.sample_source     = "user";
#endif
    int err = dev_ioctl(audio_dev, AUDIOC_SET_FMT, (u32)&f);

    u32 arg[2];
    arg[0] = (u32)&__cbuffer;
    arg[1] = (u32)audio_dec_irq_handler;
    dev_ioctl(audio_dev, IOCTL_REGISTER_IRQ_HANDLER, (u32)arg);



}


int cloud_rt_talk_write(char *data, int len)
{
#if (CONFIG_AUDIO_FORMAT_FOR_SPEARK == AUDIO_FORMAT_AAC)

    if (cloud_rt_talk_init_flag) {
        doorbell_event_dac_mute(1);
        len = cbuf_write(&__cbuffer, data, len);
        if (len == 0) {
            putchar('B');
            cbuf_clear(&__cbuffer);
        } else {
            if (!os_sem_query(&r_sem)) {
                os_sem_post(&r_sem);
            }
        }

        if (!audio && cbuf_get_data_size(&__cbuffer)) {
#ifndef CONFIG_USB_UVC_AND_UAC_ENABLE
            if (!audio_dev) {
                pcm_decode_init();
            }
#endif
            extern void aac_decode_init(void);
            aac_decode_init();
        }
    }
#else

    if (cloud_rt_talk_init_flag) {
        if (!audio_dev) {
            pcm_decode_init();
        }

        len = cbuf_write(&__cbuffer, data, len);
        if (len == 0) {
            putchar('B');
            cbuf_clear(&__cbuffer);
        }
    }
#endif // CONFIG_AUDIO_FORMAT_FOR_SPEARK


    return len;
}



int cloud_rt_talk_init(void)
{

    if (cloud_rt_talk_init_flag) {
        cbuf_clear(&__cbuffer);
        return -1;
    }


    os_sem_create(&r_sem, 0);
    cbuf_init(&__cbuffer, buffer, RT_TALK_BUFFER_SIZE);
    cloud_rt_talk_init_flag = 1;

    return 0;

}

#include "system/includes.h"
#include "asm/lcd_config.h"

static void *fd = NULL;
static OS_MUTEX dec_server_mutex;
static u8 start = 0;
static u8 video_dec_start;

int get_dec_server_mutex(void)
{
    return os_mutex_pend(&dec_server_mutex,0);
}

int put_dec_server_mutex(void)
{
    return os_mutex_post(&dec_server_mutex);
}


int dec_server_open(void)
{
    struct video_format f = {0};

    if(!os_mutex_valid(&dec_server_mutex)){
        os_mutex_create(&dec_server_mutex);
    }

    os_mutex_pend(&dec_server_mutex, 0);
    start = 0;
    if (!fd) {
        fd = dev_open("video_dec", NULL);
        if (!fd) {
            ASSERT(0, "video_dec no define");
        }
        //f.type                      = VIDEO_BUF_TYPE_VIDEO_PLAY;
        f.type                      = VIDEO_BUF_TYPE_VIDEO_OVERLAY;
        f.src_width                 = 0;
        f.src_height                = 0;
        f.pixelformat           = VIDEO_PIX_FMT_JPEG; // VIDEO_PIX_FMT_JPEG
        f.win.left          = 0;
        f.win.top           = 0;
        f.win.width         =   LCD_DEV_WIDTH;
        f.win.height        = LCD_DEV_HIGHT;
        f.win.border_left   = 0;
        f.win.border_top    = 0;
        f.win.border_right  = 0;
        f.win.border_bottom = 0;

        f.private_data      = "fb1";
        dev_ioctl(fd, VIDIOC_SET_FMT, (u32)&f);
    }
    os_mutex_post(&dec_server_mutex);

}

void jpeg_dec_info(u8 *buf,int len){

    struct jpeg_image_info info = {0};
    u16 width, height;
    info.input.data.buf = buf;
    info.input.data.len = len;
    if (jpeg_decode_image_info(&info)) {
        printf("jpeg_decode_image_info err\n");

        goto free_buf;
    }

    width = info.width;
    height = info.height;
    switch (info.sample_fmt) {
        case JPG_SAMP_FMT_YUV444:


            break;//444
        case JPG_SAMP_FMT_YUV420:

            break;//420
        default:

            break;//422
    }



    printf("\n width:%d,%d,%d\n",width,height,info.sample_fmt);
free_buf:

    return ;

}


int dec_server_start(void)
{
   os_mutex_pend(&dec_server_mutex, 0);
    video_dec_start = 1;
    if(fd){
        dev_ioctl(fd, VIDIOC_PLAY, 1);
    }
    os_mutex_post(&dec_server_mutex);
}



int dec_server_write(u8 *data, u32 len) {
   // printf("\n len:%d KB\n",len/1024);
    os_mutex_pend(&dec_server_mutex, 0);
    if (video_dec_start && fd) {
    //    putchar('P');


        //jpeg_dec_info(data,len);
    #if  0
    static void *tfile=NULL;
    if(!tfile)
      {
        tfile = fopen(CONFIG_STORAGE_PATH"/C/test_****.jpg", "w+");
        if(tfile){


          int len3 = fwrite(tfile,data, len);
          printf("\n len3:%d\n",len3);
          fclose(tfile);
          tfile=NULL;
        }

      }
    #endif
        dev_write(fd, data, len);
    }
    os_mutex_post(&dec_server_mutex);

    return 0;
}

int dec_server_stop(void)
{
   os_mutex_pend(&dec_server_mutex, 0);
    video_dec_start = 0;
    if(fd){
        dev_ioctl(fd, VIDIOC_PLAY, 0);
    }
   os_mutex_post(&dec_server_mutex);
}


int dec_server_close(void)
{
    printf("\n %s %d\n", __func__, __LINE__);

    printf("\n %s %d\n", __func__, __LINE__);
    os_mutex_pend(&dec_server_mutex, 0);
    if (fd) {
    printf("\n %s %d\n", __func__, __LINE__);
    dev_ioctl(fd, VIDIOC_PLAY, 0);
    printf("\n %s %d\n", __func__, __LINE__);
        dev_close(fd);
    printf("\n %s %d\n", __func__, __LINE__);
        fd = NULL;
          printf("\n %s %d\n", __func__, __LINE__);
        video_dec_start = 0;
          printf("\n %s %d\n", __func__, __LINE__);
    }
      printf("\n %s %d\n", __func__, __LINE__);
    os_mutex_post(&dec_server_mutex);
      printf("\n %s %d\n", __func__, __LINE__);
}
static int videox_disp_stop(int id);


const char *background_cfg_file[] = {
    "mnt/spiflash/res/pro1.jpg",
    "mnt/spiflash/res/pro2.jpg",
    "mnt/spiflash/res/pro3.jpg",
    "mnt/spiflash/res/pro4.jpg",
};

char* find_substring(const char* a, const char* b) {
    // 使用strstr函数查找B在A中的位置
    const char* result = strstr(a, b);
    return (char*)result;  // 将const char*转为char*并返回
}

int find_string(char *a,char *b,char *out ) {

    char* result = find_substring(a, b);
    if (result != NULL) {

        memcpy(out,result-4,8);// 云台需固定图片格式8个字节长度    pro?.jpg
        printf("Found '%s' in '%s' at address: %p\n", b, a, result);
        return 1;
    } else {
        printf("'%s' not found in '%s'.\n", b, a);
         return 0;
    }

    return 0;
}

char *image_buf = NULL;

int  read_back_crc(char *path, char *dm5, char *url){
//固定资源路径 "mnt/spiflash/res/"

     if(!os_mutex_valid(&dec_server_mutex)){

        printf("\n\n\n\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>%s %d \n\n\n\n", __func__, __LINE__);

    }
    os_mutex_pend(&dec_server_mutex, 0);
    char *path1="mnt/spiflash/res/";
    int image_len;
    char name[64]={0};
    char file_path[128]={0};
    void *fd=NULL;

    int ret_number=0xff;

   char *send_md5=malloc(33);
    if(!send_md5){
    goto __exit;
    }
    memset(send_md5,0,33);
    memcpy(send_md5,dm5,32);
    printf("\n send_md5 :%s\n",send_md5);

    if(path){

        //printf("\n ======%d, %s,  path:%s=========\n",__LINE__ , __FUNCTION__,path );

       int ret=find_string(url,".jpg",name );

       // put_buf(name,9 );
        //printf("\n name::::  %s\n",name);
        sprintf(file_path,"%s%s",path1,name);

      //  printf("\n file_path====%s\n",file_path);
        for(int i=0;i<sizeof(background_cfg_file)/sizeof(background_cfg_file[0]);i++){

            if(!strcmp(file_path,background_cfg_file[i])){
                printf("\n background_cfg_file[%d]:%s\n ",i,background_cfg_file[i]);
                fd = fopen(background_cfg_file[i],"r");
                ret_number=i;
                break;
            }
            printf("\n i:%d\n",i);
        }

        if(!fd){

             printf("\n  mnt/spiflash/res/ no file\n");
            goto __exit;
        }
        image_len = flen(fd);
        image_buf = malloc(image_len);
        printf("\n ======%d, %d,  buf:%d=========\n",__LINE__ , image_len,image_buf );
        if(!image_buf){
            printf("\n malloc err %s %d>>>>>>>>>>\n",__func__,__LINE__);
            goto __exit;
        }
        fread(fd,image_buf,image_len);
        fclose(fd);
       // 校验
            uint8_t obuf[16]={0};
            extern  int get_picture_md5_crc( char *ibuf,char *obuf,int len);
            int err=get_picture_md5_crc( image_buf,&obuf[0],image_len);
            if(err){

            printf(" MD5_OUT: \n");
            int hex_array_len1 = sizeof(obuf) / sizeof(obuf[0]);

            // 计算转换后字符串的最大长度（每个字节两个字符加上可能的空字符）
            int string_len1 = hex_array_len1 * 2 + 1; // +1 for the null terminator
            char hex_string1[string_len1];

            // 使用sprintf将十六进制数组转换为字符串
            for (int i = 0, j = 0; i < hex_array_len1; ++i) {
                sprintf(&hex_string1[j], "%02x", obuf[i]); // 使用%02X格式化为两位大写的十六进制数
                j += 2;
            }

            //输出转换后的字符串
            printf("The hexadecimal array as a string is: %s\n", hex_string1);
            printf("\n send_md5 :%s\n",send_md5);

           // if(strcmp(dm5 ,hex_string1)){
            if(strcmp(send_md5 ,hex_string1)){
             printf("\n ======%d, %s,  no data=========\n",__LINE__ , __FUNCTION__ );
             goto __exit;

            }
            os_mutex_post(&dec_server_mutex);
            if(send_md5){
            free(send_md5);
            }
            return ret_number;
          }


        goto __exit;


}


__exit:
     if(send_md5){
            free(send_md5);
    }
    printf("\n err>>>>>>>>>> ");
    os_mutex_post(&dec_server_mutex);
    return -1;
}

void dec_pro_file(char *path,int flag)
{


    if(!os_mutex_valid(&dec_server_mutex)){

        printf("\n\n\n\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>%s %d \n\n\n\n", __func__, __LINE__);
        printf("\n\n\n\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>%s %d \n\n\n\n", __func__, __LINE__);
        printf("\n\n\n\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>%s %d \n\n\n\n", __func__, __LINE__);
        printf("\n\n\n\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>%s %d \n\n\n\n", __func__, __LINE__);
    }

    os_mutex_pend(&dec_server_mutex, 0);
    int image_len;
    char *image_buf = NULL;
    if(path){

      //  printf("\n ======%d, %s,  path:%s=========\n",__LINE__ , __FUNCTION__,path );
        void *fd = fopen(path,"r");


        image_len = flen(fd);


        image_buf = malloc(image_len);
         printf("\n ======%d, %d,  buf:%d=========\n",__LINE__ , image_len,image_buf );
        if(!image_buf){
            printf("\n malloc err %s %d>>>>>>>>>>\n",__func__,__LINE__);
            goto __exit;
        }
        fread(fd,image_buf,image_len);

       #if  0
        if(flag){// 校验
            char obuf[16]={0};
            extern  int get_picture_md5_crc( char *ibuf,char *obuf,int len);
            get_picture_md5_crc( image_buf,obuf,image_len);
            if(strcmp(obuf,md5)){
            printf("\n ======%d, %s,  no data=========\n",__LINE__ , __FUNCTION__ );
                return 0;

            }
        }
        #endif
        #if 1
        extern u8 product_write_startup_screens(u8 idx, u8 *buf, u32 len, u32 file_size);
        product_write_startup_screens(0,image_buf,image_len,image_len);
        #else

        extern u8 product_write_bootscreens_file(u8 type, u8 idx, u8 *buf, u32 len, u32 file_size);
        product_write_bootscreens_file(0, 0, image_buf, image_len, image_len);
        #endif
        fclose(fd);
        free(image_buf);
        //fd = NULL;
    }

    printf("\n %s %d>>>>>>>>>>\n",__func__,__LINE__);
    u32 info[4];
    product_read_bootscreens_info(info);
    printf("\n %s %d>>>>>>>>>>\n",__func__,__LINE__);
    image_len = info[1];
    if (image_len == 0 || image_len == -1) {
        printf("\n %s %d>>>>>>>>>>\n",__func__,__LINE__);
        goto __exit;
    }



    struct video_format f = {0};


    image_buf = malloc(image_len);
          printf("\n %s %d>>>>>>>>>>\n",__func__,__LINE__);
    if (image_buf) {
                  printf("\n %s %d>>>>>>>>>>\n",__func__,__LINE__);
        if(product_read_bootscreens(0, image_buf, image_len) == 0){
 printf("\n %s %d>>>>>>>>>>\n",__func__,__LINE__);
            if(fd){
                      printf("\n %s %d>>>>>>>>>>\n",__func__,__LINE__);
                  dev_write(fd, image_buf, image_len);
                  printf("\n %s %d>>>>>>>>>>\n",__func__,__LINE__);
                  dev_ioctl(fd, VIDIOC_PLAY, 0);
                        printf("\n %s %d>>>>>>>>>>\n",__func__,__LINE__);
            }

        }
        free(image_buf);
    }
      printf("\n %s %d>>>>>>>>>>\n",__func__,__LINE__);


    extern void clear_media_list_data_buf();
    clear_media_list_data_buf();
    extern void clear_audio_list_data_buf();
    clear_audio_list_data_buf();

__exit:

    os_mutex_post(&dec_server_mutex);



}





#define PCM_TYPE_AUDIO      1
#define JPEG_TYPE_VIDEO     2
#define H264_TYPE_VIDEO     3

typedef struct cloud_media_info {
    struct list_head entry;
    int type;
    u32 len;
    u32 difftime;
    u8 is_key_frame;
    u8 *data;
} CLOUD_MEDIA_INFO;
// struct list_head name = LIST_HEAD_INIT(name)
//  struct list_head cloud_media_info_video_list_head={ &cloud_media_info_video_list_head,&cloud_media_info_video_list_head  }
static LIST_HEAD(cloud_media_info_video_list_head);
static LIST_HEAD(cloud_media_info_audio_list_head);
static OS_MUTEX cloud_media_info_video_list_mutex;
static OS_MUTEX cloud_media_info_audio_list_mutex;
static OS_SEM net_scr_dec_video_ctrl_sem;
static OS_SEM net_scr_dec_audio_ctrl_sem;
static int is_wait_iframe = 1;
static int video_frame_count;

void net_video_wrete_frame(int type, u32 len, u32 timestamp, u8 is_key_frame, char *buffer)
{
    CLOUD_MEDIA_INFO  *info = NULL;
   // printf("\n A1\n");
    os_mutex_pend(&cloud_media_info_video_list_mutex, 0);
  //  printf("\n A\n");
    if (is_wait_iframe && !is_key_frame) {
        os_mutex_post(&cloud_media_info_video_list_mutex);
     //   printf("\n B\n");
        return ;
    }
    if (video_frame_count >= 60) {
        os_mutex_post(&cloud_media_info_video_list_mutex);
      //  printf("\n C\n");
       // return;
    }
    info = (CLOUD_MEDIA_INFO *)calloc(1, sizeof(CLOUD_MEDIA_INFO) + len); //lbuf内申请一块空间
    if (info) {
      //   printf("\n D\n");
        info->type = type;
        info->is_key_frame = is_key_frame;
        info->difftime = timestamp;
        info-> len = len;
        info->data = malloc(len);
        memcpy(info->data, buffer, len);
        list_add_tail(&info->entry, &cloud_media_info_video_list_head);
    }

    video_frame_count++;
    is_wait_iframe = 0;
    if (video_frame_count >= 3 && start == 0) {
        os_sem_set(&net_scr_dec_video_ctrl_sem, 3);
        os_sem_post(&net_scr_dec_video_ctrl_sem);
      //  printf("\n E\n");
    } else if (start == 1) {
        os_sem_post(&net_scr_dec_video_ctrl_sem);
    }

    os_mutex_post(&cloud_media_info_video_list_mutex);
}

void net_audio_wrete_frame(int type, u32 len, u32 timestamp, u8 is_key_frame, char *buffer)
{
    CLOUD_MEDIA_INFO  *info = NULL;
    os_mutex_pend(&cloud_media_info_audio_list_mutex, 0);

    info = (CLOUD_MEDIA_INFO *)calloc(1, sizeof(CLOUD_MEDIA_INFO)); //lbuf内申请一块空间
    if (info) {
        info->type = type;
        info->is_key_frame = is_key_frame;
        info->difftime = timestamp;
        info-> len = len;

        info->data = malloc(len);
        memcpy(info->data, buffer, len);
        list_add_tail(&info->entry, &cloud_media_info_audio_list_head);
    }

    os_sem_post(&net_scr_dec_audio_ctrl_sem);

    os_mutex_post(&cloud_media_info_audio_list_mutex);
}




void clear_media_list_data_buf(){

    CLOUD_MEDIA_INFO  *p = NULL, *n = NULL;
    CLOUD_MEDIA_INFO  *info = NULL;


    while (1) {

        if (list_empty(&cloud_media_info_video_list_head)) {

            printf("\n list media entry is empty \n");
               break;
        }

        list_for_each_entry_safe(p, n, &cloud_media_info_video_list_head, entry) {
            list_del(&p->entry);
            break;
        }

        if (p) {
            if (p->type == H264_TYPE_VIDEO || p->type == JPEG_TYPE_VIDEO) {

                printf("\n >>>>>>>>>>>>>>>> clear media data <<<<<<<<<<<<<<\n");

                memset(p->data,0,p->len );
                continue;

            }
        }


     }

}


void clear_audio_list_data_buf(){

    CLOUD_MEDIA_INFO  *p = NULL, *n = NULL;
    CLOUD_MEDIA_INFO  *info = NULL;


    while (1) {

        if (list_empty(&cloud_media_info_audio_list_head)) {

            printf("\n list audio entry is empty \n");
               break;
        }

        list_for_each_entry_safe(p, n, &cloud_media_info_audio_list_head, entry) {
            list_del(&p->entry);
            break;
        }

        if (p) {
             if (p->type == PCM_TYPE_AUDIO) {

                printf("\n >>>>>>>>>>>>>>>> clear audio data <<<<<<<<<<<<<<\n");

                memset(p->data,0,p->len );
                continue;

            }
        }


     }

}


static int videox_disp_start(int id,const struct video_window *win)
{
    printf("\n %s %d\n", __func__, __LINE__);
#ifdef CONFIG_VIDEO0_ENABLE
    doorbell_video_disp_start(0,win);
#else

    int video3_disp_start(int sub_id,const struct video_window *win);
    video3_disp_start(id,win);

#endif // CONFIG_VIDEO0_ENABLE
    printf("\n %s %d\n", __func__, __LINE__);
}

static int videox_disp_stop(int id)
{
    printf("\n %s %d\n", __func__, __LINE__);
#ifdef CONFIG_VIDEO0_ENABLE
    doorbell_video_disp_stop(0);
#else

    video3_disp_stop(0);

#endif // CONFIG_VIDEO0_ENABLE
    printf("\n %s %d\n", __func__, __LINE__);
}



void net_video_dec_task(void *priv)
{
    int res;
    int msg[32];
    CLOUD_MEDIA_INFO  *info = NULL;
    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case APP_ANSWER_STATE:
                    printf("\n\n APP_ANSWER_STATE\n");

                    //hide 呼叫，显示通话中
                    notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"accept_call_without_video",0);
                    break;
                case APP_REFUSE_STATE:      //APP拒接
                     if(get_video_call_state() != 0){
                        printf("\n\n APP_REFUSE_STATE\n");
                        notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"app_refuse_calling",0);
                     }
                     break;
                case APP_ANSWER_TIMEOUT_STATE:  //拨号超时//需要收到定时器发送的超时
                    puts("\n APP_ANSWER_TIMEOUT_STATE\n");
                     if(get_video_call_state() != 0){
                        notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"calling_timeout",0);
                        avsdk_call_user_end();
                     }
                     break;
                case DEVICE_HANGUP_STATE:       //设备主动挂断
                     puts("\n DEVICE_HANGUP_STATE\n");
                     RT_TALK_INFO  tmp = {0};
                     extern u8 find_device_video_play_info(RT_TALK_INFO  *tmp);
                     if(find_device_video_play_info(&tmp)){
                        printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>pause  video\n");
                        avsdk_video_pause(tmp.conn_id,tmp.device_video_play_ch);
                        post_msg_doorbell_task("video_dec_task", 3, APP_STOP_VIDEO_CALL_STATE,DEVICE_HANGUP_STATE,tmp.conn_id);

                         post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "stop_call.adp");



                     }else if(find_audio_play_info() /*|| find_video_play_info()*/){
                        //要显示对方正忙，不允许挂断
                        printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>pause  video 2\n");

                        avsdk_video_pause(tmp.conn_id,tmp.device_video_play_ch);
                        post_msg_doorbell_task("video_dec_task", 3, APP_STOP_VIDEO_CALL_STATE,DEVICE_HANGUP_STATE,tmp.conn_id);
                        post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "stop_call.adp");

                     }else{
                        //还没有接通
                         post_msg_doorbell_task("doorbell_event_task", 2, DOORBELL_EVENT_PLAY_VOICE, "stop_call.adp");
                         printf("\n >>>>>>>>>>%s %d\n",__func__,__LINE__);
                         avsdk_call_user_end();
                         printf("\n >>>>>>>>>>%s %d\n",__func__,__LINE__);
                         post_msg_doorbell_task("video_dec_task", 3, APP_STOP_VIDEO_CALL_STATE,DEVICE_HANGUP_STATE,0);
                         printf("\n >>>>>>>>>>%s %d\n",__func__,__LINE__);
                     }


                     break;
                case APP_VIDEO_CALLING_STATE: {
                    //printf("\n\n APP_VIDEO_CALLING_STATE\n");
                    //  printf("\n K\n");
                    static CAL_INFO cal_info = {
                        .func = __func__,
                        .line = __LINE__,
                        .des = "dec",
                    };
                    calculate_frame_rate(&cal_info);


                    info = (CLOUD_MEDIA_INFO *)msg[2];
                    printf("\n >>>>>>>>>>>info->len = %d\n",info->len);
                    if (start == 0 && info->is_key_frame == 1) {
                        start = 1;
                        dec_server_write(info->data, info->len);
                    } else if (start == 1) {
                        dec_server_write(info->data, info->len);
                    }
                    free(info->data);
                    free(info);
                    break;
                }
                case APP_START_VIDEO_CALL_STATE: {

                    printf("\n\n APP_START_VIDEO_CALL_STATE\n");


                    if(get_lcd_pro_flag()){
                        struct sys_event event = {0};
                        event.arg = "VoiceCmd";
                        event.type = SYS_DEVICE_EVENT;
                        event.u.dev.event  = 1;
                        sys_event_notify(&event);
                    }
                    notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"app_accept_call",0);

                    //videox_disp_stop(0);
//                    int net_hide_main_ui(void);
//                    net_hide_main_ui();
                    os_sem_pend(&net_scr_dec_video_ctrl_sem,100);//等app数据过来

                    dec_server_start();
//                    struct video_window win = {0};
//                    win.left = 0;
//                    win.top = 0;
//                    win.width = 320;
//                    win.height = 240;
//                    videox_disp_start(0,&win);


                    break;
                }
                case APP_STOP_VIDEO_CALL_STATE: {
                    //videox_disp_stop(0);

                    printf("\n >>>>>>>>>>>>>APP_STOP_VIDEO_CALL_STATE\n");
                    int state = msg[2];
                    int conn_id = msg[3];
                    printf("\n %s %d  conn_id = %d \n",__func__,__LINE__,conn_id);
                    printf("\n >>>>>>>>>>>state  = %d \n",state);

                        printf("\n >>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
                        if(state == APP_REFUSE_STATE){
                            printf("\n >>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
                            if(get_video_call_state() == CALL_STA_TALKING || get_video_call_state() == CALL_STA_TALKING_WITHOUT_VIDEO){
                                printf("\n >>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
                                dec_server_stop(); //不使用大小窗不需要使用这个
                                notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"app_close_call",0);


                            }else{
                                printf("\n >>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
                                dec_server_stop(); //不使用大小窗不需要使用这个
                                if(get_video_call_state() == CALL_STA_STARTING){
                                    printf("\n >>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
                                    notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"app_refuse_calling",0);
                                }
                            }
                        }else{
                            printf("\n >>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
                              if(get_video_call_state() != 0){
                                printf("\n >>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
                                dec_server_stop(); //不使用大小窗不需要使用这个
                                notify_msg_to_video_rec(ACTION_VIDEO_REC_CHANGE_WORK,"machine_close_calling",0);
                              }
                        }
                        if(conn_id > 0){
                          printf("\n %s %d  conn_id = %d \n",__func__,__LINE__,conn_id);
                          del_video_play_info(conn_id);
                          del_audio_play_info(conn_id);
                          del_device_video_play_info(conn_id);
                          net_set_phone_status(0);
                          printf("\n\n\n %s find_device_video_play_info(&info) = %d  find_audio_play_info() = %d \n\n\n",__func__,find_device_video_play_info(&info), find_audio_play_info());
                        }

//                    struct video_window win = {0};
//                    win.left = 0;
//                    win.top = 0;
//                    win.width = 320;
//                    win.height = 240;
//                    videox_disp_start(0,&win);
                    break;
                }

                default:
                    break;
                }
                break;
            }
            break;
        default:
            break;
        }
    }
}


void net_audio_dec_task(void *priv)
{
    int res;
    int audio_start;
    int msg[32];
    CLOUD_MEDIA_INFO  *info = NULL;

    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case RT_AUDIO_CTRL_START:
                    audio_start = 1;
                    break;
                case RT_AUDIO_CTRL_WRITE: {
                    if(!audio_start){
                        break;
                    }
                    info = (CLOUD_MEDIA_INFO *)msg[2];
                    extern int cloud_rt_talk_write(char *data, int len);
                    cloud_rt_talk_write(info->data, info->len);
                    free(info->data);
                    free(info);
                    if(bindex == 0xff && audio_start){
                        //printf("\n\n >>>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
                        if(audio_dev){
                            dev_ioctl(audio_dev, AUDIOC_STREAM_ON, (u32)&bindex);
                            doorbell_event_dac_mute(1);
                        }
                    }
                    break;
                }
                case RT_AUDIO_CTRL_STOP:
                    if(audio_start){

                        if(bindex != 0xff){
                            if(audio_dev){
                                printf("\n\n >>>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
                                doorbell_event_dac_mute(0);
                                dev_ioctl(audio_dev, AUDIOC_STREAM_OFF, (u32)bindex);
                                bindex = 0xff;
                            }
                        }
                        printf("\n\n >>>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
                        aac_decode_uninit();
                        printf("\n\n >>>>>>>>>>>>>>>%s %d\n\n",__func__,__LINE__);
                        audio_start = 0;
                    }

                    break;
                default:
                    break;
                }
                break;
            }
            break;
        default:
            break;
        }
    }
}



void net_video_dec_ctrl_task(void *priv)
{
    CLOUD_MEDIA_INFO  *p = NULL, *n = NULL;
    CLOUD_MEDIA_INFO  *info = NULL;
    static int ms = 60;


    while (1) {
        os_sem_pend(&net_scr_dec_video_ctrl_sem, 0);
       //  printf("\n F\n");
        os_mutex_pend(&cloud_media_info_video_list_mutex, 0);

       //  printf("\n G\n");
        if (list_empty(&cloud_media_info_video_list_head)) {
            os_mutex_post(&cloud_media_info_video_list_mutex);
            continue;
        }
       //  printf("\n H\n");
        list_for_each_entry_safe(p, n, &cloud_media_info_video_list_head, entry) {
            list_del(&p->entry);
            break;
        }

        if (p) {
            // printf("\n I\n");
            if (p->type == H264_TYPE_VIDEO || p->type == JPEG_TYPE_VIDEO) {
                //printf("\n >>>>>>>>>>>>>>>>video_frame_count = %d\n", video_frame_count);
                video_frame_count--;
            // printf("\n J\n");
                post_msg_doorbell_task("video_dec_task", 2, APP_VIDEO_CALLING_STATE, p);
            }
        }
        os_mutex_post(&cloud_media_info_video_list_mutex);

        int delay_time;
        static int difftime = 0;
        if(video_frame_count >= 8){
            difftime = difftime >= 1 ? difftime - 1 :  0 ;
        }else if (video_frame_count >=6 && video_frame_count < 8) { //当缓存大于等于6但是小于8时不应该增加延时
            difftime += 0;
        }else if(video_frame_count < 6){ //当缓存小于6时，应该每次加3ms延时
            difftime += 3;
        }
        delay_time = 80 + difftime;

        msleep(delay_time);
        if (difftime >= 10) {
            difftime = 0;
        }
    }
}

void net_audio_dec_ctrl_task(void *priv)
{
    CLOUD_MEDIA_INFO  *p = NULL, *n = NULL;
    CLOUD_MEDIA_INFO  *info = NULL;

    while (1) {

        os_sem_pend(&net_scr_dec_audio_ctrl_sem, 0);
        os_mutex_pend(&cloud_media_info_audio_list_mutex, 0);
        if (list_empty(&cloud_media_info_audio_list_head)) {
            os_mutex_post(&cloud_media_info_audio_list_mutex);
            continue;
        }
        list_for_each_entry_safe(p, n, &cloud_media_info_audio_list_head, entry) {
            list_del(&p->entry);
            break;
        }
        if (p) {
            if (p->type == PCM_TYPE_AUDIO) {
                post_msg_doorbell_task("audio_dec_task", 2, RT_AUDIO_CTRL_WRITE, p);
            }
        }
        os_mutex_post(&cloud_media_info_audio_list_mutex);
    }
}


int net_media_dec_init()
{
    os_sem_create(&net_scr_dec_video_ctrl_sem, 0);
    os_sem_create(&net_scr_dec_audio_ctrl_sem, 0);
    os_mutex_create(&cloud_media_info_video_list_mutex);
    os_mutex_create(&cloud_media_info_audio_list_mutex);


    thread_fork("video_dec_ctrl_task", 20, 1000, 512, 0, net_video_dec_ctrl_task, NULL);
    thread_fork("audio_dec_ctrl_task", 20, 1000, 512, 0, net_audio_dec_ctrl_task, NULL);
    thread_fork("video_dec_task", 20, 1000, 512, 0, net_video_dec_task, NULL);
    thread_fork("audio_dec_task", 20, 1000, 512, 0, net_audio_dec_task, NULL);

    return 0;
}


