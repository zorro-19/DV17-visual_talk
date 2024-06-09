#include "asm/includes.h"
#include "asm/sysinfo.h"
#include "asm/ldo.h"
#include "system/task.h"

#include "app_config.h"
#include "power_manage.h"



extern void exception_irq_handler(void *sp);
#ifdef CONFIG_DEBUG_ENABLE
extern void debug_uart_init();
#endif

static char *debug_msg[32] = {
    /*0---7*/
    "wdt_timeout",
    "reserved",
    "prp_ex_limit_err",
    "sdr_wr_err",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    /*8---15*/
    "c1_div_zero",
    "c0_div_zero",
    "c1_pc_limit_err_r",
    "c1_wr_limit_err_r",
    "c0_pc_limit_err_r",
    "c0_wr_limit_err_r",
    "c1_misaligned",
    "c0_misaligned",
    /*16---23*/
    "c1_if_bus_inv",
    "c1_rd_bus_inv",
    "c1_wr_bus_inv",
    "c0_if_bus_inv",
    "c0_rd_bus_inv",
    "c0_wr_bus_inv",
    "prp_bus_inv",
    "reserved",
    /*24---31*/
    "c1_mmu_wr_excpt",
    "c1_mmu_rd_excpt",
    "c0_mmu_wr_excpt",
    "c0_mmu_rd_excpt",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
};


void exception_analyze(u32 *sp)
{
    int i ;
    log_d("================================exception_analyze===============================\n\r");
    u32 cpu_id = current_cpu_id();
    if (cpu_id == 0) {
        log_d("CPU1 run addr = 0x%x \n", q32DSP(1)->PCRS);
        log_d("!!!!! cpu 0 %s:  DEBUG_MSG = 0x%x, DEBUG_PRP_NUM = 0x%x DSPCON=%x\n",
              __func__,  DEBUG_MSG, DEBUG_PRP_NUM, C0_CON);

    } else {
        log_d("CPU0 run addr = 0x%x \n", q32DSP(0)->PCRS);
        log_d("!!!!! cpu 1 %s:  DEBUG_MSG = 0x%x, DEBUG_PRP_NUM = 0x%x DSPCON=%x\n",
              __func__,  DEBUG_MSG, DEBUG_PRP_NUM, C1_CON);

    }


    unsigned int reti = sp[16];
    unsigned int rete = sp[17];
    unsigned int retx = sp[18];
    unsigned int rets = sp[19];
    unsigned int psr  = sp[20];
    unsigned int icfg = sp[21];
    unsigned int usp  = sp[22];
    unsigned int ssp  = sp[23];
    unsigned int _sp  = sp[24];

    for (int r = 0; r < 16; r++) {
        log_i("R%d: %lx\n", r, sp[r]);
    }

    log_i("icfg: %x \n", icfg);
    log_i("psr:  %x \n", psr);
    log_i("rets: 0x%x \n", rets);
    log_i("rete: 0x%x \n", rete);
    log_i("retx: 0x%x \n", retx);
    log_i("reti: 0x%x \n", reti);
    log_i("usp : %x, ssp : %x sp: %x\r\n\r\n", usp, ssp, _sp);

    log_i("WR_LIM4H : 0x%x, WR_LIM4L : 0x%x, SDRDBG : 0x%x\n", WR_LIM4H, WR_LIM4L, SDRDBG);
    for (i = 0; i < 32; i++) {
        if (BIT(i)&DEBUG_MSG) {
            puts(debug_msg[i]);
        }
    }



    log_flush();
    cpu_reset();
    while (1);
}

static void cpu_xbus_init()
{

    EVA_CON |= BIT(0);
    delay(10);
    EVA_CON |= BIT(1);
    delay(10);


#ifdef  CONFIG_VIDEO4_ENABLE
    //dly
    /* xbus_ch00_lvl = 1; //isp0 src wr */
    /* xbus_ch01_lvl = 1; //isp0 tnr wr */
    /* xbus_ch02_lvl = 0; //isp0 stc wr */
    xbus_ch03_lvl = 3; //imc ch0 wr
    xbus_ch04_lvl = 3; //imc ch1 wr
    xbus_ch05_lvl = 0; //imc ch2 wr
    xbus_ch06_lvl = 0; //imc ch3 wr
    xbus_ch07_lvl = 3; //imc ch4 wr
    xbus_ch08_lvl = 3; //imc ch5 wr

    xbus_ch09_lvl = 2; //imb obuf wr
    xbus_ch22_lvl = 2; //imb ibuf rd

    xbus_ch10_lvl = 2; //imr obuf wr
    xbus_ch23_lvl = 2; //imr ibuf rd

    xbus_ch24_lvl = 0; //imd ibuf rd

    xbus_ch18_lvl = 0; //imc replay

    xbus_ch19_lvl = 2; //imc ch0 osd
    xbus_ch20_lvl = 2; //imc ch1 osd
#else

    //dly
    xbus_ch00_lvl = 1; //isp0 src wr
    xbus_ch01_lvl = 1; //isp0 tnr wr
    xbus_ch02_lvl = 0; //isp0 stc wr
    xbus_ch03_lvl = 3; //imc ch0 wr
    xbus_ch04_lvl = 3; //imc ch1 wr
    /* xbus_ch03_lvl = 0; //imc ch0 wr */
    /* xbus_ch04_lvl = 0; //imc ch1 wr */

    xbus_ch05_lvl = 0; //imc ch2 wr
    xbus_ch06_lvl = 0; //imc ch3 wr
    xbus_ch07_lvl = 0; //imc ch4 wr
    xbus_ch08_lvl = 0; //imc ch5 wr
    xbus_ch09_lvl = 2; //imb obuf wr
    xbus_ch10_lvl = 3; //imr obuf wr
   // xbus_ch10_lvl = 2; //imr obuf wr

    xbus_ch16_lvl = 1; //isp0 src rd
    xbus_ch17_lvl = 1; //isp0 tnr rd
    xbus_ch18_lvl = 2; //imc replay
    xbus_ch19_lvl = 3; //imc ch0 osd
    xbus_ch20_lvl = 3; //imc ch1 osd
    xbus_ch21_lvl = 2; //
    xbus_ch22_lvl = 2; //imb ibuf rd
    xbus_ch23_lvl = 2; //imr ibuf rd
    xbus_ch24_lvl = 0; //imd ibuf rd
    xbus_ch25_lvl = 2; //
    xbus_ch26_lvl = 2; //

    xbus_lv1_prd = 8;
    xbus_lv2_prd = 16;
#endif

    AVC_BUS_CON = 0;
    /* AVC_BUS_CON = 1 | (31 << 1) | (9 << 6); */
    AVC_BUS_CON = 1 | (8 << 1) | (3 << 6);

    xbus_dist0_ext = 1;
    /* xbus_dist1_ext = 1; */

    log_i("\n\nxbus config\n\n\n");

    clk_set("avc", 360000000);
}

extern u32 text_rodata_begin, text_rodata_end;

void cpu1_main()
{
    local_irq_disable();

    interrupt_init();

    request_irq(1, 7, exception_irq_handler, 1);

    debug_init();

    os_start();

    local_irq_enable();

    while (1) {
        __asm__ volatile("idle");
    }
}

static void wdt_init()
{
    /*
     * 超时: 0-15 对应 {1ms, 2ms, 4ms, 8ms, ...512ms, 1s, 2s, 4s, 8s, 16s, 32s}
     */
    CLK_CON0 |= BIT(9);
    CRC1_REG = 0x6EA5;
    WDT_CON = BIT(5) | BIT(6) | BIT(4) | 0x0c;
    CRC1_REG = 0;
}

__attribute__((noinline))
void clr_wdt()
{
    WDT_CON |= BIT(6);
    //do st here

}

void close_wdt()
{
    CRC1_REG = 0x6EA5;
    WDT_CON &= ~BIT(4);
    CRC1_REG = 0;
}


/*
 *lev: 0--->2.2v
 *lev: 1--->2.3v
 *lev: 2--->2.4v
 *lev: 3--->2.5v
 *lev: 4--->2.6v
 *lev: 5--->2.7v
 *lev: 6--->2.8v
 *lev: 7--->2.9v
 */
void lvd_cfg(u8 lev)
{
    LVD_CON = 0;//先关闭,再配置

    LVD_CON |= (0x7 & (lev));//lev
    /* LVD_CON &= ~BIT(7);//0:force reset at onece   1:force reset delay after 40us */
    LVD_CON |= BIT(7);//0:force reset at onece   1:force reset delay after 40us
    delay(10);
    LVD_CON &= ~BIT(6);//force reset system
    delay(10);
    LVD_CON &= ~BIT(5);//ldo_in
    delay(10);
    LVD_CON |= BIT(4);//AEN
    delay(100);
    LVD_CON |= BIT(3);//AOE
    delay(10);
}


u32 sdfile_init(u32 cmd_zone_addr, u32 *head_addr, int num);

/*
 * 此函数在cpu0上电后首先被调用,负责初始化cpu内部模块
 *
 * 此函数返回后，操作系统才开始初始化并运行
 *
 */

int net_config_mode=0;
void setup_arch()
{
    /*c++运行初始化*/
#if  0
unsigned char read_net_config_key();
static int count=0;
if(read_net_config_key()==0){

  // for(int i=0;i++;i<100){

    if(read_net_config_key()==0){


     net_config_mode=1;


    }else{
      net_config_mode=0;
    // break ;
    }
   // delay2ms(1);
  // }

}
#endif
#ifdef CONFIG_FAST_CAPTURE
    gpio_direction_output(IO_PORTA_05, 1);
#endif



  //  extern void usb_switch_mode(u8 state);

  //  usb_switch_mode(0);
#ifdef LONG_POWER_IPC



#ifndef CONFIG_ISP_PCCAMERA_MODE_ENABLE


    #if  1
     gpio_direction_output(IO_PORTH_09, 0);//DV17 connect 7016
    #else
     gpio_direction_output(IO_PORTH_09, 1);//三极管方式控制
    #endif


#else

     gpio_direction_output(IO_PORTH_09, 1); //DV17 connect PC

#endif

#endif


    wdt_init();

    void cpp_run_init(void);
    cpp_run_init();

    clk_early_init();

    interrupt_init();

#ifdef CONFIG_DEBUG_ENABLE
    debug_uart_init();
#endif
    log_early_init(8 * 1024);

    puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("         setup_arch %s %s\n", __DATE__, __TIME__);
    puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    rtc_early_init();

    cpu_xbus_init();

    sys_power_early_init();

    debug_init();

    dac_early_init(1,  DAC_ISEL_FULL_PWR, 100);



    /*
     * 内部ldo设为1.1v
     */
    /* dvdd_ctrl(DVDD_0904); */
    /* ddrvdd_ctrl(DDRVDD_1596_2300, 0, 0); *///ddrvdd uboot用于控制ddr的供电使能，此处不可操作
    lvd_cfg(6);

    request_irq(1, 7, exception_irq_handler, 0);



}








