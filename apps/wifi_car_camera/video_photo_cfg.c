#include "system/includes.h"
#include "server/video_server.h"
#include "server/ui_server.h"
#include "action.h"
#include "style.h"
#include "app_config.h"
#include "video_photo.h"
#include "video_system.h"
#include "user_isp_cfg.h"
#include "app_database.h"


extern struct video_photo_handle tph_handler;
#define __this 	(&tph_handler)


/***************************camera0***********************************/
extern int set_camera0_reso(u8 reso);

int phm_set_function_camera0(u32 parm)
{
    return 0;
}
int res_set_function_camera0(u32 parm)
{
    set_camera0_reso(parm);

    return 0;
}

int cyt_set_function_camera0(u32 parm)
{

    return 0;
}
int qua_set_function_camera0(u32 parm)
{

    return 0;
}
int acu_set_function_camera0(u32 parm)
{
    struct photo_camera *camera;
    struct server *server;
    union video_req req;

    camera = &__this->camera[0];
    server = camera->server;
    if (!camera->server) {
        server = __this->display;
    }
    if (server) {
        req.camera.shpn_level = parm;
        req.camera.cmd = SET_CAMERA_SHP_LEVEL;
        server_request(server, VIDEO_REQ_CAMERA_EFFECT, &req);
    }

    return 0;
}
int wbl_set_function_camera0(u32 parm)
{
    struct photo_camera *camera;
    struct server *server;
    union video_req req;

    camera = &__this->camera[0];
    server = camera->server;
    if (!camera->server) {
        server = __this->display;
    }
    if (server) {
        req.camera.white_blance = parm;
        req.camera.cmd = SET_CAMERA_WB;
        server_request(server, VIDEO_REQ_CAMERA_EFFECT, &req);
    }
    return 0;
}

int iso_set_function_camera0(u32 parm)
{

    return 0;
}
int exp_set_function_camera0(u32 parm)
{
    struct photo_camera *camera;
    struct server *server;
    union video_req req;

    camera = &__this->camera[0];
    server = camera->server;
    if (!camera->server) {
        server = __this->display;
    }
    if (server) {
        req.camera.ev = parm;
        req.camera.cmd = SET_CAMERA_EV;
        server_request(server, VIDEO_REQ_CAMERA_EFFECT, &req);
    }

    return 0;
}
int sok_set_function_camera0(u32 parm)
{

    return 0;
}
int dat_set_function_camera0(u32 parm)
{

    return 0;
}
int col_set_function_camera0(u32 parm)
{
    struct photo_camera *camera;

    camera = &__this->camera[0];
    if (__this->display) {
        set_isp_special_effect(__this->display, parm);
    }
    return 0;
}
int sca_set_function_camera0(u32 parm)
{

    return 0;
}

static int backlight_set_function(u32 parm)
{
#ifdef CONFIG_PWM_BACKLIGHT_ENABLE
    pwm_ch0_backlight_set_duty(parm);
#endif
    return 0;
}

int set_camera_config(struct photo_camera *camera)
{
    int err;
    union video_req req;
    struct server *server;

    server = __this->display;
    if (!server) {
        server = camera->server;
        if (!server) {
            return -EINVAL;
        }
    }

    req.camera.mode = ISP_MODE_IMAGE_CAPTURE;
    req.camera.ev = db_select("pexp");
    req.camera.white_blance = db_select("wbl");
    req.camera.cmd = /*SET_CAMERA_MODE | */SET_CAMERA_EV | SET_CAMERA_WB | SET_CAMERA_SHP_LEVEL;
    err = server_request(server, VIDEO_REQ_CAMERA_EFFECT, &req);
    if (err) {
        return err;
    }

    return 0;
}

static struct app_cfg cfg_table_camera0[] = {
    {"phm", phm_set_function_camera0},
    {"pres", res_set_function_camera0},
    {"cyt", cyt_set_function_camera0},
    {"qua", qua_set_function_camera0},
    {"acu", acu_set_function_camera0},
    {"wbl", wbl_set_function_camera0},
    {"iso", iso_set_function_camera0},
    {"pexp", exp_set_function_camera0},
    {"sok", sok_set_function_camera0},
    {"pdat", dat_set_function_camera0},
    {"col", col_set_function_camera0},
    {"sca", sca_set_function_camera0},
    {"bkl", backlight_set_function },
};
extern int stop_update_isp_scenes(void);
/* 拍照模式特效参数重置函数 */
void video_photo_cfg_reset(void)
{
    wbl_set_function_camera0(0);//白平衡设为自动
    col_set_function_camera0(0);//色彩设为无

    stop_update_isp_scenes();
}

int video_photo_set_config(struct intent *it)
{
    ASSERT(it != NULL);
    ASSERT(it->data != NULL);

    app_set_config(it, cfg_table_camera0, ARRAY_SIZE(cfg_table_camera0));

    return 0;
}


void video_photo_restore()
{
    if (get_default_setting_st()) {
        clear_default_setting_st();

        acu_set_function_camera0(db_select("acu"));
        wbl_set_function_camera0(db_select("wbl"));
        col_set_function_camera0(db_select("col"));
    }
}

