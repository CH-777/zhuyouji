//sys
#include "sys.h"
#include "usart.h"
//bsp
#include "usart.h"	  
#include "input_gpio.h"
#include "output_gpio.h"
#include "oled.h"
#include "delay.h"
#include "adc.h"
#include "encoder.h"
#include "sd.h"
#include "gpio.h"
//rtos
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
//modbus
#include "mb.h"
#include "mbport.h"
#include "mbutils.h"
//easylogger
#include "elog.h"
//app
#include "oil.h"

#include "reg_def.h"
/**************************************************************************
 *                   ！！！！注意！！！
 *  本程序采用内部时钟源
 *  若需要改为外部时钟源需进行以下几处的修改
 *   1、 修改 system_stm32f10x.c文件212行的SystemInit函数中的SetSysClock函数（456行）
 *      注释掉470行的SetSysClockTo64，取消注释469行的SetSysClockTo72
 *   2、将该文件中115行的宏定义#define SYSCLK_FREQ_72MHz  64000000  修改为 
 *       #define SYSCLK_FREQ_72MHz  72000000
 *   
 *************************************************************************/

void FreeModbus_Task(void * pvParameters)
{ 
	uint8_t slave_addr;
	slave_addr = get_address1();   //?无法读取addr1的值
	slave_addr = (slave_addr << 1) | get_address0();
	if(slave_addr == 0)
		slave_addr = 2;
	log_i("modbus slave addr is %d",slave_addr);
//	if(slave_addr == 0) {
//	 	log_e("modbus slave addr error");
//	}
	// //使用串口1作为modbus通讯接口，从机地址为0x01   
    eMBInit(MB_RTU, slave_addr, 1, 9600, MB_PAR_NONE); 
//    eMBInit(MB_RTU, 0x01, 1, 9600, MB_PAR_NONE); 
    eMBEnable();
	log_i("modbus slave task init");
	
	// start_pulse_count();
    while(1) {
       (void)eMBPoll();
		vTaskDelay(pdMS_TO_TICKS(7));
		// log_i("cnt is %d", get_pulse_count());
		// vTaskDelay(1000);
    }
}

void Bsp_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);	
	
	uart2_init(9600);   //使用串口2(PA2、PA3)作为调试串口
	
	GPIO_clk_enable();
	input_gpios_register();
	input_gpios_it_register();
	output_gpios_register();
	
	elog_init();
	elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL & ~ELOG_FMT_P_INFO);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
	elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
	elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC|ELOG_FMT_P_INFO));
	elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
	elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));	
	elog_start();

	log_i("system start");
	
	adc_init();
	TIM2_Cap_Init(0XFFFF,64-1);
//	#if USE_SD
//	SD_Initialize();   //SD卡初始化,该片选引脚为PA3，板子上是PC4
//	#endif
	oil_init();
}
/**********************************   MAIN    ***********************************/
/***********************************************************/
/**********************************************************/
int main(void)
{
	Bsp_Init();
	
	// open_test_led0();
	// open_test_led1();
	
	xTaskCreate(FreeModbus_Task, "FreeModbus", 256, NULL, 4, NULL);
	vTaskStartScheduler();
	while(1){
	} 
	
}


/************************************************************************/
/*                  自动模式流程
 * 一、打开总开关上电---手动设定PID温控仪加热温度
 *       将模式选择开关置于自动运行模式状态	
 * 二、按下启动按钮 --- 进入自动运行状态
 * 三、当PID温控仪控制加热到设定温度时，注油就绪，中断到达
 * 四、触摸屏上设定注油参数----选配方
 *    选择好配方后，按下注油枪按钮---触发注油，并开启脉冲计数
 * 五、当脉冲计数大于等于计算脉冲数时，注油完成，触发回吸
 * 
 * 
 *            手动模式流程
 * 一、打开总开关上电----手动设定PID温控仪加热温度
 *        将模式选择开关置于手动运行状态
 * 二、触摸屏进行油液加热操作
 * 三、加热温度范围到达，注油就绪
 * 
 *           配方相关操作
 *  一、触摸屏发送一个配方编号值到指定的寄存器中，触发相应的回调函数
 *      向指定的寄存器区域写入编号对应的配方值，
 *  二、触摸屏读配方就是读取编号对应的配方寄存器区域
 *  三、触摸屏保存则是向对应的寄存器区域写值
 *  四、触摸屏发送使用，则是向使用对应的寄存器写1，触发相应的回调函数
 * 
 */
/***************************************************************************/
