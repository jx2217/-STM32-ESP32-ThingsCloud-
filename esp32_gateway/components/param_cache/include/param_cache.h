#ifndef PARAM_CACHE_H
#define PARAM_CACHE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t upload_period_ms;
    uint16_t temp_high_th;
    uint16_t light_low_th;
    uint16_t work_mode;
    uint32_t last_update_ms;
} device_param_cache_t;

int param_cache_init(void);
device_param_cache_t *param_cache_get(void);

void param_cache_set_upload_period(uint16_t period_ms);
void param_cache_set_temp_high_th(uint16_t temp_th);
void param_cache_set_light_low_th(uint16_t light_th);
void param_cache_set_work_mode(uint16_t mode);

#ifdef __cplusplus
}
#endif

#endif /* PARAM_CACHE_H */
