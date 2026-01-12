/*******************************************************************************************
										    �� ��
    ����Ŀ�����������ѧϰʹ�ã�����������ֲ�޸ģ������뱣����������Ϣ����ֲ�����г�������
	
���ɹ�����BUG��������������κ����Ρ��������ã�

* ����汾��V1.01
* �������ڣ�2018-8-18
* �������ߣ���ŭ��С��
* ��Ȩ���У��������������Ϣ�������޹�˾
*******************************************************************************************/
#include "stm32f1xx_hal.h"


#define Moto_PwmMax 1000
int16_t MOTO1_PWM = 0;
int16_t MOTO2_PWM = 0;
int16_t MOTO3_PWM = 0;
int16_t MOTO4_PWM = 0;


/******************************************************************************************
* ��  ����void MOTOR_Init(void)
* ��  �ܣ�������ų�ʼ�� �Լ�TIM3 �������PWM
* ��  ������
* ����ֵ����
* ��  ע��TIM3 CH1(PWM1) -> PA6
*         TIM3 CH2(PWM2) -> PA7
*         TIM3 CH3(PWM3) -> PB0
*         TIM3 CH4(PWM4) -> PB1
*******************************************************************************************/
void MOTOR_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;   //����GPIO�ṹ�����
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;   //���嶨ʱ���ṹ�����
	TIM_OCInitTypeDef TIM_OCInitStruct;   //��������ȽϽṹ�����
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);   //GPIOA��B������ʱ��ʹ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);   //ʹ��TIM3��ʱ��
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6|GPIO_Pin_7;   //����GPIO��6��7����
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;   	  //����GPIOΪ�����������
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;  	  //����GPIO����
	GPIO_Init(GPIOA,&GPIO_InitStruct);   //GPIO��ʼ������
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1;   //����GPIO��0��1����
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;   	  //����GPIOΪ�����������
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;   	  //����GPIO����
	GPIO_Init(GPIOB,&GPIO_InitStruct);   //GPIO��ʼ������
	
	TIM_TimeBaseInitStruct.TIM_Period=1000-1;   	  //�����Զ���װ�ص�����ֵ
	TIM_TimeBaseInitStruct.TIM_Prescaler=100;   	  //����Ԥ��Ƶֵ
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;     //����ʱ�ӷָ�
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);   		   //��ʱ����ʼ������
	
	TIM_OCInitStruct.TIM_OCMode=TIM_OCMode_PWM1;   //PWMģʽ1
	TIM_OCInitStruct.TIM_Pulse=0;   						   //��ʼ��ռ�ձ�Ϊ0
	TIM_OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High;   	   //����Ƚϼ��Ե�
	TIM_OCInitStruct.TIM_OutputState=TIM_OutputState_Enable;   //�Ƚ����ʹ��
	TIM_OC1Init(TIM3,&TIM_OCInitStruct);   					   //�Ƚ������ʼ������
	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable);   //����Ƚ�1Ԥװ�ؼĴ���ʹ��
	
	TIM_OCInitStruct.TIM_OCMode=TIM_OCMode_PWM1;   //PWMģʽ1
	TIM_OCInitStruct.TIM_Pulse=0;   					   		//��ʼ��ռ�ձ�Ϊ0
	TIM_OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High;   		//����Ƚϼ��Ե�
	TIM_OCInitStruct.TIM_OutputState=TIM_OutputState_Enable;   	//�Ƚ����ʹ��
	TIM_OC2Init(TIM3,&TIM_OCInitStruct);   						//�Ƚ������ʼ������
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);   //����Ƚ�2Ԥװ�ؼĴ���ʹ��
	
	TIM_OCInitStruct.TIM_OCMode=TIM_OCMode_PWM1;   //PWMģʽ1
	TIM_OCInitStruct.TIM_Pulse=0;   							//��ʼ��ռ�ձ�Ϊ0
	TIM_OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High;   		//����Ƚϼ��Ե�
	TIM_OCInitStruct.TIM_OutputState=TIM_OutputState_Enable;   	//�Ƚ����ʹ��
	TIM_OC3Init(TIM3,&TIM_OCInitStruct);   						//�Ƚ������ʼ������
	TIM_OC3PreloadConfig(TIM3,TIM_OCPreload_Enable);   //����Ƚ�3Ԥװ�ؼĴ���ʹ��
	
	TIM_OCInitStruct.TIM_OCMode=TIM_OCMode_PWM1;   //PWMģʽ1
	TIM_OCInitStruct.TIM_Pulse=0;   							//��ʼ��ռ�ձ�Ϊ0
	TIM_OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High;   		//����Ƚϼ��Ե�
	TIM_OCInitStruct.TIM_OutputState=TIM_OutputState_Enable;   	//�Ƚ����ʹ��
	TIM_OC4Init(TIM3,&TIM_OCInitStruct);   						//�Ƚ������ʼ������
	TIM_OC4PreloadConfig(TIM3,TIM_OCPreload_Enable);   //����Ƚ�4Ԥװ�ؼĴ���ʹ��
	
	TIM_Cmd(TIM3,ENABLE);   //TIM3ʹ��
}

/************************************************************************************************
* ��  ����void Moto_Pwm(int16_t MOTO1_PWM,int16_t MOTO2_PWM,int16_t MOTO3_PWM,int16_t MOTO4_PWM)
* ��  �ܣ����Ҫ�����ֵת����PWM�������
* ��  ����MOTO1_PWM ���1
*         MOTO2_PWM ���2
*         MOTO3_PWM ���3
*         MOTO3_PWM ���4
* ����ֵ���� 
* ��  ע����
************************************************************************************************/
void Moto_Pwm(int16_t MOTO1_PWM,int16_t MOTO2_PWM,int16_t MOTO3_PWM,int16_t MOTO4_PWM)
{		
	if(MOTO1_PWM>Moto_PwmMax)	MOTO1_PWM = Moto_PwmMax;
	if(MOTO2_PWM>Moto_PwmMax)	MOTO2_PWM = Moto_PwmMax;
	if(MOTO3_PWM>Moto_PwmMax)	MOTO3_PWM = Moto_PwmMax;
	if(MOTO4_PWM>Moto_PwmMax)	MOTO4_PWM = Moto_PwmMax;
	if(MOTO1_PWM<0)	MOTO1_PWM = 0;
	if(MOTO2_PWM<0)	MOTO2_PWM = 0;
	if(MOTO3_PWM<0)	MOTO3_PWM = 0;
	if(MOTO4_PWM<0)	MOTO4_PWM = 0;
	
	TIM3->CCR1 = MOTO1_PWM;
	TIM3->CCR2 = MOTO2_PWM;
	TIM3->CCR3 = MOTO3_PWM;
	TIM3->CCR4 = MOTO4_PWM;
}
