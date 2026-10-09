#include "bsp_iic_soft.h"
#include "systick.h"
#include <stdio.h>

#define IIC_SCL_RCU RCC_APB2Periph_GPIOB
#define IIC_SCL_PORT GPIOB
#define IIC_SCL_PIN GPIO_Pin_12

#define IIC_SDA_RCU RCC_APB2Periph_GPIOB
#define IIC_SDA_PORT GPIOB
#define IIC_SDA_PIN GPIO_Pin_13

#define IIC_SCL_H GPIO_SetBits(IIC_SCL_PORT, IIC_SCL_PIN)
#define IIC_SCL_L GPIO_ResetBits(IIC_SCL_PORT, IIC_SCL_PIN)

#define IIC_SDA_H GPIO_SetBits(IIC_SDA_PORT, IIC_SDA_PIN)
#define IIC_SDA_L GPIO_ResetBits(IIC_SDA_PORT, IIC_SDA_PIN)
#define IIC_SDA_STA GPIO_ReadInputDataBit(IIC_SDA_PORT, IIC_SDA_PIN)

#define IIC_DELAY delay_1us(2)

// IIC 软实现 初始化
void bsp_iic_soft_init(void)
{
    // SCL
    // 打开时钟
    RCC_APB2PeriphClockCmd(IIC_SCL_RCU, ENABLE);
    // 配置GPIO
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = IIC_SCL_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // SDA
    // 打开时钟
    RCC_APB2PeriphClockCmd(IIC_SDA_RCU, ENABLE);
    // 配置GPIO
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = IIC_SDA_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IIC_SDA_PORT, &GPIO_InitStruct);
}

// 开始信号
static void bsp_iic_soft_start(void)
{
    // SCL高电平时，SDA下降沿

    // SDA高电平持续
    IIC_SDA_H;
    IIC_DELAY;
    // SCL高电平持续
    IIC_SCL_H;
    IIC_DELAY;
    // SDA低电平持续
    IIC_SDA_L;
    IIC_DELAY;
    // SCL低电平持续
    IIC_SCL_L;
    IIC_DELAY;
}

// 结束信号
static void bsp_iic_soft_stop(void)
{
    // SCL高电平时，SDA上升沿

    // SDA低电平持续
    IIC_SDA_L;
    IIC_DELAY;
    // SCL高电平持续
    IIC_SCL_H;
    IIC_DELAY;
    // SDA高电平持续
    IIC_SDA_H;
    IIC_DELAY;
}

// 发送一个字节 高位先行
static void bsp_iic_soft_send_byte(uint8_t byte)
{
    // SDA在SCL高电平时发送数据
    // 从高位开始发送，每次发送一位

    // SCL低电平持续
    IIC_SCL_L;
    IIC_DELAY;

    for (uint8_t i = 0; i < 8; i++)
    {
        // SDA根据数据配置电平
        if ((byte >> (7 - i)) & 0x01)
        {
            IIC_SDA_H;
        }
        else
        {
            IIC_SDA_L;
        }

        // SCL高电平持续
        IIC_SCL_H;
        IIC_DELAY;

        // SCL低电平持续
        IIC_SCL_L;
        IIC_DELAY;
    }
}

// 等待应答 0收到从机应答 1未收到从机应答
static uint8_t bsp_iic_soft_wait_ack(void)
{
    // SCL确定会在低电平，可以改变SDA电平
    // 主机主动拉低SDA，交出控制权

    // SDA高电平持续
    // 主动拉高SDA，保证后续拉低的回复是可信的
    IIC_SDA_H;
    IIC_DELAY;

    // 交出控制权
    IIC_DELAY;

    // SCL高电平持续
    IIC_SCL_H;
    IIC_DELAY;

    // 检查SDA电平
    if (IIC_SDA_STA)
    {
        return 1;
    } // 读 SDA
    IIC_SCL_L;
    IIC_DELAY; // 成功时拉低 SCL
    return 0;
}

// IIC 软实现 写入n个字节
uint8_t bsp_iic_soft_write_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    // 起始信号
    bsp_iic_soft_start();
    // 设备地址（写地址）
    bsp_iic_soft_send_byte(addr << 1 | 0x0);
    // 等待响应
    if (bsp_iic_soft_wait_ack())
    {
        printf("IIC device not found!\n");
        return 1;
    }
    // 寄存器地址
    bsp_iic_soft_send_byte(reg);
    // 等待响应
    if (bsp_iic_soft_wait_ack())
    {
        printf("IIC device not acknowledged!\n");
        return 2;
    }
    // 循环发送数据
    for (uint16_t i = 0; i < len; i++)
    {
        bsp_iic_soft_send_byte(data[i]);
        if (bsp_iic_soft_wait_ack())
        {
            printf("IIC device not acknowledged");
            return 3;
        }
    }
    // 停止信号
    bsp_iic_soft_stop();
    return 0;
}

uint8_t bsp_iic_soft_write_col_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t offset, uint16_t len)
{
    // 起始信号
    bsp_iic_soft_start();
    // 设备地址（写地址）
    bsp_iic_soft_send_byte(addr << 1 | 0x0);
    // 等待响应
    if (bsp_iic_soft_wait_ack())
    {
        printf("IIC device not found!\n");
        return 1;
    }
    // 寄存器地址
    bsp_iic_soft_send_byte(reg);
    // 等待响应
    if (bsp_iic_soft_wait_ack())
    {
        printf("IIC device not acknowledged!\n");
        return 2;
    }
    // 循环发送数据
    for (uint16_t i = 0; i < len; i++)
    {
        bsp_iic_soft_send_byte(data[i * offset]);
        if (bsp_iic_soft_wait_ack())
        {
            printf("IIC device not acknowledged");
            return 3;
        }
    }
    // 停止信号
    bsp_iic_soft_stop();
    return 0;
}

// 接收一字节 高位先行
static uint8_t bsp_iic_soft_recv_byte(void)
{
    uint8_t byte = 0;

    // SCL低电平持续
    IIC_SCL_L;
    IIC_DELAY;

    for (uint8_t i = 0; i < 8; i++)
    {

        // 等待从机写入SDA

        // SCL高电平持续
        IIC_SCL_H;

        // 读取SDA电平
        if (IIC_SDA_STA)
        {
            byte |= (0x01 << (7 - i));
        }
        else
        {
            byte &= ~(0x01 << (7 - i));
        }
        IIC_DELAY;

        // SCL低电平持续
        IIC_SCL_L;
        IIC_DELAY;
    }
    return byte;
}

// 发送应答
static void bsp_iic_soft_send_ack(void)
{

    // SDA低电平持续
    IIC_SDA_L;
    IIC_DELAY;

    // SCL高电平持续
    IIC_SCL_H;
    IIC_DELAY;

    // 等待从机读取

    // SCL低电平持续
    IIC_SCL_L;
    IIC_DELAY;
}

// 发送空应答
static void bsp_iic_soft_send_nack(void)
{

    // SDA高电平持续
    IIC_SDA_H;
    IIC_DELAY;

    // SCL高电平持续
    IIC_SCL_H;
    IIC_DELAY;

    // 等待从机读取

    // SCL低电平持续
    IIC_SCL_L;
    IIC_DELAY;
}

// IIC 软实现 读取n个字节
uint8_t bsp_iic_soft_read_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    // 起始信号
    bsp_iic_soft_start();
    // 设备地址（写地址）
    bsp_iic_soft_send_byte(addr << 1 | 0x0);
    // 等待响应
    if (bsp_iic_soft_wait_ack())
    {
        printf("read1: IIC device not found!\n");
        return 1;
    }
    // 寄存器地址
    bsp_iic_soft_send_byte(reg);
    // 等待响应
    if (bsp_iic_soft_wait_ack())
    {
        printf("read2: IIC device not found!\n");
        return 2;
    }

    // 起始信号
    bsp_iic_soft_start();
    // 设备地址（读地址）
    bsp_iic_soft_send_byte(addr << 1 | 0x1);
    // 等待响应
    if (bsp_iic_soft_wait_ack())
    {
        printf("read3: IIC device not found!\n");
        return 3;
    }
    // 循环读取数据
    for (uint16_t i = 0; i < len - 1; i++)
    {
        // 读取
        data[i] = bsp_iic_soft_recv_byte();
        // 发送响应
        bsp_iic_soft_send_ack();
    }
    // 读取最后一字节
    data[len - 1] = bsp_iic_soft_recv_byte();
    // 发送空响应
    bsp_iic_soft_send_nack();
    // 停止信号
    bsp_iic_soft_stop();
    return 0;
}
