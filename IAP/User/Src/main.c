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
u16 PageNUM = 0; //�����ܰ�ҳ��
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
		//������ֱ��ת��APP
		if (DEBUGFLAG)
			printf("NO update ---Run App...\n");
    	IAP_Load_App(FLASH_APP_ADDR);//ת��app
	}
	else
	{
		Flash_WriteFlag(FLASH_UPDATA_FLAG,Flag);
		printf("update\n");
		//��һ�׶� ����
		for (i = 60;i > 0;i--)
		{
			//��һ�׶�����60s���ɹ��˳�����
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
				if ((UPDATE_BUFF[0] == ':') &&((UPDATE_BUFF[10] == '!'))) //���������:MK.UPDATA!��
				{
					memset(UPDATE_BUFF,0,sizeof(UPDATE_BUFF));
					USART2_RX_CNT = 0;
					Delay_ms(20);
					SendBuf[0] = MK_SUCCEEDED;
					UART2_Send_Date(SendBuf,sizeof(SendBuf));
					if (DEBUGFLAG)
						printf("Handshake Succed!\n");
					Delay_ms(10);//��ʱ10ms
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
					IAP_Load_App(FLASH_APP_ADDR);//ת��app
				}
			}
			//60sδ�յ���������˳�����������ת��APP����
			if (i == 1)
			{
				if(((*(vu32*)(FLASH_APP_ADDR + 4))&0xFF000000) == 0x08000000)
				{
					if (DEBUGFLAG)
						printf("Exit update ---Run App...\n");
					SendBuf[0] = MK_FAILED;
		    		UART2_Send_Date(SendBuf,sizeof(SendBuf));
		    		IAP_Load_App(FLASH_APP_ADDR);//ת��app
				}
			}		
			memset(UPDATE_BUFF,0,sizeof(UPDATE_BUFF));
			USART2_RX_CNT = 0;
			Delay_ms(1000);//��ʱ1s
			if (DEBUGFLAG)
				printf("Wait Date\n");
		}

		//�ڶ��׶�ƥ����Ϣ
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
			Delay_ms(10);//��ʱ10ms
		}
		else //ƥ��ʧ���˳���������ת��APP��������
		{
			SendBuf[0] = MK_FAILED;
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
			if (DEBUGFLAG)
				printf("Date Match Fail!\n");		    
	    	IAP_Load_App(FLASH_APP_ADDR);//ת��app
		}

		if (DEBUGFLAG)
		{
			printf("Three\n");
			printf("rx:%d date:%d %d %d %d\n",USART2_RX_CNT,UPDATE_BUFF[0],UPDATE_BUFF[1],UPDATE_BUFF[2],UPDATE_BUFF[3]);
		}

		//�����׶λ�ȡ��������
		PageNUM = 0;
		if (UPDATE_BUFF[3] != 0) //���Ȼ�о��int����
		{
			PageNUM = UPDATE_BUFF[3];
			memset(UPDATE_BUFF,0,sizeof(UPDATE_BUFF));
			USART2_RX_CNT = 0;
			Delay_ms(10);//��ʱ10ms
			if (DEBUGFLAG)
				printf("PageNUM:%d\n",PageNUM);
		}
		else //��ȡpagenumʧ���˳���������ת��APP��������
		{
			SendBuf[0] = MK_FAILED;
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
			if (DEBUGFLAG)
				printf("Page is NULL!\n");		    
	    	IAP_Load_App(FLASH_APP_ADDR);//ת��app
		}

		//���ݵ�ǰAPP
		IAP_BACK_APP();
		SendBuf[0] = MK_ACK; //Э����û������page size��ACKӦ���ϲ�ȴ�1min
		UART2_Send_Date(SendBuf,sizeof(SendBuf));
		//����д��Download����
		for (i=0;i < PageNUM;i++)
		{
			Delay_ms(50);
			memset(UPDATE_BUFF,0,sizeof(UPDATE_BUFF));
			USART2_RX_CNT = 0;
			SendBuf[0] = MK_ACK;//��ȡ���ݴ�СACK
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
			Delay_ms(4000);
			printf("size:%d\n",USART2_RX_CNT);
			//������д��Download����
			IAP_WriteBin(addr,UPDATE_BUFF,USART2_RX_CNT);
			addr += USART2_RX_CNT;
			memset(UPDATE_BUFF,0,sizeof(UPDATE_BUFF));
			USART2_RX_CNT = 0;
			SendBuf[0] = MK_ACK; //д��ACK
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
			Delay_ms(100);
			
		}

		//���Ľ׶�
		IAP_Copy_App();
		 //�ж��Ƿ���APP����
		//�ж��������ж�
		printf("0x%x",*(vu32*)(FLASH_APP_ADDR + 4));
		if(((*(vu32*)(FLASH_APP_ADDR + 4))&0xFF000000) == 0x08000000)
		{
			if (DEBUGFLAG)
				printf("Run App...\n");
		    SendBuf[0] = MK_ACK;
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
		    IAP_Load_App(FLASH_APP_ADDR);//ת��app
		}
		else
		{
			if (DEBUGFLAG)
				printf("No APP!\n");
			SendBuf[0] = MK_FAILED;
			UART2_Send_Date(SendBuf,sizeof(SendBuf));
			//��ԭAPP
			IAP_REST_APP();
			IAP_Load_App(FLASH_APP_ADDR);//ת��app			
		}
	}
}
