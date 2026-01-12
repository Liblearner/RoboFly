/*******************************************************************************************
										    �� ��
    ����Ŀ�����������ѧϰʹ�ã�����������ֲ�޸ģ������뱣����������Ϣ����ֲ�����г�������
	
���ɹ�����BUG��������������κ����Ρ��������ã�

* ����汾��V1.01
* �������ڣ�2018-8-18
* �������ߣ���ŭ��С��
* ��Ȩ���У��������������Ϣ�������޹�˾
*******************************************************************************************/
#include “stm32f1xx_hal.h”
#include "iic.h"
#include "delay.h"


////////////////////////////////////////// ��ֲ��֪��//////////////////////////////////////
//
// ��ֲ��ģ��IICʱ����Ҫע�� IIC_WriteByteToSlave() д�������ĵ�ַʱ�����ַ�ʽ�ĵ�ַ���Ѿ�
// ���ƹ�1λ�ģ�ע���� ( I2C_Addr <<1 | 0 )�ķ�ʽ�������֣��������������ͨ��,������ֲ!
//
///////////////////////////////////////////////////////////////////////////////////////////


/******************************************************************************
* ��  ����void IIC_GPIO_Init(void)
* �����ܣ�ģ��IIC���ų�ʼ��
* ��  ������
* ����ֵ����
* ��  ע��PB6->SCL	PB7->SDA
*******************************************************************************/
void IIC_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;   
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);   
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6|GPIO_Pin_7; 
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP; 
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;  
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7);
}

/******************************************************************************
* ��  ����void SDA_OUT(void)
* �����ܣ�����ģ��IIC SDA����Ϊ���
* ��  ������
* ����ֵ����
* ��  ע����
*******************************************************************************/
void SDA_OUT(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7 ;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/******************************************************************************
* ��  ����void SDA_IN(void)
* �����ܣ�����ģ��IIC SDA����Ϊ����
* ��  ������
* ����ֵ����
* ��  ע����
*******************************************************************************/
void SDA_IN(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7 ;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/******************************************************************************
* ��  ����void IIC_Start(void)
* �����ܣ�����IIC��ʼ�ź�
* ��  ������
* ����ֵ����
* ��  ע����
*******************************************************************************/	
void IIC_Start(void)
{
	SDA_OUT(); //SDA����� 
	SDA_H;
	SCL_H;	
	Delay_us(4);
 	SDA_L;
	Delay_us(4);
	SCL_L;
}

/******************************************************************************
* ��  ����void IIC_Stop(void)
* �����ܣ�����IICֹͣ�ź�
* ��  ������
* ����ֵ����
* ��  ע����
*******************************************************************************/	  
void IIC_Stop(void)
{
	SDA_OUT(); //SDA�����
	SCL_L;
	SDA_L;
	Delay_us(4);
	SCL_H; 
	SDA_H;
	Delay_us(4);							   	
}

/******************************************************************************
* ��  ��: uint8_t IIC_WaitAck(void)
* ������: �ȴ�Ӧ���źŵ��� ����ЧӦ�𣺴ӻ���9�� SCL=0 ʱ SDA ���ӻ�����,
*         ���� SCL = 1ʱ SDA��ȻΪ�ͣ�
* ��  ������
* ����ֵ��1������Ӧ��ʧ��	0������Ӧ��ɹ�
* ��  ע���ӻ���������Ӧ��
*******************************************************************************/
uint8_t IIC_WaitAck(void)
{
	uint8_t ucErrTime=0;
	SDA_IN(); //SDA����Ϊ����  ���ӻ���һ���͵�ƽ��ΪӦ�� 
	SDA_H;
	Delay_us(1);	   
	SCL_H;
	Delay_us(1);;	 
	while(SDA_read)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	SCL_L; //ʱ�������	   
	return 0;  
} 

/******************************************************************************
* ��  ��: void IIC_Ack(void)
* ������: ����ACKӦ�� ������������һ���ֽ����ݺ�����������ACK֪ͨ�ӻ�һ��
*         �ֽ���������ȷ���գ�
* ��  ������
* ����ֵ����
* ��  ע���������ӻ���Ӧ��
*******************************************************************************/
void IIC_Ack(void)
{
	SCL_L;
	SDA_OUT();
	SDA_L;
	Delay_us(1);
	SCL_H;
	Delay_us(2);
	SCL_L;
}

/******************************************************************************
* ��  ��: void IIC_NAck(void)
* ������: ����NACKӦ�� ���������������һ���ֽ����ݺ�����������NACK֪ͨ�ӻ�
*         ���ͽ������ͷ�SDA,�Ա���������ֹͣ�źţ�
* ��  ������
* ����ֵ����
* ��  ע���������ӻ���Ӧ��
*******************************************************************************/
void IIC_NAck(void)
{
	SCL_L;
	SDA_OUT();
	SDA_H;
	Delay_us(1);
	SCL_H;
	Delay_us(1);
	SCL_L;
}					 				     

/******************************************************************************
* ��  ����void IIC_SendByte(uint8_t data)
* ��  �ܣ�IIC����һ���ֽ�
* ��  ����data Ҫд������
* ����ֵ����
* ��  ע���������ӻ���
*******************************************************************************/		  
void IIC_SendByte(uint8_t data)
{                        
    uint8_t t;   
	SDA_OUT(); 	    
    SCL_L; //����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {
		if(data&0x80)
			SDA_H;
		else
			SDA_L;
		data <<= 1;
		Delay_us(1);			
		SCL_H;
		Delay_us(1);
		SCL_L;	
		Delay_us(1);
    }	 
} 	 
   
/******************************************************************************
* ��  ����uint8_t IIC_ReadByte(uint8_t ack)
* ��  �ܣ�IIC��ȡһ���ֽ�
* ��  ����ack=1 ʱ���������ݻ�û������ ack=0 ʱ����������ȫ���������
* ����ֵ����
* ��  ע���ӻ���������
*******************************************************************************/	
uint8_t IIC_ReadByte(uint8_t ack)
{
	uint8_t i,receive=0;
	SDA_IN(); //SDA����Ϊ����ģʽ �ȴ����մӻ���������
	for(i=0;i<8;i++ )
	{
		SCL_L; 
		Delay_us(1);
		SCL_H;
		receive<<=1;
		if(SDA_read)receive++; //�ӻ����͵ĵ�ƽ
			Delay_us(1); 
	}					 
    if(ack)
        IIC_Ack(); //����ACK 
    else
        IIC_NAck(); //����nACK  
    return receive;
}

/******************************************************************************
* ��  ����uint8_t IIC_ReadByteFromSlave(uint8_t I2C_Addr,uint8_t addr)
* �����ܣ���ȡָ���豸 ָ���Ĵ�����һ��ֵ
* ��  ����I2C_Addr  Ŀ���豸��ַ
		  reg	    �Ĵ�����ַ
          *buf      ��ȡ����Ҫ�洢�ĵ�ַ    
* ����ֵ������ 1ʧ�� 0�ɹ�
* ��  ע����
*******************************************************************************/ 
uint8_t IIC_ReadByteFromSlave(uint8_t I2C_Addr,uint8_t reg,uint8_t *buf)
{
	IIC_Start();	
	IIC_SendByte(I2C_Addr);	 //���ʹӻ���ַ
	if(IIC_WaitAck()) //����ӻ�δӦ�������ݷ���ʧ��
	{
		IIC_Stop();
		return 1;
	}
	IIC_SendByte(reg); //���ͼĴ�����ַ
	IIC_WaitAck();	  
	
	IIC_Start();
	IIC_SendByte(I2C_Addr+1); //�������ģʽ			   
	IIC_WaitAck();
	*buf=IIC_ReadByte(0);	   
	IIC_Stop(); //����һ��ֹͣ����
	return 0;
}

/*************************************************************************************
* ��  ����uint8_t IIC_WriteByteFromSlave(uint8_t I2C_Addr,uint8_t addr��uint8_t buf))
* �����ܣ�д��ָ���豸 ָ���Ĵ�����һ��ֵ
* ��  ����I2C_Addr  Ŀ���豸��ַ
		  reg	    �Ĵ�����ַ
          buf       Ҫд�������
* ����ֵ��1 ʧ�� 0�ɹ�
* ��  ע����
**************************************************************************************/ 
uint8_t IIC_WriteByteToSlave(uint8_t I2C_Addr,uint8_t reg,uint8_t data)
{
	IIC_Start();
	IIC_SendByte(I2C_Addr); //���ʹӻ���ַ
	if(IIC_WaitAck())
	{
		IIC_Stop();
		return 1; //�ӻ���ַд��ʧ��
	}
	IIC_SendByte(reg); //���ͼĴ�����ַ
	IIC_WaitAck();	  
	IIC_SendByte(data); 
	if(IIC_WaitAck())
	{
		IIC_Stop(); 
		return 1; //����д��ʧ��
	}
	IIC_Stop(); //����һ��ֹͣ����
	return 0;
}

/***************************************************************************************
* ��  ����uint8_t IICreadBytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data)
* �����ܣ���ȡָ���豸 ָ���Ĵ����� length��ֵ
* ��  ����dev     Ŀ���豸��ַ
		  reg	  �Ĵ�����ַ
          length  Ҫ�����ֽ���
		  *data   ���������ݽ�Ҫ��ŵ�ָ��
* ����ֵ��1�ɹ� 0ʧ��
* ��  ע����
***************************************************************************************/ 
uint8_t IIC_ReadMultByteFromSlave(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data)
{
	uint8_t count = 0;
	uint8_t temp;
	IIC_Start();
	IIC_SendByte(dev); //���ʹӻ���ַ
	if(IIC_WaitAck())
	{
		IIC_Stop(); 
		return 1; //�ӻ���ַд��ʧ��
	}
	IIC_SendByte(reg); //���ͼĴ�����ַ
	IIC_WaitAck();	  
	IIC_Start();
	IIC_SendByte(dev+1); //�������ģʽ	
	IIC_WaitAck();
	for(count=0;count<length;count++)
	{
		if(count!=(length-1))
			temp = IIC_ReadByte(1); //��ACK�Ķ�����
		else  
			temp = IIC_ReadByte(0); //���һ���ֽ�NACK

		data[count] = temp;
	}
    IIC_Stop(); //����һ��ֹͣ����
	return 0;
}

/****************************************************************************************
* ��  ����uint8_t IICwriteBytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data)
* �����ܣ�������ֽ�д��ָ���豸 ָ���Ĵ���
* ��  ����dev     Ŀ���豸��ַ
		  reg	  �Ĵ�����ַ
		  length  Ҫд���ֽ���
		  *data   Ҫд������ݽ�Ҫ��ŵ�ָ��
* ����ֵ��1�ɹ� 0ʧ��
* ��  ע����
****************************************************************************************/ 
uint8_t IIC_WriteMultByteToSlave(uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data)
{
 	uint8_t count = 0;
	IIC_Start();
	IIC_SendByte(dev); //���ʹӻ���ַ
	if(IIC_WaitAck())
	{
		IIC_Stop();
		return 1; //�ӻ���ַд��ʧ��
	}
	IIC_SendByte(reg); //���ͼĴ�����ַ
	IIC_WaitAck();	  
	for(count=0;count<length;count++)
	{
		IIC_SendByte(data[count]); 
		if(IIC_WaitAck()) //ÿһ���ֽڶ�Ҫ�ȴӻ�Ӧ��
		{
			IIC_Stop();
			return 1; //����д��ʧ��
		}
	}
	IIC_Stop(); //����һ��ֹͣ����
	return 0;
}


