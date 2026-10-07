#ifndef MSP_TIMER_H
#define MSP_TIMER_H

#include "stm32f10x.h"

#define WS2812_PORT GPIOA
#define WS2812_PIN  GPIO_Pin_8

// 用于ws2812的PWM定时器的初始化，包含DMA的初始化
void msp_ws2812_timer_init(void);

// 用于ws2812的DMA发送数据
void msp_ws2812_dma_send(const uint16_t *buf, uint16_t len);

#endif // MSP_TIMER_H