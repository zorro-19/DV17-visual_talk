#ifndef  _AAC_EN_API_H
#define  _AAC_EN_API_H

#include "fs/fs.h"
#ifndef u16
#define  u32  unsigned int
#define  u16  unsigned short
#define  s16  short
#define  u8   unsigned char
#endif

typedef struct _AAC_EN_FILE_IO_ {
    void *priv;
    u16(*input_data)(void *priv, s16 *buf, u16 len);   //short
    void(*output_data)(void *priv, u8 *buf, u16 len);  //bytes
} AAC_EN_FILE_IO;


typedef struct __AAC_ENC_OPS {
    u32(*need_buf)(u16 samplerate);      //samplerate=16k   ignore
    u32(*open)(u8 *ptr, AAC_EN_FILE_IO *audioIO, void *param);
    u32(*run)(u8 *ptr);
} AAC_ENC_OPS;



typedef  struct {
    int	  sampleRate;          /*! audio file sample rate */
    int	  bitRate;             /*! encoder bit rate in bits/sec */
    short   nChannels;		   /*! number of channels on input (1,2) */
    int     OP_STDAAC;
} AACENC_PARAM;


#endif
