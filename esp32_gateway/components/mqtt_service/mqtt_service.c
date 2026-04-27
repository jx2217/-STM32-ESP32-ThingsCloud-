/**
 * @file mqtt_service.c
 * @brief MQTT服务模块
 *
 * 提供MQTT客户端功能，实现设备与云端服务器的通信。
 * 支持设备状态上报、远程命令接收和执行。
 * 主要功能包括：
 * - 连接到MQTT服务器
 * - 订阅命令主题
 * - 定期上报设备状态和参数
 * - 解析并执行云端下发的JSON格式命令
 */

#include "mqtt_service.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "mqtt_client.h"

#include "project_config.h"
#include "common_def.h"
#include "status_cache.h"
#include "stm32_ctrl.h"
#include "wifi_service.h"
#include "param_cache.h"

/** @brief MQTT客户端句柄 */
static esp_mqtt_client_handle_t s_mqtt_client = NULL;
/** @brief MQTT连接状态标志 */
static int s_mqtt_connected = 0;

/** @brief MQTT发布任务函数声明 */
static void mqtt_publish_task(void *pvParameters);

/**
 * @brief MQTT状态快照结构体
 *
 * 用于存储设备状态的快照，用于比较状态变化和避免重复上报。
 */
typedef struct
{
    float temperature;     /**< 温度值 */
    float humidity;        /**< 湿度值 */
    uint16_t light;        /**< 光照强度 */
    float voltage;         /**< 电压值 */

    uint8_t led_state;     /**< LED状态 */
    uint8_t relay_state;   /**< 继电器状态 */
    uint8_t buzzer_state;  /**< 蜂鸣器状态 */

    uint8_t alarm_code;    /**< 报警码 */
    uint8_t link_state;    /**< 连接状态 */

    uint32_t tick_ms;      /**< 系统滴答计数 */
    uint32_t last_update_ms; /**< 最后更新时间戳 */
} mqtt_status_snapshot_t;

/** @brief 上次上报的状态快照 */
static mqtt_status_snapshot_t s_last_reported = {0};
/** @brief 是否有上次上报记录的标志 */
static int s_has_last_reported = 0;
/** @brief 上次发布的时间戳（毫秒） */
static uint32_t s_last_publish_ms = 0U;

/**
 * @brief 解析工作模式字符串并转换为对应的模式码
 *
 * 将字符串格式的工作模式转换为数值模式码。
 *
 * @param mode_str 模式字符串（"manual"=手动模式, "auto"=自动模式, "remote"=远程模式）
 * @param mode 输出参数，存储转换后的模式码（0=手动, 1=自动, 2=远程）
 *
 * @return RET_OK 解析成功
 * @return RET_INVALID_PARAM 参数无效或模式字符串不匹配
 */
static int mqtt_parse_mode_string(const char *mode_str, uint16_t *mode)
{
    if ((mode_str == NULL) || (mode == NULL))
    {
        return RET_INVALID_PARAM;
    }

    if (strcmp(mode_str, "manual") == 0)
    {
        *mode = 0U;
        return RET_OK;
    }
    else if (strcmp(mode_str, "auto") == 0)
    {
        *mode = 1U;
        return RET_OK;
    }
    else if (strcmp(mode_str, "remote") == 0)
    {
        *mode = 2U;
        return RET_OK;
    }

    return RET_INVALID_PARAM;
}


/**
 * @brief 从JSON字符串中提取字符串值
 *
 * 从JSON格式的payload中查找指定key对应的字符串值。
 * 假设JSON格式为：{"key":"value"}
 *
 * @param payload JSON字符串
 * @param key 要查找的键名
 * @param out 输出缓冲区，用于存储提取的字符串值
 * @param out_size 输出缓冲区大小
 *
 * @return RET_OK 提取成功
 * @return RET_INVALID_PARAM 参数无效或未找到匹配的key
 */
static int json_get_string_value(const char *payload,
                                 const char *key,
                                 char *out,
                                 size_t out_size)
{
    char pattern[32];
    const char *p;
    const char *start;
    const char *end;
    size_t len;

    if ((payload == NULL) || (key == NULL) || (out == NULL) || (out_size == 0))
    {
        return RET_INVALID_PARAM;
    }

    // 构造搜索模式："key":"
    snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);

    // 查找模式在payload中的位置
    p = strstr(payload, pattern);
    if (p == NULL)
    {
        return RET_INVALID_PARAM;
    }

    // 找到值的起始位置（跳过模式字符串）
    start = p + strlen(pattern);
    // 找到值的结束位置（下一个双引号）
    end = strchr(start, '"');
    if (end == NULL)
    {
        return RET_INVALID_PARAM;
    }

    // 计算值长度并复制到输出缓冲区
    len = (size_t)(end - start);
    if (len >= out_size)
    {
        len = out_size - 1; // 留出空间给终止符
    }

    memcpy(out, start, len);
    out[len] = '\0';

    return RET_OK;
}

/**
 * @brief 从JSON字符串中提取整数值
 *
 * 从JSON格式的payload中查找指定key对应的整数值。
 * 假设JSON格式为：{"key":123}
 *
 * @param payload JSON字符串
 * @param key 要查找的键名
 * @param out_value 输出参数，存储提取的整数值
 *
 * @return RET_OK 提取成功
 * @return RET_INVALID_PARAM 参数无效或未找到匹配的key
 */
static int json_get_int_value(const char *payload,
                              const char *key,
                              int *out_value)
{
    char pattern[32];
    const char *p;
    const char *start;

    if ((payload == NULL) || (key == NULL) || (out_value == NULL))
    {
        return RET_INVALID_PARAM;
    }

    // 构造搜索模式："key":
    snprintf(pattern, sizeof(pattern), "\"%s\":", key);

    // 查找模式在payload中的位置
    p = strstr(payload, pattern);
    if (p == NULL)
    {
        return RET_INVALID_PARAM;
    }

    // 找到值的起始位置（跳过模式字符串）
    start = p + strlen(pattern);
    // 使用strtol转换字符串为整数
    *out_value = (int)strtol(start, NULL, 10);

    return RET_OK;
}

/**
 * @brief 解析并执行云平台下发的MQTT命令JSON
 *
 * 从JSON格式的payload中解析命令并执行相应的操作。
 * 支持的命令包括：
 * - "led": LED控制（"on"/"off"/"toggle"）
 * - "set_period": 设置上报周期（100-10000ms）
 * - "set_temp_th": 设置温度阈值（0-80°C）
 * - "set_light_th": 设置光照阈值（0-4095）
 * - "set_mode": 设置工作模式（"manual"/"auto"/"remote"）
 *
 * JSON格式示例：{"cmd":"led","value":"on"}
 *
 * @param payload JSON格式的命令字符串
 *
 * @return RET_OK 命令执行成功
 * @return RET_NULL_PTR payload为NULL
 * @return RET_INVALID_PARAM JSON格式错误或参数无效
 */
static int mqtt_parse_command_json(const char *payload)
{
    char cmd[32] = {0};
    char value_str[32] = {0};
    int value_int = 0;
    int ret;
    uint16_t mode;

    if (payload == NULL)
    {
        return RET_NULL_PTR;
    }

    // 提取命令类型
    ret = json_get_string_value(payload, "cmd", cmd, sizeof(cmd));
    if (ret != RET_OK)
    {
        return RET_INVALID_PARAM;
    }

    // 处理LED控制命令
    if (strcmp(cmd, "led") == 0)
    {
        ret = json_get_string_value(payload, "value", value_str, sizeof(value_str));
        if (ret != RET_OK)
        {
            return RET_INVALID_PARAM;
        }

        if (strcmp(value_str, "on") == 0)
        {
            return stm32_ctrl_send_led_set(1U);
        }
        else if (strcmp(value_str, "off") == 0)
        {
            return stm32_ctrl_send_led_set(0U);
        }
        else if (strcmp(value_str, "toggle") == 0)
        {
            return stm32_ctrl_send_led_set(2U);
        }
        else
        {
            return RET_INVALID_PARAM;
        }
    }
    // 处理设置上报周期命令
    else if (strcmp(cmd, "set_period") == 0)
    {
        ret = json_get_int_value(payload, "value", &value_int);
        if (ret != RET_OK || (value_int < 100) || (value_int > 10000))
        {
            return RET_INVALID_PARAM;
        }

        ret = stm32_ctrl_send_upload_period((uint16_t)value_int);
        if (ret == RET_OK)
        {
            param_cache_set_upload_period((uint16_t)value_int);
        }
        return ret;
    }
    // 处理设置温度阈值命令
    else if (strcmp(cmd, "set_temp_th") == 0)
    {
        ret = json_get_int_value(payload, "value", &value_int);
        if (ret != RET_OK || (value_int < 0) || (value_int > 80))
        {
            return RET_INVALID_PARAM;
        }

        ret = stm32_ctrl_send_temp_high_th((uint16_t)value_int);
        if (ret == RET_OK)
        {
            param_cache_set_temp_high_th((uint16_t)value_int);
        }
        return ret;
    }
    // 处理设置光照阈值命令
    else if (strcmp(cmd, "set_light_th") == 0)
    {
        ret = json_get_int_value(payload, "value", &value_int);
        if (ret != RET_OK || (value_int < 0) || (value_int > 4095))
        {
            return RET_INVALID_PARAM;
        }

        ret = stm32_ctrl_send_light_low_th((uint16_t)value_int);
        if (ret == RET_OK)
        {
            param_cache_set_light_low_th((uint16_t)value_int);
        }
        return ret;
    }
    // 处理设置工作模式命令
    else if (strcmp(cmd, "set_mode") == 0)
    {
        ret = json_get_string_value(payload, "value", value_str, sizeof(value_str));
        if (ret != RET_OK)
        {
            return RET_INVALID_PARAM;
        }

        ret = mqtt_parse_mode_string(value_str, &mode);
        if (ret != RET_OK)
        {
            return RET_INVALID_PARAM;
        }

        ret = stm32_ctrl_send_work_mode(mode);
        if (ret == RET_OK)
        {
            param_cache_set_work_mode(mode);
        }
        return ret;
    }

    return RET_INVALID_PARAM;
}


/**
 * @brief 浮点数比较函数
 *
 * 比较两个浮点数是否发生显著变化，给定一个很小的阈值，
 * 用于避免极小抖动导致频繁上报。
 *
 * @param a 第一个浮点数
 * @param b 第二个浮点数
 * @param eps 比较阈值（容差）
 *
 * @return 1 如果两个数的差值超过阈值（认为发生了变化）
 * @return 0 如果两个数的差值在阈值内（认为没有变化）
 */
static int float_changed(float a, float b, float eps)
{
    return (fabsf(a - b) > eps) ? 1 : 0;
}

/**
 * @brief 从缓存复制状态到快照
 *
 * 将设备状态缓存中的数据复制到MQTT状态快照结构体中。
 *
 * @param dst 目标快照结构体指针
 * @param src 源设备状态缓存指针
 */
static void snapshot_from_cache(mqtt_status_snapshot_t *dst, const device_status_cache_t *src)
{
    if ((dst == NULL) || (src == NULL))
    {
        return;
    }

    dst->temperature    = src->temperature;
    dst->humidity       = src->humidity;
    dst->light          = src->light;
    dst->voltage        = src->voltage;
    dst->led_state      = src->led_state;
    dst->relay_state    = src->relay_state;
    dst->buzzer_state   = src->buzzer_state;
    dst->alarm_code     = src->alarm_code;
    dst->link_state     = src->link_state;
    dst->tick_ms        = src->tick_ms;
    dst->last_update_ms = src->last_update_ms;
}

/**
 * @brief 判断设备状态是否发生变化
 *
 * 比较当前设备状态与上次上报的状态，判断是否有显著变化。
 * 用于决定是否需要重新上报状态，避免不必要的重复上报。
 *
 * @param st 当前设备状态缓存指针
 *
 * @return 1 如果状态发生变化
 * @return 0 如果状态没有变化或参数无效
 */
static int mqtt_status_changed(const device_status_cache_t *st)
{
    if (st == NULL)
    {
        return 0;
    }

    // 如果没有上次上报记录，认为发生了变化
    if (!s_has_last_reported)
    {
        return 1;
    }

    // 比较浮点数值（带容差）
    if (float_changed(st->temperature, s_last_reported.temperature, 0.1f)) return 1;
    if (float_changed(st->humidity,    s_last_reported.humidity,    0.1f)) return 1;
    if (st->light        != s_last_reported.light)        return 1;
    if (float_changed(st->voltage,     s_last_reported.voltage,     0.01f)) return 1;

    // 比较状态值
    if (st->led_state    != s_last_reported.led_state)    return 1;
    if (st->relay_state  != s_last_reported.relay_state)  return 1;
    if (st->buzzer_state != s_last_reported.buzzer_state) return 1;

    if (st->alarm_code   != s_last_reported.alarm_code)   return 1;
    if (st->link_state   != s_last_reported.link_state)   return 1;

    return 0;
}

/**
 * @brief 发布设备状态到MQTT服务器
 *
 * 将当前设备状态、参数和最后ACK信息组装成JSON格式并发布到MQTT主题。
 * 同时更新上次上报的快照和时间戳。
 */
static void mqtt_publish_status(void)
{
    char payload[512];
    device_status_cache_t *st = status_cache_get_device();
    ack_info_cache_t *ack = status_cache_get_ack();
    device_param_cache_t *param = param_cache_get();

    // 检查必要的缓存是否有效
    if ((s_mqtt_client == NULL) || (st == NULL) || (ack == NULL) || (param == NULL))
    {
        return;
    }

    // 组装JSON格式的状态数据
    snprintf(payload, sizeof(payload),
            "{"
            "\"temperature\":%.1f,"      // 温度
            "\"humidity\":%.1f,"         // 湿度
            "\"luminosity\":%u,"         // 光照强度
            "\"voltage\":%.2f,"          // 电压
            "\"led_state\":%u,"          // LED状态
            "\"relay_state\":%u,"        // 继电器状态
            "\"buzzer_state\":%u,"       // 蜂鸣器状态
            "\"alarm_code\":%u,"         // 报警码
            "\"link_state\":%u,"         // 连接状态
            "\"tick_ms\":%lu,"           // 系统滴答计数

            "\"upload_period_ms\":%u,"   // 上报周期
            "\"temp_high_th\":%u,"       // 温度高阈值
            "\"light_low_th\":%u,"       // 光照低阈值
            "\"work_mode\":%u,"          // 工作模式

            "\"last_ack_cmd\":%u,"       // 最后ACK命令
            "\"last_ack_seq\":%u,"       // 最后ACK序列号
            "\"last_ack_result\":%u"     // 最后ACK结果
            "}",
            st->temperature,
            st->humidity,
            st->light,
            st->voltage,
            st->led_state,
            st->relay_state,
            st->buzzer_state,
            st->alarm_code,
            st->link_state,
            (unsigned long)st->tick_ms,

            param->upload_period_ms,
            param->temp_high_th,
            param->light_low_th,
            param->work_mode,

            ack->ack_cmd,
            ack->ack_seq,
            ack->ack_result);

    // 发布到属性主题
    esp_mqtt_client_publish(s_mqtt_client,
                            PROJECT_MQTT_TOPIC_ATTRIBUTES,
                            payload,
                            0,  // qos
                            0,  // retain
                            0); // dup

    // 更新上报快照和时间戳
    snapshot_from_cache(&s_last_reported, st);
    s_has_last_reported = 1;
    s_last_publish_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);

    ESP_LOGI(TAG_MQTT_SERVICE, "attributes published");
}

/**
 * @brief MQTT事件处理器
 *
 * 处理MQTT客户端的各种事件，包括连接、断开、错误和数据接收。
 *
 * @param handler_args 处理器参数（未使用）
 * @param base 事件基础
 * @param event_id 事件ID
 * @param event_data 事件数据
 */
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    (void)handler_args;
    (void)base;

    ESP_LOGI(TAG_MQTT_SERVICE, "MQTT event id=%ld", (long)event_id);

    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:
        {
            s_mqtt_connected = 1;
            ESP_LOGI(TAG_MQTT_SERVICE, "MQTT connected");

            // 订阅命令主题
            esp_mqtt_client_subscribe(s_mqtt_client, PROJECT_MQTT_TOPIC_CMD, 0);
            ESP_LOGI(TAG_MQTT_SERVICE, "subscribed topic=%s", PROJECT_MQTT_TOPIC_CMD);

            /*
             * 刚连上先主动上报一次，便于云端立刻看到设备状态
             */
            mqtt_publish_status();
            break;
        }

        case MQTT_EVENT_DISCONNECTED:
        {
            s_mqtt_connected = 0;
            ESP_LOGW(TAG_MQTT_SERVICE, "MQTT disconnected");
            break;
        }

        case MQTT_EVENT_ERROR:
        {
            s_mqtt_connected = 0;
            ESP_LOGE(TAG_MQTT_SERVICE, "MQTT error type=%d",
                     event->error_handle->error_type);

            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
            {
                ESP_LOGE(TAG_MQTT_SERVICE,
                         "esp_tls_last_esp_err=0x%x, tls_stack_err=0x%x, sock_errno=%d",
                         event->error_handle->esp_tls_last_esp_err,
                         event->error_handle->esp_tls_stack_err,
                         event->error_handle->esp_transport_sock_errno);
            }
            break;
        }

        case MQTT_EVENT_DATA:
        {
            char topic[128] = {0};
            char data[256] = {0};
            int ret;

            // 复制主题字符串
            if (event->topic_len < (int)sizeof(topic))
            {
                memcpy(topic, event->topic, event->topic_len);
                topic[event->topic_len] = '\0';
            }

            // 复制数据字符串
            if (event->data_len < (int)sizeof(data))
            {
                memcpy(data, event->data, event->data_len);
                data[event->data_len] = '\0';
            }

            ESP_LOGI(TAG_MQTT_SERVICE, "MQTT data topic=%s data=%s", topic, data);

            // 处理命令主题的数据
            if (strcmp(topic, PROJECT_MQTT_TOPIC_CMD) == 0)
            {
                ret = mqtt_parse_command_json(data);
                if (ret != RET_OK)
                {
                    ESP_LOGW(TAG_MQTT_SERVICE, "command parse/exec failed ret=%d", ret);
                }
                else
                {
                    ESP_LOGI(TAG_MQTT_SERVICE, "command executed OK");
                }
            }
            break;
        }

        default:
            break;
    }
}

/**
 * @brief 初始化MQTT服务
 *
 * 配置MQTT客户端参数，注册事件处理器，启动客户端并创建发布任务。
 *
 * @return RET_OK 初始化成功
 */
int mqtt_service_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.hostname = PROJECT_MQTT_HOST,          // MQTT服务器主机名
        .broker.address.port = PROJECT_MQTT_PORT,              // MQTT服务器端口
        .broker.address.transport = MQTT_TRANSPORT_OVER_TCP,   // 传输协议

        .credentials.username = PROJECT_MQTT_USERNAME,         // 用户名
        .credentials.authentication.password = PROJECT_MQTT_PASSWORD, // 密码
        .credentials.client_id = "",                            // 客户端ID（空字符串让服务器分配）

        .session.protocol_ver = MQTT_PROTOCOL_V_3_1_1,         // MQTT协议版本
    };

    ESP_LOGI(TAG_MQTT_SERVICE, "mqtt_service_init");

    // 初始化上报状态标志
    s_has_last_reported = 0;
    s_last_publish_ms = 0U;

    // 初始化MQTT客户端
    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    // 注册事件处理器
    esp_mqtt_client_register_event(s_mqtt_client,
                                   ESP_EVENT_ANY_ID,
                                   mqtt_event_handler,
                                   NULL);
    // 启动MQTT客户端
    esp_mqtt_client_start(s_mqtt_client);

    // 创建MQTT发布任务
    xTaskCreate(mqtt_publish_task,
                "mqtt_publish_task",
                4096,               // 任务栈大小
                NULL,               // 任务参数
                7,                  // 任务优先级
                NULL);              // 任务句柄

    return RET_OK;
}

/**
 * @brief MQTT发布任务
 *
 * 定期检查设备状态变化和心跳周期，决定是否需要发布状态到MQTT服务器。
 * 只有在WiFi连接且MQTT连接的情况下才会发布。
 *
 * @param pvParameters 任务参数（未使用）
 */
static void mqtt_publish_task(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        // 检查WiFi和MQTT连接状态
        if (wifi_service_is_connected() && s_mqtt_connected)
        {
            uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
            device_status_cache_t *st = status_cache_get_device();
            int changed = 0;        // 状态是否变化
            int heartbeat_due = 0;  // 是否到心跳周期

            // 检查状态是否变化
            if (st != NULL)
            {
                changed = mqtt_status_changed(st);
            }

            // 检查是否到定期心跳上报时间
            if ((now_ms - s_last_publish_ms) >= PROJECT_MQTT_PUBLISH_PERIOD_MS)
            {
                heartbeat_due = 1;
            }

            // 如果状态变化或到心跳时间，则发布状态
            if (changed || heartbeat_due)
            {
                mqtt_publish_status();
            }
        }

        // 任务延时
        vTaskDelay(pdMS_TO_TICKS(PROJECT_MQTT_SCAN_PERIOD_MS));
    }
}

