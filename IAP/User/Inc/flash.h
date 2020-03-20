#ifndef _FLASH_H
#define _FLASH_H
#include "stm32f10x.h"

#define STM32_SECTOR_SIZE   2048    //ҳ��С 1page = 2k = 2048 byte
#define STM32_SECTOR_NUM    127     //ҳ������0-127ҳ     

//STM32 �ڲ�FLASH����ֹ��ַ
#define STM32_FLASH_BASE 0x08000000
#define STM32_FLASH_END 0x0803ffff

//STM32������ַ���� ��ʼҳ*ÿҳ��С = ��ַ
#define FLASH_APP_ADDR					0x08002000//APP����д���ַ
#define FLASH_DOWNLOAD_ADDR				0x08016000//���մ��ڸ������ݱ����ַ
#define FLASH_OLDAPP_ADDR 				0x0802a000//���ݾɵ�APP��ַ����������ʧ�����ݻ���
#define FLASH_UPDATA_FLAG				0x0803e000//���±�־λ��ַ

//STM32������ֹҳ
#define FLASH_APP_START_PAGE			4//APP����д����ʼҳ
#define FLASH_APP_END_PAGE			43

#define FLASH_DOWNLOAD_START_PAGE		44//DOWNLOAD����д����ʼҳ
#define FLASH_DOWNLOAD_END_PAGE		83

#define FLASH_BACKAPP_START_PAGE			84//OLDAPP����д����ʼҳ
#define FLASH_BACKAPP_END_PAGE				123

#define FLASH_UPDATA_FLAG_START_PAGE			124//UPDATA����д����ʼҳ
#define FLASH_UPDATA_FLAG_END_PAGE				125


uint8_t FLASH_ReadByte(uint32_t Addr);
uint16_t FLASH_ReadHalfWord(uint32_t Addr);
void FLASH_ReadNByte(uint32_t Addr,uint8_t *pBuff,uint32_t Len);
void FLASH_ReadPage(uint8_t Page_Num,uint8_t *pBuff);
uint8_t FLASH_WritePage(uint8_t Page_Num,uint8_t *pBuff);
uint8_t FLASH_WriteNData(uint32_t Addr,uint8_t *pBuff,uint32_t Len);
void FLASH_WriteNByte(uint32_t Addr,uint8_t *pBuff,uint32_t Len);
void Flash_EraseSector(uint8_t Start_Page,uint8_t End_Page);
void Flash_WriteFlag(uint32_t Addr,uint16_t buf);






#endif

