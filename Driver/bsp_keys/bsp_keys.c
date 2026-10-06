#include "bsp_keys.h"

// 按键映射结构体
typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} key_pin;

// 按键映射表
static key_pin s_key_map[] = {
    {KEY_PORT_A, KEY_PIN_A},        /* 0  A     PB8  */
    {KEY_PORT_B, KEY_PIN_B},        /* 1  B     PB7  */
    {KEY_PORT_X, KEY_PIN_X},        /* 2  X     PB9  */
    {KEY_PORT_Y, KEY_PIN_Y},        /* 3  Y     PB6  */
    {KEY_PORT_LB, KEY_PIN_LB},      /* 4  LB    PB15 */
    {KEY_PORT_RB, KEY_PIN_RB},      /* 5  RB    PB14 */
    {KEY_PORT_VIEW, KEY_PIN_VIEW},  /* 6  View  PB5  */
    {KEY_PORT_MENU, KEY_PIN_MENU},  /* 7  Menu  PB4  */
    {KEY_PORT_HOME, KEY_PIN_HOME},  /* 8  Home  PB3  */
    {KEY_PORT_L3, KEY_PIN_L3},      /* 9  L3    PA7  */
    {KEY_PORT_R3, KEY_PIN_R3},      /* 10 R3    PA6  */
    {KEY_PORT_UP, KEY_PIN_UP},      /* 11 UP    PB11 */
    {KEY_PORT_DOWN, KEY_PIN_DOWN},  /* 12 DOWN  PB1  */
    {KEY_PORT_LEFT, KEY_PIN_LEFT},  /* 13 LEFT  PB10 */
    {KEY_PORT_RIGHT, KEY_PIN_RIGHT} /* 14 RIGHT PB0  */
};

static uint16_t s_raw;                // 最近一次原始电平 uint16_t 每个位对应一个按键（1=按下）
static uint16_t s_state;              // 去抖后的稳定状态 uint16_t 每个位对应一个按键（1=按下）
static uint16_t s_evt_press;          // 上升沿事件（待取走）
static uint16_t s_evt_release;        // 下降沿事件（待取走）
static uint8_t s_cnt[KEY_COUNT];      // 每个按键扫描时电平与稳定状态不一致的计数（单位ms，要保证扫描函数每1ms调用一次）
static uint32_t s_down_ms[KEY_COUNT]; // 每个按键按下起始时间戳（单位ms）

static void GPIO_config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 打开时钟 GPIOA GPIOB AFIO(复用功能)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // 解除 PB3(JTDO)/PB4(NJTRST) 的 JTAG 占用，保留 SWD
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    // 上拉输入
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    // GPIOA组 L3 R3
    GPIO_InitStruct.GPIO_Pin = KEY_PIN_L3 | KEY_PIN_R3;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // GPIOB组 Y B A X 上 下 左 右 VIEW MENU HOME LB RB
    GPIO_InitStruct.GPIO_Pin = KEY_PIN_Y | KEY_PIN_B | KEY_PIN_A | KEY_PIN_X |
                               KEY_PIN_UP | KEY_PIN_DOWN | KEY_PIN_LEFT | KEY_PIN_RIGHT |
                               KEY_PIN_VIEW | KEY_PIN_MENU | KEY_PIN_HOME |
                               KEY_PIN_LB | KEY_PIN_RB;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}

// 读取按键原始电平
static uint16_t keys_read_raw(void)
{
    uint16_t raw = 0;
    uint8_t i;

    for (i = 0; i < KEY_COUNT; i++)
    {
        if (GPIO_ReadInputDataBit(s_key_map[i].port, s_key_map[i].pin) == Bit_RESET)
        {
            raw |= (uint16_t)(1u << i); /* 低电平 = 按下 */
        }
    }
    return raw;
}

// 初始化按键
void bsp_keys_init(void)
{
    GPIO_config();

    uint8_t i;
    // 读取上电瞬间所有按键原始电平
    s_raw = keys_read_raw();
    // 当前电平作为稳定状态
    s_state = s_raw;
    // 初始化事件标志
    s_evt_press = 0;
    s_evt_release = 0;

    for (i = 0; i < KEY_COUNT; i++)
    {
        // 上电时按住的键，立刻被认定为“已按下”，避免开机瞬间误动作
        s_cnt[i] = KEY_DEBOUNCE_MS;
        // 如果按键按下，则记录按下时间
        s_down_ms[i] = (s_raw & (1u << i)) ? get_ms() : 0;
    }
}

// 按键扫描
// 1ms 调一次
void bsp_keys_scan(void)
{
    uint8_t i;
    uint32_t now = get_ms();        // 本次扫描时间戳
    uint16_t raw = keys_read_raw(); // 本次扫描按键电平

    s_raw = raw;

    for (i = 0; i < KEY_COUNT; i++)
    {
        uint16_t raw_mask = (uint16_t)(1u << i);
        uint8_t key_state = (raw & raw_mask) ? 1u : 0u;        // 本次扫描的按键电平
        uint8_t stable_state = (s_state & raw_mask) ? 1u : 0u; // 对应按键的稳定状态

        if (key_state != stable_state)
        {
            // 本次扫描按键电平状态与稳定状态不一致，累加计数
            if (s_cnt[i] < KEY_DEBOUNCE_MS)
            {
                s_cnt[i]++;
            }

            // 达到消抖延时计数之后仍然不一致
            if (s_cnt[i] >= KEY_DEBOUNCE_MS)
            {
                // 计数清零准备下次
                s_cnt[i] = 0;

                // 确认按键按下或松开
                if (key_state)
                {
                    s_state |= raw_mask;     // 更新稳定状态
                    s_down_ms[i] = now;      // 记录按下时间戳
                    s_evt_press |= raw_mask; // 记录按键按下事件
                }
                else
                {
                    s_state &= (uint16_t)~raw_mask; // 更新稳定状态
                    s_down_ms[i] = 0;               // 清除按下时间戳
                    s_evt_release |= raw_mask;      // 记录松开事件
                }
            }
        }
        else
        {
            // 本次扫描按键电平状态与稳定状态一致，没有按下，计数清零
            s_cnt[i] = 0;
        }
    }
}

// 获取所有按键去抖后的稳定状态
uint16_t keys_get_state(void)
{
    return s_state;
}

// 获取所有按键最近一次原始电平
uint16_t keys_get_raw(void)
{
    return s_raw;
}

// 获取按键按下事件
uint16_t keys_get_pressed(void)
{
    uint16_t v = s_evt_press;
    s_evt_press = 0; // 读后清零：一次性事件
    return v;
}

// 获取按键松开事件
uint16_t keys_get_released(void)
{
    uint16_t v = s_evt_release;
    s_evt_release = 0; // 读后清零：一次性事件
    return v;
}

// 判断按键是否按下，参数为按键索引枚举
uint8_t keys_is_down(uint8_t idx)
{
    return (idx < KEY_COUNT) ? (uint8_t)((s_state >> idx) & 1u) : 0u;
}

// 判断按键按下ms，参数为按键索引枚举
uint16_t keys_hold_ms(uint8_t idx)
{
    if (idx >= KEY_COUNT || s_down_ms[idx] == 0)
    {
        return 0;
    }
    return (uint16_t)(get_ms() - s_down_ms[idx]); // 无符号回绕安全
}
