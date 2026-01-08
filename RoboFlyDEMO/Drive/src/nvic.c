/*******************************************************************************************
										    �� ��
    ����Ŀ�����������ѧϰʹ�ã�����������ֲ�޸ģ������뱣����������Ϣ����ֲ�����г�������
	
���ɹ�����BUG��������������κ����Ρ��������ã�

* ����汾��V1.01
* �������ڣ�2018-8-18
* �������ߣ���ŭ��С��
* ��Ȩ���У��������������Ϣ�������޹�˾
*******************************************************************************************/
#include "stm32f10x.h"

/*****************************************************************************
* ��  ����void NvicConfig(void)
* ��  �ܣ����ù����������жϵ����ȼ�
* ��  ������
* ����ֵ����
* ��  ע�������ȼ��жϲ�Ҫ������Ŷ
*****************************************************************************/
void NvicConfig(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitStruct.NVIC_IRQChannel=TIM4_IRQn;   //TIM4�ж�ͨ��
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=1;   //��ռ���ȼ�0
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=1;   //�����ȼ�1
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;   //ʹ��TIM4�ж�ͨ��
	NVIC_Init(&NVIC_InitStruct);   //�ж����ȼ���ʼ������
	
	NVIC_InitStruct.NVIC_IRQChannel=USART1_IRQn;  //USART1�ж�ͨ��
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=1;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel=EXTI2_IRQn;   //�����ⲿ�ж�ͨ��
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;   //������ռ���ȼ�Ϊ0
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;   //���������ȼ�Ϊ1
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;   //ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStruct);   //�ж����ȼ���ʼ������
}

