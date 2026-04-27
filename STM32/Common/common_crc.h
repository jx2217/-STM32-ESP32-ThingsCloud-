#ifndef COMMON_CRC_H
#define COMMON_CRC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 第一版先用简单累加校验 */
uint8_t common_checksum8(const uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_CRC_H */
