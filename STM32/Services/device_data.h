#ifndef DEVICE_DATA_H
#define DEVICE_DATA_H

#include "common_def.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float temperature;
    float humidity;
    uint16_t light;
    float voltage;

		uint8_t led_state; 
    uint8_t relay_state;
    uint8_t buzzer_state;

    uint8_t alarm_code;
    uint8_t link_state;

    uint32_t tick_ms;
} device_status_t;

typedef struct
{
    uint16_t sample_period_ms;
    uint16_t upload_period_ms;

    float temp_high_th;
    float light_low_th;

    uint8_t work_mode;
    uint8_t relay_default;
} device_param_t;

int device_data_init(void);

device_status_t *device_data_get_status(void);
device_param_t *device_data_get_param(void);

void device_data_set_tick(uint32_t tick_ms);
void device_data_set_link_state(uint8_t link_state);

/* 新增：设置上报周期 */
int device_data_set_upload_period(uint16_t period_ms);
int device_data_set_temp_high_th(uint16_t temp_th);
int device_data_set_light_low_th(uint16_t light_th);
int device_data_set_work_mode(uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_DATA_H */
