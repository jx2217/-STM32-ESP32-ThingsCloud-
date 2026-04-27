#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "project_config.h"
#include "status_cache.h"
#include "uart_service.h"
#include "app_service.h"
#include "stm32_ctrl.h"
#include "console_service.h"
#include "wifi_service.h"
#include "mqtt_service.h"
#include "param_cache.h"

void app_main(void)
{
    ESP_LOGI(TAG_APP_MAIN, "ESP32 gateway start");

    status_cache_init();
    param_cache_init();
    app_service_init();
    uart_service_init();
    stm32_ctrl_init();
    console_service_init();

    wifi_service_init();
    mqtt_service_init();

    xTaskCreate(console_service_task,
                "console_service_task",
                4096,
                NULL,
                8,
                NULL);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
