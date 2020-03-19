#ifndef _FLASH_H
#define _FLASH_H
#include "stm32f10x.h"
void IAP_WriteBin(uint32_t Addr,uint8_t *pBuff,uint32_t Len);
void IAP_Load_App(uint32_t Addr);
u8 IAP_Copy_App(void);
void IAP_BACK_APP(void);


#endif


