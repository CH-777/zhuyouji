#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"
#include "stm32f10x.h"
#include "stm32f10x_adc.h"

#define Channels  3

/*       采用150欧姆的电阻
 *             液位传感器     压力传感器     温度传感器
 *  输出模拟量   4-20mA        4-20mA        4-20mA
 *  输出电压值   0.6V-3.0V    0.6V-3.0V      0.6-3.0V
 *  对应ADC值    745-3724     745-3724       745-3724
 *   测量范围            0-100bar(1bar = 0.1Mpa)   -50-150
 */
#define 	Liquid_Channel        ADC_Channel_1
#define  	Pressure_Channel      ADC_Channel_9
#define 	Temperatrue_Channel   ADC_Channel_8
//#define	    Unknow_Channel        ADC_Channel_13


//  3.3 / 4096 = 0.000805664
#define to_voltage(adc_val) (float)(0.000805664*(uint16_t)adc_val)


#define LIQUID_MAX_ADC_VAL  3724
#define LIQUID_MIN_ADC_VAL  745

#define LIQUID_UPPER_LIMIT  3426   //上限，最大液位的90%
#define LIQUID_LOWER_LIMIT  1341   //下限，最大液位的20%

#define START_REPLENISH_LEFT  1341  //补充油液的左边界
#define START_REPLENISH_RIGHT 1937  //补充油液的右边界

#define STOP_REPLENISH        3128 

//根据ADC值获取液位的百分比
#define to_liquid(adc_val)      (float)((adc_val - 745)/29.79)
//将ADC值转换为压力值，单位为Mpa
#define to_pressure(adc_val)    (float)(10.0*adc_val/2979.0 - 0.5) 
//将ADC值转换为温度值
#define to_temperatrue(adc_val) (float)(0.067137*adc_val - 100.01678)
/**********************************************/

#define get_liquid_adc_value()   get_adc_average(Liquid_Channel,10)
#define get_pressure_adc_value() get_adc_average(Pressure_Channel,10)
#define get_temperatrue_adc_value() get_adc_average(Temperatrue_Channel,10)
/************************************************/
void adc_init(void);
uint16_t get_adc(uint8_t ch);
uint16_t get_adc_average(uint8_t ch,uint8_t times);

#endif 


