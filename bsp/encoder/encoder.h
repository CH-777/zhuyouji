#ifndef __ENCODER_H
#define __ENCODER_H
#include "sys.h"

// void TIM3_Encoder_Init(void);      //编码器模式
//uint16_t get_encoder_count(void);
//void end_pulse_count(void);
//void Tim_SetPulse(uint16_t pulse);


void TIM2_Cap_Init(u16 arr,u16 psc);  //单脉冲输入捕获
void start_pulse_count(void);
uint16_t get_pulse_count(void);
void end_pulse_count(void);
#endif


