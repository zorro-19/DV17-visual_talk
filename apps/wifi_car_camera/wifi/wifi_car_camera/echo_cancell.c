#include "app_config.h"
#include "printf.h"
#include "circular_buf.h"
#include "device/audio_dev.h"
#include "os/os_api.h"
#include "os/os_compat.h"
#include "system/init.h"


#define DAC_8KHZ	8


#ifdef ECHO_CANCELL_ENABLE

int BlueTooth_Calling_QueryBufSize(void);
int BlueTooth_Calling_init(void *buf, int var1, int var2, const int *eq_tab);
int BlueTooth_Calling_run(void *ptr, short *mic_buf, short *far_buf, short *out, int *cur_gain);
int BlueTooth_Calling_SetVariable(void *ptr, const char *VarName, int Var);
int BlueTooth_Calling_GetVariable(void *ptr, const char *VarName, int *var);
const char **BlueTooth_Calling_GetVariableList(int *nVar);
//DisableBit:
/*
0:NLMS
1:NLP
2:ANS
3:ComfortNoise
4:TwoPath
5:ARS
6:UseCohDE


//DisableBit:
0: AEC
1: Echo Suppress
2: Noise Reduce
3: Comfort Noise
4: Two Path (define for earphone,default enable)
5: ARS      (define for speaker,when mic and loudspeaker in the same space)
6: UseCohDE (define for speaker,another Echo Suppress coefficient)

*/



#if 0
static const int eq_tab[] = {
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768,
};
#endif

static char echo_cancell_state;
static char echo_cancell_buf[18000];
static short ref_dac_buf[128];


static OS_SEM adc_sem;
static OS_SEM dac_sem;


static u8 adc_cbuf_buffer[4 * 4 * 1024];
static u8 dac_cbuf_buffer[4 * 4 * 1024];
static u8 mux_cbuf_buffer[4 * 4 * 1024];
static cbuffer_t adc_cbuf_handler;
static cbuffer_t dac_cbuf_handler;
static cbuffer_t mux_cbuf_handler;

static void echo_cancell_task(void *arg)
{
    u32 adc_len, dac_len, mux_len;
    short adc_buf[128];
    short dac_buf[128];
    short mux_buf[128];
    while (1) {
        adc_len = cbuf_read(&adc_cbuf_handler, (u8 *)adc_buf, sizeof(adc_buf));
        if (adc_len < sizeof(adc_buf)) {
            /* putchar('8'); */
            os_sem_pend(&adc_sem, 0);
            continue;
        }
        memset(dac_buf, 0, sizeof(dac_buf));
        dac_len = cbuf_read(&dac_cbuf_handler, (u8 *)dac_buf, sizeof(dac_buf));
        if (dac_len < sizeof(dac_buf)) {
            /* putchar('7'); */
            /* os_sem_pend(&dac_sem,0); */
            /* continue; */
        }



        BlueTooth_Calling_run(echo_cancell_buf, adc_buf, dac_buf, mux_buf, NULL); //200+ us


        /* put_buf(mux_buf,32); */
        mux_len = cbuf_write(&mux_cbuf_handler, (u8 *)mux_buf, sizeof(mux_buf));
        if (mux_len == 0) {
            putchar('U');
            cbuf_clear(&mux_cbuf_handler);
        }

    }

}


static void echo_cancell_init(void)
{
    int suppress_coeff1 = 2, suppress_coeff2 = 0;//1000;
    int aec_ctl = 0;
    int ret = 0;

    while (sizeof(echo_cancell_buf) < BlueTooth_Calling_QueryBufSize()) {
        printf("echo_cancell_buf not enough = %d,  \r\n", sizeof(echo_cancell_buf), BlueTooth_Calling_QueryBufSize());
    }

//BlueTooth_Calling_init( echo_cancell_buf, suppress_coeff1, suppress_coeff2, eq_tab );
    BlueTooth_Calling_init(echo_cancell_buf, suppress_coeff1, suppress_coeff2, NULL);
    BlueTooth_Calling_SetVariable(echo_cancell_buf, "NLPClearThr", suppress_coeff2);
    BlueTooth_Calling_SetVariable(echo_cancell_buf, "AnalogAGC_BaseGain", 0);
    BlueTooth_Calling_SetVariable(echo_cancell_buf, "DisableFlag", aec_ctl);

    ret = os_sem_create(&adc_sem, 0);
    if (ret != OS_NO_ERR) {
        printf("OS_SEM create fail\n");
    }

    cbuf_init(&adc_cbuf_handler, adc_cbuf_buffer, 4 * 4 * 1024);
    cbuf_init(&dac_cbuf_handler, dac_cbuf_buffer, 4 * 4 * 1024);
    cbuf_init(&mux_cbuf_handler, mux_cbuf_buffer, 4 * 4 * 1024);



    thread_fork("echo_cancell_task", 10, 0x500, 0, 0, echo_cancell_task, NULL);
//BlueTooth_Calling_SetVariable(echo_cancell_buf,"NLPClearThr",value[i]);
//BlueTooth_Calling_SetVariable(echo_cancell_buf,"SpeechSuppress",value[i]);
}

late_initcall(echo_cancell_init);


void dac_irq_output(u8 sample_rate, short *dac_buf, int len)
{
    u32 dac_len;
    if (sample_rate != DAC_8KHZ) {
        printf("echo_cancell dac sample_rate must be 8KHZ ,,, %d\r\n", sample_rate)	;
        return;
    }

    if (len == 512) {
        u32 i;
        for (i = 0; i < len / 4; i++) {
            ref_dac_buf[i] = dac_buf[i * 2];
        }
    }

    /* if(echo_cancell_state) { */
    dac_len = cbuf_write(&dac_cbuf_handler, (u8 *)ref_dac_buf, sizeof(ref_dac_buf));
    if (dac_len == 0) {
        putchar('B');
        cbuf_clear(&dac_cbuf_handler);
    }

    /* os_sem_post(&dac_sem); */
    /* } */

    /*else if(len == 256)*/
    /*{*/
    /*memcpy(ref_dac_buf,dac_buf,len);	*/
    /*}*/
}



void adc_irq_input(u8 sample_rate, short *adc_buf, int len, short **out_buf)
{
    static short res_buf[128];
    u32 adc_len, mux_len;
    if (sample_rate != DAC_8KHZ) {
        printf("echo_cancell adc sample_rate must be 8KHZ ,,, %d\r\n", sample_rate)	;
        return;
    }

    if (len != 256) {
        printf("Calling_run buf_len must 256*,,,%d\r\n", len);
        return ;
    }

    /* if (echo_cancell_state == 0) { */
    /* echo_cancell_init(); */
    /* echo_cancell_state = 1; */
    /* } */

    /* if(echo_cancell_state) { */
    adc_len = cbuf_write(&adc_cbuf_handler, (u8 *)adc_buf, len);
    if (adc_len == 0) {
        putchar('A');
        cbuf_clear(&adc_cbuf_handler);
    }
    os_sem_post(&adc_sem);


    mux_len = cbuf_read(&mux_cbuf_handler, (u8 *)res_buf, sizeof(res_buf));
    if (mux_len < sizeof(res_buf)) {
        putchar('L');
        memset(res_buf, 0, sizeof(res_buf));
    }
    /* } */


    /* BlueTooth_Calling_run(echo_cancell_buf, adc_buf, ref_dac_buf, res_buf, NULL); //200+ us */
    /* memset(ref_dac_buf, 0, sizeof(ref_dac_buf)); */

    *out_buf = res_buf;
}

#endif


