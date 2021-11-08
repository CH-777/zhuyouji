#ifndef __OIL_H
#define __OIL_H

//#include "sys.h"

#include "stm32f10x.h"

/*
 * 注油量(ml)         = 设定注油量(ml) + 设定补偿量(ml)
 * 注油质量(g)        = 注油量(ml) * 油液密度(g/ml)
 * 计算脉冲数(pulse)  = 注油质量(g)/单位脉冲注油量(pulse/g)(取整)
 * 实际注油质量(g)    = 实际脉冲数*单位脉冲注油量
 * 实际注油量(ml)     = 实际注油质量(g)/油液密度(g/ml)
 * 注油平均流量(ml/s) = 实际注油量(ml)/注油计时(s)
 */
/********************************************************************/
/****************          注油配方            **********************/
//配方结构体
typedef struct{ 
	uint16_t magic;          //标记该块是否已保存数据
	uint8_t  index;          //当前配方编号,  
	uint8_t  oil_quality;    //油品           
	float pressure;       //设定压力(MPa)     
	float temptrue;       //设定温度(°C)
	float density;        //油液密度(g/ml)
	float oil_amount_set; //设定注油量(ml)  
	float compensation_amount_set;//设定补偿量(ml)  
}formula;

//配方操作相关事件
typedef enum{
 EV_FORMULA_INDEX_GET  = (1 << 0),   //得到上位机发送的编号值
 EV_FORMULA_SAVE       = (1 << 1),   //将配方数据区中的配方写入对应的配方数组
 EV_FORMULA_SELECT     = (1 << 2)   //选定配方
}formula_process_event_type;

#define MAGIC_NUMBER     13579
#define FORMULAS_NUMBER   50   //配方的总数
#define FORMULAS_SECTOR   0     //数据存放起始扇区为0
#define FORMULAS_SIZE     (sizeof(formula)*FORMULAS_NUMBER)

/*****************           注油参数            *****************/
typedef struct{
	float oil_unit_pulse;     //单位脉冲注油量0.1g/pluse
	uint16_t compute_pulse;    //计算脉冲数pulse
	uint16_t actual_pulse;     //实际脉冲数pulse
	uint8_t  oil_add_cnt;      //注油次数
	float compute_oil_g;      //计算注油量g
	float actual_oil_g;       //实际注油量g
	float compute_oil_ml;     //计算注油量ml
	float actual_oil_ml;      //实际注油量ml
	float oil_add_time;       //注油计时s
	float average_flow_rate;  //平均流量ml/s
	float real_time_flow_rate;//实时流量ml/s
}oil_add_param;
/***************************************************************/
/*******************        注油操作           *****************/
//注油机运行状态
typedef enum{
	run_init       = 0,        //初始状态
	auto_running,              //自动运行状态
	manual_running             //手动运行状态
}run_mode_type;
//注油机注油流程
typedef enum{
	oil_add_init        = 0,
	oli_heating         = 1,    //加热中
	oil_heat_completed  = 2,    //加热完成
	oil_parameter_seted = 3,    //注油参数设置完成
	oil_adding          = 4,    //注油中
	oil_add_completed   = 5,    //注油完成
	oil_stop            = 6     //停止
}oil_add_process_type;
//注油错误类型
typedef enum{
	no_error             = 0,
	liquid_upper_error   = 1,       //上上限90%报警，液位传感器
	liquid_lower_error   = 2,       //下下限20%报警，液位传感器
	pressure_over_error  = 3,       //实际压力在设定压力的正负0.2MPa之外报警，压力传感器
	high_temptrue_error  = 4,       //加热器附近高温(100度)报警，温度传感器
	temptrue_overlimit_error   = 5, //加热温度高于设定温度时，PID温控仪超限报警
	pressure_transmit_error    = 6, //过滤器堵塞报警，压差发讯器
	motor_tempt_overload_error = 7, //电机热过载保护
	pulse_cnt_error            = 8, //流量计高速脉冲计数故障
	oil_add_timeout_error      = 9  //注油计时超过15s，触发注油超时报警
}error_type;


/******************************************************************/
//void set_formula_oil_quality(uint8_t quality);
//uint8_t get_formula_oil_quality(void);

//void set_formula_pressure(float pressure);
//float get_formula_pressure(void);

//void set_formula_temptrue(float temp);
//float get_formula_temptrue(void);

//void set_formula_density(float density);
//float get_formula_density(void);

//void set_formula_oil_amount(uint16_t oil_amount);
//uint16_t get_formula_oil_amount(void);

//void set_formula_compensation_amount(uint16_t compensation_amount);
//uint16_t get_formula_compensation_amount(void);
/************************           配方相关      ***************************/
//uint8_t set_oil_add_param(void);

//uint8_t select_formula(void);
//uint8_t get_selected_formula_index(void);

//uint8_t wirte_formula_param_to_SRegHold(uint8_t index);
//uint8_t read_formula_param_from_SRegHold(void);

uint8_t read_all_formula_from_SD(void);
uint8_t write_all_formula_to_SD(void);

void formula_event_post(formula_process_event_type eEvent);

/********************      运行模式以及注油过程相关      ********************/
void set_run_mode(run_mode_type mode);
run_mode_type get_run_mode(void);

void set_oil_add_process(oil_add_process_type state);
oil_add_process_type get_oil_add_process(void);

void auto_heating_oil(void);
void goto_manual_running(void);
void goto_auto_running(void);
void add_oil(void);
void oil_resorpt(void);

void alerm(uint8_t error);
void reset_alerm(void);

void start_replenish_oil(void);
void stop_replenish_oil(void);

void oil_init(void);
/*********************           输入中断回调函数           ******************/
void ev_start_cb(void);
void ev_stop_cb(void);
void ev_reset_cb(void);
void ev_urgent_stop_cb(void);
void ev_mode_select_cb(void);
void ev_pulse_cnt_error_cb(void);
void ev_start_add_oil_cb(void);
void ev_temprature_overlimit_cb(void);
void ev_heating_temprature_reached(void);
void ev_presssure_diff_signal_cb(void);
void ev_temperature_overload_cb(void);

/*****************************************************************************/
#endif


