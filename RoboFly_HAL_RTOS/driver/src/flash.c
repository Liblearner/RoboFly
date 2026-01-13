/*******************************************************************************************
										    声 明
    本项目代码仅供个人学习使用,可以自由移植修改,但必须保留此声明信息。移植过程中出现其他
	
不可估量的BUG,天际智联不负任何责任。请勿商用!

* 程序版本:V1.01
* 程序日期:2018-8-18
* 程序作者:愤怒的小孩
* 版权所有:西安天际智联信息技术有限公司
* 移植说明:移植到STM32 HAL库
*******************************************************************************************/
#include "stm32f1xx_hal.h"
#include "flash.h"
#include "usart.h"

/******************************************************************************
* 函  数:uint32_t STMFLASH_ReadWord(uint32_t faddr)
* 功  能:读取指定地址的半字(32位数据) 
* 参  数:faddr:读地址 
* 返回值:对应数据
* 备  注:无
*******************************************************************************/
uint32_t STMFLASH_ReadWord(uint32_t faddr)
{
	return *(__IO uint32_t*)faddr; 
}

/******************************************************************************
* 函  数:uint32_t STMFLASH_GetFlashPage(uint32_t addr)
* 功  能:获取某个地址所在的flash页
* 参  数:addr:flash地址
* 返回值:页地址
* 备  注:STM32F103中等容量设备每页1KB(0x400字节)
*******************************************************************************/
uint32_t STMFLASH_GetFlashPage(uint32_t addr)
{
	uint32_t page = 0;
	
	if (addr < STM32_FLASH_BASE || addr >= (STM32_FLASH_BASE + 128 * 1024))
	{
		return 0xFFFFFFFF; // 非法地址
	}
	
	// 计算页地址 (每页1KB = 1024 = 0x400)
	page = (addr - STM32_FLASH_BASE) / FLASH_PAGE_SIZE;
	
	return page;
}

/******************************************************************************
* 函  数:void STMFLASH_Write(uint32_t WriteAddr, uint32_t *pBuffer, uint32_t NumToWrite)	
* 功  能:从指定地址开始写入指定长度的数据 
* 参  数:WriteAddr:起始地址(此地址必须为4的倍数!!)
*         pBuffer:数据指针
*         NumToWrite:字(32位)数(就是要写入的32位数据的个数.) 
* 返回值:无
* 备  注:STM32F1的Flash写入前必须先擦除,擦除后为0xFFFF...F
*******************************************************************************/
void STMFLASH_Write(uint32_t WriteAddr, uint32_t *pBuffer, uint32_t NumToWrite)	
{ 
	HAL_StatusTypeDef status = HAL_OK;
	uint32_t addrx = 0;
	uint32_t endaddr = 0;
	uint32_t PageError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;
	
	// 非法地址判断
	if (WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 128 * 1024)))
	{
		return;	// 非法地址
	}
	
	// 解锁Flash
	HAL_FLASH_Unlock();
	
	addrx = WriteAddr;				          // 写入的起始地址
	endaddr = WriteAddr + NumToWrite * 4;	  // 写入的结束地址
	
	// 只在主存储区,操作才有效.(防止擦除别的扇区的程序)
	if (addrx < 0x08020000)			       
	{
		// 扫描需要擦除的扇区
		while (addrx < endaddr)		       
		{
			// 如果不是0xFFFFFFFF,需要擦除这个扇区
			if (STMFLASH_ReadWord(addrx) != 0xFFFFFFFF)
			{   
				// 配置擦除参数
				EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;  // 页擦除
				EraseInitStruct.PageAddress = addrx;                // 擦除页的地址
				EraseInitStruct.NbPages = 1;                        // 一次擦除一页
				
				// 执行擦除
				status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
				
				if (status != HAL_OK)
				{
					break;	// 发生错误
				}
			}
			else 
			{
				addrx += 4;
			}
		} 
	}
	
	// 擦除成功后开始写入
	if (status == HAL_OK)
	{
		while (WriteAddr < endaddr)
		{
			// 写入数据(字编程,32位)
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, WriteAddr, *pBuffer) != HAL_OK)
			{ 
				break; // 写入异常
			}
			WriteAddr += 4;
			pBuffer++;
		} 
	}
	
	// 上锁Flash
	HAL_FLASH_Lock();
}

/******************************************************************************
* 函  数:void STMFLASH_Read(uint32_t ReadAddr, uint32_t *pBuffer, uint32_t NumToRead)  
* 功  能:从指定地址开始读出指定长度的数据
* 参  数:ReadAddr:起始地址
*         pBuffer:数据指针
*         NumToRead:字(4位)数
* 返回值:无
* 备  注:无
*******************************************************************************/
void STMFLASH_Read(uint32_t ReadAddr, uint32_t *pBuffer, uint32_t NumToRead)   	
{
	uint32_t i;
	for (i = 0; i < NumToRead; i++)
	{
		pBuffer[i] = STMFLASH_ReadWord(ReadAddr);  // 读取4个字节
		ReadAddr += 4;  // 偏移4个字节	
	}
}
