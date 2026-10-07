#include "stm32f10x.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_keys.h"
#include "bsp_rgb_leds.h"
#include "msp_uart.h"

/* 私有函数声明 */
void bsp_led_init(void);

void led_test(void);

void led_rgb_test(void);

void rgb_led_mono_test(void);

/* 主函数 */
int main(void)
{
    // 设置优先级分组： 2位抢占，2位子优先级
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    // 系统滴答时钟初始化
    systick_config();

    //============ 片上外设 ============
    // USART1
    msp_uart_init();

    //============ 片外外设 ============
    // LED
    bsp_led_init();
    // RGB LED
    bsp_rgb_leds_init();
    // KEYS
    bsp_keys_init();

    printf("============ start ============\n");
    printf("SystemCoreClock = %u\r\n", (unsigned int)SystemCoreClock);

    uint32_t t_print = 0;
    // 主循环
    while (1)
    {
        // led_test();
        led_rgb_test();
        // rgb_led_mono_test();

        if ((get_ms() - t_print) >= 100)
        {
            t_print = get_ms();
            printf("KEY map=%04X press=%04X\r\n",
                   keys_get_state(), keys_get_pressed());
        }
    }
}

/* LED 初始化 (PC13) */
void bsp_led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能 GPIOC 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* 配置 PC13 为推挽输出 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 默认熄灭 LED */
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void led_test(void)
{
    /* 点亮 PC13 LED (低电平点亮) */
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    // printf("LED ON\r\n");

    delay_1ms(1000);

    /* 熄灭 PC13 LED */
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
    // printf("LED OFF\r\n");

    delay_1ms(1000);
}

/* HSV → RGB */
static void hsv2rgb(uint16_t h, uint8_t s, uint8_t v,
                    uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t region, remainder, p, q, t;

    if (s == 0)
    {
        *r = *g = *b = v;
        return;
    }

    region = h / 60;
    remainder = (h - region * 60) * 255 / 60;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
    case 0:
        *r = v;
        *g = t;
        *b = p;
        break;
    case 1:
        *r = q;
        *g = v;
        *b = p;
        break;
    case 2:
        *r = p;
        *g = v;
        *b = t;
        break;
    case 3:
        *r = p;
        *g = q;
        *b = v;
        break;
    case 4:
        *r = t;
        *g = p;
        *b = v;
        break;
    default:
        *r = v;
        *g = p;
        *b = q;
        break;
    }
}

void led_rgb_test(void)
{
    static uint32_t t_last = 0;
    static uint16_t last_hue = 0xFFFF; /* 记录上次色相 */

    if ((get_ms() - t_last) < 50u)
    { /* ★ 20Hz，不是 50Hz */
        return;
    }
    t_last = get_ms();

    /* ★ 10 秒走一圈，比 4 秒慢一倍多 */
    uint16_t hue = (uint16_t)((get_ms() % 10000u) * 360u / 10000u);

    /* ★ 颜色没变就不刷新，减少关中断次数 */
    if (hue == last_hue)
    {
        return;
    }
    last_hue = hue;

    uint8_t r, g, b;
    hsv2rgb(hue, 255, 96, &r, &g, &b); /* ★ 亮度 32 → 96，更醒目 */

    bsp_rgb_leds_set_color(0, r, g, b);
    bsp_rgb_leds_refresh();
}

/* 单色测试：红 → 绿 → 蓝，每种保持 1 秒，三颗灯同时亮同一种颜色。
 * 串口会打印本次真正发送的 r/g/b，拿它和眼睛看到的颜色对照：
 * 颜色不对 → 灯珠线序不是 GRB；颜色对但还在闪 → 查 3.3V 驱动 5V 灯珠的电平余量。 */
void rgb_led_mono_test(void)
{
    static uint32_t t_last = 0;
    static uint8_t step = 0;
    static uint8_t started = 0;
    uint8_t r, g, b;

    if (started && (get_ms() - t_last) < 1000u)
    {
        return;
    }
    started = 1;
    t_last = get_ms();

    r = (step == 0u) ? 64u : 0u;
    g = (step == 1u) ? 64u : 0u;
    b = (step == 2u) ? 64u : 0u;

    bsp_rgb_leds_set_all_color(r, g, b);
    bsp_rgb_leds_refresh();

    printf("mono step %u: sent r=%u g=%u b=%u\r\n",
           (unsigned)step, (unsigned)r, (unsigned)g, (unsigned)b);

    step = (uint8_t)((step + 1u) % 3u);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    while (1)
    {
    }
}
#endif