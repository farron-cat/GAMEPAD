#ifndef BSP_IIC_OLED_H
#define BSP_IIC_OLED_H

#include <stdlib.h>
#include "stm32f10x.h"
#include "systick.h"
#include "bsp_iic_soft.h"

#define OLED_CMD  0 // 写命令
#define OLED_DATA 1 // 写数据

/* 函数声明 */
void bsp_oled_color_turn(uint8_t i);
void bsp_oled_display_turn(uint8_t i);

void bsp_oled_write_byte(uint8_t dat, uint8_t mode);

void bsp_oled_refresh(void);
void bsp_oled_clear(void);

void bsp_oled_draw_point(uint8_t x, uint8_t y, uint8_t t);
void bsp_oled_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode);
void bsp_oled_draw_circle(uint8_t x, uint8_t y, uint8_t r);

void bsp_oled_show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t size1, uint8_t mode);
void bsp_oled_show_string(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1, uint8_t mode);
uint32_t OLED_Pow(uint8_t m, uint8_t n);
void bsp_oled_show_number(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1, uint8_t mode);
void bsp_oled_show_chinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1, uint8_t mode);
void bsp_oled_scroll_display(uint8_t num, uint8_t space, uint8_t mode);
void bsp_oled_show_picture(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, uint8_t BMP[], uint8_t mode);

void bsp_oled_init(void);

#endif /* BSP_IIC_OLED_H */