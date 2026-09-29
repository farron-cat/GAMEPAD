#ifndef MSP_UART_H
#define MSP_UART_H

#include "stm32f10x.h"
#include <stdio.h>

void msp_uart_init();

void msp_uart_send_byte(USART_TypeDef *USARTx, uint8_t data);

void msp_uart_send_string(USART_TypeDef *USARTx, char *str);

extern void on_uart_receive(USART_TypeDef *USARTx);

#endif // MSP_UART_H