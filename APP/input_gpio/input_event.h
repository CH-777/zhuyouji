#ifndef __INPUT_EVENT_H
#define __INPUT_EVENT_H	 

#include "FreeRTOS.h"
#include "task.h"

/*IO输入事件*/
// typedef enum{
//     EV_START                   = 1<<0, //启动按钮按下 
//     EV_STOP                    = 1<<1, //停止按钮按下
// 	EV_RESET                   = 1<<2, //复位按钮按下
//     EV_URGENT_STOP             = 1<<3, //急停按钮按下
//     EV_MODE_SELECT             = 1<<4, //选择按钮按下
//     EV_START_ADD_OIL           = 1<<5, //注油启动按钮按下
//     EV_PRESSURE_DIFF_SIGNAL    = 1<<6, //压差发讯器输入事件
//     EV_TEMPRATURE_OVERLOAD     = 1<<7, //热过载继电器输入事件
//     EV_TEMPRATURE_REACHED      = 1<<8, //PID温控仪温度达到事件
//     EV_TEMPRATURE_OVERLIMIT    = 1<<9, //PID温控仪温度超限报警事件
// }eGPIOInputEventType;

// uint8_t xGPIOInputEventInit(void);
// uint8_t xGPIOInputEventEventPost(eGPIOInputEventType eEvent);
// void eGPIOInputEventPoll(void);


#endif
