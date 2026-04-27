#include "param_cache.h"
#include "esp_timer.h"

static device_param_cache_t s_param_cache;

int param_cache_init(void)
{
    s_param_cache.upload_period_ms = 5000U;
    s_param_cache.temp_high_th     = 29U;
    s_param_cache.light_low_th     = 2000U;
    s_param_cache.work_mode        = 0U;   /* manual */
    s_param_cache.last_update_ms   = 0U;
    return 0;
}

device_param_cache_t *param_cache_get(void)
{
    return &s_param_cache;
}

void param_cache_set_upload_period(uint16_t period_ms)
{
    s_param_cache.upload_period_ms = period_ms;
    s_param_cache.last_update_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
}

void param_cache_set_temp_high_th(uint16_t temp_th)
{
    s_param_cache.temp_high_th = temp_th;
    s_param_cache.last_update_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
}

void param_cache_set_light_low_th(uint16_t light_th)
{
    s_param_cache.light_low_th = light_th;
    s_param_cache.last_update_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
}

void param_cache_set_work_mode(uint16_t mode)
{
    s_param_cache.work_mode = mode;
    s_param_cache.last_update_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
}
