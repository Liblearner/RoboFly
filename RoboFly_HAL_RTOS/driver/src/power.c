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
#include "structconfig.h"
#include "power.h"
#include "si24r1.h"
#include "stdio.h"
#include "filter.h"

extern ADC_HandleTypeDef hadc1;

BATT_TYPE BAT =
{
	.BattAdc = 0,        // 电池电压采集ADC值
	.BattRealV = 3.31f,  // 实际测量的飞机供电电压 (注意此电压必须亲测否则测量的电压不准)
	.BattMeasureV = 0,   // 程序测量的实际电池电压
	.BattAlarmV = 3.2f,  // 电池低电压报警瞬时值 (这个值需要根据机身不同重量实测,实测380mh是2.8v)
	.BattFullV = 4.2f,   // 电池充满电值 4.2V
};
uint8_t BATT_LEDflag = 0;

/******************************************************************************************
* 函  数:void BATT_Init(void)
* 功  能:电压检测引脚初始化 以及ADC1初始化
* 参  数:无
* 返回值:无
* 备  注:无
*******************************************************************************************/
void BATT_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	ADC_ChannelConfTypeDef sConfig = {0};
	
	/* 使能时钟 */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_ADC1_CLK_ENABLE();
	
	/* 配置ADC时钟 (PCLK2/6) */
	/* 注意:HAL库中ADC时钟配置通常在SystemClock_Config()中完成 */
	/* 如果需要在这里配置,可以使用: */
	// RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
	// PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	// PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	// HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
	
	/* 配置GPIO引脚PA1为模拟输入 */
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/* 配置ADC1 */
	hadc1.Instance = ADC1;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;              // 非扫描模式
	hadc1.Init.ContinuousConvMode = DISABLE;                 // 单次转换模式
	hadc1.Init.DiscontinuousConvMode = DISABLE;              // 禁止不连续模式
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;        // 软件触发
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;              // 数据右对齐
	hadc1.Init.NbrOfConversion = 1;                          // 1个转换通道
	
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		// Error_Handler();
	}
	
	/* 配置ADC通道 */
	sConfig.Channel = ADC_CHANNEL_1;                         // 通道1
	sConfig.Rank = ADC_REGULAR_RANK_1;                       // 规则组序列1
	sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;       // 采样时间239.5周期
	
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		// Error_Handler();
	}
	
	/* ADC校准 */
	HAL_ADCEx_Calibration_Start(&hadc1);
}

/******************************************************************************************
* 函  数:uint16_t Get_BatteryAdc(uint8_t ch)
* 功  能:获取电池采样点电压的ADC值
* 参  数:ch  ADC采样通道
* 返回值:返回通道AD值
* 备  注:电池电压采样点的ADC值,电池电压采样电路见原理图
*******************************************************************************************/
uint16_t Get_BatteryAdc(uint8_t ch)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	
	/* 配置要转换的通道 */
	sConfig.Channel = ch;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		// Error_Handler();
	}
	
	/* 启动ADC转换 */
	HAL_ADC_Start(&hadc1);
	
	/* 等待转换完成 */
	if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK)
	{
		/* 读取转换结果 */
		return (uint16_t)HAL_ADC_GetValue(&hadc1);
	}
	
	return 0;
}

/******************************************************************************************
* 函  数:void BATT_GetVoltage(void)
* 功  能:获取电池电压
* 参  数:无
* 返回值:无
* 备  注:电池电压 = ADC检测电压*2 具体原理可看原理图
*******************************************************************************************/
void BATT_GetVoltage(void)
{
	float V;
	Aver_Filter((float)Get_BatteryAdc(ADC_CHANNEL_1), &BAT.BattAdc, 6); // 滑动滤波电压值,提高精度
	if(BAT.BattAdc)
		V = BAT.BattAdc * BAT.BattRealV / 4095.0f;
	BAT.BattMeasureV = 2 * V; // 根据原理电阻分压,可知 电池实际电压 = ADC侧量电压 * 2
	// printf("Test Voltage :%0.2f   temp:%0.0f \r\n ", BAT.BattMeasureV, BAT.BattAdc);
}

/******************************************************************************************
* 函  数:void LowVoltage_Alarm(void)
* 功  能:低电量报警
* 参  数:无
* 返回值:无
* 备  注:无
*******************************************************************************************/
void LowVoltage_Alarm(void)
{
	static uint8_t cnt = 0, cnt1 = 0;
	BATT_GetVoltage();
	if(Airplane_Enable)
	{
		if(BAT.BattMeasureV < BAT.BattAlarmV) // 飞行时测量
		{
			if(cnt1++ > 10)
			{
				cnt1 = 0;
				BATT_LEDflag = 1;
			}
		}
		else
		{
			cnt1 = 0;
			BATT_LEDflag = 0;
		}
	}
	else
	{
		if(BAT.BattMeasureV < 3.7f) // 落地时测量(380mh时是3.5V)
		{
			if(cnt++ > 10)
			{
				Run_flag = 0;
				cnt = 0;
				BATT_LEDflag = 1;
			}
		}
		else
		{
			Run_flag = 1;
			cnt = 0;
			BATT_LEDflag = 0;
		}
	}
}

// /* 错误处理函数 */
// void Error_Handler(void)
// {
// 	__disable_irq();
// 	while (1)
// 	{
// 	}
// }
