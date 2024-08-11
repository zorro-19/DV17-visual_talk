#include "asm/adkey.h"
#include "asm/adc.h"
#include "generic/gpio.h"
#include "device/key_driver.h"
#include "device/device.h"
#include "system/init.h"
#include "system/database.h"
#include "system/timer.h"
#include "doorbell_event.h"
#include "system/spinlock.h"
extern int os_taskq_post(const char *name, int argc, ...);
extern int ircut_get_isp_scenes_flag(void);
extern u8 get_video_rec_state(void);


#define SWITCH_THRESHOLD    2    //1s 切换
static int ircut_status = -1; // -1:表示初始状态 0:表示关，1表示开
void set_ircut_status(int status)
{
    ircut_status = status;
}
#ifdef CONFIG_IRCUT_ENABLE

static DEFINE_SPINLOCK(ntclock);
void ntc_in_updata();

REGISTER_ADC_SCAN(light_adc_scan)
.value = 0,
 .channel = 3,
  .updata = ntc_in_updata,
};

static int ntcin_sum = 0;
static int ntcin_cnt = 0;
static int count_adc_numbers=0;
static void ntc_in_updata()
{
    spin_lock(&ntclock);
    ntcin_sum += light_adc_scan.value;
    ntcin_cnt++;

    count_adc_numbers++;
   // printf("\n d:%d\n",count_adc_numbers);
    if(count_adc_numbers%(10*15)==0){
    // puts('a');
     ntcin_sum = 0;
     ntcin_cnt = 0;
     count_adc_numbers=0;
    }


    spin_unlock(&ntclock);
}



//  光敏adc采集
int get_ntc_voltage()
{
    static u16 in_val = 0;
    static u16 refer_val = 0;
    static u16 val = 0;

    if (cpu_irq_disabled()) {

        in_val = light_adc_scan.value;
    } else {
        spin_lock(&ntclock);
        if (ntcin_cnt) {

            in_val = ntcin_sum / ntcin_cnt;

            ntcin_sum = 0;
            ntcin_cnt = 0;
        }
        spin_unlock(&ntclock);
    }

  //printf("\n in_val=====%d \n", in_val);



    return in_val;
}

u8 isp_state=0,pre_isp_state=0;
u8 set_isp_effect_state(u8 state){



    isp_state=state;
}


u8 get_isp_effect_state(){


    return isp_state;

}
u8 set_pre_isp_effect_state(u8 state){



    pre_isp_state=state;
}

u8 get_pre_isp_effect_state(){


    return pre_isp_state;

}

void light_adc_value_check(void *priv)
{
    static int night_cnt, day_cnt;
    static int is_night = 0;//0:表示白天 ：1表示黑夜
    static int night_state;
    static int vm_ircut_state_=0;// 保存 ircut 状态
    if (ircut_get_isp_scenes_flag()) {
        return;
    }
    extern u8 get_poweron_net_config_state();
    if(get_poweron_net_config_state()==0){


       printf("\n is config net \n");
       return ;

    }

    #if  1
 // printf("\n  light_adc_scan.value======%d,get_ntc_voltage():%d\n",light_adc_scan.value,get_ntc_voltage());
// if (light_adc_scan.value <= 300) {

    if(is_night){

     if(get_ntc_voltage()<350){ //防止频繁切换

     if(db_select("ircut")==vm_ircut_state_){

         return ;

        }
     }
    }
    if (get_ntc_voltage() <= 300) {


        night_cnt++;
        if (night_cnt > SWITCH_THRESHOLD) {
            night_cnt = 0;
            day_cnt = 0;
            is_night = 1;

            set_isp_effect_state(2); // 夜晚效果
        }
    } else {  // 280- 400,  400



        day_cnt++;
        if (day_cnt > SWITCH_THRESHOLD) {
            night_cnt = 0;
            day_cnt = 0;
            is_night = 0;

         #if  0 //黄昏场景
            if(get_ntc_voltage()<=400){

            set_isp_effect_state(0);//黄昏

            }else{


            set_isp_effect_state(1);//白天


            }
         #else

              set_isp_effect_state(1);//白天

         #endif

        }
    }
    u8 ircut = db_select("ircut");    /** 1 常开；2 常关； 3 自动*/

  //  printf("\n  ===========page====ircut:%d,ircut_status:%d,%d,%s\n",ircut,ircut_status,__LINE__,__FUNCTION__);
    if (ircut == 1) {
        if (ircut_status != IRCUT_ON) {
            ircut_status = IRCUT_ON;
            vm_ircut_state_=ircut;
            post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_IRCUT_ON);
        }
    } else if (ircut == 2) {

        if(is_night==1){
            set_isp_effect_state(0);
        }
        if ((get_isp_effect_state()!=get_pre_isp_effect_state())||is_night==1) {
            ircut_status = IRCUT_OFF;
            vm_ircut_state_=ircut;
            post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_IRCUT_OFF);
        }
    } else if (ircut == 3) {
        if (ircut_status != IRCUT_ON && is_night == 1) {
            ircut_status = IRCUT_ON;
            vm_ircut_state_=ircut;
            post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_IRCUT_ON);
        } else if ( ( is_night == 0)&&(get_isp_effect_state()!=get_pre_isp_effect_state()) ){
            ircut_status = IRCUT_OFF;
            vm_ircut_state_=ircut;
            post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_IRCUT_OFF);
        }
    }

   #else

   static u8  flasg=0;

    flasg ^=BIT(0)  ;
    printf("\n flasg:%d\n",flasg);

     printf("\n  light_adc_scan.value ================%d\n",light_adc_scan.value);
     if(flasg){
      post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_IRCUT_ON);
    }else{

      post_msg_doorbell_task("doorbell_event_task", 1, DOORBELL_EVENT_IRCUT_OFF);

    }
   #endif




}

int light_adc_init(void)
{

    static int light_adc_init_flag = 0;
    if (light_adc_init_flag) {
        return 0;
    }
    light_adc_init_flag = 1;
    extern void light_adc_channel_init(void);
    light_adc_channel_init();


    sys_timer_add_to_task("sys_timer", NULL, light_adc_value_check, 200);

    printf("\n light_adc_init \n");
   // sys_timer_add_to_task("sys_timer", NULL, light_adc_value_check, 2000);
   // sys_timer_add_to_task("sys_timer", NULL, light_adc_value_check, 1000);

    return 0;
}
#else
int light_adc_init(void)
{

}
#endif // CONFIG_IRCUT_ENABLE
//late_initcall(light_adc_init);
