
#include "deal_function_state.h"


static dec_to_disp_poweroff = 0;
void  set_dec_to_disp_poweroff(u8 is)
{


    dec_to_disp_poweroff = is;

}

int get_dec_to_disp_poweroff_flag()
{


    return  dec_to_disp_poweroff;

}

// ����PR �ڻ��ѷ�ʽ
void set_poweroff()
{



    printf("\n  set_pr_state_to power off \n");
    sys_power_set_port_wakeup("wkup_gsen", 1);

    sys_power_set_port_wakeup("wkup_usb", 1);
    if (db_select("delay_pv")) {
        sys_power_set_port_wakeup("wkup_pir", 0);
    } else {
        sys_power_set_port_wakeup("wkup_pir", 1);
    }
    sys_power_poweroff(0);
}

static void get_sys_time(struct sys_time *time)
{
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        memset(time, 0, sizeof(*time));
        return;
    }
    dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)time);
    dev_close(fd);
}
// ���ڷ��������ִ���״̬�����ڹػ�ʱ���ò�ͬ����Դ
extern void  power_off();
void analyize_poweron_state()
{

    struct sys_time ws_time, rtc_time;

//�Ƿ��˶�ʱ����
    if (!db_select("delay_pv")) {

        get_sys_time(&rtc_time);

        printf("\n rtc_sys_time:%d:%d:%d:%d:%d:%d\n", \
               rtc_time.year, \
               rtc_time.month, \
               rtc_time.day, \
               rtc_time.hour, \
               rtc_time.min, \
               rtc_time.sec);


        db_select_buffer(GAP_TIMER_INFO, &ws_time, sizeof(struct sys_time));

        printf("\n getget_gap_time.min:%d,ws_time.sec:%d\n", ws_time.min, ws_time.sec);
        int gtalarm_time = ws_time.hour * 60 * 60 + ws_time.min * 60 + ws_time.sec - 1;


        printf("\n <<<<<<<<<ready to-> pir wakeup>>>>>\n");
        set_poweroff();


    } else {


        printf("\n is not not pir wakeup\n");

    }


}
//  ����pr1  pr2���ѣ��ر�pr3����
void close_pir_power_off()
{

    sys_power_set_port_wakeup("wkup_gsen", 1);

    sys_power_set_port_wakeup("wkup_usb", 1);
    sys_power_set_port_wakeup("wkup_pir", 0);

    sys_power_poweroff(0);
}

void  lowbat_power_off()
{


    printf("\n  lowbat_power_off \n");
    sys_power_set_port_wakeup("wkup_gsen", 1);

    sys_power_set_port_wakeup("wkup_usb", 1);
    sys_power_set_port_wakeup("wkup_pir", 0);

    sys_power_poweroff(0);
}


/*
 * �ػ������������, �رյ�Դ , �Բ�ͬ״̬��������

 */
void  power_off()
{

    struct sys_time s_time, ws_time;

    sys_power_set_port_wakeup("wkup_gsen", 1);

    sys_power_set_port_wakeup("wkup_usb", 1);

    printf("\n db_select(delay_pv):::::::::::::::::::::::::%d\n ", db_select("delay_pv"));
    switch (db_select("delay_pv")) {




    case 1: //  ��ʱ����

        //if(db_select("alarm")){

        //��ȡ����ʱ����
        struct sys_time alarm_time;

        db_select_buffer(WORK_TIMER_INFO, &alarm_time, sizeof(struct sys_time));

        printf("\n alarm_work.hour:%d,alarm_time.min:%d, alarm_time.sec:%d,%d\n", alarm_time.hour, alarm_time.min, alarm_time.sec, sizeof(struct sys_time));
        int talarm_time = alarm_time.hour * 60 * 60 + alarm_time.min * 60 + alarm_time.sec;

        extern  int alarm_wakeup_set_config(u32 sec, int enable);

        if (get_workmode_bit() == 2) {

            printf("\n close alarm\n");
            alarm_wakeup_set_config(talarm_time, 0);
        } else {
            printf("\n talarm_time:::::::%d\n ", talarm_time);
            alarm_wakeup_set_config(talarm_time, 1);
        }
        printf("\n close pir \n");
        sys_power_set_port_wakeup("wkup_pir", 0); //�ر�PIR ��Ӧ��PR2�ڻ��ѣ� ������ʱ���գ�����ʱ�������գ���Ҫ��Ƶ�ϳ�


        // }

        break ;


    case 0: //  PIR ���ʱ��


        db_select_buffer(GAP_TIMER_INFO, &ws_time, sizeof(struct sys_time));

        printf("\n every_timer_layout_onkey_set_ws_time.hour:%d,ws_time.min:%d, ws_time.sec:%d,%d\n", ws_time.hour, ws_time.min, ws_time.sec, sizeof(struct sys_time));


        // int gtalarm_time=ws_time.hour*60*60+ws_time.min*60+ws_time.sec-1;
        int gtalarm_time = ws_time.min * 60 + ws_time.sec - 1;

        extern  int alarm_wakeup_set_config(u32 sec, int enable);
        if (get_workmode_bit() == 2) {
            printf("\n close alarm\n");
            alarm_wakeup_set_config(talarm_time, 0);
        } else {
            printf("\n current set alarm_time:::::::%d s\n ", gtalarm_time);
            alarm_wakeup_set_config(gtalarm_time, 1);
        }
        printf("\n close pir \n");
        sys_power_set_port_wakeup("wkup_pir", 0);






        break ;


    case 3:

        break ;

    }


// ����pr0
    sys_power_poweroff(0);



}

void working_show_ledstate()
{


    extern int set_led_state(u8 state);
    set_led_state(1);

    os_time_dly(20);

    set_led_state(0);


}

void special_power_off_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;
    u32 park_en;

    // printf("\n power_off_play_end>>>>>>>>>>>>>>>>>>>>>>>>>>>\n ");
    extern  void ui_lcd_light_off(void);
    ui_lcd_light_off();
    if (ui) {
        server_close(ui);
    }
    //if(!switch_off_state()){//�Ƿ�����
    if (switch_work_on_state()) { //�Ƿ�����

        working_show_ledstate();
    }
    power_off();//  ֱ�ӵ��ùػ�

    return ;
}



void check_pirgap_timer()
{


    if (!db_select("delay_pv")) {

        return ;

    }

    struct sys_time s_time;


    db_select_buffer(GAP_TIMER_INFO, &s_time, sizeof(struct sys_time));

    printf("\n get_gap_time.min:%d,s_time.sec:%d,%d\n", s_time.min, s_time.sec, sizeof(struct sys_time));

}
//  判断小于2023年的 默认rtc 时间

void check_rtc_time()
{
    struct sys_time time;
    get_sys_time(&time);
    printf("\n get_rtc_sys_time---------- %d:%d:%d:%d:%d:%d -----------\n", \
           time.year, time.month, time.day, time.hour, time.min, time.sec);
    if (time.year < 2023) { //  判断小于2023年的 默认rtc 时间
        extern int def_rtc_sys_date_set();
        def_rtc_sys_date_set();
    }

}

int Deal_work_timer()
{


    struct sys_time s_time, e_time, time, gap_time;


    //  判断小于2023年的 默认rtc 时间
    check_rtc_time();

    db_select_buffer(ALLOW_START_TIMER_INFO, &s_time, sizeof(struct sys_time));

    printf("\n Allow__Start_timer---------%d:%d:%d------------\n", s_time.hour, s_time.min, s_time.sec);

    db_select_buffer(ALLOW_END_TIMER_INFO, &e_time, sizeof(struct sys_time));

    printf("\n Allow__End_timer-----------%d:%d:%d-------------\n", e_time.hour, e_time.min, e_time.sec);


    if (s_time.hour > 23 || s_time.min > 59 || s_time.sec > 59 || e_time.hour > 23 || e_time.min > 59 || e_time.sec > 59) {

        s_time.hour = 0;
        s_time.min = 0;
        s_time.sec = 0;

        db_update_buffer(ALLOW_START_TIMER_INFO, &s_time, sizeof(struct sys_time));

        e_time.hour = 23;
        e_time.min = 59;
        e_time.sec = 59;
        db_update_buffer(ALLOW_END_TIMER_INFO, &e_time, sizeof(struct sys_time));


        db_select_buffer(ALLOW_START_TIMER_INFO, &s_time, sizeof(struct sys_time));
        printf("\n correct__Start_timer---------%d:%d:%d------------\n", s_time.hour, s_time.min, s_time.sec);

        db_select_buffer(ALLOW_END_TIMER_INFO, &e_time, sizeof(struct sys_time));

        printf("\n correct__End_timer-----------%d:%d:%d-------------\n", e_time.hour, e_time.min, e_time.sec);



    }
    int s_totaltimer = s_time.hour * 60 * 60 + s_time.min * 60 + s_time.sec;
    int e_totaltimer = e_time.hour * 60 * 60 + e_time.min * 60 + e_time.sec;
    int rtc_totaltimer = time.hour * 60 * 60 + time.min * 60 + time.sec;


// pir ʱ��Ƿ���� Ĭ�� 15s
    db_select_buffer(GAP_TIMER_INFO, &gap_time, sizeof(struct sys_time));

    printf("\n gap__worktime_set_reset_.min:%d, gap_time.sec:%d,%d\n", gap_time.min,  gap_time.sec,  sizeof(struct sys_time));


    if (gap_time.min > 59 || gap_time.sec > 59) { // �Ƿ�ʱ�� ����Ĭ��15s Pir���
        gap_time.min = 0;
        gap_time.sec = 15;
        db_update_buffer(GAP_TIMER_INFO, &gap_time, sizeof(struct sys_time));

    }


    if ((rtc_totaltimer >= s_totaltimer) && (rtc_totaltimer <= e_totaltimer)) {

        printf("\n work timer is ok\n");
        return 1;

    } else {

        printf("\n work timer is not ok\n");
        return  0;
    }

}








