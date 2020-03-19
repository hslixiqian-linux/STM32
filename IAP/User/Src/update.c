#include "stm32f10x.h"
#include "flash.h"

static uint8_t STM32_FLASH_BUFF[STM32_SECTOR_SIZE] = {0};

typedef void (*IAP_Fun)(void);
IAP_Fun JumpApp;



/*-------------------------------------------------------------------------------
���ܣ���BACK������������д�뵽APP�������ɳ���ԭ��
������void
����ֵ��void
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
		FLASH_WritePage(FLASH_APP_START_PAGE + i,STM32_FLASH_BUFF); //stm32һ��д�������2k 1pag
	}
}


/*-------------------------------------------------------------------------------
���ܣ���APP������������д�뵽BACK�������ɳ��򱸷ݣ�
������void
����ֵ��void
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
		FLASH_WritePage(FLASH_BACKAPP_START_PAGE + i,STM32_FLASH_BUFF); //stm32һ��д�������2k 1pag
	}
}

/*-------------------------------------------------------------------------------
���ܣ���Download�����������ݱ���д�뵽APP�������³�����£�
������void
����ֵ��void
---------------------------------------------------------------------------------*/
u8 IAP_Copy_App(void)
{
    uint8_t i;
	uint8_t ret=0;
    //uint8_t buf[2] = {0x00,0x00};
    //����App����
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    Flash_EraseSector(FLASH_APP_START_PAGE,FLASH_APP_END_PAGE);
    for(i = 0;i < 40;i++) //������ռҳ��
    {
		//����DOWNLOAD����Ҳ�ж�������
		FLASH_ReadPage(FLASH_DOWNLOAD_START_PAGE + i,STM32_FLASH_BUFF);
		//������������д�뵽APP������
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
���ܣ���ת��APP����
������1.�����ַ
����ֵ��void
---------------------------------------------------------------------------------*/
void IAP_Load_App(uint32_t Addr)
{
    //���ջ����ַ�Ƿ�Ϸ�
    if(((*(vu32*)Addr) & 0x2FFE0000) == 0x20000000)
    {
		printf("Load_APP sucess!\n");
		__disable_irq(); //����ر������жϣ��������������ܷ�
        JumpApp = (IAP_Fun)*(vu32 *)(Addr + 4);
        //MSR_MSP(*(vu32 *)Addr);
        __set_MSP(*(vu32*) Addr); //����ջ��
        JumpApp();
    }
}

/*-------------------------------------------------------------------------------
���ܣ���bin������д��Download���������ݽ��գ�
������1.Download������ַ 2.���ڽ��յ������� 3.д�����ݵĳ���
����ֵ��void
---------------------------------------------------------------------------------*/

void IAP_WriteBin(uint32_t Addr,uint8_t *pBuff,uint32_t Len)
{
	static u8 flag = 1;
	//uint8_t buf[2] = {0x55,0xAA};
    //����DOWNLOAD����
    //Flash_EraseSector(FLASH_APP_START_PAGE,FLASH_APP_END_PAGE); //һ�߲�һ��д��ȫ����д��ʧ��
    //���±��
    //FLASH_WriteNData(FLASH_UPDATA_FLAG,buf,2);
    //д�����
    FLASH_WriteNByte(Addr,pBuff,Len);
    //��λ��Ƭ��
    //NVIC_SystemReset();
}


