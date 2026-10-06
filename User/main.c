#include "stm32f10x.h"
#include "systick.h"
#include <stdio.h>

#include "bsp_keys.h"
#include "msp_uart.h"

/* 私有函数声明 */
void bsp_led_init(void);

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
    // KEYS
    bsp_keys_init();

    printf("============ start ============\n");
    printf("SystemCoreClock = %u\r\n", (unsigned int)SystemCoreClock);

    uint32_t t_print = 0;
    // 主循环
    while (1)
    {
        // led_test();

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

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    while (1)
    {
    }
}
#endif