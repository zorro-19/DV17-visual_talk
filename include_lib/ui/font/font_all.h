#ifndef __FONT_ALL_H__
#define __FONT_ALL_H__

#include "generic/typedef.h"
#include "font/font_sdfs.h"

typedef struct {
    u8  width;
    u8  size;
    u16 addr;
} ASCSTRUCT;

//标志位
#define FONT_GET_WIDTH    		0x01
#define FONT_SHOW_PIXEL   		0x02
#define FONT_SHOW_MULTI_LINE 	0x04 /* 显示多行文本(默认显示一行) */
#define FONT_DEFAULT			(FONT_SHOW_PIXEL)

#define FONT_ENCODE_ANSI		0x00
#define FONT_ENCODE_UNICODE		0x01
#define FONT_ENCODE_UTF8		0x02

#define FONT_ENDIAN_BIG			0x00
#define FONT_ENDIAN_SMALL		0x01

struct font_file {
    char *name;
    FILE *fd;
};

struct font {
    struct font_file file;
    u16 nbytes;
    u8 size;
    u8 *pixelbuf;
};
struct dispbuf {
    int format;
    u32 color;
    void *rect;
    void *map;
};

struct rotate_disp {
    u8 *buf;         //旋转处理临时显存
    int x;
    int y;
    int width;
};

enum FONT_STATUS {
    FT_ERROR_NONE,
    FT_ERROR_NOPIXFILE = 0x01,			//没有字模文件
    FT_ERROR_NOASCPIXFILE = 0x02,		//没有ASCII字模文件
    FT_ERROR_NOTABFILE = 0x04,			//没有TAB文件
    FT_ERROR_NOMEM = 0x08,				//内存不足
    FT_ERROR_CODEPAGE = 0x10,			//代码页错误
};

struct font_info {
    struct font ascpixel;		//ASCII像素
    struct font pixel;			//像素
    struct font_file tabfile;	//UNICODE转内码文件
    u8 sta;						//状态
    u8 ratio;					//放大倍数,默认为1
    u8 language_id;				//语言ID
    u8 bigendian;				//大端模式(unicode编码)
    u8 isgb2312;                //是否GB2312,用以区分GBK以及GB2312字库
    u8 codepage;				//代码页
    u8 fontsize;                //字号
    u16 x;
    u16 y;
    u16 text_width;				//文本宽度
    u16 text_height;			//文本高度
    u16 string_width;			//字符串宽度
    u32 flags;					//标志位
    struct dispbuf disp;		//显示相关信息
    struct rotate_disp r_disp;  //旋转处理缓存信息
    void (*putchar)(struct font_info *info, u8 *pixel, u16 width, u16 height, u16 x, u16 y);
};

#define font_ntohl(x) (unsigned long)((x>>24)|((x>>8)&0xff00)|(x<<24)|((x&0xff00)<<8))
#define font_ntoh(x) (unsigned short int )((x>>8&0x00ff)|x<<8&0xff00)

#define REGISTER_LANGUAGE_BEGIN(lang) \
	static struct font_info font_##lang##_t sec(.font_info_t)


extern struct font_info font_info_begin[];
extern struct font_info font_info_end[];

#define list_for_each_font_info(p) \
	for (p=font_info_begin; p < font_info_end; p++)

#endif
