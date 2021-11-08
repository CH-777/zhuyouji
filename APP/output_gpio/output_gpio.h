#ifndef __OUTPUT_GPIO_H
#define __OUTPUT_GPIO_H	 

#include "stm32f10x.h"
	

/*****************************************************************/
/************         定义灯、蜂鸣器、阀的等开关电平       *********/
//两测试灯低电平点亮
#ifndef TEST_LED_ON
    #define TEST_LED_ON       0
    #define TEST_LED_OFF      1
#endif

#ifndef LED_ON
    #define  LED_ON            1     //高电平有效
    #define  LED_OFF           0
#endif

#ifndef BUZZER_ON
    #define  BUZZER_ON         1
    #define  BUZZER_OFF        0
#endif

#ifndef AIR_VALVE_ON
    #define  AIR_VALVE_ON      1
    #define  AIR_VALVE_OFF     0
#endif

#ifndef ELECT_VALVE_ON
    #define  ELECT_VALVE_ON    1
    #define  ELECT_VALVE_OFF   0
#endif

#ifndef HEARTINT_ON
    #define  HEATING_ON        1
    #define  HEATING_OFF       0
#endif

#ifndef MOTOR_ON
    #define  MOTOR_ON          1
    #define  MOTOR_OFF         0
#endif
/*****************************************************************/
/*******************        gpio操作宏定义          **************/
typedef enum{
    green_led       = 0,
    red_led         = 1,
    yellow_led      = 2,
    buzzer          = 3,
    finish_oil_led  = 4,
    air_valve_1     = 5,
    air_valve_2     = 6,
    air_valve_3     = 7,
    air_valve_4     = 8,
    air_valve_5     = 9,
    oil_gun_valve   = 10,
    elect_valve_1   = 11,
    elect_valve_2   = 12,
    elect_valve_3   = 13,
    heat_switch     = 14,
    reserved        = 15,
    motor           = 16,
    test_led0       = 17,
    test_led1       = 18
}output_gpio_index;
/*******************************/
#define  open_test_led0()         set_output_gpio_level(test_led0, TEST_LED_ON)
#define  close_test_led0()        set_output_gpio_level(test_led0, TEST_LED_OFF)

#define  open_test_led1()         set_output_gpio_level(test_led1, TEST_LED_ON)
#define  close_test_led1()        set_output_gpio_level(test_led1, TEST_LED_OFF)

#define  open_green_led()         set_output_gpio_level(green_led, LED_ON)
#define  close_green_led()        set_output_gpio_level(green_led, LED_OFF)

#define  open_red_led()           set_output_gpio_level(red_led, LED_ON)
#define  close_red_led()          set_output_gpio_level(red_led, LED_OFF)

#define  open_yellow_led()        set_output_gpio_level(yellow_led, LED_ON)
#define  close_yellow_led()       set_output_gpio_level(yellow_led, LED_OFF)

#define  open_finish_oil_led()    set_output_gpio_level(finish_oil_led, LED_ON)
#define  close_finish_oil_led()   set_output_gpio_level(finish_oil_led, LED_OFF)

#define  open_buzzer()            set_output_gpio_level(buzzer, BUZZER_ON)
#define  close_buzzer()           set_output_gpio_level(buzzer, BUZZER_OFF)

#define  open_air_valve_1()       set_output_gpio_level(air_valve_1, AIR_VALVE_ON)
#define  close_air_valve_1()      set_output_gpio_level(air_valve_1, AIR_VALVE_OFF)

#define  open_air_valve_2()       set_output_gpio_level(air_valve_2, AIR_VALVE_ON)
#define  close_air_valve_2()      set_output_gpio_level(air_valve_2, AIR_VALVE_OFF)

#define  open_air_valve_3()       set_output_gpio_level(air_valve_3, AIR_VALVE_ON)
#define  close_air_valve_3()      set_output_gpio_level(air_valve_3, AIR_VALVE_OFF)

#define  open_air_valve_4()       set_output_gpio_level(air_valve_4, AIR_VALVE_ON)
#define  close_air_valve_4()      set_output_gpio_level(air_valve_4, AIR_VALVE_OFF)

#define  open_air_valve_5()       set_output_gpio_level(air_valve_5, AIR_VALVE_ON)
#define  close_air_valve_5()      set_output_gpio_level(air_valve_5, AIR_VALVE_OFF)

#define  open_oil_gun_valve()     set_output_gpio_level(oil_gun_valve, ELECT_VALVE_ON)
#define  close_oil_gun_valve()    set_output_gpio_level(oil_gun_valve, ELECT_VALVE_OFF)

#define  open_elect_valve_1()     set_output_gpio_level(elect_valve_1, ELECT_VALVE_ON)
#define  close_elect_valve_1()    set_output_gpio_level(elect_valve_1, ELECT_VALVE_OFF)

#define  open_elect_valve_2()     set_output_gpio_level(elect_valve_2, ELECT_VALVE_ON)
#define  close_elect_valve_2()    set_output_gpio_level(elect_valve_2, ELECT_VALVE_OFF)

#define  open_elect_valve_3()     set_output_gpio_level(elect_valve_3, ELECT_VALVE_ON)
#define  close_elect_valve_3()    set_output_gpio_level(elect_valve_3, ELECT_VALVE_OFF)

#define  open_heat_switch()       set_output_gpio_level(heat_switch, HEATING_ON)
#define  close_heat_switch()      set_output_gpio_level(heat_switch, HEATING_OFF)

#define  open_reserved()          set_output_gpio_level(reserved, HEATING_ON)
#define  close_reserved()         set_output_gpio_level(reserved, HEATING_OFF)

#define  open_motor()             set_output_gpio_level(motor, MOTOR_ON)
#define  close_motor()            set_output_gpio_level(motor, MOTOR_OFF)

/***********************************************************************/
#define  get_green_led_level()        get_output_gpio_level(green_led)
#define  get_red_led_level()          get_output_gpio_level(red_led)
#define  get_yellow_led_level()       get_output_gpio_level(yellow_led)
#define  get_finish_oil_led_level()   get_output_gpio_level(finish_oil_led)
#define  get_buzzer_level()           get_output_gpio_level(buzzer)
#define  get_air_valve_1_level()      get_output_gpio_level(air_valve_1)
#define  get_air_valve_2_level()      get_output_gpio_level(air_valve_2)
#define  get_air_valve_3_level()      get_output_gpio_level(air_valve_3)
#define  get_air_valve_4_level()      get_output_gpio_level(air_valve_4)
#define  get_air_valve_5_level()      get_output_gpio_level(air_valve_5)
#define  get_oil_gun_valve_level()    get_output_gpio_level(oil_gun_valve)
#define  get_elect_valve_1_level()    get_output_gpio_level(elect_valve_1)
#define  get_elect_valve_2_level()    get_output_gpio_level(elect_valve_2)
#define  get_elect_valve_3_level()    get_output_gpio_level(elect_valve_3)
#define  get_heat_switch_level()      get_output_gpio_level(heat_switch)
#define  get_reserved_level()         get_output_gpio_level(reserved)
#define  get_motor_level()            get_output_gpio_level(motor)

/**********************************************************************/
//输出gpio初始化函数
void output_gpios_register(void);

//相应的gpio操作函数
void set_output_gpio_level(output_gpio_index index, uint8_t level);
uint8_t get_output_gpio_level(output_gpio_index index);
void set_output_gpios_level(uint8_t index, uint8_t len, uint8_t *wbuf);
void get_output_gpios_level(uint8_t index, uint8_t len, uint8_t *rbuf);
#endif




