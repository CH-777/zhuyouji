#include "output_gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"

/**********************************************************************/
/**********************        输出GPIO引脚定义         ****************/
//    绿灯    ------->  Y0  ------->   PB10
#define  GREEN_LED_PORT         GPIOB
#define  GREEN_LED_PIN          GPIO_Pin_10
//    红灯    ------->  Y1  ------->   PB11 
#define  RED_LED_PORT           GPIOB
#define  RED_LED_PIN            GPIO_Pin_11
//    黄灯    ------->  Y2  ------->   PB12 
#define  YELLOW_LED_PORT        GPIOB
#define  YELLOW_LED_PIN         GPIO_Pin_12
//   蜂鸣器   ------->  Y3  ------->   PB13 
#define  BUZZER_PORT            GPIOB
#define  BUZZER_PIN             GPIO_Pin_13
// 注油完成灯  ------->  Y4  ------>   PB14 
#define  FINISH_OIL_LED_PORT    GPIOB
#define  FINISH_OIL_LED_PIN     GPIO_Pin_14
//  气控球阀1  ------->  K1  ------>   PB15 
#define  AIR_VALVE_1_PORT       GPIOB
#define  AIR_VALVE_1_PIN        GPIO_Pin_15
//  气控球阀2  ------->  K2  ------>   PC6
#define  AIR_VALVE_2_PORT       GPIOC
#define  AIR_VALVE_2_PIN        GPIO_Pin_6
//  气控球阀3  ------->  K3  ------>   PC7
#define  AIR_VALVE_3_PORT       GPIOC
#define  AIR_VALVE_3_PIN        GPIO_Pin_7
//  气控球阀4  ------->  K4  ------>   PC8
#define  AIR_VALVE_4_PORT       GPIOC
#define  AIR_VALVE_4_PIN        GPIO_Pin_8
//  气控球阀5  ------->  K5  ------>   PC9
#define  AIR_VALVE_5_PORT       GPIOC
#define  AIR_VALVE_5_PIN        GPIO_Pin_9
// 注油枪电磁阀  ----->  K6  ------>   PA8 
#define  OIL_GUN_VALVE_PORT     GPIOA
#define  OIL_GUN_VALVE_PIN      GPIO_Pin_8
// 电磁通断1  ------->  K7  ------->   PA12 
#define  ELECT_VALVE_1_PORT     GPIOA
#define  ELECT_VALVE_1_PIN      GPIO_Pin_12
// 电磁通断2  ------->  K8  ------->   PA15
#define  ELECT_VALVE_2_PORT     GPIOA
#define  ELECT_VALVE_2_PIN      GPIO_Pin_15
// 电磁通断3  ------->  K9  ------->   PC10 
#define  ELECT_VALVE_3_PORT     GPIOC
#define  ELECT_VALVE_3_PIN      GPIO_Pin_10
//温控仪加热启动 ---->  K10  ------>   PC11 
#define  HEAT_SWITCH_PORT       GPIOC
#define  HEAT_SWITCH_PIN        GPIO_Pin_11
// 温控仪预留  ------>  K11  ------>   PC12
#define  RESERVED_PORT          GPIOC
#define  RESERVED_PIN           GPIO_Pin_12
//   电机    ------->  KM1  ------>   PD2
#define  MOTOR_PORT             GPIOD
#define  MOTOR_PIN              GPIO_Pin_2

//    LED0  --------------------->    PB7
#define  TEST_LED0_PORT         GPIOB
#define  TEST_LED0_PIN          GPIO_Pin_7
//    LED1  -------------------->     PB6
#define  TEST_LED1_PORT         GPIOB
#define  TEST_LED1_PIN          GPIO_Pin_6
/************************************************************************/
/******************        输出gpio结构体全局变量       ******************/
static gpio output_gpios[19];
/***********************************************************************/
/*******************          注册相应的gpio          ******************/
void output_gpios_register(void)
{
	gpio_register(&output_gpios[test_led0], TEST_LED0_PORT, TEST_LED0_PIN, OUTPUT, TRUE, TEST_LED_OFF); //推挽输出
	gpio_register(&output_gpios[test_led1], TEST_LED1_PORT, TEST_LED1_PIN, OUTPUT, TRUE, TEST_LED_OFF); //推挽输出

	gpio_register(&output_gpios[green_led], GREEN_LED_PORT, GREEN_LED_PIN, OUTPUT, TRUE, LED_OFF); //推挽输出
	gpio_register(&output_gpios[red_led], RED_LED_PORT, RED_LED_PIN, OUTPUT, TRUE, LED_OFF); //推挽输出
	gpio_register(&output_gpios[yellow_led], YELLOW_LED_PORT, YELLOW_LED_PIN, OUTPUT, TRUE, LED_OFF); //推挽输出
	gpio_register(&output_gpios[finish_oil_led], FINISH_OIL_LED_PORT, FINISH_OIL_LED_PIN, OUTPUT, TRUE, LED_OFF);//推挽输出

	gpio_register(&output_gpios[buzzer], BUZZER_PORT, BUZZER_PIN, OUTPUT, TRUE, BUZZER_OFF);  //推挽输出
	
	gpio_register(&output_gpios[air_valve_1], AIR_VALVE_1_PORT, AIR_VALVE_1_PIN, OUTPUT, TRUE, AIR_VALVE_OFF); 
	gpio_register(&output_gpios[air_valve_2], AIR_VALVE_2_PORT, AIR_VALVE_2_PIN, OUTPUT, TRUE, AIR_VALVE_OFF);
	gpio_register(&output_gpios[air_valve_3], AIR_VALVE_3_PORT, AIR_VALVE_3_PIN, OUTPUT, TRUE, AIR_VALVE_OFF);
	gpio_register(&output_gpios[air_valve_4], AIR_VALVE_4_PORT, AIR_VALVE_4_PIN, OUTPUT, TRUE, AIR_VALVE_OFF);
	gpio_register(&output_gpios[air_valve_5], AIR_VALVE_5_PORT, AIR_VALVE_5_PIN, OUTPUT, TRUE, AIR_VALVE_OFF);

	gpio_register(&output_gpios[oil_gun_valve], OIL_GUN_VALVE_PORT, OIL_GUN_VALVE_PIN, OUTPUT, TRUE, ELECT_VALVE_OFF);
	
	gpio_register(&output_gpios[elect_valve_1], ELECT_VALVE_1_PORT, ELECT_VALVE_1_PIN, OUTPUT, TRUE, ELECT_VALVE_OFF);

	/*使能jtag所用的IO口，使PB3、PB4、PA15成为普通IO，使用SWD调试*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//使能端口复用时钟
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//失能JTAG
	gpio_register(&output_gpios[elect_valve_2], ELECT_VALVE_2_PORT, ELECT_VALVE_2_PIN, OUTPUT, TRUE, ELECT_VALVE_OFF);
	
	gpio_register(&output_gpios[elect_valve_3], ELECT_VALVE_3_PORT, ELECT_VALVE_3_PIN, OUTPUT, TRUE, ELECT_VALVE_OFF);
	gpio_register(&output_gpios[heat_switch], HEAT_SWITCH_PORT, HEAT_SWITCH_PIN  , OUTPUT, TRUE, HEATING_OFF);
	gpio_register(&output_gpios[reserved], RESERVED_PORT, RESERVED_PIN, OUTPUT, TRUE, HEATING_OFF);
	gpio_register(&output_gpios[motor], MOTOR_PORT, MOTOR_PIN, OUTPUT, TRUE, MOTOR_OFF);
}

/******************          操作相应的输出gpio         *******************/
void set_output_gpio_level(output_gpio_index index, uint8_t level)
{
	write_gpio(&output_gpios[index], level);
}
uint8_t get_output_gpio_level(output_gpio_index index)
{	
	return read_gpio(&output_gpios[index]);
}
//写多个io
// index: 起始索引， len：写io的数量，
void set_output_gpios_level(uint8_t index, uint8_t len, uint8_t *wbuf)
{	
	uint8_t i;
    taskENTER_CRITICAL();
	for(i = 0; i < len; ++i) {
		if((wbuf[i/8] & (1 << i%8)) != 0)
        	GPIO_SetBits(output_gpios[index + i].port, output_gpios[index + i].pin);
    	else
        	GPIO_ResetBits(output_gpios[index + i].port, output_gpios[index + i].pin);
	}
    taskEXIT_CRITICAL();
}
//读多个io
// index: 起始索引， len：写io的数量，
void get_output_gpios_level(uint8_t index, uint8_t len, uint8_t *rbuf)
{	
	uint8_t i;
	uint8_t read_val;
	

    taskENTER_CRITICAL();
	for(i = 0; i < len; ++i) {
        read_val = GPIO_ReadOutputDataBit(output_gpios[i + index].port, output_gpios[i + index].pin);
		if(read_val == 0)
			rbuf[i/8] &= ~(1 << (i%8));
		else
			rbuf[i/8] |= (1 << (i%8));
	}
    taskEXIT_CRITICAL();
}

