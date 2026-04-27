/**
 * @file proto_frame.c
 * @brief 协议帧打包和解包模块
 * 
 * 提供协议帧的编码（打包）和解码（解包）功能。
 * 协议帧格式：[Header1][Header2][Payload_len][Cmd][Seq][Payload...][Checksum]
 * 其中Header1=0xAA, Header2=0x55，Checksum为CRC8校验和。
 */

#include "proto_frame.h"
#include "common_def.h"
#include "checksum.h"

#include <string.h>

/**
 * @brief 打包协议帧
 * 
 * 将命令、序列号和载荷数据组装成协议帧格式，并计算校验和。
 * 
 * 帧格式（总长度 = payload_len + 7字节开销）：
 * | Header1 | Header2 | Len | Cmd | Seq | Payload(0-255) | Checksum |
 * |  0xAA   |  0x55   | 1B  | 1B  | 1B  |   0-255 bytes  |   1B     |
 * 
 * @param cmd 命令码（1字节）
 * @param seq 序列号（1字节），用于请求-响应匹配
 * @param payload 载荷数据指针，可为NULL（当payload_len=0时）
 * @param payload_len 载荷长度（字节数，范围：0-PROTO_MAX_PAYLOAD）
 * @param out_buf 输出缓冲区指针，用于存储打包后的完整帧数据
 * @param out_len 输出参数指针，返回打包后的总帧长度（字节数）
 * 
 * @return RET_OK 打包成功
 * @return RET_NULL_PTR out_buf或out_len为NULL指针
 * @return RET_INVALID_PARAM payload_len超过最大限制(PROTO_MAX_PAYLOAD)
 * 
 * @note 调用者需要确保out_buf有足够的空间（至少payload_len + 7字节）
 */
int proto_frame_pack(uint8_t cmd,
                     uint8_t seq,
                     const uint8_t *payload,
                     uint8_t payload_len,
                     uint8_t *out_buf,
                     uint16_t *out_len)
{
    uint16_t frame_len = 0U;
    uint8_t checksum = 0U;

    // 检查输出指针的有效性
    if ((out_buf == 0) || (out_len == 0))
    {
        return RET_NULL_PTR;
    }

    // 检查载荷长度是否在允许范围内
    if (payload_len > PROTO_MAX_PAYLOAD)
    {
        return RET_INVALID_PARAM;
    }

    // 计算完整帧长度 = 载荷长度 + 固定开销（帧头2 + 长度1 + 命令1 + 序列号1 + 校验和1 = 7）
    frame_len = (uint16_t)(payload_len + PROTO_FRAME_OVERHEAD);

    // 填充帧头部分
    out_buf[0] = PROTO_HEADER1;           // 帧头1：0xAA
    out_buf[1] = PROTO_HEADER2;           // 帧头2：0x55
    out_buf[2] = payload_len;             // 载荷长度
    out_buf[3] = cmd;                     // 命令码
    out_buf[4] = seq;                     // 序列号

    // 复制载荷数据到帧中（如果载荷存在）
    if ((payload != 0) && (payload_len > 0U))
    {
        memcpy(&out_buf[5], payload, payload_len);
    }

    // 计算校验和：从长度字段开始，包括长度(1) + 命令(1) + 序列号(1) + 载荷数据(payload_len)
    checksum = checksum8_sum(&out_buf[2], (uint16_t)(payload_len + 3U));
    out_buf[5U + payload_len] = checksum;

    // 返回完整的帧长度
    *out_len = frame_len;
    return RET_OK;
}

/**
 * @brief 解包（解析）协议帧
 * 
 * 将接收到的原始帧数据进行格式验证、校验和检验，提取命令、序列号和载荷数据。
 * 
 * 验证步骤：
 * 1. 检查帧头是否正确（0xAA 0x55）
 * 2. 检查接收长度是否与帧长度字段匹配
 * 3. 验证校验和是否正确
 * 
 * @param buf 指向接收到的原始帧数据的指针
 * @param len 接收到的数据长度（字节数）
 * @param msg 输出参数指针，存储解析后的消息结构体，包含cmd、seq、len和data字段
 * 
 * @return RET_OK 解包成功，消息数据已提取到msg中
 * @return RET_NULL_PTR buf或msg为NULL指针
 * @return RET_PROTO_ERR 帧格式错误，包括：
 *         - 帧长度小于最小开销
 *         - 帧头不正确
 *         - 接收长度与帧声称的长度不匹配
 * @return RET_CHECKSUM_ERR 校验和验证失败，帧数据可能被损坏
 * 
 * @note msg.data缓冲区大小至少应为PROTO_MAX_PAYLOAD字节
 */
int proto_frame_unpack(const uint8_t *buf, uint16_t len, proto_msg_t *msg)
{
    uint8_t payload_len = 0U;
    uint8_t checksum = 0U;
    uint16_t expected_len = 0U;

    // 检查输入指针的有效性
    if ((buf == 0) || (msg == 0))
    {
        return RET_NULL_PTR;
    }

    // 检查接收数据长度是否至少包含帧的固定部分
    if (len < PROTO_FRAME_OVERHEAD)
    {
        return RET_PROTO_ERR;
    }

    // 验证帧头是否正确
    if ((buf[0] != PROTO_HEADER1) || (buf[1] != PROTO_HEADER2))
    {
        return RET_PROTO_ERR;
    }

    // 从帧中提取载荷长度
    payload_len = buf[2];
    // 计算完整帧的期望长度
    expected_len = (uint16_t)(payload_len + PROTO_FRAME_OVERHEAD);

    // 检查接收到的数据长度是否与帧声称的长度一致
    if (len != expected_len)
    {
        return RET_PROTO_ERR;
    }

    // 计算校验和并与帧中的校验和字段进行比较
    checksum = checksum8_sum(&buf[2], (uint16_t)(payload_len + 3U));
    if (checksum != buf[5U + payload_len])
    {
        return RET_CHECKSUM_ERR;
    }

    // 提取并填充消息结构体中的字段
    msg->cmd = buf[3];                    // 命令码
    msg->seq = buf[4];                    // 序列号
    msg->len = payload_len;               // 载荷长度

    // 复制载荷数据到消息结构体中（如果存在载荷）
    if (payload_len > 0U)
    {
        memcpy(msg->data, &buf[5], payload_len);
    }

    return RET_OK;
}
