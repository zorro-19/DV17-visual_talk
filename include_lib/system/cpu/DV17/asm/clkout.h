#ifndef __CLKOUT_H
#define __CLKOUT_H


typedef enum _clkout0_io_signal {
    CLKOUT0_XOSC0_CLK         = 0,
    CLKOUT0_IRTC_CLK          = 1,
    CLKOUT0_PLL_12M           = 2,
    CLKOUT0_PLL_24M           = 3,
    CLKOUT0_PLL_48M           = 4,
    CLKOUT0_PLL_CH0_CLK       = 5,
    CLKOUT0_PLL_CH1_CLK       = 6,
    CLKOUT0_SYS_CLK           = 7,
} clkout0_signal;


typedef enum _clkout1_io_signal {
    CLKOUT1_XOSC1_CLK         = 0,
    CLKOUT1_IRTC_CLK          = 1,
    CLKOUT1_PLL_12M           = 2,
    CLKOUT1_PLL_24M           = 3,
    CLKOUT1_PLL_48M           = 4,
    CLKOUT1_PLL_CH0_CLK       = 5,
    CLKOUT1_PLL_CH1_CLK       = 6,
    CLKOUT1_SYS_CLK           = 7,
} clkout1_signal;


typedef enum _clkout2_io_signal {
    CLKOUT2_OSC_CLK           = 0,
    CLKOUT2_HTC_CLK           = 1,
    CLKOUT2_IRTC_CLK          = 2,
    CLKOUT2_PLL_12M           = 3,
    CLKOUT2_PLL_24M           = 4,
    CLKOUT2_PLL_CH0_CLK       = 5,
    CLKOUT2_PLL_CH1_CLK       = 6,
    CLKOUT2_SYS_CLK           = 7,
} clkout2_signal;

void clkout0_func(clkout0_signal signal, u8 en);
void clkout1_func(clkout1_signal signal, u8 en);
void clkout2_func(clkout2_signal signal, u8 en);

#endif
