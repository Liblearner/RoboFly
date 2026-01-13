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
#include "motor.h"

#define Moto_PwmMax 1000

int16_t MOTO1_PWM = 0;
int16_t MOTO2_PWM = 0;
int16_t MOTO3_PWM = 0;
int16_t MOTO4_PWM = 0;

extern TIM_HandleTypeDef htim3;

/******************************************************************************************
* 函  数:void MOTOR_Init(void)
* 功  能:配置电机引脚初始化 以及TIM3 定时器输出PWM
* 参  数:无
* 返回值:无
* 备  注:TIM3 CH1(PWM1) -> PA6
*         TIM3 CH2(PWM2) -> PA7
*         TIM3 CH3(PWM3) -> PB0
*         TIM3 CH4(PWM4) -> PB1
*******************************************************************************************/
void MOTOR_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	
	/* 使能时钟 */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_TIM3_CLK_ENABLE();
	
	/* 配置GPIO引脚 PA6和PA7 */
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;           // 复用推挽输出
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;     // 高速
	GPIO_InitStruct.Pull = GPIO_NOPULL;               // 无上下拉
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/* 配置GPIO引脚 PB0和PB1 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;           // 复用推挽输出
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;     // 高速
	GPIO_InitStruct.Pull = GPIO_NOPULL;               // 无上下拉
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	/* TIM3部分重映射配置 (如果需要) */
	/* STM32F103默认TIM3_CH1/2在PA6/PA7, TIM3_CH3/4在PB0/PB1,无需重映射 */
	
	/* 配置TIM3基本参数 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 100;                       // 预分频器值为100 (实际分频系数101)
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;      // 向上计数模式
	htim3.Init.Period = 1000 - 1;                     // 自动重装载值999
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;// 时钟分频
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; // 使能自动重装载预装载
	
	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
	{
		// Error_Handler();
	}
	
	/* 配置定时器主输出 */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		// Error_Handler();
	}
	
	/* 配置PWM通道参数 (4个通道使用相同配置) */
	sConfigOC.OCMode = TIM_OCMODE_PWM1;               // PWM模式1
	sConfigOC.Pulse = 0;                              // 初始占空比为0
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;       // 输出极性为高
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;        // 快速模式禁用
	
	/* 配置通道1 */
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		// Error_Handler();
	}
	
	/* 配置通道2 */
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
	{
		// Error_Handler();
	}
	
	/* 配置通道3 */
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
	{
		// Error_Handler();
	}
	
	/* 配置通道4 */
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
	{
		// Error_Handler();
	}
	
	/* 启动PWM输出 */
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}

/************************************************************************************************
* 函  数:void Moto_Pwm(int16_t MOTO1_PWM, int16_t MOTO2_PWM, int16_t MOTO3_PWM, int16_t MOTO4_PWM)
* 功  能:依次对四个电机赋值各自的PWM占空比数值
* 参  数:MOTO1_PWM 电机1
*         MOTO2_PWM 电机2
*         MOTO3_PWM 电机3
*         MOTO4_PWM 电机4
* 返回值:无 
* 备  注:PWM值范围0-1000
************************************************************************************************/
void Moto_Pwm(int16_t MOTO1_PWM, int16_t MOTO2_PWM, int16_t MOTO3_PWM, int16_t MOTO4_PWM)
{		
	/* 限幅处理 */
	if(MOTO1_PWM > Moto_PwmMax) MOTO1_PWM = Moto_PwmMax;
	if(MOTO2_PWM > Moto_PwmMax) MOTO2_PWM = Moto_PwmMax;
	if(MOTO3_PWM > Moto_PwmMax) MOTO3_PWM = Moto_PwmMax;
	if(MOTO4_PWM > Moto_PwmMax) MOTO4_PWM = Moto_PwmMax;
	if(MOTO1_PWM < 0) MOTO1_PWM = 0;
	if(MOTO2_PWM < 0) MOTO2_PWM = 0;
	if(MOTO3_PWM < 0) MOTO3_PWM = 0;
	if(MOTO4_PWM < 0) MOTO4_PWM = 0;
	
	/* 设置各通道的PWM占空比 */
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, MOTO1_PWM);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, MOTO2_PWM);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, MOTO3_PWM);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, MOTO4_PWM);
}

// /* 错误处理函数 (如果main.c中没有定义,需要添加) */
// void Error_Handler(void)
// {
// 	__disable_irq();
// 	while (1)
// 	{
// 	}
// }
