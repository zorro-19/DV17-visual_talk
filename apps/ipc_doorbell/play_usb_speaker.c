#include "app_config.h"
#include "system/includes.h"
#include "usb_host_user.h"
#include "doorbell_event.h"

static struct device *dev;
static u8 bindex;
static u8 *audio_buf;
static u8 spk_ch;
static u32 buf_offset;
static struct video_buffer b;
static u8 running;

#ifdef ENABLE_SAVE_FILE_FOR_AEC_DNS

#include "doorbell_event.h"
#define LEN_TOTAL_DAC_BUF   1024*1024

extern u8 save_down_dac_buf[LEN_TOTAL_DAC_BUF];

extern int audio_down_len;
extern u8 save_down_file_flag;


#endif




#define USB_AUDIO_BUF_SIZE          (9600)
#define  DAC_DEC_LEN    32
static u8 tmp_data1[3 * DAC_DEC_LEN];

static u32 usb_speaker_get_stream_data(u8 *buf, u32 len)
{
    int tmp_data1_len;

#ifdef CONFIG_USB_UAC_DEBUG
    static int t;
    static int total_len;
    if (t == 0) {
        t = timer_get_ms();
    }
    total_len += len;
    if (timer_get_ms() - t >= 1000) {
    //    printf("\n %s total_len = %d\n", __func__, total_len);
        total_len = 0;
        t = timer_get_ms();
    }
#endif
    int ret;
    ASSERT(len <= DAC_DEC_LEN,"\n Please modify DAC_ DEC_ LEN \n" )

#if (CONFIG_AUDIO_FORMAT_FOR_SPEARK == AUDIO_FORMAT_AAC)

#if (CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK != CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC)


    tmp_data1_len = 3 * len;



    memset(buf, 0, len);
    ret = user_dec_get_data(tmp_data1, tmp_data1_len);
    if (ret) {
        pcm_change_sampleRate(CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK, CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC, tmp_data1, tmp_data1_len, buf, len);
    }

#else


    memset(buf, 0, len);
    ret = user_dec_get_data(buf, len);


    #ifdef ENABLE_SAVE_FILE_FOR_AEC_DNS

  //  putchar('r');
    if(!save_down_file_flag){
    if(audio_down_len<LEN_TOTAL_DAC_BUF){

    memcpy(save_down_dac_buf+audio_down_len, buf,len );

    audio_down_len+=len;

    }else{
    save_down_file_flag=1;

    post_msg_doorbell_task("audio_file_test_task", 1, 2);

    }



   }

    #endif



#endif // CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK

#else



#if (CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK != CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC)

    memset(buf, 0, len);
    tmp_data1_len = 3 * len;
    user_dec_get_data(tmp_data1, tmp_data1_len);
    pcm_change_sampleRate(CONFIG_AUDIO_SAMPLE_RATE_FOR_SPEARK, CONFIG_AUDIO_SAMPLE_RATE_FOR_MIC, tmp_data1, tmp_data1_len, buf, len);
#else
    extern int user_dec_get_data(u8 * data, int len);
    memset(buf, 0, len);
    user_dec_get_data(buf, len);
#endif

#endif

    return len;
}

int play_usb_speaker_start()
{
    int ret = 0;
    struct host_speaker_attr spk_attr = {0};
    struct host_speaker_ops spk_ops = {0};
    printf("\n play_usb_speaker_start \n");
    usb_host_speaker_get_attr(&spk_attr);
    printf("get usb speaker attribute: %d, %d, %d, %d, %d, %d\n", spk_attr.vol_l, spk_attr.vol_r, spk_attr.ch, spk_attr.bitwidth, spk_attr.sr, spk_attr.mute);
    spk_attr.mute = 0;
    spk_attr.vol_l = 100;
    spk_attr.vol_r = 100;
    /* spk_attr.sr = 48000; */
    usb_host_speaker_set_attr(&spk_attr);
    printf("set usb speaker attribute: %d, %d, %d, %d, %d, %d\n", spk_attr.vol_l, spk_attr.vol_r, spk_attr.ch, spk_attr.bitwidth, spk_attr.sr, spk_attr.mute);
    spk_ops.get_stream_data = usb_speaker_get_stream_data;
    usb_host_speaker_set_ops(&spk_ops);

    spk_ch = spk_attr.ch;

    ret = usb_host_speaker_open();
    if (ret) {
        ret = -1;
        goto __exit;
    }


    return 0;

__exit:
    usb_host_speaker_close();

    return ret;
}

int play_usb_speaker_stop()
{
    int ret;
    printf("\n play_usb_speaker_stop \n");
    struct host_speaker_ops spk_ops = {0};
    spk_ops.get_stream_data = NULL;
    usb_host_speaker_set_ops(&spk_ops);
    usb_host_speaker_close();

    return 0;
}


