#include "device_data.h"
#include <string.h>

static device_status_t g_device_status;
static device_param_t  g_device_param;

int device_data_init(void)
{
    memset(&g_device_status, 0, sizeof(g_device_status));
    memset(&g_device_param, 0, sizeof(g_device_param));

    /* 默认状态 */
    g_device_status.temperature = 25.0f;
    g_device_status.humidity    = 60.0f;
    g_device_status.light       = 300U;
    g_device_status.voltage     = 3.30f;
		g_device_status.led_state   = 0U;
    g_device_status.relay_state = 0U;
    g_device_status.buzzer_state= 0U;
    g_device_status.alarm_code  = ALARM_NONE;
    g_device_status.link_state  = LINK_ONLINE;
    g_device_status.tick_ms     = 0U;

    /* 默认参数 */
    g_device_param.sample_period_ms = 1000U;
    g_device_param.upload_period_ms = 5000U;
    g_device_param.temp_high_th     = 28.0f;
    g_device_param.light_low_th     = 2000.0f;
    g_device_param.work_mode        = MODE_MANUAL;
    g_device_param.relay_default    = 0U;

    return RET_OK;
}

device_status_t *device_data_get_status(void)
{
    return &g_device_status;
}

device_param_t *device_data_get_param(void)
{
    return &g_device_param;
}

void device_data_set_tick(uint32_t tick_ms)
{
    g_device_status.tick_ms = tick_ms;
}

void device_data_set_link_state(uint8_t link_state)
{
    g_device_status.link_state = link_state;
}

int device_data_set_upload_period(uint16_t period_ms)
{
    /* 做一个基本合法性限制，避免设置得过小或过大 */
    if ((period_ms < 100U) || (period_ms > 10000U))
    {
        return RET_INVALID_PARAM;
    }

    g_device_param.upload_period_ms = period_ms;
    return RET_OK;
}

int device_data_set_temp_high_th(uint16_t temp_th)
{
    /*
     * 第一版：用整数摄氏度传输
     * 合理范围先设为 0~80℃
     */
    if (temp_th > 80U)
    {
        return RET_INVALID_PARAM;
    }

    g_device_param.temp_high_th = (float)temp_th;
    return RET_OK;
}

int device_data_set_light_low_th(uint16_t light_th)
{
    /*
     * 光照当前用 ADC 原始值，F103 12位 ADC 0~4095
     */
    if (light_th > 4095U)
    {
        return RET_INVALID_PARAM;
    }

    g_device_param.light_low_th = (float)light_th;
    return RET_OK;
}

int device_data_set_work_mode(uint8_t mode)
{
    if ((mode != MODE_MANUAL) &&
        (mode != MODE_AUTO) &&
        (mode != MODE_REMOTE))
    {
        return RET_INVALID_PARAM;
    }

    g_device_param.work_mode = mode;
    return RET_OK;
}
