#include "ntc_adc.h"

#define FIRST_CHANNEL() \
	adc_scan_begin

#define NEXT_CHANNEL(ch) \
	++ch >= adc_scan_end? adc_scan_begin:ch


static DEFINE_SPINLOCK(lock);

static struct adc_scan *channel = FIRST_CHANNEL();
static u8 manual_flag = 0;
static u8 init = 0;



extern void adc_spin_lck();
extern void adc_spin_unlck();




static void ldoin_updata();
static void ldoin_updata1();



REGISTER_ADC_SCAN(ad_red)
.channel = NTC_ADC_GPIO_CHANNEL,
 .value = 0,
  .updata = ldoin_updata,
};


static int ldoin_sum = 0;
static int ldoin_cnt = 0;
static int ldoin_sum1 = 0;
static int ldoin_cnt1 = 0;
static void ldoin_updata()
{
    spin_lock(&lock);
    ldoin_sum += ad_red.value;
    ldoin_cnt++;
    spin_unlock(&lock);
}

//  光敏adc采集
int get_ntc_voltage()
{
    static u16 in_val = 0;
    static u16 refer_val = 0;
    static u16 val = 0;

    if (cpu_irq_disabled()) {
        //  adc_scan_process();
        //  refer_val = ldo_vbg_scan.value;
        in_val = ad_red.value;
    } else {
        spin_lock(&lock);
        if (ldoin_cnt) {
            //  refer_val = ldo_vbg_sum / ldo_vbg_cnt;
            in_val = ldoin_sum / ldoin_cnt;
            // ldo_vbg_sum = 0;
            //  ldo_vbg_cnt = 0;
            ldoin_sum = 0;
            ldoin_cnt = 0;
        }
        spin_unlock(&lock);
    }
#if  0
    if (refer_val) {
#if ENABLE_SAMPLE_VAL
        const u16 vbg_volt = 129;  //��ֵ��Ҫͨ������valУ׼
        val = in_val * vbg_volt / refer_val * 3;
#else
        val = (in_val * 3 * LDO_REFERENCE_VOL + 0x181 * 2) / refer_val;
#endif
    }
#endif
    printf("\n in_val==============%d \n", in_val);

    // return 100;

    return in_val;
}

int ntc_adc_init()
{


    ad_red.channel = NTC_ADC_GPIO_CHANNEL;

    gpio_direction_input(NTC_ADC_GPIO);
    gpio_set_pull_up(NTC_ADC_GPIO, 0);
    gpio_set_pull_down(NTC_ADC_GPIO, 0);
    gpio_set_die(NTC_ADC_GPIO, 1);



    return 0;
}


#if 1
static int sw = 0;

void auto_test_ir_cut_state()
{


    if (get_ntc_voltage() >= 400) {



        turn_onoff_ir_cut(0);

    } else if (get_ntc_voltage() >= 160 && get_ntc_voltage() <= 400) {


        turn_onoff_ir_cut(0);
    } else if (get_ntc_voltage() < 160) {


        turn_onoff_ir_cut(1);
    }





}

#endif
