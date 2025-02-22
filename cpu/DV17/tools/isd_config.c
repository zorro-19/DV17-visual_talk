

// *INDENT-OFF*
#include "app_config.h"


[：：：：：：：：： 配置文件使用说明 ：：：：：：：：：：：：：：]
[：： ‘#’注释一行 ：：：：：：：：：：：：：：：：：：：：：：：：]
[：： 配置格式为 ：配置项=参数;必须以 ';'结束 。：：：：：：：：：]
[：： 配置项';'后面的内容也是被注释的。：：：：：：：：：：：：：：]
[：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：]





[flash信息描述]
FLASH_DESCRIBE_BEGIN;
{
73647466,SD&TF,2G,1c00,1c01,1c01,1c01,1c01;
9B12,ATO25D1GA,128M,1c00,1c01,1c01,1c01,1c01;
C8F2,GD5F1G,128M,1c00,1c01,1c01,1c01,1c01;
A1E1,PN26G01A,8M,1400,1c01,1c01,1c01,1c01;
D84014,md25D80,1M,1C01,1C01,1C01,1C01,1C01;
C22018,BG25Q80,1M,1C01,1C01,1C01,1C01,1C01;
C22014,BG25Q80,1M,1C01,1C01,1C01,1C01,1C01;
514012,MD25D20,256K,14,14,14,14,14;
514013,MD25D40,512K,18,18,18,18,18;
514014,MD25D80,1M,18,18,18,18,18;
514015,MD25D16,2M,18,18,18,18,18;
C84012,GD25Q20,256K,10028,10028,10028,10028,10028;
C84013,GD25Q40,512K,1002C,1002C,1002C,1002C,1002C;
C84014,GD25Q80,1M,10030,10030,10030,10030,10030;
C84015,GD25Q16,2M,10034,10034,10034,10034,10034;
C84016,GD25Q32,4M,10038,10038,10038,10038,10038;
C84017,GD25Q64,8M,10038,10038,10038,10038,10038;
C84018,GD25Q128,16M,10038,10038,10038,10038,10038;
EF4014,W25Q80,1M,10030,10030,10030,10030,10030;
EF4015,W25Q16,2M,10034,10034,10034,10034,10034;
EF4016,W25Q32,4M,10038,10038,10038,10038,10038;
EF4017,W25Q64,8M,10038,10038,10038,10038,10038;
EF4018,W25Q128,16M,10038,10038,10038,10038,10038;
E04013,BG25Q40,512K,1002C,1002C,1002C,1002C,1002C;
E04014,BG25Q80,1M,10030,10030,10030,10030,10030;
E04015,BG25Q16,2M,14,14,14,14,14;

C22015,KH25L1606E,2M,30,30,30,30,30;

[BOYA Flash]
684013,BY25D40,512K,18,0,0,0,1c;
684015,BY25D16,2M,18,0,0,0,1c;
684016,BY25Q32,4M,14004,0,0,0,1c;
}
FLASH_DESCRIBE_END;

[flash信息描述结束，新加的flash信息必须放在此行的上面]
[FLASH型号选择]
FLASH_ID=EF4016;




[SPI FLASH 引脚配置]
SPI_CS_DESELECT = 3;
SPI_INPUT_DELAY=1;
SPI_DATA_WIDTH=2;		[0--1wire_1bit 1--2wire_1bit 2--2wire_2bit 3/4--4wire_4bit]
SPI_IS_CONTINUE_READ=0;	[1/0]
SPI_IS_OUTPUT=1;		[1 -- Output mode 0 --I/O mode]
SPI_CLK_DIV=4;			[0-7]


[SD卡出口和电源配置]
sdmmc=1;	[0|1|2]
sdmmc_port=B;	[A|B|C]

[SD卡电源脚 + 有效电平]
sdmmc_power_io = -1;        [SD卡电源脚 + 有效电平]

[sd卡内代码文件的文件名，需要是短文件名]
code_file_name=JL_AC5X.BFU;	[sd卡内代码文件的文件名，需要是短文件名]


[时钟模块配置]
[clock modules configuration]
[当osc_type = HTC时，osc_freq的解析如下：]
[  osc_freq = 0  需要接usb主机trim htc]
[  osc_freq = 1  htc trim值从efuse读出]
[  osc_freq = 2  按键按下则需要usb主机trim htc，否则trim值从efuse读出]
[                    a. 配置port_input则按键检测port_input]
[                    b. 配置port_input和port_output则检测port_input && port_output]
osc_type = OSC_RTC;             [OSC0|OSC1|OSC_RTC|HTC]
osc_freq = 32768;
osc_hc_en = 0;                  [0|1]
osc_1pin_en = 0;

[系统时钟配置   240|192|160|120]
sys_clk=240MHz;		[240|192|160|120]

[编码时钟配置]
eva_clk=240MHz;		[240|192|160|120]
#ifdef SDRAM_DDR2
avc_clk=360MHz;		[160 240 300 330 360]
#else
avc_clk=300MHz;		[160 240 300 330 360]
#endif

[使能DDR]
enable_sdram=1;

[LSB 分频 0-7]
lsb_div=1;			[0-7 ]

[uboot 串口引脚定义 不定义uart_io则关闭uboot打印]
uart_io=PH13;
uart_baud=3;

[调试开关 部分bt芯片支持]
;#port_input=PR03_00 	;	PR3 PU=0 PD=0
;#port_output=PH12_1 ;	PORTG0 out high



[sdtap 引脚配置]
[sdtap：CLK	DAT]
[0：disable]
[1：PF0	PF1]
[2：PD12	PD13]
[3：PA6	PA8]
[4：PA10	PA11]
sdtap=0	;


[DDR配置]
#if defined SDRAM_DDR1
sdram_clk = 200MHz;//[300 400 500]
sdram_mode = DDR1; //[ddr2:ddr1]
#if SDRAM_SIZE == (64 * 1024 * 1024)
sdram_colum = 1;              // [9~10]The Column Address is specified byA0-9
sdram_size = 64M;
#elif SDRAM_SIZE == (32 * 1024 * 1024)
sdram_colum = 0;              // [9~10]The Column Address is specified byA0-9
sdram_size = 32M;
#elif SDRAM_SIZE == (16 * 1024 * 1024)
sdram_colum = 0;              // [9~10]The Column Address is specified byA0-9
sdram_size = 16M;
#else
#error "unknown SDRAM_SIZE"
#endif
sdram_cl = 3;        // [2-7]
sdram_wlcnt = 0;
sdram_d_dly = 0;
sdram_rlcnt = 3;
sdram_q_dly = 0;
sdram_phase = 0;

sdram_dq_dly_trm      = 0;
sdram_dqs_dly_trm     = 2;
sdram_udqs_dly_trm    = 2;//[13-27] 内置DDR 10，外置ddr 20
sdram_ldqs_dly_trm    = 2;//[13-27] 内置DDR 10，外置ddr 20
sdram_dq_dly_inner_trm= 2;

[DDR universally timing configuration]
sdram_trfc = 70;              //  Refresh Cycle Time Unit is ns[1-63]
sdram_trp = 18;              //   Row Precharge time Unit is ns[1-15]
sdram_trcd = 16;             //   Row to Column Delay Unit is ns[1-15]
sdram_trrd = 10;             //   Act to Act Delay time Unit is ns[1-15]
sdram_twtr = 2;			//		[5]个tck
sdram_trtw = 4;			//		[4-5]个tck 外置ddr使用5，内置ddr使用4
sdram_twr  = 3;			//		2个tck
sdram_trc  = 60;		//		0-15 Unit is ns




#elif defined SDRAM_DDR1_ET
sdram_clk = 250MHz;//[300 400 500]
sdram_mode = DDR1; //[ddr2:ddr1]
#if SDRAM_SIZE == (64 * 1024 * 1024)
sdram_colum = 1;              // [9~10]The Column Address is specified byA0-9
sdram_size = 64M;
#elif SDRAM_SIZE == (32 * 1024 * 1024)
sdram_colum = 0;              // [9~10]The Column Address is specified byA0-9
sdram_size = 32M;
#elif SDRAM_SIZE == (16 * 1024 * 1024)
sdram_colum = 0;              // [9~10]The Column Address is specified byA0-9
sdram_size = 16M;
#else
#error "unknown SDRAM_SIZE"
#endif
sdram_cl = 3;        // [2-7]
sdram_wlcnt = 0;
sdram_d_dly = 0;
sdram_rlcnt = 3;
sdram_q_dly = 0;
sdram_phase = 0;

sdram_dq_dly_trm      = 7;
sdram_dqs_dly_trm     = 0;
sdram_udqs_dly_trm    = 0;//[13-27] 内置DDR 10，外置ddr 20
sdram_ldqs_dly_trm    = 0;//[13-27] 内置DDR 10，外置ddr 20
sdram_dq_dly_inner_trm= 3;

[DDR universally timing configuration]
sdram_trfc = 70;              //  Refresh Cycle Time Unit is ns[1-63]
sdram_trp = 18;              //   Row Precharge time Unit is ns[1-15]
sdram_trcd = 16;             //   Row to Column Delay Unit is ns[1-15]
sdram_trrd = 10;             //   Act to Act Delay time Unit is ns[1-15]
sdram_twtr = 2;			//		[5]个tck
sdram_trtw = 4;			//		[4-5]个tck 外置ddr使用5，内置ddr使用4
sdram_twr  = 3;			//		2个tck
sdram_trc  = 60;		//		0-15 Unit is ns





#elif defined SDRAM_DDR2
sdram_clk = 400MHz; //[300 400 500]
sdram_mode = DDR2; //[ddr2:ddr1]
#if SDRAM_SIZE == (64 * 1024 * 1024)
sdram_colum = 1;     //          [9~10]The Column Address is specified byA0-9
sdram_size = 64M;
#elif SDRAM_SIZE == (32 * 1024 * 1024)
sdram_colum = 0;     //          [9~10]The Column Address is specified byA0-9
sdram_size = 32M;
#elif SDRAM_SIZE == (16 * 1024 * 1024)
sdram_colum = 0;     //          [9~10]The Column Address is specified byA0-9
sdram_size = 16M;
#else
#error "unknown SDRAM_SIZE"
#endif
sdram_cl = 5;       //  [2-7]
sdram_wlcnt = 0;
sdram_rlcnt = 5;
sdram_q_dly = 1;
sdram_d_dly = 1;
sdram_phase = 2;

sdram_dq_dly_trm      = 7;
sdram_dqs_dly_trm     = 0;
sdram_udqs_dly_trm    = 0;//[13-27] 内置DDR 10，外置ddr 20
sdram_ldqs_dly_trm    = 0;//[13-27] 内置DDR 10，外置ddr 20
sdram_dq_dly_inner_trm= 0;

sdram_trfc = 105;           //     Refresh Cycle Time Unit is ns[1-63]
sdram_trp = 13;             //    Row Precharge time Unit is ns[1-15]
sdram_trcd = 13;            //    Row to Column Delay Unit is ns[1-15]
sdram_trrd = 12;            //    Act to Act Delay time Unit is ns[1-15]
sdram_twtr = 3;				//	[2]个tck
sdram_trtw = 3;				//	[4-5]个tck 外置ddr使用5，内置ddr使用4
sdram_twr  = 4;				//	2个tck
sdram_trc  = 58;		//		0-15






#else
#error "unknown DDR"
#endif

[DDR universally configuration]
sdram_refresh_time = 32;    // refresh cycles
sdram_refresh_cycles=8K;//	[4096|8192]



