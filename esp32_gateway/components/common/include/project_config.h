#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* UART2 与 STM32 通信 */
#define PROJECT_UART_PORT_NUM       2
#define PROJECT_UART_BAUD_RATE      115200
#define PROJECT_UART_TX_PIN         17
#define PROJECT_UART_RX_PIN         16
#define PROJECT_UART_BUF_SIZE       256

/* Wi-Fi 配置：第一版先写死，后面再做 NVS/网页配置 */
#define PROJECT_WIFI_SSID           "TP-LINK_96C0"
#define PROJECT_WIFI_PASS           "104104104"

/* MQTT 配置：第一版先写死 */
// #define PROJECT_MQTT_BROKER_URI     "mqtt://192.168.1.100"
// #define PROJECT_MQTT_TOPIC_STATUS   "gateway/device001/status"
// #define PROJECT_MQTT_TOPIC_ACK      "gateway/device001/ack"
// #define PROJECT_MQTT_TOPIC_CMD      "gateway/device001/cmd"

#define PROJECT_MQTT_HOST             "sh-3-mqtt.iot-api.com"
#define PROJECT_MQTT_PORT             1883
#define PROJECT_MQTT_USERNAME         "ku7slllbtba33q06"
#define PROJECT_MQTT_PASSWORD         "kU9FbqpRb3"
#define PROJECT_MQTT_CLIENT_ID        ""

#define PROJECT_MQTT_TOPIC_ATTRIBUTES "attributes"

/* 先留空，等你确认 ThingsCloud 下行命令 topic 后再填 */
#define PROJECT_MQTT_TOPIC_CMD        "data/stream/set"

/* MQTT 状态上报周期 */
#define PROJECT_MQTT_PUBLISH_PERIOD_MS    10000
#define PROJECT_MQTT_SCAN_PERIOD_MS       200

#define TAG_APP_MAIN                "APP_MAIN"
#define TAG_UART_SERVICE            "UART_SERVICE"
#define TAG_PROTO_BRIDGE            "PROTO_BRIDGE"
#define TAG_STATUS_CACHE            "STATUS_CACHE"
#define TAG_APP_SERVICE             "APP_SERVICE"
#define TAG_STM32_CTRL              "STM32_CTRL"
#define TAG_CONSOLE_SERVICE         "CONSOLE"
#define TAG_WIFI_SERVICE            "WIFI"
#define TAG_MQTT_SERVICE            "MQTT"

#ifdef __cplusplus
}
#endif

#endif /* PROJECT_CONFIG_H */