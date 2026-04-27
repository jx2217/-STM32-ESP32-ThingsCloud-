#include "status_cache.h"
#include "common_def.h"

#include <string.h>

static device_status_cache_t s_device_status;
static ack_info_cache_t s_ack_info;

int status_cache_init(void)
{
    memset(&s_device_status, 0, sizeof(s_device_status));
    memset(&s_ack_info, 0, sizeof(s_ack_info));
    return RET_OK;
}

device_status_cache_t *status_cache_get_device(void)
{
    return &s_device_status;
}

ack_info_cache_t *status_cache_get_ack(void)
{
    return &s_ack_info;
}