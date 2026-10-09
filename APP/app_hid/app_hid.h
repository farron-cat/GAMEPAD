/* ---------- App/app_hid.h ---------- */
#ifndef __APP_HID_H__
#define __APP_HID_H__

#include "stm32f10x.h"

#define HID_REPORT_SIZE 9u

/* 传输层函数原型（★ 返回值不能是 void）：
   0 = 成功 / 1 = 主机未配置 / 2 = 端点忙（丢帧）/ 3 = 长度不符
   USB 的 usb_hid_send_report() 与蓝牙的 ble_hid_send_report() 都是这个签名 */
typedef uint8_t (*app_hid_transport_t)(const uint8_t *rpt, uint8_t len);

void app_hid_init(void);
void app_hid_build_report(void);     /* 每 1ms：采集结果 → 9 字节报告 */
void app_hid_flush(void);            /* 有变化或到周期 → 交给传输层 */
const uint8_t *app_hid_report(void); /* 读当前报告（调试/OLED 用） */
void app_hid_set_transport(app_hid_transport_t send);
void app_hid_get_stats(uint32_t *sent, uint32_t *changed);

#endif
