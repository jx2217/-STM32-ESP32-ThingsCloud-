#include "report_service.h"
#include "device_data.h"
#include "proto_cmd.h"
#include "proto_frame.h"
#include "bsp_uart.h"
#include <stdint.h>

/*
 * 这里假定你已经有这个函数。
 * 如果你的函数签名不一样，改这里即可。
 */

static uint8_t s_seq = 0U;

static void put_u16_le(uint8_t *buf, uint16_t value)
{
    buf[0] = (uint8_t)(value & 0xFFU);
    buf[1] = (uint8_t)((value >> 8U) & 0xFFU);
}

static void put_i16_le(uint8_t *buf, int16_t value)
{
    buf[0] = (uint8_t)((uint16_t)value & 0xFFU);
    buf[1] = (uint8_t)(((uint16_t)value >> 8U) & 0xFFU);
}

static void put_u32_le(uint8_t *buf, uint32_t value)
{
    buf[0] = (uint8_t)( value        & 0xFFU);
    buf[1] = (uint8_t)((value >> 8U) & 0xFFU);
    buf[2] = (uint8_t)((value >> 16U)& 0xFFU);
    buf[3] = (uint8_t)((value >> 24U)& 0xFFU);
}

int report_service_init(void)
{
    s_seq = 0U;
    return RET_OK;
}

/*
 * payload 布局：
 * temp_x10    int16   2B
 * humi_x10    uint16  2B
 * light       uint16  2B
 * volt_x100   uint16  2B
 * led         uint8   1B
 * relay       uint8   1B
 * buzzer      uint8   1B
 * alarm       uint8   1B
 * link        uint8   1B
 * reserved    uint8   1B
 * tick_ms     uint32  4B
 * 总计 18B
 */
int report_service_build_payload(uint8_t *buf, uint8_t *len)
{
    device_status_t *st = device_data_get_status();
    int16_t  temp_x10  = 0;
    uint16_t humi_x10  = 0U;
    uint16_t volt_x100 = 0U;

    if ((buf == 0U) || (len == 0U) || (st == 0U))
    {
        return RET_NULL_PTR;
    }

    temp_x10  = (int16_t)(st->temperature * 10.0f);
    humi_x10  = (uint16_t)(st->humidity * 10.0f);
    volt_x100 = (uint16_t)(st->voltage * 100.0f);

    put_i16_le(&buf[0],  temp_x10);
    put_u16_le(&buf[2],  humi_x10);
    put_u16_le(&buf[4],  st->light);
    put_u16_le(&buf[6],  volt_x100);

    buf[8]  = st->led_state;
    buf[9]  = st->relay_state;
    buf[10] = st->buzzer_state;
    buf[11] = st->alarm_code;
    buf[12] = st->link_state;
    buf[13] = 0U; /* reserved */

    put_u32_le(&buf[14], st->tick_ms);

    *len = 18U;
    return RET_OK;
}


// 函数功能：上报服务 - 发送一帧完整数据（单次发送）
// 返回值：int 类型的错误码，RET_OK 表示成功，其他值表示失败
int report_service_send_once(void)
{
    // 定义【有效载荷】数组（存放真正要上报的数据）
    uint8_t payload[PROTO_MAX_PAYLOAD] = {0};
    
    // 定义【完整协议帧】数组（包含帧头、数据、校验等完整格式）
    uint8_t frame[PROTO_MAX_FRAME_LEN] = {0};
    
    // 有效载荷的长度（初始化为 0，无符号 8 位）
    uint8_t payload_len = 0U;
    
    // 完整帧的长度（初始化为 0，无符号 16 位）
    uint16_t frame_len = 0U;
    
    // 函数返回值，默认初始化为成功
    int ret = RET_OK;

    // ===================== 第一步：组装要上报的数据 =====================
    // 调用函数，把需要上报的内容填到 payload 里，并得到长度 payload_len
    ret = report_service_build_payload(payload, &payload_len);
    
    // 如果组装失败，直接返回错误码
    if (ret != RET_OK)
    {
        return ret;
    }

    // ===================== 第二步：打包成通信协议帧 =====================
    // 把命令字、序列号、有效数据 打包成 完整的通信帧
    ret = proto_frame_pack(
        (uint8_t)CMD_REPORT,   // 命令：上报指令
        s_seq++,               // 帧序列号：每发一帧自增 1（防止丢包/重复）
        payload,               // 刚才组装好的有效数据
        payload_len,           // 有效数据长度
        frame,                 // 输出：打包好的完整帧存在这里
        &frame_len             // 输出：完整帧的长度
    );
    
    // 如果打包失败，直接返回错误码
    if (ret != RET_OK)
    {
        return ret;
    }

    // ===================== 第三步：通过串口硬件发送出去 =====================
    // 调用底层串口发送函数，发送完整帧
    return bsp_uart_send(frame, frame_len);
}

/*
 * ACK payload:
 * [0] ack_cmd
 * [1] ack_seq
 * [2] result
 */
int report_service_send_ack(uint8_t ack_cmd, uint8_t ack_seq, uint8_t result)
{
    uint8_t payload[3] = {0};
    uint8_t frame[PROTO_MAX_FRAME_LEN] = {0};
    uint16_t frame_len = 0U;
    int ret;

    payload[0] = ack_cmd;
    payload[1] = ack_seq;
    payload[2] = result;

    ret = proto_frame_pack((uint8_t)CMD_ACK,
                           s_seq++,
                           payload,
                           3U,
                           frame,
                           &frame_len);
    if (ret != RET_OK)
    {
        return ret;
    }

    return bsp_uart_send(frame, frame_len);
}
