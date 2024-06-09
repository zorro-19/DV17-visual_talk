#include "gpio.h"
#include "app_config.h"
#include "device/device.h"
#include "system/init.h"

typedef __uint8_t uint8 ;
extern void delay_2ms(int cnt);
#if  1
#define PORT_0  IO_PORTG_12//IO_PORTH_02  //蓝色线
#define PORT_1  IO_PORTG_13//IO_PORTH_03  //棕色线
#define PORT_2  IO_PORTG_14//IO_PORTH_04  //黄色线
#define PORT_3  IO_PORTG_15//IO_PORTH_05  //黑色线
#else

#define PORT_0  IO_PORTH_00  //蓝色线
#define PORT_1  IO_PORTH_01 //棕色线
#define PORT_2  IO_PORTH_04  //黄色线
#define PORT_3  IO_PORTH_05  //黑色线


#endif




//#define STEP_MOTOR_A_PIN          gpio_direction_output(PORT_0,STEP_MOTOR_A_PIN);    /*蓝色线*/
//#define STEP_MOTOR_B_PIN          gpio_direction_output(PORT_1,STEP_MOTOR_B_PIN);   /*棕色线*/
//#define STEP_MOTOR_C_PIN          gpio_direction_output(PORT_2,STEP_MOTOR_C_PIN);   /*黄色线*/
//#define STEP_MOTOR_D_PIN          gpio_direction_output(PORT_3,STEP_MOTOR_D_PIN);   /*黑色线*/

#define STEP_MOTOR_SPEED          2

#define STEP_MOTOR_DIRE_FOREWARD  1     /*步进电机正向转：顺时针*/
#define STEP_MOTOR_DIRE_ROLLBACK  2     /*步进电机正向转：逆时针*/

static uint8 MotorStepCount = 0;

/*函数声明*/
static void step_motor_a_output(void);
static void step_motor_ab_output(void);
static void step_motor_b_output(void);
static void step_motor_bc_output(void);
static void step_motor_c_output(void);
static void step_motor_cd_output(void);
static void step_motor_d_output(void);
static void step_motor_da_output(void);
static void step_motor_foreward_output(uint8 step);
static void step_motor_rollback_output(uint8 step);
static void step_motor_foreward(uint8 speed);
static void step_motor_rollback(uint8 speed);


//========================================================================
// 简述: 步进电机停止
// 参数: 无
// 返回: 无
// 详述: （ULN2003有反向功能）
//========================================================================
void step_motor_stop(void)
{

 #if 0
    gpio_direction_output(PORT_0,1);
	gpio_direction_output(PORT_1,1);
	gpio_direction_output(PORT_2,1);
	gpio_direction_output(PORT_3,1);
#else

    gpio_direction_output(PORT_0,0);
	gpio_direction_output(PORT_1,0);
	gpio_direction_output(PORT_2,0);
	gpio_direction_output(PORT_3,0);



#endif
}

//========================================================================
// 简述: 步进电机正转
// 参数: 无
// 返回: 无
// 详述: 顺时针转
//========================================================================
static void step_motor_foreward(uint8 speed)
{



	 if((++MotorStepCount) > 7) MotorStepCount = 0;
	  printf("\n ===page====%d, %d,%d,  %s\n",MotorStepCount,speed,__LINE__,__FUNCTION__);
   step_motor_foreward_output(MotorStepCount);
	 delay_2ms(speed);
}

//========================================================================
// 简述: 步进电机反转
// 参数: 无
// 返回: 无
// 详述: 逆时针转
//========================================================================
static void step_motor_rollback(uint8 speed)
{
	 if((++MotorStepCount) > 7) MotorStepCount = 0;

    printf("\n ===page====%d, %d,%d,  %s\n",MotorStepCount,speed,__LINE__,__FUNCTION__);
   step_motor_rollback_output(MotorStepCount);
	 delay_2ms(speed);
}

//========================================================================
// 简述: 步进电机正向输出
// 参数: 无
// 返回: 无
// 详述:
//========================================================================
static void step_motor_foreward_output(uint8 step)
{
   switch(step)
	 {
		 case 0:
		 {
			  step_motor_a_output();
		    break;
		 }
		 case 1:
		 {
			  step_motor_ab_output();
		    break;
		 }
		 case 2:
		 {
			  step_motor_b_output();
		    break;
		 }
		 case 3:
		 {
			  step_motor_bc_output();
		    break;
		 }
		 case 4:
		 {
			  step_motor_c_output();
		    break;
		 }
		 case 5:
		 {
			  step_motor_cd_output();
		    break;
		 }
		 case 6:
		 {
			  step_motor_d_output();
		    break;
		 }
		 case 7:
		 {
			  step_motor_da_output();
		    break;
		 }
		 default:
			  break;
	 }
}

//========================================================================
// 简述: 步进电机反向输出
// 参数: 无
// 返回: 无
// 详述:
//========================================================================
static void step_motor_rollback_output(uint8 step)
{
   switch(step)
	 {
		 case 0:
		 {
			 step_motor_da_output();// step_motor_a_output();
		    break;
		 }
		 case 1:
		 {
			  step_motor_d_output();// step_motor_da_output();
		    break;
		 }
		 case 2:
		 {
			  step_motor_cd_output();;// step_motor_d_output();
		    break;
		 }
		 case 3:
		 {
			  step_motor_c_output();//step_motor_cd_output();
		    break;
		 }
		 case 4:
		 {
			  step_motor_bc_output();//step_motor_c_output();
		    break;
		 }
		 case 5:
		 {
			  step_motor_b_output();//step_motor_bc_output();
		    break;
		 }
		 case 6:
		 {
			  step_motor_ab_output();//step_motor_b_output();
		    break;
		 }
		 case 7:
		 {
			  step_motor_a_output();//step_motor_ab_output();
		    break;
		 }
		 default:
			  break;
	 }
}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机A相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_a_output(void)
{

	gpio_direction_output(PORT_0,1);
	gpio_direction_output(PORT_1,1);
	gpio_direction_output(PORT_2,1);
	gpio_direction_output(PORT_3,0);

}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机AB相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_ab_output(void)
{
    gpio_direction_output(PORT_0,1);
	gpio_direction_output(PORT_1,1);
	gpio_direction_output(PORT_2,0);
	gpio_direction_output(PORT_3,0);
}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机B相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_b_output(void)
{
    gpio_direction_output(PORT_0,1);
	gpio_direction_output(PORT_1,1);
	gpio_direction_output(PORT_2,0);
	gpio_direction_output(PORT_3,1);
}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机BC相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_bc_output(void)
{
    gpio_direction_output(PORT_0,1);
	gpio_direction_output(PORT_1,0);
	gpio_direction_output(PORT_2,0);
	gpio_direction_output(PORT_3,1);
}
//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机C相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_c_output(void)
{
    gpio_direction_output(PORT_0,1);
	gpio_direction_output(PORT_1,0);
	gpio_direction_output(PORT_2,1);
	gpio_direction_output(PORT_3,1);
}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机CD相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_cd_output(void)
{
    gpio_direction_output(PORT_0,0);
	gpio_direction_output(PORT_1,0);
	gpio_direction_output(PORT_2,1);
	gpio_direction_output(PORT_3,1);
}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机D相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_d_output(void)
{
    gpio_direction_output(PORT_0,0);
	gpio_direction_output(PORT_1,1);
	gpio_direction_output(PORT_2,1);
	gpio_direction_output(PORT_3,1);
}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机DA相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_da_output(void)
{
    gpio_direction_output(PORT_0,0);
	gpio_direction_output(PORT_1,1);
	gpio_direction_output(PORT_2,1);
	gpio_direction_output(PORT_3,0);
}


//========================================================================
// 简述: 步进电机驱动
// 参数: dire：步进电机方向   speed：步进电机转速 ,angle  旋转度数 ,point_step  1为单步  0为持续步进
// 返回: 无
// 详述:
//========================================================================
void step_motor_drive(uint8 dire,uint8 speed,int angle,u8 point_step)
{

    int beats = angle * 4096 / 360;

    int dir = 0;
    if(beats < 0){
        dir = 1;
        beats = -beats;
    }

   if(point_step){

   beats=1;
   }

   while(beats != 0){
   beats--;



   switch(dire)
    {
		 case STEP_MOTOR_DIRE_FOREWARD:
		 {
			  step_motor_foreward(speed);
		    break;
		 }
		 case STEP_MOTOR_DIRE_ROLLBACK:
		 {
			  step_motor_rollback(speed);
		    break;
		 }
		 default:
			  break;
    }


   }




}


void montor_thread(void *priv)
{
    msleep(3000);
#if 0
    startmortor(360 * 7);
    startmortor(-360 * 7);
#else

   step_motor_drive(1,2,180,0);

   step_motor_stop();
   step_motor_drive(2,2,-180,0);

   step_motor_stop();
#endif



}

int montor_init(void)
{
    thread_fork("montor_thread", 15, 2048, 64, NULL, montor_thread, NULL);
}
//late_initcall(montor_init);

