/*******************************************************************************************
                                        声 明
    本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现其他
    
不可估量的BUG，天际智联不负任何责任。请勿商用！

* 程序版本：V1.01
* 程序日期：2018-8-18
* 程序作者：愤怒的小孩
* 版权所有：西安天际智联信息技术有限公司
* 移植说明：移植到STM32 HAL库
*******************************************************************************************/
#include "stm32f1xx_hal.h"
#include "si24r1.h"
#include "spi_io.h"
#include "stdio.h"
#include "stdlib.h"

#define SI24R1AddrMax 50  // NRF最后一个字节地址最大为50

uint8_t SI24R1addr = 0xFF;  // 初始化NRF最后一字节地址

uint8_t SI24R1_TX_DATA[TX_PAYLO_WIDTH];  // NRF发送缓冲区
uint8_t SI24R1_RX_DATA[RX_PAYLO_WIDTH];  // NRF接收缓冲区

uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0xF1};  // 发送地址
uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0xF1};  // 接收地址

/**
  * @brief 微秒级延时函数
  * @param us: 延时的微秒数
  * @retval None
  */
static void Delay_us(uint32_t us)
{
    uint32_t count = us * (SystemCoreClock / 1000000) / 10;
    for(uint32_t i = 0; i < count; i++) 
    {
        __NOP();
    }
}

/*****************************************************************************
 * 函  数：void SI24R1_Init(void)
 * 功  能：NRF引脚GPIO初始化
 * 参  数：无
 * 返回值：无
 * 备  注：使用HAL库初始化GPIO
 *****************************************************************************/
void SI24R1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 使能GPIOA和GPIOB时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // 配置CSN引脚 (PB12)
    GPIO_InitStruct.Pin = SI24R1_CSN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;      // 推挽输出
    GPIO_InitStruct.Pull = GPIO_PULLUP;              // 上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;    // 高速
    HAL_GPIO_Init(SI24R1_CSN_PORT, &GPIO_InitStruct);
    
    // 配置CE引脚 (PA8)
    GPIO_InitStruct.Pin = SI24R1_CE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;      // 推挽输出
    GPIO_InitStruct.Pull = GPIO_PULLUP;              // 上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;    // 高速
    HAL_GPIO_Init(SI24R1_CE_PORT, &GPIO_InitStruct);
    
    // 初始化引脚状态
    SI24R1_CSN_HIGH;  // 失能NRF
    SI24R1_CE_LOW;    // 待机模式
    
    SPI_GPIO_Init();  // SPI初始化 (需要先实现SPI的HAL库版本)
    
    SI24R1_Check();   // 检查SI24R1是否与MCU通信
}

// /*****************************************************************************
//  * 函  数：uint8_t SPI2_WriteReadByte(uint8_t TxData)
//  * 功  能：SPI2发送并接收一个字节
//  * 参  数：TxData - 要发送的数据
//  * 返回值：接收到的数据
//  * 备  注：阻塞式收发
//  *****************************************************************************/
// uint8_t SPI2_WriteReadByte(uint8_t TxData)
// {
//     uint8_t RxData = 0;
    
//     // HAL库SPI发送接收函数
//     if (HAL_SPI_TransmitReceive(&hspi2, &TxData, &RxData, 1, 1000) != HAL_OK)
//     {
//         // 传输错误处理
//         return 0xFF;
//     }
    
//     return RxData;
// }


/*****************************************************************************
 * 函  数：uint8_t SI24R1_write_reg(uint8_t reg, uint8_t value)
 * 功  能：写一字节数据到寄存器
 * 参  数：reg: 寄存器地址
 *         value: 要写入的数据
 * 返回值：status
 * 备  注：SI24R1代码移植只需把SPI驱动修改成自己的即可
 *****************************************************************************/
uint8_t SI24R1_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t status;
    
    SI24R1_CSN_LOW;
    status = SPI2_WriteReadByte(reg);
    SPI2_WriteReadByte(value);
    SI24R1_CSN_HIGH;
    
    return status;
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_read_reg(uint8_t reg)
 * 功  能：读一字节数据从寄存器
 * 参  数：reg: 寄存器地址
 * 返回值：reg_val
 * 备  注：SI24R1代码移植只需把SPI驱动修改成自己的即可
 *****************************************************************************/
uint8_t SI24R1_read_reg(uint8_t reg)
{
    uint8_t reg_val;
    
    SI24R1_CSN_LOW;
    SPI2_WriteReadByte(reg);
    reg_val = SPI2_WriteReadByte(0xFF);
    SI24R1_CSN_HIGH;
    
    return reg_val;
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
 * 功  能：写一组数据到寄存器
 * 参  数：reg: 寄存器地址
 *         pBuf: 要写入数据的地址
 *         len: 要写入的数据长度
 * 返回值：status
 * 备  注：SI24R1代码移植只需把SPI驱动修改成自己的即可
 *****************************************************************************/
uint8_t SI24R1_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
    uint8_t status;
    uint8_t i;
    
    SI24R1_CSN_LOW;
    status = SPI2_WriteReadByte(reg);
    for(i = 0; i < len; i++)
    {
        SPI2_WriteReadByte(*pBuf);
        pBuf++;
    }
    SI24R1_CSN_HIGH;
    
    return status;
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
 * 功  能：读一组数据从寄存器
 * 参  数：reg: 寄存器地址
 *         pBuf: 要读取数据的地址
 *         len: 要读取的数据长度
 * 返回值：status
 * 备  注：SI24R1代码移植只需把SPI驱动修改成自己的即可
 *****************************************************************************/
uint8_t SI24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
    uint8_t status;
    uint8_t i;
    
    SI24R1_CSN_LOW;
    status = SPI2_WriteReadByte(reg);
    for(i = 0; i < len; i++)
    {
        *pBuf = SPI2_WriteReadByte(0xFF);
        pBuf++;
    }
    SI24R1_CSN_HIGH;
    
    return status;
}

/*****************************************************************************
 * 函  数：void SI24R1set_Mode(uint8_t mode)
 * 功  能：切换SI24R1的工作模式
 * 参  数：mode: IT_TX(发送模式) 或 IT_RX(接收模式)
 * 返回值：无
 * 备  注：无
 *****************************************************************************/
void SI24R1set_Mode(uint8_t mode)
{
    if(mode == IT_TX)
    {
        SI24R1_CE_LOW;
        SI24R1_write_reg(W_REGISTER + CONFIG, IT_TX);
        SI24R1_write_reg(W_REGISTER + STATUS, 0x7E);  // 清除所有中断
        SI24R1_CE_HIGH;
        Delay_us(15);
    }
    else
    {
        SI24R1_CE_LOW;
        SI24R1_write_reg(W_REGISTER + CONFIG, IT_RX);  // 配置为接收模式
        SI24R1_write_reg(W_REGISTER + STATUS, 0x7E);   // 清除所有中断
        SI24R1_CE_HIGH;
        Delay_us(200);
    }
}

/*****************************************************************************
 * 函  数：void SI24R1_Config(void)
 * 功  能：SI24R1基本参数配置，并初始化为接收模式
 * 参  数：无
 * 返回值：无
 * 备  注：无
 *****************************************************************************/
void SI24R1_Config(void)
{
    SI24R1_CE_LOW;
    
    // 配置通信地址长度为5字节
    SI24R1_write_reg(W_REGISTER + SETUP_AW, 0x03);
    
    // 写TX节点地址
    SI24R1_Write_Buf(W_REGISTER + TX_ADDR, (uint8_t *)TX_ADDRESS, TX_ADR_WIDTH);
    
    // 设置TX节点地址,主要为了使能ACK
    SI24R1_Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t *)TX_ADDRESS, RX_ADR_WIDTH);
    
    // 设置自动重发间隔时间:500us + 86us; 最大自动重发次数:10次
    SI24R1_write_reg(W_REGISTER + SETUP_RETR, 0x1A);
    
    // 使能通道0的接收地址
    SI24R1_write_reg(W_REGISTER + EN_RXADDR, 0x01);
    
    // 使能通道0自动应答
    SI24R1_write_reg(W_REGISTER + EN_AA, 0x01);
    
    // 选择通道0的有效数据宽度
    SI24R1_write_reg(W_REGISTER + RX_PW_P0, RX_PAYLO_WIDTH);
    
    // 写RX节点地址
    SI24R1_Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t *)RX_ADDRESS, RX_ADR_WIDTH);
    
    // 设置RF通道为30 (建议1-125之间)
    SI24R1_write_reg(W_REGISTER + RF_CH, 30);
    
    // 设置TX发射参数: 0dB增益, 2Mbps, 低噪声增益关闭
    // 0x27 = 0010 0111 (2Mbps, 0dBm, LNA增益)
    SI24R1_write_reg(W_REGISTER + RF_SETUP, 0x27);
    
    // 默认为接收模式
    SI24R1set_Mode(IT_RX);
    
    SI24R1_CE_HIGH;
}

/*****************************************************************************
 * 函  数：void SI24R1_TxPacket(uint8_t *txbuf)
 * 功  能：SI24R1发送一包数据
 * 参  数：txbuf: 要发送数据地址
 * 返回值：无
 * 备  注：无
 *****************************************************************************/
void SI24R1_TxPacket(uint8_t *txbuf)
{
    SI24R1_CE_LOW;
    
    // 写TX节点地址
    SI24R1_Write_Buf(W_REGISTER + TX_ADDR, (uint8_t *)TX_ADDRESS, TX_ADR_WIDTH);
    
    // 设置TX节点地址,主要为了使能ACK
    SI24R1_Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t *)TX_ADDRESS, RX_ADR_WIDTH);
    
    // 写数据到TX_FIFO
    SI24R1_Write_Buf(W_RX_PAYLOAD, txbuf, TX_PAYLO_WIDTH);
    
    // 设置为发送模式,开启所有中断
    SI24R1_write_reg(W_REGISTER + CONFIG, 0x0E);
    
    // 清除所有中断
    SI24R1_write_reg(W_REGISTER + STATUS, 0x7E);
    
    SI24R1_CE_HIGH;
    Delay_us(10);  // CE持续高电平至少10us
}

/*****************************************************************************
 * 函  数：void SI24R1_RxPacket(uint8_t *rxbuf)
 * 功  能：SI24R1接收一包数据
 * 参  数：rxbuf: 接收数据存储地址
 * 返回值：无
 * 备  注：无
 *****************************************************************************/
void SI24R1_RxPacket(uint8_t *rxbuf)
{
    SI24R1_CE_LOW;
    
    // 读取RX的有效数据
    SI24R1_Read_Buf(R_RX_PAYLOAD, rxbuf, RX_PAYLO_WIDTH);
    
    // 清除RX FIFO (注意:这句话很必要)
    SI24R1_write_reg(FLUSH_RX, 0xFF);
    
    SI24R1_CE_HIGH;
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_testConnection(void)
 * 功  能：检查SI24R1与MCU的SPI总线是否通信正常
 * 参  数：无
 * 返回值：1-已连接 0-未连接
 * 备  注：无
 *****************************************************************************/
uint8_t SI24R1_testConnection(void)
{
    uint8_t buf[5] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
    uint8_t i;
    
    // 写入5个字节的地址
    SI24R1_Write_Buf(W_REGISTER + TX_ADDR, buf, 5);
    
    // 读出写入的地址
    SI24R1_Read_Buf(TX_ADDR, buf, 5);
    
    // 检查读出的数据是否正确
    for(i = 0; i < 5; i++)
    {
        if(buf[i] != 0xA5)
            break;
    }
    
    if(i != 5)
        return 0;  // 检测SI24R1错误
    
    return 1;      // 检测到SI24R1
}

/*****************************************************************************
 * 函  数：void SI24R1_Check(void)
 * 功  能：检测SI24R1是否连接
 * 参  数：无
 * 返回值：无
 * 备  注：如果未连接会一直等待
 *****************************************************************************/
void SI24R1_Check(void)
{
    while(!SI24R1_testConnection())
    {
        printf("\r\nSI24R1 no connect...\r\n");
        HAL_Delay(500);  // 延时500ms
        // 如果有LED指示,可以在这里添加LED闪烁代码
    }
    printf("\r\nSI24R1 connected!\r\n");
}

/*****************************************************************************
 * 函  数：void SI24R1_GetAddr(void)
 * 功  能：给飞机上的SI24R1获取一个地址
 * 参  数：无
 * 返回值：无
 * 备  注：此函数需要与遥控器的对频函数联合使用,否则SI24R1通信不成功
 *         如果自己做的遥控器可直接用固定地址
 *****************************************************************************/
void SI24R1_GetAddr(void)
{
    if(SI24R1addr > SI24R1AddrMax)  // 当SI24R1addr大于50,说明此时SI24R1还未初始化完成
    {
        srand(SysTick->VAL);  // 给随机数种子
        SI24R1addr = rand() % SI24R1AddrMax;  // 随机获取SI24R1最后一位地址(地址:0~49)
        // PID_WriteFlash();  // 保存此地址到Flash (需要实现Flash保存功能)
        printf("Generated SI24R1 Address: %d\r\n", SI24R1addr);
    }
    else if(SI24R1addr != TX_ADDRESS[TX_ADR_WIDTH - 1])
    {
        TX_ADDRESS[TX_ADR_WIDTH - 1] = SI24R1addr;
        RX_ADDRESS[RX_ADR_WIDTH - 1] = SI24R1addr;
        SI24R1_Config();
        printf("SI24R1 Address set to: %d\r\n", SI24R1addr);
    }
}

/*****************************************************************************
 * 函  数：void SI24R1_Test(void)
 * 功  能：SI24R1通信测试函数
 * 参  数：无
 * 返回值：无
 * 备  注：测试时用
 *****************************************************************************/
void SI24R1_Test(void)
{
    uint8_t t = 0;
    static uint8_t mode = ' ';
    static uint8_t key;
    
    key = mode;
    
    // 填充测试数据
    for(t = 0; t < TX_PAYLO_WIDTH; t++)
    {
        key++;
        if(key > '~')
            key = ' ';
        SI24R1_TX_DATA[t] = key;
    }
    
    mode++;
    if(mode > '~')
        mode = ' ';
    
    // 发送测试数据
    SI24R1_TxPacket(SI24R1_TX_DATA);
    
    printf("SI24R1 Test packet sent\r\n");
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_GetStatus(void)
 * 功  能：读取SI24R1状态寄存器
 * 参  数：无
 * 返回值：状态寄存器的值
 * 备  注：可用于判断接收/发送状态
 *****************************************************************************/
uint8_t SI24R1_GetStatus(void)
{
    return SI24R1_read_reg(STATUS);
}

/*****************************************************************************
 * 函  数：void SI24R1_ClearIRQ(uint8_t irq_source)
 * 功  能：清除SI24R1中断标志
 * 参  数：irq_source: 要清除的中断标志 (RX_DR/TX_DR/MAX_TX)
 * 返回值：无
 * 备  注：写1清除对应的中断标志位
 *****************************************************************************/
void SI24R1_ClearIRQ(uint8_t irq_source)
{
    SI24R1_write_reg(W_REGISTER + STATUS, irq_source);
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_RxAvailable(void)
 * 功  能：检查是否有数据可读
 * 参  数：无
 * 返回值：1-有数据 0-无数据
 * 备  注：可用于轮询方式接收数据
 *****************************************************************************/
uint8_t SI24R1_RxAvailable(void)
{
    uint8_t status = SI24R1_GetStatus();
    
    if(status & (1 << RX_DR))  // 检查RX_DR标志
    {
        return 1;
    }
    return 0;
}

/*****************************************************************************
 * 函  数：uint8_t SI24R1_TxComplete(void)
 * 功  能：检查发送是否完成
 * 参  数：无
 * 返回值：1-发送完成 0-发送未完成
 * 备  注：可用于轮询方式发送数据
 *****************************************************************************/
uint8_t SI24R1_TxComplete(void)
{
    uint8_t status = SI24R1_GetStatus();
    
    if(status & (1 << TX_DR))  // 检查TX_DR标志
    {
        SI24R1_ClearIRQ(1 << TX_DR);  // 清除发送完成标志
        return 1;
    }
    else if(status & (1 << 4))  // 检查MAX_RT标志
    {
        SI24R1_ClearIRQ(1 << 4);  // 清除最大重传标志
        SI24R1_write_reg(FLUSH_TX, 0xFF);  // 清空TX FIFO
        return 2;  // 发送失败(达到最大重传次数)
    }
    return 0;
}
