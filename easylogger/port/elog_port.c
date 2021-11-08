/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <elog.h>
#include <elog_flash.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "usart.h"


static xSemaphoreHandle output_lock;

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
static xSemaphoreHandle output_notice;

static void async_output(void *arg);
#endif

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
    //rt_sem_init(&output_lock, "elog lock", 1, RT_IPC_FLAG_PRIO);
    vSemaphoreCreateBinary(output_lock);     //初始值为1
    
    TaskHandle_t async_thread = NULL;
    
    // rt_sem_init(&output_notice, "elog async", 0, RT_IPC_FLAG_PRIO);
     vSemaphoreCreateBinary(output_notice);    
    xSemaphoreTake(output_notice, portMAX_DELAY);  //将信号量output_notice初始为0 

    xTaskCreate((TaskFunction_t)async_output, (const char*)"elog_async",
                (uint16_t)256, NULL, 1, (TaskHandle_t *)&async_thread);
#endif

    return result;
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    /* output to terminal */
    printf("%.*s", size, log);
    /* output to flash */
    // elog_flash_write(log, size);
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
#ifdef ELOG_ASYNC_OUTPUT_ENABLE
    // rt_sem_take(&output_lock, RT_WAITING_FOREVER);
    xSemaphoreTake(output_lock, portMAX_DELAY); //获取信号量
#else
    __disable_irq();
#endif
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
#ifdef ELOG_ASYNC_OUTPUT_ENABLE
    // rt_sem_release(&output_lock);
    xSemaphoreGive(output_lock);    //释放信号量
#else
    __enable_irq();
#endif
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
#ifdef ELOG_ASYNC_OUTPUT_ENABLE
    static char cur_system_time[20] = { 0 };
    uint8_t seconds = 0;
    uint8_t minutes = 0;  // 1 minute = 60 seconds
    uint8_t hours = 0;    // 1 hour = 3600 seconds
    uint16_t days = 0;    // 1 day = 86400 seconds
    unsigned long cur_second = xTaskGetTickCount()/configTICK_RATE_HZ;

    days = cur_second / 86400;
    hours = (cur_second - 86400*days) / 3600;
    minutes = (cur_second - 86400*days - 3600*hours)/60;
    seconds = cur_second % 60;

    snprintf(cur_system_time, 20, "time:%d:%02d:%02d:%02d", days, hours, minutes, seconds);
    return cur_system_time;
#else
    return "";
#endif
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
#ifdef ELOG_ASYNC_OUTPUT_ENABLE
    return (const char *)pcTaskGetTaskName(xTaskGetCurrentTaskHandle());
#else
    return "";
#endif
}

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
void elog_async_output_notice(void) {
    // rt_sem_release(&output_notice);
    xSemaphoreGive(output_notice);
}

static void async_output(void *arg) {
    size_t get_log_size = 0;
    static char poll_get_buf[ELOG_LINE_BUF_SIZE - 4];
    while(true) {
        /* waiting log */
        // rt_sem_take(&output_notice, RT_WAITING_FOREVER);
        xSemaphoreTake(output_notice, portMAX_DELAY);

        /* polling gets and outputs the log */
        while(true) {

#ifdef ELOG_ASYNC_LINE_OUTPUT
            get_log_size = elog_async_get_line_log(poll_get_buf, sizeof(poll_get_buf));
#else
            get_log_size = elog_async_get_log(poll_get_buf, sizeof(poll_get_buf));
#endif

            if (get_log_size) {
                elog_port_output(poll_get_buf, get_log_size);
            } else {
                break;
            }
        }
    }
}
#endif
