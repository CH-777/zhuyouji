#include "oil.h"
//bsp
#include "output_gpio.h"
#include "input_gpio.h"
#include "encoder.h"
#include "sd.h"
#include "adc.h"
//modbus
#include "reg_def.h"
#include "mbport.h"
//freertos
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
//easylogger
#include "elog.h"
/*****************************************************************/
/**********************      全局变量       **********************/
//注油机运行状态
static run_mode_type      run_mode;
//注油操作流程
static oil_add_process_type  oil_add_process;
//配方数组
static formula formulas[FORMULAS_NUMBER];
static uint8_t cur_formula_index;      //当前配方寄存器区域保存的配方编号
static formula* formula_selected;      /*选中的配方*/
//注油参数
oil_add_param add_param = {.oil_unit_pulse = 0.1};    
//注油中按下停止，延时处理标志位
uint8_t stop_action_to_handle = 0;
/*****************************************************************/
/******************       配方操作         ***********************/

/***************************************************************************/
/**************                 注油参数设定               ******************/
//根据选中的配方来设定注油参数
static uint8_t set_oil_add_param(void)
{	
	if(formula_selected == NULL)
		return 1;
	log_i("set add oil paramter");
	add_param.compute_oil_ml = formula_selected->oil_amount_set + formula_selected->compensation_amount_set;
	add_param.compute_oil_g = add_param.compute_oil_ml * formula_selected->density;
	add_param.compute_pulse = add_param.compute_oil_g / add_param.oil_unit_pulse;
	//将相应的配方参数写入到对应的保持寄存器中
	taskENTER_CRITICAL();
	//将浮点数存放进short数组里，结果与大小端有关
	usSRegHoldBuf[OIL_UNIT_PULSE] = *((uint16_t *)&add_param.oil_unit_pulse + 1); 
	usSRegHoldBuf[OIL_UNIT_PULSE + 1] = *((uint16_t*)&add_param.oil_unit_pulse);
	usSRegHoldBuf[COMPUTE_PULSE] = add_param.compute_pulse;
	usSRegHoldBuf[COMPUTE_OIL_G] = *((uint16_t *)&add_param.compute_oil_g + 1);   
	usSRegHoldBuf[COMPUTE_OIL_G + 1] = *((uint16_t*)&add_param.compute_oil_g);
	usSRegHoldBuf[COMPUTE_OIL_ML] = *((uint16_t *)&add_param.compute_oil_ml + 1); 
	usSRegHoldBuf[COMPUTE_OIL_ML + 1] = *((uint16_t*)&add_param.compute_oil_ml);
	taskEXIT_CRITICAL();
	return 0;
}

/***************************************************************************/
/*
 * 选择配方
 * 成功返回0
 */
static uint8_t select_formula(void)
{	
	taskENTER_CRITICAL();
	formula_selected = &formulas[cur_formula_index];
	set_oil_add_param();  //根据选定的配方来设置注油参数
	log_i("formula have seleted");
    taskEXIT_CRITICAL();
	return 0;
}
////获取选定配方的编号
//static uint8_t get_selected_formula_index(void)
//{
//	uint8_t index;
//	taskENTER_CRITICAL();
//	index = formula_selected->index;
//    taskEXIT_CRITICAL();
//	return index;
//}
//将选中的配方参数写入到对应的保持寄存器中,成功则返回0
static uint8_t wirte_formula_param_to_SRegHold(uint8_t index)
{
	// if(formula_selected == NULL)                //未选择任何配方
	// 	return 1;
	// if(formula_selected->magic != MAGIC_NUMBER) 
	// 	return 2;
	if(index >= FORMULAS_NUMBER)
		return 1;
	// if(formulas[index].magic != MAGIC_NUMBER) //该配方未保存任何数据
	// 	return 2;

	// taskENTER_CRITICAL();
	cur_formula_index = index;
	usSRegHoldBuf[OIL_QUALITY] = formulas[index].oil_quality;
	usSRegHoldBuf[PRESSURE] = *((uint16_t*)&formulas[index].pressure + 1);
	usSRegHoldBuf[PRESSURE + 1] = *((uint16_t*)&formulas[index].pressure);
	usSRegHoldBuf[TEMPTRUE] = *((uint16_t*)&formulas[index].temptrue + 1);
	usSRegHoldBuf[TEMPTRUE + 1] = *((uint16_t*)&formulas[index].temptrue );
	usSRegHoldBuf[DENSITY] = *((uint16_t *)&formulas[index].density + 1);
	usSRegHoldBuf[DENSITY + 1] = *((uint16_t *)&formulas[index].density );
	usSRegHoldBuf[OIL_AMOUNT] = *((uint16_t *)&formulas[index].oil_amount_set + 1);  
	usSRegHoldBuf[OIL_AMOUNT + 1] = *((uint16_t *)&formulas[index].oil_amount_set );
	usSRegHoldBuf[COMPENSATION_AMOUNT] = *((uint16_t *)&formulas[index].compensation_amount_set + 1); 
	usSRegHoldBuf[COMPENSATION_AMOUNT + 1] = *((uint16_t *)&formulas[index].compensation_amount_set ); 
	
    // taskEXIT_CRITICAL();	
	return 0;
}

//将保持寄存器中改变的配方参数值写入参数数组中,成功返回0
static uint8_t read_formula_param_from_SRegHold(void)
{
    // if(formula_selected == NULL)
	// 	return 1;
	uint16_t data[2];
	taskENTER_CRITICAL();
	formulas[cur_formula_index].magic = MAGIC_NUMBER;
	formulas[cur_formula_index].index = usSRegHoldBuf[FORMULA_INDEX];
	formulas[cur_formula_index].oil_quality = usSRegHoldBuf[OIL_QUALITY];
	data[0] = usSRegHoldBuf[PRESSURE + 1];
	data[1] = usSRegHoldBuf[PRESSURE ];
	formulas[cur_formula_index].pressure = *(float *)data;
	data[0] = usSRegHoldBuf[TEMPTRUE + 1];
	data[1] = usSRegHoldBuf[TEMPTRUE ];
	formulas[cur_formula_index].temptrue = *(float *)data;
	data[0] = usSRegHoldBuf[DENSITY + 1];
	data[1] = usSRegHoldBuf[DENSITY ];
	formulas[cur_formula_index].density = *(float *)data;
	data[0] = usSRegHoldBuf[OIL_AMOUNT + 1];                 
	data[1] = usSRegHoldBuf[OIL_AMOUNT ];
	formulas[cur_formula_index].oil_amount_set = *(float *)data;
	data[0] = usSRegHoldBuf[COMPENSATION_AMOUNT + 1];          
	data[1] = usSRegHoldBuf[COMPENSATION_AMOUNT ];
	formulas[cur_formula_index].compensation_amount_set = *(float *)data;
    taskEXIT_CRITICAL();
	return 0;
}

//从SD卡中读取配方
//返回值:0，成功
uint8_t read_all_formula_from_SD(void)
{
	return SD_ReadDisk((uint8_t *)formulas, FORMULAS_SECTOR, (FORMULAS_SIZE + 511)/512);
}
//将修改过的配方写入SD卡中
uint8_t write_all_formula_to_SD(void)
{
	return SD_WriteDisk((uint8_t *)formulas, FORMULAS_SECTOR, (FORMULAS_SIZE + 511)/512);
}
/*******************************************************************/
/**************             注油计时任务              **************/
//开始注油信号量
static SemaphoreHandle_t oil_add_time_cnt_sem = NULL;
//注油计时任务头
static TaskHandle_t oil_add_time_cnt_task_Handle = NULL;

static void oil_add_time_cnt_task(void *parameter)
{
	BaseType_t xReturn = pdPASS;
	float cur_seconds;
	uint16_t prv_pulse;  //上一时刻脉冲数，用于计算实时流量
	static portTickType preWakeTime;
	const portTickType timeIncrement = pdMS_TO_TICKS(100);

	while(1) {
		    //获取二值信号量 xSemaphore,没获取到则一直等待
		xReturn = xSemaphoreTake(oil_add_time_cnt_sem,/* 二值信号量句柄 */
                              portMAX_DELAY); /* 等待时间 */
        if(xReturn == pdTRUE) {
			log_i("start add oil timer");
			cur_seconds = 0.0;
			prv_pulse = 0;
			preWakeTime = xTaskGetTickCount();
			while(1) {
				vTaskDelayUntil(&preWakeTime, timeIncrement);  //绝对延时0.1s
				cur_seconds += 0.1;

				//计算实际脉冲
				add_param.actual_pulse = get_pulse_count();
				
				//实际注油量
				add_param.actual_oil_g = add_param.actual_pulse * add_param.oil_unit_pulse;
				add_param.actual_oil_ml = add_param.actual_oil_g / formula_selected->density;
				//注油平均流量ml/g
				add_param.average_flow_rate = add_param.actual_oil_ml / cur_seconds;
				//实时流量
				add_param.real_time_flow_rate = 10*(add_param.actual_pulse - prv_pulse)
					*add_param.oil_unit_pulse/formula_selected->density;
			
				prv_pulse = add_param.actual_pulse;
				//将相关参数写入到对应的寄存器中,可以通过事件完成
				taskENTER_CRITICAL();
				usSRegHoldBuf[ACTUAL_PULSE] = add_param.actual_pulse;              //实际脉冲量
				usSRegHoldBuf[ACTUAL_OIL_G] = *((uint16_t*)&add_param.actual_oil_g + 1);     //实际注油量(g)
				usSRegHoldBuf[ACTUAL_OIL_G + 1] = *((uint16_t*)&add_param.actual_oil_g );
				usSRegHoldBuf[ACTUAL_OIL_ML] = *((uint16_t*)&add_param.actual_oil_ml + 1);      //实际注油量(ml)
				usSRegHoldBuf[ACTUAL_OIL_ML + 1] = *((uint16_t*)&add_param.actual_oil_ml );
				usSRegHoldBuf[OIL_ADD_TIME] = *((uint16_t*)&cur_seconds + 1);      //注油计时
				usSRegHoldBuf[OIL_ADD_TIME + 1] = *((uint16_t*)&cur_seconds );
				usSRegHoldBuf[AVERAGE_FLOW_RATE] = *((uint16_t*)&add_param.average_flow_rate + 1);      //平均流量
				usSRegHoldBuf[AVERAGE_FLOW_RATE + 1] = *((uint16_t*)&add_param.average_flow_rate );
				usSRegHoldBuf[REAL_TIME_FLOW_RATE] = *((uint16_t*)&add_param.real_time_flow_rate + 1);       //实时流量
				usSRegHoldBuf[REAL_TIME_FLOW_RATE + 1] = *((uint16_t*)&add_param.real_time_flow_rate );
				taskEXIT_CRITICAL();
				
				if(cur_seconds > 15.0) { //注油时间超过15s，触发注油超时报警
					log_i("oil add over time(15s)");
					end_pulse_count();       //结束脉冲计数
					alerm(oil_add_timeout_error); 
					set_oil_add_process(oil_parameter_seted); 
					break;
				}
				if(add_param.actual_pulse >= add_param.compute_pulse) { //注油完成
					log_i("complete once oil add");
					add_param.oil_add_cnt++;
					open_finish_oil_led();   //点亮注油完成灯
					end_pulse_count();       //关闭脉冲计数器
					set_oil_add_process(oil_add_completed); 
					taskENTER_CRITICAL();
					usSRegHoldBuf[OIL_ADD_START] = 0;  //注油结束
					usSRegHoldBuf[OIL_ADD_CNT] = add_param.oil_add_cnt;      //注油次数
					taskEXIT_CRITICAL();
					oil_resorpt();          //回吸
					//停止按钮按下，注油完成进行关闭操作
					if(stop_action_to_handle == 1) {
						close_green_led();    
						close_elect_valve_1();  //关闭电磁通断阀1
						close_motor();   //关闭电机一体泵
						close_air_valve_1();//关闭气控球阀1
						close_air_valve_2();//关闭气控球阀2
						close_air_valve_3(); //关闭气控球阀3
						close_air_valve_4();//关闭气控球阀4
						close_air_valve_5();//关闭气控球阀5
						close_heat_switch();//关闭PID加热
						close_oil_gun_valve();//关闭注油枪电磁阀
						close_elect_valve_2();//关闭电磁通断阀2
						close_finish_oil_led();
						stop_action_to_handle = 0;
					}
					break;
				}	
			}
        }
	}
}
/*****************************************************************/
/******************          传感器定时读取任务     ***************/
static TaskHandle_t sensor_poll_task_Handle = NULL;

//输入事件中断处理函数
static void sensor_poll_task(void *parameter)
{
	uint16_t liquid_adc_value, pressure_adc_value, temperatrue_adc_value;
	float liquid, pressure, temperatrue;
	while(1) {
		/*液位传感器*/
		liquid_adc_value = get_liquid_adc_value();
		liquid = to_liquid(liquid_adc_value);			
		taskENTER_CRITICAL();
		usSRegInBuf[LIQQUID_INREG_INDEX] = *((uint16_t*)&liquid + 1);
		usSRegInBuf[LIQQUID_INREG_INDEX + 1] = *((uint16_t*)&liquid );
		taskEXIT_CRITICAL(); 
		
		if(liquid_adc_value <= LIQUID_LOWER_LIMIT)
			alerm(liquid_lower_error);  //低于油液的20%，下限报警     //暂时不用
		else if(liquid_adc_value <= START_REPLENISH_RIGHT)
			start_replenish_oil();     //补油
		else if(liquid_adc_value >= LIQUID_UPPER_LIMIT)
			alerm(liquid_upper_error); //高于油液的90%，上限报警
		else if(liquid_adc_value >= STOP_REPLENISH)
			stop_replenish_oil();      //高于油液的80%，停止补油
		
		/*压力传感器*/
		pressure_adc_value = get_pressure_adc_value();
		pressure = to_pressure(pressure_adc_value);
		taskENTER_CRITICAL();
		usSRegInBuf[PRESSURE_INREG_INDEX] = *((uint16_t*)&pressure + 1);
		usSRegInBuf[PRESSURE_INREG_INDEX + 1] = *((uint16_t*)&pressure );
		taskEXIT_CRITICAL();
		// 只有当注油参数已经设定，压力传感器报警才起作用
		if(get_oil_add_process() >= oil_parameter_seted) {    //暂时不用
			if((pressure - formula_selected->pressure) <= -0.2
			|| (pressure - formula_selected->pressure) >= 0.2)
				alerm(pressure_over_error);
		}

		/*温度传感器*/
		temperatrue_adc_value = get_temperatrue_adc_value();
		temperatrue = to_temperatrue(temperatrue_adc_value);
		usSRegInBuf[TEMPERATRUE_INREG_INDEX] = *((uint16_t*)&temperatrue + 1);
		usSRegInBuf[TEMPERATRUE_INREG_INDEX + 1] = *((uint16_t*)&temperatrue );
		if(temperatrue > 100.0)
			alerm(high_temptrue_error);

		vTaskDelay(pdMS_TO_TICKS(200));     //采样周期为200ms
	}
}
/*****************************************************************/
/******************          配方操作相关任务     ***************/
static TaskHandle_t formula_process_task_Handle = NULL;
static EventGroupHandle_t  formula_process_event = NULL;

static void formula_process_task(void *parameter)
{
	EventBits_t recvedEvent;
	while(1) {
    	recvedEvent = xEventGroupWaitBits(formula_process_event,
            EV_FORMULA_INDEX_GET | EV_FORMULA_SELECT | EV_FORMULA_SAVE,
			pdTRUE,        /*退出时清除事件位*/
			pdFALSE,       /*满足感兴趣的任一事件则返回*/ 
			portMAX_DELAY);
		switch (recvedEvent) {
			case EV_FORMULA_INDEX_GET:
				log_i("EV_FORMULA_INDEX_GET get");
				wirte_formula_param_to_SRegHold(usSRegHoldBuf[FORMULA_INDEX]);
				log_i("cur formula index is %d", cur_formula_index);
				break;
			case EV_FORMULA_SAVE:
				log_i("EV_FORMULA_SAVE get");
				read_formula_param_from_SRegHold();  //写入到配方数组中
				break;
			case EV_FORMULA_SELECT:
				log_i("EV_FORMULA_SELECT get");
				//选定配方应在加热完成或注油完成后
				if((get_temperature_reached_level() == INPUT_SET) || get_oil_add_process() == oil_add_completed) {
					select_formula();     //选定配方
					//设置注油状态为注油参数已选定
					set_oil_add_process(oil_parameter_seted);  
					#if USE_SD
					write_all_formula_to_SD();           //保存到SD卡中
					#endif
				}		
				break;
		}
	}
}
void formula_event_post(formula_process_event_type eEvent)
{
	xEventGroupSetBits(formula_process_event, eEvent);
}
/*******************************************************************/
//设置注油机运行状态
void set_run_mode(run_mode_type mode)
{
	taskENTER_CRITICAL();
	run_mode = mode;
	usSRegHoldBuf[RUN_MODE] = mode;   //改写运行状态寄存器值
    taskEXIT_CRITICAL();
}
run_mode_type get_run_mode(void)
{
	run_mode_type state;
	taskENTER_CRITICAL();
	state = run_mode;
    taskEXIT_CRITICAL();
	
	return state;
}
//设置注油过程
void set_oil_add_process(oil_add_process_type state)
{
	taskENTER_CRITICAL();
	oil_add_process = state;
	usSRegHoldBuf[OIL_ADD_PROCESS] = state;
    taskEXIT_CRITICAL();
}
oil_add_process_type get_oil_add_process(void)
{
	oil_add_process_type state;
	taskENTER_CRITICAL();
	state = oil_add_process;
    taskEXIT_CRITICAL();
	
	return state;
}

/***********************************************************************/
//   自动运行状态：绿灯亮
//   报警状态：   红灯亮 + 蜂鸣器
//   手动运行状态：黄灯亮
//   完成一次注油：注油枪完成显示灯
//进入自动运行状态,开始对油进行加热
void auto_heating_oil(void)
{
	if(get_run_mode() != auto_running)
		return ;
	// if(get_oil_add_process() != oil_add_init && get_oil_add_process() != oil_stop)
	// 	return;

	log_i("auto heating oil");

	open_green_led();      //绿灯亮
	
	open_elect_valve_1();  //电磁阀1打开
	open_motor();          //电机一体泵KM1打开
	open_air_valve_1();    //气控球阀1打开
	close_air_valve_2();   //气控球阀2关闭
	open_air_valve_3();    //气控球阀3打开
	close_air_valve_4();   //气控球阀4关闭
	close_air_valve_5();   //气控球阀5关闭
	open_heat_switch();    //PID温控器启动加热，
	set_oil_add_process(oli_heating);
}

//进入手动运行状态,
/*        手动运行控制（触摸屏）：
	电磁阀-气控球阀1 Y5，电磁阀-气控球阀2 Y6，电磁阀-气控球阀3 Y7，
	电磁阀-气控球阀4 Y8，电磁阀-气控球阀5 Y9，电磁阀-注油枪 Y10。
	油罐通气——电控阀1（电磁通断1）Y11、回吸油箱回吸——电控阀2（电磁通断2）Y12
	油罐补油——电控阀3（电磁通断3）Y13
	PID温控仪启动加热Y14、PID温控仪预留Y15。
	电机一体泵启停——电机Y16
	当PID温控仪控制加热器加热到设定温度时，PID温控仪温度达到X8
    ——加热温度范围到达、注油就绪（触摸屏上显示）
 */
void goto_manual_running(void)
{
	if(get_run_mode() == manual_running)
		return ;
	open_yellow_led();
	set_run_mode(manual_running);
}
//进入自动运行状态,
void goto_auto_running(void)
{
	if(get_run_mode() == auto_running)
		return ;
	// if(get_oil_add_process() != oil_tempt_seted)
	// 	return ;
	open_green_led();
	set_run_mode(auto_running);
}

//注油参数设定完成，开始注油，或者运行状态为注油完成也能进行下一次注油
void add_oil(void)
{
	oil_add_process_type add_process;
	
	// if(get_run_mode() != auto_running) {       //注油按键仅在自动运行模式生效    暂时允许手动模式注油，正式代码需添加 test_code
	// 	log_a("auto add oil only in auto_running mode");
	// 	return;
	// }
	
	add_process = get_oil_add_process();
	if(add_process == oil_parameter_seted || add_process == oil_add_completed) {

	// if(usSRegHoldBuf[OIL_ADD_START] == 0)  //注油触发位未置一
	// 	return;
	
		log_i("start add oil");

		set_oil_add_process(oil_adding); //切换为注油中
		close_finish_oil_led();  //注油枪完成显示灯熄灭
		
		open_air_valve_2();    //气控球阀2打开
		close_air_valve_3();   //气控球阀3关闭
		open_air_valve_4();    //气控球阀4打开
		open_oil_gun_valve();  //注油枪电磁阀打开
		close_elect_valve_2(); //电磁通断阀2关闭
		
		/*流量计高速脉冲计数开始，完成时停止注油计时，*/
		start_pulse_count();   //开启脉冲计数
		
		usSRegHoldBuf[OIL_ADD_START] = 1;
		//释放信号量，开始注油计时
		xSemaphoreGive(oil_add_time_cnt_sem);
	}
}

//回吸
void oil_resorpt(void)
{
	if(get_oil_add_process() != oil_add_completed)
		return ;
	
	log_i("start resorpt");

//	open_air_valve_3();  //打开气控球阀3
	close_air_valve_4(); //关闭气控球阀4
	close_oil_gun_valve();//关闭注油枪阀
	open_elect_valve_2(); //打开电磁通断阀2
	
	open_finish_oil_led(); //注油完成灯打开
}

void alerm(uint8_t error)
{
	open_red_led();
	open_buzzer();
	
	taskENTER_CRITICAL();
	usSRegHoldBuf[ALERM_ERROR_CODE] = error;
    taskEXIT_CRITICAL();
	// WriteSRegHoldBuf(ALERM_ERROR_CODE, error);
}

void reset_alerm(void)
{
	close_red_led();
	close_buzzer();
	taskENTER_CRITICAL();
	usSRegHoldBuf[ALERM_ERROR_CODE] = no_error;
    taskEXIT_CRITICAL();
	// WriteSRegHoldBuf(ALERM_ERROR_CODE, 0);
}
//补油
void start_replenish_oil(void)
{
	log_i("start replenish oil");
	open_elect_valve_3(); //打开电磁通断阀3
}
void stop_replenish_oil(void)
{
	log_i("stop replenish oil");
	close_elect_valve_3(); //关闭电磁通断阀3
}

/****************************************************************/
//初始化注油状态以及创建注油计时任务
void oil_init(void)
{
	set_run_mode(run_init);
	ev_mode_select_cb();     //选择运行模式
	set_oil_add_process(oil_add_init);
	//创建注油计时任务
	xTaskCreate((TaskFunction_t )oil_add_time_cnt_task, /* 任务入口函数 */
			(const char*    )"task_gpio_it",/* 任务名字 */
			(uint16_t       )128,   /* 任务栈大小 */
			(void*          )NULL,	/* 任务入口函数参数 */
			(UBaseType_t    )5,	    /* 任务的优先级：设置为最高优先级 */
			(TaskHandle_t*  )&oil_add_time_cnt_task_Handle);/* 任务控制块指针 */
	if(oil_add_time_cnt_task_Handle != NULL)
		log_i("oil_add_time_cnt_task init!");
	//创建传感器轮询任务
	xTaskCreate((TaskFunction_t )sensor_poll_task, /* 任务入口函数 */
			(const char*    )"sensor_poll",/* 任务名字 */
			(uint16_t       )128,   /* 任务栈大小 */
			(void*          )NULL,	/* 任务入口函数参数 */
			(UBaseType_t    )3,	    /* 任务的优先级：设置为最高优先级 */
			(TaskHandle_t*  )&sensor_poll_task_Handle);/* 任务控制块指针 */
	if(sensor_poll_task_Handle != NULL)
		log_i("sensor_task init!");
	oil_add_time_cnt_sem = xSemaphoreCreateBinary();
    if(oil_add_time_cnt_sem == NULL) {
        log_a("oil_add_time_cnt_sem init error!");
        return;
    }else
        log_i("oil_add_time_cnt_sem init");
	#if USE_SD
	//创建配方操作任务
	if(read_all_formula_from_SD() == 0)  //从SD卡中读取配方
		log_i("read formula from sd");
	#endif
	xTaskCreate((TaskFunction_t )formula_process_task, /* 任务入口函数 */
			(const char*    )"sensor_poll",/* 任务名字 */
			(uint16_t       )128,   /* 任务栈大小 */
			(void*          )NULL,	/* 任务入口函数参数 */
			(UBaseType_t    )2,	    /* 任务的优先级：设置为最高优先级 */
			(TaskHandle_t*  )&formula_process_task_Handle);/* 任务控制块指针 */
	if(formula_process_task_Handle != NULL)
		log_i("formula process task init!");
	formula_process_event = xEventGroupCreate();
	if(formula_process_event != NULL)
		log_i("formula process event init");
}

//关闭Y5~Y16
static void close_y5_y16(void)
{
	close_air_valve_1();   //关闭气控球阀1
	close_air_valve_2();   //关闭气控球阀2
	close_air_valve_3();   //关闭气控球阀3
	close_air_valve_4();   //关闭气控球阀4
	close_air_valve_5();   //关闭气控球阀5
	close_oil_gun_valve(); //关闭注油枪电磁阀
	close_elect_valve_1(); //关闭电磁通断阀1
	close_elect_valve_2(); //关闭电磁通断阀2
	close_elect_valve_3(); //关闭电磁通断阀3
	close_heat_switch();   //关闭PID加热
	close_reserved();
	close_motor();         //关闭电机一体泵
}

//打开Y5~Y16
static void open_y5_y16(void)
{
	open_air_valve_1();   //关闭气控球阀1
	open_air_valve_2();   //关闭气控球阀2
	open_air_valve_3();   //关闭气控球阀3
	open_air_valve_4();   //关闭气控球阀4
	open_air_valve_5();   //关闭气控球阀5
	open_oil_gun_valve(); //关闭注油枪电磁阀
	open_elect_valve_1(); //关闭电磁通断阀1
	open_elect_valve_2(); //关闭电磁通断阀2
	open_elect_valve_3(); //关闭电磁通断阀3
	open_heat_switch();   //关闭PID加热
	open_reserved();
	open_motor();         //关闭电机一体泵
}


/**************************************************************/
/*******************      按键回调函数       ******************/
/*      启动按钮
 *  当前设备无报警，停止按钮、急停按钮没有被按下，手自动选择在自动模式
 */
void ev_start_cb(void)
{
	if(get_start_button_level() == BUTTON_SET) {    //处于有效电平
		log_i("start button clicked");
		open_test_led0();
		if(get_red_led_level() == LED_ON)  {        //处于报警状态
			log_i("alerm state, can not start");
			return;
		}
		if(get_stop_button_level() == BUTTON_SET) { //停止按钮按下
			log_i("stop state, can not start");
			return;
		}
		if(get_urgent_stop_button_level() == BUTTON_SET) {//急停按钮按下
			log_i("urgent stop state, can not start");
			return;
		}

		if(get_oil_add_process() == oil_stop) { //处于停止状态
			if(get_select_button_level() == AUTO) {
				log_i("remove stop, restore auto run");
				close_red_led();   //关闭红灯
				set_run_mode(auto_running);//恢复到自动运行模式
				set_oil_add_process(oil_add_init);
			}else{
				log_i("mannul mode, cannot remove stop mode");
				return;
			}
		}

		if(get_run_mode() != auto_running) {
			log_i("manual run, can not start");
			return;
		}

		auto_heating_oil();           //执行自动运行动作，主要是进行加热等等
		ucSDiscInBuf[start_button/8] |= (1 << (start_button%8));
	}
}

/*     停止按钮
 *  自动运行状态解除，如果设备注油中，则注油完成后才生效
 */
void ev_stop_cb(void)
{
	if(get_stop_button_level() == BUTTON_SET) {
		log_i("stop button clicked");
		if(get_run_mode() != auto_running) {
			log_i("stop button valid only in auto mode");
			return;
		}

		set_run_mode(run_init);   //切换为初始态
		//usSRegHoldBuf[RUN_MODE] = run_init;
		set_oil_add_process(oil_stop);
		open_red_led();          //打开红灯
		close_green_led();       //关闭绿灯
		if(get_oil_add_process() == oil_adding) {
			stop_action_to_handle = 1;
		}else {
			// close_green_led();    
			close_elect_valve_1();  //关闭电磁通断阀1
			close_motor();   //关闭电机一体泵
			close_air_valve_1();//关闭气控球阀1
			close_air_valve_2();//关闭气控球阀2
			close_air_valve_3(); //关闭气控球阀3
			close_air_valve_4();//关闭气控球阀4
			close_air_valve_5();//关闭气控球阀5
			close_heat_switch();//关闭PID加热
			close_oil_gun_valve();//关闭注油枪电磁阀
			close_elect_valve_2();//关闭电磁通断阀2
			close_finish_oil_led();
		}
		ucSDiscInBuf[stop_button/8] |= (1 << (stop_button%8));
	}
}
/*    复位按钮
 *  清除报警
 */
void ev_reset_cb(void)
{	
	if(get_reset_button_level() == BUTTON_SET) {
		log_i("reset button clicked");
		reset_alerm();
		ucSDiscInBuf[reset_button/8] |= (1 << (reset_button%8));
	}
}
/*   加热温度范围到达，注油就绪(触摸屏上显示)
 *	当PID温控仪加热到设定温度是，PID温度到达
 */
void ev_heating_temprature_reached(void)
{
	if(get_temperature_reached_level() == INPUT_SET) {
		log_i("pid heating temprature reached");
		set_oil_add_process(oil_heat_completed);   //加热完成，注油就绪
                                                //下一步进行设定注油参数
	}
}
/*    急停按钮
 *  自动运行状态解除，无论注油与否
 *   由于急停与PID温度到达公用一个中断线，因此公用一个中断服务函数
 */
void ev_urgent_stop_cb(void)
{
	if(get_urgent_stop_button_level() == BUTTON_SET) {
		log_i("urgent stop button clicked");		
		if(get_run_mode() != auto_running) {
			log_i("urgent stop button valid only in auto mode");
			return;
		}
		set_run_mode(run_init);
		//usSRegHoldBuf[RUN_MODE] = run_init;
		set_oil_add_process(oil_stop);
		open_red_led();        //打开红灯
		close_green_led();     //关闭绿灯
		close_y5_y16();
		close_finish_oil_led();
//		ucSDiscInBuf[urgent_stop_button/8] |= (1 << (urgent_stop_button%8));
	}
}
/*    手动/自动选择按钮
 *  切换设备的手动或者自动运行状态
 *  与流量计高速脉冲计数错误公用一个中断服务函数
 */
void ev_mode_select_cb(void)
{
	if(get_select_button_level() == AUTO) {
		log_i("auto running mode select");
		set_run_mode(auto_running);
		close_yellow_led();        //关闭黄灯
		close_y5_y16();            //关闭Y5~Y16
		// ucSDiscInBuf[select_button/8] |= (1 << (select_button%8));
	}else {
		log_i("manual running mode select");
		set_run_mode(manual_running);
		open_yellow_led();       //打开黄灯	
		close_y5_y16();          //关闭Y5~Y16	
		// ucSDiscInBuf[select_button/8] &= ~(1 << (select_button%8));
	}
}
//脉冲计数错误
void ev_pulse_cnt_error_cb(void)
{
	if(get_pulse_cnt_error_level() == INPUT_SET) {
		log_i("pulse cnt error reach");
		alerm(pulse_cnt_error);
//		ucSDiscInBuf[pulse_cnt_error_reach/8] |= (1 << (pulse_cnt_error_reach%8));
	}
}

/*   注油枪按钮
 *  自动运行状态下，注油触发
 *  与PID温度超限报警公用一个中断服务函数
 */
void ev_start_add_oil_cb(void)
{
	if(get_start_add_oil_button_level() == BUTTON_SET) {
		log_i("start add oil button clicked");
		add_oil();  //开始注油，
//		ucSDiscInBuf[start_add_oil_button/8] |= (1 << (start_add_oil_button%8));
		usSRegHoldBuf[OIL_ADD_START] = 1;   //标记已经开始注油，由于触摸屏也需要注油按键，因此需要在保持寄存器中写1
	}
}
//PID超限报警
void ev_temprature_overlimit_cb(void)
{
	if(get_temperature_overlimit_level() == INPUT_SET) {
		log_i("temperature overlimit reach");
		alerm(temptrue_overlimit_error);
//		ucSDiscInBuf[temperature_overlimit/8] |= (1 << (temperature_overlimit%8));
	}	
}

/*   压差发讯器中断回调函数
 */
void ev_presssure_diff_signal_cb(void)
{
	if(get_presssure_diff_signal_level() == INPUT_SET) {
		log_i("presssure diff signal reach");
		alerm(pressure_transmit_error);
//		ucSDiscInBuf[presssure_diff_signal/8] |= (1 << (presssure_diff_signal%8));
	}
}
/*  温度超限报警中断回调函数
 */
void ev_temperature_overload_cb(void)
{
	if(get_temperature_overload_level() == INPUT_SET) {
		log_i("temperature overload reach");
		alerm(motor_tempt_overload_error);
//		ucSDiscInBuf[temperature_overload/8] |= (1 << (temperature_overload%8));
	}
}


/*******************************************************************/
/*                   自动注油流程
 *   1、打开总开关上电，手动设定PID温控仪加热温度
 *   2、按下自动运行模式按钮，再按下启动按钮
 *   3、PID温控仪加热到设定温度
 *   4、设定注油参数
 * 		  触摸屏上选配方，按下注油枪按钮，触发注油
 *   5、开始注油
 *       流量计高速脉冲计数，当大于等于计算脉冲数时，注油完成
 *       进行回吸
 */

