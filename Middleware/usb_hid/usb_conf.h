/**
 ******************************************************************************
 * @file    usb_conf.h
 * @author  MCD Application Team
 * @version V4.1.0
 * @date    26-May-2017
 * @brief   Custom HID demo configuration file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CONF_H
#define __USB_CONF_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* External variables --------------------------------------------------------*/
/*-------------------------------------------------------------*/
/* EP_NUM */
/* defines how many endpoints are used by the device */
/*-------------------------------------------------------------*/
#define EP_NUM (2)

/*-------------------------------------------------------------*/
/* --------------   Buffer Description Table  -----------------*/
/*-------------------------------------------------------------*/
/* buffer table base address */
/* buffer table base address */
#define BTABLE_ADDRESS (0x00)

/* EP0  */
/* rx/tx buffer base address */
#define ENDP0_RXADDR (0x18)
#define ENDP0_TXADDR (0x58)

/* EP1  */
/* tx buffer base address */
#define ENDP1_TXADDR (0x100)
#define ENDP1_RXADDR (0x104)

/*-------------------------------------------------------------*/
/* -------------------   ISTR events  -------------------------*/
/*-------------------------------------------------------------*/
/* IMR_MSK */
/* mask defining which events has to be handled */
/* by the device application software */

/* 固定优先级中断掩码
   ★ CNTR_CTRM 绝不能少：usb_istr.c 第 94 行是 `#if (IMR_MSK & ISTR_CTR)`，
     属于"编译期"开关 —— 少了它，CTR_LP() 整段被编译掉，
     端点回调（EP1_IN_Callback）与控制传输（枚举）全部失效，
     现象是"插上没反应"或"只发出第一帧就再也不动"。 */
#define IMR_MSK (CNTR_CTRM | CNTR_WKUPM | CNTR_SUSPM | CNTR_ERRM | CNTR_SOFM | CNTR_ESOFM | CNTR_RESETM)

/* CTR service routines */
/* associated to defined endpoints */
/* #define  EP1_IN_Callback   NOP_Process */
#define EP2_IN_Callback NOP_Process
#define EP3_IN_Callback NOP_Process
#define EP4_IN_Callback NOP_Process
#define EP5_IN_Callback NOP_Process
#define EP6_IN_Callback NOP_Process
#define EP7_IN_Callback NOP_Process

/* 本工程不用 EP1 OUT（回调已在 usb_endp.c 里删除）：
   这里必须接 NOP_Process，否则 usb_istr.c 的 pEpInt_OUT[] 会引用未定义符号 EP1_OUT_Callback
   → 链接错误 L6218E。 */
#define EP1_OUT_Callback NOP_Process
#define EP2_OUT_Callback NOP_Process
#define EP3_OUT_Callback NOP_Process
#define EP4_OUT_Callback NOP_Process
#define EP5_OUT_Callback NOP_Process
#define EP6_OUT_Callback NOP_Process
#define EP7_OUT_Callback NOP_Process

#endif /*__USB_CONF_H*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
