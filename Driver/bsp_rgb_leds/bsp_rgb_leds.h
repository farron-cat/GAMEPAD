#ifndef BSP_RGB_LEDS_H
#define BSP_RGB_LEDS_H

#include "stm32f10x.h"

#define RGB_LEDS_COUNT 3

// GRB数据格式类型
typedef struct
{
    uint8_t g;
    uint8_t r;
    uint8_t b;
} rgb_t;

// ws2812 RGB LED 初始化
void bsp_rgb_leds_init(void);

// 设置指定LED颜色
void bsp_rgb_leds_set_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

// 设置所有LED颜色
void bsp_rgb_leds_set_all_color(uint8_t r, uint8_t g, uint8_t b);

// 清空所有LED
void bsp_rgb_leds_clear(void);

// 刷新
void bsp_rgb_leds_refresh(void);

// 调试用：打印缓冲区前 count 个值（不要在灯效循环里调用）
void bsp_rgb_leds_dump(uint8_t count);

#endif // BSP_RGB_LEDS_H