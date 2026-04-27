#ifndef STATUS_CACHE_H
#define STATUS_CACHE_H

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
    uint32_t last_update_ms;
} device_status_cache_t;

typedef struct
{
    uint8_t ack_cmd;
    uint8_t ack_seq;
    uint8_t ack_result;
    uint32_t last_update_ms;
} ack_info_cache_t;

int status_cache_init(void);

device_status_cache_t *status_cache_get_device(void);
ack_info_cache_t *status_cache_get_ack(void);

#ifdef __cplusplus
}
#endif

#endif /* STATUS_CACHE_H */