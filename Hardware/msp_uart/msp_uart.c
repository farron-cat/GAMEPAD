#include "msp_uart.h"

#define USART_RECEIVE_LENGTH 1024

uint8_t recv_buffer[USART_RECEIVE_LENGTH];
uint16_t recv_length;

void GPIO_config()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 打开时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 配置 PA9 (TX) 为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 PA10 (RX) 为浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void UART_config()
{
    USART_InitTypeDef USART_InitStructure;

    // 打开时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // 配置 USART1 参数：115200, 8位数据, 1位停止, 无校验
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    // 发送和接收功能使能
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // 使能 USART1 中断
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;         // 通道
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 使能
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 响应优先级
    NVIC_Init(&NVIC_InitStructure);

    // 使能读取数据缓冲区不为空中断 （一帧数据结束）
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    // 使能空闲中断（一包数据结束）
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

    // 使能 USART1
    USART_Cmd(USART1, ENABLE);
}

void msp_uart_init()
{
    GPIO_config();
    UART_config();
}

// 发送一个字节数据
void msp_uart_send_byte(USART_TypeDef *USARTx, uint8_t data)
{
    // 通过USARTx发送一个字节的数据
    USART_SendData(USARTx, data);
    // 判断发送数据寄存器是否为空
    // TXE：发送数据寄存器空，可以写下一个字节
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
        ;
}

// 发送一个字符串
void msp_uart_send_string(USART_TypeDef *USARTx, char *str)
{
    // str非空 且 *str非'\0'
    while (str && *str)
    {
        msp_uart_send_byte(USARTx, (uint8_t)*str);
        str++;
    }
}

// 重定向 printf 到 USART1
int fputc(int ch, FILE *f)
{
    msp_uart_send_byte(USART1, (uint8_t)ch);
    return ch;
}

#ifndef __weak
#define __weak __attribute__((weak)) // GCC / armclang
#endif

// 接收回调函数
__weak void on_uart_receive(USART_TypeDef *USARTx)
{
    // 结尾加 '\0'
    recv_buffer[recv_length] = '\0';

    // 原样发回
    msp_uart_send_string(USARTx, (char *)recv_buffer);
}

// 实际中断处理函数
static void uart_irq_handle(USART_TypeDef *USARTx)
{
    // 处理读取数据缓冲区不为空中断 （每接收一个字节（一帧）就会有一个RXNE中断）
    if (USART_GetITStatus(USARTx, USART_IT_RXNE) == SET)
    {
        // 读 DR，自动清 RXNE
        uint8_t data = USART_ReceiveData(USARTx);
        // 存入一个字节到接收缓冲区
        recv_buffer[recv_length++] = data;
    }

    //
    if (USART_GetITStatus(USARTx, USART_IT_IDLE) == SET)
    {
        // IDLE 标志不能通过读 DR 清除，需要先读 SR 再读 DR
        volatile uint32_t tmp;
        tmp = USARTx->SR;
        tmp = USARTx->DR;
        (void)tmp;

        // 回调函数
        on_uart_receive(USARTx);

        // 清空接收缓冲区
        recv_length = 0;
    }
}

// USART1 中断处理函数
// startup.s中查到
void USART1_IRQHandler(void)
{
    uart_irq_handle(USART1);
}