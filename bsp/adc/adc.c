#include "adc.h"
//uint16_t ADC_Value[Channels];

//static void DMA_Configuration(void)
//{
//	DMA_InitTypeDef DMA_InitStructure;
//	DMA_DeInit(DMA1_Channel1);
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
//	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
//	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_Value;
//	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//	DMA_InitStructure.DMA_BufferSize = Channels;
//	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//16
//	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;    //连续模式
//	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
//	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

//}


//初始化ADC1,使用规则通道
void  adc_init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

	//PA1(ADC1) PB0(ADC8) PB1(ADC9) 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	    //模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  //模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = Channels;	 //顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	                   //根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   
	
  	//设置指定ADC的规则组通道4，一个序列，采样时间
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_0,  1, ADC_SampleTime_239Cycles5);	//ADC1,ADC通道,采样时间为239.5周期	 
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  2, ADC_SampleTime_239Cycles5);
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_239Cycles5);
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_239Cycles5);
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 5, ADC_SampleTime_239Cycles5);
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 6, ADC_SampleTime_239Cycles5);
	
//	ADC_DMACmd(ADC1, ENABLE);      //使能ADC的DMA功能。
	
	ADC_Cmd(ADC1, ENABLE);	       //使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	   //使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	   //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
	
//	DMA_Configuration();           //配置DMA的通道1
//	
//	DMA_Cmd(DMA1_Channel1, ENABLE);
//	
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);//使能指定的ADC1的软件转换启动功能
}	

//获得ADC值
//ch:通道值 0~3
uint16_t get_adc(uint8_t ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}
uint16_t get_adc_average(uint8_t ch,uint8_t times)
{
	uint32_t temp_val=0;
	uint8_t t;
	for(t = 0; t < times; t++)
	{
		temp_val += get_adc(ch);
	}
	return temp_val/times;
} 





/************************************ END of FILE  ************************************/
	 
	 
