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
 * File: $Id: portserial.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */
#include <stm32f10x.h>
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    //ENTER_CRITICAL_SECTION();
	
	if (xRxEnable) {
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
		GPIO_ResetBits(GPIOA, GPIO_Pin_11);              //拉低PA11电平，切换为接收模式
	}else {
		GPIO_SetBits(GPIOA, GPIO_Pin_11);                //拉高PA11电平，切换为发送模式
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	}

	if (xTxEnable)
		USART_ITConfig(USART1, USART_IT_TC, ENABLE);
	else
		USART_ITConfig(USART1, USART_IT_TC, DISABLE);

	//EXIT_CRITICAL_SECTION();
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	BOOL    bInitialized = TRUE;
	
	(void)ucPORT;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/*485收发模式切换引脚，PA11*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = ulBaudRate;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	switch ( eParity )
	{
	case MB_PAR_NONE:
		USART_InitStructure.USART_Parity = USART_Parity_No;
		break;
	case MB_PAR_ODD:
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
		break;
	case MB_PAR_EVEN:
		USART_InitStructure.USART_Parity = USART_Parity_Even;
		break;
	}
	
	switch ( ucDataBits )
	{
	case 8:
		if(eParity==MB_PAR_NONE)
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		else
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		break;
	case 7:
		break;
	default:
		bInitialized = FALSE;
	}	
	
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
			
	USART_Init(USART1, &USART_InitStructure);

	USART_Cmd(USART1, ENABLE);


	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);

    return bInitialized;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */

	USART_SendData(USART1, ucByte);
	
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */

	*pucByte = USART_ReceiveData(USART1);
	
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
		prvvUARTRxISR();
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}

	if (USART_GetITStatus(USART1, USART_IT_TC) == SET) {
		prvvUARTTxReadyISR();
		USART_ClearITPendingBit(USART1, USART_IT_TC);
	}
}
