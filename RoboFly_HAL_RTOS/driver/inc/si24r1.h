#ifndef   _SI24R1_H
#define   _SI24R1_H
#include "stdint.h"
#include "stm32f1xx_hal.h"

/******* SI24R1命令 *********/
#define R_REGISTER   0x00  // 读配置寄存器
#define W_REGISTER   0x20  // 写配置寄存器
#define R_RX_PAYLOAD 0x61  // 读RX有效数据
#define W_RX_PAYLOAD 0xA0  // 写TX有效数据
#define FLUSH_TX     0xE1  // 清除TX FIFO寄存器 应用于发射模式
#define FLUSH_RX     0xE2  // 清除RX FIFO寄存器 应用于接收模式
#define REUSE_TX_PL  0xE3  // 重新使用上一数据包
#define NOP          0xFF

/******* SI24R1寄存器地址 *******/
#define CONFIG       0x00  // bit0(PRIM_RX):1接收模式 0发射模式 bit1(pwr_up):1上电0掉电
                           // bit2:CRC模式 bit3:CRC使能 bit4:可屏蔽中断MAX_RT 
                           // bit5:可屏蔽中断TX_DS bit6:可屏蔽中断RX_DR
#define EN_AA        0x01  // 使能0-5通道的自动应答功能
#define EN_RXADDR    0x02  // 接收地址允许 0-5通道，默认通道0通道1启动
#define SETUP_AW     0x03  // 设置地址宽度 00:非法 01:3字节 10:4字节 11:5字节
#define SETUP_RETR   0x04  // 建立自动重发 3:0自动重发计数 7:4自动重发延时
#define RF_CH        0x05  // 射频通道 6:0设置nRF24L01工作频率
#define RF_SETUP     0x06  // 射频寄存器 0:低噪声放大器增益 2:1发射功率 3:传输速率
#define STATUS       0x07  // 状态寄存器 0:TX FIFO满标志 3:1接收数据通道号 
                           // 4:达到最大重发中断 5:数据发送完成中断 6:数据接收中断
#define MAX_TX       0x10  // 最大重发次数
#define TX_OK        0x20  // 发送完成
#define RX_OK        0x40  // 接收完成
#define RX_P_NO      0x0E  // 接收通道号掩码

#define OBSERVE_TX   0x08  // 3:0重发计数器 7:4数据包丢失计数器
#define CD           0x09  // 载波检测
#define RX_ADDR_P0   0x0A  // 数据通道0接收地址，最大长度5字节
#define RX_ADDR_P1   0x0B  // 数据通道1接收地址，最大长度5字节
#define RX_ADDR_P2   0x0C  // 数据通道2接收地址
#define RX_ADDR_P3   0x0D  // 数据通道3接收地址
#define RX_ADDR_P4   0x0E  // 数据通道4接收地址
#define RX_ADDR_P5   0x0F  // 数据通道5接收地址
#define TX_ADDR      0x10  // 发送地址
#define RX_PW_P0     0x11  // 接收数据通道0有效数据宽度(1-32字节)
#define RX_PW_P1     0x12  // 接收数据通道1有效数据宽度(1-32字节)
#define RX_PW_P2     0x13  // 接收数据通道2有效数据宽度(1-32字节)
#define RX_PW_P3     0x14  // 接收数据通道3有效数据宽度(1-32字节)
#define RX_PW_P4     0x15  // 接收数据通道4有效数据宽度(1-32字节)
#define RX_PW_P5     0x16  // 接收数据通道5有效数据宽度(1-32字节)
#define FIFO_STATUS  0x17  // FIFO状态寄存器

// SI24R1引脚定义 (便于移植)
#define SI24R1_CSN_PORT     GPIOB
#define SI24R1_CSN_PIN      GPIO_PIN_12
#define SI24R1_CE_PORT      GPIOA
#define SI24R1_CE_PIN       GPIO_PIN_8

// 设置引脚电平 (HAL库方式)
#define SI24R1_CSN_LOW      HAL_GPIO_WritePin(SI24R1_CSN_PORT, SI24R1_CSN_PIN, GPIO_PIN_RESET)
#define SI24R1_CSN_HIGH     HAL_GPIO_WritePin(SI24R1_CSN_PORT, SI24R1_CSN_PIN, GPIO_PIN_SET)
#define SI24R1_CE_LOW       HAL_GPIO_WritePin(SI24R1_CE_PORT, SI24R1_CE_PIN, GPIO_PIN_RESET)
#define SI24R1_CE_HIGH      HAL_GPIO_WritePin(SI24R1_CE_PORT, SI24R1_CE_PIN, GPIO_PIN_SET)

#define RX_DR            6  // 数据接收完成中断标志位
#define TX_DR            5  // 数据发送完成中断标志位
#define IT_TX            0x0E  // 发送模式
#define IT_RX            0x0F  // 接收模式

#define TX_ADR_WIDTH     5
#define RX_ADR_WIDTH     5
#define TX_PAYLO_WIDTH   32
#define RX_PAYLO_WIDTH   32

// 全局变量声明
extern uint8_t SI24R1_TX_DATA[TX_PAYLO_WIDTH];
extern uint8_t SI24R1_RX_DATA[RX_PAYLO_WIDTH];
extern uint8_t SI24R1addr;

// 函数声明
void SI24R1_Init(void);
void SI24R1_Check(void);
void SI24R1_Config(void);
void SI24R1_GetAddr(void);
void SI24R1set_Mode(uint8_t mode);
uint8_t SI24R1_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len);
uint8_t SI24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len);
uint8_t SI24R1_read_reg(uint8_t reg);
uint8_t SI24R1_write_reg(uint8_t reg, uint8_t value);
void SI24R1_TxPacket(uint8_t *txbuf);
void SI24R1_RxPacket(uint8_t *rxbuf);
void SI24R1_Test(void);
uint8_t SI24R1_testConnection(void);

#endif
