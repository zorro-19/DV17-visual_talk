
#include "typedef.h"
#include "asm/cpu.h"
#include "device/video.h"

#define BS_LEN 100*1024
#define MAX_WIDTH 1920
#define MAX_HEIGHT 1088

#define SLICE_TYPE_I    2
#define SLICE_TYPE_P    0

#define DECODE_NO_REF_FRAME 0

struct h264_dec_info;

struct h264_decode {
    // int sample_index;
    // int frame_cnt;
    int frame_type;
    int ref_flag;
    // int yuv_flag;

    int i_deblocking_filter_alphac0;
    int i_deblocking_filter_beta;
    int i_disable_deblocking_filter_idc;
    int i_alpha_c0_offset;
    int i_beta_offset;
    int b_deblocking_filter_control;

    int qp_min;
    int qp_max;
    int i_pic_init_qp;
    int i_pic_init_qs;
    int i_global_qp;
    int i_chroma_qp_index_offset;

    int yuv_pic_w;
    int yuv_pic_h;
    int i_mb_width;
    int i_mb_height;
    int i_log2_max_frame_num;

    // int frame_size;
    // int total_size;
    // int next_frame_ptr;
    int i_nal_type;
    int i_cabac_init_idc;
    int i_level;
    int i_profile_idc;

    int dec_no_ref_frame_en;
    int no_ref_frame_flag;
    int pre_no_ref_frame_flag;
    int pskip_frame_flag;
    int pre_pskip_frame_flag;
    int ref_id;
    int i_ref0;
    int dsw_size;
    int dsw_limit_pos;


    // int dec_data_addr;
    // int dec_data_addr_hw;
    int out_yuv_addr0;
    int out_yuv_addr1;
    int out_yuv_addr2;
    // int remain_len;
    // int left_len;

    // int lirp_cnt;
    // int sirp_cnt;

    u32 sza_addr;
    u32 szb_addr;
    u32 cpa_addr;
    u32 cpb_addr;
    u32 cpa_waddr;
    u32 cpb_waddr;

    // u16 cache_data;
    // u8  cache_bit;
    // u8 *fp_dec_inbuf;
    // u8 skip_pframe;

    void *output_priv;
    int (*output_handler)(void *, struct YUV_frame_data *);

    void *output_stop_priv;
    int (*output_stop)(void *);

    u16 dec_lines;
    int status;
};


typedef struct {
    u8 rbuf[512];
    u8 lbuf[512];
    u8 wbuf[1920 * 1090];
} H264_DEC_BUF;

enum nal_unit_type_e {
    NAL_UNKNOWN = 0,
    NAL_SLICE   = 1,
    NAL_SLICE_DPA   = 2,
    NAL_SLICE_DPB   = 3,
    NAL_SLICE_DPC   = 4,
    NAL_SLICE_IDR   = 5,    /* ref_idc != 0 */
    NAL_SEI         = 6,    /* ref_idc == 0 */
    NAL_SPS         = 7,
    NAL_PPS         = 8,
    NAL_AUD         = 9,
    NAL_FILL        = 12,
    /* ref_idc == 0 for 6,9,10,11,12 */
};



void h264_dec_init();
void *h264_dec_open();
int h264_dec_input_data(void *fd, u8 *stream, int len);
int h264_dec_stop(void *fd);
int h264_dec_close(void *fd);
int h264_dec_set_output_handler(void *_avc, void *priv,
                                int (*handler)(void *priv, struct YUV_frame_data *));
void h264_dec_set_output_callback(void *_avc, void *stop_priv, int (*stop_cb)(void *));

/*extern H264_DEC_BUF x264_dec_buf;*/

extern void x264_skip_head_4word();
extern void x264_sps_decode(struct h264_decode *h264);
extern void x264_pps_decode(struct h264_decode *h264);
extern void x264_sliceheader_decode(struct h264_decode *h264);
extern void x264_fill_data_decode(struct h264_decode *h264);
extern void my_flush_dcache_dec();
extern void flush_align_hasbits();
extern void flush_align_hasbits();
extern u8 *get_ref_header_addr();
extern u8 *get_ref_addr();
extern u8 *get_stream_addr();
extern void dec_isr(void);
int get_rbsp_trailing(void);
void flush_naltag_bits();

