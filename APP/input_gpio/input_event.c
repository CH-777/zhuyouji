#include "input_event.h"

#include "elog.h"


#include "event_groups.h"

/****************************************************************/
/*******************          全局变量              *************/
//gpio输入中断事件组
// static EventGroupHandle_t xGPIOInputEvent = NULL;  

// /***************************************************************/
// //初始化中断事件组
// uint8_t xGPIOInputEventInit(void)
// {
// 	xGPIOInputEvent = xEventGroupCreate();
//     return (xGPIOInputEvent != NULL) ? 0 : 1;
// }
// //发送中断事件组事件
// uint8_t xGPIOInputEventEventPost(eGPIOInputEventType eEvent)
// {
//     xEventGroupSetBits(xGPIOInputEvent, eEvent);
//     return 0;
// }

// //处理中断事件组
// void eGPIOInputEventPoll(void)
// {
//     EventBits_t recvedEvent;
	
//     recvedEvent = xEventGroupWaitBits(xGPIOInputEvent,
//             EV_START | EV_STOP | EV_RESET| EV_URGENT_STOP | 
// 			EV_MODE_SELECT| EV_START_ADD_OIL|
// 	        EV_PRESSURE_DIFF_SIGNAL| EV_TEMPRATURE_OVERLOAD|
//             EV_TEMPRATURE_REACHED| EV_TEMPRATURE_OVERLIMIT,
// 	        pdTRUE,    /*退出时清除事件位*/
//             pdFALSE,   /*满足感兴趣的任一事件则返回*/ 
//             portMAX_DELAY);

// 	if((recvedEvent & EV_START) == EV_START){
// 		log_i("EV_START recved!");

// 	}
// 	if((recvedEvent & EV_STOP) == EV_STOP){
// 		log_i("EV_STOP recved!");
		
// 	}
// 	if((recvedEvent & EV_RESET) == EV_RESET){	
// 		log_i("EV_RESET recved!");

// 	}
// 	if((recvedEvent & EV_URGENT_STOP) == EV_URGENT_STOP){	
// 		log_i("EV_URGENT_STOP recved!");
		
// 	}
// 	if((recvedEvent & EV_MODE_SELECT) == EV_MODE_SELECT){	
// 		log_i("EV_MODE_SELECT recved!");
		
// 	}
// 	if((recvedEvent & EV_START_ADD_OIL) == EV_START_ADD_OIL){	
// 		log_i("EV_START_ADD_OIL recved!");
		
// 	}
// 	if((recvedEvent & EV_PRESSURE_DIFF_SIGNAL) == EV_PRESSURE_DIFF_SIGNAL){	
// 		log_i("EV_PRESSURE_DIFF_SIGNAL recved!");
		
// 	}
// 	if((recvedEvent & EV_TEMPRATURE_OVERLOAD) == EV_TEMPRATURE_OVERLOAD){	
// 		log_i("EV_TEMPRATURE_OVERLOAD recved!");
		
// 	}
// 	if((recvedEvent & EV_TEMPRATURE_REACHED) == EV_TEMPRATURE_REACHED){	
// 		log_i("EV_TEMPRATURE_REACHED recved!");
		
// 	}
// 	if((recvedEvent & EV_TEMPRATURE_OVERLIMIT) == EV_TEMPRATURE_OVERLIMIT){	
// 		log_i("EV_TEMPRATURE_OVERLIMIT recved!");
		
// 	}
// }


/***************************************************************/







