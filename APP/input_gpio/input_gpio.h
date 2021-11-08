#ifndef __INPUT_GPIO_H
#define __INPUT_GPIO_H	 

#include "gpio.h"

#include "stm32f10x_exti.h"

/***********************************************************/
#ifndef BUTTON_SET
    #define BUTTON_SET     1      //按钮在高电平时有效
    #define BUTTON_RESET   0
#endif

#ifndef INPUT_SET
    #define INPUT_SET      1
    #define INPUT_RESET    0
#endif

#define AUTO     1   //自动处于高电平
#define MANUAL   0

/***********************************************************************/
typedef enum{
    start_button           = 0,
    stop_button            = 1,
    reset_button           = 2,
    urgent_stop_button     = 3,
    select_button          = 4,
    start_add_oil_button   = 5,
    presssure_diff_signal  = 6,
    temperature_overload   = 7,
    temperature_reached    = 8,
    temperature_overlimit  = 9,
    pulse_cnt_error_reach  = 10,
    address0               = 11,
    address1               = 12
}input_gpio_index;

#define get_start_button_level()           get_input_gpio_level(start_button)
#define get_stop_button_level()            get_input_gpio_level(stop_button)
#define get_reset_button_level()           get_input_gpio_level(reset_button)
#define get_urgent_stop_button_level()     get_input_gpio_level(urgent_stop_button)
#define get_select_button_level()          get_input_gpio_level(select_button)
#define get_start_add_oil_button_level()   get_input_gpio_level(start_add_oil_button)
#define get_presssure_diff_signal_level()  get_input_gpio_level(presssure_diff_signal)
#define get_temperature_overload_level()   get_input_gpio_level(temperature_overload)
#define get_temperature_reached_level()    get_input_gpio_level(temperature_reached)
#define get_temperature_overlimit_level()  get_input_gpio_level(temperature_overlimit)
#define get_pulse_cnt_error_level()        get_input_gpio_level(pulse_cnt_error_reach)

#define get_address0()        get_input_gpio_level(address0)
#define get_address1()        get_input_gpio_level(address1)
/***********************************************************************/
void input_gpios_register(void);    //初始化输入gpio
uint8_t get_input_gpio_level(input_gpio_index index);
void input_gpios_it_register(void);

#endif
