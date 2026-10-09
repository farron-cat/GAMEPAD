/* ---------- Middleware/usb_hid/usb_hid_app.h ---------- */
#ifndef __USB_HID_APP_H__
#define __USB_HID_APP_H__
#include "stm32f10x.h"

void usb_hid_init(void);                                      /* 初始化 USB 设备栈 */
uint8_t usb_hid_is_configured(void);                          /* PC 是否已完成配置 */
uint8_t usb_hid_send_report(const uint8_t *rpt, uint8_t len); /* 发一帧报告 */
void usb_hid_on_ep1_in(void);
void usb_hid_on_reset(void);
#endif
