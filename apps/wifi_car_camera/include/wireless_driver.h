#ifndef __WIRE_DRIVER_H__
#define __WIRE_DRIVER_H__


#include "system/includes.h"
#include "cpu.h"
#include "ioctl.h"
#include "typedef.h"
// #include "device.h"
// #include "errno-base.h"


//无线设备类型
#define RF_DEV		1



#define RF_MODE_HOST			0xa1
#define RF_MODE_SLAVE			0xa2

#define RF_WORK_TX 				0x01
#define RF_WORK_RX 				0x02

#define RF_PAIR_ID       		0xEEEE

#define MAX_RESEND_CNT          3

#define PACK_TYPE_CMD    		0xFC
#define PACK_TYPE_DAT    		0xFD
#define CMD_PACK_INFO_LEN			16
#define DAT_PACK_INFO_LEN			16

//标识上一帧视频帧是否正常接收完成
#define VID_NORMAL_ACCEPT			0xae
#define VID_ABORT_ACCEPT			0xbb

//数据包ack/req
#define CHAT_ACK_VID			BIT(0)
#define CHAT_ACK_VID_FEND		BIT(1)
#define CHAT_ACK_CMD			BIT(2)
#define CHAT_REQ				BIT(7)


enum CHAT_STREAM_TYPE {
    CHAT_STREAM_NULL,
    CHAT_STREAM_VID,
    CHAT_STREAM_AUD,
    CHAT_STREAM_CMD,
    CHAT_STREAM_CMD_ACK,

    CHAT_STREAM_TYPE_END,
};


enum PAYLOAD_CMD {
    PCMD_NONE,
    PCMD_SET_PAIR = 0x10,
};

enum transfer_status {
    RF_STA_IDLE,
    RF_TX_STA_BEG,
    RF_TX_STA_END,
    RF_TX_STA_ERR,
    RF_TX_STA_LIMIT,

    RF_RX_STA_BEG,
    RF_RX_STA_END,
    RF_RX_STA_ERR,
    RF_RX_STA_LIMIT,
};

enum {
    WLESS_SET_PAIR,
    WLESS_GET_CMD_SIZE,
    WLESS_GET_DAT_SIZE,
};

struct wireless_platform_data {
    char *spi_name;
    u32 io_trig0;
    u32 io_trig1;
    u32 rx_sw;
    u32 tx_sw;
    u32 spi_cs;
    u32 wkup_ev0;
    u32 wkup_ev1;
};

struct wireless_drv;

//每个无线模块驱动，都实现以下接口，统一管理
struct wireless_ops {
    // bool (*online)(struct wireless_drv * drv);
    void (*init)(void *priv);
    int (*open)(struct wireless_drv *drv, u32 mode, u32 arg);
    int (*ioctl)(struct wireless_drv *drv, u32 cmd, u32 arg);
    int (*close)(struct wireless_drv *drv);
    int (*check)(struct wireless_platform_data *pdata);
    int (*host_tx)(void *drv, u8 *buf, u32 len);
    void *(*host_rx)(void *drv, u8 *buf, u32 len);
    int (*slave_tx)(void *drv, u8 *buf, u32 len);
    void *(*slave_rx)(void *drv, u8 *buf, u32 len);
};

struct wireless_drv {
    const char *name;
    int type;
    u8 used;
    void *private_data;

    struct wireless_ops ops;

};


extern struct wireless_drv wireless_driver_begin[];
extern struct wireless_drv wireless_driver_end[];


#define REGISTER_WIRELESS(drv) \
	static struct wireless_drv drv sec(.wireless_driver)


#define list_for_each_wireless(w) \
	for (w=wireless_driver_begin; w<wireless_driver_end; w++)






#define WIRELESS_PLATFORM_DATA_BEGIN(data) \
	const struct wireless_platform_data data = { \


#define WIRELESS_PLATFORM_DATA_END() \
	};



int wless_drv_open(u32 type, u32 mode, u32 arg);
int wless_drv_close();
int wless_drv_ioctl(u32 cmd, u32 arg);
int wless_drv_htx(u8 *buf, u32 len);
void *wless_drv_hrx(u8 *buf, u32 len);
int wless_drv_stx(u8 *buf, u32 len);
void *wless_drv_srx(u8 *buf, u32 len);


















#endif








