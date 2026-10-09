/* ---------- Middleware/usb_hid/usb_hid_app.c ---------- */
#include "usb_hid_app.h"
#include "app_hid_report_desc.h"
#include "hw_config.h"
#include "usb_desc.h"
#include "usb_lib.h"
#include "usb_pwr.h"

static volatile uint8_t s_ep1_busy = 0;

void usb_hid_init(void)
{
    Set_System();            /* PA11/PA12 = AF_PP */
    USB_Interrupts_Config(); /* 先开 NVIC（用 NVIC_PriorityGroup_2 分组） */
    Set_USBClock();          /* RCC_USBCLKSource_PLLCLK_1Div5 → 48MHz + 开 USB 外设时钟 */
    USB_Init();              /* 内部 → CustomHID_init() → PowerOn() → USB_SIL_Init() */

    s_ep1_busy = 0;
    /* bDeviceState 由 CustomHID_init() 置为 UNCONNECTED，这里不必手写 */
}

uint8_t usb_hid_is_configured(void)
{
    return (bDeviceState == CONFIGURED) ? 1u : 0u;
}

uint8_t usb_hid_send_report(const uint8_t *rpt, uint8_t len)
{
    if (bDeviceState != CONFIGURED)
    {
        return 1; /* PC 还没配置完（或已拔出）：丢这一帧 */
    }
    if (len != (uint8_t)HID_REPORT_SIZE)
    {
        return 3; /* 长度与报告描述符不一致：直接拒绝，别让主机读越界 */
    }
    if (s_ep1_busy)
    {
        return 2; /* 上一帧还没被主机取走：丢帧而不是阻塞主循环 */
    }

    /* ENDP1 = 1（usb_regs.h L108）；EP1 IN 在 PMA 里的地址来自 usb_conf.h 的 ENDP1_TXADDR */
    UserToPMABufferCopy((uint8_t *)rpt, GetEPTxAddr(ENDP1), len);
    SetEPTxCount(ENDP1, len);
    SetEPTxValid(ENDP1);
    s_ep1_busy = 1;
    return 0;
}

/* 由 usb_endp.c 的 EP1_IN_Callback() 调用：主机取走了数据 */
void usb_hid_on_ep1_in(void)
{
    s_ep1_busy = 0;
}

void usb_hid_on_reset(void)
{
    s_ep1_busy = 0; /* 掉线重连后必须清，否则永久 return 2（10 篇"断线恢复"验收项） */
}
