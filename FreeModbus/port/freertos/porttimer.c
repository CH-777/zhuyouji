/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: porttimer.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

/* ----------------------- Platform includes --------------------------------*/
#include <stm32f10x.h>
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR( void );

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortTimersInit( USHORT usTim1Timerout50us )
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	USHORT PrescalerValue = 0;

    /* Enable the TIM5 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);

    /* TIM5 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	PrescalerValue = (USHORT) (SystemCoreClock / 20000) - 1; 

    TIM_TimeBaseStructure.TIM_Period = usTim1Timerout50us;

    TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;

    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

    TIM_ClearITPendingBit(TIM5, TIM_IT_Update);

    TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);

    TIM_Cmd(TIM5, DISABLE); 
	
    return TRUE;
}


void vMBPortTimersEnable( void )
{
    /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
    //ENTER_CRITICAL_SECTION();

	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);

	TIM_SetCounter(TIM5, 0x0000);

    TIM_Cmd(TIM5, ENABLE);
	
	//EXIT_CRITICAL_SECTION();
}

void vMBPortTimersDisable( void )
{
    /* Disable any pending timers. */
    //ENTER_CRITICAL_SECTION();
	
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	
    TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);

	TIM_SetCounter(TIM5, 0x0000);

    TIM_Cmd(TIM5, DISABLE); 	
	
	//EXIT_CRITICAL_SECTION();
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
static void prvvTIMERExpiredISR( void )
{
    ( void )pxMBPortCBTimerExpired(  );
}

void TIM5_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) == SET) {
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
		prvvTIMERExpiredISR();
	}
}
