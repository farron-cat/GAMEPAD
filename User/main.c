#include "stm32f10x.h"
#include "systick.h"
#include <stdio.h>

#include "app_hid.h"
#include "app_ui.h"
#include "bsp_iic_soft.h"
#include "bsp_keys.h"
#include "bsp_oled.h"
#include "bsp_rgb_leds.h"
#include "msp_adc.h"
#include "msp_uart.h"
#include "usb_conf.h"
#include "usb_hid_app.h"

/* 私有函数声明 */
void bsp_led_init(void);
void led_test(void);
void oled_test(void);

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
    // ADC
    msp_adc_init();

    //============ USB HID ============
    usb_hid_init();                             /* ① USB 设备栈 */
    app_hid_init();                             /* ② 07 篇：9 字节报告容器 */
    app_hid_set_transport(usb_hid_send_report); /* ③ 把传输层接上（唯一真理源约定） */

    //============ 片外外设 ============
    // LED
    bsp_led_init();
    // RGB LED
    bsp_rgb_leds_init();
    // KEYS
    bsp_keys_init();
    // OLED
    bsp_iic_soft_init();
    bsp_oled_init();

    //============    APP    ============
    // UI
    app_ui_init();

    printf("============ start ============\n");
    printf("SystemCoreClock = %u\r\n", (unsigned int)SystemCoreClock);

    uint32_t t_print = 0;
    // 主循环
    while (1)
    {

        app_hid_build_report(); /* 采输入 → 填 9 字节 */
        app_hid_flush();        /* 变化即发 + 保活（07 篇 §5 的发送策略） */

        // led_test();
        led_rgb_test();
        // rgb_led_mono_test();

        // oled_test();
        app_ui_task();

        if ((get_ms() - t_print) >= 1000)
        {
            t_print = get_ms();

            // printf("KEY map=%04X press=%04X\r\n",
            //        keys_get_state(), keys_get_pressed());

            /* 标定模式：每 200ms 打印原始值，人工把每个摇杆推到 8 个极限位置、扳机全行程 */
            printf("RAW LX=%4u LY=%4u RX=%4u RY=%4u LT=%4u RT=%4u\r\n",
                   msp_adc_raw(AXIS_LX), msp_adc_raw(AXIS_LY),
                   msp_adc_raw(AXIS_RX), msp_adc_raw(AXIS_RY),
                   msp_adc_raw(AXIS_LT), msp_adc_raw(AXIS_RT));
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

void oled_test(void)
{
    bsp_oled_show_string(0, 0, (uint8_t *)"hello gamepad!", 16, 1);
    bsp_oled_refresh();
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    while (1)
    {
    }
}
#endif