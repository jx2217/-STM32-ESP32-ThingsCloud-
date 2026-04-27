/**
 * @file wifi_service.c
 * @brief WiFi服务模块
 *
 * 提供WiFi连接功能，管理WiFi STA模式的连接和断开。
 * 主要功能包括：
 * - 初始化WiFi系统
 * - 连接到指定的WiFi网络
 * - 处理连接/断开事件
 * - 提供连接状态查询接口
 */

#include "wifi_service.h"

#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "project_config.h"
#include "common_def.h"

/** @brief WiFi连接状态标志 (0=未连接, 1=已连接) */
static int s_wifi_connected = 0;

/**
 * @brief WiFi事件处理器
 *
 * 处理WiFi和IP事件，包括WiFi启动、连接断开和获得IP地址等事件。
 *
 * 处理的事件类型：
 * - WIFI_EVENT_STA_START: WiFi启动，开始连接
 * - WIFI_EVENT_STA_DISCONNECTED: WiFi连接断开，尝试重新连接
 * - IP_EVENT_STA_GOT_IP: 成功获得IP地址，WiFi连接完成
 *
 * @param arg 处理器参数（未使用）
 * @param event_base 事件基础（WIFI_EVENT或IP_EVENT）
 * @param event_id 事件ID
 * @param event_data 事件数据（未使用）
 */
static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    (void)arg; 
    (void)event_data;

    // WiFi启动事件：启动WiFi连接过程
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG_WIFI_SERVICE, "wifi start, try connect");
    }
    // WiFi断开连接事件：标记为未连接，并尝试重新连接
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        s_wifi_connected = 0;
        ESP_LOGW(TAG_WIFI_SERVICE, "wifi disconnected, reconnect...");
        esp_wifi_connect();
    }
    // IP事件：获得IP地址，标记为已连接
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        s_wifi_connected = 1;
        ESP_LOGI(TAG_WIFI_SERVICE, "wifi connected, got ip");
    }
}

/**
 * @brief 初始化WiFi服务
 *
 * 执行WiFi系统的完整初始化流程，包括：
 * - 初始化NVS存储
 * - 初始化网络接口
 * - 创建事件循环
 * - 初始化WiFi模块
 * - 注册事件处理器
 * - 配置WiFi SSID和密码
 * - 启动WiFi连接
 *
 * 初始化顺序很重要，必须按照WiFi系统的要求进行。
 * WiFi配置来自 project_config.h 中的宏定义：
 * - PROJECT_WIFI_SSID: WiFi网络名称
 * - PROJECT_WIFI_PASS: WiFi连接密码
 *
 * @return RET_OK 初始化成功
 */
int wifi_service_init(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();  // 获取WiFi默认配置
    wifi_config_t wifi_config = {0};                        // WiFi连接配置

    ESP_LOGI(TAG_WIFI_SERVICE, "wifi_service_init");

    // 初始化NVS（非易失性存储），用于保存WiFi配置
    nvs_flash_init();
    // 初始化网络接口
    esp_netif_init();
    // 创建默认事件循环
    esp_event_loop_create_default();
    // 创建默认WiFi STA接口
    esp_netif_create_default_wifi_sta();

    // 使用默认配置初始化WiFi驱动
    esp_wifi_init(&cfg);

    // 注册WiFi事件处理器（处理所有WiFi事件）
    esp_event_handler_register(WIFI_EVENT,
                               ESP_EVENT_ANY_ID,
                               &wifi_event_handler,
                               NULL);

    // 注册IP事件处理器（处理IP获取事件）
    esp_event_handler_register(IP_EVENT,
                               IP_EVENT_STA_GOT_IP,
                               &wifi_event_handler,
                               NULL);

    // 配置WiFi SSID（网络名称）
    strncpy((char *)wifi_config.sta.ssid,
            PROJECT_WIFI_SSID,
            sizeof(wifi_config.sta.ssid) - 1);

    // 配置WiFi密码
    strncpy((char *)wifi_config.sta.password,
            PROJECT_WIFI_PASS,
            sizeof(wifi_config.sta.password) - 1);

    // 设置WiFi工作模式为STA（Station模式，连接到AP）
    esp_wifi_set_mode(WIFI_MODE_STA);
    // 应用WiFi配置
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    // 启动WiFi（开始连接过程）
    esp_wifi_start();

    return RET_OK;
}

/**
 * @brief 查询WiFi连接状态
 *
 * 返回当前WiFi的连接状态。只有在成功获得IP地址后才认为连接成功。
 *
 * @return 1 if WiFi已连接（获得IP地址）
 * @return 0 if WiFi未连接
 */
int wifi_service_is_connected(void)
{
    return s_wifi_connected;
}


