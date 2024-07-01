/*******************************************************************************************
											声 明
	本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现其他

不可估量的BUG，天际智联不负任何责任。请勿商用！

* 程序版本：V1.01
* 程序日期：2018-8-18
* 程序作者：愤怒的小孩
* 版权所有：西安天际智联信息技术有限公司
*******************************************************************************************/
#include "delay.h"
#include "stm32f10x.h"

static uint8_t D_us = 0;  // 微秒系数
static uint16_t D_ms = 0; // 毫秒系数

/****************************************************************************************************
 * 函  数: void Delay_Init(void)
 * 功  能: 延时函数初始化
 * 参  数: 无
 * 返回值：无
 * 备  注: T(s) = 1/F(Hz) //赫兹时间转换公式
 ****************************************************************************************************/
void Delay_Init(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

	D_us = SystemCoreClock / 8000000;
	D_ms = (uint16_t)D_us * 1000;
}

/****************************************************************************************************
 * 函  数: void Delay_us(uint32_t nus)
 * 功  能: 微秒级延时
 * 参  数: nus 传入微秒参数
 * 返回值：无
 * 备  注: LOAD为24位寄存器，nus 的最大传入值为 2的24次方(0xFFFFFF)/ D_us(9) = 1864135 us
 ****************************************************************************************************/
void Delay_us(uint32_t nus)
{
	uint32_t temp;
	SysTick->CTRL = 0x00;		// 关闭SysTick定时器
	SysTick->LOAD = nus * D_us; // 延时重装载值
	SysTick->VAL = 0x00;		// 清空计数器
	SysTick->CTRL |= 0x01;		// 启动SysTick定时器
	do
	{
		temp = SysTick->CTRL;
	} while ((temp & 0x01) && !(temp & (1 << 16))); // 等待延时结束
	SysTick->CTRL = 0x00; // 关闭SysTick定时器
	SysTick->VAL = 0X00;  // 清空计数器
}

/****************************************************************************************************
 * 函  数: void Delay_ms(uint32_t nms)
 * 功  能: 毫秒级延时
 * 参  数: nms 传入微秒参数
 * 返回值：无
 * 备  注: LOAD为24位寄存器，nms 的最大传入值为 2的24次方(0xFFFFFF)/ D_ms(9) = 1864 ms
 ****************************************************************************************************/
void Delay_ms(uint32_t nms)
{
	uint32_t temp;
	SysTick->CTRL = 0x00;		// 关闭SysTick定时器
	SysTick->LOAD = nms * D_ms; // 延时重装载值
	SysTick->VAL = 0x00;		// 清空计数器
	SysTick->CTRL |= 0x01;		// 启动SysTick定时器
	do
	{
		temp = SysTick->CTRL;
	} while ((temp & 0x01) && !(temp & (1 << 16))); // 等待延时结束
	SysTick->CTRL = 0x00; // 关闭SysTick定时器
	SysTick->VAL = 0X00;  // 清空计数器
}
