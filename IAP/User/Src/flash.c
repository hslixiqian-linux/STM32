#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "flash.h"

static uint8_t STM32_FLASH_BUFF[STM32_SECTOR_SIZE] = {0};



/*-------------------------------------------------------------------------------
功能：读1个字节数据
参数：1.地址
返回值：读出的真实数据值
---------------------------------------------------------------------------------*/
uint8_t FLASH_ReadByte(uint32_t Addr)
{
    return *(vu8 *)Addr;
}



/*-------------------------------------------------------------------------------
功能：读2个字节数据,用于读取标志位
参数：1.地址
返回值：读出的真实数据值
---------------------------------------------------------------------------------*/
uint16_t FLASH_ReadHalfWord(uint32_t Addr)
{
    return *(vu16 *)Addr;
}


/*-------------------------------------------------------------------------------
功能：读N个字节数据
参数：1.地址 2.数据缓存区 3.读数据的长度
返回值：void
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
功能：按分区页读出数据
参数：1.读出的页数 2.数据缓存区
返回值：void
---------------------------------------------------------------------------------*/
void FLASH_ReadPage(uint8_t Page_Num,uint8_t *pBuff)
{
    uint16_t i;
    uint32_t Buff;
    uint32_t Addr;
    
    //是否超出范围
    if(Page_Num > STM32_SECTOR_NUM)
        return;
    //先计算页首地址
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
功能：按分区页写入
参数：1.写入的页数 2.写入的数据
返回值：void
---------------------------------------------------------------------------------*/
u8 FLASH_WritePage(uint8_t Page_Num,uint8_t *pBuff)
{
	u8 ret = 0;
	uint16_t i;
    uint16_t Buff;
    uint32_t Addr;
    
    //是否超出范围
    if(Page_Num > STM32_SECTOR_NUM)
        return;
    //解锁
    FLASH_Unlock();
    //先计算页首地址
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
    //上锁
    FLASH_Lock();
	return ret;
}


/*-------------------------------------------------------------------------------
功能：按地址写入函数
参数：1.写入地址 2.写入的数据 3.数据的长度
返回值：-1:空间不够 0：写入成功
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
    
    Offset = Addr - STM32_FLASH_BASE;//偏移地址
    Page_Num = Offset / STM32_SECTOR_SIZE;//得到地址所在页
    Page_Offset = Offset % STM32_SECTOR_SIZE;//在页内的偏移地址
    Free_Space = STM32_SECTOR_SIZE -  Page_Offset;//页区剩余空间
    //要写入的数据是否大于剩余空间
    if(Len <= Free_Space)
        Free_Space = Len;
    FLASH_Unlock();//解锁
    
    while(1)
    {
        FLASH_ReadPage(Page_Num,STM32_FLASH_BUFF);//先把数据读到缓存中
        FLASH_ErasePage(Page_Num * STM32_SECTOR_SIZE + STM32_FLASH_BASE);//页擦除
        //修改缓存数据
        for(i = 0;i < Free_Space;i++)
        {
            STM32_FLASH_BUFF[i+Page_Offset] = pBuff[i];
        }
        FLASH_WritePage(Page_Num,STM32_FLASH_BUFF);//把缓存数据写入
        //判断是否超出当前页，超出进入下一页
        if(Len == Free_Space)
            break;
        else
        {
            Page_Num++;//下一页
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
功能：按地址写N字节数据
参数：1.写入地址 2.写入的数据 3.数据的长度
返回值：void
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
    
    FLASH_Unlock();//解锁
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
功能：按页擦除
参数：1.起始页 2.结束页
返回值：void
---------------------------------------------------------------------------------*/
void Flash_EraseSector(uint8_t Start_Page,uint8_t End_Page)
{
    uint8_t i;
    uint8_t num = 0;
    
    if(Start_Page > End_Page)
        return;
    
    FLASH_Unlock();//解锁
    
    num = End_Page - Start_Page;//擦除页数
    
    for(i = 0;i <= num;i++)
    {
        FLASH_ErasePage((Start_Page + i) * STM32_SECTOR_SIZE + STM32_FLASH_BASE);//页擦除
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

