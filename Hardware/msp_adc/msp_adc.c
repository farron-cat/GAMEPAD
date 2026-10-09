#include "msp_adc.h"
#include "systick.h"

#define ADC_BUF_LEN AXIS_COUNT // 6

static volatile uint16_t s_dma_buf[ADC_BUF_LEN]; // DMA 缓冲区 原始采样数据
static uint16_t s_filt[AXIS_COUNT];              // 滤波后的数据
static axis_cal_t s_cal[AXIS_COUNT];             // 标定数据 运行时使用

// 只读出厂默认标定：摇杆 min/mid/max 为 2026-10-08 实测回填（电位器实际接近满量程 0..4095）
// 扳机 0..4095（松=0）——⚠️ 与实测静置值(~1920)不符，待确认 H4/H5 后重标，见 DOC/实测记录.md B 表
static const axis_cal_t s_cal_default[AXIS_COUNT] = {
    /* AXIS_LX */ {5, 2040, 4093, 0, 40},
    /* AXIS_LY */ {7, 2004, 4090, 0, 40}, // Y 默认反向：推上 = 小值
    /* AXIS_RX */ {8, 2018, 4090, 0, 40},
    /* AXIS_RY */ {8, 2048, 4091, 0, 40},
    /* AXIS_LT */ {0, 0, 4095, 1, 0},
    /* AXIS_RT */ {0, 0, 4095, 1, 0}};

void msp_adc_init(void)
{
    // ADC
    // 打开时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    // ADC时钟为72MHz/6=12MHz
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    // 配置ADC多通道连续扫描
    ADC_InitTypeDef ADC_InitStruct;
    ADC_StructInit(&ADC_InitStruct);
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;                  // 独立模式 ADC1 单独工作，和 ADC2 无关
    ADC_InitStruct.ADC_ScanConvMode = ENABLE;                        // 多通道扫描模式
    ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;                  // 连续转换模式
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 不使用外部触发
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;              // 数据右对齐 12位数据在16位寄存器的低12位
    ADC_InitStruct.ADC_NbrOfChannel = ADC_BUF_LEN;                   // 转换通道数
    ADC_Init(ADC1, &ADC_InitStruct);

    // 配置ADC通道 PCB上的摇杆旋转了90度，导致XY接反了，之后修改第二版的pcb
    // ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5); // AXIS_LX
    // ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_55Cycles5); // AXIS_LY
    // ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_55Cycles5); // AXIS_RX
    // ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_55Cycles5); // AXIS_RY
    // ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_55Cycles5); // AXIS_LT
    // ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_55Cycles5); // AXIS_RT
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5); // AXIS_LX
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 2, ADC_SampleTime_55Cycles5); // AXIS_LY
    ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_55Cycles5); // AXIS_RX
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 4, ADC_SampleTime_55Cycles5); // AXIS_RY
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_55Cycles5); // AXIS_LT
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_55Cycles5); // AXIS_RT

    // DMA
    // 打开时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA1_Channel1);
    DMA_InitTypeDef DMA_InitStruct;
    DMA_StructInit(&DMA_InitStruct);
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;             // 外设地址
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)s_dma_buf;                 // 内存地址
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;                          // 外设到内存
    DMA_InitStruct.DMA_BufferSize = ADC_BUF_LEN;                             // 一次搬运数据量
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            // 外设地址不递增
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;                     // 内存地址递增
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 外设数据宽度 每次搬运多少位到外设 半字16位
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         // 内存数据宽度 每次搬运多少位到内存 半字16位
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;                             // 循环模式
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;                         // 高优先级
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;                                // 禁止内存到内存模式
    DMA_Init(DMA1_Channel1, &DMA_InitStruct);

    // 使能DMA
    DMA_Cmd(DMA1_Channel1, ENABLE);
    // 使能DMA请求
    ADC_DMACmd(ADC1, ENABLE);
    // 使能ADC
    ADC_Cmd(ADC1, ENABLE);

    // ADC校准 在ADC使能之后
    ADC_ResetCalibration(ADC1);                        // 复位校准寄存器，清除上次校准值
    while (SET == ADC_GetResetCalibrationStatus(ADC1)) // 等待复位完成 判断RSTCAL==1说明还在复位，复位完成后会==0
        ;
    ADC_StartCalibration(ADC1);                   // 开始校准
    while (SET == ADC_GetCalibrationStatus(ADC1)) // 等待校准完成 判断CAL==1说明还在校准，校准完成后会==0
        ;

    // 软件启动ADC转换
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // 设定摇杆和扳机的校准参数
    axis_cal_set_default();

    // 初始化滤波
    msp_adc_poll();
}

// 指数移动平均（EMA）滤波器，用于平滑ADC采样值
#define EMA_SHIFT 3 // 移位位数，用于计算EMA的α值 越大平滑效果越好，但延迟也越大

// 1ms 调一次
void msp_adc_poll(void)
{
    uint8_t i;

    for (i = 0; i < AXIS_COUNT; i++)
    {
        uint16_t sample = s_dma_buf[i];
        // 滤波计算
        // 一阶指数平均：只用移位，无除法、无浮点
        // 新值 = 旧值 × (1 - α) + 采样值 × α
        // s_filt[i] = s_filt[i] * (1 - 1/8) + sample * (1/8)
        // α = 1/8 = 1 >> 3
        s_filt[i] = (uint16_t)((s_filt[i] - (s_filt[i] >> EMA_SHIFT)) + (sample >> EMA_SHIFT));
    }
}

// 获取原始值
uint16_t msp_adc_raw(AXIS id)
{
    return (id < AXIS_COUNT) ? s_dma_buf[id] : 0;
}

// 获取滤波后的值
uint16_t msp_adc_filtered(AXIS id)
{
    return (id < AXIS_COUNT) ? s_filt[id] : 0;
}

// 标定
// 设定摇杆和扳机的校准参数
void axis_cal_set_default(void)
{
    uint8_t i;
    for (i = 0; i < AXIS_COUNT; i++)
    {
        s_cal[i] = s_cal_default[i]; // 使用只读的默认值
    }
}

void axis_cal_set(AXIS id, const axis_cal_t *cal)
{
    if (id < AXIS_COUNT && cal != 0)
    {
        s_cal[id] = *cal;
    }
}

const axis_cal_t *axis_cal_get(AXIS id)
{
    return (id < AXIS_COUNT) ? &s_cal[id] : 0;
}

/* 上电静止时抓中点：解决"电位器回中偏差 + 温漂"
   注意：调用时手必须离开摇杆；建议取 32 次平均 */
void axis_cal_capture_mid(void)
{
    uint8_t i, k;
    uint32_t sum[AXIS_COUNT] = {0};

    for (k = 0; k < 32u; k++)
    {
        for (i = 0; i < AXIS_COUNT; i++)
        {
            sum[i] += msp_adc_raw((AXIS)i);
        }
        delay_1ms(2);
    }
    for (i = 0; i < AXIS_COUNT; i++)
    {
        // 扳机没有"中点"概念
        if (i == AXIS_LT || i == AXIS_RT)
        {
            continue;
        }
        s_cal[i].mid = (uint16_t)(sum[i] / 32u);
    }
}

// 归一化
/* 摇杆归一化：-127..+127，中位 0（带死区） */
int8_t axis_get_signed(AXIS id)
{
    const axis_cal_t *c;
    int32_t v, center, span, out;

    if (id >= AXIS_COUNT)
    {
        return 0;
    }
    c = &s_cal[id];
    v = s_filt[id];
    if (c->invert)
    {
        v = (int32_t)c->max + (int32_t)c->min - v; /* 反向 */
    }

    /* 死区：中位附近直接归零，避免摇杆"自己漂" */
    center = c->mid;
    if (v > center - (int32_t)c->deadzone && v < center + (int32_t)c->deadzone)
    {
        return 0;
    }

    if (v >= center)
    {
        span = (int32_t)c->max - center;
        out = (span > 0) ? ((v - center) * 127) / span : 0;
        if (out > 127)
        {
            out = 127;
        }
    }
    else
    {
        span = center - (int32_t)c->min;
        out = (span > 0) ? ((v - center) * 127) / span : 0;
        if (out < -127)
        {
            out = -127;
        }
    }
    return (int8_t)out;
}

/* 摇杆归一化：0..255，中位 128（HID 8 位无符号） */
uint8_t axis_get_unsigned(AXIS id)
{
    return (uint8_t)((int16_t)axis_get_signed(id) + 128);
}

/* 扳机归一化：0..255，0=松开（摇杆标定表的 mid 字段对扳机无意义） */
uint8_t trigger_get_8bit(AXIS id)
{
    const axis_cal_t *c;
    int32_t v, span, out;

    if (id >= AXIS_COUNT)
    {
        return 0;
    }
    c = &s_cal[id];
    v = (int32_t)s_filt[id];
    if (c->invert)
    {
        v = (int32_t)c->max + (int32_t)c->min - v;
    }

    span = (int32_t)c->max - (int32_t)c->min;
    if (span <= 0)
    {
        return 0;
    }
    out = ((v - (int32_t)c->min) * 255) / span;
    if (out < 0)
    {
        out = 0;
    }
    if (out > 255)
    {
        out = 255;
    }
    return (uint8_t)out;
}

/* 扳机数字量（给 HID 按钮域 B7/B8 用：行程超过 50% 算按下） */
#define TRIGGER_DIGITAL_TH 128
uint8_t trigger_is_pressed(AXIS id)
{
    return (trigger_get_8bit(id) >= TRIGGER_DIGITAL_TH) ? 1u : 0u;
}