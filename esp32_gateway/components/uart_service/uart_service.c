#include "uart_service.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#include "project_config.h"
#include "common_def.h"
#include "proto_bridge.h"

static void uart_rx_task(void *pvParameters)
{
    uint8_t rx_buf[PROJECT_UART_BUF_SIZE];

    (void)pvParameters;

    while (1)
    {
        int len = uart_read_bytes((uart_port_t)PROJECT_UART_PORT_NUM,
                                  rx_buf,
                                  sizeof(rx_buf),
                                  pdMS_TO_TICKS(100));

        if (len > 0)
        {
            proto_bridge_feed_bytes(rx_buf, (uint16_t)len);
        }
    }
}

int uart_service_init(void)
{
    uart_config_t uart_cfg = {
        .baud_rate = PROJECT_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_LOGI(TAG_UART_SERVICE, "uart_service_init, use UART%d TX=%d RX=%d",
             PROJECT_UART_PORT_NUM,
             PROJECT_UART_TX_PIN,
             PROJECT_UART_RX_PIN);

    uart_driver_install((uart_port_t)PROJECT_UART_PORT_NUM,
                        PROJECT_UART_BUF_SIZE * 2,
                        0,
                        0,
                        NULL,
                        0);

    uart_param_config((uart_port_t)PROJECT_UART_PORT_NUM, &uart_cfg);

    uart_set_pin((uart_port_t)PROJECT_UART_PORT_NUM,
                 PROJECT_UART_TX_PIN,
                 PROJECT_UART_RX_PIN,
                 UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);

    xTaskCreate(uart_rx_task,
                "uart_rx_task",
                4096,
                NULL,
                10,
                NULL);

    return RET_OK;
}

int uart_service_send_bytes(const uint8_t *buf, uint16_t len)
{
    int written = 0;

    if ((buf == 0) || (len == 0U))
    {
        return RET_INVALID_PARAM;
    }

    written = uart_write_bytes((uart_port_t)PROJECT_UART_PORT_NUM, buf, len);
    if (written < 0 || written != (int)len)
    {
        return RET_ERR;
    }

    return RET_OK;
}
