#ifndef APP_UI_H
#define APP_UI_H

#include "stm32f10x.h"

void app_ui_init(void);
void app_ui_task(void); /* 放主循环里轮询，内部限速 20Hz */

#endif