#include "gpio.h"
#include "app_config.h"
#include "device/device.h"
#include "system/init.h"

typedef __uint8_t uint8 ;
extern void delay_2ms(int cnt);

#define PORT_0  IO_PORTG_12//IO_PORTH_02  //蓝色线
#define PORT_1  IO_PORTG_13//IO_PORTH_03  //棕色线
#define PORT_2  IO_PORTG_14//IO_PORTH_04  //黄色线
#define PORT_3  IO_PORTG_15//IO_PORTH_05  //黑色线



#define PORT_4  IO_PORTH_01  //蓝色线
#define PORT_5  IO_PORTH_05//棕色线
#define PORT_6  IO_PORTH_00   //黄色线
#define PORT_7  IO_PORTH_04  //黑色线



#define STEP_MOTOR_SPEED          2

#define STEP_MOTOR_DIRE_FOREWARD  1      /*步进电机正向转：逆时针*/
#define STEP_MOTOR_DIRE_ROLLBACK  2     /*步进电机正向转：顺时针*/
#define STEP_MOTOR_DIRE_UP_DIREC  3     /*步进电机向上正向转：顺时针*/
#define STEP_MOTOR_DIRE_DOWN_DIREC  4     /*步进电机向下正向转：逆时针*/


#define  MONTOR_TIME    200// 600  //时间
int montor_timer_hander=0;
static uint8 MotorStepCount = 0;



/*函数声明*/
//左右方向
static void step_motor_lf_a_output(void);
static void step_motor_lf_ab_output(void);
static void step_motor_lf_b_output(void);
static void step_motor_lf_bc_output(void);
static void step_motor_lf_c_output(void);
static void step_motor_lf_cd_output(void);
static void step_motor_lf_d_output(void);
static void step_motor_lf_da_output(void);
static void step_motor_lf_foreward_output(uint8 step);
static void step_motor_lf_rollback_output(uint8 step);
static void step_motor_foreward(uint8 speed);
static void step_motor_rollback(uint8 speed);



//上下方向
static void step_motor_ud_a_output(void);
static void step_motor_ud_ab_output(void);
static void step_motor_ud_b_output(void);
static void step_motor_ud_bc_output(void);
static void step_motor_ud_c_output(void);
static void step_motor_ud_cd_output(void);
static void step_motor_ud_d_output(void);
static void step_motor_ud_da_output(void);
static void step_motor_up_direction_output(uint8 step);
static void step_motor_down_direction_output(uint8 step);
static void step_motor_up_direction(uint8 speed);
static void step_motor_down_direction(uint8 speed);


enum {
    DIRECTION_STOP = 10,
    DIRECTION_UP = 11,
    DIRECTION_DOWN = 12,
    DIRECTION_LEFT = 13,
    DIRECTION_RIGHT = 14,


};
//===================== 左右方向电机控制================================

//========================================================================
// 简述: 步进电机停止
// 参数: 无
// 返回: 无
// 详述: （ULN2003有反向功能）
//========================================================================
void step_motor_lf_stop(void)
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
	//  printf("\n ===page====%d, %d,%d,  %s\n",MotorStepCount,speed,__LINE__,__FUNCTION__);
   step_motor_lf_foreward_output(MotorStepCount);
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

   // printf("\n ===page====%d, %d,%d,  %s\n",MotorStepCount,speed,__LINE__,__FUNCTION__);
   step_motor_lf_rollback_output(MotorStepCount);
	 delay_2ms(speed);
}

//========================================================================
// 简述: 步进电机正向输出
// 参数: 无
// 返回: 无
// 详述:
//========================================================================
static void step_motor_lf_foreward_output(uint8 step)
{
   switch(step)
	 {
		 case 0:
		 {
			  step_motor_lf_a_output();
		    break;
		 }
		 case 1:
		 {
			  step_motor_lf_ab_output();
		    break;
		 }
		 case 2:
		 {
			  step_motor_lf_b_output();
		    break;
		 }
		 case 3:
		 {
			  step_motor_lf_bc_output();
		    break;
		 }
		 case 4:
		 {
			  step_motor_lf_c_output();
		    break;
		 }
		 case 5:
		 {
			  step_motor_lf_cd_output();
		    break;
		 }
		 case 6:
		 {
			  step_motor_lf_d_output();
		    break;
		 }
		 case 7:
		 {
			  step_motor_lf_da_output();
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
static void step_motor_lf_rollback_output(uint8 step)
{
   switch(step)
	 {
		 case 0:
		 {
			 step_motor_lf_da_output();// step_motor_a_output();
		    break;
		 }
		 case 1:
		 {
			  step_motor_lf_d_output();// step_motor_da_output();
		    break;
		 }
		 case 2:
		 {
			  step_motor_lf_cd_output();;// step_motor_d_output();
		    break;
		 }
		 case 3:
		 {
			  step_motor_lf_c_output();//step_motor_cd_output();
		    break;
		 }
		 case 4:
		 {
			  step_motor_lf_bc_output();//step_motor_c_output();
		    break;
		 }
		 case 5:
		 {
			  step_motor_lf_b_output();//step_motor_bc_output();
		    break;
		 }
		 case 6:
		 {
			  step_motor_lf_ab_output();//step_motor_b_output();
		    break;
		 }
		 case 7:
		 {
			  step_motor_lf_a_output();//step_motor_ab_output();
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
static void step_motor_lf_a_output(void)
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
static void step_motor_lf_ab_output(void)
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
static void step_motor_lf_b_output(void)
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
static void step_motor_lf_bc_output(void)
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
static void step_motor_lf_c_output(void)
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
static void step_motor_lf_cd_output(void)
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
static void step_motor_lf_d_output(void)
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
static void step_motor_lf_da_output(void)
{
    gpio_direction_output(PORT_0,0);
	gpio_direction_output(PORT_1,1);
	gpio_direction_output(PORT_2,1);
	gpio_direction_output(PORT_3,0);
}


//===================== 上下方向电机控制================================


//========================================================================
// 简述: 步进上下电机停止
// 参数: 无
// 返回: 无
// 详述: （ULN2003有反向功能）
//========================================================================
void step_motor_ud_stop(void)
{

#if 0
    gpio_direction_output(PORT_4,1);
	gpio_direction_output(PORT_5,1);
	gpio_direction_output(PORT_6,1);
	gpio_direction_output(PORT_7,1);
#else

    gpio_direction_output(PORT_4,0);
	gpio_direction_output(PORT_5,0);
	gpio_direction_output(PORT_6,0);
	gpio_direction_output(PORT_7,0);



#endif
}


//========================================================================
// 简述: 步进电机正转
// 参数: 无
// 返回: 无
// 详述: 顺时针转
//========================================================================
static void step_motor_up_direction(uint8 speed)
{



	 if((++MotorStepCount) > 7) MotorStepCount = 0;
	//  printf("\n ===page====%d, %d,%d,  %s\n",MotorStepCount,speed,__LINE__,__FUNCTION__);
    step_motor_up_direction_output(MotorStepCount);
	 delay_2ms(speed);
}

//========================================================================
// 简述: 步进电机反转
// 参数: 无
// 返回: 无
// 详述: 逆时针转
//========================================================================
static void step_motor_down_direction(uint8 speed)
{
	 if((++MotorStepCount) > 7) MotorStepCount = 0;

   // printf("\n ===page====%d, %d,%d,  %s\n",MotorStepCount,speed,__LINE__,__FUNCTION__);
   step_motor_down_direction_output(MotorStepCount);
	 delay_2ms(speed);
}

//========================================================================
// 简述: 步进电机正向输出
// 参数: 无
// 返回: 无
// 详述:
//========================================================================
static void step_motor_up_direction_output(uint8 step)
{
   switch(step)
	 {
		 case 0:
		 {
			  step_motor_ud_a_output();
		    break;
		 }
		 case 1:
		 {
			  step_motor_ud_ab_output();
		    break;
		 }
		 case 2:
		 {
			  step_motor_ud_b_output();
		    break;
		 }
		 case 3:
		 {
			  step_motor_ud_bc_output();
		    break;
		 }
		 case 4:
		 {
			  step_motor_ud_c_output();
		    break;
		 }
		 case 5:
		 {
			  step_motor_ud_cd_output();
		    break;
		 }
		 case 6:
		 {
			  step_motor_ud_d_output();
		    break;
		 }
		 case 7:
		 {
			  step_motor_ud_da_output();
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
static void step_motor_down_direction_output(uint8 step)
{
   switch(step)
	 {
		 case 0:
		 {
			 step_motor_ud_da_output();// step_motor_a_output();
		    break;
		 }
		 case 1:
		 {
			  step_motor_ud_d_output();// step_motor_da_output();
		    break;
		 }
		 case 2:
		 {
			  step_motor_ud_cd_output();;// step_motor_d_output();
		    break;
		 }
		 case 3:
		 {
			  step_motor_ud_c_output();//step_motor_cd_output();
		    break;
		 }
		 case 4:
		 {
			  step_motor_ud_bc_output();//step_motor_c_output();
		    break;
		 }
		 case 5:
		 {
			  step_motor_ud_b_output();//step_motor_bc_output();
		    break;
		 }
		 case 6:
		 {
			  step_motor_ud_ab_output();//step_motor_b_output();
		    break;
		 }
		 case 7:
		 {
			  step_motor_ud_a_output();//step_motor_ab_output();
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
static void step_motor_ud_a_output(void)
{

	gpio_direction_output(PORT_4,1);
	gpio_direction_output(PORT_5,1);
	gpio_direction_output(PORT_6,1);
	gpio_direction_output(PORT_7,0);

}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机AB相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_ud_ab_output(void)
{
    gpio_direction_output(PORT_4,1);
	gpio_direction_output(PORT_5,1);
	gpio_direction_output(PORT_6,0);
	gpio_direction_output(PORT_7,0);
}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机B相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_ud_b_output(void)
{
    gpio_direction_output(PORT_4,1);
	gpio_direction_output(PORT_5,1);
	gpio_direction_output(PORT_6,0);
	gpio_direction_output(PORT_7,1);
}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机BC相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_ud_bc_output(void)
{
    gpio_direction_output(PORT_4,1);
	gpio_direction_output(PORT_5,0);
	gpio_direction_output(PORT_6,0);
	gpio_direction_output(PORT_7,1);
}
//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机C相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_ud_c_output(void)
{
    gpio_direction_output(PORT_4,1);
	gpio_direction_output(PORT_5,0);
	gpio_direction_output(PORT_6,1);
	gpio_direction_output(PORT_7,1);
}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机CD相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_ud_cd_output(void)
{
    gpio_direction_output(PORT_4,0);
	gpio_direction_output(PORT_5,0);
	gpio_direction_output(PORT_6,1);
	gpio_direction_output(PORT_7,1);
}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机D相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_ud_d_output(void)
{
    gpio_direction_output(PORT_4,0);
	gpio_direction_output(PORT_5,1);
	gpio_direction_output(PORT_6,1);
	gpio_direction_output(PORT_7,1);
}

//========================================================================
// 简述: 步进电机引脚控制
// 参数: 无
// 返回: 无
// 详述: 步进电机DA相输出（ULN2003有反向功能）
//========================================================================
static void step_motor_ud_da_output(void)
{
    gpio_direction_output(PORT_4,0);
	gpio_direction_output(PORT_5,1);
	gpio_direction_output(PORT_6,1);
	gpio_direction_output(PORT_7,0);
}





extern void clr_wdt();



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

    printf("\n ==============page=============dire:%d,%d,%s\n ",dire,__LINE__,__FUNCTION__);


    if(beats < 0){
        dir = 1;
        beats = -beats;
    }
#if  0
    if(point_step){


      return ;
    }
#endif

   while(beats != 0){
   beats--;

    if(point_step){

        beats=0;
      break;
    }



   switch(dire)
    {
		 case STEP_MOTOR_DIRE_FOREWARD:
		 {
			  step_motor_foreward(speed);
			//  delay_2ms(speed);
		    break;
		 }
		 case STEP_MOTOR_DIRE_ROLLBACK:
		 {
			  step_motor_rollback(speed);
			 // delay_2ms(speed);
		    break;
		 }


		  case STEP_MOTOR_DIRE_UP_DIREC:
		 {
			  step_motor_up_direction(speed);
		    break;
		 }
		 case STEP_MOTOR_DIRE_DOWN_DIREC:
		 {
			  step_motor_down_direction(speed);
		    break;
		 }

		 default:
			  break;
    }


   }




}

void montor_detect_ctrl(void *priv){

    printf("\n ==============page=============dir:%s, %d,%s\n ",priv,__LINE__,__FUNCTION__);

    if(!strcmp(priv,"left")){
    step_motor_drive(1,2,10,0);

    }else if(!strcmp(priv,"right")){


     step_motor_drive(2,2,10,0);
    }else if(!strcmp(priv,"up")){


    step_motor_drive(3,2,10,0);
    }else if(!strcmp(priv,"down")){

     step_motor_drive(4,2,10,0);

    }



}

void step_montor_control(u8 dir,int req){

  static stop_flag=0,start_flag=0;
  static u8 left_flag,right_flag,up_flag,down_flag;
  int degree=0;
  printf("\n ==============page=============dir:%d,%d %d,%s\n ",dir,montor_timer_hander,__LINE__,__FUNCTION__);

//   if(dir==DIRECTION_STOP){
//
//    stop_flag=1;
//
//   }else{
//
//   stop_flag=0;
//
//   }

   switch(dir){



    case DIRECTION_STOP:

    if(montor_timer_hander){
    sys_timer_del(montor_timer_hander);
    montor_timer_hander=0;
    }


    step_motor_drive(3,2,10,1);



    step_motor_lf_stop();
    step_motor_ud_stop();



    break ;

    case DIRECTION_UP:

   // step_motor_drive(3,2,10,0);

     if(!montor_timer_hander){

    montor_detect_ctrl("up");
    montor_timer_hander =  sys_timer_add("up", montor_detect_ctrl, MONTOR_TIME);
    }

    break ;

    case DIRECTION_DOWN:


     if(!montor_timer_hander){
    montor_detect_ctrl("down");
    montor_timer_hander =  sys_timer_add("down", montor_detect_ctrl, MONTOR_TIME);
    }

    break ;

    case DIRECTION_LEFT:


    if(!montor_timer_hander){

    montor_detect_ctrl("left");
    montor_timer_hander =  sys_timer_add("left", montor_detect_ctrl, MONTOR_TIME);
    }

    break ;

    case DIRECTION_RIGHT:




   if(!montor_timer_hander){
     montor_detect_ctrl("right");
     montor_timer_hander =  sys_timer_add("right", montor_detect_ctrl, MONTOR_TIME);
   }



    break ;



    default:

      break;





   }







}



// 简述: 上电旋转纠正位置
// 参数: dire：步进电机方向   speed：步进电机转速 ,angle  旋转度数 ,point_step  1为单步  0为持续步进
// 返回: 无
// 详述:
//========================================================================

void  turn_360_degree(uint8 dire,uint8 speed,int angle,u8 point_step ){


       step_motor_drive(dire,speed,angle,point_step);

}

void  stop_turn_degree(){


    step_montor_control(10,0);

}


void montor_thread(void *priv)
{
    msleep(3000);
#if 0
    startmortor(360 * 7);
    startmortor(-360 * 7);
#else

   step_motor_drive(1,2,180,0);

   step_motor_lf_stop();
   step_motor_drive(2,2,-180,0);

   step_motor_lf_stop();
#endif



}








int montor_init(void)
{
    thread_fork("montor_thread", 15, 2048, 64, NULL, montor_thread, NULL);
}
//late_initcall(montor_init);

