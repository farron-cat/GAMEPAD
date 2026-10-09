/**
 ******************************************************************************
 * @file    usb_desc.c
 * @author  MCD Application Team
 * @version V4.1.0
 * @date    26-May-2017
 * @brief   Descriptors for Custom HID Demo
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usb_desc.h"
#include "usb_lib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* USB Standard Device Descriptor */
const uint8_t CustomHID_DeviceDescriptor[CUSTOMHID_SIZ_DEVICE_DESC] =
    {
        0x12,                       /*bLength */
        USB_DEVICE_DESCRIPTOR_TYPE, /*bDescriptorType*/
        0x00,                       /*bcdUSB */
        0x02,
        0x00, /*bDeviceClass*/
        0x00, /*bDeviceSubClass*/
        0x00, /*bDeviceProtocol*/
        0x40, /*bMaxPacketSize40*/
        0x34, /*idVendor (0x1234)*/
        0x12,
        0x78, /*idProduct = 0x5678*/
        0x56,
        0x00, /*bcdDevice rel. 2.00*/
        0x02,
        1,   /*Index of string descriptor describing
                           manufacturer */
        2,   /*Index of string descriptor describing
                          product*/
        3,   /*Index of string descriptor describing the
                          device serial number */
        0x01 /*bNumConfigurations*/
}; /* CustomHID_DeviceDescriptor */

/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
const uint8_t CustomHID_ConfigDescriptor[CUSTOMHID_SIZ_CONFIG_DESC] =
    {
        0x09,                              /* bLength: Configuration Descriptor size */
        USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
        CUSTOMHID_SIZ_CONFIG_DESC,
        /* wTotalLength: Bytes returned */
        0x00,
        0x01, /* bNumInterfaces: 1 interface */
        0x01, /* bConfigurationValue: Configuration value */
        0x00, /* iConfiguration: Index of string descriptor describing
                             the configuration*/
        0x80, /* bmAttributes: Bus-powered */
        0x50, /* MaxPower 160 mA: this current is used for detecting Vbus */

        /************** Descriptor of Custom HID interface ****************/
        /* 09 */
        0x09,                          /* bLength: Interface Descriptor size */
        USB_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType: Interface descriptor type */
        0x00,                          /* bInterfaceNumber: Number of Interface */
        0x00,                          /* bAlternateSetting: Alternate setting */
        0x01,                          /* bNumEndpoints */
        0x03,                          /* bInterfaceClass: HID */
        0x00,                          /* bInterfaceSubClass : 1=BOOT, 0=no boot */
        0x00,                          /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
        0,                             /* iInterface: Index of string descriptor */
        /******************** Descriptor of Custom HID HID ********************/
        /* 18 */
        0x09,                /* bLength: HID Descriptor size */
        HID_DESCRIPTOR_TYPE, /* bDescriptorType: HID */
        0x10,                /* bcdHID: HID Class Spec release number */
        0x01,
        0x00,                      /* bCountryCode: Hardware target country */
        0x01,                      /* bNumDescriptors: Number of HID class descriptors to follow */
        0x22,                      /* bDescriptorType */
        CUSTOMHID_SIZ_REPORT_DESC, /* wItemLength: Total length of Report descriptor */
        0x00,
        /******************** Descriptor of Custom HID endpoints ******************/
        /* 27 */
        0x07,                         /* bLength: Endpoint Descriptor size */
        USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: */

        0x81, /* bEndpointAddress: Endpoint Address (IN) */
        0x03, /* bmAttributes: Interrupt endpoint */
        0x09, /* wMaxPacketSize: 9 Bytes max */
        0x00,
        0x01, /* bInterval: Polling Interval (1 ms) */
              /* 34 */
}; /* CustomHID_ConfigDescriptor */
const uint8_t CustomHID_ReportDescriptor[CUSTOMHID_SIZ_REPORT_DESC] =
    {
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
}; /* CustomHID_ReportDescriptor */

/* USB String Descriptors (optional) */
const uint8_t CustomHID_StringLangID[CUSTOMHID_SIZ_STRING_LANGID] =
    {
        CUSTOMHID_SIZ_STRING_LANGID,
        USB_STRING_DESCRIPTOR_TYPE,
        0x09,
        0x04}; /* LangID = 0x0409: U.S. English */

const uint8_t CustomHID_StringVendor[CUSTOMHID_SIZ_STRING_VENDOR] =
    {
        CUSTOMHID_SIZ_STRING_VENDOR, /* Size of Vendor string */
        USB_STRING_DESCRIPTOR_TYPE,  /* bDescriptorType */
        /* Manufacturer: "made by Farron Cat" */
        'm', 0, 'a', 0, 'd', 0, 'e', 0, ' ', 0,
        'b', 0, 'y', 0, ' ', 0,
        'F', 0, 'a', 0, 'r', 0, 'r', 0, 'o', 0, 'n', 0,
        ' ', 0,
        'C', 0, 'a', 0, 't', 0};

const uint8_t CustomHID_StringProduct[CUSTOMHID_SIZ_STRING_PRODUCT] =
    {
        CUSTOMHID_SIZ_STRING_PRODUCT, /* bLength */
        USB_STRING_DESCRIPTOR_TYPE,   /* bDescriptorType */
        'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0, ' ', 0,
        'G', 0, 'a', 0, 'm', 0, 'e', 0, 'P', 0, 'a', 0, 'd', 0,
        ' ', 0,
        'F', 0, 'a', 0, 'r', 0, 'r', 0, 'o', 0, 'n', 0,
        ' ', 0,
        'C', 0, 'a', 0, 't', 0};
uint8_t CustomHID_StringSerial[CUSTOMHID_SIZ_STRING_SERIAL] =
    {
        CUSTOMHID_SIZ_STRING_SERIAL, /* bLength */
        USB_STRING_DESCRIPTOR_TYPE,  /* bDescriptorType */
        'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0};

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
