#ifndef __MCV_H__
#define __MCV_H__


#include "typedef.h"
#include "system/includes.h"








int mcv_mount(isp_sen_t *mcv, void *platform_data, int ch_num);
int mcv_unmount(isp_sen_t *mcv);
int mcv_ioctrl(isp_sen_t *mcv, u32 cmd, void *parm);










#endif





