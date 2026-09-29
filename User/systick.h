#ifndef SYS_TICK_H
#define SYS_TICK_H
#include "stm32f10x.h"

void systick_config(void); /* 1ms 中断一次 */
void delay_1us(uint32_t count);
void delay_1ms(uint32_t ms); /* 阻塞延时：只允许启动阶段用 */
void delay_decrement(void);
uint32_t get_ms(void); /* 毫秒时间戳：去抖/心跳/超时都用它 */

#endif
