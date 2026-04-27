#include "stm32_ctrl.h"

#include "common_def.h"
#include "proto_cmd.h"
#include "proto_frame.h"
#include "uart_service.h"
#include "esp_log.h"
#include "project_config.h"

static uint8_t s_tx_seq = 0U;

static void put_u16_le(uint8_t *buf, uint16_t value)
{
    buf[0] = (uint8_t)(value & 0xFFU);
    buf[1] = (uint8_t)((value >> 8U) & 0xFFU);
}

static int stm32_ctrl_send_param_u16(uint8_t param_id, uint16_t value)
{
    uint8_t payload[3] = {0};
    uint8_t frame[PROTO_MAX_FRAME_LEN] = {0};
    uint16_t frame_len = 0U;
    uint8_t seq;
    int ret;

    payload[0] = param_id;
    put_u16_le(&payload[1], value);

    seq = s_tx_seq++;

    ret = proto_frame_pack((uint8_t)CMD_PARAM_SET,
                           seq,
                           payload,
                           3U,
                           frame,
                           &frame_len);
    if (ret != RET_OK)
    {
        return ret;
    }

    return uart_service_send_bytes(frame, frame_len);
}

int stm32_ctrl_init(void)
{
    s_tx_seq = 0U;
    return RET_OK;
}

int stm32_ctrl_send_led_set(uint8_t action)
{
    uint8_t payload[1] = {0};
    uint8_t frame[PROTO_MAX_FRAME_LEN] = {0};
    uint16_t frame_len = 0U;
    uint8_t seq;
    int ret;

    if (action > 2U)
    {
        return RET_INVALID_PARAM;
    }

    payload[0] = action;
    seq = s_tx_seq++;

    ret = proto_frame_pack((uint8_t)CMD_LED_SET,
                           seq,
                           payload,
                           1U,
                           frame,
                           &frame_len);
    if (ret != RET_OK)
    {
        return ret;
    }

    ESP_LOGI(TAG_APP_MAIN, "send LED_SET action=%u seq=%u", action, seq);
    return uart_service_send_bytes(frame, frame_len);
}

int stm32_ctrl_send_upload_period(uint16_t period_ms)
{
    return stm32_ctrl_send_param_u16((uint8_t)PARAM_ID_UPLOAD_PERIOD_MS, period_ms);
}

int stm32_ctrl_send_temp_high_th(uint16_t temp_th)
{
    return stm32_ctrl_send_param_u16((uint8_t)PARAM_ID_TEMP_HIGH_TH, temp_th);
}

int stm32_ctrl_send_light_low_th(uint16_t light_th)
{
    return stm32_ctrl_send_param_u16((uint8_t)PARAM_ID_LIGHT_LOW_TH, light_th);
}

int stm32_ctrl_send_work_mode(uint16_t mode)
{
    return stm32_ctrl_send_param_u16((uint8_t)PARAM_ID_WORK_MODE, mode);
}
