#include "checksum.h"

uint8_t checksum8_sum(const uint8_t *buf, uint16_t len)
{
    uint16_t i = 0;
    uint8_t sum = 0U;

    if (buf == 0)
    {
        return 0U;
    }

    for (i = 0; i < len; i++)
    {
        sum = (uint8_t)(sum + buf[i]);
    }

    return sum;
}