#include "input_gpio.h"
#include "usart.h"
#include "oil.h"

/****************************************************************/
/******************         输入gpio引脚定义        *************/
//         启动按钮            ------->  X0  ------->   PC13
#define  START_BUTTON_PORT               GPIOC
#define  START_BUTTON_PIN                GPIO_Pin_13
//         停止按钮           ------->  X1  ------->   PC14
#define  STOP_BUTTON_PORT                GPIOC
#define  STOP_BUTTON_PIN                 GPIO_Pin_14
//         复位按钮           ------->  X2  ------->   PC15
#define  RESET_BUTTON_PORT               GPIOC
#define  RESET_BUTTON_PIN                GPIO_Pin_15
//         急停按钮           ------->  X3  ------->   PC0  
#define  URGENT_STOP_BUTTON_PORT         GPIOC
#define  URGENT_STOP_BUTTON_PIN          GPIO_Pin_0
//         选择按钮           ------->  X4  ------->   PC2  
#define  SELECT_BUTTON_PORT              GPIOC
#define  SELECT_BUTTON_PIN               GPIO_Pin_2
//        注油启动按钮        ------->  X5  ------->   PC1  
#define  START_ADD_OIL_BUTTON_PORT       GPIOC
#define  START_ADD_OIL_BUTTON_PIN        GPIO_Pin_1
//        压差发讯器         ------->  X6  ------->   PC3
#define  PRESSURE_DIFF_SIGNAL_PORT       GPIOC
#define  PRESSURE_DIFF_SIGNAL_PIN        GPIO_Pin_3
//    热过载继电器           ------->  X7  ------->   PC5
#define  TEMPERATURE_OVERLOAD_PORT       GPIOC
#define  TEMPERATURE_OVERLOAD_PIN        GPIO_Pin_5
//    PID温控仪温度达到      ------->  X8  ------->   PB9
#define  TEMPERATURE_REACHED_PORT        GPIOB
#define  TEMPERATURE_REACHED_PIN         GPIO_Pin_9
//    PID温控仪温度超限报警  ------->  X9  ------->   PB8
#define  TEMPERATURE_OVERLIMIT_PORT      GPIOB
#define  TEMPERATURE_OVERLIMIT_PIN       GPIO_Pin_8
/*     通过ADD0和ADD1两个引脚来确定modbus从机的地址，地址可以为1,2，3  */
//          ADD0    ---------->    PB4
#define  ADD0_PORT            GPIOB
#define  ADD0_PIN             GPIO_Pin_4
//          ADD1    ---------->    PB3
#define  ADD1_PORT            GPIOB
#define  ADD1_PIN             GPIO_Pin_3
/*****************************************************************/
/*****************           输入gpio全局变量          ************/
static gpio input_gpios[13];

/*****************************************************************/
/*****************          注册相应的gpio          ***************/
void input_gpios_register(void)
{
	gpio_register(&input_gpios[start_button], START_BUTTON_PORT, START_BUTTON_PIN, INPUT, TRUE, BUTTON_RESET);
	gpio_register(&input_gpios[stop_button], STOP_BUTTON_PORT, STOP_BUTTON_PIN, INPUT, TRUE, BUTTON_RESET);
	gpio_register(&input_gpios[reset_button], RESET_BUTTON_PORT, RESET_BUTTON_PIN, INPUT, TRUE, BUTTON_RESET);
	gpio_register(&input_gpios[urgent_stop_button], URGENT_STOP_BUTTON_PORT, URGENT_STOP_BUTTON_PIN, INPUT, TRUE, BUTTON_RESET);
	gpio_register(&input_gpios[select_button], SELECT_BUTTON_PORT, SELECT_BUTTON_PIN, INPUT, TRUE, BUTTON_RESET);
	gpio_register(&input_gpios[start_add_oil_button], START_ADD_OIL_BUTTON_PORT, START_ADD_OIL_BUTTON_PIN, INPUT, TRUE, BUTTON_RESET);
	gpio_register(&input_gpios[presssure_diff_signal], PRESSURE_DIFF_SIGNAL_PORT, PRESSURE_DIFF_SIGNAL_PIN, INPUT, TRUE, INPUT_RESET);
	gpio_register(&input_gpios[temperature_overload], TEMPERATURE_OVERLOAD_PORT, TEMPERATURE_OVERLOAD_PIN, INPUT, TRUE, INPUT_RESET);
	gpio_register(&input_gpios[temperature_reached], TEMPERATURE_REACHED_PORT, TEMPERATURE_REACHED_PIN, INPUT, TRUE, INPUT_RESET);
	gpio_register(&input_gpios[temperature_overlimit], TEMPERATURE_OVERLIMIT_PORT, TEMPERATURE_OVERLIMIT_PIN, INPUT, TRUE, INPUT_RESET);
	/*使能jtag所用的IO口，使PB3、PB4、PA15成为普通IO，使用SWD调试*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//使能端口复用时钟
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//失能JTAG
	gpio_register(&input_gpios[address0], ADD0_PORT, ADD0_PIN, INPUT, TRUE, INPUT_RESET);
	gpio_register(&input_gpios[address1], ADD1_PORT, ADD1_PIN, INPUT, TRUE, INPUT_RESET);
}

uint8_t get_input_gpio_level(input_gpio_index index)
{	
	return read_gpio(&input_gpios[index]);
}


/*
 * 注册相应的输入中断
 */
void input_gpios_it_register(void)
{
	//按键输入中断，上升沿触发
	//启动按键中断注册
	if(gpio_it_register(&input_gpios[start_button], EXTI_Trigger_Rising, ev_start_cb, TRUE) != 0) {
		printf("start button it register error");
		return;
	}
	//停止按键中断注册
	if(gpio_it_register(&input_gpios[stop_button], EXTI_Trigger_Rising, ev_stop_cb, TRUE)!= 0) {
		printf("stop button it register error");
		return;
	}
	//复位按键中断注册
	if(gpio_it_register(&input_gpios[reset_button], EXTI_Trigger_Rising, ev_reset_cb, TRUE)!= 0) {
		printf("reset button it register error");
		return;
	}
	//急停按键中断注册
	if(gpio_it_register(&input_gpios[urgent_stop_button], EXTI_Trigger_Rising, ev_urgent_stop_cb, TRUE)!= 0) {
		printf("urgent stop button it register error");
		return;
	}
	//运行模式按键中断注册
	if(gpio_it_register(&input_gpios[select_button], EXTI_Trigger_Rising_Falling, ev_mode_select_cb, TRUE)!= 0) {
		printf("select button it register error");
		return;
	}
	//开始注油按键中断注册
	if(gpio_it_register(&input_gpios[start_add_oil_button], EXTI_Trigger_Rising, ev_start_add_oil_cb, TRUE)!= 0) {
		printf("start_add_oil button it register error");
		return;
	}
	//压差发讯器报警中断注册
	if(gpio_it_register(&input_gpios[presssure_diff_signal], EXTI_Trigger_Rising, ev_presssure_diff_signal_cb, FALSE)!= 0) {
		printf("presssure_diff_signal it register error");
		return;
	}
	//温度过载中断注册
	if(gpio_it_register(&input_gpios[temperature_overload], EXTI_Trigger_Rising, ev_temperature_overload_cb, FALSE)!= 0) {
		printf("temperature_overload it register error");
		return;
	}
	//温度到达中断注册
	if(gpio_it_register(&input_gpios[temperature_reached], EXTI_Trigger_Rising, ev_heating_temprature_reached, FALSE)!= 0) {
		printf("temperature_reached it register error");
		return;
	}
	//温度超限中断注册
	if(gpio_it_register(&input_gpios[temperature_overlimit], EXTI_Trigger_Rising, ev_temprature_overlimit_cb, FALSE)!= 0) {
		printf("temperature_overlimit it register error");
		return;
	}
	
}
/******************************************************************/








