/* ---------- App/app_hid.c ---------- */
#include "app_hid.h"
#include "bsp_keys.h"
#include "msp_adc.h"
#include "systick.h"
#include <string.h>

/* 传输层钩子：由 app_hid_init 或外部注入（USB / BLE 模块 / SPP） */
static app_hid_transport_t s_transport_send;

static uint8_t s_report[HID_REPORT_SIZE];
static uint8_t s_last[HID_REPORT_SIZE];
static uint32_t s_t_last;
static uint32_t s_sent;
static uint32_t s_changed;

/* 十字键查表：bit0=RIGHT, bit1=LEFT, bit2=DOWN, bit3=UP */
static const uint8_t s_hat_table[16] = {
    0x0F, 2, 6, 0x0F, 4, 3, 5, 0x0F,
    0x00, 1, 7, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F};

static uint8_t dpad_to_hat(uint16_t st)
{
    uint8_t idx = 0;

    if (st & (1u << KEY_RIGHT))
    {
        idx |= 0x01u;
    }
    if (st & (1u << KEY_LEFT))
    {
        idx |= 0x02u;
    }
    if (st & (1u << KEY_DOWN))
    {
        idx |= 0x04u;
    }
    if (st & (1u << KEY_UP))
    {
        idx |= 0x08u;
    }
    return s_hat_table[idx];
}

void app_hid_init(void)
{
    memset(s_report, 0, sizeof(s_report));
    memset(s_last, 0, sizeof(s_last));
    s_report[2] = s_report[3] = s_report[4] = s_report[5] = 128u; /* 摇杆居中 */
    s_report[8] = 0x0Fu;                                          /* hat = 空 */
    memcpy(s_last, s_report, sizeof(s_report));
    s_t_last = get_ms();
    s_sent = s_changed = 0;
}

void app_hid_build_report(void)
{
    uint16_t ks = keys_get_state();
    uint8_t b0 = 0, b1 = 0;

    /* ---- 按钮域（byte0 / byte1）---- */
    if (ks & (1u << KEY_A))
    {
        b0 |= 0x01u;
    } /* Button 1  */
    if (ks & (1u << KEY_B))
    {
        b0 |= 0x02u;
    } /* Button 2  */
    if (ks & (1u << KEY_X))
    {
        b0 |= 0x04u;
    } /* Button 3  */
    if (ks & (1u << KEY_Y))
    {
        b0 |= 0x08u;
    } /* Button 4  */
    if (ks & (1u << KEY_LB))
    {
        b0 |= 0x10u;
    } /* Button 5  */
    if (ks & (1u << KEY_RB))
    {
        b0 |= 0x20u;
    } /* Button 6  */
    if (trigger_is_pressed(AXIS_LT))
    {
        b0 |= 0x40u;
    } /* Button 7  */
    if (trigger_is_pressed(AXIS_RT))
    {
        b0 |= 0x80u;
    } /* Button 8  */

    if (ks & (1u << KEY_VIEW))
    {
        b1 |= 0x01u;
    } /* Button 9  Back/Select */
    if (ks & (1u << KEY_MENU))
    {
        b1 |= 0x02u;
    } /* Button 10 Start       */
    if (ks & (1u << KEY_HOME))
    {
        b1 |= 0x04u;
    } /* Button 11 Guide/Home  */
    if (ks & (1u << KEY_L3))
    {
        b1 |= 0x08u;
    } /* Button 12 左摇杆按下   */
    if (ks & (1u << KEY_R3))
    {
        b1 |= 0x10u;
    } /* Button 13 右摇杆按下   */
    /* bit5..bit7 保持 0（填充） */

    /* ---- 轴与扳机 ---- */
    s_report[0] = b0;
    s_report[1] = b1;
    s_report[2] = axis_get_unsigned(AXIS_LX);
    s_report[3] = axis_get_unsigned(AXIS_LY);
    s_report[4] = axis_get_unsigned(AXIS_RX);
    s_report[5] = axis_get_unsigned(AXIS_RY);
    s_report[6] = trigger_get_8bit(AXIS_LT);
    s_report[7] = trigger_get_8bit(AXIS_RT);

    /* ---- 十字键（低 4 位） ---- */
    s_report[8] = (uint8_t)(dpad_to_hat(ks) & 0x0Fu);
}

/* 发送策略：变化即发（低延迟） + 至少 20Hz 保活（防止主机认为设备静止） */
#define HID_MIN_INTERVAL_MS 10u /* 最快 100Hz */
#define HID_KEEPALIVE_MS 50u    /* 最慢 20Hz */

void app_hid_flush(void)
{
    uint32_t now = get_ms();
    uint8_t changed = (memcmp(s_report, s_last, HID_REPORT_SIZE) != 0) ? 1u : 0u;

    if (changed)
    {
        if ((now - s_t_last) < HID_MIN_INTERVAL_MS)
        {
            return; /* 限速：不要超过 100Hz */
        }
    }
    else
    {
        if ((now - s_t_last) < HID_KEEPALIVE_MS)
        {
            return; /* 没变化也没到保活点 */
        }
    }

    if (s_transport_send != 0)
    {
        /* 返回码：0=成功 1=主机未配置 2=端点忙（丢帧） 3=长度不符 */
        uint8_t st = s_transport_send(s_report, HID_REPORT_SIZE);

        if (st == 0u)
        {
            s_sent++;
            if (changed)
            {
                s_changed++;
            }
            memcpy(s_last, s_report, HID_REPORT_SIZE);
            s_t_last = now;
        }
        /* st != 0（未配置 / 忙）：不刷新 s_last 与 s_t_last，下个周期自动重试 */
    }
}

const uint8_t *app_hid_report(void)
{
    return s_report;
}

void app_hid_set_transport(app_hid_transport_t send)
{
    s_transport_send = send;
}

void app_hid_get_stats(uint32_t *sent, uint32_t *changed)
{
    if (sent)
    {
        *sent = s_sent;
    }
    if (changed)
    {
        *changed = s_changed;
    }
}
