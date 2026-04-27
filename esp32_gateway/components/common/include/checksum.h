#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t checksum8_sum(const uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* CHECKSUM_H */