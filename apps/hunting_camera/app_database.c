#include "system/includes.h"

#include "res.h"
#include "app_database.h"
#include "app_config.h"


#define CN_PA   ((0xA9BE << 16) | ('A' << 8)  | ('B' << 0))
#define CN_PB   (('C'    << 24) | ('D' << 16) | ('E' << 8) | ('F' << 0))


/*
 * app配置项表
 * 参数1: 配置项名字
 * 参数2: 配置项需要多少个bit存储
 * 参数3: 配置项的默认值
 */
static const struct db_table app_config_table[] = {
    /*
     *  录像模式配置项
     */
    {"mic",     1,      1},                         // 录音开关
    {"mot",     1,      0},                         // 移动侦测开关
    {"par",     1,      0},                         // 停车守卫开关
    {"wdr",     1,      0},                         // 夜视增强开关
    {"num",     1,      0},                         // 车牌开关
    {"dat",     1,      1},                         // 时间标签开关
    {"two",     1,      1},                         // 双路开关
    {"gra",     2,      GRA_SEN_MD},               // 重力感应灵敏度
#ifdef CONFIG_VIDEO4_ENABLE
    {"res",     3,      VIDEO_RES_720P},           // 录像分辨率
#else
    {"res",     8,      VIDEO_RES_1080P},           // 录像分辨率
#endif
    {"cyc",     4,      0},                         // 循环录像时间，单位分钟
    {"exp",     8,      0},                         // 曝光, 范围-3到+3
    {"gap",     16,     0},                         // 间隔录影设置
    {"cna",     32,     CN_PA},                     // 车牌号码前3个字符
    {"cnb",     32,     CN_PB},                     // 车牌号码后4个字符

    /*
     *  系统模式配置项
     */
    {"kvo",     1,      1},                         // 按键音开关
    {"lag",     5,      Chinese_Simplified},        // 语言设置
    {"fre",     8,      50},                        // 灯光频率
    {"aff",     8,      0},                         // 自动关机时间, 单位分钟
    {"pro",     8,      0},                         // 屏幕保护时间, 单位秒
    {"tvm",     8,      TVM_PAL},                   // 电视制式
    {"lan",     32,     0},                         // 轨道偏移
    {"hlw",     1,      0},                         // 前照灯提醒开关
    {"bkl",     8,      100},                       // 屏背光等级(20-100)

    /*
     *  拍照模式配置项
     */
    {"sok",     1,      0},                         // 防手抖开关
    {"pdat",    1,      1},                         // 图片日期标签开关
    {"cyt",     4,      1},                         // 连拍开关
    {"qua",     3,      PHOTO_QUA_HI},              // 图片质量
    {"acu",     3,      PHOTO_ACU_HI},              // 图片锐度
    {"phm",     4,      0},                         // 延时拍照， 单位秒
    {"pres",    8,      PHOTO_RES_2M},              // 图片分辨率
    {"wbl",     4,      PHOTO_WBL_AUTO},            // 白平衡
    {"col",     4,      PHOTO_COLOR_NORMAL},        // 颜色模式
    {"sca",     4,      0},                         // 快速预览时间，单位s
    {"pexp",    8,      0},                         // 曝光设置，范围-3到+3
    {"iso",     16,     0},                         // iso

    {"mode",     3,     0},                         // 工作模式
    {"Senity",     2,   SEN_MD},                      // 感应灵敏度
    {"tallow",     1,     0},                         // allow 定时允许pir感应开机
    {"sentimer",     1,     0},                         // 设备定时起来工作

    {"vtimer",     16,     10},                         // 录影时长设置
    {"aud_vol",     16,     AUDIO_MD},                         // 音量设置  0-31   注意默认值MD  要和这个值一致
    {"dac",     16,     0x55aa},                         // dac_trim

#if (APP_CASE == __WIFI_CAR_CAMERA__)
    {"res2",    3,      VIDEO_RES_VGA},           // 录像分辨率
    {"rtf",     3,      VIDEO_RES_VGA},            //预览前视分辨率
    {"rtb",     3,      VIDEO_RES_VGA},             //预览后视分辨率
    {"wfo",     1,      1},                         //wifi开关
    {"bvo",     1,      1},                         // 开机音开关
#endif
#if (APP_CASE == __WIFI_IPCAM__)
    {"vqua",     32,     1},                     // 实时流码率
#endif


    {"jpp_avi",     1,      1},                         // 开机音开关

    {"alarm",     1,      1},                         // 闹钟参数 1 表示打开，0 关闭 ，用于PIR 间隔 和定时拍照用
//   {"pir_gap",     16,      10},                         // pir 间隔时间设置，针对第一次关机后
    {"delay_pv",     1,      0},                         // 延时拍照， 定时起来拍照，无需 PIR干涉
    {"key_state",     1,      1},                         // 用于记录按键切换状态的改变，记录照片合并成视频文件用

//  相关时间默认设置项
#if 1
// rtc系统时间
    {"rtc_y",     16,      2023},
    {"rtc_m",     8,          6},
    {"rtc_d",     8,          26},
    {"rtc_h",     8,           8},
    {"rtc_mn",     8,          8},
    {"rtc_s",     8,           8},


//pir间隔时间
    {"pir_m",     8,      0},
    {"pir_s",     8,      15},

//定时工作
    {"time_h",     8,      0},
    {"time_m",     8,      0},
    {"time_s",     8,      15},

//工作时间范围
    {"sran_h",     16,      0},
    {"sran_m",     8,      0},
    {"sran_s",     8,      0},
    {"eran_h",     8,      23},
    {"eran_m",     8,      59},
    {"eran_s",     8,      59},

#endif

};



int app_set_config(struct intent *it, const struct app_cfg *cfg, int size)
{
    int i;

    printf("app_set_config: %s, %d\n", it->data, it->exdata);

    for (i = 0; i < size; i++) {
        if (!strcmp(it->data, cfg[i].table)) {
            if (cfg[i].set) {
                int err = cfg[i].set(it->exdata);
                if (err) {
                    return err;
                }
            }
            db_update(cfg[i].table, it->exdata);
            return 0;
        }
    }

    return -EINVAL;
}





static int app_config_init()
{
    int err;

    extern int spi_flash_write_protect();
    spi_flash_write_protect();

#if defined CONFIG_DATABASE_2_RTC
    err = db_create("rtc");
    ASSERT(err == 0, "open device rtc faild\n");
#elif defined CONFIG_DATABASE_2_FLASH
    err = db_create("vm");
    ASSERT(err == 0, "open device vm faild\n");
#else
#error "undefine database device"
#endif


    printf("\n <<<<<<<<<<<<app_config_init>>>>>>>>>>>>>\n    ");
    return db_create_table(app_config_table, ARRAY_SIZE(app_config_table));
}
__initcall(app_config_init);


