#include "main.h"
#include "device_data.h"
#include "acquire_service.h"
#include "report_service.h"
#include "control_service.h"
#include "proto_rx.h"
#include "storage_service.h"
#include "buzzer.h"

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    proto_rx_uart_cplt_callback(huart);
}

void app_main(void)
{
    uint32_t last_acquire_tick = 0U;
    uint32_t last_report_tick  = 0U;
    int ret;

    device_data_init();
    storage_service_init();

    /*
     * 上电后尝试从 Flash 加载参数
     * 如果失败，说明：
     * - 首次上电
     * - 数据无效
     * - 校验失败
     * 这时使用默认参数，并可选择保存一次
     */
    ret = storage_service_load_param();
    if (ret != RET_OK)
    {
        (void)storage_service_reset_param_to_default();
    }
		
		device_data_init();
		buzzer_init();
    acquire_service_init();
    report_service_init();
    control_service_init();

    proto_rx_init();
    proto_rx_start();

    while (1)
    {
        uint32_t now = HAL_GetTick();
        device_param_t *param = device_data_get_param();

        if ((now - last_acquire_tick) >= param->sample_period_ms)
        {
            last_acquire_tick = now;
            acquire_service_update();
        }

        if ((now - last_report_tick) >= param->upload_period_ms)
        {
            last_report_tick = now;
            report_service_send_once();
        }

        HAL_Delay(10);
    }
}

