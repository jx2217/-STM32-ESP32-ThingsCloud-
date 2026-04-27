#ifndef PROTO_MSG_H
#define PROTO_MSG_H

#include "proto_cmd.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t cmd;
    uint8_t seq;
    uint8_t len;
    uint8_t data[PROTO_MAX_PAYLOAD];
} proto_msg_t;

#ifdef __cplusplus
}
#endif

#endif /* PROTO_MSG_H */
