#include "misc.h"
#include "uart.h"
#include "core_cm3.h"
#include "misc.h"
#include <stdio.h>
#include "flash.h"
#include "delay.h"

//extern u8 DATENUM;
#define USART_DATA_MAX  10*1024
#define DEBUGFLAG 1
extern u16 USART2_RX_CNT;
extern u8 UPDATE_BUFF[USART_DATA_MAX];
u16 PageNUM = 0; //升级总包页数
int main(void)
{

	u8 i = 0;
	u8 j =0;
	u8 SendBuf[1] = {0};
	u16 Flag = 0x2;
	u32 addr = FLASH_DOWNLOAD_ADDR;
	SysTick_Init();
 	SystemInit();
	USARTINIT(9600);
	if (DEBUGFLAG)
		printf("One\n");
	
	if(FLASH_ReadHalfWord(FLASH_UPDATA_FLAG) != 0x1)
	{
		//不升级直接转到APP
		if (DEBUGFLAG)
			printf("NO update ---Run App...\n");
    	IAP_Load_App(FLASH_APP_ADDR);//转到app
	}
	else
	{
		Flash_WriteFlag(FLASH_UPDATA_FLAG,Flag);
		printf("update\n");
		//第一阶段 握手
		for (i = 60;i > 0;i--)
		{
			//第一阶段握手60s不成功退出握手
			if (DEBUGFLAG)
			{
				printf("recv len:%d\n",USART2_RX_CNT);
				if (USART2_RX_CNT)
				{
					for (j=0;j<USART2_RX_CNT;j++)
					{
						printf("%x",UPDATE_BUFF[j]);
					}
				}
			}
			if (USART2_RX_CNT >= 10)
			{
				if (DEBUGFLAG)
				{
					printf("Handshake:%c %c\n",UPDATE_BUFF[0],UPDATE_BUFF[10]);
				}
				if ((UPDATE_BUFF[0] == ':') &&((UPDATE_BUFF[10] == '!'))) //握手命令：“:MK.UPDATA!”
				{
					memset(UPDATE_BUFF,0,sizeof(UPDATE_BUFF));
					USART2_RX_CNT = 0;
					Delay_ms(20);
					SendBuf[0] = MK_SUCCEEDED;
					UART2_Send_Date(SendBuf,sizeof(SendBuf));
					if (DEBUGFLAG)
						printf("Handshake Succed!\n");
					Delay_ms(10);//延时10ms
					break;
				}
				else
				{
					memset(UPDATE_BUFF,0,sizeof(UPDATE_BUFF));
					USART2_RX_CNT = 0;
					Delay_ms(20);
					SendBuf[0] = MK_FAILED;
					UART2_Send_Date(SendBuf,sizeof(SendBuf));
					if (DEBUGFLAG)
						printf("Handshake Fail!\n");
					IAP_Load_App(FLASH_APP_ADDR);//转到app
				}
			}
			//60s未收到握手命令，退出在线升级，转到APP程序
			if (i == 1)
			{
				if(((*(vu32*)(FLASH_APP_ADDR + 4))&0xFF000000) == 0x08000000)
				{
					if (DEBUGFLAG)
						printf("Exit update ---Run App...\n");
					SendBuf[0] = MK_FAILED;
		    		UART2_Send_Date(SendBuf,sizeof(SendBuf));
		    		IAP_Load_App(FLASH_APP_ADDR);//转到app
				}
			}		
			memset(UPDATE_BUFF,0,sizeof(UPDATE_BUFF));
			USART2_RX_CNT = 0;
			Delay_ms(1000);//延时1s
			if (DEBUGFLAG)
				printf("Wait Date\n");
		}

		//第二阶段匹配信息
		if (DEBUGFLAG)
		{
			printf("Two\n");
			printf("USART2_RX_CNT:%d\n",USART2_RX_CNT);
			printf("%x%x%x%x%x%x%x%x",UPDATE_BUFF[0],UPDATE_BUFF[1],UPDATE_BUFF[2],UPDATE_BUFF[3],UPDATE_BUFF[4],\
				UPDATE_BUFF[5],UPDATE_BUFF[6],UPDATE_BUFF[7]);
		}
		if ((UPDATE_BUFF[0]== 0x46) && (UPDATE_BUFF[1]== 0x76)&& (UPDATE_BUFF[2]== 0x50)&& (UPDATE_BUFF[3]== 0x74)&& \
			(UPDATE_BUFF[4]== 0x7a)&& (UPDATE_BUFF[5]== 0x56)&& (UPDATE_BUFF[6]== 0x30)&& (UPDATE_BUFF[7]== 0x31))
		{
			memset(UPDATE_BUFF,0,sizeof(UPDATE_BUFF));
			USART2_RX_CNT = 0;
			SendBuf[0] = MK_SUCCEEDED;
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
			if (DEBUGFLAG)
				printf("Date Match Succed!\n");
			Delay_ms(10);//延时10ms
		}
		else //匹配失败退出在线升级转到APP程序运行
		{
			SendBuf[0] = MK_FAILED;
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
			if (DEBUGFLAG)
				printf("Date Match Fail!\n");		    
	    	IAP_Load_App(FLASH_APP_ADDR);//转到app
		}

		if (DEBUGFLAG)
		{
			printf("Three\n");
			printf("rx:%d date:%d %d %d %d\n",USART2_RX_CNT,UPDATE_BUFF[0],UPDATE_BUFF[1],UPDATE_BUFF[2],UPDATE_BUFF[3]);
		}

		//第三阶段获取升级数据
		PageNUM = 0;
		if (UPDATE_BUFF[3] != 0) //长度机芯以int发送
		{
			PageNUM = UPDATE_BUFF[3];
			memset(UPDATE_BUFF,0,sizeof(UPDATE_BUFF));
			USART2_RX_CNT = 0;
			Delay_ms(10);//延时10ms
			if (DEBUGFLAG)
				printf("PageNUM:%d\n",PageNUM);
		}
		else //获取pagenum失败退出在线升级转到APP程序运行
		{
			SendBuf[0] = MK_FAILED;
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
			if (DEBUGFLAG)
				printf("Page is NULL!\n");		    
	    	IAP_Load_App(FLASH_APP_ADDR);//转到app
		}

		//备份当前APP
		IAP_BACK_APP();
		SendBuf[0] = MK_ACK; //协议中没有则共用page size的ACK应答上层等待1min
		UART2_Send_Date(SendBuf,sizeof(SendBuf));
		//接收写入Download分区
		for (i=0;i < PageNUM;i++)
		{
			Delay_ms(50);
			memset(UPDATE_BUFF,0,sizeof(UPDATE_BUFF));
			USART2_RX_CNT = 0;
			SendBuf[0] = MK_ACK;//获取数据大小ACK
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
			Delay_ms(4000);
			printf("size:%d\n",USART2_RX_CNT);
			//将数据写入Download分区
			IAP_WriteBin(addr,UPDATE_BUFF,USART2_RX_CNT);
			addr += USART2_RX_CNT;
			memset(UPDATE_BUFF,0,sizeof(UPDATE_BUFF));
			USART2_RX_CNT = 0;
			SendBuf[0] = MK_ACK; //写入ACK
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
			Delay_ms(100);
			
		}

		//第四阶段
		IAP_Copy_App();
		 //判断是否有APP程序
		//中断向量表判断
		printf("0x%x",*(vu32*)(FLASH_APP_ADDR + 4));
		if(((*(vu32*)(FLASH_APP_ADDR + 4))&0xFF000000) == 0x08000000)
		{
			if (DEBUGFLAG)
				printf("Run App...\n");
		    SendBuf[0] = MK_ACK;
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
		    IAP_Load_App(FLASH_APP_ADDR);//转到app
		}
		else
		{
			if (DEBUGFLAG)
				printf("No APP!\n");
			SendBuf[0] = MK_FAILED;
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
			//还原APP
			IAP_REST_APP();
			IAP_Load_App(FLASH_APP_ADDR);//转到app			
		}
	}
}
