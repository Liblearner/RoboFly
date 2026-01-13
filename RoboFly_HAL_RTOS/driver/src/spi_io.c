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
#include "spi_io.h"

extern SPI_HandleTypeDef hspi2;

/*****************************************************************************
 * 函  数:void SPI_GPIO_Init(void)
 * 功  能:配置SI24R1的 SCK、MISO、MOSI引脚,以及SPI2初始化
 * 参  数:无
 * 返回值:无
 * 备  注:调试SPI通信时一定要分清主机从机模式
 *         主机从机模式的 空闲状态 电平
 *		  2.4G模块通信时,SPI速率一般不大于10Mbps
 *****************************************************************************/
void SPI_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* 使能时钟 */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_SPI2_CLK_ENABLE();

	/* 配置SPI2引脚: SCK(PB13), MISO(PB14), MOSI(PB15) */
	GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_15;  // SCK和MOSI
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;           // 复用推挽输出
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;     // 高速
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_14;                // MISO
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;           // 浮空输入(HAL库中使用INPUT)
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* SPI2配置 */
	hspi2.Instance = SPI2;
	hspi2.Init.Mode = SPI_MODE_MASTER;                      // 主机模式
	hspi2.Init.Direction = SPI_DIRECTION_2LINES;            // 全双工
	hspi2.Init.DataSize = SPI_DATASIZE_8BIT;                // 8位数据
	hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;              // 时钟空闲状态为低电平
	hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;                  // 第一个时钟沿捕获
	hspi2.Init.NSS = SPI_NSS_SOFT;                          // 软件NSS管理
	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8; // 8分频 (36MHz/8=4.5MHz)
	hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;                 // 高位先行
	hspi2.Init.TIMode = SPI_TIMODE_DISABLE;                 // 禁用TI模式
	hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; // 禁用CRC
	hspi2.Init.CRCPolynomial = 7;                           // CRC多项式(虽然禁用了)

	if (HAL_SPI_Init(&hspi2) != HAL_OK)
	{
		/* 初始化错误处理 */
		// Error_Handler();
	}
}

/*****************************************************************************
 * 函  数:uint8_t SPI2_WriteReadByte(uint8_t data)
 * 功  能:SPI2读写一个字节
 * 参  数:data - 要发送的数据
 * 返回值:接收到的数据
 * 备  注:使用HAL库的阻塞式传输
 *****************************************************************************/
uint8_t SPI2_WriteReadByte(uint8_t data)
{
	uint8_t rxData = 0;
	
	/* HAL库的阻塞式收发函数 */
	HAL_SPI_TransmitReceive(&hspi2, &data, &rxData, 1, HAL_MAX_DELAY);
	
	return rxData;
}

/* 错误处理函数(如果main.c中没有定义,需要添加) */
// void Error_Handler(void)
// {
// 	/* 用户可以添加自己的错误处理代码 */
// 	__disable_irq();
// 	while (1)
// 	{
// 	}
// }
