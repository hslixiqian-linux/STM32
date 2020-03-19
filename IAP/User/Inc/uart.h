#ifndef _UART_H
#define _UART_H
#include "stm32f10x.h"

//握手机芯云台协议
#define MK_SUCCEEDED	'T'      
#define MK_FAILED		'F'			
#define MK_ACK			'A'
#define MK_OK			'O'


void USARTINIT(u32 bound);
u8 UART2_Send_Date(uc8 *s,u8 len);

#endif


