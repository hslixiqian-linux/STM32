#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "flash.h"

static uint8_t STM32_FLASH_BUFF[STM32_SECTOR_SIZE] = {0};



/*-------------------------------------------------------------------------------
���ܣ���1���ֽ�����
������1.��ַ
����ֵ����������ʵ����ֵ
---------------------------------------------------------------------------------*/
uint8_t FLASH_ReadByte(uint32_t Addr)
{
    return *(vu8 *)Addr;
}



/*-------------------------------------------------------------------------------
���ܣ���2���ֽ�����,���ڶ�ȡ��־λ
������1.��ַ
����ֵ����������ʵ����ֵ
---------------------------------------------------------------------------------*/
uint16_t FLASH_ReadHalfWord(uint32_t Addr)
{
    return *(vu16 *)Addr;
}


/*-------------------------------------------------------------------------------
���ܣ���N���ֽ�����
������1.��ַ 2.���ݻ����� 3.�����ݵĳ���
����ֵ��void
---------------------------------------------------------------------------------*/
void FLASH_ReadNByte(uint32_t Addr,uint8_t *pBuff,uint32_t Len)
{
    uint32_t i;
    
    for(i = 0;i < Len;i++)
    {
        pBuff[i] = FLASH_ReadByte(Addr);
        Addr += 1;
    }
}


uint32_t FLASH_ReadWord(uint32_t Addr)
{
    return *(vu32 *)Addr;
}


/*-------------------------------------------------------------------------------
���ܣ�������ҳ��������
������1.������ҳ�� 2.���ݻ�����
����ֵ��void
---------------------------------------------------------------------------------*/
void FLASH_ReadPage(uint8_t Page_Num,uint8_t *pBuff)
{
    uint16_t i;
    uint32_t Buff;
    uint32_t Addr;
    
    //�Ƿ񳬳���Χ
    if(Page_Num > STM32_SECTOR_NUM)
        return;
    //�ȼ���ҳ�׵�ַ
    Addr = Page_Num * STM32_SECTOR_SIZE + STM32_FLASH_BASE;
    
    for(i = 0;i < STM32_SECTOR_SIZE;i += 4)
    {
        Buff = FLASH_ReadWord(Addr); /*********/
        //printf("%x",Buff);
        pBuff[i]   = Buff;
        pBuff[i+1] = Buff >> 8;
        pBuff[i+2] = Buff >> 16;
        pBuff[i+3] = Buff >> 24;
        
        Addr += 4;
    }
}


/*-------------------------------------------------------------------------------
���ܣ�������ҳд��
������1.д���ҳ�� 2.д�������
����ֵ��void
---------------------------------------------------------------------------------*/
u8 FLASH_WritePage(uint8_t Page_Num,uint8_t *pBuff)
{
	u8 ret = 0;
	uint16_t i;
    uint16_t Buff;
    uint32_t Addr;
    
    //�Ƿ񳬳���Χ
    if(Page_Num > STM32_SECTOR_NUM)
        return;
    //����
    FLASH_Unlock();
    //�ȼ���ҳ�׵�ַ
    Addr = Page_Num * STM32_SECTOR_SIZE + STM32_FLASH_BASE;
    for(i = 0;i < STM32_SECTOR_SIZE ;i += 2)
    {
        Buff = ((uint16_t)pBuff[i+1] << 8) | pBuff[i];
        ret = FLASH_ProgramHalfWord(Addr,Buff);
		if (ret != 4)
		{
			printf("write erro ret:%d",ret);
			return ret;
		}
        Addr += 2;
    }
    //����
    FLASH_Lock();
	return ret;
}


/*-------------------------------------------------------------------------------
���ܣ�����ַд�뺯��
������1.д���ַ 2.д������� 3.���ݵĳ���
����ֵ��-1:�ռ䲻�� 0��д��ɹ�
---------------------------------------------------------------------------------*/
s8 FLASH_WriteNData(uint32_t Addr,uint8_t *pBuff,uint32_t Len)
{
    uint32_t Offset;
    uint8_t  Page_Num;
    uint16_t Page_Offset;
    uint16_t Free_Space;
    uint16_t i;
    
    if((Addr < STM32_FLASH_BASE) || (Addr > STM32_FLASH_END))
        return -1;
    
    Offset = Addr - STM32_FLASH_BASE;//ƫ�Ƶ�ַ
    Page_Num = Offset / STM32_SECTOR_SIZE;//�õ���ַ����ҳ
    Page_Offset = Offset % STM32_SECTOR_SIZE;//��ҳ�ڵ�ƫ�Ƶ�ַ
    Free_Space = STM32_SECTOR_SIZE -  Page_Offset;//ҳ��ʣ��ռ�
    //Ҫд��������Ƿ����ʣ��ռ�
    if(Len <= Free_Space)
        Free_Space = Len;
    FLASH_Unlock();//����
    
    while(1)
    {
        FLASH_ReadPage(Page_Num,STM32_FLASH_BUFF);//�Ȱ����ݶ���������
        FLASH_ErasePage(Page_Num * STM32_SECTOR_SIZE + STM32_FLASH_BASE);//ҳ����
        //�޸Ļ�������
        for(i = 0;i < Free_Space;i++)
        {
            STM32_FLASH_BUFF[i+Page_Offset] = pBuff[i];
        }
        FLASH_WritePage(Page_Num,STM32_FLASH_BUFF);//�ѻ�������д��
        //�ж��Ƿ񳬳���ǰҳ������������һҳ
        if(Len == Free_Space)
            break;
        else
        {
            Page_Num++;//��һҳ
            Page_Offset = 0;
            pBuff += Free_Space;
            
            Len -= Free_Space;
            if(Len > STM32_SECTOR_SIZE)
                Free_Space = STM32_SECTOR_SIZE;
            else
                Free_Space = Len;
        }
    }
    FLASH_Lock();
	return 0;
}


/*-------------------------------------------------------------------------------
���ܣ�����ַдN�ֽ�����
������1.д���ַ 2.д������� 3.���ݵĳ���
����ֵ��void
---------------------------------------------------------------------------------*/
void FLASH_WriteNByte(uint32_t Addr,uint8_t *pBuff,uint32_t Len)
{
    uint16_t i;
    uint16_t temp = 0;
    u8 ret = 0;
    if((Addr < STM32_FLASH_BASE) || (Addr > STM32_FLASH_END))
    {
    	printf("Date>Flash sieze! Addr:0x%x\n",Addr);
        return;
    }
    
    FLASH_Unlock();//����
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(Addr);
    for(i = 0;i < Len;i += 2)
    {
        temp = pBuff[i];
		//printf("0x%02x 0x%02x ",pBuff[i],pBuff[i+1]);
        temp |= (uint16_t)pBuff[i+1] << 8;
        ret = FLASH_ProgramHalfWord(Addr,temp);
		if (ret != 4)
			printf("ret:%d\n",ret);
        Addr += 2;
        if(Addr > STM32_FLASH_END)
        {
            FLASH_Lock();
            return;
        }
    }
    FLASH_Lock();
}



/*-------------------------------------------------------------------------------
���ܣ���ҳ����
������1.��ʼҳ 2.����ҳ
����ֵ��void
---------------------------------------------------------------------------------*/
void Flash_EraseSector(uint8_t Start_Page,uint8_t End_Page)
{
    uint8_t i;
    uint8_t num = 0;
    
    if(Start_Page > End_Page)
        return;
    
    FLASH_Unlock();//����
    
    num = End_Page - Start_Page;//����ҳ��
    
    for(i = 0;i <= num;i++)
    {
        FLASH_ErasePage((Start_Page + i) * STM32_SECTOR_SIZE + STM32_FLASH_BASE);//ҳ����
    }
    
    FLASH_Lock();
}

void Flash_WriteFlag(uint32_t Addr,uint16_t buf)
{
	u8 ret = 0;
	FLASH_Unlock();//??
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(Addr);
	ret = FLASH_ProgramHalfWord(Addr,buf);
	if (ret != 4)
		printf("Write flag Err ret:%d\n",ret);
	FLASH_Lock();
}

