#include "common_crc.h"

uint8_t common_checksum8(const uint8_t *buf, uint16_t len)
{
    uint16_t i = 0;
    uint8_t sum = 0;

    if (buf == 0U)
    {
        return 0U;
    }

    for (i = 0; i < len; i++)
    {
        sum = (uint8_t)(sum + buf[i]);
    }

    return sum;
}
