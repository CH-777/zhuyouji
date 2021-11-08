#include "encoder.h"
#include "usart.h"
//#include "output_gpio.h"

// static void NVIC_Conf(void)
// {
// 	NVIC_InitTypeDef NVIC_InitStructure;

// 	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
// 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x09;  //先占优先级
// 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级
// 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
// 	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器	
	
// 	TIM_ClearITPendingBit(TIM3, TIM_IT_Update); 
// 	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); 
// }
// /**
//  * TIM2编码器模式初始化
//  * TIM3_CH1  : PA6
//  * TIM3_CH2  : PA7
//  */
// void TIM3_Encoder_Init(void)
// {  
	
// 	GPIO_InitTypeDef         GPIO_InitStructure;
// 	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;      //定义时基结构体
// 	TIM_ICInitTypeDef        TIM_ICInitStructure;        //定义输入比较结构体

// 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//打开定时器3的时钟 
//  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIO外设时钟使能
	                                                                     	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;         //TIM_CH1 : PA6
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //复用推挽输出
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; //TIM_CH2 : PA7
// 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
// 	TIM_TimeBaseStructure.TIM_Period = 65535; //装载值
// 	TIM_TimeBaseStructure.TIM_Prescaler = 0; //设置用来作为TIMx时钟频率除数的预分频值  不分频
// 	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
// 	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
// 	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

// 	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    
// 	TIM_ICStructInit(&TIM_ICInitStructure);
// 	TIM_ICInitStructure.TIM_ICFilter = 7;
// 	TIM_ICInit(TIM3,&TIM_ICInitStructure);
	
// 	NVIC_Conf();
// 	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
// 	TIM_SetCounter(TIM3, 0);
// 	TIM_Cmd(TIM3, DISABLE);
// }



/********************************************************************************/
//单PWM输入捕获,输入捕获通道为TIM2_CH1 : PA0

static uint16_t pulse_cnt = 0;

void TIM2_Cap_Init(u16 arr,u16 psc)
{	 
	
	TIM_ICInitTypeDef  TIM2_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//使能TIM2时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIOA时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;  //PA0 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 输入  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);						 //PA0 下拉
	
	//初始化定时器2 TIM2	 
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
  
	//初始化TIM2输入捕获参数
	TIM2_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM2_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  	TIM2_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM2_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM2_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM2, &TIM2_ICInitStructure);
	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
	TIM_ITConfig(TIM2,TIM_IT_CC1,DISABLE);//允许更新中断 ,允许CC1IE捕获中断	
	
  	TIM_Cmd(TIM2,ENABLE ); 	//使能定时器2
}

//tim2输入捕获中断
void TIM2_IRQHandler(void)
{ 
	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET) {//捕获1发生捕获事件
		pulse_cnt++;			
	}			     
	TIM_ClearITPendingBit(TIM2, TIM_IT_CC1); //清除中断标志位	    					    
}
//开启脉冲计数
void start_pulse_count(void)
{
	pulse_cnt = 0;
	TIM_Cmd(TIM2, ENABLE); 	//使能定时器2
	TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);//开中断
}


//获取计数的脉冲量
uint16_t get_pulse_count(void)
{
	return pulse_cnt;
}
//结束脉冲计数
void end_pulse_count(void)
{
	TIM_ITConfig(TIM2, TIM_IT_CC1, DISABLE);  //关中断
}

//void Tim_SetPulse(uint16_t pulse)
//{
//	TIM3->ARR = pulse;
//	TIM_SetCounter(TIM3, 0);
//	TIM_Cmd(TIM3, ENABLE);
//}











