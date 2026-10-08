#include "msp_timer.h"

void msp_ws2812_timer_init(void)
{

    // GPIO配置
    // 打开时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    // PA8 TIM1_CH1
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = WS2812_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(WS2812_PORT, &GPIO_InitStructure);

    // TIMER配置(PWM)
    // 打开时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    // 目标1.25us 800kHz
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Prescaler = 0;                    // 预分频 计数时钟 = 72MHz / (PSC + 1) = 72MHz / 1 = 72MHz
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseStructure.TIM_Period = 90 - 1;                  // 计数器周期  72Mhz <=> 1s <=> 1000000us => 72 <=> 1us 目标PWM周期 1.25us => 90 <=> 1.25us
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 时钟分频 给数字滤波器用的分频，不影响计数频率和 PWM 周期
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;            // 重复计数器 只有高级定时器（TIM1/TIM8）才有，决定"数多少次溢出才产生一次事件"
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    // CH1 PWM1
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;             // PWM输出比较模式1 正相 PWM：CNT < CCR1 输出高，CNT ≥ CCR1 输出低
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 输出使能 允许该通道的 PWM 波形送到引脚
    TIM_OCInitStructure.TIM_Pulse = 0;                            // 占空比 初始占空比为0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     // 输出极性
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    // 必须开预装载（OC1PE=1）
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    // 打开总开关MOE
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    // DMA配置
    // 打开时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_InitTypeDef DMA_InitStructure;
    DMA_StructInit(&DMA_InitStructure);
    DMA_DeInit(DMA1_Channel2);                                                  // DMA1_CH2
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&TIM1->CCR1;           // 外设地址
    DMA_InitStructure.DMA_MemoryBaseAddr = 0;                                   // 内存地址 保留，发送时填
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                          // 内存到外设
    DMA_InitStructure.DMA_BufferSize = 1;                                       // 一次搬运数量 保留，发送时填
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            // 外设地址不自增
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     // 内存地址自增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 外设数据宽度 每次搬运多少位到外设 半字16位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         // 内存数据宽度 每次搬运多少位到内存 半字16位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                               // 单次模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                     // 优先级 设置为最高
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                // 内存到内存模式 禁用
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);
    // 清理DMA标志
    // TC2 传输完成标志，HT2 半传输标志，TE2 传输错误标志，GL2 全局标志(前三个的或)
    DMA_ClearFlag(DMA1_FLAG_TC2 | DMA1_FLAG_HT2 | DMA1_FLAG_TE2 | DMA1_FLAG_GL2);

    // 使能
    TIM_DMACmd(TIM1, TIM_DMA_CC1, ENABLE); // 捕获/比较通道1的DMA请求
    TIM_Cmd(TIM1, ENABLE);                 // 启动定时器
}

void msp_ws2812_dma_send(const uint16_t *buf, uint16_t len)
{
    // 关闭DMA使能
    DMA_Cmd(DMA1_Channel2, DISABLE);
    // 配置数据长度
    DMA_SetCurrDataCounter(DMA1_Channel2, len);
    // 配置内存源地址
    DMA1_Channel2->CMAR = (uint32_t)buf;
    // 清理DMA标志
    DMA_ClearFlag(DMA1_FLAG_TC2 | DMA1_FLAG_GL2);
    // 开启DMA使能
    DMA_Cmd(DMA1_Channel2, ENABLE);

    // 等待搬运完成
    while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET)
        ;
    // 清理TC2搬运完成标志
    DMA_ClearFlag(DMA1_FLAG_TC2);
    // 关闭DMA使能
    DMA_Cmd(DMA1_Channel2, DISABLE);
}
