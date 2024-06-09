#ifndef __DSI_API_H__
#define __DSI_API_H__

#include "asm/dsi.h"
#include "asm/dsi_packet.h"

void lcd_cmd_init(struct mipi_dev *dev);
void dsi_dev_init(struct mipi_dev *dev_t);
extern void dsi_video_kick_start();

#endif
