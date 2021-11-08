#ifndef __GPIO_H
#define __GPIO_H	 
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"

#ifndef bool
    #define bool uint8_t
#endif

#ifndef TRUE
    #define TRUE (uint8_t)1
#endif

#ifndef FALSE
    #define FALSE (uint8_t)0
#endif


//通用gpio模式：INPUT,OUTPUT
typedef enum{
    INPUT = 0,
    OUTPUT = 1,
}gpio_mode;
#define IS_IO_MODE(MODE) (((MODE) == INPUT) || ((MODE) == OUTPUT))

//通用GPIO结构体
typedef struct gpio{ 
    gpio_mode mode;     //模式
    uint16_t pin;       //引脚
    GPIO_TypeDef *port;  //端口
    // uint8_t (*read)(struct gpio *gpio_to_read); //读
    // void (*write)(struct gpio *gpio_to_write, uint8_t write_val); //写
    // void (*toggle)(struct gpio *gpio_to_toggle);  //反转电平
}gpio;

//gpio输入中断回调函数
typedef void (*gpio_it_callback_t)(void);

//开时钟
void GPIOA_clk_enable(void);
void GPIOB_clk_enable(void);
void GPIOC_clk_enable(void);
void GPIOD_clk_enable(void);
void GPIO_clk_enable(void);
//可嵌套的开关中断
int32_t interrupt_disable( void );
void interrupt_enable(int32_t level);
//注册gpio
void gpio_register(gpio *reg_gpio, GPIO_TypeDef *port, uint16_t pin, 
                   gpio_mode mode, uint8_t is_out_pp, uint8_t pin_data);
//操作gpio
uint8_t read_gpio(gpio *gpio_to_read);
void write_gpio(gpio *gpio_to_write, uint8_t write_val);
void toggle_gpio(gpio *gpio_to_toggle);
//注册gpio的中断
uint8_t gpio_it_register(gpio *gpio_it_reg, EXTITrigger_TypeDef EXTI_trigger, gpio_it_callback_t callback, uint8_t is_key);
#endif
