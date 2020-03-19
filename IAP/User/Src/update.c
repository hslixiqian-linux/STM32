#include "stm32f10x.h"
#include "flash.h"

static uint8_t STM32_FLASH_BUFF[STM32_SECTOR_SIZE] = {0};

typedef void (*IAP_Fun)(void);
IAP_Fun JumpApp;



/*-------------------------------------------------------------------------------
功能：从BACK分区读出数据写入到APP分区（旧程序还原）
参数：void
返回值：void
---------------------------------------------------------------------------------*/
void IAP_REST_APP(void)
{
	uint8_t i;
	printf("REST APP!\n");
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	Flash_EraseSector(FLASH_APP_START_PAGE,FLASH_APP_END_PAGE);
	memset(STM32_FLASH_BUFF,0,sizeof(STM32_FLASH_BUFF));
	for (i=0;i<40;i++)
	{
		FLASH_ReadPage(FLASH_BACKAPP_START_PAGE + i,STM32_FLASH_BUFF);
		FLASH_WritePage(FLASH_APP_START_PAGE + i,STM32_FLASH_BUFF); //stm32一次写入必须是2k 1pag
	}
}


/*-------------------------------------------------------------------------------
功能：从APP分区读出数据写入到BACK分区（旧程序备份）
参数：void
返回值：void
---------------------------------------------------------------------------------*/
void IAP_BACK_APP(void)
{
	uint8_t i;
	printf("BACK APP!\n");
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	Flash_EraseSector(FLASH_BACKAPP_START_PAGE,FLASH_BACKAPP_END_PAGE);
	memset(STM32_FLASH_BUFF,0,sizeof(STM32_FLASH_BUFF));
	for (i=0;i<40;i++)
	{
		FLASH_ReadPage(FLASH_APP_START_PAGE + i,STM32_FLASH_BUFF);
		FLASH_WritePage(FLASH_BACKAPP_START_PAGE + i,STM32_FLASH_BUFF); //stm32一次写入必须是2k 1pag
	}
}

/*-------------------------------------------------------------------------------
功能：从Download分区读出数据保存写入到APP分区（新程序更新）
参数：void
返回值：void
---------------------------------------------------------------------------------*/
u8 IAP_Copy_App(void)
{
    uint8_t i;
	uint8_t ret=0;
    //uint8_t buf[2] = {0x00,0x00};
    //擦除App扇区
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    Flash_EraseSector(FLASH_APP_START_PAGE,FLASH_APP_END_PAGE);
    for(i = 0;i < 40;i++) //数据所占页数
    {
		//按从DOWNLOAD分区也中读出数据
		FLASH_ReadPage(FLASH_DOWNLOAD_START_PAGE + i,STM32_FLASH_BUFF);
		//将读出的数据写入到APP分区中
        ret = FLASH_WritePage(FLASH_APP_START_PAGE + i,STM32_FLASH_BUFF);
		if (ret != 4)
		{
			return ret;
		}
    }
    return ret;
    //FLASH_WriteNData(FLASH_UPDATA_FLAG,buf,2);
}

/*-------------------------------------------------------------------------------
功能：跳转到APP分区
参数：1.程序地址
返回值：void
---------------------------------------------------------------------------------*/
void IAP_Load_App(uint32_t Addr)
{
    //检查栈顶地址是否合法
    if(((*(vu32*)Addr) & 0x2FFE0000) == 0x20000000)
    {
		printf("Load_APP sucess!\n");
		__disable_irq(); //必须关闭所有中断！！！否则程序会跑飞
        JumpApp = (IAP_Fun)*(vu32 *)(Addr + 4);
        //MSR_MSP(*(vu32 *)Addr);
        __set_MSP(*(vu32*) Addr); //设置栈顶
        JumpApp();
    }
}

/*-------------------------------------------------------------------------------
功能：将bin包数据写入Download分区（数据接收）
参数：1.Download分区地址 2.串口接收到的数据 3.写入数据的长度
返回值：void
---------------------------------------------------------------------------------*/

void IAP_WriteBin(uint32_t Addr,uint8_t *pBuff,uint32_t Len)
{
	static u8 flag = 1;
	//uint8_t buf[2] = {0x55,0xAA};
    //擦除DOWNLOAD扇区
    //Flash_EraseSector(FLASH_APP_START_PAGE,FLASH_APP_END_PAGE); //一边擦一边写，全部擦写入失败
    //更新标记
    //FLASH_WriteNData(FLASH_UPDATA_FLAG,buf,2);
    //写入程序
    FLASH_WriteNByte(Addr,pBuff,Len);
    //复位单片机
    //NVIC_SystemReset();
}


