#ifndef __SC2336_MIPI_H__
#define __SC2336_MIPI_H__

#include "typedef.h"

#define SC2336_MIPI   1

#define SC2336P 1

#define SC2336_MIPI_OUTPUT_W    1920
#define SC2336_MIPI_OUTPUT_H    1080



s32 SC2336_mipi_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 SC2336_mipi_power_ctl(u8 isp_dev, u8 is_work);

s32 SC2336_mipi_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);


void SC2336_mipi_sleep();
void SC2336_mipi_wakeup();
void SC2336_mipi_W_Reg(u16 addr, u16 val);
u16 SC2336_mipi_R_Reg(u16 addr);


#endif


