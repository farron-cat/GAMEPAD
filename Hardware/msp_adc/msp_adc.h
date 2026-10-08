#ifndef MSP_ADC_H
#define MSP_ADC_H

#include "stm32f10x.h"

// 摇杆与扳机索引枚举
typedef enum {
    AXIS_LX = 0,
    AXIS_LY,
    AXIS_RX,
    AXIS_RY,
    AXIS_LT,
    AXIS_RT,
    AXIS_COUNT // 数量
} AXIS;

// 标定表
typedef struct {
    uint16_t min;      // ADC原始最小值
    uint16_t mid;      // ADC原始中位值
    uint16_t max;      // ADC原始最大值
    uint8_t invert;    // 方向标志 ==1 读数增大方向与"正向"相反（Y 轴通常要反
    uint16_t deadzone; // 中位死区（原始读数单位）
} axis_cal_t;

// 初始化
void msp_adc_init(void);

//
void msp_adc_poll(void);

// 获取原始值
uint16_t msp_adc_raw(AXIS id);

// 获取滤波后的值
uint16_t msp_adc_filtered(AXIS id);
void axis_cal_set(AXIS id, const axis_cal_t *cal);
const axis_cal_t *axis_cal_get(AXIS id);
void axis_cal_capture_mid(void);

// 标定
void axis_cal_set_default(void);

// 归一化
int8_t axis_get_signed(AXIS id);
uint8_t axis_get_unsigned(AXIS id);
uint8_t trigger_get_8bit(AXIS id);
uint8_t trigger_is_pressed(AXIS id);

#endif // MSP_ADC_H