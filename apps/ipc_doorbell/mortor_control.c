#include "gpio.h"
#include "app_config.h"
#include "device/device.h"
#include "system/init.h"
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

#define LOG3(fmt, ...)          printf("<%s:%s>:"fmt"\r\n", __FILE__, __FUNCTION__, ##__VA_ARGS__)

static unsigned char beatcode[8] = {
    0x0e,0x0c,0x0d,0x09,
   // 0x08,0x03,0x07,0x06
    0x0B,0x03,0x07,0x06
};
//旋转度数,八个节拍为一组 5.625/64 度
void startmortor(int angle)
{
   // int beats = angle * 4076 / 360;
    int beats = angle * 4096 / 360;
    int index = 0;
    int dir = 0;
    if(beats < 0){
        dir = 1;
        beats = -beats;
    }

    while(beats != 0){




        int code = beatcode[index];

        gpio_direction_output(PORT_0,!!(code & BIT(0)));  // 1110   1100
        gpio_direction_output(PORT_1,!!(code & BIT(1)));
        gpio_direction_output(PORT_2,!!(code & BIT(2)));
        gpio_direction_output(PORT_3,!!(code & BIT(3)));
        beats--;
        if(dir){
            index = ++index & 0x07; //   0001    1110  1111
        }else{

        printf("\n ===page====%d,%d,%d, %d,  %s\n",dir,beats,index,__LINE__,__FUNCTION__);
            index = --index & 0x07;
        }
        printf("\n ===page====%d,%d,%d, %d,  %s\n",dir,beats,index,__LINE__,__FUNCTION__);
        delay_2ms(3);
    }

    gpio_direction_output(PORT_0,1);
    gpio_direction_output(PORT_1,1);
    gpio_direction_output(PORT_2,1);
    gpio_direction_output(PORT_3,1);
}



void test_montor_thread(void *priv)
{
    msleep(3000);
#if 0
    startmortor(360 * 7);
    startmortor(-360 * 7);
#else

    startmortor(180 *1);
    startmortor(-180*1);

#endif

}


int test_montor_init(void)
{
    thread_fork("test_montor_thread", 20, 2048, 64, NULL, test_montor_thread, NULL);
}
//late_initcall(test_montor_init);
