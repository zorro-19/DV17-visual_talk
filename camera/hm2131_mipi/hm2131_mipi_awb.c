
#include "asm/isp_alg.h"
#include "hm2131_mipi_awb.h"


static u8 awb_weight_cust[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};


static isp_awb_gw1_params_t awb_gw1_params = {
    .wp_th = 50,
    .r_th = 235,
    .g_th = 235,
    .b_th = 235,

    .wp_ratio_th_numerator = 1,
    .wp_ratio_th_denominator = 32,

    .rgain_min = 256,
    .rgain_max = 3072,
    .bgain_min = 256,
    .bgain_max = 3072,

    .ev_th = 2048,
};

static isp_awb_params_t hm2131_mipi_awb_params = {
    .awb_alg_type = AWB_ALG_TYPE,
    .awb_scene_type = AWB_SCENE_TYPE,
    .awb_win_type = AWB_WIN_TYPE,

    .awb_init_gain =
    {.r_gain = ISP_AWB_ONE, .g_gain = ISP_AWB_ONE, .b_gain = ISP_AWB_ONE},

    .awb_scene_gains =
    {
        { .r_gain = 1620, .g_gain = ISP_AWB_ONE, .b_gain = 1580 },
        { .r_gain = 1893, .g_gain = ISP_AWB_ONE, .b_gain = 1480 },
        { .r_gain = 1170, .g_gain = ISP_AWB_ONE, .b_gain = 2330 },
        { .r_gain = 1893, .g_gain = ISP_AWB_ONE, .b_gain = 1480 },
        { .r_gain = 1467, .g_gain = ISP_AWB_ONE, .b_gain = 1986 },

    },

    .awb_win_cust_weights = awb_weight_cust,

    .awb_gw1_params = &awb_gw1_params,
};


void *hm2131_mipi_get_awb_params()
{
    return (void *)&hm2131_mipi_awb_params;
}
