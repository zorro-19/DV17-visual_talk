#ifndef __QRCODE_H__
#define __QRCODE_H__


#ifdef __cplusplus
extern "C" {
#endif

#define QRCODE_MODE_FAST		0
#define QRCODE_MODE_NORMAL		1
#define QRCODE_MODE_ACCURATE	2


#define SYM_QRCODE	0
#define SYM_BARCODE	1

#define SYM_RIFILE_BOLT  0 //1 ： 进入枪机扫描模式； 0： 关闭枪机扫描模式
#define SYM_DIFF 0       //1 :  启动运动侦测；     0： 关闭运动侦测

/////////////////////////////////////////////////////////////////////
// function:qrcode_init
// parameters:
//		width:  the width of input image
//		height: the height of input image
//		stride: the stride of input image
//      qrmode: qrcode detect mode, QRCODE_MODE_FAST, QRCODE_MODE_NORMAL, QRCODE_MODE_ACCURATE
//      barenable:  barcode enable, enable the barcode decode
//		interval:   the barcode decode interval. barcode decoded at N frame interval, the N is between 1 and 20
//		md_thre:  运动侦测阈值,默认60
//		md_active: 运动侦测启动(1:启动，0：关闭)
//		rifile_bolt：枪机模式,(1:启动一维码枪机模式, 0:关闭一维码枪机模式)
//////////////////////////////////////////////////////////////////////

void *qrcode_init(int width, int height, int stride, int qrmode, int barenable, int interval, int md_thre, int md_active, int rifile_bolt);
int qrcode_deinit(void *decoder);
int qrcode_detectAndDecode(void *decoder, unsigned char *pixels, int *md_detected);
int qrcode_get_result(void *decoder, char **result_buf, int *result_buf_size);
int qrcode_get_symbol_type(void *decoder);
int qrcode_get_dimension(void *decoder);
int qrcode_get_qr_detect_success(void *decoder);
int qrcode_get_qr_decode_success(void *decoder);
int qrcode_get_bar_decode_success(void *decoder);

#ifdef __cplusplus
}
#endif

#endif

