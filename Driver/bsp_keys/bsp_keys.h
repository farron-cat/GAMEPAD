#ifndef BSP_KEYS_H
#define BSP_KEYS_H

#include "stm32f10x.h"
#include "systick.h"

// 15 路按键：GPIO -> 按键 -> GND，上拉输入，按下=0
#define KEY_PORT_L3    GPIOA
#define KEY_PIN_L3     GPIO_Pin_7 // PA7  L_SW1  左摇杆按下
#define KEY_PORT_R3    GPIOA
#define KEY_PIN_R3     GPIO_Pin_6 // PA6  R_SW2  右摇杆按下

#define KEY_PORT_Y     GPIOB
#define KEY_PIN_Y      GPIO_Pin_6
#define KEY_PORT_B     GPIOB
#define KEY_PIN_B      GPIO_Pin_7
#define KEY_PORT_A     GPIOB
#define KEY_PIN_A      GPIO_Pin_8
#define KEY_PORT_X     GPIOB
#define KEY_PIN_X      GPIO_Pin_9

#define KEY_PORT_LB    GPIOB
#define KEY_PIN_LB     GPIO_Pin_15
#define KEY_PORT_RB    GPIOB
#define KEY_PIN_RB     GPIO_Pin_14

#define KEY_PORT_VIEW  GPIOB
#define KEY_PIN_VIEW   GPIO_Pin_5 // PB5  View
#define KEY_PORT_MENU  GPIOB
#define KEY_PIN_MENU   GPIO_Pin_4 // PB4  Menu
#define KEY_PORT_HOME  GPIOB
#define KEY_PIN_HOME   GPIO_Pin_3 // PB3  Home

#define KEY_PORT_UP    GPIOB
#define KEY_PIN_UP     GPIO_Pin_11 // PB11 UP-KEY
#define KEY_PORT_DOWN  GPIOB
#define KEY_PIN_DOWN   GPIO_Pin_1 // PB1  DOWN-KEY
#define KEY_PORT_LEFT  GPIOB
#define KEY_PIN_LEFT   GPIO_Pin_10 // PB10 LEFT-KEY
#define KEY_PORT_RIGHT GPIOB
#define KEY_PIN_RIGHT  GPIO_Pin_0 // PB0  RIGHT-KEY

// 按键索引枚举
typedef enum {
    KEY_A = 0,
    KEY_B,
    KEY_X,
    KEY_Y,

    KEY_LB,
    KEY_RB,

    KEY_VIEW,
    KEY_MENU,
    KEY_HOME,

    KEY_L3,
    KEY_R3,

    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_COUNT // 按键数量
} KEY;

#define KEY_DEBOUNCE_MS 5u   // 去抖确认次数 = 毫秒数（1kHz 采样）
#define KEY_HOLD_MS     600u // 长按判定阈值

// 初始化按键
void bsp_keys_init(void);

// 按键扫描, 这个函数需要 1ms 调一次
void bsp_keys_scan(void);

// 获取所有按键去抖后的稳定状态
uint16_t keys_get_state(void);

// 获取所有按键最近一次原始电平
uint16_t keys_get_raw(void);

// 获取按键按下事件
uint16_t keys_get_pressed(void);

// 获取按键松开事件
uint16_t keys_get_released(void);

// 判断按键是否按下，参数为按键索引枚举
uint8_t keys_is_down(uint8_t idx);

// 判断按键按下ms，参数为按键索引枚举
uint16_t keys_hold_ms(uint8_t idx);

#endif // BSP_KEYS_H