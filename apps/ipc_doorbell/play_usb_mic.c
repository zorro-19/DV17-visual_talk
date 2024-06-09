#include "system/includes.h"
#include "usb_host_user.h"
#include "app_config.h"

static struct device *dev;
static cbuffer_t cbuf;
static u8 bindex;

static u8 state;
static u8 restart;
static u8 running;

#define CBUF_SIZE     16000 * 2
static cbuf_space[CBUF_SIZE];



#ifdef ENABLE_SAVE_FILE_FOR_AEC_DNS

#include "doorbell_event.h"
#define LEN_TOTAL_DAC_BUF   1024*1024
u8 save_dac_buf[LEN_TOTAL_DAC_BUF]={0};
static int audio_len=0;
u8 save_file_flag=0;
u8 save_down_dac_buf[LEN_TOTAL_DAC_BUF]={0};
int audio_down_len=0;
u8 save_down_file_flag=0;


#endif


extern int net_video_rec_get_audio_rate();


static void usb_mic_recv_handler(u8 *buf, u32 len)
{
#ifdef CONFIG_USB_UAC_DEBUG
    static int t;
    static int total_len;
    if (t == 0) {
        t = timer_get_ms();
    }
    total_len += len;
    if (timer_get_ms() - t >= 1000) {
     //   printf("\n %s total_len = %d\n", __func__, total_len);
        total_len = 0;
        t = timer_get_ms();
    }
#endif


#ifdef ENABLE_SAVE_FILE_FOR_AEC_DNS

  //  putchar('r');
    if(!save_file_flag){
    if(audio_len<LEN_TOTAL_DAC_BUF){

    memcpy(save_dac_buf+audio_len, buf,len );

    audio_len+=len;

    }else{
    save_file_flag=1;

    post_msg_doorbell_task("audio_file_test_task", 1, 1);

    }



   }



#endif


    //只允许单通道的mic
    static int w_addr;
    static char tmpbuf[1024];

    if (w_addr + len <= 1024) {
        memcpy(tmpbuf + w_addr, buf, len);
        w_addr += len;
    }
    if (w_addr > 512) {
        static char data[512];
        memcpy(data, tmpbuf, 512);
        extern void usb_mic_data_input(u8 * buf, u32 len);
        usb_mic_data_input(data, 512);

        memcpy(data, tmpbuf + 512, w_addr - 512);
        memcpy(tmpbuf, data, w_addr - 512);
        w_addr = w_addr - 512;
    } else if (w_addr == 512) {
        extern void usb_mic_data_input(u8 * buf, u32 len);
        usb_mic_data_input(tmpbuf, 512);
        w_addr = 0;
    }
}


int play_usb_mic_start()
{
    int ret = 0;

    struct host_mic_attr mic_attr = {0};
    struct host_mic_ops mic_ops = {0};


    if (state) {
        return 0;
    }

//    cbuf_space = malloc(net_video_rec_get_audio_rate() * 2);
//    if (!cbuf_space) {
//        return -1;
//    }
    cbuf_init(&cbuf, cbuf_space, net_video_rec_get_audio_rate() * 2);
    usb_host_mic_get_attr(&mic_attr);
    printf("get usb mic attribute: %d %d %d %d %d\n", mic_attr.vol, mic_attr.ch, mic_attr.bitwidth, mic_attr.sr, mic_attr.mute);
    mic_attr.mute = 0;
    mic_attr.vol = 100;
    usb_host_mic_set_attr(&mic_attr);
    printf("set usb mic attribute: %d %d %d %d %d\n", mic_attr.vol, mic_attr.ch, mic_attr.bitwidth, mic_attr.sr, mic_attr.mute);
    mic_ops.recv_handler = usb_mic_recv_handler;
    usb_host_mic_set_ops(&mic_ops);

    ret = usb_host_mic_open();
    if (ret < 0) {
        ret = -1;
        goto __exit;
    }
    running = 1;
    return 0;

__exit:
    usb_host_mic_close();
//    if (cbuf_space) {
//        free(cbuf_space);
//        cbuf_space = NULL;
//    }
    return ret;
}

int play_usb_mic_stop()
{
    int ret = 0;
    struct host_mic_ops mic_ops = {0};
    u32 ot = jiffies + msecs_to_jiffies(100);

    if (!state) {
        return 0;
    }

    state = 0;

    mic_ops.recv_handler = NULL;
    usb_host_mic_set_ops(&mic_ops);
    usb_host_mic_close();
    cbuf_clear(&cbuf);
//    free(cbuf_space);
//    cbuf_space = NULL;
    return 0;
}


#ifdef ENABLE_SAVE_FILE_FOR_AEC_DNS
FILE *dac_fp;
FILE *down_fp;


void audio_file_test_task(void *priv)
{
    int res;
    int msg[32];

    static int dac_fp1_len ;
    static int dac_fp2_len ;
    static int adc_fp_len ;
    static int out_fp_len ;


    printf("\n  audio_file_test_task: %d,%s\n",__LINE__,__FUNCTION__);
    while(!(storage_device_ready())){

    printf("\n no sd in\n");
    os_time_dly(100);
    }

    while (1) {
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        printf("\n msg[1]====================%d\n\n",msg[1]);
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_USER:
                switch (msg[1]) {
                case 0:

                    break;
                case 1:

                  #ifdef ENABLE_SAVE_FILE_FOR_AEC_DNS
                    printf("\n up_fp============%d\n",dac_fp);
                    if(!dac_fp)
                    {
                     dac_fp = fopen(CONFIG_ROOT_PATH"up***.pcm", "w+");
                    }
                    if(dac_fp){
                    fwrite(dac_fp, &save_dac_buf[0],sizeof(save_dac_buf));
                    memset(&save_dac_buf[0],0,sizeof(save_dac_buf));
                    }

                    if(dac_fp){
                    fclose(dac_fp);
                    dac_fp=NULL;
                    }
                  audio_len=0;
                  save_file_flag=0;
                  #endif



                    break;
                case 2:

                #ifdef ENABLE_SAVE_FILE_FOR_AEC_DNS
                    printf("\n  down_fp============%d\n",down_fp);
                    if(!down_fp)
                    {
                     down_fp = fopen(CONFIG_ROOT_PATH"down***.pcm", "w+");
                    }
                    if(down_fp){
                    fwrite(down_fp, &save_down_dac_buf[0],sizeof(save_down_dac_buf));
                    memset(&save_down_dac_buf[0],0,sizeof(save_down_dac_buf));
                    }

                    if(down_fp){
                    fclose(down_fp);
                    down_fp=NULL;
                    }
                  audio_down_len=0;
                  save_down_file_flag=0;
                #endif

                    break;
                case 4:


                 #if 0// ENABLE_SAVE_FILE_FOR_AEC_DNS
                    printf("\n =================file  is ok=======================%d\n");
                    printf("\n  file is save ok  \n");
                    if(mix_fp){
                    fclose(mix_fp);
                     mix_fp=NULL;
                    }


                    printf("\n file save ok\n");
                #endif
                    break;


                }
                break;
            default:
                break;
            }
            break;
        default:


            break;
        }
    }




}

void file_test_init()
{

    thread_fork("audio_file_test_task", 25, 0x2E00, 4096 * 3, 0, audio_file_test_task, NULL);


}

late_initcall(file_test_init);
#endif
