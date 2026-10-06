#include "systick.h"

static volatile uint32_t delay = 0;  // 延时计数
static volatile uint64_t us_cnt = 0; // 微秒计数

void systick_config(void)
{
    // 设置systick timer 1000hz <=> 1ms
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1)
        {
        }
    }
    NVIC_SetPriority(SysTick_IRQn, 0x0F); /* 最低优先级：别抢 USB/WS2812 时序 */
}

void delay_1ms(uint32_t count)
{
    delay = count;
    while (0U != delay)
    {
    }
}

void delay_decrement(void)
{
    us_cnt++;

    if (0U != delay)
    {
        delay--;
    }
}

uint32_t get_ms(void)
{
    return us_cnt;
}
