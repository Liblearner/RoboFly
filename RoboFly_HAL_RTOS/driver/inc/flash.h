#ifndef   _FLASH_H_
#define   _FLASH_H_

#include "stm32f1xx_hal.h"

// FLASH起始地址
#define STM32_FLASH_BASE 0x08000000     // STM32 FLASH的起始地址

#define FLASH_SAVE_ADDR  0x0801F800     // 设置FLASH 保存地址(必须为偶数,且其值要大于本代码所占用FLASH的大小+0X08000000
                                        // 否则,写操作的时候,可能会导致擦除整个扇区,从而引起部分程序丢失.跑飞.


// 函数声明
uint32_t STMFLASH_ReadWord(uint32_t faddr);                                           // 读取字
void STMFLASH_Write(uint32_t WriteAddr, uint32_t *pBuffer, uint32_t NumToWrite);          // 从指定地址开始写入指定长度的数据
void STMFLASH_Read(uint32_t ReadAddr, uint32_t *pBuffer, uint32_t NumToRead);             // 从指定地址开始读出指定长度的数据

// 获取页地址(STM32F103每页1KB)
uint32_t STMFLASH_GetFlashPage(uint32_t addr);

#endif
