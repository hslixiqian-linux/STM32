#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include <stdio.h>
#include "flash.h"
#include "update.h"

#define USART_DATA_MAX  20*1024

u8 UPDATE_BUFF[USART_DATA_MAX] = {0};
u16 USART2_RX_CNT = 0;
 
void USARTINIT(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	GPIO_InitTypeDef GPIO_InitStructure1;
	USART_InitTypeDef USART_InitStructure1;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,ENABLE);//复位串口2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,DISABLE);//停止复位

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3); 
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);
	USART_ClearFlag(USART2, USART_FLAG_TC);
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);
	



	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
 	USART_DeInit(USART1);  //复位串口1
	  //USART1_TX   PA.9
    GPIO_InitStructure1.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure1.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure1); //初始化PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure1.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure1);  //初始化PA10     

  	USART_InitStructure1.USART_BaudRate = bound;
  	USART_InitStructure1.USART_WordLength = USART_WordLength_8b;
  	USART_InitStructure1.USART_StopBits = USART_StopBits_1;
  	USART_InitStructure1.USART_Parity = USART_Parity_No;
  	USART_InitStructure1.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  	USART_InitStructure1.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
  	USART_Cmd(USART1, ENABLE);  
}

u8 UART2_Send_Date(uc8 *s,u8 len)
{
	u8 i = 0;
	GPIO_SetBits(GPIOA, GPIO_Pin_1);
	for (i=0;i<len;i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET); //等待发送完成
		USART_SendData(USART2, s[i]);
	}
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);
	return 0;
}

void USART2_IRQHandler(void)
{  
	u8 buf = 0;
    if(USART_GetITStatus(USART2,USART_IT_RXNE))
    {
    	UPDATE_BUFF[USART2_RX_CNT] = USART_ReceiveData(USART2);
		USART2_RX_CNT++;
    }
	//UART2_Send_Date(buf,sizeof(buf));
	//溢出-如果发生溢出需要先读SR,再读DR寄存器则可清除不断入中断的问题
	if(USART_GetFlagStatus(USART2,USART_FLAG_ORE) == SET)
	{
		USART_ReceiveData(USART2);
		USART_ClearFlag(USART2,USART_FLAG_ORE);
	}
	USART_ClearFlag(USART2,USART_IT_RXNE); //一定要清除接收中断 
  
}

int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART1, (uint8_t) ch);

  /* 循环等待直到发送结束*/
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {}

  return ch;
}



