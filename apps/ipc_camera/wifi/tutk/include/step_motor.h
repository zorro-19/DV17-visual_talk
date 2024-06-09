#ifndef __STEP_MOTOR_H
#define __STEP_MOTOR_H

#include "asm/clock.h"
#include "os/os_compat.h"
#include "generic/gpio.h"


/************************ step moto*********************************/

struct moto_cfg {
    void	*pwm_device;
    u32		moto_timer;
    long	position;
    u32		pwm_cycle;
    int		pwm_duty;
    u8      pwm_mode;
};

struct moto_pwm_dtime {
    int PORT_A;
    int PORT_B;
    int PORT_C;
    int PORT_D;
};

#define STEP_MOTO_LOW_SPEED		0	// 低速
#define STEP_MOTO_MED_SPEED		1	// 中速
#define STEP_MOTO_HIG_SPEED		2	// 快速

#define STEP_MOTO_ROTA_CLOCKWISE	0	// 顺时针旋转 1
#define STEP_MOTO_ROTA_ANTICLOCK	1	// 逆时针旋转

struct run_cfg {
    bool	direction;
    int		speed;
    int		angle;
};

struct moto_platfrom_data {
    int PORT_A;
    int PORT_B;
    int PORT_C;
    int PORT_D;
    struct moto_cfg			cfg;
    struct run_cfg			run;
    // struct moto_pwm_dtime 	dlt;
};


#define PWM_STEP_MOTO_INIT			_IOW('M',0,u32)	// 初始化
#define PWM_STEP_MOTO_RUN			_IOW('M',1,u32)	// 运行
#define PWM_STEP_MOTO_STOP			_IOW('M',2,u32) // 停止
#define PWM_STEP_MOTO_GET_STATE		_IOW('M',3,u32) // 获取状态
#define PWM_STEP_MOTO_BACK_HOME		_IOW('M',4,u32)	// 回到初始位置


#define STEP_MOTO_PLATFORM_DATA_BEGIN(data) \
		static struct moto_platfrom_data data={

#define STEP_MOTO_PLATFORM_DATA_END() \
			.cfg = { \
				.position = 0, \
				.cycle    = 0, \
				.duty     = 0, \
			\}, \
	};


// 定时器控制步进电机

#define TIMER_MIN_SPEED 250
#define TIMER_MAX_SPEED 1700

struct timer_moto_port {
    int PORT_A;
    int PORT_B;
    int PORT_C;
    int PORT_D;
    int PORT_CHECK_A;
    int PORT_CHECK_B;

    struct moto_cfg record;
};

int step_moto_timer_ctrl(int set_dire, struct timer_moto_port *port, u32 speed, int angle, u8 mode);

#define TIMER_MOTO_PORT_BEGIN(data) \
	 struct timer_moto_port data = {

#define TIMER_MOTO_PORT_END() \
		.record = { \
			.pwm_device = NULL, \
			.moto_timer = 0, \
			.pwm_cycle = 0, \
			.pwm_duty = 0, \
			.position = 0, \
		}, \
	};


/********************** step moto end ****************************/
#endif
