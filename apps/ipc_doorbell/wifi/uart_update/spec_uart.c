#include "device/uart.h"
#include "os/os_compat.h"
#include "system/includes.h"
#include "app_config.h"
#ifdef USER_UART_UPDATE_ENABLE

/*****************************************************************************
 Description:特殊串口初始化函数
******************************************************************************/

#define UART_CBUF  (1 * 1024)

static void *uart_dev_handle;

int spec_uart_upg_recv(char *buf, u32 len)
{
    return dev_read(uart_dev_handle, buf, len);
}

int spec_uart_upg_send(char *buf, u32 len)
{
    return dev_write(uart_dev_handle, buf, len);
}

extern void uart_data_decode(u8 *buf, u16 len);
static char buf[512];
static void uart_rec_task(void *arg)
{
	 int len;


	 while(1)
	 {



		 len = spec_uart_upg_recv(buf, 200);
        // printf("\n len>>>>>>>>>>>>>>>>>>>>>>>>>%d \n",len);
		 if(len > 0)
		 {

		  	 uart_data_decode(buf,  len);
           //  printf("\n  <<<<<<<<<<rec_uart>>>>>>>>>>>%s>>>>>>>>>",__FUNCTION__);
			// spec_uart_upg_send(buf, len);
			// put_buf(buf, len);
		 }
	    // os_time_dly(1);
    }
}

static void uart_send_task(void *arg)
{
	while(1)
	{
		puts("\n <<<<<<<<<<<<<<\n");
		spec_uart_upg_send("aaaaaaaaaaa", 10);
	}
}

//  int uart_clk=clk_get("uart");
//  UT1_BAUD = (uart_clk / data->baudrate) / 4 - 1;

void spec_uart_init(void *priv)
{
#if 0
	static u8 buf[512];
	int len;
#endif
	//puts("\n spec_uart_init \n");
	static char uart_circlebuf[UART_CBUF] __attribute__((aligned(32))); //串口循环数据 buf,根据需要设置大小
	int parm;

	uart_dev_handle = dev_open("uart1", 0);

	printf("\n uart_dev_handle = 0x%x \n", uart_dev_handle);


	ASSERT(uart_dev_handle != NULL, "open uart err");


	/* 1 .设置接收数据地址 */
	dev_ioctl(uart_dev_handle, UART_SET_CIRCULAR_BUFF_ADDR, (int)uart_circlebuf);
	parm = sizeof(uart_circlebuf);

	/* 2 .设置接收数据地址长度 */
	dev_ioctl(uart_dev_handle, UART_SET_CIRCULAR_BUFF_LENTH, (int)parm);

	/* 3 .是否设置为接收完指定长度数据,spec_uart_recv 才出来 */
#if 0
	parm = 1;
	dev_ioctl(uart_dev_handle, UART_SET_RECV_ALL, (int)&parm);
#endif

	/* 4 .设置接收数据为阻赛方式,需要非阻赛可以去掉，建议加上超时设置 */
#if 1
	parm = 1;
	dev_ioctl(uart_dev_handle, UART_SET_RECV_BLOCK, (int)parm);
#endif

	/* 5 . 使能特殊串口 */
	dev_ioctl(uart_dev_handle, UART_START, (int)0);


	thread_fork("uart_rec_task", 14, 0x1000, 2048, 0, uart_rec_task, NULL);
//	os_task_create(uart_rec_task, 0, 26, 1024, 32, "uart_rec_task");
#if 0
	extern void close_wdt();
	close_wdt();
	while (1)
	{
		len = spec_uart_upg_recv(buf, 200);
		os_time_dly(1);
		/*spec_uart_upg_send(buf, len);*/
		put_buf(buf, len);
		os_time_dly(1);
	}
#endif

}



#endif


