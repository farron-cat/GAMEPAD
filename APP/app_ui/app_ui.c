#include "app_ui.h"
#include "bsp_keys.h"
#include "bsp_oled.h"
#include "msp_adc.h"
#include "systick.h"

#define UI_REFRESH_MS 50u /* 20Hz */
#define OLED_W 128
#define OLED_H 64
#define FONT_SIZE 8
#define MODE_NORMAL 1
#define LINE_H 8

static uint32_t s_t_last;
static uint8_t s_fps;

/* ---------------- 小工具 ---------------- */

/* 定宽字符串：右边自动补空格，覆盖掉上一帧残留 */
static void ui_str_fixed(uint8_t x, uint8_t y, const char *s, uint8_t width)
{
    char buf[32];
    uint8_t i = 0;
    if (width > sizeof(buf) - 1)
        width = sizeof(buf) - 1;

    while (s && s[i] && i < width)
    {
        buf[i] = s[i];
        i++;
    }
    while (i < width)
    {
        buf[i] = ' ';
        i++;
    }
    buf[i] = 0;

    bsp_oled_show_string(x, y, (uint8_t *)buf, FONT_SIZE, MODE_NORMAL);
}

/* 定宽十进制数字，右对齐补前导空格 */
static void ui_num_fixed(uint8_t x, uint8_t y, uint32_t v, uint8_t width)
{
    char buf[12];
    int i = width;
    buf[i] = 0;
    while (i-- > 0)
    {
        buf[i] = (char)('0' + (v % 10u));
        v /= 10u;
    }
    bsp_oled_show_string(x, y, (uint8_t *)buf, FONT_SIZE, MODE_NORMAL);
}

/* 定宽十六进制（大写） */
static void ui_hex_fixed(uint8_t x, uint8_t y, uint16_t v, uint8_t width)
{
    const char *hex = "0123456789ABCDEF";
    char buf[12];
    int i = width;
    buf[i] = 0;
    while (i-- > 0)
    {
        buf[i] = hex[v & 0xFu];
        v >>= 4;
    }
    bsp_oled_show_string(x, y, (uint8_t *)buf, FONT_SIZE, MODE_NORMAL);
}

/* ---------------- 初始化 ---------------- */

void app_ui_init(void)
{
    bsp_oled_init();

    /* 只在初始化时清一次屏 */
    bsp_oled_clear();
    bsp_oled_refresh();

    s_t_last = get_ms();
    s_fps = 0;
}

/* ---------------- 主任务 ---------------- */

void app_ui_task(void)
{
    uint32_t dt = get_ms() - s_t_last;
    if (dt < UI_REFRESH_MS)
        return;

    s_fps = (uint8_t)(1000u / (dt ? dt : 1u));
    s_t_last = get_ms();

    /* ---- 每行固定宽度覆盖，绝不清屏 ---- */

    /* y=0 : 标题 */
    ui_str_fixed(0, 0, "GamePad Debug", 13);

    /* y=8 : LX/LY */
    ui_str_fixed(0, 8, "LX", 2);
    ui_num_fixed(18, 8, axis_get_unsigned(AXIS_LX), 3);
    ui_str_fixed(48, 8, "LY", 2);
    ui_num_fixed(66, 8, axis_get_unsigned(AXIS_LY), 3);

    /* y=16 : RX/RY */
    ui_str_fixed(0, 16, "RX", 2);
    ui_num_fixed(18, 16, axis_get_unsigned(AXIS_RX), 3);
    ui_str_fixed(48, 16, "RY", 2);
    ui_num_fixed(66, 16, axis_get_unsigned(AXIS_RY), 3);

    /* y=24 : LT/RT */
    ui_str_fixed(0, 24, "LT", 2);
    ui_num_fixed(18, 24, trigger_get_8bit(AXIS_LT), 3);
    ui_str_fixed(48, 24, "RT", 2);
    ui_num_fixed(66, 24, trigger_get_8bit(AXIS_RT), 3);

    /* y=32 : KEY 十六进制 */
    ui_str_fixed(0, 32, "KEY", 3);
    ui_hex_fixed(24, 32, (uint16_t)keys_get_state(), 4);

    /* y=40 : 原始 ADC */
    ui_str_fixed(0, 40, "RAW", 3);
    ui_num_fixed(24, 40, msp_adc_raw(AXIS_LX), 4);
    ui_num_fixed(60, 40, msp_adc_raw(AXIS_LY), 4);

    /* y=48 : FPS */
    ui_str_fixed(0, 48, "FPS", 3);
    ui_num_fixed(24, 48, s_fps, 2);

    /* y=56 : 运行时间（秒） */
    ui_str_fixed(0, 56, "UP", 2);
    ui_num_fixed(24, 56, get_ms() / 1000u, 5);
    ui_str_fixed(72, 56, "s", 1);

    /* 如果你的驱动是"GRAM + refresh"结构，保留这行；
       如果 show_string 本身就直写 IIC，可以去掉 refresh 试试 */
    bsp_oled_refresh();
}