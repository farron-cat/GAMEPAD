/* ---------- Middleware/usb_hid/hw_config.c ---------- */
/* 由例程 Projects/Custom_HID/src/hw_config.c 裁剪而来（V4.1.0）
   裁剪原则：只保留 USB 必需的 5 个函数；删掉评估板的 LED/按键/摇杆/EXTI/ADC 部分 */

#include "hw_config.h"
#include "usb_desc.h" /* CustomHID_StringSerial（★ 非 const，可写） */
#include "usb_lib.h"

/* ① 系统与 USB 引脚：PA11/PA12 = 复用推挽（F103 必须显式配） */
void Set_System(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* ★ 不要照抄例程的 RCC_APB2Periph_ALLGPIO 与 USB_DISCONNECT 配置 */
}

/* ② 48MHz：PLL / 1.5 = 48MHz；再开 USB 外设时钟 */
void Set_USBClock(void)
{
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

/* ③ USB 中断：只开"USB 低优先级"和"唤醒"，不要再碰 EXTI / DMA */
void USB_Interrupts_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
}

/* ④ 本板没有"软件断开"电路 ⇒ 空实现，但函数体必须存在（usb_pwr.c 会调） */
void USB_Cable_Config(FunctionalState NewState)
{
    (void)NewState;
}

/* ⑤ UID（96 位唯一 ID）→ 字符串描述符 3
      地址取自例程 platform_config.h 的 STM32F10X_MD 分支（第 92~94 行）；
      这里直接写死，省掉一个 platform_config.h 的依赖 */
#define UID_ADDR0 ((volatile uint32_t *)0x1FFFF7E8)
#define UID_ADDR1 ((volatile uint32_t *)0x1FFFF7EC)
#define UID_ADDR2 ((volatile uint32_t *)0x1FFFF7F0)

static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len)
{
    uint8_t idx = 0;

    for (idx = 0; idx < len; idx++)
    {
        if (((value >> 28)) < 0xA)
        {
            pbuf[2 * idx] = (uint8_t)((value >> 28) + '0');
        }
        else
        {
            pbuf[2 * idx] = (uint8_t)((value >> 28) + 'A' - 10);
        }
        value = value << 4;
        pbuf[2 * idx + 1] = 0;
    }
}

void Get_SerialNum(void)
{
    uint32_t Device_Serial0 = *UID_ADDR0;
    uint32_t Device_Serial1 = *UID_ADDR1;
    uint32_t Device_Serial2 = *UID_ADDR2;

    Device_Serial0 += Device_Serial2; /* 例程就这么写的：把第三个字加到第一个 */

    if (Device_Serial0 != 0)
    {
        /* CustomHID_StringSerial 长度 26 = 2 + 2×12：前 8 字符 + 后 4 字符 */
        IntToUnicode(Device_Serial0, &CustomHID_StringSerial[2], 8);
        IntToUnicode(Device_Serial1, &CustomHID_StringSerial[18], 4);
    }
}
