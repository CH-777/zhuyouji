#include "gpio.h"

#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "elog.h"

#include <stddef.h>
#include <stdlib.h>
/***************************************************************/
#define Q_MAXSIZE   10
//循环队列定义
typedef struct{
    uint8_t data[Q_MAXSIZE];  /*循环队列的容量*/
    uint8_t front;    /*头指针*/
    uint8_t rear;     /*尾指针*/
}queue;
//返回循环队列的元素个数
// static uint8_t queueLength(queue q)
// {
//     uint8_t ret;
//     taskENTER_CRITICAL();
//     ret = (q.rear - q.front + Q_MAXSIZE) % Q_MAXSIZE;
//     taskEXIT_CRITICAL();
//     return ret;
// }
//判断循环队列是否为空
// static uint8_t isEmpty(queue q)
// {
//     uint8_t ret;
//     taskENTER_CRITICAL();
//     ret = (q.front == q.rear);
//     taskEXIT_CRITICAL();
//     return ret;
// }
//判断循环队列是否已满
// static uint8_t isFull(queue q)
// {
//     uint8_t ret;
//     taskENTER_CRITICAL();
//     ret = (q.rear + 1)%Q_MAXSIZE == q.front;
//     taskEXIT_CRITICAL();
//     return ret;  //队列满时，保留一个元素空间
// }
//入队，若队列未满，则返回0，否则返回1
// static uint8_t EnQueue(queue *q, uint8_t e)
// {
//     taskENTER_CRITICAL();
//     if((q->rear + 1) % Q_MAXSIZE == q->front) { //队列已满
//         taskEXIT_CRITICAL();
//         return 1;
//     }
//     q->data[q->rear] = e;
//     q->rear = (q->rear + 1) % Q_MAXSIZE;
//     taskEXIT_CRITICAL();
//     return 0;
// }
//出队，若队列不为空，则返回0，否则返回1,de为队列首元素
static uint8_t Dequeue(queue *q, uint8_t *de)
{
    taskENTER_CRITICAL();
    if(q->front == q->rear) {   //队列为空
        taskEXIT_CRITICAL();
        return 1;
    }
    *de = q->data[q->front];
    q->front = (q->front + 1) % Q_MAXSIZE;
    taskEXIT_CRITICAL();
    return 0;
}
/***************************************************************/
/******************         全局变量         *******************/
//gpio输入中断回调函数数组，gpio的引脚对应数组的下标
static gpio_it_callback_t gpio_it_callback[16];
static bool key_time_inited = FALSE;      //标记按键消抖时钟是否已被初始化        
static uint16_t is_key_flag = 0;         //判断位对应的中断线是否为按键
#define NO_EXTI_LINIE_TRIGGERED  0xFF
static volatile uint8_t which_EXTI_line_triggered = NO_EXTI_LINIE_TRIGGERED; //标记触发中断的中断线
//输入事件中断处理任务信号量
static SemaphoreHandle_t gpio_it_task_sem = NULL;
//输入事件中断处理任务头
static TaskHandle_t gpio_it_task_Handle = NULL;
//输入事件中断处理函数
static void gpio_it_handler_task(void *parameter);
//待处理中断回调函数循环队列，队列元素为回调函数数组下标
//在中断中入队，在输入事件中断处理任务中出队并执行相关的回调函数
static queue pinding_cb_queue = {.front = 0,.rear = 0};
/***************************************************************/
//打开GPIOA的时钟
void GPIOA_clk_enable(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
}
void GPIOB_clk_enable(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
}
void GPIOC_clk_enable(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
}

void GPIOD_clk_enable(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
}
//打开所有GPIO的时钟
void GPIO_clk_enable(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|
                           RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);
}


/*
 * 读取gpio的值
 * gpio_to_read：需要读取的gpio结构体
 * 返回值    读取到的值
 */
uint8_t read_gpio(gpio *gpio_to_read)
{
    assert_param((gpio_to_read != NULL));
    uint8_t read_val;

    taskENTER_CRITICAL();	
    if(gpio_to_read->mode == INPUT)
        read_val = GPIO_ReadInputDataBit(gpio_to_read->port, gpio_to_read->pin);
    else
        read_val = GPIO_ReadOutputDataBit(gpio_to_read->port, gpio_to_read->pin);
    taskEXIT_CRITICAL();
    return read_val;
}
/*
 * 向gpio写值
 * gpio_to_write：需要写值的gpio结构体
 * *write_val:    写入的值
 */
void write_gpio(gpio *gpio_to_write, uint8_t write_val)
{
    assert_param(gpio_to_write);
    assert_param(gpio_to_write->mode == OUTPUT);

    taskENTER_CRITICAL(); //需要使用FreeRTOS调用vTaskStartScheduler()后使用
    if(write_val == 1)
        GPIO_SetBits(gpio_to_write->port, gpio_to_write->pin);
    else
        GPIO_ResetBits(gpio_to_write->port, gpio_to_write->pin);
    taskEXIT_CRITICAL();
}
/*
 * 翻转输入gpio的值
 * gpio_to_toggle：需要翻转的gpio结构体
 * 返回值：   错误码，0表示正常读取
 */
void toggle_gpio(gpio *gpio_to_toggle)
{
    assert_param(gpio_to_toggle);
    assert_param(gpio_to_toggle->mode == OUTPUT);
    if(read_gpio(gpio_to_toggle) == 1)
        write_gpio(gpio_to_toggle, 0);
    else 
        write_gpio(gpio_to_toggle, 1);
}

/*
 * 注册一个通用gpio,调用该函数前需要打开gpio的时钟
 * reg_gpio 需要进行注册的gpio
 * port     端口
 * pin      引脚
 * mode     模式，可以配置为输入或者输出模式
 * is_out_pp 是否为推挽输出
 * pin_data gpio的预设值。 0：低电平  1：高电平  >1  :浮空 
 * 返回值   
 */
void gpio_register(gpio *reg_gpio, GPIO_TypeDef *port, uint16_t pin, 
                   gpio_mode mode, uint8_t is_out_pp, uint8_t pin_data)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    assert_param(IS_IO_MODE(mode));
    assert_param(reg_gpio);

    reg_gpio->mode = mode;
    reg_gpio->pin = pin;
    reg_gpio->port = port;

    // reg_gpio->read = gpio_read;
    // reg_gpio->write = gpio_write;
    // reg_gpio->toggle = gpio_toggle;

    GPIO_InitStructure.GPIO_Pin = pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    if(mode == INPUT) {  //输入模式
            switch(pin_data) {
            case 0 :
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  //下拉输入
                break;
            case 1 : 
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //上拉输入
                break;
            default:
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
         }
        GPIO_Init(port, &GPIO_InitStructure);	  //初始化GPIO
    }else{            //输出模式
        if(is_out_pp != 0)
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽输出
        else
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;    //开漏输出

        GPIO_Init(port, &GPIO_InitStructure);	  //初始化GPIO
        if(pin_data == 0)
            GPIO_ResetBits(port, pin);
        else 
            GPIO_SetBits(port, pin);
    }
}

/*
 * 利用定时器6进行按键消抖
 * 功能：按键消抖定时器初始化
 * delay_time_ms:  消抖时间，单位为ms
 */
static void key_time_init(uint8_t delay_time_ms)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE); //时钟使能,APB1:36MHz

	TIM_TimeBaseStructure.TIM_Period = delay_time_ms - 1; 
	TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t) (SystemCoreClock / 2000) - 1;//32000 - 1;//36000 - 1;  //1kHz  (使用的是内部时钟，频率为64MHz) 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

    //开启定时器6更新中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;  //TIM6中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;  //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  
	
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update); 
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);     
	TIM_Cmd(TIM6, DISABLE); 	    //关闭时钟，等待外部中断触发时打开时钟
}
 
/*
 * 注册gpio的输入中断
 * gpio_it_reg : 需要注册的gpio
 * EXTI_trigger: 外部中断触发方式
 * callback    : 中断回调函数
 * is_key      : 是否为按键，若TURE则需要进行消抖处理
 * 返回值      : 0正常注册，1该中断线已被注册
 */
uint8_t gpio_it_register(gpio *gpio_it_reg, EXTITrigger_TypeDef EXTI_trigger, gpio_it_callback_t callback, uint8_t is_key)
{
    assert_param(gpio_it_reg);
    assert_param(callback);
    assert_param(gpio_it_reg->mode == INPUT);

    uint8_t pin_offset;
    uint8_t port_offset;
    IRQn_Type IRQ_Chanl;     //外部中断通道
    switch(gpio_it_reg->pin) {
        case GPIO_Pin_0:  
			pin_offset = 0;  break;
        case GPIO_Pin_1:  
			pin_offset = 1;  break;
        case GPIO_Pin_2:  
			pin_offset = 2;  break;
        case GPIO_Pin_3:  
			pin_offset = 3;  break;
        case GPIO_Pin_4:  
			pin_offset = 4;  break;
        case GPIO_Pin_5:  
			pin_offset = 5;  break;
        case GPIO_Pin_6:  
			pin_offset = 6;  break;
        case GPIO_Pin_7:  
			pin_offset = 7;  break;
        case GPIO_Pin_8:  
			pin_offset = 8;  break;
        case GPIO_Pin_9:  
			pin_offset = 9;  break;
        case GPIO_Pin_10: 
			pin_offset = 10; break;
        case GPIO_Pin_11: 
			pin_offset = 11; break;
        case GPIO_Pin_12: 
			pin_offset = 12; break;
        case GPIO_Pin_13: 
			pin_offset = 13; break;
        case GPIO_Pin_14: 
			pin_offset = 14; break;
        case GPIO_Pin_15: 
			pin_offset = 15; break;
		default:
			break;
    }

    port_offset = (gpio_it_reg->port - GPIOA) / (GPIOB - GPIOA);
    
    if(5 <= pin_offset && pin_offset <= 9)
        IRQ_Chanl = EXTI9_5_IRQn;
    else if(10 <= pin_offset && pin_offset <= 15)
        IRQ_Chanl = EXTI15_10_IRQn;
    else {
        switch(pin_offset) {
            case 0: IRQ_Chanl = EXTI0_IRQn; break;
            case 1: IRQ_Chanl = EXTI1_IRQn; break;
            case 2: IRQ_Chanl = EXTI2_IRQn; break;
            case 3: IRQ_Chanl = EXTI3_IRQn; break;
            case 4: IRQ_Chanl = EXTI4_IRQn; break;
        }
    }

    if(gpio_it_callback[pin_offset] != NULL) {
         printf("callback %d have reg\n", pin_offset);
         return 1;
     }
    gpio_it_callback[pin_offset] = callback;
    if(is_key == TRUE) {
        is_key_flag |= (1 << pin_offset);
        //初始化消抖定时器
        if(key_time_inited == FALSE) {
            key_time_init(10);
            key_time_inited = TRUE; 
        }
    }
	if(gpio_it_task_Handle == NULL)
		xTaskCreate((TaskFunction_t )gpio_it_handler_task, /* 任务入口函数 */
					(const char*    )"task_gpio_it",/* 任务名字 */
					(uint16_t       )256,   /* 任务栈大小 */
					(void*          )NULL,	/* 任务入口函数参数 */
					(UBaseType_t    )configMAX_PRIORITIES - 1,	    /* 任务的优先级：设置为最高优先级 */
					(TaskHandle_t*  )&gpio_it_task_Handle);/* 任务控制块指针 */ 
    
    EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//外部中断，需要使能AFIO时钟	
    //中断线以及中断初始化配置
    GPIO_EXTILineConfig(port_offset, pin_offset);
    EXTI_InitStructure.EXTI_Line = (uint32_t)(1 << pin_offset);//外部中断线，只有IO中断，因此中断线为0-15
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_trigger;       //触发方式
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;             
  	EXTI_Init(&EXTI_InitStructure);	
    //外部中断通断配置
    NVIC_InitStructure.NVIC_IRQChannel = IRQ_Chanl;			//使能外部中断线所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;	//抢占优先级2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;	//子优先级1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  	

    EXTI_ClearITPendingBit((uint32_t)(1 << pin_offset));
    return 0;
}
/*************************************************************************/
/*
 * 中断处理函数
 * 根据gpio输入中断线是否连接key进行分别处理
 * 连接key则需要进行定时器消抖处理，最终回调函数在定时器中断中执行
 * 否则，直接在外部中断处理函数中执行
 * EXTI_Line  :触发外部中断的中断线
 */
static void gpio_it_handler(uint8_t EXTI_Line)
{
    BaseType_t pxHigherPriorityTaskWoken;
    if(gpio_it_callback[EXTI_Line] == NULL)
        return;   

    if(((1 << EXTI_Line) & is_key_flag) != 0)  { //该中断线连接key
        which_EXTI_line_triggered = EXTI_Line;  //标记哪一个中断线被触发，以便在定时器中断中执行对应的回调函数	
        TIM_SetCounter(TIM6, 0);       //重置定时器
		TIM_Cmd(TIM6, ENABLE);         //打开时钟
    }else {
        if(pinding_cb_queue.front == pinding_cb_queue.rear) { //循环队列为空，则需要释放信号量
            xSemaphoreGiveFromISR(gpio_it_task_sem, &pxHigherPriorityTaskWoken);//释放二值信号量
            //如果需要的话进行一次任务切换，系统会判断是否需要进行切换
            portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
        }
        //入队
        pinding_cb_queue.data[pinding_cb_queue.rear] = EXTI_Line;
        pinding_cb_queue.rear = (pinding_cb_queue.rear + 1) % Q_MAXSIZE;
    }

}
/*********************      外部中断线中断处理函数      *******************/
void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET){
        gpio_it_handler(0);
        EXTI_ClearITPendingBit(EXTI_Line0); 
    }
}
void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line1) != RESET){
        gpio_it_handler(1);
        EXTI_ClearITPendingBit(EXTI_Line1); 
    }
}
void EXTI2_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line2) != RESET){
        gpio_it_handler(2);
        EXTI_ClearITPendingBit(EXTI_Line2); 
    }
}
void EXTI3_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line3) != RESET){
        gpio_it_handler(3);
        EXTI_ClearITPendingBit(EXTI_Line3); 
    }
}
void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4) != RESET){
        gpio_it_handler(4);
        EXTI_ClearITPendingBit(EXTI_Line4); 
    }
}
void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line5) != RESET){
        gpio_it_handler(5);
        EXTI_ClearITPendingBit(EXTI_Line5); 
    }
    if(EXTI_GetITStatus(EXTI_Line6) != RESET){
        gpio_it_handler(6);
        EXTI_ClearITPendingBit(EXTI_Line6); 
    }
    if(EXTI_GetITStatus(EXTI_Line7) != RESET){
        gpio_it_handler(7);
        EXTI_ClearITPendingBit(EXTI_Line7); 
    }
    if(EXTI_GetITStatus(EXTI_Line8) != RESET){
        gpio_it_handler(8);
        EXTI_ClearITPendingBit(EXTI_Line8); 
    }
    if(EXTI_GetITStatus(EXTI_Line9) != RESET){
        gpio_it_handler(9);
        EXTI_ClearITPendingBit(EXTI_Line9); 
    }
}
void EXTI15_10_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line10) != RESET){
        gpio_it_handler(10);
        EXTI_ClearITPendingBit(EXTI_Line10); 
    }
    if(EXTI_GetITStatus(EXTI_Line11) != RESET){
        gpio_it_handler(11);
        EXTI_ClearITPendingBit(EXTI_Line11); 
    }
    if(EXTI_GetITStatus(EXTI_Line12) != RESET){
        gpio_it_handler(12);
        EXTI_ClearITPendingBit(EXTI_Line12); 
    }
    if(EXTI_GetITStatus(EXTI_Line13) != RESET){
        gpio_it_handler(13);
        EXTI_ClearITPendingBit(EXTI_Line13); 
    }
    if(EXTI_GetITStatus(EXTI_Line14) != RESET){
        gpio_it_handler(14);
        EXTI_ClearITPendingBit(EXTI_Line14); 
    }
    if(EXTI_GetITStatus(EXTI_Line15) != RESET){
        gpio_it_handler(15);
        EXTI_ClearITPendingBit(EXTI_Line15); 
    }
}

/*******************************************************/
/*
 * 定时器6溢出中断处理函数
 */
void TIM6_IRQHandler(void)   //TIM6溢出中断
{
    BaseType_t pxHigherPriorityTaskWoken;
	if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {	
		TIM_Cmd(TIM6, DISABLE);           //关闭时钟
		if(which_EXTI_line_triggered != NO_EXTI_LINIE_TRIGGERED) {
            if(pinding_cb_queue.front == pinding_cb_queue.rear) { //循环队列为空，则需要释放信号量
                xSemaphoreGiveFromISR(gpio_it_task_sem, &pxHigherPriorityTaskWoken);//释放二值信号量
                //如果需要的话进行一次任务切换，系统会判断是否需要进行切换
                portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
            }
            //入队
            pinding_cb_queue.data[pinding_cb_queue.rear] = which_EXTI_line_triggered;
            pinding_cb_queue.rear = (pinding_cb_queue.rear + 1) % Q_MAXSIZE;
            which_EXTI_line_triggered = NO_EXTI_LINIE_TRIGGERED;
        }
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	}
}

/**********************************************************/
//输入gpio中断处理任务
static void gpio_it_handler_task(void *parameter)
{
    BaseType_t xReturn = pdPASS;
    gpio_it_task_sem = xSemaphoreCreateBinary();
    if(gpio_it_task_sem == NULL) {
        log_a("gpio_it_task_sem init error!");
        return;
    }else
        log_i("gpio_it_task_sem init");
    while(1) {
    //获取二值信号量 xSemaphore,没获取到则一直等待
		xReturn = xSemaphoreTake(gpio_it_task_sem,/* 二值信号量句柄 */
                              portMAX_DELAY); /* 等待时间 */
        if(xReturn == pdTRUE) {
            //循环取出队列中的元素，执行相应的回调函数
            uint8_t cb_index;
            while(Dequeue(&pinding_cb_queue, &cb_index) == 0) { //循环队列不为空
                gpio_it_callback[cb_index]();
            }
        }
    }
}




/*********************************************************/


