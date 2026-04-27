/**
 * @file proto_bridge.c
 * @brief 协议桥接模块
 *
 * 提供协议帧的接收和解析功能。该模块作为传输层和应用层之间的桥接，
 * 负责接收来自传输层（如串口）的原始字节数据，进行帧同步和解析，
 * 最终将完整的协议消息传递给应用层处理。
 *
 * 主要功能包括：
 * - 接收和缓存原始字节数据
 * - 进行帧同步（检测帧头）
 * - 计算预期帧长度
 * - 解析协议帧
 * - 将协议消息交给应用层处理
 */

#include "proto_bridge.h"

#include "esp_log.h"
#include "project_config.h"
#include "common_def.h"
#include "proto_cmd.h"
#include "proto_frame.h"
#include "app_service.h"

#include <string.h>

/** @brief 接收缓冲区，用于存储接收到的原始字节数据 */
static uint8_t s_rx_buf[PROTO_MAX_FRAME_LEN];
/** @brief 接收缓冲区中的有效字节数 */
static uint16_t s_rx_len = 0U;

/**
 * @brief 输入单个字节到协议桥接器
 *
 * 该函数实现了协议帧的接收状态机，包括帧同步、长度验证和帧解析。
 * 整个解析过程是字节流式的，每输入一个字节就检查是否已收到完整帧。
 *
 * 解析步骤：
 * 1. 检查缓冲区是否满（防止溢出）
 * 2. 存储字节到缓冲区
 * 3. 第1个字节：检查帧头1（0xAA）
 * 4. 第2个字节：检查帧头2（0x55）
 * 5. 第3个字节及以后：
 *    - 计算预期帧长度（= payload_len + 帧开销7字节）
 *    - 验证预期长度的有效性
 *    - 当接收到的字节数等于预期长度时，调用frame_unpack解析帧
 *    - 解析成功后，将消息交给应用层处理
 *
 * @param byte 接收到的单个字节
 *
 * @return RET_OK 字节已处理（可能需要继续接收更多字节）
 * @return RET_ERR 帧头或长度验证失败，缓冲区已重置
 * @return 其他值 帧解析或消息处理的返回值
 */
static int proto_bridge_feed_byte(uint8_t byte)
{
    proto_msg_t msg;
    uint16_t expected_frame_len = 0U;
    int ret;

    // 防止缓冲区溢出
    if (s_rx_len >= PROTO_MAX_FRAME_LEN)
    {
        s_rx_len = 0U;
    }

    // 将字节存储到缓冲区
    s_rx_buf[s_rx_len++] = byte;

    // 第1个字节：验证帧头1
    if (s_rx_len == 1U)
    {
        if (s_rx_buf[0] != PROTO_HEADER1)  // 期望 0xAA
        {
            s_rx_len = 0U;  // 重置缓冲区
            return RET_ERR;
        }
    }

    // 第2个字节：验证帧头2
    if (s_rx_len == 2U)
    {
        if (s_rx_buf[1] != PROTO_HEADER2)  // 期望 0x55
        {
            s_rx_len = 0U;  // 重置缓冲区
            return RET_ERR;
        }
    }

    // 第3个字节及以后：计算预期帧长度并检查完整性
    if (s_rx_len >= 3U)
    {
        // s_rx_buf[2] 是载荷长度，总帧长度 = 载荷长度 + 帧开销(7)
        expected_frame_len = (uint16_t)(s_rx_buf[2] + PROTO_FRAME_OVERHEAD);

        // 验证预期帧长度是否在有效范围内
        if ((expected_frame_len < PROTO_FRAME_OVERHEAD) ||
            (expected_frame_len > PROTO_MAX_FRAME_LEN))
        {
            s_rx_len = 0U;  // 长度无效，重置缓冲区
            return RET_ERR;
        }

        // 检查是否已接收到完整帧
        if (s_rx_len == expected_frame_len)
        {
            // 初始化消息结构体
            memset(&msg, 0, sizeof(msg));

            // 解析协议帧
            ret = proto_frame_unpack(s_rx_buf, s_rx_len, &msg);
            s_rx_len = 0U;  // 重置缓冲区准备接收下一帧

            // 如果解析成功，将消息交给应用层处理
            if (ret == RET_OK)
            {
                return app_service_handle_proto_msg(&msg);
            }

            return ret;
        }
    }

    return RET_OK;  // 继续等待更多字节
}

/**
 * @brief 将多个字节输入到协议桥接解析器中
 *
 * 该函数接收来自传输层（如串口）的原始字节缓冲区，并将每个字节逐个传递给内部解析器。
 * 这是协议桥接模块的主要入口点，通常由串口接收中断处理程序调用。
 *
 * @param buf 输入字节缓冲区指针，不能为空
 * @param len 缓冲区中的字节数量
 *
 * @return RET_OK 所有字节已成功传递给解析器
 * @return RET_NULL_PTR 输入缓冲区指针为空
 *
 * @note 该函数会对缓冲区中的每一个字节调用proto_bridge_feed_byte进行处理
 * @note 个别字节的解析错误不会中断整个处理流程，会继续处理后续字节
 */
int proto_bridge_feed_bytes(const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    // 参数有效性检查
    if (buf == 0)
    {
        return RET_NULL_PTR;
    }

    // 逐字节传递给解析器
    for (i = 0; i < len; i++)
    {
        proto_bridge_feed_byte(buf[i]);
    }

    return RET_OK;
}