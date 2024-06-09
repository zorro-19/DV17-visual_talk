#if  1
#include "system/includes.h"
#include "server/ui_server.h"
#include "action.h"
#include "ani_style.h"
#include "style.h"
#include "res_ver.h"
#include "app_config.h"
#include "gSensor_manage.h"
#include "video_rec.h"
#include "asm/rtc.h"
#include "storage_device.h"
#include "system/spinlock.h"
#include "asm/imc_driver.h"
#include "asm/mpu.h"
#include "key_commucation.h"
#include "deal_function_state.h"


static char power_off_flag = 0;



static int sysctr_task_pid;

void auto_power_down()
{
    printf("\n <<<<<<<<<<<<<<<<<<<<<<auto_power_down>>>>>>>>>>>>>>>>>>>>\n ");
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg = "sys_power";
    e.u.dev.event = DEVICE_EVENT_SPECIAL_POWER_SHUTDOWN;
    sys_event_notify(&e);


}


void std_power_down()
{
    printf("\n gip_power_down>>>>>>>>>>>>>>>>>>>>\n ");
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg = "sys_power";
    e.u.dev.event = DEVICE_EVENT_STD_POWER_SHUTDOWN;
    sys_event_notify(&e);


}

#ifdef CONFIG_UI_ENABLE
#include "server/video_server.h"

#include "video_rec.h"





extern void video_rec_post_msg(const char *msg, ...);
extern void video_photo_post_msg(const char *msg, ...);
void  mode_switch_power_off()
{



    if (get_rec_menu_state() || get_photo_menu_state() || get_dec_menu_state()) {

        printf("\n  is menu state>>>>>>>>>>>>>>\n");
        video_sys_post_msg("re_sys");
    }



    struct application *app;
    app = get_current_app();
    if (app) {
        if (!strcmp(app->name, "video_rec")) {

            printf("\n <<<<<<<<<<<<<<<<<rec power sharnk>>>>>>>>>>>>>>>>>>>>>>>\n ");
            video_rec_post_msg("HlightOn");//倒计时显示
        } else if (!strcmp(app->name, "video_photo")) {

            printf("\n <<<<<<<<<<<<<<<<<<photo power sharnk>>>>>>>>>>>>>>>>>>>>>>>\n ");
            video_photo_post_msg("HlightOn");//倒计时显示
        } else if (!strcmp(app->name, "video_dec")) {


            struct intent it;


            it.name = "video_dec";
            init_intent(&it);
            it.action = ACTION_BACK;
#if  0
            start_app(&it);
#else
            start_app_async(&it, NULL, NULL);
#endif


            switch (db_select("mode")) {

            case  0:
            case  2:


                set_dec_to_disp_poweroff(1);
                it.name = "video_photo";
                it.action = ACTION_PHOTO_DISP_MAIN ;
#if  1
                start_app(&it);
#else
                start_app_async(&it, NULL, NULL);
#endif



                printf("\n photo power sharnk>>>>>>>>>>>>>>>>>>>>>>>\n ");
                video_photo_post_msg("HlightOn");//倒计时显示
                break ;



            case  1:


                set_dec_to_disp_poweroff(1);
                it.name = "video_rec";
                it.action = ACTION_AUTO_VIDEO_REC_DISP;
#if  0
                start_app(&it);
#else
                start_app_async(&it, NULL, NULL);
#endif


                printf("\n rec power sharnk>>>>>>>>>>>>>>>>>>>>>>>\n ");
                video_rec_post_msg("HlightOn");//倒计时显示
                break ;



            default:
                set_dec_to_disp_poweroff(1);
                it.name = "video_photo";
                it.action = ACTION_PHOTO_DISP_MAIN ;
#if  0
                start_app(&it);
#else
                start_app_async(&it, NULL, NULL);
#endif
                break;



            }





        }
    }






}


void  mode_menu_set_close_delay_off()
{


    struct application *app;
    app = get_current_app();
    if (app) {
        if (!strcmp(app->name, "video_rec")) {

            // printf("\n close rec sharnk>>>>>>>>>>>>>>>>>>>>>>>\n ");
            video_rec_post_msg("HlightOff");//倒计时显示
        } else if (!strcmp(app->name, "video_photo")) {

            //  printf("\n close photo  sharnk>>>>>>>>>>>>>>>>>>>>>>>\n ");
            video_photo_post_msg("HlightOff");//倒计时显示
        }
    }






}




#endif

static int  timer_handle = 0;
static u8 run_once = 0;



// 1 为菜单设置状态， 0 为 工作或者关机状态
int get_menu_state()
{


    return  gpio_read(MENU_ON_SET_GPIO);
}
// off 档位

int switch_off_state()
{


    return  gpio_read(BL_OFF_GPIO);
}

//  work档位
int switch_work_on_state()
{

    return !gpio_read(WORKING_MODE_GPIO);



}
// menu 档位
int lcd_disp_busy_state()
{


    return  !gpio_read(LCD_DISP_BUSY_GPIO);

}
// led 照明灯  1 关闭  0 打开
int  set_led_state(u8 state)
{

    printf("\n  !state ==================%d\n", !state);
    gpio_direction_output(WORKING_LED_GPIO, !state);


}
//  档位按键初始化
void  init_gpio()
{

    static unsigned char init = 0;
    if (!init) {
        init = 1;
        gpio_direction_input(MENU_ON_SET_GPIO);//  PR1
        gpio_set_pull_up(MENU_ON_SET_GPIO, 0);
        gpio_set_pull_down(MENU_ON_SET_GPIO, 0);
        gpio_set_die(MENU_ON_SET_GPIO, 1);

        gpio_direction_input(BL_OFF_GPIO);  //  PR2
        gpio_set_pull_up(BL_OFF_GPIO, 0);
        gpio_set_pull_down(BL_OFF_GPIO, 0);
        gpio_set_die(BL_OFF_GPIO, 1);

        gpio_direction_input(WORKING_MODE_GPIO);  // IO_PORTH_10
        gpio_set_pull_up(WORKING_MODE_GPIO, 1);
        gpio_set_pull_down(WORKING_MODE_GPIO, 0);
        gpio_set_die(WORKING_MODE_GPIO, 1);


        gpio_direction_input(LCD_DISP_BUSY_GPIO);  //  IO_PORTH_13
        gpio_set_pull_up(LCD_DISP_BUSY_GPIO, 1);
        gpio_set_pull_down(LCD_DISP_BUSY_GPIO, 0);
        gpio_set_die(LCD_DISP_BUSY_GPIO, 1);



    }




}



// 按键检测task

void thread_key_ctr()
{
    //struct application *app;
    //static int chargePicFlag = 0;
    int send_timer = 0, k1_value = 0, k2_value = 0, k1_first = -1, k2_first = 0, manual_k2 = 0;
    u8 mode_on = 0;


//static u8 once_flag=0;

    /********************** 需要用的IO 必须初始化，不然读取状态不准***********************/
    init_gpio();

#if 1
    while (1) {




        // gpio_set_pull_down(BL_OFF_GPIO,1);
        manual_k2 = gpio_read(LCD_DISP_BUSY_GPIO);//PA05 MODE_OFF为低   PR1 为高   菜单项设置

        // printf ("manual_k2(PH11) = %d\r\n", manual_k2);


        mode_on = gpio_read(WORKING_MODE_GPIO); // MODE_ON , IO_PORTA_08  为低，PR1 为高 工作状态


        k1_value = gpio_read(MENU_ON_SET_GPIO); //PR1
        if (-1 == k1_first) {
            k1_first = k1_value;
        }

        k2_value = gpio_read(BL_OFF_GPIO);//PR2

        // printf ("k1 = %d, k2 = %d\r\n", k1_value, k2_value);
        if (0 == manual_k2) {

            //  printf ("\n set_menu>>>>>>\r\n");

        }
        //k1_first=gpio_read(MENU_ON_SET_GPIO);
        //  printf ("k2 = %d,k1_first:%d,mode_on:%d\r\n", k2_value,k1_first,mode_on);

        if (0 == mode_on && 1 == k1_value) {


            //    printf("\n k1_first:::%d,power_off_flag:%d\n ",k1_first,power_off_flag);


            if (0 == manual_k2) {
                // 进入关机模式 可能是从中间档位拨到关机档位

                //once_flag=1;
                //    printf ("\n 1111start_turn k2 to 1, to power off\r\n");

                //   printf("\n  manual_k2=====================%d\n ",manual_k2);
                msleep(1000);
                auto_power_down();

                msleep(2000);



                msleep(50);
            } else {

                if (0 == mode_on && k1_value == 1) { //  进入工作模式，进关机流程跑完关机logo 时,发关机命令给mcu
                    // printf (" 222start_manual quick turn to k2, 3 seconds countdown to sleep\r\n");



                    //once_flag=1;

#if  1
                    printf("\n get_wakeup_flag():::::%d\n", get_wakeup_flag());
                    if (get_wakeup_flag()) {

                        goto exit ;

                    }

#endif



                    while (1) {



                        printf("\n  <<while>>>>>>>>>>>>>>>>>>>>>>>>\n");
                        if (!run_once) {





                            run_once = 1;
                            mode_switch_power_off();

                            // sys_timer_add(NULL, send_poweroff_command_func, 1* 1000);
                            for (int i = 0; i < 6; i++) {
                                msleep(1000);
                                //   printf("\n gpio_read(BL_OFF_GPIO):%d\n  ",gpio_read(BL_OFF_GPIO));
                                //   printf("\n gpio_read(LCD_DISP_BUSY_GPIO):%d\n  ",gpio_read(LCD_DISP_BUSY_GPIO));
                                //   printf("\n gpio_read(MENU_ON_SET_GPIO):%d\n  ",gpio_read(MENU_ON_SET_GPIO));

                                printf("\n delay 1s\n");

                                if (lcd_disp_busy_state() || switch_off_state()) { //  防止倒计时关机时拨动开关到其他档位，退出当前倒计时状态
                                    run_once = 0;
                                    printf("\n  <break_working0>\n");
                                    mode_menu_set_close_delay_off();

                                    break ;
                                }



                            }
                        }

                        if (!gpio_read(MENU_ON_SET_GPIO) || !gpio_read(LCD_DISP_BUSY_GPIO)) { // 无用 ？


                            printf("\n  <break_working1>\n");
                            run_once = 0;
                            break ;
                        }
#if  1
                        if (get_workmode_bit() == 1) { //工作状态不允许走 thread_key_ctr 任务按键检测方式去关机，走其他方式关机


                            printf("\n  <get_workmode_bit>\n");
                            run_once = 0;
                            break ;
                            set_workmode_bit(0);
                            goto exit ;

                        }

#endif
                        printf("\n  check key  power off ");
                        auto_power_down();


                        msleep(1000);




                    }




                } else {
                    //    once_flag=1;
                    printf("PIR wakeup...don't power off\r\n");
                }
            }
        } else {

            if (gpio_read(BL_OFF_GPIO)) { //  PR2 口  ，拨到关机档位


                printf("\n  <<<<<<<<<<<<<<<<<<<<power offf>>>>>>>>>>>>>>\n");
                set_workmode_bit(2);
                auto_power_down();

                printf("\n  turn off key \n ");
            }

            if (lcd_disp_busy_state()) { // 工作中突然拨动到 menu档位
                if (get_workmode_bit())  {
                    //    video_rec_stop(0);
                    std_power_down();
                }

                //printf("\n menu on \n");

            }


        }


exit:



        msleep(500);
    }

#endif


}







#endif
