#include "asm/cpu.h"
#include "asm/adc.h"
#include "system/init.h"
#include "system/timer.h"

#define FIRST_CHANNEL() \
	adc_scan_begin

#define NEXT_CHANNEL(ch) \
	++ch >= adc_scan_end? adc_scan_begin:ch

static struct adc_scan *channel = FIRST_CHANNEL();
static u8 manual_flag = 0;
static u8 init = 0;

extern void adc_spin_lck();
extern void adc_spin_unlck();
static void adc_scan_process(void *p)
{
#if 0
    adc_spin_lck();
    if (!manual_flag) {
        channel = NEXT_CHANNEL(channel);
    }
    ADCSEL(channel->channel);
    KITSTART();
    while (!ADC_PND());
    if (!manual_flag) {
        channel->value = GPADC_RES;
    } else {
        manual_flag = 0;
    }
    /* printf(">>>>>>>>>>>>>>>>> ch: %d val:0x%x\n",channel->channel,channel->value); */
    adc_spin_unlck();
#else
    while (!ADC_PND());
    if (!manual_flag) {
        channel->value = GPADC_RES;
        if (channel->updata) {
            channel->updata();
        }
        channel = NEXT_CHANNEL(channel);
    } else {
        manual_flag = 0;
    }

    /* printf(">>>>>>>>>>>>>>>>> ch: %d val:0x%x\n",channel->channel,channel->value); */
    ADCSEL(channel->channel);
    KITSTART();
#endif
}


static int adc_scan_init()
{
    ADCSEL(channel->channel);
    ADC_EN(1);
    ADCEN(1);
    ADC_BAUD(0x7);
    ADC_WTIME(0x1);
    KITSTART();

    //LDO_CON |= BIT(11);

    sys_hi_timer_add(0, adc_scan_process, 4);
    init = 1;

    return 0;
}
platform_initcall(adc_scan_init);



u16 adc_scan_manual(u8 ch)
{
    if (!init) {
        return -1;
    }
    u16 val = 0;
    manual_flag = 1;
    while (!ADC_PND());
    /* log_d("adc_scan_manual ch%d\n",ch); */
    ADCSEL(ch);
    KITSTART();
    while (!ADC_PND());
    val = GPADC_RES;
    return val;
}

