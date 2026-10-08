#include "bsp_rgb_leds.h"
#include "msp_timer.h"
#include "systick.h"
#include <stdio.h>

#define CCR_ZERO 29                                 /* 0.403µs（0 码高电平） */
#define CCR_ONE 58                                  /* 0.806µs（1 码高电平） */
#define RESET_SLOTS 64                              /* 复位低电平 64×1.25µs = 80µs */
#define BUF_LEN (RGB_LEDS_COUNT * 24 + RESET_SLOTS) /* 136 */

static rgb_t s_rgb_leds_color[RGB_LEDS_COUNT];
static uint16_t s_dma_buf[BUF_LEN];

// ws2812 RGB LED 初始化
void bsp_rgb_leds_init(void)
{
    // 初始化GPIO TIMER(PWM) DMA
    msp_ws2812_timer_init();
    // 清除
    bsp_rgb_leds_clear();
    // 刷新
    bsp_rgb_leds_refresh();
}

// 设置指定LED颜色
void bsp_rgb_leds_set_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index < RGB_LEDS_COUNT)
    {
        s_rgb_leds_color[index].r = r;
        s_rgb_leds_color[index].g = g;
        s_rgb_leds_color[index].b = b;
    }
}

// 设置所有LED颜色
void bsp_rgb_leds_set_all_color(uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = 0; i < RGB_LEDS_COUNT; i++)
    {
        bsp_rgb_leds_set_color(i, r, g, b);
    }
}

// 清空所有LED
void bsp_rgb_leds_clear(void)
{
    bsp_rgb_leds_set_all_color(0, 0, 0);
}

// 写一个bit到DMA缓冲
static void encode_bit(uint16_t *p, uint8_t bit)
{
    *p = bit ? CCR_ONE : CCR_ZERO;
}

// 写一个字节到DMA缓冲（MSB 先行）
static uint16_t *encode_byte(uint16_t *p, uint8_t v)
{
    uint8_t i;
    for (i = 0; i < 8u; i++)
    {
        encode_bit(p, (uint8_t)((v >> (7u - i)) & 1u));
        p++;
    }
    return p;
}

// 刷新
void bsp_rgb_leds_refresh(void)
{
    uint16_t *p = s_dma_buf;
    uint8_t i;

    for (i = 0; i < RGB_LEDS_COUNT; i++)
    {
        // GRB
        p = encode_byte(p, s_rgb_leds_color[i].g); // 先绿
        p = encode_byte(p, s_rgb_leds_color[i].r);
        p = encode_byte(p, s_rgb_leds_color[i].b);
    }
    for (i = 0; i < RESET_SLOTS; i++)
    {
        *p++ = 0; // 复位间隔：CCR=0 → 全程低
    }

    // 时序敏感区：关中断 170µs，避免任何 ISR 打断时序
    __disable_irq();
    msp_ws2812_dma_send(s_dma_buf, BUF_LEN);
    __enable_irq();
}

// ================
/* 调试用：打印缓冲区长度与缓冲区前 count 个值（不要放在灯效循环里调用！
 * 一帧只应该花 ~170µs，printf 一帧要 2ms 以上，会拖慢主循环） */
void bsp_rgb_leds_dump(uint8_t count)
{
    uint8_t i;

    if (count > BUF_LEN)
    {
        count = BUF_LEN;
    }

    printf("BUF_LEN=%u, RGB_LEDS_COUNT=%u, buf[0..%u]=",
           (unsigned)BUF_LEN, (unsigned)RGB_LEDS_COUNT, (unsigned)count);
    for (i = 0; i < count; i++)
    {
        printf("%u ", (unsigned)s_dma_buf[i]);
    }
    printf("\r\n");
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
    static uint16_t last_hue = 0xFFFF; // 记录上次色相

    if ((get_ms() - t_last) < 50u) // 20Hz 50ms
    {
        return;
    }
    t_last = get_ms();

    uint16_t hue = (uint16_t)((get_ms() % 10000u) * 360u / 10000u);

    // 颜色没变就不刷新，减少关中断次数
    if (hue == last_hue)
    {
        return;
    }
    last_hue = hue;

    uint8_t r, g, b;
    hsv2rgb(hue, 255, 96, &r, &g, &b);

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