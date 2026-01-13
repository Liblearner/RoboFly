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
#include "iic.h"

////////////////////////////////////////// 移植须知 //////////////////////////////////////
//
// 移植软件模拟IIC时需要注意 IIC_WriteByteToSlave() 写入函数的地址时传入的地址是左移
// 过1位的,注意看 ( I2C_Addr <<1 | 0 )的方式发送地址,其余函数都通用,请放心移植!
//
///////////////////////////////////////////////////////////////////////////////////////////

/**
  * @brief 微秒级延时函数
  * @param us: 延时的微秒数
  * @retval None
  * @note  在72MHz下，每个__NOP()约消耗13.89ns
  */
static void Delay_us(uint32_t us)
{
    uint32_t count = us * 72 / 10;  // 72MHz下的精确延时 (根据实际调整)
    for(uint32_t i = 0; i < count; i++) 
    {
        __NOP();  // 空操作，消耗一个时钟周期
    }
}

/******************************************************************************
* 函  数:void IIC_GPIO_Init(void)
* 功  能:模拟IIC引脚初始化
* 参  数:无
* 返回值:无
* 备  注:PB6->SCL	PB7->SDA
*******************************************************************************/
void IIC_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	// 使能GPIOB时钟
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	// 配置PB6(SCL)和PB7(SDA)为推挽输出
	GPIO_InitStruct.Pin = IIC_SCL_PIN | IIC_SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;      // 推挽输出
	GPIO_InitStruct.Pull = GPIO_PULLUP;              // 上拉
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;    // 高速
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// 初始化时拉高SCL和SDA
	SCL_H;
	SDA_H;
}

/******************************************************************************
* 函  数:void SDA_OUT(void)
* 功  能:设置模拟IIC SDA引脚为输出
* 参  数:无
* 返回值:无
* 备  注:无
*******************************************************************************/
static void SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.Pin = IIC_SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;      // 推挽输出
	GPIO_InitStruct.Pull = GPIO_PULLUP;              // 上拉
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;    // 高速
	HAL_GPIO_Init(IIC_SDA_PORT, &GPIO_InitStruct);
}

/******************************************************************************
* 函  数:void SDA_IN(void)
* 功  能:设置模拟IIC SDA引脚为输入
* 参  数:无
* 返回值:无
* 备  注:无
*******************************************************************************/
static void SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.Pin = IIC_SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;          // 浮空输入
	GPIO_InitStruct.Pull = GPIO_NOPULL;              // 无上下拉
	HAL_GPIO_Init(IIC_SDA_PORT, &GPIO_InitStruct);
}

/******************************************************************************
* 函  数:void IIC_Start(void)
* 功  能:产生IIC起始信号
* 参  数:无
* 返回值:无
* 备  注:SCL高电平期间,SDA由高到低跳变为起始信号
*******************************************************************************/	
void IIC_Start(void)
{
	SDA_OUT();   // SDA输出模式
	SDA_H;
	SCL_H;	
	Delay_us(4);
	SDA_L;       // START信号: SCL为高时,SDA下降沿
	Delay_us(4);
	SCL_L;       // 钳住I2C总线,准备发送或接收数据
}

/******************************************************************************
* 函  数:void IIC_Stop(void)
* 功  能:产生IIC停止信号
* 参  数:无
* 返回值:无
* 备  注:SCL高电平期间,SDA由低到高跳变为停止信号
*******************************************************************************/	  
void IIC_Stop(void)
{
	SDA_OUT();   // SDA输出模式
	SCL_L;
	SDA_L;
	Delay_us(4);
	SCL_H;       // STOP信号: SCL为高时,SDA上升沿
	SDA_H;
	Delay_us(4);							   	
}

/******************************************************************************
* 函  数: uint8_t IIC_WaitAck(void)
* 功  能: 等待应答信号到来 有效应答：从机在第9个 SCL=0 时 SDA 由从机拉低,
*         当在 SCL = 1时 SDA仍然为低
* 参  数:无
* 返回值:1-接收应答失败	0-接收应答成功
* 备  注:从机发送应答
*******************************************************************************/
uint8_t IIC_WaitAck(void)
{
	uint8_t ucErrTime = 0;
	
	SDA_IN();    // SDA设置为输入,等待从机拉低作为应答
	SDA_H;       // 主机释放SDA线
	Delay_us(1);	   
	SCL_H;
	Delay_us(1);
	
	while(SDA_read)  // 等待应答
	{
		ucErrTime++;
		if(ucErrTime > 250)
		{
			IIC_Stop();
			return 1;  // 超时,返回失败
		}
	}
	SCL_L;       // 时钟输出0
	return 0;  
} 

/******************************************************************************
* 函  数: void IIC_Ack(void)
* 功  能: 产生ACK应答 主机在接收完一个字节数据后,向从机发送ACK通知从机一个
*         字节数据已经接收完成
* 参  数:无
* 返回值:无
* 备  注:主机给从机应答
*******************************************************************************/
void IIC_Ack(void)
{
	SCL_L;
	SDA_OUT();
	SDA_L;       // ACK信号
	Delay_us(1);
	SCL_H;
	Delay_us(2);
	SCL_L;
}

/******************************************************************************
* 函  数: void IIC_NAck(void)
* 功  能: 产生NACK应答 主机在接收完最后一个字节数据后,向从机发送NACK通知从机
*         数据传输结束,释放SDA,以便主机产生停止信号
* 参  数:无
* 返回值:无
* 备  注:主机给从机应答
*******************************************************************************/
void IIC_NAck(void)
{
	SCL_L;
	SDA_OUT();
	SDA_H;       // NACK信号
	Delay_us(1);
	SCL_H;
	Delay_us(1);
	SCL_L;
}					 				     

/******************************************************************************
* 函  数:void IIC_SendByte(uint8_t data)
* 功  能:IIC发送一个字节
* 参  数:data 要写入的数据
* 返回值:无
* 备  注:主机发送给从机
*******************************************************************************/		  
void IIC_SendByte(uint8_t data)
{                        
    uint8_t t;   
	SDA_OUT(); 	    
    SCL_L;       // 拉低时钟开始数据传输
	
    for(t = 0; t < 8; t++)
    {
		if(data & 0x80)
			SDA_H;
		else
			SDA_L;
		data <<= 1;
		Delay_us(1);			
		SCL_H;   // 时钟拉高,数据有效
		Delay_us(1);
		SCL_L;	 // 时钟拉低,准备下一位
		Delay_us(1);
    }	 
} 	 
   
/******************************************************************************
* 函  数:uint8_t IIC_ReadByte(uint8_t ack)
* 功  能:IIC读取一个字节
* 参  数:ack=1 时发送完数据还有后续 ack=0 时发送完数据全部接收完成
* 返回值:读取的数据
* 备  注:从机发送给主机
*******************************************************************************/	
uint8_t IIC_ReadByte(uint8_t ack)
{
	uint8_t i, receive = 0;
	SDA_IN();    // SDA设置为输入模式,等待接收从机发送的数据
	
	for(i = 0; i < 8; i++)
	{
		SCL_L; 
		Delay_us(1);
		SCL_H;
		receive <<= 1;
		if(SDA_read)
			receive++;   // 从机发送的电平
		Delay_us(1); 
	}					 
	
	if(ack)
		IIC_Ack();   // 发送ACK 
	else
		IIC_NAck();  // 发送NACK  
	
	return receive;
}

/******************************************************************************
* 函  数:uint8_t IIC_ReadByteFromSlave(uint8_t I2C_Addr,uint8_t reg,uint8_t *buf)
* 功  能:读取指定设备 指定寄存器的一个值
* 参  数:I2C_Addr  目标设备地址
*         reg       寄存器地址
*         *buf      读出数据要存储的地址    
* 返回值:错误 1失败 0成功
* 备  注:无
*******************************************************************************/ 
uint8_t IIC_ReadByteFromSlave(uint8_t I2C_Addr, uint8_t reg, uint8_t *buf)
{
	IIC_Start();	
	IIC_SendByte(I2C_Addr);   // 发送从机地址(写)
	if(IIC_WaitAck())         // 如果从机未应答,数据发送失败
	{
		IIC_Stop();
		return 1;
	}
	IIC_SendByte(reg);        // 发送寄存器地址
	IIC_WaitAck();	  
	
	IIC_Start();              // 重新开始
	IIC_SendByte(I2C_Addr + 1);  // 进入接收模式(读)
	IIC_WaitAck();
	*buf = IIC_ReadByte(0);   // 读取数据,发送NACK
	IIC_Stop();               // 产生一个停止条件
	return 0;
}

/*************************************************************************************
* 函  数:uint8_t IIC_WriteByteToSlave(uint8_t I2C_Addr,uint8_t reg,uint8_t data)
* 功  能:写入指定设备 指定寄存器一个值
* 参  数:I2C_Addr  目标设备地址
*         reg       寄存器地址
*         data      要写入的数据
* 返回值:1 失败 0成功
* 备  注:无
**************************************************************************************/ 
uint8_t IIC_WriteByteToSlave(uint8_t I2C_Addr, uint8_t reg, uint8_t data)
{
	IIC_Start();
	IIC_SendByte(I2C_Addr);   // 发送从机地址(写)
	if(IIC_WaitAck())
	{
		IIC_Stop();
		return 1;             // 从机地址写入失败
	}
	IIC_SendByte(reg);        // 发送寄存器地址
	IIC_WaitAck();	  
	IIC_SendByte(data);       // 发送数据
	if(IIC_WaitAck())
	{
		IIC_Stop(); 
		return 1;             // 数据写入失败
	}
	IIC_Stop();               // 产生一个停止条件
	return 0;
}

/***************************************************************************************
* 函  数:uint8_t IIC_ReadMultByteFromSlave(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data)
* 功  能:读取指定设备 指定寄存器的 length个值
* 参  数:dev      目标设备地址
*         reg      寄存器地址
*         length   要读的字节数
*         *data    读出的数据将要存放的指针
* 返回值:0成功 1失败
* 备  注:无
***************************************************************************************/ 
uint8_t IIC_ReadMultByteFromSlave(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data)
{
	uint8_t count = 0;
	
	IIC_Start();
	IIC_SendByte(dev);        // 发送从机地址(写)
	if(IIC_WaitAck())
	{
		IIC_Stop(); 
		return 1;             // 从机地址写入失败
	}
	IIC_SendByte(reg);        // 发送寄存器地址
	IIC_WaitAck();	  
	
	IIC_Start();              // 重新开始
	IIC_SendByte(dev + 1);    // 进入接收模式(读)
	IIC_WaitAck();
	
	for(count = 0; count < length; count++)
	{
		if(count != (length - 1))
			data[count] = IIC_ReadByte(1);  // 带ACK的读数据
		else  
			data[count] = IIC_ReadByte(0);  // 最后一个字节NACK
	}
	
	IIC_Stop();               // 产生一个停止条件
	return 0;
}

/****************************************************************************************
* 函  数:uint8_t IIC_WriteMultByteToSlave(uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data)
* 功  能:连续写字节到指定设备 指定寄存器
* 参  数:dev      目标设备地址
*         reg      寄存器地址
*         length   要写的字节数
*         *data    要写入的数据将要存放的指针
* 返回值:0成功 1失败
* 备  注:无
****************************************************************************************/ 
uint8_t IIC_WriteMultByteToSlave(uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data)
{
	uint8_t count = 0;
	
	IIC_Start();
	IIC_SendByte(dev);        // 发送从机地址(写)
	if(IIC_WaitAck())
	{
		IIC_Stop();
		return 1;             // 从机地址写入失败
	}
	IIC_SendByte(reg);        // 发送寄存器地址
	IIC_WaitAck();	  
	
	for(count = 0; count < length; count++)
	{
		IIC_SendByte(data[count]); 
		if(IIC_WaitAck())     // 每一个字节都要等从机应答
		{
			IIC_Stop();
			return 1;         // 数据写入失败
		}
	}
	IIC_Stop();               // 产生一个停止条件
	return 0;
}
