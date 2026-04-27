/**
 * @file app_service.c
 * @brief 应用服务模块
 *
 * 提供应用层的协议消息处理功能。该模块作为应用层的核心，
 * 负责解析来自协议桥接模块的完整消息，更新设备状态缓存，
 * 并记录应答信息。
 *
 * 支持的协议消息类型：
 * - CMD_REPORT (0x01): 设备上报状态消息
 *   包含：温度、湿度、光照、电压、LED、继电器、蜂鸣器、报警码、连接状态等
 * - CMD_ACK (0x05): 设备应答消息
 *   包含：应答命令、应答序列号、应答结果码
 *
 * 数据格式：所有多字节数据采用小端序（Little-Endian）存储
 */

#include "app_service.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "project_config.h"
#include "common_def.h"
#include "proto_cmd.h"
#include "proto_frame.h"
#include "status_cache.h"
#include "uart_service.h"

/**
 * @brief 从字节缓冲区读取16位无符号整数（小端序）
 *
 * 读取缓冲区中的两个连续字节，按小端序组合成16位无符号整数。
 * 小端序：低字节在前，高字节在后。
 *
 * @param buf 指向字节缓冲区的指针，至少包含2个字节
 * @return 读取到的16位无符号整数值
 *
 * @note 调用者需确保缓冲区有足够的字节
 */
static uint16_t get_u16_le(const uint8_t *buf)
{
    return (uint16_t)(buf[0] | (buf[1] << 8));
}

/**
 * @brief 从字节缓冲区读取16位有符号整数（小端序）
 *
 * 读取缓冲区中的两个连续字节，按小端序组合成16位有符号整数。
 * 小端序：低字节在前，高字节在后。
 *
 * @param buf 指向字节缓冲区的指针，至少包含2个字节
 * @return 读取到的16位有符号整数值
 *
 * @note 调用者需确保缓冲区有足够的字节
 */
static int16_t get_i16_le(const uint8_t *buf)
{
    return (int16_t)(buf[0] | (buf[1] << 8));
}

/**
 * @brief 从字节缓冲区读取32位无符号整数（小端序）
 *
 * 读取缓冲区中的四个连续字节，按小端序组合成32位无符号整数。
 * 小端序：低字节在前，高字节在后。
 * 字节顺序：buf[0]<-LSB, buf[1], buf[2], buf[3]<-MSB
 *
 * @param buf 指向字节缓冲区的指针，至少包含4个字节
 * @return 读取到的32位无符号整数值
 *
 * @note 调用者需确保缓冲区有足够的字节
 */
static uint32_t get_u32_le(const uint8_t *buf)
{
    return ((uint32_t)buf[0]) |
           ((uint32_t)buf[1] << 8) |
           ((uint32_t)buf[2] << 16) |
           ((uint32_t)buf[3] << 24);
}

/**
 * @brief 初始化应用服务
 *
 * 初始化应用层服务模块。当前实现主要是日志输出，
 * 可在此处添加必要的初始化逻辑。
 *
 * @return RET_OK 初始化成功
 */
int app_service_init(void)
{
    ESP_LOGI(TAG_APP_SERVICE, "app_service_init");
    return RET_OK;
}

/**
 * @brief 处理解析后的协议消息
 *
 * 该函数作为应用层的核心处理函数，根据协议消息的命令类型处理消息内容，
 * 将设备状态或应答信息更新到对应的状态缓存中。
 *
 * 支持的命令类型：
 * 1. CMD_REPORT (0x01) - 设备状态上报
 *    载荷结构（18字节）：
 *    | 偏移 | 长度 | 类型 | 字段 | 说明 |
 *    |------|------|------|------|------|
 *    | 0-1  | 2    | i16  | 温度 | 单位0.1°C，小端序 |
 *    | 2-3  | 2    | u16  | 湿度 | 单位0.1%，小端序 |
 *    | 4-5  | 2    | u16  | 光照 | ADC值，小端序 |
 *    | 6-7  | 2    | u16  | 电压 | 单位0.01V，小端序 |
 *    | 8    | 1    | u8   | LED状态 | 0=关，1=开 |
 *    | 9    | 1    | u8   | 继电器状态 | 0=关，1=开 |
 *    | 10   | 1    | u8   | 蜂鸣器状态 | 0=关，1=开 |
 *    | 11   | 1    | u8   | 报警码 | 0=无报警，其他值为报警类型 |
 *    | 12   | 1    | u8   | 连接状态 | 0=离线，1=在线 |
 *    | 13   | 1    | 保留 | 预留字段 | - |
 *    | 14-17| 4    | u32  | 设备tick计数 | 单位ms，小端序 |
 *
 * 2. CMD_ACK (0x05) - 设备应答
 *    载荷结构（3字节）：
 *    | 偏移 | 长度 | 类型 | 字段 | 说明 |
 *    |------|------|------|------|------|
 *    | 0    | 1    | u8   | 应答命令码 | 应答的原始命令码 |
 *    | 1    | 1    | u8   | 应答序列号 | 应答的原始序列号 |
 *    | 2    | 1    | u8   | 结果码 | 0x00=OK, 0x01=参数错误, 0x02=不支持, 0x03=执行错误 |
 *
 * @param msg 指向解析后协议消息的指针
 *
 * @return RET_OK 消息处理成功
 * @return RET_NULL_PTR msg为NULL或缓存获取失败
 * @return RET_INVALID_PARAM 消息长度不足或命令类型不支持
 *
 * @note 消息的时间戳会被更新为当前系统时间
 * @note 消息中的浮点数据（温度、湿度、电压）会被自动缩放到正确的单位
 */
int app_service_handle_proto_msg(const proto_msg_t *msg)
{
    uint32_t now_ms;
    device_status_cache_t *dev_cache;
    ack_info_cache_t *ack_cache;

    // 检查消息指针有效性
    if (msg == 0)
    {
        return RET_NULL_PTR;
    }

    // 获取当前时间戳（毫秒）
    now_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);

    switch (msg->cmd)
    {
        case CMD_REPORT:
        {
            // 检查消息长度是否满足要求（最少18字节）
            if (msg->len < 18U)
            {
                return RET_INVALID_PARAM;
            }

            // 获取设备状态缓存
            dev_cache = status_cache_get_device();
            if (dev_cache == 0)
            {
                return RET_NULL_PTR;
            }

            // 解析温度值（i16，单位0.1°C）
            dev_cache->temperature = (float)get_i16_le(&msg->data[0]) / 10.0f;
            // 解析湿度值（u16，单位0.1%）
            dev_cache->humidity    = (float)get_u16_le(&msg->data[2]) / 10.0f;
            // 解析光照值（u16，ADC原始值）
            dev_cache->light       = get_u16_le(&msg->data[4]);
            // 解析电压值（u16，单位0.01V）
            dev_cache->voltage     = (float)get_u16_le(&msg->data[6]) / 100.0f;

            // 解析设备状态
            dev_cache->led_state    = msg->data[8];      // LED状态
            dev_cache->relay_state  = msg->data[9];      // 继电器状态
            dev_cache->buzzer_state = msg->data[10];     // 蜂鸣器状态
            dev_cache->alarm_code   = msg->data[11];     // 报警码
            dev_cache->link_state   = msg->data[12];     // 连接状态

            // 解析设备时间戳
            dev_cache->tick_ms        = get_u32_le(&msg->data[14]);
            // 记录缓存的最后更新时间
            dev_cache->last_update_ms = now_ms;

            // 记录详细的日志信息
            ESP_LOGI(TAG_APP_SERVICE,
                     "REPORT seq=%u temp=%.1f humi=%.1f light=%u volt=%.2f led=%u relay=%u alarm=%u link=%u tick=%lu",
                     msg->seq,
                     dev_cache->temperature,
                     dev_cache->humidity,
                     dev_cache->light,
                     dev_cache->voltage,
                     dev_cache->led_state,
                     dev_cache->relay_state,
                     dev_cache->alarm_code,
                     dev_cache->link_state,
                     (unsigned long)dev_cache->tick_ms);

            return RET_OK;
        }

        case CMD_ACK:
        {
            // 检查消息长度是否满足要求（最少3字节）
            if (msg->len < 3U)
            {
                return RET_INVALID_PARAM;
            }

            // 获取ACK信息缓存
            ack_cache = status_cache_get_ack();
            if (ack_cache == 0)
            {
                return RET_NULL_PTR;
            }

            // 解析ACK信息
            ack_cache->ack_cmd        = msg->data[0];     // 被应答的原始命令码
            ack_cache->ack_seq        = msg->data[1];     // 被应答的序列号
            ack_cache->ack_result     = msg->data[2];     // 执行结果码
            // 记录ACK信息的最后更新时间
            ack_cache->last_update_ms = now_ms;

            // 记录详细的日志信息
            ESP_LOGI(TAG_APP_SERVICE,
                     "ACK seq=%u ack_cmd=%u ack_seq=%u result=%u",
                     msg->seq,
                     ack_cache->ack_cmd,
                     ack_cache->ack_seq,
                     ack_cache->ack_result);

            return RET_OK;
        }

        default:
        {
            // 不支持的命令类型
            ESP_LOGW(TAG_APP_SERVICE, "unsupported cmd=%u len=%u", msg->cmd, msg->len);
            return RET_INVALID_PARAM;
        }
    }
}
