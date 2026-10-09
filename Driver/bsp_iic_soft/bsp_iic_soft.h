#ifndef BSP_IIC_SOFT_H
#define BSP_IIC_SOFT_H

#include "stm32f10x.h"

// IIC 软实现 初始化
void bsp_iic_soft_init(void);
// IIC 软实现 写入n个字节
uint8_t bsp_iic_soft_write_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);

uint8_t bsp_iic_soft_write_col_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t offset, uint16_t len);

// IIC 软实现 读取n个字节
uint8_t bsp_iic_soft_read_nbyte(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);

#endif // BSP_IIC_SOFT_H