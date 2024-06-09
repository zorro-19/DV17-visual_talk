

#include "asm/clock.h"
#include "os/os_compat.h"
#include "generic/gpio.h"
#include "step_motor.h"


/************************* step moto begin *******************************/
#if 1

struct timer_moto_port *moto_port = NULL;
int dire;
u32 got_step = 0;
u32 now_step = 0;
u32 cs = 0;

void moto_timer_test(void)
{
    static int half_step = 0;

    if (++half_step > 7) {
        half_step = 0;
    }

    if (cs) {
        gpio_set_pull_up(moto_port->PORT_CHECK_A, 1);
        gpio_set_pull_down(moto_port->PORT_CHECK_A, 0);
        u32 status = gpio_read(moto_port->PORT_CHECK_A);

        gpio_set_pull_up(moto_port->PORT_CHECK_A, 0);
        gpio_set_pull_down(moto_port->PORT_CHECK_A, 1);
        u32 status1 = gpio_read(moto_port->PORT_CHECK_A);
        /* printf("status:%d  status1:%d\n",status,status1); */
        if (status ^ status1) {
            /* printf("running running \n"); */

        } else {
            if (status == 0 && status1 == 0 && dire == STEP_MOTO_ROTA_CLOCKWISE) {
                printf("step motor stop stop \n\n\n\n");
                T3_CON = 0;//BIT(14);
                gpio_direction_output(moto_port->PORT_A, 0);
                gpio_direction_output(moto_port->PORT_B, 0);
                gpio_direction_output(moto_port->PORT_C, 0);
                gpio_direction_output(moto_port->PORT_D, 0);
                now_step = 0;
                got_step = 0;
                dire = 0;
                cs = 0;
            }
            if (status == 1 && status1 == 1 && dire == STEP_MOTO_ROTA_ANTICLOCK) {
                printf("step motor stop stop \n\n\n\n");
                T3_CON = 0;//BIT(14);
                gpio_direction_output(moto_port->PORT_A, 0);
                gpio_direction_output(moto_port->PORT_B, 0);
                gpio_direction_output(moto_port->PORT_C, 0);
                gpio_direction_output(moto_port->PORT_D, 0);
                now_step = 0;
                got_step = 0;
                dire = 0;
                cs = 0;



            }


        }


    }

    if (moto_port->record.pwm_mode == 1) {
        if (dire == STEP_MOTO_ROTA_CLOCKWISE) {
            switch (half_step) {
            case 0:
                gpio_direction_output(moto_port->PORT_D, 0);
                break;
            case 1:
                gpio_direction_output(moto_port->PORT_B, 1);
                break;
            case 2:
                gpio_direction_output(moto_port->PORT_A, 0);
                break;
            case 3:
                gpio_direction_output(moto_port->PORT_C, 1);
                break;
            case 4:
                gpio_direction_output(moto_port->PORT_B, 0);
                break;
            case 5:
                gpio_direction_output(moto_port->PORT_D, 1);
                break;
            case 6:
                gpio_direction_output(moto_port->PORT_C, 0);
                break;
            case 7:
                gpio_direction_output(moto_port->PORT_A, 1);
                break;
            default:
                break;
            }
        } else if (dire == STEP_MOTO_ROTA_ANTICLOCK) {
            switch (half_step) {
            case 0:
                gpio_direction_output(moto_port->PORT_D, 1);
                break;
            case 1:
                gpio_direction_output(moto_port->PORT_A, 0);
                break;
            case 2:
                gpio_direction_output(moto_port->PORT_C, 1);
                break;
            case 3:
                gpio_direction_output(moto_port->PORT_D, 0);
                break;
            case 4:
                gpio_direction_output(moto_port->PORT_B, 1);
                break;
            case 5:
                gpio_direction_output(moto_port->PORT_C, 0);
                break;
            case 6:
                gpio_direction_output(moto_port->PORT_A, 1);
                break;
            case 7:
                gpio_direction_output(moto_port->PORT_B, 0);
                break;
            default:
                break;
            }
        } else {
            printf("step moto direction %d set err!", dire);
        }
    } else if (moto_port->record.pwm_mode == 2) {


        if (dire == STEP_MOTO_ROTA_CLOCKWISE) {
            switch (half_step) {
            case 0:
                gpio_direction_output(moto_port->PORT_A, 1);
                gpio_direction_output(moto_port->PORT_B, 0);
                gpio_direction_output(moto_port->PORT_C, 0);
                gpio_direction_output(moto_port->PORT_D, 0);
                break;
            case 1:
                gpio_direction_output(moto_port->PORT_A, 1);
                gpio_direction_output(moto_port->PORT_B, 0);
                gpio_direction_output(moto_port->PORT_C, 1);
                gpio_direction_output(moto_port->PORT_D, 0);

                break;
            case 2:
                gpio_direction_output(moto_port->PORT_A, 0);
                gpio_direction_output(moto_port->PORT_B, 0);
                gpio_direction_output(moto_port->PORT_C, 1);
                gpio_direction_output(moto_port->PORT_D, 0);


                break;
            case 3:
                gpio_direction_output(moto_port->PORT_A, 0);
                gpio_direction_output(moto_port->PORT_B, 1);
                gpio_direction_output(moto_port->PORT_C, 1);
                gpio_direction_output(moto_port->PORT_D, 0);



                break;
            case 4:
                gpio_direction_output(moto_port->PORT_A, 0);
                gpio_direction_output(moto_port->PORT_B, 1);
                gpio_direction_output(moto_port->PORT_C, 0);
                gpio_direction_output(moto_port->PORT_D, 0);



                break;
            case 5:
                gpio_direction_output(moto_port->PORT_A, 0);
                gpio_direction_output(moto_port->PORT_B, 1);
                gpio_direction_output(moto_port->PORT_C, 0);
                gpio_direction_output(moto_port->PORT_D, 1);



                break;
            case 6:

                gpio_direction_output(moto_port->PORT_A, 0);
                gpio_direction_output(moto_port->PORT_B, 0);
                gpio_direction_output(moto_port->PORT_C, 0);
                gpio_direction_output(moto_port->PORT_D, 1);


                break;
            case 7:
                gpio_direction_output(moto_port->PORT_A, 1);
                gpio_direction_output(moto_port->PORT_B, 0);
                gpio_direction_output(moto_port->PORT_C, 0);
                gpio_direction_output(moto_port->PORT_D, 1);



                break;
            default:
                break;
            }
        } else if (dire == STEP_MOTO_ROTA_ANTICLOCK) {
            switch (half_step) {
            case 0:
                gpio_direction_output(moto_port->PORT_A, 1);
                gpio_direction_output(moto_port->PORT_B, 0);
                gpio_direction_output(moto_port->PORT_C, 0);
                gpio_direction_output(moto_port->PORT_D, 1);
                break;
            case 1:

                gpio_direction_output(moto_port->PORT_A, 0);
                gpio_direction_output(moto_port->PORT_B, 0);
                gpio_direction_output(moto_port->PORT_C, 0);
                gpio_direction_output(moto_port->PORT_D, 1);
                break;
            case 2:
                gpio_direction_output(moto_port->PORT_A, 0);
                gpio_direction_output(moto_port->PORT_B, 1);
                gpio_direction_output(moto_port->PORT_C, 0);
                gpio_direction_output(moto_port->PORT_D, 1);
                break;
            case 3:
                gpio_direction_output(moto_port->PORT_A, 0);
                gpio_direction_output(moto_port->PORT_B, 1);
                gpio_direction_output(moto_port->PORT_C, 0);
                gpio_direction_output(moto_port->PORT_D, 0);

                break;
            case 4:
                gpio_direction_output(moto_port->PORT_A, 0);
                gpio_direction_output(moto_port->PORT_B, 1);
                gpio_direction_output(moto_port->PORT_C, 1);
                gpio_direction_output(moto_port->PORT_D, 0);


                break;
            case 5:
                gpio_direction_output(moto_port->PORT_A, 0);
                gpio_direction_output(moto_port->PORT_B, 0);
                gpio_direction_output(moto_port->PORT_C, 1);
                gpio_direction_output(moto_port->PORT_D, 0);

                break;
            case 6:
                gpio_direction_output(moto_port->PORT_A, 1);
                gpio_direction_output(moto_port->PORT_B, 0);
                gpio_direction_output(moto_port->PORT_C, 1);
                gpio_direction_output(moto_port->PORT_D, 0);

                break;

            case 7:
                gpio_direction_output(moto_port->PORT_A, 1);
                gpio_direction_output(moto_port->PORT_B, 0);
                gpio_direction_output(moto_port->PORT_C, 0);
                gpio_direction_output(moto_port->PORT_D, 0);
                break;



            default:
                break;
            }
        } else {
            printf("step moto direction %d set err!", dire);
        }
    }

    if (++now_step > got_step) {	// 到达指定位置
        T3_CON = 0;//BIT(14);
        gpio_direction_output(moto_port->PORT_A, 0);
        gpio_direction_output(moto_port->PORT_B, 0);
        gpio_direction_output(moto_port->PORT_C, 0);
        gpio_direction_output(moto_port->PORT_D, 0);
        now_step = 0;
        got_step = 0;
        dire = 0;
        cs = 0;
    }
    /* } */
}


___interrupt
static void timer3_isr()
{
    irq_handler_enter(TIME3_INT);
    T3_CON |= BIT(14);
    moto_timer_test();
    irq_handler_exit(TIME3_INT);
}

static int timer3_init(u32 freq)
{
    T3_CNT = 0;
    request_irq(TIME3_INT, 3, timer3_isr, 0);
    T3_PRD = clk_get("timer") / freq;
    T3_CNT = 0;
    T3_CON = BIT(0);
    T3_CON |= BIT(14);
    moto_port->record.pwm_cycle = T3_PRD;
    return 0;
}

int step_moto_timer_ctrl(int set_dire, struct timer_moto_port *port, u32 speed, int angle, u8 mode)
{
    dire = set_dire;
    moto_port = port;

    got_step = angle * 8 * 512 / 360;

    if (speed < TIMER_MIN_SPEED) {
        printf("set tiemr speed to low: %d\n", speed);
        return -1;
    } else if (speed > TIMER_MAX_SPEED) {
        printf("set tiemr speed to high: %d\n", speed);
        return -1;
    } else {
        timer3_init(speed);
    }

    if (port->PORT_CHECK_A != -1) {
        gpio_direction_input(port->PORT_CHECK_A);
        cs = 1;
    }




    /* moto_port->record.position  *= 1000;	// 放大1000倍后记录,避免浮点数处理 */
    moto_port->record.moto_timer = 3;
    moto_port->record.pwm_duty   = 37;
    moto_port->record.pwm_device = NULL;
    moto_port->record.pwm_mode   = mode;

    return 0;
}

#endif

