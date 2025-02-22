

#ifndef __sc2313_MIPI_AE_H__
#define __sc2313_MIPI_AE_H__

#include "typedef.h"
#include "sc2313_mipi.h"



#define AE_MAX_EV                   (21004615)
#define AE_MIN_EV                   106//((1<<5)*16)
#define AE_INIT_EV                  ((1 << 10)* 16)

#define AE_CURVE_TYPE               AE_CURVE_50HZ
#if !SC2313_HDR_EN
#define AE_TARGET_LUMA              60*16

#define AE_PHASE_COMP               3

#define AE_LUMA_SMOOTH_NUM          3

#define AE_CONVER_H                 5*16
#define AE_CONVER_L                 5*16

#define AE_DIVER_H                  8*16
#define AE_DIVER_L                  8*16

#else

#define AE_TARGET_LUMA              50

#define AE_PHASE_COMP               3

#define AE_LUMA_SMOOTH_NUM          3

#define AE_CONVER_H                 10
#define AE_CONVER_L                 10

#define AE_DIVER_H                  18
#define AE_DIVER_L                  18

#endif

#define AE_RATIO_MAX_H              2048    //base=1024
#define AE_RATIO_MAX_L              512     //base=1024

#define AE_RATIO_MAX2_H             1030    //base=1024
#define AE_RATIO_MAX2_L             1018    //base=1024

#define AE_RATIO_SLOPE              128     //base=1024

#define AE_EXPLINE_TH               0

#define AE_INIT_WEIGHT_TYPE         AE_WIN_WEIGHT_AVG

#endif

