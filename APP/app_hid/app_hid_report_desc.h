/* ---------- App/hid_report_desc.h ---------- */
/* STM32 GamePad 报告描述符：9 字节报告
   byte0    : Buttons 1..8
   byte1    : Buttons 9..15 + 1 位填充
   byte2..5 : X, Y, Rx, Ry   （左摇杆、右摇杆；0..255，128 居中）
   byte6..7 : Z, Rz          （LT、RT；0..255，0 = 松开）
   byte8    : Hat（低 4 位，0..7，0x0F = 空）+ 高 4 位填充          */
#ifndef __HID_REPORT_DESC_H__
#define __HID_REPORT_DESC_H__

#define HID_REPORT_SIZE 9u /* 报告总字节数（务必与描述符一致） */

static const uint8_t s_hid_report_desc[] = {
    0x05, 0x01, /* Usage Page (Generic Desktop)              0x01 */
    0x09, 0x05, /* Usage (Game Pad)                          0x05 */
    0xA1, 0x01, /* Collection (Application)                        */

    /* ---- 按钮域：Buttons 1..15（15 位）+ 1 位填充 ---- */
    0x05, 0x09, /*   Usage Page (Button)                     0x09 */
    0x19, 0x01, /*   Usage Minimum (Button 1)                0x01 */
    0x29, 0x0F, /*   Usage Maximum (Button 15)               0x0F */
    0x15, 0x00, /*   Logical Minimum (0)                          */
    0x25, 0x01, /*   Logical Maximum (1)                          */
    0x75, 0x01, /*   Report Size (1 bit)                          */
    0x95, 0x0F, /*   Report Count (15)   → 15 bit                 */
    0x81, 0x02, /*   Input (Data,Var,Abs) → byte0 + byte1[0..6]   */
    0x95, 0x01, /*   Report Count (1)                             */
    0x81, 0x03, /*   Input (Const,Var,Abs) → 填 1 bit（凑满 16）   */

    /* ---- 左/右摇杆：X, Y, Rx, Ry（各 8 位，0..255） ---- */
    0x05, 0x01,       /*   Usage Page (Generic Desktop)                 */
    0x09, 0x30,       /*   Usage (X)   左摇杆水平                       */
    0x09, 0x31,       /*   Usage (Y)   左摇杆垂直                       */
    0x09, 0x33,       /*   Usage (Rx)  右摇杆水平                       */
    0x09, 0x34,       /*   Usage (Ry)  右摇杆垂直                       */
    0x15, 0x00,       /*   Logical Minimum (0)                          */
    0x26, 0xFF, 0x00, /*   Logical Maximum (255)  ← 2 字节写法           */
    0x75, 0x08,       /*   Report Size (8 bit)                          */
    0x95, 0x04,       /*   Report Count (4)     → byte2..byte5          */
    0x81, 0x02,       /*   Input (Data,Var,Abs)                         */

    /* ---- 扳机：Z, Rz（各 8 位，0=松开，255=到底） ---- */
    0x09, 0x32, /*   Usage (Z)   左扳机 LT                        */
    0x09, 0x35, /*   Usage (Rz)  右扳机 RT                        */
    0x95, 0x02, /*   Report Count (2)     → byte6..byte7          */
    0x81, 0x02, /*   Input (Data,Var,Abs)                         */
    /* Logical Max(255)/Report Size(8) 沿用全局状态，无需重复声明 */

    /* ---- 十字键：Hat switch（4 位，0..7，允许空值） ---- */
    0x09, 0x39,       /*   Usage (Hat switch)                           */
    0x15, 0x00,       /*   Logical Minimum (0)                          */
    0x25, 0x07,       /*   Logical Maximum (7)                          */
    0x35, 0x00,       /*   Physical Minimum (0)                         */
    0x46, 0x3B, 0x01, /*   Physical Maximum (315) ← 8 方向 ×45°          */
    0x65, 0x14,       /*   Unit (Eng Rot: Degrees)                      */
    0x75, 0x04,       /*   Report Size (4 bit)                          */
    0x95, 0x01,       /*   Report Count (1)                             */
    0x81, 0x42,       /*   Input (Data,Var,Abs,Null State) → byte8[0..3] */
    0x65, 0x00,       /*   Unit (None)                                  */
    0x75, 0x04,       /*   Report Size (4 bit)                          */
    0x95, 0x01,       /*   Report Count (1)                             */
    0x81, 0x03,       /*   Input (Const,Var,Abs) → 填 byte8[4..7]        */

    0xC0 /* End Collection                                 */
};

#define HID_REPORT_DESC_LEN (sizeof(s_hid_report_desc)) /* = 83 字节 = 0x53（已逐字节核对） */

#endif
