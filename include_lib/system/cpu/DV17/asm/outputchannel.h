#ifndef __OUTPUTCHANNEL_H
#define __OUTPUTCHANNEL_H
#include "asm/gpio.h"
typedef enum outputchannel_io_signal {
    CHX_CLKOUT0      = 0,
    CHX_CLKOUT1      = 1,
    CHX_CLKOUT2      = 2,
    CHX_PLL24M       = 3,
    CHX_PLL48M       = 4,
    CHX_UT0_TX       = 5,
    CHX_UT1_TX       = 6,
    CHX_UT2_TX       = 7,
    CHX_UT3_TX       = 8,
    CHX_TMR0_PWM_OUT = 9,
    CHX_TMR1_PWM_OUT = 10,
    CHX_TMR2_PWM_OUT = 11,
    CHX_TMR3_PWM_OUT = 12,
    CHX_PWM_CH0      = 13,
    CHX_PWM_CH1      = 14,
    CHX_PWM_CH2      = 15,
    CHX_PWM_CH3      = 16,
    CHX_PWM_CH4      = 17,
    CHX_PWM_CH5      = 18,
    CHX_PWM_CH6      = 19,
    CHX_PWM_CH7      = 20,
    CHX_PWM8_OUT     = 21,
    CHX_PLNK_CLK_OUT = 22,

    CHX_AVC_PORT_CH0 = 28,
    CHX_AVC_PORT_CH1 = 29,
    CHX_AVC_PORT_CH2 = 30,
    CHX_AVC_PORT_CH3 = 31,
} channlex_signal;

typedef enum outputchannle_num {
    CHNUM_0 = 0,
    CHNUM_1 = 1,
    CHNUM_2 = 2,
    CHNUM_3 = 3,

} channel_num;

extern void OutputChannelx(channel_num ch, channlex_signal io_signal, int gpio);
extern void OutputChannelx_close(channel_num ch, int gpio);
#endif

